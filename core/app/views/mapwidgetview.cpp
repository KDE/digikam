/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-15
 * Description : central Map view
 *
 * Copyright (C) 2010         by Gabriel Voicu <ping dot gabi at gmail dot com>
 * Copyright (C) 2010         by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "mapwidgetview.h"

// Qt includes

#include <QWidget>
#include <QVBoxLayout>
#include <QItemSelectionModel>
#include <QAbstractItemModel>
#include <QPersistentModelIndex>

// Local includes

#include "mapwidget.h"
#include "itemmarkertiler.h"

//local includes

#include "digikam_debug.h"
#include "camerathumbsctrl.h"
#include "imageposition.h"
#include "imageinfo.h"
#include "imagemodel.h"
#include "importfiltermodel.h"
#include "importimagemodel.h"
#include "coredbwatch.h"
#include "coredbfields.h"
#include "imagepropertiessidebardb.h"
#include "gpsimageinfosorter.h"
#include "importui.h"

namespace Digikam
{

class ImageAlbumModel;
class ImageFilterModel;

/**
 * @class MapWidgetView
 *
 * @brief Class containing digiKam's central map view.
 */
class MapWidgetView::Private
{
public:

    Private()
        :vbox(0),
         mapWidget(0),
         imageFilterModel(0),
         imageModel(0),
         importFilterModel(0),
         importModel(0),
         selectionModel(0),
         mapViewModelHelper(0),
         gpsImageInfoSorter(0),
         application(MapWidgetView::ApplicationDigikam)
    {
    }

    DVBox*                      vbox;
    MapWidget*                  mapWidget;
    ImageFilterModel*           imageFilterModel;
    ImageAlbumModel*            imageModel;
    ImportFilterModel*          importFilterModel;
    ImportImageModel*           importModel;
    QItemSelectionModel*        selectionModel;
    MapViewModelHelper*         mapViewModelHelper;
    GPSImageInfoSorter*         gpsImageInfoSorter;
    MapWidgetView::Application  application;
};

/**
 * @brief Constructor
 * @param selectionModel digiKam's selection model
 * @param imageFilterModel digikam's filter model
 * @param parent Parent object
 */
MapWidgetView::MapWidgetView(QItemSelectionModel* const selectionModel,
                             DCategorizedSortFilterProxyModel* const imageFilterModel,
                             QWidget* const parent,
                             const MapWidgetView::Application application)
    : QWidget(parent),
      StateSavingObject(this),
      d(new Private())
{
    d->application    = application;
    d->selectionModel = selectionModel;

    switch(d->application)
    {
        case ApplicationDigikam:
            d->imageFilterModel   = dynamic_cast<ImageFilterModel*>(imageFilterModel);
            d->imageModel         = dynamic_cast<ImageAlbumModel*>(imageFilterModel->sourceModel());
            d->mapViewModelHelper = new MapViewModelHelper(d->selectionModel, imageFilterModel, this, ApplicationDigikam);
            break;

        case ApplicationImportUI:
            d->importFilterModel  = dynamic_cast<ImportFilterModel*>(imageFilterModel);
            d->importModel        = dynamic_cast<ImportImageModel*>(imageFilterModel->sourceModel());
            d->mapViewModelHelper = new MapViewModelHelper(d->selectionModel, d->importFilterModel, this, ApplicationImportUI);
            break;
    }

    QVBoxLayout* const vBoxLayout = new QVBoxLayout(this);
    d->mapWidget                  = new MapWidget(this);
    d->mapWidget->setAvailableMouseModes(MouseModePan|MouseModeZoomIntoGroup|MouseModeSelectThumbnail);
    d->mapWidget->setVisibleMouseModes(MouseModePan|MouseModeZoomIntoGroup|MouseModeSelectThumbnail);
    ItemMarkerTiler* const geoifaceMarkerModel = new ItemMarkerTiler(d->mapViewModelHelper, this);
    d->mapWidget->setGroupedModel(geoifaceMarkerModel);
    d->mapWidget->setBackend(QLatin1String("marble"));

    d->gpsImageInfoSorter         = new GPSImageInfoSorter(this);
    d->gpsImageInfoSorter->addToMapWidget(d->mapWidget);
    vBoxLayout->addWidget(d->mapWidget);
    vBoxLayout->addWidget(d->mapWidget->getControlWidget());
}

/**
 * @brief Destructor
 */
MapWidgetView::~MapWidgetView()
{

}

void MapWidgetView::doLoadState()
{
    KConfigGroup group = getConfigGroup();

    d->gpsImageInfoSorter->setSortOptions(GPSImageInfoSorter::SortOptions(group.readEntry(QLatin1String("Sort Order"),
                                                                                          int(d->gpsImageInfoSorter->getSortOptions()))));

    const KConfigGroup groupCentralMap = KConfigGroup(&group, QLatin1String("Central Map Widget"));
    d->mapWidget->readSettingsFromGroup(&groupCentralMap);
}

void MapWidgetView::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    group.writeEntry(QLatin1String("Sort Order"), int(d->gpsImageInfoSorter->getSortOptions()));

