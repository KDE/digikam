//////////////////////////////////////////////////////////////////////////////
//
//    ALBUMICONITEM.CPP
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles Caulier <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

// Qt includes.

#include <qapplication.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpalette.h>
#include <qpen.h>
#include <qfontmetrics.h>
#include <qfont.h>
#include <qdatetime.h>
#include <qstringlist.h>

// KDE includes.

#include <kfileitem.h>
#include <kmimetype.h>
#include <kfilemetainfo.h>
#include <kurl.h>
#include <kio/global.h>
#include <kglobal.h>
#include <klocale.h>
#include <libkexif/kexifdata.h>

// X11 includes.

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Imlib2.h>

// Local includes.

#include "themeengine.h"
#include "thumbdb.h"
#include "thumbnailsize.h"
#include "albumsettings.h"
#include "albumiconview.h"
#include "albumiconitem.h"

void dateToString(const QDateTime& datetime, QString& str)
{
	str = KGlobal::locale()->formatDateTime(datetime, true, false);
}

QString squeezedText(QPainter* p, int width, const QString& text)
{
    QString fullText(text);
    fullText.replace("\n"," ");
    QFontMetrics fm(p->fontMetrics());
    int textWidth = fm.width(fullText);
    if (textWidth > width) {
        // start with the dots only
        QString squeezedText = "...";
        int squeezedWidth = fm.width(squeezedText);

        // estimate how many letters we can add to the dots on both sides
        int letters = fullText.length() * (width - squeezedWidth) / textWidth;
        if (width < squeezedWidth) letters=1;
        squeezedText = fullText.left(letters) + "...";
        squeezedWidth = fm.width(squeezedText);

        if (squeezedWidth < width) {
            // we estimated too short
            // add letters while text < label
            do {
                letters++;
                squeezedText = fullText.left(letters) + "..."; 
                squeezedWidth = fm.width(squeezedText);
            } while (squeezedWidth < width);
            letters--;
            squeezedText = fullText.left(letters) + "..."; 
        } else if (squeezedWidth > width) {
            // we estimated too long
            // remove letters while text > label
            do {
                letters--;
                squeezedText = fullText.left(letters) + "...";
                squeezedWidth = fm.width(squeezedText);
            } while (letters && squeezedWidth > width);
        }


        if (letters >= 5) {
            return squeezedText;
        }
    }
    
    return fullText;   
}

AlbumIconItem::AlbumIconItem(AlbumIconView* parent,
                             const QString& text,
                             const KFileItem* fileItem)
             : ThumbItem(parent, text, *parent->itemBaseRegPixmap())
{
    view_        = parent;
    fileItem_    = fileItem;
    metaInfo_    = 0;
    time_        = fileItem_->time(KIO::UDS_MODIFICATION_TIME);

    setRect(view_->itemRect());
}


AlbumIconItem::~AlbumIconItem()
{
    if (metaInfo_)
        delete metaInfo_;
}

void AlbumIconItem::setPixmap(const QPixmap& thumbnail,
                              const KFileMetaInfo* metaInfo)
{
    thumbnail_ = thumbnail;
    if (metaInfo_)
    {
        delete metaInfo_;
        metaInfo_ = 0;
    }
    metaInfo_  = metaInfo;

    QRect r(view_->contentsX(), view_->contentsY(),
            view_->visibleWidth(), view_->visibleHeight());
    if (r.intersects(rect()))
    {
        repaint();
    }
}

void AlbumIconItem::setMetaInfo(const KFileMetaInfo* metaInfo)
{
    if (metaInfo_)
    {
        delete metaInfo_;
        metaInfo_ = 0;
    }
    metaInfo_  = metaInfo;

    QRect r(view_->contentsX(), view_->contentsY(),
            view_->visibleWidth(), view_->visibleHeight());
    if (r.intersects(rect()))
    {
        repaint();
    }
}

int AlbumIconItem::compare(ThumbItem *item)
{
    const AlbumSettings *settings = view_->settings();
    AlbumIconItem *iconItem = static_cast<AlbumIconItem*>(item);
    
    switch (settings->getImageSortOrder())
    {
    case(AlbumSettings::ByIName):
    {
        return ThumbItem::compare(item);
    }
    case(AlbumSettings::ByIPath):
    {
        return fileItem_->url().path().compare(iconItem->fileItem_->url().path());
    }
    case(AlbumSettings::ByIDate):
    {
        if (time_ < iconItem->time_)
            return -1;
        else if (time_ > iconItem->time_)
            return 1;
        else
            return 0;
    }
    case(AlbumSettings::ByISize):
    {
        int mysize(fileItem_->size());
        int hissize(iconItem->fileItem_->size());
        if (mysize < hissize)
            return -1;
        else if (mysize > hissize)
            return 1;
        else
            return 0;
    }
    }

    return 0;
}

