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

#include <khbox.h>
#include <kvbox.h>
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
#include "digikam2kmap_database.h"

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
        :vbox(0),
         mapWidget(0),
         imageFilterModel(0),
         imageModel(0),
         selectionModel(0),
         mapViewModelHelper(0),
         gpsImageInfoSorter(0)
    {
    }

    KVBox*               vbox;
    KMap::KMapWidget*    mapWidget;
    ImageFilterModel*    imageFilterModel;
    ImageAlbumModel*     imageModel;
    QItemSelectionModel* selectionModel;
    MapViewModelHelper*  mapViewModelHelper;
    GPSImageInfoSorter*  gpsImageInfoSorter;
};

/**
 * @brief Constructor
 * @param selectionModel digiKam's selection model
 * @param imageFilterModel digikam's filter model
 * @param parent Parent object
 */
MapWidgetView::MapWidgetView(QItemSelectionModel* const selectionModel,
                             ImageFilterModel* const imageFilterModel, QWidget* const parent)
    : QWidget(parent), StateSavingObject(this), d(new MapWidgetViewPriv())
{
    d->imageFilterModel     = imageFilterModel;
    d->imageModel           = qobject_cast<ImageAlbumModel*>(imageFilterModel->sourceModel());
    d->selectionModel       = selectionModel;
    d->mapViewModelHelper   = new MapViewModelHelper(d->selectionModel, imageFilterModel, this);
    QVBoxLayout* const vBoxLayout = new QVBoxLayout(this);

    d->mapWidget = new KMap::KMapWidget(this);
    d->mapWidget->setAvailableMouseModes(KMap::MouseModePan|KMap::MouseModeZoomIntoGroup|KMap::MouseModeSelectThumbnail);
    d->mapWidget->setVisibleMouseModes(KMap::MouseModePan|KMap::MouseModeZoomIntoGroup|KMap::MouseModeSelectThumbnail);
    KMap::ItemMarkerTiler* const kmapMarkerModel = new KMap::ItemMarkerTiler(d->mapViewModelHelper, this);
    d->mapWidget->setGroupedModel(kmapMarkerModel);
    d->mapWidget->setBackend("marble");
    d->gpsImageInfoSorter = new GPSImageInfoSorter(this);
    d->gpsImageInfoSorter->addToKMapWidget(d->mapWidget);
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

void MapWidgetView::doLoadState()
{
    KConfigGroup group = getConfigGroup();

    d->gpsImageInfoSorter->setSortOptions(
        GPSImageInfoSorter::SortOptions(group.readEntry("Sort Order", int(d->gpsImageInfoSorter->getSortOptions())))
    );

    const KConfigGroup groupCentralMap = KConfigGroup(&group, "Central Map Widget");
    d->mapWidget->readSettingsFromGroup(&groupCentralMap);
}

void MapWidgetView::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    group.writeEntry("Sort Order", int(d->gpsImageInfoSorter->getSortOptions()));

    KConfigGroup groupCentralMap = KConfigGroup(&group, "Central Map Widget");
    d->mapWidget->saveSettingsToGroup(&groupCentralMap);

    group.sync();
}

/**
 * @brief Switch that opens the current album.
 * @param album Current album.
 */
void MapWidgetView::openAlbum(Album* const album)
{
    d->imageModel->openAlbum(album);
}

/**
 * @brief Set the map active/inactive.
 * @param state If true, the map is active.
 */
void MapWidgetView::setActive(const bool state)
{
    d->mapWidget->setActive(state);
}

/**
 * @return The map's active state
 */
bool MapWidgetView::getActiveState() const
{
    return d->mapWidget->getActiveState();
}

//-------------------------------------------------------------------------------------------------------------