    KConfigGroup groupCentralMap = KConfigGroup(&group, QLatin1String("Central Map Widget"));
    d->mapWidget->saveSettingsToGroup(&groupCentralMap);

    group.sync();
}

/**
 * @brief Switch that opens the current album.
 * @param album Current album.
 */
void MapWidgetView::openAlbum(Album* const album)
{
    if (album)
    {
        d->imageModel->openAlbum(QList<Album*>() << album);
    }
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

class MapViewModelHelper::Private
{
public:
    Private()
        : model(0),
          importModel(0),
          selectionModel(0),
          thumbnailLoadThread(0),
          application(MapWidgetView::ApplicationDigikam)
    {

    }

    ImageFilterModel*           model;
    ImportFilterModel*          importModel;
    QItemSelectionModel*        selectionModel;
    ThumbnailLoadThread*        thumbnailLoadThread;
    MapWidgetView::Application  application;
};

MapViewModelHelper::MapViewModelHelper(QItemSelectionModel* const selection,
                                       DCategorizedSortFilterProxyModel* const filterModel,
                                       QObject* const parent,
                                       const MapWidgetView::Application application)
    : GeoModelHelper(parent),
      d(new Private())
{
    d->selectionModel = selection;
    d->application    = application;

    switch (d->application)
    {
        case MapWidgetView::ApplicationDigikam:
            d->model               = dynamic_cast<ImageFilterModel*>(filterModel);
            d->thumbnailLoadThread = new ThumbnailLoadThread(this);

            connect(d->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription, QPixmap)),
                    this, SLOT(slotThumbnailLoaded(LoadingDescription, QPixmap)));

            // Note: Here we only monitor changes to the database, because changes to the model
            //       are also sent when thumbnails are generated, and we don't want to update
            //       the marker tiler for that!
            connect(CoreDbAccess::databaseWatch(), SIGNAL(imageChange(ImageChangeset)),
                    this, SLOT(slotImageChange(ImageChangeset)), Qt::QueuedConnection);
            break;

        case MapWidgetView::ApplicationImportUI:
            d->importModel = dynamic_cast<ImportFilterModel*>(filterModel);

            connect(ImportUI::instance()->getCameraThumbsCtrl(), SIGNAL(signalThumbInfoReady(CamItemInfo)),
                    this, SLOT(slotThumbnailLoaded(CamItemInfo)));

            break;
    }
}

/**
 * @brief Destructor
 */
MapViewModelHelper::~MapViewModelHelper()
{
}

/**
 * @return Returns digiKam's filter model.
 */
