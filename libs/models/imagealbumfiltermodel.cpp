/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-11
 * Description : Qt item model for database entries, using AlbumManager
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

#include "imagealbumfiltermodel.h"
#include "imagealbumfiltermodel.moc"

// KDE includes

#include <kstringhandler.h>

// Local includes

#include "imagefiltermodelpriv.h"
#include "album.h"
#include "albummanager.h"
#include "databaseaccess.h"
#include "databasechangesets.h"
#include "databasewatch.h"
#include "imagealbummodel.h"

namespace Digikam
{

class ImageAlbumFilterModelPrivate : public ImageFilterModelPrivate
{
public:

    QHash<int, QString> tagNamesHash;
    QHash<int, QString> albumNamesHash;

    bool albumChange(Album *album);
};

ImageAlbumFilterModel::ImageAlbumFilterModel(QObject *parent)
                     : ImageFilterModel(*new ImageAlbumFilterModelPrivate, parent)
{
}

ImageAlbumFilterModel::~ImageAlbumFilterModel()
{
}

void ImageAlbumFilterModel::setSourceImageModel(ImageAlbumModel* model)
{
    /*
    Q_D(ImageAlbumFilterModel)
    if (dynamic_cast<IoslaveImageModel*>(d->imageModel))
    {
        disconnect(d->imageModel, SIGNAL(listedAlbumChanged(Album*)),
                   this, SLOT(slotListedAlbumChanged(Album *)));
    }

    connect(model, SIGNAL(listedAlbumChanged(Album*)),
            this, SLOT(slotListedAlbumChanged(Album *)));
            */

    ImageFilterModel::setSourceImageModel(model);
}

/*
void ImageAlbumFilterModel::slotListedAlbumChanged(Album *album)
{
    Q_D(ImageAlbumFilterModel)
    d->currentAlbum = album;
}
*/

void ImageAlbumFilterModel::setImageFilterSettings(const ImageFilterSettings &s)
{
    Q_D(ImageAlbumFilterModel);
    ImageFilterSettings settings(s);
    settings.setAlbumNames(d->albumNamesHash);
    settings.setTagNames(d->tagNamesHash);
    ImageFilterModel::setImageFilterSettings(settings);
}

int ImageAlbumFilterModel::compareInfosCategories(const ImageInfo &left, const ImageInfo &right) const
{
    Q_D(const ImageAlbumFilterModel);
    switch (d->categorizationMode)
    {
        case NoCategories:
            return 0;
        case CategoryByAlbum:
        {
            PAlbum *leftAlbum = AlbumManager::instance()->findPAlbum(left.albumId());
            PAlbum *rightAlbum = AlbumManager::instance()->findPAlbum(right.albumId());
            if (!leftAlbum || !rightAlbum)
                return -1;
            if (d->sortOrder == SortByCreationDate || d->sortOrder == SortByModificationDate)
                return leftAlbum->date() < rightAlbum->date();

            return KStringHandler::naturalCompare(leftAlbum->albumPath(), rightAlbum->albumPath());
        }
        case CategoryByFormat:
            return KStringHandler::naturalCompare(left.format(), right.format());
        default:
            return 0;
    }
}

void ImageAlbumFilterModel::albumChange(Album *album)
{
    Q_D(ImageAlbumFilterModel);
    if (album->type() == Album::PHYSICAL)
    {
        d->albumNamesHash = AlbumManager::instance()->albumTitles();
        if (d->filter.isFilteringByText())
            setImageFilterSettings(d->filter);
    }
    else if (album->type() == Album::TAG)
    {
        d->tagNamesHash = AlbumManager::instance()->tagNames();
        if (d->filter.isFilteringByText() || d->filter.isFilteringByTags())
            setImageFilterSettings(d->filter);
    }
}

void ImageAlbumFilterModel::slotAlbumRenamed(Album *album)
{
    albumChange(album);
}

void ImageAlbumFilterModel::slotAlbumAdded(Album *album)
{
    albumChange(album);
}

void ImageAlbumFilterModel::slotAlbumAboutToBeDeleted(Album *album)
{
    albumChange(album);
}

} // namespace Digikam
