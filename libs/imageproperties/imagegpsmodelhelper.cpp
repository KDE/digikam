/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-15
 * Description : central Map view
 *
 * Copyright (C) 2010      by Gabriel Voicu <ping dot gabi at gmail dot com>
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "imagegpsmodelhelper.h"

namespace Digikam
{

class ImageGPSModelHelper::Private
{
public:

    Private()
    {
        itemModel           = 0;
        itemSelectionModel  = 0;
        thumbnailLoadThread = 0;
    }

    QStandardItemModel*  itemModel;
    QItemSelectionModel* itemSelectionModel;
    ThumbnailLoadThread* thumbnailLoadThread;
};

ImageGPSModelHelper::ImageGPSModelHelper(QStandardItemModel* const itemModel, QObject* const parent)
    : GeoModelHelper(parent),
      d(new Private())
{

    d->itemModel           = itemModel;
    d->itemSelectionModel  = new QItemSelectionModel(d->itemModel);
    d->thumbnailLoadThread = new ThumbnailLoadThread(this);

    connect(d->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));

    connect(d->itemModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SIGNAL(signalModelChangedDrastically()));
}

ImageGPSModelHelper::~ImageGPSModelHelper()
{
    delete d;
}

QAbstractItemModel* ImageGPSModelHelper::model() const
{
    return d->itemModel;
}

QItemSelectionModel* ImageGPSModelHelper::selectionModel() const
{
    return d->itemSelectionModel;
}

bool ImageGPSModelHelper::itemCoordinates(const QModelIndex& index,
                                          GeoCoordinates* const coordinates) const
{
    const GPSImageInfo currentGPSImageInfo = index.data(RoleGPSImageInfo).value<GPSImageInfo>();
    *coordinates                           = currentGPSImageInfo.coordinates;

    if (currentGPSImageInfo.coordinates.hasCoordinates())
    {
        return true;
    }
    else
    {
        return false;
    }
}

QPixmap ImageGPSModelHelper::pixmapFromRepresentativeIndex(const QPersistentModelIndex& index,
                                                           const QSize& size)
{
    if (!index.isValid())
    {
        return QPixmap();
    }

    const QModelIndex currentIndex(index);
    const GPSImageInfo currentGPSImageInfo = currentIndex.data(RoleGPSImageInfo).value<GPSImageInfo>();

    QPixmap thumbnail;
    ThumbnailIdentifier thumbId;
    thumbId.filePath = currentGPSImageInfo.url.toLocalFile();
    thumbId.id       = currentGPSImageInfo.id;

    if (d->thumbnailLoadThread->find(thumbId, thumbnail, qMax(size.width(), size.height())))
    {
        // digikam returns thumbnails with a border around them,
        // but the geolocation interface expects them without a border
        return thumbnail.copy(1, 1, thumbnail.size().width()-2, thumbnail.size().height()-2);
    }
    else
    {
        return QPixmap();
    }
}

QPersistentModelIndex ImageGPSModelHelper::bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list,
                                                                           const int sortKey)
{
    QModelIndex bestIndex         = list.first();
    GPSImageInfo bestGPSImageInfo = bestIndex.data(RoleGPSImageInfo).value<GPSImageInfo>();

    for (int i = 1; i < list.count(); ++i)
    {
        const QModelIndex currentIndex(list.at(i));
        const GPSImageInfo currentGPSImageInfo = currentIndex.data(RoleGPSImageInfo).value<GPSImageInfo>();
        const bool currentFitsBetter           = GPSImageInfoSorter::fitsBetter(bestGPSImageInfo,
                                                                                SelectedNone,
                                                                                currentGPSImageInfo,
                                                                                SelectedNone,
                                                                                SelectedNone,
                                                                                GPSImageInfoSorter::SortOptions(sortKey));

        if (currentFitsBetter)
        {
            bestGPSImageInfo = currentGPSImageInfo;
            bestIndex        = currentIndex;
        }
    }

    return QPersistentModelIndex(bestIndex);
}

void ImageGPSModelHelper::slotThumbnailLoaded(const LoadingDescription& loadingDescription,
                                              const QPixmap& thumb)
{
    for (int i = 0; i < d->itemModel->rowCount(); ++i)
    {
        const QStandardItem* const item        = static_cast<QStandardItem*>(d->itemModel->item(i));
        const GPSImageInfo currentGPSImageInfo = item->data(RoleGPSImageInfo).value<GPSImageInfo>();

        if (currentGPSImageInfo.url.toLocalFile() == loadingDescription.filePath)
        {
            const QPersistentModelIndex goodIndex(d->itemModel->index(i,0));

            emit(signalThumbnailAvailableForIndex(goodIndex, thumb));
        }
    }
}

} // namespace Digikam
