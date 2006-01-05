/* ============================================================
 * File  : albumiconitem.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-04-25
 * Description : 
 * 
 * Copyright 2003-2004 by Renchi Raju and Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */
 
// Qt includes.

#include <qpainter.h>
#include <qpixmap.h>
#include <qpalette.h>
#include <qpen.h>
#include <qfontmetrics.h>
#include <qfont.h>
#include <qdatetime.h>
#include <qstringlist.h>

// KDE includes.

#include <kurl.h>
#include <kglobal.h>
#include <klocale.h>
#include <kio/global.h>

// Local includes.

#include "themeengine.h"
#include "thumbnailsize.h"
#include "imageinfo.h"
#include "albumsettings.h"
#include "icongroupitem.h"
#include "pixmapmanager.h"
#include "albumiconview.h"
#include "albumiconitem.h"

namespace Digikam
{

static void dateToString(const QDateTime& datetime, QString& str)
{
    str = KGlobal::locale()->formatDateTime(datetime, true, false);
}

static QString squeezedText(QPainter* p, int width, const QString& text)
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

AlbumIconItem::AlbumIconItem(IconGroupItem* parent, ImageInfo* info)
             : IconItem(parent)
{
    view_        = (AlbumIconView*) parent->iconView();
    info_        = info;
    dirty_       = true;
}


AlbumIconItem::~AlbumIconItem()
{
}

ImageInfo* AlbumIconItem::imageInfo() const
{
    return info_;
}

int AlbumIconItem::compare(IconItem *item)
{
    const AlbumSettings *settings = view_->settings();
    AlbumIconItem *iconItem = static_cast<AlbumIconItem*>(item);
    
    switch (settings->getImageSortOrder())
    {
    case(AlbumSettings::ByIName):
    {
        return info_->name().localeAwareCompare(iconItem->info_->name());
    }
    case(AlbumSettings::ByIPath):
    {
        return info_->kurl().path().compare(iconItem->info_->kurl().path());
    }
    case(AlbumSettings::ByIDate):
    {
        if (info_->dateTime() < iconItem->info_->dateTime())
            return -1;
        else if (info_->dateTime() > iconItem->info_->dateTime())
            return 1;
        else
            return 0;
    }
    case(AlbumSettings::ByISize):
    {
        int mysize(info_->fileSize());
        int hissize(iconItem->info_->fileSize());
        if (mysize < hissize)
            return -1;
        else if (mysize > hissize)
            return 1;
        else
            return 0;
    }
    case(AlbumSettings::ByIRating):
    {
        int myrating(info_->rating());
        int hisrating(iconItem->info_->rating());
        if (myrating < hisrating)
            return 1;
        else if (myrating > hisrating)
            return -1;
        else
            return 0;
    }
    }

    return 0;
}

QRect AlbumIconItem::clickToOpenRect()
{
    if (tightPixmapRect_.isNull())
        return rect();
    
    QRect pixmapRect = tightPixmapRect_;
    QRect r          = rect();

    pixmapRect.moveBy(r.x(), r.y());
    return pixmapRect;
}

void AlbumIconItem::paintItem()
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


    dirty_ = true;
    
    QPixmap *thumbnail = view_->pixmapManager()->find(info_->kurl());
    if (thumbnail)
    {
        r = view_->itemPixmapRect();
        p.drawPixmap(r.x() + (r.width()-thumbnail->width())/2,
                     r.y() + (r.height()-thumbnail->height())/2,
                     *thumbnail);
        tightPixmapRect_.setRect(r.x() + (r.width()-thumbnail->width())/2,
                                 r.y() + (r.height()-thumbnail->height())/2,
                                 thumbnail->width(), thumbnail->height());
        dirty_ = false;
    }

    if (settings->getIconShowRating())
    {
        r = view_->itemRatingRect();
        QPixmap ratingPixmap = view_->ratingPixmap();

        int rating = info_->rating();
        
        int x, w;
        x = r.x() + (r.width() - rating * ratingPixmap.width())/2;
        w = rating * ratingPixmap.width();
        
        p.drawTiledPixmap(x, r.y(), w, r.height(), ratingPixmap);
    }
    
    
    if (settings->getIconShowName())
    {
        r = view_->itemNameRect();
        p.setFont(view_->itemFontReg());
        p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(),
                                                    info_->name()));
    }

    p.setFont(view_->itemFontCom());
    
    if (settings->getIconShowComments())
    {
        QString comments = info_->caption();
        
        r = view_->itemCommentsRect();
        p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(), comments));
    }

    p.setFont(view_->itemFontXtra());

    if (settings->getIconShowDate())
    {
        QDateTime date(info_->dateTime());

        r = view_->itemDateRect();    
        p.setFont(view_->itemFontXtra());
        QString str;
        dateToString(date, str);
        p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(), str));
    }
    
    if (settings->getIconShowResolution())
    {
        QSize dims = info_->dimensions();
        if (dims.isValid())
        {
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
                   squeezedText(&p, r.width(),
                                KIO::convertSize(info_->fileSize())));
    }

    p.setFont(view_->itemFontCom());
    p.setPen(isSelected() ? te->textSpecialSelColor() : te->textSpecialRegColor());

    if (settings->getIconShowTags())
    {
        QString tags = info_->tagNames().join(", ");
        
        r = view_->itemTagRect();    
        p.drawText(r, Qt::AlignCenter, 
                   squeezedText(&p, r.width(), tags));
    }

    if (this == view_->currentItem())
    {
        p.setPen(QPen(isSelected() ? te->textSelColor() : te->textRegColor(),
                      0, Qt::DotLine));
        p.drawRect(1, 1, pix.width()-2, pix.height()-2);
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

    pixmapRect.moveBy(r.x(), r.y());
    return pixmapRect;
}

}  // namespace Digikam

