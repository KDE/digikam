/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-20
 * Description : Qt item view for images - category drawer
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imagecategorydrawer.h"

// Qt includes

#include <QPainter>

// KDE includes

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "imagecategorizedview.h"
#include "imagedelegate.h"
#include "imagefiltermodel.h"
#include "themeengine.h"

namespace Digikam
{

class ImageCategoryDrawerPriv
{
public:

    ImageCategoryDrawerPriv()
    {
        lowerSpacing = 0;
        view         = 0;
    }

    QFont                 font;
    QRect                 rect;
    QPixmap               pixmap;
    int                   lowerSpacing;
    ImageCategorizedView  *view;
};

ImageCategoryDrawer::ImageCategoryDrawer(ImageCategorizedView *parent)
                   : d(new ImageCategoryDrawerPriv)
{
    d->view = parent;
}

ImageCategoryDrawer::~ImageCategoryDrawer()
{
    delete d;
}

int ImageCategoryDrawer::categoryHeight(const QModelIndex &/*index*/, const QStyleOption &/*option*/) const
{
    return d->rect.height() + d->lowerSpacing;
}

int ImageCategoryDrawer::maximumHeight() const
{
    return d->rect.height() + d->lowerSpacing;
}

void ImageCategoryDrawer::setLowerSpacing(int spacing)
{
    d->lowerSpacing = spacing;
}

void ImageCategoryDrawer::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    d->font = option.font;
    if (option.rect.width() != d->rect.width())
        updateRectsAndPixmaps(option.rect.width());
}

void ImageCategoryDrawer::invalidatePaintingCache()
{
    if (d->rect.isNull())
        return;
    updateRectsAndPixmaps(d->rect.width());
}

void ImageCategoryDrawer::drawCategory(const QModelIndex& index, int /*sortRole*/,
                                       const QStyleOption& option, QPainter *p) const
{
    if (option.rect.width() != d->rect.width())
        const_cast<ImageCategoryDrawer*>(this)->updateRectsAndPixmaps(option.rect.width());

    p->save();

    p->translate(option.rect.topLeft());

    ImageFilterModel::CategorizationMode mode =
        (ImageFilterModel::CategorizationMode)index.data(ImageFilterModel::CategorizationModeRole).toInt();

    p->drawPixmap(0, 0, d->pixmap);

    QFont fontBold(d->font);
    QFont fontNormal(d->font);
    fontBold.setBold(true);
    int fnSize = fontBold.pointSize();
    bool usePointSize;
    if (fnSize > 0)
    {
        fontBold.setPointSize(fnSize+2);
        usePointSize = true;
    }
    else
    {
        fnSize = fontBold.pixelSize();
        fontBold.setPixelSize(fnSize+2);
        usePointSize = false;
    }

    QString header;
    QString subLine;

    if (mode == ImageFilterModel::CategoryByAlbum)
        textForAlbum(index, &header, &subLine);
    //TODO: other modes

    p->setPen(ThemeEngine::instance()->textSelColor());
    p->setFont(fontBold);

    QRect tr;
    p->drawText(5, 5, d->rect.width(), d->rect.height(),
            Qt::AlignLeft | Qt::AlignTop,
            header, &tr);

    int y = tr.height() + 2;

    p->setFont(fontNormal);

    p->drawText(5, y, d->rect.width(), d->rect.height() - y,
                Qt::AlignLeft | Qt::AlignVCenter, subLine);

    p->restore();
}

void ImageCategoryDrawer::textForAlbum(const QModelIndex& index, QString *header, QString *subLine) const
{
    int albumId   = index.data(ImageFilterModel::CategoryAlbumIdRole).toInt();
    PAlbum* album = AlbumManager::instance()->findPAlbum(albumId);

    if (album)
    {
        int count  = d->view->categoryRange(index).height();
        QDate date = album->date();

        KLocale tmpLocale(*KGlobal::locale());

        tmpLocale.setDateFormat("%d"); // day of month with two digits
        QString day = tmpLocale.formatDate(date);

        tmpLocale.setDateFormat("%b"); // short form of the month
        QString month = tmpLocale.formatDate(date);

        tmpLocale.setDateFormat("%Y"); // long form of the year
        QString year = tmpLocale.formatDate(date);

        *subLine = i18ncp("%1: day of month with two digits, %2: short month name, %3: year",
                          "Album Date: %2 %3 %4 - 1 Item", "Album Date: %2 %3 %4 - %1 Items",
                          count, day, month, year);

        if (!album->caption().isEmpty())
        {
            QString caption = album->caption();
            *subLine += " - " + caption.replace('\n', ' ');
        }

        *header = album->prettyUrl().left(-1);
    }
}

void ImageCategoryDrawer::updateRectsAndPixmaps(int width)
{
    d->rect = QRect(0, 0, 0, 0);

    // Title --------------------------------------------------------

    QFont fn(d->font);
    int fnSize = fn.pointSize();
    bool usePointSize;
    if (fnSize > 0)
    {
        fn.setPointSize(fnSize+2);
        usePointSize = true;
    }
    else
    {
        fnSize = fn.pixelSize();
        fn.setPixelSize(fnSize+2);
        usePointSize = false;
    }

    fn.setBold(true);
    QFontMetrics fm(fn);
    QRect tr = fm.boundingRect(0, 0, width,
                               0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                               "XXX");
    d->rect.setHeight(tr.height());

    if (usePointSize)
        fn.setPointSize(d->font.pointSize());
    else
        fn.setPixelSize(d->font.pixelSize());

    fn.setBold(false);
    fm = QFontMetrics(fn);

    tr = fm.boundingRect(0, 0, width,
                         0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                         "XXX");

    d->rect.setHeight(d->rect.height() + tr.height() + 10);
    d->rect.setWidth(width);

    d->pixmap = ThemeEngine::instance()->bannerPixmap(d->rect.width(), d->rect.height());
}

} // namespace Digikam
