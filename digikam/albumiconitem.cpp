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
#include <klocale.h>
#include <libkexif/kexifdata.h>

// X11 includes.

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Imlib2.h>

// Local includes.

#include "albumsettings.h"
#include "albumiconview.h"
#include "albumiconitem.h"


AlbumIconItem::AlbumIconItem(AlbumIconView* parent,
                             const QString& text,
                             const QPixmap& pix,
                             int size,
                             const KFileItem* fileItem,
                             int imageWidth, int imageHeight)
             : ThumbItem(parent, text, pix)
{
    view_        = parent;
    size_        = size;
    fileItem_    = fileItem;
    imageWidth_  = imageWidth;
    imageHeight_ = imageHeight;
    metaInfo_    = 0;
    
    updateExtraText();
    calcRect();
}


AlbumIconItem::~AlbumIconItem()
{
    if (metaInfo_)
        delete metaInfo_;
}


void AlbumIconItem::setPixmap(const QImage& thumbnail)
{
    QPainter painter;
    painter.begin(pixmap());
    painter.fillRect(0, 0, size_, size_,
                     QBrush(iconView()->colorGroup().base()));
    painter.drawImage((size_-thumbnail.width())/2,
                      (size_-thumbnail.height())/2,
                      thumbnail);
    painter.end();

    QRect pRect(iconView()->contentsRectToViewport(pixmapRect(false)));
    if (pRect.intersects(iconView()->visibleRect())) {
        pRect.moveBy(-1,-1);
        pRect.setWidth(pRect.width()+2);
        pRect.setHeight(pRect.height()+2);
        painter.begin(iconView()->viewport());
        painter.drawPixmap(pRect.x()+1, pRect.y()+1, *pixmap());
        painter.end();
    }

}

void AlbumIconItem::setPixmap(const QPixmap& thumbnail)
{
    QPainter painter;
    painter.begin(pixmap());
    painter.fillRect(0, 0, size_, size_,
                     QBrush(iconView()->colorGroup().base()));
    painter.drawPixmap((size_-thumbnail.width())/2,
                      (size_-thumbnail.height())/2,
                      thumbnail);
    painter.end();

    QRect pRect(iconView()->contentsRectToViewport(pixmapRect(false)));
    if (pRect.intersects(iconView()->visibleRect())) {
        pRect.moveBy(-1,-1);
        pRect.setWidth(pRect.width()+2);
        pRect.setHeight(pRect.height()+2);
        bitBlt(iconView()->viewport(), pRect.x()+1, pRect.y()+1,
               pixmap(), 0, 0, pRect.width(), pRect.height());
        //painter.begin(iconView()->viewport());
        //painter.drawPixmap(pRect.x()+1, pRect.y()+1, *pixmap());
        //painter.end();
    }
}

