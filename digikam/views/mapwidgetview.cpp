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

// libkgeomap includes

#include <libkgeomap/kgeomap_widget.h>
#include <libkgeomap/itemmarkertiler.h>

//local includes

#include "imageposition.h"
#include "imageinfo.h"
#include "imagemodel.h"
#include "importfiltermodel.h"
#include "importimagemodel.h"
#include "databasewatch.h"
#include "databasefields.h"
#include "digikam2kgeomap_database.h"
#include "importui.h"

namespace Digikam
{

class ImageAlbumModel;
class ImageFilterModel;

/**
 * @class MapWidgetView
 *
 * @brief Class containing Digikam's central map view.
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
         mode(false)
    {
    }

    KVBox*                  vbox;
    KGeoMap::KGeoMapWidget* mapWidget;
    ImageFilterModel*       imageFilterModel;
    ImageAlbumModel*        imageModel;
    ImportFilterModel*      importFilterModel;
    ImportImageModel*       importModel;
    QItemSelectionModel*    selectionModel;
    MapViewModelHelper*     mapViewModelHelper;
    GPSImageInfoSorter*     gpsImageInfoSorter;
    bool                    mode;
};

/**
 * @brief Constructor
 * @param selectionModel digiKam's selection model
 * @param imageFilterModel digikam's filter model
 * @param parent Parent object
 */
MapWidgetView::MapWidgetView(QItemSelectionModel* const selectionModel,
                             KCategorizedSortFilterProxyModel* const imageFilterModel, QWidget* const parent, bool mode)
    : QWidget(parent), StateSavingObject(this), d(new Private())
{
    d->mode = mode;

    if (d->mode)
    {
        d->imageFilterModel   = dynamic_cast<ImageFilterModel*>(imageFilterModel);
        d->imageModel         = dynamic_cast<ImageAlbumModel*>(imageFilterModel->sourceModel());
        d->mapViewModelHelper = new MapViewModelHelper(d->selectionModel, imageFilterModel, this);
    }
    else
    {
        d->importFilterModel  = dynamic_cast<ImportFilterModel*>(imageFilterModel);
        d->importModel        = dynamic_cast<ImportImageModel*>(imageFilterModel->sourceModel());
        d->mapViewModelHelper = new MapViewModelHelper(d->selectionModel, d->importFilterModel, this, false);
    }

    d->selectionModel             = selectionModel;
    QVBoxLayout* const vBoxLayout = new QVBoxLayout(this);
    d->mapWidget                  = new KGeoMap::KGeoMapWidget(this);
    d->mapWidget->setAvailableMouseModes(KGeoMap::MouseModePan|KGeoMap::MouseModeZoomIntoGroup|KGeoMap::MouseModeSelectThumbnail);
    d->mapWidget->setVisibleMouseModes(KGeoMap::MouseModePan|KGeoMap::MouseModeZoomIntoGroup|KGeoMap::MouseModeSelectThumbnail);
    KGeoMap::ItemMarkerTiler* const kgeomapMarkerModel = new KGeoMap::ItemMarkerTiler(d->mapViewModelHelper, this);
    d->mapWidget->setGroupedModel(kgeomapMarkerModel);
    d->mapWidget->setBackend("marble");

    d->gpsImageInfoSorter         = new GPSImageInfoSorter(this);
    d->gpsImageInfoSorter->addToKGeoMapWidget(d->mapWidget);
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

class MapViewModelHelper::Private
{
public:
    Private()
        : model(0),
          importModel(0),
          selectionModel(0),
          thumbnailLoadThread(0),
          mode(false)
    {
    }

    ImageFilterModel*    model;
    ImportFilterModel*   importModel;
    QItemSelectionModel* selectionModel;
    ThumbnailLoadThread* thumbnailLoadThread;
    bool                 mode;
};

MapViewModelHelper::MapViewModelHelper(QItemSelectionModel* const selection,
                                       KCategorizedSortFilterProxyModel* const filterModel, QObject* const parent, bool mode)
    : KGeoMap::ModelHelper(parent), d(new Private())
{
    d->selectionModel = selection;
    d->mode           = mode;

    if (d->mode)
    {
        d->model               = dynamic_cast<ImageFilterModel*>(filterModel);
        d->thumbnailLoadThread = new ThumbnailLoadThread(this);

        connect(d->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
                this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));

        // Note: Here we only monitor changes to the database, because changes to the model
        //       are also sent when thumbnails are generated, and we don't want to update
        //       the marker tiler for that!
        connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(ImageChangeset)),
                this, SLOT(slotImageChange(ImageChangeset)), Qt::QueuedConnection);
    }
    else
    {
        d->importModel = dynamic_cast<ImportFilterModel*>(filterModel);

        connect(ImportUI::instance()->getCameraController(), SIGNAL(signalThumbInfo(QString,QString,CamItemInfo,QImage)),
                this, SLOT(slotThumbnailLoaded(QString,QString,CamItemInfo,QImage)));
    }
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
    if (d->mode)
    {
        return d->model;
    }
    else
    {
        return d->importModel;
    }
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
bool MapViewModelHelper::itemCoordinates(const QModelIndex& index, KGeoMap::GeoCoordinates* const coordinates) const
{
    if (d->mode)
    {
        const ImageInfo info = d->model->imageInfo(index);

        if (info.isNull() || !info.hasCoordinates())
        {
            return false;
        }

        *coordinates = KGeoMap::GeoCoordinates(info.latitudeNumber(), info.longitudeNumber());
    }
    else
    {
        const CamItemInfo info = d->importModel->camItemInfo(index);

        if (info.isNull())
        {
            return false;
        }

        double lat, lng;
        const DMetadata meta(info.url().toLocalFile());
        const bool haveCoordinates = meta.getGPSLatitudeNumber(&lat) && meta.getGPSLongitudeNumber(&lng);

        if (haveCoordinates)
        {
            double alt;
            const bool haveAlt = meta.getGPSAltitude(&alt);
            KGeoMap::GeoCoordinates tmpCoordinates(lat, lng);

            if (haveAlt)
            {
                tmpCoordinates.setAlt(alt);
            }

            *coordinates = tmpCoordinates;
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

    if (d->mode)
    {
        const ImageInfo info = d->model->imageInfo(index);

        if (!info.isNull())
        {
            const QString path = info.filePath();
            QPixmap thumbnail;

            if (d->thumbnailLoadThread->find(path, thumbnail, qMax(size.width()+2, size.height()+2)))
            {
                return thumbnail.copy(1, 1, thumbnail.size().width()-2, thumbnail.size().height()-2);
            }
            else
            {
                return QPixmap();
            }
        }
    }
    else
    {
        return index.data(ImportImageModel::ThumbnailRole).value<QPixmap>();
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

    for (int i=0; i<list.count(); ++i)
    {
        const QModelIndex newIndex(list.at(i));
        indexList.append(newIndex);
    }

    if (d->mode)
    {
        // now get the ImageInfos and convert them to GPSImageInfos
        const QList<ImageInfo> imageInfoList =  d->model->imageInfos(indexList);
        GPSImageInfo::List gpsImageInfoList;

        foreach(const ImageInfo& imageInfo, imageInfoList)
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
        bestIndex                     = indexList.first();
        GPSImageInfo bestGPSImageInfo = gpsImageInfoList.first();

        for (int i=1; i<gpsImageInfoList.count(); ++i)
        {
            const GPSImageInfo& currentInfo = gpsImageInfoList.at(i);

            if (GPSImageInfoSorter::fitsBetter(bestGPSImageInfo, KGeoMap::KGeoMapSelectedNone,
                                               currentInfo, KGeoMap::KGeoMapSelectedNone,
                                               KGeoMap::KGeoMapSelectedNone, GPSImageInfoSorter::SortOptions(sortKey)))
            {
                bestIndex        = indexList.at(i);
                bestGPSImageInfo = currentInfo;
            }
        }
    }
    else
    {
        // now get the CamItemInfo and convert them to GPSImageInfos
        const QList<CamItemInfo> imageInfoList =  d->importModel->camItemInfos(indexList);
        GPSImageInfo::List       gpsImageInfoList;

        foreach(const CamItemInfo& imageInfo, imageInfoList)
        {
            const DMetadata meta(imageInfo.url().toLocalFile());
            double lat, lng;
            meta.getGPSLatitudeNumber(&lat) && meta.getGPSLongitudeNumber(&lng);

            double alt;
            const bool haveAlt = meta.getGPSAltitude(&alt);
            KGeoMap::GeoCoordinates coordinates(lat, lng);

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

        for (int i=1; i<gpsImageInfoList.count(); ++i)
        {
            const GPSImageInfo& currentInfo = gpsImageInfoList.at(i);

            if (GPSImageInfoSorter::fitsBetter(bestGPSImageInfo, KGeoMap::KGeoMapSelectedNone,
                                               currentInfo, KGeoMap::KGeoMapSelectedNone,
                                               KGeoMap::KGeoMapSelectedNone, GPSImageInfoSorter::SortOptions(sortKey)))
            {
                bestIndex        = indexList.at(i);
                bestGPSImageInfo = currentInfo;
            }
        }
    }
    // and return the index
    return QPersistentModelIndex(bestIndex);
}

/**
 * @brief Because of a call to pixmapFromRepresentativeIndex, some thumbnails are not yet loaded at the time of requesting. 
 *        When each thumbnail loads, this slot is called and emits a signal that announces the map that the thumbnail is available.
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
 *        When each thumbnail loads, this slot is called and emits a signal that announces the map that the thumbnail is available.
 */
void MapViewModelHelper::slotThumbnailLoaded(const QString& folder, const QString& file, const CamItemInfo& /*info*/, const QImage& thumb)
{
    if (thumb.isNull())
    {
        return;
    }

    QPixmap pix                    = QPixmap::fromImage(thumb);
    const QModelIndex currentIndex = d->importModel->indexForPath(folder + file);

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
    const DatabaseFields::Set changes = changeset.changes();
//    const DatabaseFields::ImagePositions imagePositionChanges = changes;

    /// @todo More detailed check
    if (   ( changes & DatabaseFields::LatitudeNumber )  ||
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
    /// @todo Have kgeomapwidget honor the 'current index'
    QModelIndex currentIndex = d->selectionModel->currentIndex();

    if (!currentIndex.isValid())
    {
        /// @todo This is temporary until kgeomapwidget marks a 'current index'
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
    /// @todo Have kgeomapwidget honor the 'current index'
    QModelIndex currentIndex = d->selectionModel->currentIndex();

    if (!currentIndex.isValid())
    {
        /// @todo This is temporary until kgeomapwidget marks a 'current index'
        if (!d->selectionModel->hasSelection())
        {
            return CamItemInfo();
        }

        currentIndex = d->selectionModel->selectedIndexes().first();
    }

    return d->importFilterModel->camItemInfo(currentIndex);
}

} //namespace Digikam
