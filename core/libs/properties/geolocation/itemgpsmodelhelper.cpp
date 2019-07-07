/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-15
 * Description : Model for central Map view
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

#include "itemgpsmodelhelper.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemGPSModelHelper::Private
{
public:

    explicit Private()
    {
        itemModel           = nullptr;
        itemSelectionModel  = nullptr;
        thumbnailLoadThread = nullptr;
    }

    QStandardItemModel*  itemModel;
    QItemSelectionModel* itemSelectionModel;
    ThumbnailLoadThread* thumbnailLoadThread;
};

ItemGPSModelHelper::ItemGPSModelHelper(QStandardItemModel* const itemModel, QObject* const parent)
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

ItemGPSModelHelper::~ItemGPSModelHelper()
{
    delete d;
}

QAbstractItemModel* ItemGPSModelHelper::model() const
{
    return d->itemModel;
}

QItemSelectionModel* ItemGPSModelHelper::selectionModel() const
{
    return d->itemSelectionModel;
}

bool ItemGPSModelHelper::itemCoordinates(const QModelIndex& index,
                                         GeoCoordinates* const coordinates) const
{
    const GPSItemInfo currentGPSItemInfo = index.data(RoleGPSItemInfo).value<GPSItemInfo>();
    *coordinates                         = currentGPSItemInfo.coordinates;

    if (currentGPSItemInfo.coordinates.hasCoordinates())
    {
        return true;
    }
    else
    {
        return false;
    }
}

QPixmap ItemGPSModelHelper::pixmapFromRepresentativeIndex(const QPersistentModelIndex& index,
                                                          const QSize& size)
{
    if (!index.isValid())
    {
        return QPixmap();
    }

    const QModelIndex currentIndex(index);
    const GPSItemInfo currentGPSItemInfo = currentIndex.data(RoleGPSItemInfo).value<GPSItemInfo>();

    QPixmap thumbnail;
    ThumbnailIdentifier thumbId;
    thumbId.filePath = currentGPSItemInfo.url.toLocalFile();
    thumbId.id       = currentGPSItemInfo.id;

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

QPersistentModelIndex ItemGPSModelHelper::bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list,
                                                                          const int sortKey)
{
    QModelIndex bestIndex         = list.first();
    GPSItemInfo bestGPSItemInfo = bestIndex.data(RoleGPSItemInfo).value<GPSItemInfo>();

    for (int i = 1 ; i < list.count() ; ++i)
    {
        const QModelIndex currentIndex(list.at(i));
        const GPSItemInfo currentGPSItemInfo = currentIndex.data(RoleGPSItemInfo).value<GPSItemInfo>();
        const bool currentFitsBetter         = GPSItemInfoSorter::fitsBetter(bestGPSItemInfo,
                                                                             SelectedNone,
                                                                             currentGPSItemInfo,
                                                                             SelectedNone,
                                                                             SelectedNone,
                                                                             GPSItemInfoSorter::SortOptions(sortKey));

        if (currentFitsBetter)
        {
            bestGPSItemInfo = currentGPSItemInfo;
            bestIndex        = currentIndex;
        }
    }

    return QPersistentModelIndex(bestIndex);
}

void ItemGPSModelHelper::slotThumbnailLoaded(const LoadingDescription& loadingDescription,
                                             const QPixmap& thumb)
{
    for (int i = 0 ; i < d->itemModel->rowCount() ; ++i)
    {
        const QStandardItem* const item      = static_cast<QStandardItem*>(d->itemModel->item(i));
        const GPSItemInfo currentGPSItemInfo = item->data(RoleGPSItemInfo).value<GPSItemInfo>();

        if (currentGPSItemInfo.url.toLocalFile() == loadingDescription.filePath)
        {
            const QPersistentModelIndex goodIndex(d->itemModel->index(i, 0));

            emit signalThumbnailAvailableForIndex(goodIndex, thumb);
        }
    }
}

} // namespace Digikam