void AlbumIconItem::calcRect()
{
    QRect itemIconRect = QRect(0,0,0,0);
    QRect itemTextRect = QRect(0,0,0,0);
    itemExtraRect_ = QRect(0,0,0,0);
    QRect itemRect = rect();

    itemRect.setWidth(100);
    itemRect.setHeight(0xFFFFFFFF);

    // set initial pixrect
    int pw = pixmap()->width();
    int ph = pixmap()->height();

    itemIconRect.setWidth(pw);
    itemIconRect.setHeight(ph);

    // word wrap main text
    QFontMetrics fm(view_->font());
    QRect r = QRect(fm.boundingRect(0, 0, itemIconRect.width(),
                                    0xFFFFFFFF, Qt::AlignHCenter |
                                    Qt::WordBreak | Qt::BreakAnywhere
                                    | Qt::AlignTop,
                                    text()));
    r.setWidth(r.width() + 2);

    itemTextRect.setWidth(r.width());
    itemTextRect.setHeight(r.height());

    // word wrap extra Text
    if (!extraText_.isEmpty()) {

        QFont font(view_->font());
        int fontSize = font.pointSize();
        if (fontSize > 0) {
            font.setPointSize(fontSize-2);
        }
        else {
            fontSize = font.pixelSize();
            font.setPixelSize(fontSize-2);
        }
        
        fm = QFontMetrics(font);

        r = QRect(fm.boundingRect(0, 0, itemIconRect.width(),
                                  0xFFFFFFFF, Qt::AlignHCenter |
                                  Qt::WordBreak | Qt::BreakAnywhere
                                  | Qt::AlignTop,
                                  extraText_));
        r.setWidth(r.width() + 2);

        itemExtraRect_.setWidth(r.width());
        itemExtraRect_.setHeight(r.height());

        itemTextRect.setWidth(QMAX(itemTextRect.width(), itemExtraRect_.width()));
        itemTextRect.setHeight(itemTextRect.height() + itemExtraRect_.height());
    }


    // Now start updating the rects
    int w = QMAX(itemTextRect.width(), itemIconRect.width() );
    int h = itemTextRect.height() + itemIconRect.height() + 1;

    itemRect.setWidth(w);
    itemRect.setHeight(h);

    // Center the pix and text rect

    itemIconRect = QRect((itemRect.width() - itemIconRect.width())/2,
                         0,
                         itemIconRect.width(), itemIconRect.height());
    itemTextRect = QRect((itemRect.width() - itemTextRect.width())/2,
                         itemRect.height() - itemTextRect.height(),
                         itemTextRect.width(), itemTextRect.height());
    if (!itemExtraRect_.isEmpty()) {
        itemExtraRect_ = QRect((itemRect.width() - itemExtraRect_.width())/2,
                               itemRect.height() - itemExtraRect_.height(),
                               itemExtraRect_.width(), itemExtraRect_.height());
    }

    // Update rects
    if ( itemIconRect != pixmapRect() )
        setPixmapRect( itemIconRect );
    if ( itemTextRect != textRect() )
        setTextRect( itemTextRect );
    if ( itemRect != rect() )
        setRect( itemRect );

}

void AlbumIconItem::paintItem(QPainter *, const QColorGroup& cg)
{

    QRect pRect=pixmapRect(true);
    QRect tRect=textRect(true);

    QPixmap pix(rect().width(), rect().height());
    pix.fill(cg.base());
    QPainter painter(&pix);
    painter.drawPixmap(pRect.x(), pRect.y(), *pixmap() );

    if (isSelected()) {
        QPen pen;
        pen.setColor(cg.highlight());
        painter.setPen(pen);
        painter.drawRect(0, 0, pix.width(), pix.height());
        painter.fillRect(0, tRect.y(), pix.width(),
                     tRect.height(), cg.highlight() );
        painter.setPen( QPen( cg.highlightedText() ) );
    }
    else
        painter.setPen( cg.text() );

    painter.drawText(tRect, Qt::WordBreak|Qt::BreakAnywhere|
                     Qt::AlignHCenter|Qt::AlignTop,text());

    if (!extraText_.isEmpty()) {
        QFont font(view_->font());
        int fontSize = font.pointSize();
        if (fontSize > 0) {
            font.setPointSize(fontSize-2);
        }
        else {
            fontSize = font.pixelSize();
            font.setPixelSize(fontSize-2);
        }
        painter.setFont(font);
        if (!isSelected())
            painter.setPen(QPen("steelblue"));
        painter.drawText(itemExtraRect_, Qt::WordBreak|
                         Qt::BreakAnywhere|Qt::AlignHCenter|
                         Qt::AlignTop,extraText_);
    }

    painter.end();

    QRect r(rect());
    r = QRect(view_->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));

    bitBlt(view_->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width(), r.height());
}


