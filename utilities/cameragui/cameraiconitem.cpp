/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2004-09-21
 * Description : camera icon view item 
 * 
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

// KDE includes.

#include <kiconloader.h>

// Local includes.

#include "iconview.h"
#include "thumbnailsize.h"
#include "albumiconitem.h"
#include "gpiteminfo.h"
#include "themeengine.h"
#include "cameraiconview.h"
#include "cameraiconitem.h"

namespace Digikam
{

class CameraIconViewItemPriv
{

public:

    CameraIconViewItemPriv()
    {
        itemInfo = 0;
    }

    QString     downloadName;

    QPixmap     pixmap;
    QPixmap     thumbnail;

    QRect       pixRect;
    QRect       textRect;
    QRect       extraRect;

    GPItemInfo *itemInfo;
};

CameraIconViewItem::CameraIconViewItem(IconGroupItem* parent, const GPItemInfo& itemInfo,
                                       const QImage& thumbnail, const QString& downloadName)
                  : IconItem(parent)
{
    d = new CameraIconViewItemPriv;
    d->itemInfo     = new GPItemInfo(itemInfo);
    d->downloadName = downloadName;
    setThumbnail(thumbnail);
}

CameraIconViewItem::~CameraIconViewItem()
{
    delete d->itemInfo;
    delete d;
}

void CameraIconViewItem::setThumbnail(const QImage& thumbnail)
{
    d->thumbnail = QPixmap(thumbnail);
}

GPItemInfo* CameraIconViewItem::itemInfo() const
{
    return d->itemInfo; 
}

void CameraIconViewItem::paintItem()
{
    CameraIconView* view = (CameraIconView*)iconView();
    QFont fn(view->font());

    QPixmap pix;
    QRect r(rect());

    if (isSelected())
        pix = *(view->itemBaseSelPixmap());
    else
        pix = *(view->itemBaseRegPixmap());
    
    ThemeEngine* te = ThemeEngine::instance();

    QPainter p(&pix);

    QString itemName     = AlbumIconItem::squeezedText(&p, r.width()-5, d->itemInfo->name);
    QString downloadName = AlbumIconItem::squeezedText(&p, r.width()-5, d->downloadName);
    calcRect(itemName, downloadName);

    p.setPen(isSelected() ? te->textSelColor() : te->textRegColor());

    p.drawPixmap(d->pixRect.x() + (d->pixRect.width()  - d->pixmap.width())  /2,
                 d->pixRect.y() + (d->pixRect.height() - d->pixmap.height()) /2,
                 d->pixmap);

    p.drawText(d->textRect, Qt::AlignHCenter|Qt::AlignTop, itemName);

    if (!d->downloadName.isEmpty())
    {
        if (fn.pointSize() > 0)
            fn.setPointSize(QMAX(fn.pointSize()-2, 6));

        p.setFont(fn);
        p.setPen(isSelected() ? te->textSpecialSelColor() : te->textSpecialRegColor());
        p.drawText(d->extraRect, Qt::AlignHCenter|Qt::AlignTop, downloadName);
    }

    if (this == iconView()->currentItem())
    {
        p.setPen(QPen(isSelected() ? Qt::white : Qt::black, 1, Qt::DotLine));
        p.drawRect(0, 0, r.width(), r.height());
    }

    // Draw download status icon.
    QPixmap downloaded;
    
    switch (d->itemInfo->downloaded)
    {
        case GPItemInfo::NewPicture:
        {
            downloaded = QPixmap(view->newPicturePixmap());
            break;
        }
        case GPItemInfo::DownloadedYes:
        {
            downloaded = SmallIcon( "button_ok" );
            break;
        }
        case GPItemInfo::DownloadStarted:
        {
            downloaded = SmallIcon( "run" );
            break;
        }
        case GPItemInfo::DownloadFailed:
        {
            downloaded = SmallIcon( "button_cancel" );
            break;
        }
        /* TODO: see B.K.O #107316 : disable temporally the unknow download status until     
                 a new method to identify the already downloaded pictures from camera is 
                 implemented.
  
        case GPItemInfo::DownloadUnknow:
        {
            downloaded = view->unknowPicturePixmap();
            break;
        }
        */
    }

    if (!downloaded.isNull())
        p.drawPixmap(rect().width() - downloaded.width() - 5, 5, downloaded);

    // If camera item is locked (read only), draw a "Lock" icon.
    if (d->itemInfo->writePermissions == 0) 
        p.drawPixmap(5, 5, SmallIcon( "encrypted" ));

    p.end();

    r = QRect(view->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));

