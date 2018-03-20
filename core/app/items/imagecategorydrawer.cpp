/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-20
 * Description : Qt item view for images - category drawer
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011      by Andi Clemens <andi dot clemens at gmail dot com>
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
#include <QApplication>
#include <QLocale>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "imagealbummodel.h"
#include "imagecategorizedview.h"
#include "imagedelegate.h"
#include "imagefiltermodel.h"
#include "imagemodel.h"
#include "imagescanner.h"
#include "searchfolderview.h"

namespace Digikam
{

class ImageCategoryDrawer::Private
{
public:

    Private()
    {
        lowerSpacing = 0;
        view         = 0;
    }

    QFont                 font;
    QRect                 rect;
    QPixmap               pixmap;
    int                   lowerSpacing;
    ImageCategorizedView* view;
};

ImageCategoryDrawer::ImageCategoryDrawer(ImageCategorizedView* const parent)
    : DCategoryDrawer(0), d(new Private)
{
    d->view = parent;
}

ImageCategoryDrawer::~ImageCategoryDrawer()
{
    delete d;
}

int ImageCategoryDrawer::categoryHeight(const QModelIndex& /*index*/, const QStyleOption& /*option*/) const
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
    {
        updateRectsAndPixmaps(option.rect.width());
    }
}

void ImageCategoryDrawer::invalidatePaintingCache()
{
    if (d->rect.isNull())
    {
        return;
    }

    updateRectsAndPixmaps(d->rect.width());
}

void ImageCategoryDrawer::drawCategory(const QModelIndex& index, int /*sortRole*/,
                                       const QStyleOption& option, QPainter* p) const
{
    if (option.rect.width() != d->rect.width())
    {
        const_cast<ImageCategoryDrawer*>(this)->updateRectsAndPixmaps(option.rect.width());
    }

    p->save();

    p->translate(option.rect.topLeft());

    ImageSortSettings::CategorizationMode mode = (ImageSortSettings::CategorizationMode)index.data(ImageFilterModel::CategorizationModeRole).toInt();

    p->drawPixmap(0, 0, d->pixmap);

    QFont fontBold(d->font);
    QFont fontNormal(d->font);
    fontBold.setBold(true);
    int fnSize = fontBold.pointSize();

    if (fnSize > 0)
    {
        fontBold.setPointSize(fnSize+2);
    }
    else
    {
        fnSize = fontBold.pixelSize();
        fontBold.setPixelSize(fnSize+2);
    }

    QString header;
    QString subLine;

    switch (mode)
    {
        case ImageSortSettings::NoCategories:
            break;
        case ImageSortSettings::OneCategory:
            viewHeaderText(index, &header, &subLine);
            break;
        case ImageSortSettings::CategoryByAlbum:
            textForAlbum(index, &header, &subLine);
            break;
        case ImageSortSettings::CategoryByFormat:
            textForFormat(index, &header, &subLine);
            break;
    }

    p->setPen(qApp->palette().color(QPalette::HighlightedText));
    p->setFont(fontBold);

    QRect tr;
    p->drawText(5, 5, d->rect.width(), d->rect.height(),
                Qt::AlignLeft | Qt::AlignTop,
                p->fontMetrics().elidedText(header, Qt::ElideRight, d->rect.width() - 10), &tr);

    int y = tr.height() + 2;

    p->setFont(fontNormal);

    p->drawText(5, y, d->rect.width(), d->rect.height() - y,
                Qt::AlignLeft | Qt::AlignVCenter,
                p->fontMetrics().elidedText(subLine, Qt::ElideRight, d->rect.width() - 10));

    p->restore();
}

void ImageCategoryDrawer::viewHeaderText(const QModelIndex& index, QString* header, QString* subLine) const
{
    ImageModel* const sourceModel = index.data(ImageModel::ImageModelPointerRole).value<ImageModel*>();

    if (!sourceModel)
    {
        return;
    }

    int count = d->view->categoryRange(index).height();

    // Add here further model subclasses in use with ImageCategoryDrawer.
    // Note you need a Q_OBJECT in the class's header for this to work.
    ImageAlbumModel* const albumModel = qobject_cast<ImageAlbumModel*>(sourceModel);

    if (albumModel)
    {
        QList<Album*> albums = albumModel->currentAlbums();
        Album* album         = 0;

        if (albums.isEmpty())
        {
            return;
        }

        album = albums.first();

        if (!album)
        {
            return;
        }

        switch (album->type())
        {
            case Album::PHYSICAL:
                textForPAlbum(static_cast<PAlbum*>(album), albumModel->isRecursingAlbums(), count, header, subLine);
                break;
            case Album::TAG:
                textForTAlbum(static_cast<TAlbum*>(album), albumModel->isRecursingTags(), count, header, subLine);
                break;
            case Album::DATE:
                textForDAlbum(static_cast<DAlbum*>(album), count, header, subLine);
                break;
            case Album::SEARCH:
                textForSAlbum(static_cast<SAlbum*>(album), count, header, subLine);
                break;
            case Album::FACE:
            default:
                break;
        }
    }
}