void AlbumIconItem::calcRect()
{

}

void AlbumIconItem::paintItem(QPainter *, const QColorGroup&)
{
    QPixmap pix;
    QRect   r;
    const AlbumSettings *settings = view_->settings();
    
    if (isSelected())
        pix = *(view_->itemBaseSelPixmap());
    else
        pix = *(view_->itemBaseRegPixmap());

    ThemeEngine* te = ThemeEngine::instance();
    
    QPainter p(&pix);
    p.setPen(isSelected() ? te->textSelColor() : te->textRegColor());

    if (thumbnail_.isNull())
    {
        int w = view_->thumbnailSize().size();
        ThumbDB::instance()->getThumb(fileItem_->url().path(1),  thumbnail_,
                                      w, w);
    }
            
    r = view_->itemPixmapRect();
    p.drawPixmap(r.x() + (r.width()-thumbnail_.width())/2,
                 r.y() + (r.height()-thumbnail_.height())/2,
                 thumbnail_);
    
    if (settings->getIconShowName())
    {
        r = view_->itemNameRect();
        p.setFont(view_->itemFontReg());
        p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(), text()));
    }

    p.setFont(view_->itemFontCom());
    
    if (settings->getIconShowComments())
    {
        QString comments(view_->itemComments(this));
        
        r = view_->itemCommentsRect();
        p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(), comments));
    }

    if (settings->getIconShowFileComments())
    {
        if (metaInfo_ && metaInfo_->isValid() &&
            metaInfo_->containsGroup("Jpeg EXIF Data"))
        {
            QString jpegComments = metaInfo_->group("Jpeg EXIF Data").
                                   item("Comment").value().toString();
        
            r = view_->itemFileCommentsRect();
            p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(),
                                                        jpegComments));
        }
    }
    
    p.setFont(view_->itemFontXtra());

    if (settings->getIconShowDate())
    {
        QDateTime date;
        date.setTime_t(time_);

        r = view_->itemDateRect();    
        p.setFont(view_->itemFontXtra());
        QString str;
        dateToString(date, str);
        p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(), str));
    }
    
    if (settings->getIconShowResolution())
    {
        if (metaInfo_ && metaInfo_->isValid())
        {
            QSize dims;
            if (metaInfo_->containsGroup("Jpeg EXIF Data"))
            {
                dims = metaInfo_->group("Jpeg EXIF Data").
                       item("Dimensions").value().toSize();
            }
            else if (metaInfo_->containsGroup("General"))
            {
                dims = metaInfo_->group("General").
                       item("Dimensions").value().toSize();
            }
            else if (metaInfo_->containsGroup("Technical"))
            {
                dims = metaInfo_->group("Technical").
                       item("Dimensions").value().toSize();
            }
            QString resolution = QString("%1x%2 %3")
                                 .arg(dims.width())
                                 .arg(dims.height())
                                 .arg(i18n("pixels"));
            r = view_->itemResolutionRect();    
            p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(), resolution));
        }
    }

    if (settings->getIconShowSize())
    {
        r = view_->itemSizeRect();    
        p.drawText(r, Qt::AlignCenter,
                   squeezedText(&p, r.width(), KIO::convertSize(fileItem_->size())));
    }

    p.setFont(view_->itemFontCom());
    p.setPen(isSelected() ? te->textSpecialSelColor() : te->textSpecialRegColor());

    if (settings->getIconShowTags())
    {
        QString tags(view_->itemTagNames(this).join(", "));
        
        r = view_->itemTagRect();    
        p.drawText(r, Qt::AlignCenter, 
                   squeezedText(&p, r.width(), tags));
    }
    
    p.end();
    
    r = rect();
    r = QRect(view_->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));

    bitBlt(view_->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width(), r.height());
}

QRect AlbumIconItem::thumbnailRect() const
{
    QRect pixmapRect = view_->itemPixmapRect();
    QRect r          = rect();
    
    return QRect(r.x()+pixmapRect.x() + (pixmapRect.width()-thumbnail_.width())/2,
                 r.y()+pixmapRect.y() + (pixmapRect.height()-thumbnail_.height())/2,
                 thumbnail_.width(), thumbnail_.height());
}