    bitBlt(view->viewport(), r.x(), r.y(), &pix);
}

void CameraIconViewItem::setDownloadName(const QString& downloadName)
{
    d->downloadName = downloadName;
    repaint();
}

QString CameraIconViewItem::getDownloadName() const
{
    return d->downloadName;
}

void CameraIconViewItem::setDownloaded(int status)
{
    d->itemInfo->downloaded = status;
    repaint();
}

void CameraIconViewItem::toggleLock()
{
    if (d->itemInfo->writePermissions == 0) 
        d->itemInfo->writePermissions = 1;
    else 
        d->itemInfo->writePermissions = 0;

    repaint();
}

void CameraIconViewItem::calcRect(const QString& itemName, const QString& downloadName)
{
    CameraIconView* view = (CameraIconView*)iconView();
    int thumbSize        = view->thumbnailSize().size();
    d->pixmap            = QPixmap(d->thumbnail.convertToImage().smoothScale(thumbSize, thumbSize, QImage::ScaleMin));
    d->pixRect           = QRect(0,0,0,0);
    d->textRect          = QRect(0,0,0,0);
    d->extraRect         = QRect(0,0,0,0);
    QRect itemRect       = rect();
    itemRect.moveTopLeft(QPoint(0, 0));

    d->pixRect.setWidth(thumbSize);
    d->pixRect.setHeight(thumbSize);

    QFontMetrics fm(iconView()->font());
    QRect r = QRect(fm.boundingRect(0, 0, thumbSize, 0xFFFFFFFF,
                                    Qt::AlignHCenter | Qt::AlignTop,
                                    itemName));
    d->textRect.setWidth(r.width());
    d->textRect.setHeight(r.height());

    if (!d->downloadName.isEmpty())
    {
        QFont fn(iconView()->font());
        if (fn.pointSize() > 0)
        {
            fn.setPointSize(QMAX(fn.pointSize()-2, 6));
        }

        fm = QFontMetrics(fn);
        r  = QRect(fm.boundingRect(0, 0, thumbSize, 0xFFFFFFFF,
                                   Qt::AlignHCenter | Qt::WordBreak,
                                   downloadName));
        d->extraRect.setWidth(r.width());
        d->extraRect.setHeight(r.height());

        d->textRect.setWidth(QMAX(d->textRect.width(), d->extraRect.width()));
        d->textRect.setHeight(d->textRect.height() + d->extraRect.height());
    }
    
    int w = QMAX(d->textRect.width(), d->pixRect.width() );
    int h = d->textRect.height() + d->pixRect.height() ;

    itemRect.setWidth(w+4);
    itemRect.setHeight(h+4);

    // Center the pix and text rect
    d->pixRect  = QRect(2, 2, d->pixRect.width(), d->pixRect.height());
    d->textRect = QRect((itemRect.width() - d->textRect.width())/2,
                        itemRect.height() - d->textRect.height(),
                        d->textRect.width(), d->textRect.height());

    if (!d->extraRect.isEmpty())
    {
        d->extraRect = QRect((itemRect.width() - d->extraRect.width())/2,
                             itemRect.height() - d->extraRect.height(),
                             d->extraRect.width(), d->extraRect.height());
    }
}

QRect CameraIconViewItem::clickToOpenRect()
{
    QRect r(rect());
    
    if (d->pixmap.isNull())
    {
        QRect pixRect(d->pixRect);
        pixRect.moveBy(r.x(), r.y());
        return pixRect;
    }

    QRect pixRect(d->pixRect.x() + (d->pixRect.width()  - d->pixmap.width())/2,
                  d->pixRect.y() + (d->pixRect.height() - d->pixmap.height())/2,
                  d->pixmap.width(), d->pixmap.height());
    pixRect.moveBy(r.x(), r.y());
    return pixRect;
}

}  // namespace Digikam