void ImageCategoryDrawer::textForAlbum(const QModelIndex& index, QString* header, QString* subLine) const
{
    int albumId         = index.data(ImageFilterModel::CategoryAlbumIdRole).toInt();
    PAlbum* const album = AlbumManager::instance()->findPAlbum(albumId);
    int count           = d->view->categoryRange(index).height();
    textForPAlbum(album, false, count, header, subLine);
}

void ImageCategoryDrawer::textForFormat(const QModelIndex& index, QString* header, QString* subLine) const
{
    QString format = index.data(ImageFilterModel::CategoryFormatRole).toString();
    format         = ImageScanner::formatToString(format);
    *header        = format;
    int count      = d->view->categoryRange(index).height();
    *subLine       = i18np("1 Item", "%1 Items", count);
}

void ImageCategoryDrawer::textForPAlbum(PAlbum* album, bool recursive, int count, QString* header, QString* subLine) const
{
    Q_UNUSED(recursive);

    if (!album)
    {
        return;
    }

    QDate date    = album->date();

    QLocale tmpLocale;

    // day of month with two digits
    QString day   = tmpLocale.toString(date, QLatin1String("dd"));

    // short form of the month
    QString month = tmpLocale.toString(date, QLatin1String("MMM"));

    // long form of the year
    QString year  = tmpLocale.toString(date, QLatin1String("yyyy"));

    *subLine      = i18ncp("%1: day of month with two digits, %2: short month name, %3: year",
                           "Album Date: %2 %3 %4 - 1 Item", "Album Date: %2 %3 %4 - %1 Items",
                           count, day, month, year);

    if (!album->caption().isEmpty())
    {
        QString caption = album->caption();
        *subLine       += QLatin1String(" - ") + caption.replace(QLatin1Char('\n'), QLatin1Char(' '));
    }

    *header = album->prettyUrl();
}

void ImageCategoryDrawer::textForTAlbum(TAlbum* talbum, bool recursive, int count, QString* header,
                                        QString* subLine) const
{
    *header = talbum->title();

    if (recursive && talbum->firstChild())
    {
        int n=0;

        for (AlbumIterator it(talbum); it.current(); ++it)
        {
            n++;
        }

        QString firstPart = i18ncp("%2: a tag title; %3: number of subtags",
                                   "%2 including 1 subtag", "%2 including %1 subtags",
                                   n, talbum->tagPath(false));

        *subLine = i18ncp("%2: the previous string (e.g. 'Foo including 7 subtags'); %1: number of items in tag",
                          "%2 - 1 Item", "%2 - %1 Items",
                          count, firstPart);
    }
    else
    {
        *subLine = i18np("%2 - 1 Item", "%2 - %1 Items", count, talbum->tagPath(false));
    }
}

void ImageCategoryDrawer::textForSAlbum(SAlbum* salbum, int count, QString* header, QString* subLine) const
{
    QString title = salbum->displayTitle();

    *header = title;

    if (salbum->isNormalSearch())
    {
        *subLine = i18np("Keyword Search - 1 Item", "Keyword Search - %1 Items", count);
    }
    else if (salbum->isAdvancedSearch())
    {
        *subLine = i18np("Advanced Search - 1 Item", "Advanced Search - %1 Items", count);
    }
    else
    {
        *subLine = i18np("1 Item", "%1 Items", count);
    }
}

void ImageCategoryDrawer::textForDAlbum(DAlbum* album, int count, QString* header, QString* subLine) const
{
    if (album->range() == DAlbum::Month)
    {
        *header = i18nc("Month String - Year String", "%1 %2",
                        QLocale().monthName(album->date().month(), QLocale::LongFormat),
                        album->date().year());
    }
    else
    {
        *header = QString::fromUtf8("%1").arg(album->date().year());
    }

    *subLine = i18np("1 Item", "%1 Items", count);
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
                               QLatin1String("XXX"));
    d->rect.setHeight(tr.height());

    if (usePointSize)
    {
        fn.setPointSize(d->font.pointSize());
    }
    else
    {
        fn.setPixelSize(d->font.pixelSize());
    }

    fn.setBold(false);
    fm = QFontMetrics(fn);
    tr = fm.boundingRect(0, 0, width,
                         0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                         QLatin1String("XXX"));

    d->rect.setHeight(d->rect.height() + tr.height() + 10);
    d->rect.setWidth(width);

    d->pixmap = QPixmap(d->rect.width(), d->rect.height());
    d->pixmap.fill(qApp->palette().color(QPalette::Highlight));
}

} // namespace Digikam