void AlbumIconItem::updateExtraText()
{
    QString extraText;
    bool firstLine = true;

    const AlbumSettings *settings = view_->settings();
    if (!settings) return;

    if (settings->getIconShowMime()) {
        firstLine = false;
        KMimeType::Ptr mimePtr =
            KMimeType::findByURL(fileItem_->url());
        extraText += mimePtr->name();
    }

    if (settings->getIconShowSize()) {
        if (!firstLine)
            extraText += "\n";
        else
            firstLine = false;
        extraText += KIO::convertSize(fileItem_->size());
    }

    if (settings->getIconShowDate()) {
        if (!firstLine)
            extraText += "\n";
        else
            firstLine = false;
        QDateTime date;
        date.setTime_t(fileItem_->time(KIO::UDS_MODIFICATION_TIME));
        extraText += date.toString();
    }

    if (settings->getIconShowComments()) {
        QString comments;
        view_->getItemComments(text(), comments);
        if (!comments.isEmpty()) {
            if (!firstLine)
                extraText += "\n";
            else
                firstLine = false;
            extraText += comments;
        }
    }

    bool metaInfoRead = false;
    
    if(settings->getIconShowFileComments())
    {
        if (metaInfo_)
            delete metaInfo_;
        metaInfo_ = new KFileMetaInfo(fileItem_->url(), "image/jpeg",
                                      KFileMetaInfo::Fastest);
        metaInfoRead = true;
        
        // read and display JPEG COM comment and JPEG EXIF UserComment
        if(metaInfo_->isValid() && metaInfo_->mimeType() == "image/jpeg"
           && metaInfo_->containsGroup("Jpeg EXIF Data"))
        {
            KExifData *exifData = new KExifData;
            exifData->readFromFile(fileItem_->url().path());

            QString jpegComments = metaInfo_->group("Jpeg EXIF Data").
                                   item("Comment").value().toString();
            QString exifComments = exifData->getUserComment();

            QString comments;
            view_->getItemComments(text(), comments);

            jpegComments = jpegComments.stripWhiteSpace();
            exifComments = exifComments.stripWhiteSpace();


            // Only append comments if they are different from the regular comment
            if( !settings->getIconShowComments() || jpegComments != comments ) {
                if( jpegComments != "" ) {
                    if (!firstLine)
                        extraText += "\n";
                    else
                        firstLine = false;
                
                    extraText += jpegComments;
                }
            }

            if( !settings->getIconShowComments() || exifComments != comments ) {
                if( jpegComments != exifComments && exifComments != "" ) {
                    if (!firstLine)
                        extraText += "\n";
                    else
                        firstLine = false;
                
                    extraText += exifComments;
                }
            }

            delete exifData;
        }  

    }
    
    if ( settings->getIconShowResolution() )
    {

        if (metaInfo_ && !metaInfoRead) {
            delete metaInfo_;
            metaInfo_ = 0;
        }

        if (!metaInfo_) {
            metaInfo_ = new KFileMetaInfo(fileItem_->url(), "image/jpeg",
                                          KFileMetaInfo::Fastest);
        }

        // get image dimensions from JPEG meta info if possible
       
        if ( metaInfo_->isValid() &&
             metaInfo_->mimeType() == "image/jpeg" &&
             metaInfo_->containsGroup("Jpeg EXIF Data")) 
        {
            QSize jpegDimensions =  metaInfo_->group("Jpeg EXIF Data").
                                    item("Dimensions").value().toSize();
       
            imageWidth_ = jpegDimensions.width();
            imageHeight_ = jpegDimensions.height();
        }
        else   // Else using imlib2 API.
        {
            Imlib_Image imlib2_im = 0;
            imlib2_im = imlib_load_image(fileItem_->url().path().latin1());

            if (imlib2_im)
            {
                imlib_context_set_image(imlib2_im);
                imageWidth_= imlib_image_get_width();
                imageHeight_= imlib_image_get_height();
                imlib_free_image();
            }
        }
       
        if (imageWidth_ != 0 && imageHeight_ != 0) 
        {
            if (!firstLine) extraText += "\n";
            else firstLine = false;

            extraText += i18n("%1x%2 pixels").arg(QString::number(imageWidth_))
                         .arg(QString::number(imageHeight_));
        }
    }
    
    extraText_ = extraText;
}

void AlbumIconItem::setImageDimensions(int width, int height)
{
    imageWidth_  = width;
    imageHeight_ = height;
}

