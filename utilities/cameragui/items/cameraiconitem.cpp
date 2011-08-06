/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-21
 * Description : camera icon view item
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "cameraiconitem.moc"

// Qt includes

#include <QPainter>
#include <QPixmap>
#include <QTimer>

// KDE includes

#include <kpixmapsequence.h>
#include <kapplication.h>
#include <kdebug.h>

// Local includes

#include "iconview.h"
#include "thumbnailsize.h"
#include "imagedelegate.h"
#include "camiteminfo.h"
#include "cameraiconview.h"

namespace Digikam
{

class CameraIconItem::CameraIconItemPriv
{

public:

    CameraIconItemPriv() :
        hasThumb(false),
        progressCount(0),
        progressTimer(0)
    {
    }

    bool        hasThumb;
    int         progressCount;         // Position of animation during downloading.

    QSize       pixSize;

    QRect       pixRect;
    QRect       textRect;
    QRect       extraRect;

    QTimer*     progressTimer;

    CamItemInfo itemInfo;
};

CameraIconItem::CameraIconItem(IconGroupItem* parent, const CamItemInfo& itemInfo)
    : IconItem(parent), d(new CameraIconItemPriv)
{
    setItemInfo(itemInfo);

    d->hasThumb      = false;
    d->progressTimer = new QTimer(this);

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

CameraIconItem::~CameraIconItem()
{
    delete d;
}

void CameraIconItem::setItemInfo(const CamItemInfo& itemInfo)
{
    d->itemInfo = itemInfo;
}

CamItemInfo CameraIconItem::itemInfo() const
{
    return d->itemInfo;
}

void CameraIconItem::setDownloadName(const QString& downloadName)
{
    d->itemInfo.downloadName = downloadName;
    update();
}

void CameraIconItem::setDownloaded(int status)
{
    d->itemInfo.downloaded = status;
    d->progressCount       = 0;

    if (d->itemInfo.downloaded == CamItemInfo::DownloadStarted)
    {
        d->progressTimer->start(500);
    }
    else
    {
        d->progressTimer->stop();
    }
    update();
}

bool CameraIconItem::isDownloaded() const
{
    return (d->itemInfo.downloaded == CamItemInfo::DownloadedYes);
}

void CameraIconItem::toggleLock()
{
    if (d->itemInfo.writePermissions == 0)
    {
        d->itemInfo.writePermissions = 1;
    }
    else
    {
        d->itemInfo.writePermissions = 0;
    }
    update();
}

void CameraIconItem::calcRect(const QString& itemName, const QString& newName, const QPixmap& thumb)
{
    CameraIconView* view = static_cast<CameraIconView*>(iconView());
    const int border     = 8;
    int thumbSize        = view->thumbnailSize() - (2*border);
    d->pixSize           = thumb.size();
    d->pixSize.scale(thumbSize, thumbSize, Qt::KeepAspectRatio);
    d->pixRect           = QRect(0, 0, 0, 0);
    d->textRect          = QRect(0, 0, 0, 0);
    d->extraRect         = QRect(0, 0, 0, 0);
    QRect itemRect       = rect();
    itemRect.moveTopLeft(QPoint(0, 0));

    d->pixRect.setWidth(thumbSize);
    d->pixRect.setHeight(thumbSize);

    QFontMetrics fm(iconView()->font());
    QRect r = QRect(fm.boundingRect(0, 0, thumbSize+(2*border), 0xFFFFFFFF, Qt::AlignHCenter | Qt::AlignTop, itemName));
    d->textRect.setWidth(r.width());
    d->textRect.setHeight(r.height());

    if (!newName.isEmpty())
    {
        QFont fn(iconView()->font());

        if (fn.pointSize() > 0)
        {
            fn.setPointSize(qMax(fn.pointSize()-2, 6));
        }

        fm = QFontMetrics(fn);
        r  = QRect(fm.boundingRect(0, 0, thumbSize+(2*border), 0xFFFFFFFF, Qt::AlignHCenter | Qt::TextWordWrap, newName));
        d->extraRect.setWidth(r.width());
        d->extraRect.setHeight(r.height());

        d->textRect.setWidth(qMax(d->textRect.width(), d->extraRect.width()));
        d->textRect.setHeight(d->textRect.height() + d->extraRect.height());
    }

    int w = qMax(d->textRect.width(), d->pixRect.width());
    int h = d->textRect.height() + d->pixRect.height() ;

    itemRect.setWidth(w+border);
    itemRect.setHeight(h+border);

    // Center the pix and text rect
    d->pixRect  = QRect(border, border, d->pixRect.width(), d->pixRect.height());
    d->textRect = QRect((itemRect.width() - d->textRect.width())/2,
                        itemRect.height() - d->textRect.height() + border,
                        d->textRect.width(), d->textRect.height());

    if (!d->extraRect.isEmpty())
    {
        d->extraRect = QRect((itemRect.width() - d->extraRect.width())/2,
                             itemRect.height() - d->extraRect.height() + border,
                             d->extraRect.width(), d->extraRect.height());
    }
}

QRect CameraIconItem::clickToOpenRect()
{
    QRect r(rect());
    QRect pixRect(d->pixRect.x() + (d->pixRect.width()  - d->pixSize.width())  / 2,
                  d->pixRect.y() + (d->pixRect.height() - d->pixSize.height()) / 2,
                  d->pixSize.width(), d->pixSize.height());
    return pixRect.translated(r.x(), r.y());
}

void CameraIconItem::paintItem(QPainter* p)
{
    CameraIconView* view = static_cast<CameraIconView*>(iconView());
    CachedItem item      = view->getThumbInfo(itemInfo());

    CamItemInfo newinf = item.first;
    // NOTE: B.K.O #260669: do not overwrite downloaded information from DB which have been set before.
    newinf.downloaded  = itemInfo().downloaded;
    // NOTE: B.K.O #246336: do not overwrite too the file mtime set previously at camera connection using cameragui settings.
    newinf.mtime       = itemInfo().mtime;
    setItemInfo(newinf);

    QFont fn(view->font());
    QRect r(rect());

    QString itemName     = ImageDelegate::squeezedText(p->fontMetrics(), r.width()-5, d->itemInfo.name);
    QString downloadName = ImageDelegate::squeezedText(p->fontMetrics(), r.width()-5, d->itemInfo.downloadName);

    calcRect(itemName, downloadName, item.second);

    p->setPen(isSelected() ? kapp->palette().color(QPalette::HighlightedText)
                           : kapp->palette().color(QPalette::Text));

    QRect pixmapDrawRect(d->pixRect.x() + (d->pixRect.width()  - d->pixSize.width())  / 2,
                         d->pixRect.y() + (d->pixRect.height() - d->pixSize.height()) / 2,
                         d->pixSize.width(), d->pixSize.height());
    p->drawPixmap(pixmapDrawRect.topLeft(), item.second.scaled(d->pixSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    p->save();

    QRegion pixmapClipRegion = QRegion(0, 0, r.width(), r.height()) - QRegion(pixmapDrawRect);
    p->setClipRegion(pixmapClipRegion);

    if (isSelected())
    {
        p->drawPixmap(0, 0, view->itemBaseSelPixmap());
    }
    else
    {
        p->drawPixmap(0, 0, view->itemBaseRegPixmap());
    }

    QPixmap borderPix = view->thumbnailBorderPixmap(pixmapDrawRect.size());
    p->drawPixmap(pixmapDrawRect.x()-3, pixmapDrawRect.y()-3, borderPix);

    p->restore();
    p->drawText(d->textRect, Qt::AlignHCenter|Qt::AlignTop, itemName);

    if (!downloadName.isEmpty())
    {
        if (fn.pointSize() > 0)
        {
            fn.setPointSize(qMax(fn.pointSize()-2, 6));
        }

        QFont oldFn = p->font();
        p->setFont(fn);
        p->setPen(isSelected() ? kapp->palette().color(QPalette::HighlightedText)
                               : kapp->palette().color(QPalette::Link));
        p->drawText(d->extraRect, Qt::AlignHCenter|Qt::AlignTop, downloadName);
        p->setFont(oldFn);
    }

    if (this == iconView()->currentItem())
    {
        p->setPen(QPen(isSelected() ? Qt::white : Qt::black, 1, Qt::DotLine));
        p->drawRect(1, 1, r.width()-3, r.height()-3);
    }

    // Draw download status icon.
    QPixmap downloaded;

    switch (d->itemInfo.downloaded)
    {
        case CamItemInfo::NewPicture:
        {
            downloaded = view->newPicturePixmap();
            break;
        }
        case CamItemInfo::DownloadedYes:
        {
            downloaded = view->downloadedPixmap();
            break;
        }
        case CamItemInfo::DownloadStarted:
        {
            QPixmap mask(d->pixSize);
            mask.fill(QColor(128, 128, 128, 192));
            p->drawPixmap(pixmapDrawRect.topLeft(), mask);

            QPixmap anim(view->progressPixmap().frameAt(d->progressCount));
            d->progressCount++;

            if (d->progressCount >= view->progressPixmap().frameCount())
            {
                d->progressCount = 0;
            }

            p->save();
            int x = pixmapDrawRect.x() + pixmapDrawRect.width()  / 2  - anim.width() / 2;
            int y = pixmapDrawRect.y() + pixmapDrawRect.height() / 2 - anim.height() / 2;
            p->drawPixmap(x, y, anim);
            p->restore();
            break;
        }
        case CamItemInfo::DownloadFailed:
        {
            downloaded = view->downloadFailedPixmap();
            break;
        }
        case CamItemInfo::DownloadUnknown:
        {
            downloaded = view->downloadUnknownPixmap();
            break;
        }
    }

    if (!downloaded.isNull())
    {
        p->drawPixmap(rect().width() - downloaded.width() - 5, 5, downloaded);
    }

    // If camera item is locked (read only), draw a "Lock" icon.
    if (d->itemInfo.writePermissions == 0)
    {
        QPixmap locked = view->lockedPixmap();
        p->drawPixmap(rect().width() - downloaded.width() - locked.width() - 10, 5, locked);
    }

    if (isHighlighted())
    {
        paintToggleSelectButton(p);

        r = view->itemRect();
        p->setPen(QPen(view->palette().color(QPalette::Highlight), 3, Qt::SolidLine));
        p->drawRect(1, 1, r.width()-3, r.height()-3);
    }
}

void CameraIconItem::slotProgressTimerDone()
{
    update();
    d->progressTimer->start(300);
}

}  // namespace Digikam
