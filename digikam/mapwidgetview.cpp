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
#include <kconfig.h>

// libkmap includes

#include <libkmap/kmap_widget.h>
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

class MapWidgetView::MapWidgetViewPriv
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
        mapWidget          = 0;
        vbox               = 0;
        imageModel         = 0;
        selectionModel     = 0;
        mapViewModelHelper = 0;
    }

    KVBox*               vbox;
    KMap::KMapWidget*    mapWidget;
    ImageAlbumModel*     imageModel;
    QItemSelectionModel* selectionModel;
    MapViewModelHelper*  mapViewModelHelper;
    bool                 activeState;
};

/**
 * @brief Constructor
 * @param selectionModel digiKam's selection model
 * @param imageFilterModel digikam's filter model
 * @param parent Parent object
 */
MapWidgetView::MapWidgetView(QItemSelectionModel* selectionModel,ImageFilterModel* imageFilterModel, QWidget* parent)
    : QWidget(parent), StateSavingObject(this), d(new MapWidgetViewPriv)
{
    d->imageModel           = qobject_cast<ImageAlbumModel*>(imageFilterModel->sourceModel());
    d->selectionModel       = selectionModel;
    d->mapViewModelHelper   = new MapViewModelHelper(d->selectionModel, imageFilterModel, this);
    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);

    d->mapWidget = new KMap::KMapWidget(this);
    d->mapWidget->setAvailableMouseModes(KMap::MouseModePan|KMap::MouseModeZoom);
    d->mapWidget->setVisibleMouseModes(KMap::MouseModePan|KMap::MouseModeZoom);
    KMap::ItemMarkerTiler* const kmapMarkerModel = new KMap::ItemMarkerTiler(d->mapViewModelHelper, this);
    d->mapWidget->setGroupedModel(kmapMarkerModel);
    d->mapWidget->setBackend("marble");
    vBoxLayout->addWidget(d->mapWidget);
    vBoxLayout->addWidget(d->mapWidget->getControlWidget());
}

/**
 * @brief Destructor
 */
MapWidgetView::~MapWidgetView()
{
    delete d;
}

void MapWidgetView::setConfigGroup(KConfigGroup group)
{
    StateSavingObject::setConfigGroup(group);
}

void MapWidgetView::doLoadState()
{
    KConfigGroup group = getConfigGroup();
    const KConfigGroup groupCentralMap = KConfigGroup(&group, "Central Map Widget");

    d->mapWidget->readSettingsFromGroup(&groupCentralMap);
}

void MapWidgetView::doSaveState()
{
    KConfigGroup group = getConfigGroup();
    KConfigGroup groupCentralMap = KConfigGroup(&group, "Central Map Widget");

    d->mapWidget->saveSettingsToGroup(&groupCentralMap);

    group.sync();
}

/**
 * @brief Switch that opens the current album.
 * @param album Current album.
 */
void MapWidgetView::openAlbum(Album* album)
{
    d->imageModel->openAlbum(album);
}

/**
 * @brief Set the map active/inactive.
 * @param state If true, the map is active.
 */
void MapWidgetView::setActive(const bool state)
{
    d->activeState = state;
    d->mapWidget->setActive(state);
}

/**
 * @return The map's active state
 */
bool MapWidgetView::getActiveState() const
{
    return d->activeState;
}

//-------------------------------------------------------------------------------------------------------------

class MapViewModelHelper::MapViewModelHelperPrivate
{
public:
    MapViewModelHelperPrivate()
    {
        model          = 0;
        selectionModel = 0;
    }

    ImageFilterModel*    model;
    QItemSelectionModel* selectionModel;
    ThumbnailLoadThread* thumbnailLoadThread;
};

MapViewModelHelper::MapViewModelHelper(QItemSelectionModel* const selection,
                                       ImageFilterModel* const filterModel, QObject* const parent)
    : KMap::ModelHelper(parent), d(new MapViewModelHelperPrivate())
{

    d->model               = filterModel;
    d->selectionModel      = selection;
    d->thumbnailLoadThread = new ThumbnailLoadThread();

    connect(d->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription&, const QPixmap&)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(const ImageChangeset&)),
            this, SLOT(slotImageChange(const ImageChangeset&)), Qt::QueuedConnection);

    // TODO: disable this connection and rely only on the database based one above
    //     connect(d->model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
    //             this, SLOT(signalModelChangedDrastically()));
}

/**
 * @brief Destructor
 */
MapViewModelHelper::~MapViewModelHelper()
{
    delete d;
}

/**
 * @return Returns digiKam's filter model.
 */
QAbstractItemModel* MapViewModelHelper::model() const
{
    return d->model;
}

/**
 * @return Returns digiKam's selection model.
 */
QItemSelectionModel* MapViewModelHelper::selectionModel() const
{
    return d->selectionModel;
}

/**
 * @brief Gets the coordinates of a marker found at current model index.
 * @param index Current model index.
 * @param coordinates Here will be returned the coordinates of the current marker.
 * @return True, if the marker has coordinates.
 */
