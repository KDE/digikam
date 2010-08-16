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

#include "mapwidgetview.moc"

//Qt includes

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QItemSelectionModel>
#include <QAbstractItemModel>
#include <QPersistentModelIndex>

//KDE includes

#include <KHBox>
#include <KVBox>

// libkmap includes

#include <libkmap/kmapwidget.h>
#include <libkmap/itemmarkertiler.h>

//local includes

#include "imageposition.h"
#include "imageinfo.h"
#include "imagemodel.h"
#include "databasewatch.h"
#include "databasefields.h"


namespace Digikam
{


class ImageAlbumModel;
class ImageFilterModel;

/**
 * @class MapWidgetView
 *
 * @brief Class containing Digikam's central map view.
 */

class MapWidgetViewPriv
{
public:

    MapWidgetViewPriv()
    :vbox(), 
     mapWidget(),
     imageModel(),
     selectionModel(), 
     mapViewModelHelper(),
     activeState(false) 
    {
        mapWidget = 0;
        vbox = 0;
        imageModel = 0;
        selectionModel = 0;
        mapViewModelHelper = 0;
    }

    KVBox                *vbox;
    KMapIface::KMapWidget *mapWidget;
    ImageAlbumModel      *imageModel;
    QItemSelectionModel  *selectionModel;
    MapViewModelHelper   *mapViewModelHelper;
    bool                 activeState;
};

MapWidgetView::MapWidgetView(QItemSelectionModel* selectionModel,ImageFilterModel* imageFilterModel, QWidget* parent)
             : QWidget(parent), d(new MapWidgetViewPriv)
{
    d->imageModel = qobject_cast<ImageAlbumModel*>(imageFilterModel->sourceModel());
    d->selectionModel = selectionModel;
    d->mapViewModelHelper = new MapViewModelHelper(d->imageModel, d->selectionModel, imageFilterModel, this);
    QVBoxLayout *vBoxLayout = new QVBoxLayout(this);
    
    d->mapWidget = new KMapIface::KMapWidget(this);
    d->mapWidget->setAvailableMouseModes(KMapIface::MouseModePan|KMapIface::MouseModeZoom);
    d->mapWidget->setVisibleMouseModes(KMapIface::MouseModePan|KMapIface::MouseModeZoom);
    KMapIface::ItemMarkerTiler* const kmapMarkerModel = new KMapIface::ItemMarkerTiler(d->mapViewModelHelper, this);
    d->mapWidget->setGroupedModel(kmapMarkerModel);
    d->mapWidget->setBackend("marble");
    vBoxLayout->addWidget(d->mapWidget);
    vBoxLayout->addWidget(d->mapWidget->getControlWidget());
}

MapWidgetView::~MapWidgetView()
{
    delete d;
}

void MapWidgetView::openAlbum(Album* album)
{
    d->imageModel->openAlbum(album);
}

void MapWidgetView::setActive(const bool state)
{
    d->activeState = state;
    d->mapWidget->setActive(state);
}

bool MapWidgetView::getActiveState() const
{
    return d->activeState;
}

//-------------------------------------------------------------------------------------------------------------

class MapViewModelHelperPrivate
{

public:
    MapViewModelHelperPrivate()
    {
        model          = 0;
        selectionModel = 0;
    }

    ImageFilterModel*        model;
    QItemSelectionModel*    selectionModel;
    ThumbnailLoadThread*    thumbnailLoadThread;

};


MapViewModelHelper::MapViewModelHelper(ImageAlbumModel* const model, QItemSelectionModel* const selection, ImageFilterModel* const filterModel, QObject* const parent)
: KMapIface::WMWModelHelper(parent), d(new MapViewModelHelperPrivate())
{
    Q_UNUSED(model)

    d->model                = filterModel;
    d->selectionModel       = selection;
    d->thumbnailLoadThread  = new ThumbnailLoadThread();
    
    connect(d->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription&, const QPixmap&)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(const ImageChangeset &)),
            this, SLOT(slotImageChange(const ImageChangeset &)));

    // TODO: disable this connection and rely only on the database based one above
//     connect(d->model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
//             this, SLOT(signalModelChangedDrastically()));
}

MapViewModelHelper::~MapViewModelHelper()
{
    delete d;
}

QAbstractItemModel* MapViewModelHelper::model() const
{
    return d->model;
}

QItemSelectionModel* MapViewModelHelper::selectionModel() const
{
    return d->selectionModel;
}

