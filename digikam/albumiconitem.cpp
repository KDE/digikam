/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-04-25
 * Description : implementation to render album icon item.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "albumiconitem.h"

// Qt includes.

#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPalette>
#include <QPen>
#include <QPixmap>
#include <QStringList>

// KDE includes.

#include <kglobal.h>
#include <kio/global.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kurl.h>

// Local includes.

#include "albumiconview.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "icongroupitem.h"
#include "imageinfo.h"
#include "themeengine.h"
#include "thumbbar.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"

namespace Digikam
{

class AlbumIconItemPriv
{
public:

    AlbumIconItemPriv()
    {
        dirty = true;
        view  = 0;
    }

    bool           dirty;

    QRect          tightPixmapRect;

    ImageInfo      info;

    AlbumIconView *view;
};

AlbumIconItem::AlbumIconItem(IconGroupItem* parent, const ImageInfo &info)
             : IconItem(parent), d(new AlbumIconItemPriv)
{
    d->view = (AlbumIconView*) parent->iconView();
    d->info = info;
}

AlbumIconItem::~AlbumIconItem()
{
    delete d;
}

void AlbumIconItem::dateToString(const QDateTime& datetime, QString& str)
{
    str = KGlobal::locale()->formatDateTime(datetime, KLocale::ShortDate, false);
}

QString AlbumIconItem::squeezedText(QPainter* p, int width, const QString& text)
{
    QString fullText(text);
    fullText.replace('\n',' ');
    QFontMetrics fm(p->fontMetrics());
    int textWidth = fm.width(fullText);

    if (textWidth > width)
    {
        // start with the dots only
        QString squeezedText = "...";
        int squeezedWidth    = fm.width(squeezedText);

        // estimate how many letters we can add to the dots on both sides
        int letters = fullText.length() * (width - squeezedWidth) / textWidth;
        if (width < squeezedWidth) letters=1;
        squeezedText  = fullText.left(letters) + "...";
        squeezedWidth = fm.width(squeezedText);

        if (squeezedWidth < width)
        {
            // we estimated too short
            // add letters while text < label
            do
            {
                letters++;
                squeezedText  = fullText.left(letters) + "...";
                squeezedWidth = fm.width(squeezedText);
            }
            while (squeezedWidth < width);

            letters--;
            squeezedText = fullText.left(letters) + "...";
        }
        else if (squeezedWidth > width)
        {
            // we estimated too long
            // remove letters while text > label
            do
            {
                letters--;
                squeezedText  = fullText.left(letters) + "...";
                squeezedWidth = fm.width(squeezedText);
            }
            while (letters && squeezedWidth > width);
        }

        if (letters >= 5)
        {
            return squeezedText;
        }
    }

    return fullText;
}

bool AlbumIconItem::isDirty()
{
    return d->dirty;
}

ImageInfo AlbumIconItem::imageInfo() const
{
    return d->info;
}

QString AlbumIconItem::filePath() const
{
    return d->info.filePath();
}

int AlbumIconItem::compare(IconItem *item)
{
    const AlbumSettings *settings = d->view->settings();
    AlbumIconItem *iconItem = static_cast<AlbumIconItem*>(item);

    switch (settings->getImageSortOrder())
    {
        case(AlbumSettings::ByIName):
        {
            return KStringHandler::naturalCompare(d->info.name(), iconItem->d->info.name());
        }
        case(AlbumSettings::ByIPath):
        {
            return KStringHandler::naturalCompare(d->info.fileUrl().path(), iconItem->d->info.fileUrl().path());
        }
        case(AlbumSettings::ByIDate):
        {
            if (d->info.dateTime() < iconItem->d->info.dateTime())
                return -1;
            else if (d->info.dateTime() > iconItem->d->info.dateTime())
                return 1;
            else
                return 0;
        }
        case(AlbumSettings::ByISize):
        {
            int mysize(d->info.fileSize());
            int hissize(iconItem->d->info.fileSize());
            if (mysize < hissize)
                return -1;
            else if (mysize > hissize)
                return 1;
            else
                return 0;
        }
        case(AlbumSettings::ByIRating):
        {
            int myrating(d->info.rating());
            int hisrating(iconItem->d->info.rating());
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

QRect AlbumIconItem::thumbnailRect() const
{
    QRect pixmapRect = d->view->itemPixmapRect();
    QRect r          = rect();

    pixmapRect.translate(r.x(), r.y());
    return pixmapRect;
}

QRect AlbumIconItem::clickToOpenRect()
{
    if (d->tightPixmapRect.isNull())
        return rect();

    QRect pixmapRect = d->tightPixmapRect;
    QRect r          = rect();

    pixmapRect.translate(r.x(), r.y());
    return pixmapRect;
}

void AlbumIconItem::paintItem(QPainter *p)
{
    QRect r;
    const AlbumSettings *settings = d->view->settings();
    ThemeEngine* te               = ThemeEngine::instance();

    QPixmap pix;
    if (isSelected())
        pix = d->view->itemBaseSelPixmap();
    else
        pix = d->view->itemBaseRegPixmap();

    p->setPen(isSelected() ? te->textSelColor() : te->textRegColor());

    d->dirty = true;

    QPixmap thumbnail;
    if (ThumbnailLoadThread::defaultIconViewThread()->find(d->info.filePath(), thumbnail))
    {
        r = d->view->itemPixmapRect();
        p->drawPixmap(r.x() + (r.width()-thumbnail.width())/2,
                      r.y() + (r.height()-thumbnail.height())/2,
                      thumbnail);

        d->tightPixmapRect.setRect(r.x() + (r.width()-thumbnail.width())/2,
                                   r.y() + (r.height()-thumbnail.height())/2,
                                   thumbnail.width(), thumbnail.height());
        d->dirty = false;

        p->save();
        QRegion pixmapClipRegion = QRegion(d->view->itemRect()) - QRegion(d->tightPixmapRect);
        p->setClipRegion(pixmapClipRegion);
        p->drawPixmap(0, 0, pix);

        QPixmap borderPix = d->view->thumbnailBorderPixmap(d->tightPixmapRect.size());
        p->drawPixmap(d->tightPixmapRect.x()-3, d->tightPixmapRect.y()-3, borderPix);

        p->restore();

    }
    else
    {
        // simplified
        p->drawPixmap(0, 0, pix);
    }

    if (settings->getIconShowRating())
    {
        r = d->view->itemRatingRect();

        int rating = d->info.rating();

        if (rating > 0 && rating <=5)
        {
            QPixmap ratingPixmap = d->view->ratingPixmap(rating, isSelected());
            p->drawPixmap(r, ratingPixmap);
        }
    }

    if (settings->getIconShowName())
    {
        r = d->view->itemNameRect();
        p->setFont(d->view->itemFontReg());
        p->drawText(r, Qt::AlignCenter, squeezedText(p, r.width(), d->info.name()));
    }

    p->setFont(d->view->itemFontCom());

    if (settings->getIconShowComments())
    {
        QString comments = d->info.comment();

        r = d->view->itemCommentsRect();
        p->drawText(r, Qt::AlignCenter, squeezedText(p, r.width(), comments));
    }

    p->setFont(d->view->itemFontXtra());

    if (settings->getIconShowDate())
    {
        QDateTime date(d->info.dateTime());

        r = d->view->itemDateRect();
        p->setFont(d->view->itemFontXtra());
        QString str;
        dateToString(date, str);
        str = i18n("created : %1",str);
        p->drawText(r, Qt::AlignCenter, squeezedText(p, r.width(), str));
    }

    if (settings->getIconShowModDate())
    {
        QDateTime date(d->info.modDateTime());

        r = d->view->itemModDateRect();
        p->setFont(d->view->itemFontXtra());
        QString str;
        dateToString(date, str);
        str = i18n("modified : %1",str);
        p->drawText(r, Qt::AlignCenter, squeezedText(p, r.width(), str));
    }

    if (settings->getIconShowResolution())
    {
        QSize dims = d->info.dimensions();
        if (dims.isValid())
        {
            QString mpixels, resolution;
            mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
            resolution = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)",
                          dims.width(),dims.height(),mpixels);
            r = d->view->itemResolutionRect();
            p->drawText(r, Qt::AlignCenter, squeezedText(p, r.width(), resolution));
        }
    }

    if (settings->getIconShowSize())
    {
        r = d->view->itemSizeRect();
        p->drawText(r, Qt::AlignCenter,
                    squeezedText(p, r.width(), KIO::convertSize(d->info.fileSize())));
    }

    p->setFont(d->view->itemFontCom());
    p->setPen(isSelected() ? te->textSpecialSelColor() : te->textSpecialRegColor());

    if (settings->getIconShowTags())
    {
        QString tags = AlbumManager::instance()->tagPaths(d->info.tagIds(), false).join(", ");

        r = d->view->itemTagRect();
        p->drawText(r, Qt::AlignCenter, squeezedText(p, r.width(), tags));
    }

    if (this == d->view->currentItem())
    {
        r = d->view->itemRect();
        p->setPen(QPen(isSelected() ? te->textSelColor() : te->textRegColor(), 0, Qt::DotLine));
        if (isHighlighted())
        {
            p->setPen(QPen(te->textSelColor(), 3, Qt::SolidLine));
        }
        p->drawRect(1, 1, r.width()-2, r.height()-2);
    }

    if (isHighlighted())
    {
        paintToggleSelectButton(p);

        r = d->view->itemRect();
        p->setPen(QPen(d->view->palette().color(QPalette::Highlight), 3, Qt::SolidLine));
        p->drawRect(1, 1, r.width()-3, r.height()-3);
    }
}

}  // namespace Digikam