QAbstractItemModel* MapViewModelHelper::model() const
{
    switch (d->application)
    {
        case MapWidgetView::ApplicationDigikam:
            return d->model;

        case MapWidgetView::ApplicationImportUI:
            return d->importModel;
    }

    return 0;
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
bool MapViewModelHelper::itemCoordinates(const QModelIndex& index, GeoCoordinates* const coordinates) const
{
    switch (d->application)
    {
        case MapWidgetView::ApplicationDigikam:
        {
            const ImageInfo info = d->model->imageInfo(index);

            if (info.isNull() || !info.hasCoordinates())
            {
                return false;
            }

            *coordinates = GeoCoordinates(info.latitudeNumber(), info.longitudeNumber());
            break;
        }

        case MapWidgetView::ApplicationImportUI:
        {
            const CamItemInfo info = d->importModel->camItemInfo(index);

            if (info.isNull())
            {
                return false;
            }

            const DMetadata meta(info.url().toLocalFile());
            double          lat, lng;
            const bool      haveCoordinates = meta.getGPSLatitudeNumber(&lat) && meta.getGPSLongitudeNumber(&lng);

            if (!haveCoordinates)
            {
                return false;
            }

            GeoCoordinates tmpCoordinates(lat, lng);

            double     alt;
            const bool haveAlt = meta.getGPSAltitude(&alt);

            if (haveAlt)
            {
                tmpCoordinates.setAlt(alt);
            }

            *coordinates = tmpCoordinates;
            break;
        }
    }

    return true;
}

/**
 * @brief This function retrieves the thumbnail for an index.
 * @param index The marker's index.
 * @param size The size of the thumbnail.
 * @return If the thumbnail has been loaded in the ThumbnailLoadThread instance, it is returned. If not, a QPixmap is returned and ThumbnailLoadThread's signal named signalThumbnailLoaded is emitted when the thumbnail becomes available.
 */
QPixmap MapViewModelHelper::pixmapFromRepresentativeIndex(const QPersistentModelIndex& index, const QSize& size)
{
    if (index == QPersistentModelIndex())
    {
        return QPixmap();
    }

    switch (d->application)
    {
        case MapWidgetView::ApplicationDigikam:
        {
            const ImageInfo info = d->model->imageInfo(index);

            if (!info.isNull())
            {
                QPixmap thumbnail;

                if (d->thumbnailLoadThread->find(info.thumbnailIdentifier(), thumbnail, qMax(size.width()+2, size.height()+2)))
                {
                    return thumbnail.copy(1, 1, thumbnail.size().width()-2, thumbnail.size().height()-2);
                }
                else
                {
                    return QPixmap();
                }
            }

            break;
        }

        case MapWidgetView::ApplicationImportUI:
        {
            QPixmap thumbnail = index.data(ImportImageModel::ThumbnailRole).value<QPixmap>();
            return thumbnail.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
    QModelIndex        bestIndex;

    for (int i=0; i < list.count(); ++i)
    {
        const QModelIndex newIndex(list.at(i));
        indexList.append(newIndex);
    }

    switch (d->application)
    {
        case MapWidgetView::ApplicationDigikam:
        {
            // now get the ImageInfos and convert them to GPSImageInfos
            const QList<ImageInfo> imageInfoList =  d->model->imageInfos(indexList);
            GPSImageInfo::List gpsImageInfoList;

            foreach(const ImageInfo& imageInfo, imageInfoList)
            {
                GPSImageInfo gpsImageInfo;

                if (ImagePropertiesSideBarDB::GPSImageInfofromImageInfo(imageInfo, &gpsImageInfo))
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
            bestIndex                     = indexList.first();
            GPSImageInfo bestGPSImageInfo = gpsImageInfoList.first();

            for (int i=1; i < gpsImageInfoList.count(); ++i)
            {
                const GPSImageInfo& currentInfo = gpsImageInfoList.at(i);

                if (GPSImageInfoSorter::fitsBetter(bestGPSImageInfo, SelectedNone,
                                                   currentInfo, SelectedNone,
                                                   SelectedNone, GPSImageInfoSorter::SortOptions(sortKey)))
                {
                    bestIndex        = indexList.at(i);
                    bestGPSImageInfo = currentInfo;
                }
            }

            break;
        }

        case MapWidgetView::ApplicationImportUI:
        {
            // now get the CamItemInfo and convert them to GPSImageInfos
            const QList<CamItemInfo> imageInfoList =  d->importModel->camItemInfos(indexList);
            GPSImageInfo::List       gpsImageInfoList;

            foreach(const CamItemInfo& imageInfo, imageInfoList)
            {
                const DMetadata meta(imageInfo.url().toLocalFile());
                double          lat, lng;
                const bool      hasCoordinates = meta.getGPSLatitudeNumber(&lat) && meta.getGPSLongitudeNumber(&lng);

                if (!hasCoordinates)
                {
                    continue;
                }

                GeoCoordinates coordinates(lat, lng);

                double alt;
                const bool haveAlt = meta.getGPSAltitude(&alt);

                if (haveAlt)
                {
                    coordinates.setAlt(alt);
                }

                GPSImageInfo gpsImageInfo;
                gpsImageInfo.coordinates = coordinates;
                gpsImageInfo.dateTime    = meta.getImageDateTime();
                gpsImageInfo.rating      = meta.getImageRating();
                gpsImageInfo.url         = imageInfo.url();
                gpsImageInfoList << gpsImageInfo;
            }

            if (gpsImageInfoList.size()!=indexList.size())
            {
                // this is a problem, and unexpected
                return indexList.first();
            }

            // now determine the best available index
            bestIndex                     = indexList.first();
            GPSImageInfo bestGPSImageInfo = gpsImageInfoList.first();

            for (int i=1; i < gpsImageInfoList.count(); ++i)
            {
                const GPSImageInfo& currentInfo = gpsImageInfoList.at(i);

                if (GPSImageInfoSorter::fitsBetter(bestGPSImageInfo, SelectedNone,
                                                   currentInfo, SelectedNone,
                                                   SelectedNone, GPSImageInfoSorter::SortOptions(sortKey)))
                {
                    bestIndex        = indexList.at(i);
                    bestGPSImageInfo = currentInfo;
                }
            }

            break;
        }
    }

    // and return the index
    return QPersistentModelIndex(bestIndex);
}

/**
 * @brief Because of a call to pixmapFromRepresentativeIndex, some thumbnails are not yet loaded at the time of requesting.
 *        When each thumbnail loads, this slot is called and emits a signal that announces the map that the thumbnail is available.
 *
 * This slot is used in the Digikam main application UI.
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
 * @brief Because of a call to pixmapFromRepresentativeIndex, some thumbnails are not yet loaded at the time of requesting.
 *        When each thumbnail loads, this slot is called and emits a signal that announces to the map that the thumbnail is available.
 *
 * This slot is used in the ImportUI interface.
 */
void MapViewModelHelper::slotThumbnailLoaded(const CamItemInfo& info)
{
    CachedItem item;
    ImportUI::instance()->getCameraThumbsCtrl()->getThumbInfo(info, item);

    QPixmap pix = item.second;

    if (pix.isNull())
    {
        return;
    }

    const QModelIndex currentIndex = d->importModel->indexForPath(info.folder + info.name);

    if (currentIndex.isValid())
    {
        QPersistentModelIndex goodIndex(currentIndex);
        emit(signalThumbnailAvailableForIndex(goodIndex, pix.copy(1, 1, pix.size().width()-2, pix.size().height()-2)));
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

    for (int i=0; i < imageInfoList.count(); ++i)
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
    const DatabaseFields::Set changes = changeset.changes();
//    const DatabaseFields::ImagePositions imagePositionChanges = changes;

    /// @todo More detailed check
    if (( changes & DatabaseFields::LatitudeNumber )  ||
        ( changes & DatabaseFields::LongitudeNumber ) ||
        ( changes & DatabaseFields::Altitude ) )
    {
        foreach(const qlonglong& id, changeset.ids())
        {
            const QModelIndex index = d->model->indexForImageId(id);

            if (index.isValid())
            {
                emit(signalModelChangedDrastically());
                break;
            }
        }
    }
}

/**
 * @brief Returns the ImageInfo for the current image
 */
ImageInfo MapWidgetView::currentImageInfo() const
{
    /// @todo Have geoifacewidget honor the 'current index'
    QModelIndex currentIndex = d->selectionModel->currentIndex();

    if (!currentIndex.isValid())
    {
        /// @todo This is temporary until geoifacewidget marks a 'current index'
        if (!d->selectionModel->hasSelection())
        {
            return ImageInfo();
        }

        currentIndex = d->selectionModel->selectedIndexes().first();
    }

    return d->imageFilterModel->imageInfo(currentIndex);
}

/**
 * @brief Returns the CamItemInfo for the current image
 */
CamItemInfo MapWidgetView::currentCamItemInfo() const
{
    /// @todo Have geoifacewidget honor the 'current index'
    QModelIndex currentIndex = d->selectionModel->currentIndex();

    if (!currentIndex.isValid())
    {
        /// @todo This is temporary until geoifacewidget marks a 'current index'
        if (!d->selectionModel->hasSelection())
        {
            return CamItemInfo();
        }

        currentIndex = d->selectionModel->selectedIndexes().first();
    }

    return d->importFilterModel->camItemInfo(currentIndex);
}

} // namespace Digikam
