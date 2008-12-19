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

#include "cameraiconitem.h"

// Qt includes.

#include <QPainter>
#include <QPixmap>

// KDE includes.

#include <kiconloader.h>

// Local includes.

#include "iconview.h"
#include "thumbnailsize.h"
#include "albumiconitem.h"
#include "gpiteminfo.h"
#include "themeengine.h"
#include "cameraiconview.h"

namespace Digikam
{

class CameraIconItemPriv
{

public:

    CameraIconItemPriv()
    {
        itemInfo            = 0;
        pixmapNewPicture    = QPixmap(newPicture_xpm);
        pixmapUnknowPicture = QPixmap(unknowPicture_xpm);
    }

    static const char *newPicture_xpm[];
    static const char *unknowPicture_xpm[];

    QString            downloadName;

    QPixmap            pixmap;
    QPixmap            pixmapNewPicture;
    QPixmap            pixmapUnknowPicture;

    QImage             thumbnail;

    QRect              pixRect;
    QRect              textRect;
    QRect              extraRect;

    GPItemInfo        *itemInfo;
};

const char *CameraIconItemPriv::newPicture_xpm[] =
{
    "13 13 8 1",
    "       c None",
    ".      c #232300",
    "+      c #F6F611",
    "@      c #000000",
    "#      c #DBDA4D",
    "$      c #FFFF00",
    "%      c #AAA538",
    "&      c #E8E540",
    "      .      ",
    "  .  .+.  .  ",
    " @#@ .$. .#. ",
    "  @$@#$#@$.  ",
    "   @$%&%$@   ",
    " ..#%&&&%#.. ",
    ".+$$&&&&&$$+@",
    " ..#%&&&%#@@ ",
    "   @$%&%$@   ",
    "  .$@#$#@$.  ",
    " @#. @$@ @#. ",
    "  .  @+@  .  ",
    "      @      "
};

const char *CameraIconItemPriv::unknowPicture_xpm[] =
{
    "16 16 78 1",
    "   g None",
    ".  g #777777",
    "+  g #7A7A7A",
    "@  g #8C8C8C",
    "#  g #787878",
    "$  g #707070",
    "%  g #878787",
    "&  g #C3C3C3",
    "*  g #EAEAEA",
    "=  g #E4E4E4",
    "-  g #E2E2E2",
    ";  g #E6E6E6",
    ">  g #CECECE",
    ",  g #888888",
    "'  g #6B6B6B",
    ")  g #969696",
    "!  g #DEDEDE",
    "~  g #D8D8D8",
    "{  g #FFFFFF",
    "]  g #F2F2F2",
    "^  g #DFDFDF",
    "/  g #9D9D9D",
    "(  g #686868",
    "_  g #848484",
    ":  g #D0D0D0",
    "<  g #F1F1F1",
    "[  g #F0F0F0",
    "}  g #EBEBEB",
    "|  g #FDFDFD",
    "1  g #DDDDDD",
    "2  g #D4D4D4",
    "3  g #838383",
    "4  g #ABABAB",
    "5  g #C8C8C8",
    "6  g #CCCCCC",
    "7  g #F4F4F4",
    "8  g #D6D6D6",
    "9  g #E8E8E8",
    "0  g #C4C4C4",
    "a  g #A4A4A4",
    "b  g #656565",
    "c  g #B4B4B4",
    "d  g #B9B9B9",
    "e  g #BDBDBD",
    "f  g #B7B7B7",
    "g  g #898989",
    "h  g #6D6D6D",
    "i  g #808080",
    "j  g #AAAAAA",
    "k  g #A9A9A9",
    "l  g #737373",
    "m  g #7F7F7F",
    "n  g #9A9A9A",
    "o  g #D3D3D3",
    "p  g #909090",
    "q  g #727272",
    "r  g #8F8F8F",
    "s  g #8E8E8E",
    "t  g #8D8D8D",
    "u  g #EEEEEE",
    "v  g #FAFAFA",
    "w  g #929292",
    "x  g #C5C5C5",
    "y  g #5F5F5F",
    "z  g #989898",
    "A  g #CFCFCF",
    "B  g #9C9C9C",
    "C  g #A0A0A0",
    "D  g #FEFEFE",
    "E  g #ACACAC",
    "F  g #5E5E5E",
    "G  g #868686",
    "H  g #AFAFAF",
    "I  g #C1C1C1",
    "J  g #818181",
    "K  g #7E7E7E",
    "L  g #7B7B7B",
    "M  g #636363",
    "                ",
    "     .+@@#$     ",
    "   .%&*=-;>,'   ",
    "  .)!~={{]^-/(  ",
    "  _::<{[}|{123  ",
    " .456{7558{90ab ",
    " +cde96df={&g,h ",
    " ijjjjjk;{=@,,l ",
    " mnnnnno{-pgggq ",
    " #rprstuvwtttt' ",
    " $tpppp6xpppp@y ",
    "  mnnnzA~Bnnn.  ",
    "  'taaCD{Eaa,F  ",
    "   (GjHI0HjJF   ",
    "     (K,,LM     ",
    "                "
};

CameraIconItem::CameraIconItem(IconGroupItem* parent, const GPItemInfo& itemInfo,
                                       const QImage& thumbnail, const QString& downloadName)
                  : IconItem(parent), d(new CameraIconItemPriv)
{
    d->itemInfo     = new GPItemInfo(itemInfo);
    d->downloadName = downloadName;
    d->thumbnail    = thumbnail;
}

CameraIconItem::~CameraIconItem()
{
    delete d->itemInfo;
    delete d;
}

void CameraIconItem::setThumbnail(const QImage& thumbnail)
{
    d->thumbnail = thumbnail;
}

GPItemInfo* CameraIconItem::itemInfo() const
{
    return d->itemInfo;
}

void CameraIconItem::setDownloadName(const QString& downloadName)
{
    d->downloadName = downloadName;
    update();
}

QString CameraIconItem::getDownloadName() const
{
    return d->downloadName;
}

void CameraIconItem::setDownloaded(int status)
{
    d->itemInfo->downloaded = status;
    update();
}

void CameraIconItem::toggleLock()
{
    if (d->itemInfo->writePermissions == 0)
        d->itemInfo->writePermissions = 1;
    else
        d->itemInfo->writePermissions = 0;

    update();
}

void CameraIconItem::calcRect(const QString& itemName, const QString& downloadName)
{
    CameraIconView* view = static_cast<CameraIconView*>(iconView());
    const int border     = 8;
    int thumbSize        = view->thumbnailSize() - (2*border);
    d->pixmap            = QPixmap::fromImage(d->thumbnail.scaled(thumbSize, thumbSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    d->pixRect           = QRect(0, 0, 0, 0);
    d->textRect          = QRect(0, 0, 0, 0);
    d->extraRect         = QRect(0, 0, 0, 0);
    QRect itemRect       = rect();
    itemRect.moveTopLeft(QPoint(0, 0));

    d->pixRect.setWidth(thumbSize);
    d->pixRect.setHeight(thumbSize);

    QFontMetrics fm(iconView()->font());
    QRect r = QRect(fm.boundingRect(0, 0, thumbSize+(2*border), 0xFFFFFFFF,
                                    Qt::AlignHCenter | Qt::AlignTop,
                                    itemName));
    d->textRect.setWidth(r.width());
    d->textRect.setHeight(r.height());

    if (!d->downloadName.isEmpty())
    {
        QFont fn(iconView()->font());
        if (fn.pointSize() > 0)
        {
            fn.setPointSize(qMax(fn.pointSize()-2, 6));
        }

        fm = QFontMetrics(fn);
        r  = QRect(fm.boundingRect(0, 0, thumbSize+(2*border), 0xFFFFFFFF,
                                   Qt::AlignHCenter | Qt::TextWordWrap,
                                   downloadName));
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

    if (d->pixmap.isNull())
    {
        return d->pixRect.translated(r.x(), r.y());
    }

    QRect pixRect(d->pixRect.x() + (d->pixRect.width()  - d->pixmap.width())/2,
                  d->pixRect.y() + (d->pixRect.height() - d->pixmap.height())/2,
                  d->pixmap.width(), d->pixmap.height());
    return pixRect.translated(r.x(), r.y());
}

void CameraIconItem::paintItem(QPainter *p)
{
    ThemeEngine* te      = ThemeEngine::instance();
    CameraIconView* view = static_cast<CameraIconView*>(iconView());

    QFont fn(view->font());
    QRect r(rect());

    QString itemName     = AlbumIconItem::squeezedText(p, r.width()-5, d->itemInfo->name);
    QString downloadName = AlbumIconItem::squeezedText(p, r.width()-5, d->downloadName);

    calcRect(itemName, downloadName);

    p->setPen(isSelected() ? te->textSelColor() : te->textRegColor());

    QRect pixmapDrawRect(d->pixRect.x() + (d->pixRect.width()  - d->pixmap.width())  /2,
                         d->pixRect.y() + (d->pixRect.height() - d->pixmap.height()) /2,
                         d->pixmap.width(), d->pixmap.height());
    p->drawPixmap(pixmapDrawRect.topLeft(), d->pixmap);
    p->save();

    QRegion pixmapClipRegion = QRegion(0, 0, r.width(), r.height()) - QRegion(pixmapDrawRect);
    p->setClipRegion(pixmapClipRegion);
    if (isSelected())
        p->drawPixmap(0, 0, view->itemBaseSelPixmap());
    else
        p->drawPixmap(0, 0, view->itemBaseRegPixmap());

    QPixmap borderPix = view->thumbnailBorderPixmap(pixmapDrawRect.size());
    p->drawPixmap(pixmapDrawRect.x()-3, pixmapDrawRect.y()-3, borderPix);

    p->restore();
    p->drawText(d->textRect, Qt::AlignHCenter|Qt::AlignTop, itemName);

    if (!d->downloadName.isEmpty())
    {
        if (fn.pointSize() > 0)
            fn.setPointSize(qMax(fn.pointSize()-2, 6));

        QFont oldFn = p->font();
        p->setFont(fn);
        p->setPen(isSelected() ? te->textSpecialSelColor() : te->textSpecialRegColor());
        p->drawText(d->extraRect, Qt::AlignHCenter|Qt::AlignTop, downloadName);
        p->setFont(oldFn);
    }

    if (this == iconView()->currentItem())
    {
        p->setPen(QPen(isSelected() ? Qt::white : Qt::black, 1, Qt::DotLine));
        p->drawRect(0, 0, r.width(), r.height());
    }

    // Draw download status icon.
    QPixmap downloaded;

    switch (d->itemInfo->downloaded)
    {
        case GPItemInfo::NewPicture:
        {
            downloaded = d->pixmapNewPicture;
            break;
        }
        case GPItemInfo::DownloadedYes:
        {
            downloaded = SmallIcon("dialog-ok");
            break;
        }
        case GPItemInfo::DownloadStarted:
        {
            downloaded = SmallIcon("system-run");
            break;
        }
        case GPItemInfo::DownloadFailed:
        {
            downloaded = SmallIcon("dialog-cancel");
            break;
        }
        case GPItemInfo::DownloadUnknow:
        {
            downloaded = d->pixmapUnknowPicture;
            break;
        }
    }

    if (!downloaded.isNull())
        p->drawPixmap(rect().width() - downloaded.width() - 5, 5, downloaded);

    // If camera item is locked (read only), draw a "Lock" icon.
    if (d->itemInfo->writePermissions == 0)
    {
        QPixmap locked = SmallIcon("object-locked");
        p->drawPixmap(rect().width() - downloaded.width() - locked.width() - 10, 5, locked);
    }

    if (isHighlighted())
    {
        r = view->itemRect();
        p->setPen(QPen(view->palette().color(QPalette::Highlight), 3, Qt::SolidLine));
        p->drawRect(1, 1, r.width()-3, r.height()-3);
        p->drawPixmap(toggleSelectRect(), isSelected() ? view->deselectPixmap()
                                                       : view->selectPixmap());
    }
}

}  // namespace Digikam