bool MapViewModelHelper::itemCoordinates(const QModelIndex& index, KMapIface::WMWGeoCoordinate * const coordinates) const
{
    ImageInfo info          = d->model->imageInfo(index);

    if(info.isNull() || !info.hasCoordinates())
        return false;

    const KMapIface::WMWGeoCoordinate gpsCoordinates(info.latitudeNumber(), info.longitudeNumber());
    *coordinates = gpsCoordinates;

    return true;
}

QPixmap MapViewModelHelper::pixmapFromRepresentativeIndex(const QPersistentModelIndex& index, const QSize& size)
{
    if(index == QPersistentModelIndex())
        return QPixmap();

    QPixmap thumbnail;
    ImageInfo info = d->model->imageInfo(index);
    if(!info.isNull())
    { 
        QString path   = info.filePath();

        if(d->thumbnailLoadThread->find(path, thumbnail,qMax(size.width(), size.height())))
            return thumbnail;
        else
            return QPixmap(); 
    }
    return QPixmap();
}

QPersistentModelIndex MapViewModelHelper::bestRepresentativeIndexFromList( const QList<QPersistentModelIndex>& list, const int sortKey)
{
    const bool oldestFirst = sortKey & 1;
    QList<QModelIndex> indexList;

    if(list.isEmpty())
        return QPersistentModelIndex();

    for(int i=0; i<list.count(); ++i)
    {
        QModelIndex newIndex(list[i]);
        indexList.append(newIndex);
    }

    QModelIndex bestIndex;
    ImageInfo bestImageInfo;
    QList<ImageInfo> imageInfoList =  d->model->imageInfos(indexList);    

    for(int i=0; i<imageInfoList.count(); ++i)
    {
        ImageInfo currentInfo = imageInfoList.at(i);
        bool takeThisIndex    = (bestImageInfo.id() == -1);
        
        if(!takeThisIndex)
        {
            if(oldestFirst)
            {
                takeThisIndex = currentInfo < bestImageInfo;
                if(takeThisIndex)
                    bestImageInfo = currentInfo;
                
            }
            else
            {
                takeThisIndex = bestImageInfo < currentInfo;
                if(takeThisIndex)
                    bestImageInfo = currentInfo;
            }
        }
        else
        {
            bestImageInfo = currentInfo;
//            bestIndex = d->model->indexForImageInfo(bestImageInfo);
        }
    }

    bestIndex = d->model->indexForImageInfo(bestImageInfo);
    QPersistentModelIndex returnedIndex(bestIndex);
    return returnedIndex;
}


void MapViewModelHelper::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb)
{
    if(thumb.isNull())
        return;

    QModelIndex currentIndex = d->model->indexForPath(loadingDescription.filePath);

    if(currentIndex.isValid())
    {
        QPersistentModelIndex goodIndex(currentIndex);
        emit(signalThumbnailAvailableForIndex(goodIndex, thumb));
    }
} 

void MapViewModelHelper::onIndicesClicked(const QList<QPersistentModelIndex>& clickedIndices)
{
    //TODO: there isn't another way to convert QPersistentModelIndex to QModelIndex?
    QList<QModelIndex> indexList;

    for(int i=0; i<clickedIndices.count(); ++i)
    {
        QModelIndex newIndex(clickedIndices[i]);
        indexList.append(newIndex);
    }

    QList<ImageInfo> imageInfoList = d->model->imageInfos(indexList);
    //KUrl::List imagesUrlList;
    QList<qlonglong> imagesIdList;

    for(int i=0; i<imageInfoList.count(); ++i)
    {
        //imagesUrlList.append(imageInfoList[i].fileUrl());    
        imagesIdList.append(imageInfoList[i].id());
    }

   // emit signalFilteredImages(imagesUrlList); 
    emit signalFilteredImages(imagesIdList);
}

void MapViewModelHelper::slotImageChange(const ImageChangeset& changeset)
{
    kDebug()<<"---------------------------------------------------------------";
    const DatabaseFields::Set changes = changeset.changes();
    const DatabaseFields::ImagePositions imagePositionChanges = changes;
    // TODO: more detailed check
    if (   ( changes & DatabaseFields::LatitudeNumber )
        || ( changes & DatabaseFields::LongitudeNumber )
        || ( changes & DatabaseFields::Altitude ) )
    {
        kDebug()<<"changes!";
        foreach (const qlonglong& id, changeset.ids())
        {
            QModelIndex index = d->model->indexForImageId(id);
            kDebug()<<id<<index;
            if (index.isValid())
            {
                kDebug()<<index;
                emit(signalModelChangedDrastically());
                break;
            }
        }
    }
    kDebug()<<"---------------------------------------------------------------";
}

} //namespace Digikam
