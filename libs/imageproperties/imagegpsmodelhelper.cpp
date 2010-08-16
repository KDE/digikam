/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-15
 * Description : central Map view
 *
 * Copyright (C) 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
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

#include "imagegpsmodelhelper.moc"

// local includes

#include "imagegpsitem.h"

namespace Digikam
{

class ImageGPSModelHelperPriv
{
public:

    ImageGPSModelHelperPriv()
    {
        itemModel           = 0;
        itemSelectionModel  = 0;
        thumbnailLoadThread = 0;
    }

    QStandardItemModel  *itemModel;
    QItemSelectionModel *itemSelectionModel;
    ThumbnailLoadThread *thumbnailLoadThread;
};

ImageGPSModelHelper::ImageGPSModelHelper(QStandardItemModel* const itemModel, QObject* const parent)
                    : KMapIface::WMWModelHelper(parent), d(new ImageGPSModelHelperPriv())
{

    d->itemModel           = itemModel;
    d->itemSelectionModel  = new QItemSelectionModel(d->itemModel);
    d->thumbnailLoadThread = new ThumbnailLoadThread();

    connect(d->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription&, const QPixmap&)));

    connect(d->itemModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(signalModelChangedDrastically()));
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

bool ImageGPSModelHelper::itemCoordinates(const QModelIndex& index, KMapIface::WMWGeoCoordinate* const coordinates) const
{
    ImageGPSItem *item =static_cast<ImageGPSItem*>(d->itemModel->itemFromIndex(index));
    QVariant var = item->data(RoleGPSInfo);
    GPSInfo currentGPSInfo = var.value<GPSInfo>();

    KMapIface::WMWGeoCoordinate currentCoordinates(currentGPSInfo.latitude, currentGPSInfo.longitude);
    *coordinates = currentCoordinates;

    if(currentCoordinates.hasCoordinates())
        return true;
    else
        return false;
}

QPixmap ImageGPSModelHelper::pixmapFromRepresentativeIndex(const QPersistentModelIndex& index, const QSize& size)
{
    if(index == QPersistentModelIndex())
        return QPixmap();

    QPixmap     thumbnail;
    QModelIndex currentIndex(index);

    ImageGPSItem *item     =static_cast<ImageGPSItem*>(d->itemModel->itemFromIndex(currentIndex));
    QVariant var           = item->data(RoleGPSInfo);
    GPSInfo currentGPSInfo = var.value<GPSInfo>();
    
    if(d->thumbnailLoadThread->find(currentGPSInfo.url.path(), thumbnail, qMax(size.width(), size.height())))
        return thumbnail;
    else 
        return QPixmap();
}

QPersistentModelIndex ImageGPSModelHelper::bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list, const int sortKey)
{
    const bool lowestRatedFirst = sortKey & 1;
    QModelIndex bestIndex;
    GPSInfo     bestGPSInfo;

    for(int i=0; i<list.count(); ++i)
    {
        QModelIndex currentIndex(list.at(i));

        ImageGPSItem *item     = static_cast<ImageGPSItem*>(d->itemModel->itemFromIndex(currentIndex));
        QVariant var           = item->data(RoleGPSInfo);
        GPSInfo currentGPSInfo = var.value<GPSInfo>();
        bool takeThisIndex     = (bestGPSInfo.id == -1); 
        
        if(!takeThisIndex)
        {
            if(lowestRatedFirst)
            {
                takeThisIndex = currentGPSInfo.rating < bestGPSInfo.rating;
                if(takeThisIndex)
                {
                   bestGPSInfo = currentGPSInfo;
                   bestIndex   = currentIndex;
                }
            }
            else
            {
                takeThisIndex = bestGPSInfo.rating < currentGPSInfo.rating;
                if(takeThisIndex)
                {
                    bestGPSInfo = currentGPSInfo;                
                    bestIndex   = currentIndex;
                }
            }
        }
        else
        {
            bestGPSInfo = currentGPSInfo;
            bestIndex   = currentIndex;
        }
    }

    QPersistentModelIndex returnedIndex(bestIndex);
    return returnedIndex;
}

void ImageGPSModelHelper::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb)
{

    for(int i=0; i<d->itemModel->rowCount(); ++i)
    {
        ImageGPSItem *item = static_cast<ImageGPSItem*>(d->itemModel->item(i));
        QVariant var           = item->data(RoleGPSInfo);
        GPSInfo currentGPSInfo = var.value<GPSInfo>();
       
        if(currentGPSInfo.url.path() == loadingDescription.filePath)
        {
            QPersistentModelIndex goodIndex(d->itemModel->index(i,0));
            emit(signalThumbnailAvailableForIndex(goodIndex, thumb));
        } 
    }
}

}
