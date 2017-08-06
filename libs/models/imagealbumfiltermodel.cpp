/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-11
 * Description : Qt item model for database entries, using AlbumManager
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "imagealbumfiltermodel.h"

// Qt includes

#include <QTimer>

// Local includes

#include "imagefiltermodelpriv.h"
#include "album.h"
#include "albummanager.h"
#include "coredbaccess.h"
#include "coredbchangesets.h"
#include "coredbwatch.h"
#include "imagealbummodel.h"

namespace Digikam
{

class ImageAlbumFilterModelPrivate : public ImageFilterModel::ImageFilterModelPrivate
{
public:

    ImageAlbumFilterModelPrivate()
    {
        delayedAlbumNamesTimer = 0;
        delayedTagNamesTimer   = 0;
    }

    QHash<int, QString> tagNamesHash;
    QHash<int, QString> albumNamesHash;
    QTimer*             delayedAlbumNamesTimer;
    QTimer*             delayedTagNamesTimer;
};

ImageAlbumFilterModel::ImageAlbumFilterModel(QObject* const parent)
    : ImageFilterModel(*new ImageAlbumFilterModelPrivate, parent)
{
    Q_D(ImageAlbumFilterModel);

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAboutToBeDeleted(Album*)),
            this, SLOT(slotAlbumAboutToBeDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    d->delayedAlbumNamesTimer = new QTimer(this);
    d->delayedAlbumNamesTimer->setInterval(250);
    d->delayedAlbumNamesTimer->setSingleShot(true);

    d->delayedTagNamesTimer = new QTimer(this);
    d->delayedTagNamesTimer->setInterval(250);
    d->delayedTagNamesTimer->setSingleShot(true);

    connect(d->delayedAlbumNamesTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedAlbumNamesTimer()));

    connect(d->delayedTagNamesTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedTagNamesTimer()));

    foreach(Album* const a, AlbumManager::instance()->allPAlbums())
    {
        albumChange(a);
    }

    foreach(Album* const a, AlbumManager::instance()->allTAlbums())
    {
        albumChange(a);
    }
}

ImageAlbumFilterModel::~ImageAlbumFilterModel()
{
}

void ImageAlbumFilterModel::setSourceImageModel(ImageAlbumModel* model)
{
    ImageFilterModel::setSourceImageModel(model);
}

ImageAlbumModel* ImageAlbumFilterModel::sourceModel() const
{
    Q_D(const ImageAlbumFilterModel);

    return static_cast<ImageAlbumModel*>(d->imageModel);
}

void ImageAlbumFilterModel::prepareThumbnails(const QList<QModelIndex>& indexesToPrepare)
{
    sourceModel()->prepareThumbnails(mapListToSource(indexesToPrepare));
}

void ImageAlbumFilterModel::setImageFilterSettings(const ImageFilterSettings& s)
{
    Q_D(ImageAlbumFilterModel);

    ImageFilterSettings settings(s);
    settings.setAlbumNames(d->albumNamesHash);
    settings.setTagNames(d->tagNamesHash);
    ImageFilterModel::setImageFilterSettings(settings);
}

int ImageAlbumFilterModel::compareInfosCategories(const ImageInfo& left, const ImageInfo& right) const
{
    Q_D(const ImageAlbumFilterModel);

    switch (d->sorter.categorizationMode)
    {
        case ImageSortSettings::CategoryByAlbum:
        {
            int leftAlbumId          = left.albumId();
            int rightAlbumId         = right.albumId();
            PAlbum* const leftAlbum  = AlbumManager::instance()->findPAlbum(leftAlbumId);
            PAlbum* const rightAlbum = AlbumManager::instance()->findPAlbum(rightAlbumId);

            if (!leftAlbum || !rightAlbum)
            {
                return -1;
            }

            if (leftAlbum == rightAlbum)
            {
                return 0;
            }

            if (d->sorter.sortRole == ImageSortSettings::SortByCreationDate ||
                d->sorter.sortRole == ImageSortSettings::SortByModificationDate)
            {
                // Here we want to sort the _categories_ by _album_ date if images are sorted by date
                // We must still make sure that categorization is unique!
                QDate leftDate  = leftAlbum->date();
                QDate rightDate = rightAlbum->date();

                if (leftDate != rightDate)
                {

                    return ImageSortSettings::compareByOrder(leftDate > rightDate ? 1 : -1,
                                                             d->sorter.currentCategorizationSortOrder);
                }
            }

            return ImageSortSettings::naturalCompare(leftAlbum->albumPath(), rightAlbum->albumPath(),
                                                     d->sorter.currentCategorizationSortOrder,
                                                     d->sorter.categorizationCaseSensitivity,
                                                     d->sorter.strTypeNatural, true);
        }
        default:
        {
            return ImageFilterModel::compareInfosCategories(left, right);
        }
    }
}

void ImageAlbumFilterModel::albumChange(Album* album)
{
    Q_D(const ImageAlbumFilterModel);

    if (album->type() == Album::PHYSICAL)
    {
        d->delayedAlbumNamesTimer->start();
    }
    else if (album->type() == Album::TAG)
    {
        d->delayedTagNamesTimer->start();
    }
}

void ImageAlbumFilterModel::slotAlbumRenamed(Album* album)
{
    albumChange(album);
}

void ImageAlbumFilterModel::slotAlbumAdded(Album* album)
{
    albumChange(album);
}

void ImageAlbumFilterModel::slotAlbumAboutToBeDeleted(Album* album)
{
    albumChange(album);
}

void ImageAlbumFilterModel::slotAlbumsCleared()
{
    Q_D(ImageAlbumFilterModel);
    d->albumNamesHash.clear();
    d->tagNamesHash.clear();
}

void ImageAlbumFilterModel::slotDelayedAlbumNamesTimer()
{
    Q_D(ImageAlbumFilterModel);

    d->albumNamesHash = AlbumManager::instance()->albumTitles();

    if (d->filter.isFilteringByText())
    {
        setImageFilterSettings(d->filter);
    }
}

void ImageAlbumFilterModel::slotDelayedTagNamesTimer()
{
    Q_D(ImageAlbumFilterModel);

    d->tagNamesHash = AlbumManager::instance()->tagNames();

    if (d->filter.isFilteringByText() || d->filter.isFilteringByTags())
    {
        setImageFilterSettings(d->filter);
    }
}

} // namespace Digikam