class MapViewModelHelper::MapViewModelHelperPrivate
{
public:
    MapViewModelHelperPrivate()
        : model(0),
          selectionModel(0),
          thumbnailLoadThread(0)
    {
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
    d->thumbnailLoadThread = new ThumbnailLoadThread(this);

    connect(d->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription&, const QPixmap&)));

    // Note: Here we only monitor changes to the database, because changes to the model
    //       are also sent when thumbnails are generated, and we don't want to update
    //       the marker tiler for that!
    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(const ImageChangeset&)),
            this, SLOT(slotImageChange(const ImageChangeset&)), Qt::QueuedConnection);
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
    const ImageInfo info = d->model->imageInfo(index);

    if (info.isNull() || !info.hasCoordinates())
    {
        return false;
    }

    *coordinates = KMap::GeoCoordinates(info.latitudeNumber(), info.longitudeNumber());

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
    const ImageInfo info = d->model->imageInfo(index);

    if (!info.isNull())
    {
        const QString path   = info.filePath();

        if (d->thumbnailLoadThread->find(path, thumbnail, qMax(size.width()+2, size.height()+2)))
        {
            return thumbnail.copy(1, 1, thumbnail.size().width()-2, thumbnail.size().height()-2);
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
 * @param sortKey Determines the sorting options and is actually of type GPSImageInfoSorter::SortOptions
 * @return Returns the index of the marker.
 */
QPersistentModelIndex MapViewModelHelper::bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list,
        const int sortKey)
{
    if (list.isEmpty())
    {
        return QPersistentModelIndex();
    }

    // first convert from QPersistentModelIndex to QModelIndex
    QList<QModelIndex> indexList;

    for (int i=0; i<list.count(); ++i)
    {
        const QModelIndex newIndex(list.at(i));
        indexList.append(newIndex);
    }

    // now get the ImageInfos and convert them to GPSImageInfos
    const QList<ImageInfo> imageInfoList =  d->model->imageInfos(indexList);
    GPSImageInfo::List gpsImageInfoList;
    foreach (const ImageInfo& imageInfo, imageInfoList)
    {
        GPSImageInfo gpsImageInfo;

        if (GPSImageInfo::fromImageInfo(imageInfo, &gpsImageInfo))
        {
            gpsImageInfoList << gpsImageInfo;
        }
    }

    if (gpsImageInfoList.size()!=indexList.size())
    {
        // this is a problem, and unexpected
        return indexList.first();
    }

    // now determine the best available index
    QModelIndex bestIndex = indexList.first();
    GPSImageInfo bestGPSImageInfo = gpsImageInfoList.first();

    for (int i=1; i<gpsImageInfoList.count(); ++i)
    {
        const GPSImageInfo& currentInfo = gpsImageInfoList.at(i);

        if (GPSImageInfoSorter::fitsBetter(bestGPSImageInfo, KMap::KMapSelectedNone,
                                           currentInfo, KMap::KMapSelectedNone,
                                           KMap::KMapSelectedNone, GPSImageInfoSorter::SortOptions(sortKey)))
        {
            bestIndex = indexList.at(i);
            bestGPSImageInfo = currentInfo;
        }
    }

    // and return the index
    return QPersistentModelIndex(bestIndex);
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

    const QModelIndex currentIndex = d->model->indexForPath(loadingDescription.filePath);

    if (currentIndex.isValid())
    {
        QPersistentModelIndex goodIndex(currentIndex);
        emit(signalThumbnailAvailableForIndex(goodIndex, thumb.copy(1, 1, thumb.size().width()-2, thumb.size().height()-2)));
    }
}

/**
 * This functions is called when one clicks on a thumbnail.
 * @param clickedIndices A list containing the marker indices belonging the group whose thumbnail has been clicked.
 */
void MapViewModelHelper::onIndicesClicked(const QList<QPersistentModelIndex>& clickedIndices)
{
    /// @todo What do we do when an image is clicked?

#if 0
    QList<QModelIndex> indexList;

    for (int i=0; i<clickedIndices.count(); ++i)
    {
        const QModelIndex newIndex(clickedIndices.at(i));
        indexList.append(newIndex);
    }

    const QList<ImageInfo> imageInfoList = d->model->imageInfos(indexList);

    QList<qlonglong> imagesIdList;

    for (int i=0; i<imageInfoList.count(); ++i)
    {
        imagesIdList.append(imageInfoList[i].id());
    }

    emit signalFilteredImages(imagesIdList);
#else
    Q_UNUSED(clickedIndices);
#endif
}

void MapViewModelHelper::slotImageChange(const ImageChangeset& changeset)
{
    kDebug() << "---------------------------------------------------------------";
    const DatabaseFields::Set changes = changeset.changes();
//    const DatabaseFields::ImagePositions imagePositionChanges = changes;

    /// @todo More detailed check
    if (   ( changes & DatabaseFields::LatitudeNumber )
           || ( changes & DatabaseFields::LongitudeNumber )
           || ( changes & DatabaseFields::Altitude ) )
    {
        kDebug() << "changes!";

        foreach (const qlonglong& id, changeset.ids())
        {
            const QModelIndex index = d->model->indexForImageId(id);
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

/**
 * @brief Returns the ImageInfo for the current image
 */
ImageInfo MapWidgetView::currentInfo()
{
    /// @todo Have kmapwidget honor the 'current index'
    QModelIndex currentIndex = d->selectionModel->currentIndex();
    kDebug()<<currentIndex;

    if (!currentIndex.isValid())
    {
        /// @todo This is temporary until kmapwidget marks a 'current index'
        if (!d->selectionModel->hasSelection())
        {
            return ImageInfo();
        }

        currentIndex = d->selectionModel->selectedIndexes().first();
    }

    kDebug()<<currentIndex;
    return d->imageFilterModel->imageInfo(currentIndex);
}

} //namespace Digikam