bool MapViewModelHelper::itemCoordinates(const QModelIndex& index, KMap::GeoCoordinates* const coordinates) const
{
    ImageInfo info = d->model->imageInfo(index);

    if (info.isNull() || !info.hasCoordinates())
    {
        return false;
    }

    const KMap::GeoCoordinates gpsCoordinates(info.latitudeNumber(), info.longitudeNumber());

    *coordinates = gpsCoordinates;

    return true;
}

/**
 * @brief This function retrieves the thumbnail for an index.
 * @param index The marker's index.
 * @param size The size of the thumbnail.
 * @return If the thumbnail has been loaded in the ThumbnailLoadThread instance, it is returned. If not, a QPixmap is returned and ThumbnailLoadThread's signal named signalThumbnailLoaded is emited when the thumbnail becomes available.
 */
QPixmap MapViewModelHelper::pixmapFromRepresentativeIndex(const QPersistentModelIndex& index, const QSize& size)
{
    if (index == QPersistentModelIndex())
    {
        return QPixmap();
    }

    QPixmap thumbnail;
    ImageInfo info = d->model->imageInfo(index);

    if (!info.isNull())
    {
        QString path   = info.filePath();

        if (d->thumbnailLoadThread->find(path, thumbnail,qMax(size.width(), size.height())))
        {
            return thumbnail;
        }
        else
        {
            return QPixmap();
        }
    }

    return QPixmap();
}

/**
 * @brief This function finds the best representative marker from a group of markers. This is needed to display a thumbnail for a marker group.
 * @param indices A list containing markers.
 * @param sortKey Sets the criteria for selecting the representative thumbnail. When sortKey == 0 the most youngest thumbnail is chosen, when sortKey == 1 the most oldest thumbnail is chosen and when sortKey == 2 the thumbnail with the highest rating is chosen(if 2 thumbnails have the same rating, the youngest one is chosen).
 * @return Returns the index of the marker.
 */
QPersistentModelIndex MapViewModelHelper::bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list,
        const int sortKey)
{
    const bool oldestFirst = sortKey & 1;
    QList<QModelIndex> indexList;

    if (list.isEmpty())
    {
        return QPersistentModelIndex();
    }

    for (int i=0; i<list.count(); ++i)
    {
        QModelIndex newIndex(list[i]);
        indexList.append(newIndex);
    }

    QModelIndex bestIndex;
    ImageInfo bestImageInfo;
    QList<ImageInfo> imageInfoList =  d->model->imageInfos(indexList);

    for (int i=0; i<imageInfoList.count(); ++i)
    {
        ImageInfo currentInfo = imageInfoList.at(i);
        bool takeThisIndex    = (bestImageInfo.id() == -1);

        if (!takeThisIndex)
        {
            if (oldestFirst)
            {
                takeThisIndex = currentInfo < bestImageInfo;

                if (takeThisIndex)
                {
                    bestImageInfo = currentInfo;
                }
            }
            else
            {
                takeThisIndex = bestImageInfo < currentInfo;

                if (takeThisIndex)
                {
                    bestImageInfo = currentInfo;
                }
            }
        }
        else
        {
            bestImageInfo = currentInfo;
        }
    }

    bestIndex = d->model->indexForImageInfo(bestImageInfo);
    QPersistentModelIndex returnedIndex(bestIndex);
    return returnedIndex;
}

/**
 * @brief Because of a call to pixmapFromRepresentativeIndex, some thumbnails are not yet loaded at the time of requesting. When each thumbnail loads, this slot is called and emits a signal that announces the map that the thumbnail is available.
 */
void MapViewModelHelper::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb)
{
    if (thumb.isNull())
    {
        return;
    }

    QModelIndex currentIndex = d->model->indexForPath(loadingDescription.filePath);

    if (currentIndex.isValid())
    {
        QPersistentModelIndex goodIndex(currentIndex);
        emit(signalThumbnailAvailableForIndex(goodIndex, thumb));
    }
}

/**
 * This functions is called when one clicks on a thumbnail.
 * @param A list containing the marker indices belonging the group whose thumbnail has been clicked.
 */
void MapViewModelHelper::onIndicesClicked(const QList<QPersistentModelIndex>& clickedIndices)
{
    //TODO: there isn't another way to convert QPersistentModelIndex to QModelIndex?
    QList<QModelIndex> indexList;

    for (int i=0; i<clickedIndices.count(); ++i)
    {
        QModelIndex newIndex(clickedIndices[i]);
        indexList.append(newIndex);
    }

    QList<ImageInfo> imageInfoList = d->model->imageInfos(indexList);
    QList<qlonglong> imagesIdList;

    for (int i=0; i<imageInfoList.count(); ++i)
    {
        imagesIdList.append(imageInfoList[i].id());
    }

    emit signalFilteredImages(imagesIdList);
}

void MapViewModelHelper::slotImageChange(const ImageChangeset& changeset)
{
    kDebug() << "---------------------------------------------------------------";
    const DatabaseFields::Set changes = changeset.changes();
    const DatabaseFields::ImagePositions imagePositionChanges = changes;

    // TODO: more detailed check
    if (   ( changes & DatabaseFields::LatitudeNumber )
           || ( changes & DatabaseFields::LongitudeNumber )
           || ( changes & DatabaseFields::Altitude ) )
    {
        kDebug() << "changes!";

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

    kDebug() << "---------------------------------------------------------------";
}

} //namespace Digikam
