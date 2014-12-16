/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-20
 * Description : GPS search marker tiler
 *
 * Copyright (C) 2010       by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010       by Gabriel Voicu <ping dot gabi at gmail dot com>
 * Copyright (C) 2010, 2011 by Michael G. Hansen <mike at mghansen dot de>
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

#include "gpsmarkertiler.moc"

// Qt includes

#include <QPair>
#include <QRectF>
#include <QTimer>

// local includes

#include "digikam2kgeomap_database.h"
#include "dnotificationwrapper.h"
#include "digikamapp.h"

/// @todo Actually use this definition!
typedef QPair<KGeoMap::TileIndex, int> MapPair;
Q_DECLARE_METATYPE(MapPair)

namespace Digikam
{

/**
 * @class GPSMarkerTiler
 *
 * @brief Marker model for storing data needed to display markers on the map. The data is retrieved from Digikam's database.
 */

class GPSMarkerTiler::MyTile : public Tile
{
public:
    MyTile()
        : Tile()
    {
    }

    /**
     * Note: MyTile is only deleted by GPSMarkerTiler::tileDelete.
     * All subclasses of AbstractMarkerTiler have to reimplement tileDelete
     * to delete their Tile subclasses.
     * This was done in order not to have any virtual functions
     * in Tile and its subclasses in order to save memory, since there
     * can be a lot of tiles in a MarkerTiler.
     */
    ~MyTile()
    {
    }

    QList<qlonglong> imagesId;
};

class GPSMarkerTiler::GPSMarkerTilerPrivate
{
public:

    class InternalJobs
    {
    public:

        InternalJobs()
            : level(0),
              kioJob(0),
              dataFromDatabase()
        {
        }

        int                      level;
        KIO::Job*                kioJob;
        QList<GPSImageInfo> dataFromDatabase;
    };

    GPSMarkerTilerPrivate()
        : jobs(),
          thumbnailLoadThread(0),
          thumbnailMap(),
          rectList(),
          rectLevel(),
          activeState(true),
          imagesHash(),
          imageFilterModel(),
          imageAlbumModel(),
          selectionModel(),
          currentRegionSelection(),
          mapGlobalGroupState()
    {
    }

    QList<InternalJobs>                    jobs;
    ThumbnailLoadThread*                   thumbnailLoadThread;
    QHash<qlonglong, QVariant>             thumbnailMap;
    QList<QRectF>                          rectList;
    QList<int>                             rectLevel;
    bool                                   activeState;
    QHash<qlonglong, GPSImageInfo>         imagesHash;
    ImageFilterModel*                      imageFilterModel;
    ImageAlbumModel*                       imageAlbumModel;
    QItemSelectionModel*                   selectionModel;
    KGeoMap::GeoCoordinates::Pair             currentRegionSelection;
    KGeoMap::KGeoMapGroupState                   mapGlobalGroupState;
};

/**
 * @brief Constructor
 * @param parent Parent object
 */
GPSMarkerTiler::GPSMarkerTiler(QObject* const parent, ImageFilterModel* const imageFilterModel, QItemSelectionModel* const selectionModel)
    : KGeoMap::AbstractMarkerTiler(parent), d(new GPSMarkerTilerPrivate())
{
    resetRootTile();

    d->thumbnailLoadThread = new ThumbnailLoadThread(this);
    d->imageFilterModel    = imageFilterModel;
    d->imageAlbumModel     = qobject_cast<ImageAlbumModel*>(imageFilterModel->sourceModel());
    d->selectionModel      = selectionModel;

    connect(d->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotImageChange(ImageChangeset)), Qt::QueuedConnection);

    connect(d->imageAlbumModel, SIGNAL(imageInfosAdded(QList<ImageInfo>)),
            this, SLOT(slotNewModelData(QList<ImageInfo>)));

    connect(d->selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)));
}

/**
 * @brief Destructor
 */
GPSMarkerTiler::~GPSMarkerTiler()
{
    // WARNING: we have to call clear! By the time AbstractMarkerTiler calls clear,
    // this object does not exist any more, and thus the tiles are not correctly destroyed!
    clear();

    delete d;
}

void GPSMarkerTiler::regenerateTiles()
{
}

/**
 * @brief Requests all images inside a given rectangle from the database.
 *
 * This function calls the database for the images found inside a rectangle
 * defined by upperLeft and lowerRight points. The images are returned from
 * the database in batches.
 *
 * @param upperLeft The North-West point.
 * @param lowerRight The South-East point.
 * @param level The requested tiling level.
 */
void GPSMarkerTiler::prepareTiles(const KGeoMap::GeoCoordinates& upperLeft, const KGeoMap::GeoCoordinates& lowerRight, int level)
{
    qreal lat1 = upperLeft.lat();
    qreal lng1 = upperLeft.lon();
    qreal lat2 = lowerRight.lat();
    qreal lng2 = lowerRight.lon();
    const QRectF requestedRect(lat1, lng1, lat2 - lat1, lng2 - lng1);

    for (int i = 0; i < d->rectList.count(); ++i)
    {
        if (level != d->rectLevel.at(i))
        {
            continue;
        }

        qreal rectLat1, rectLng1, rectLat2, rectLng2;
        const QRectF currentRect = d->rectList.at(i);
        currentRect.getCoords(&rectLat1, &rectLng1, &rectLat2, &rectLng2);

        //do nothing if this rectangle was already requested
        if (currentRect.contains(requestedRect))
        {
            return;
        }

        if (currentRect.contains(lat1, lng1))
        {
            if (currentRect.contains(lat2, lng1))
            {
                lng1 = rectLng2;
                break;
            }
        }
        else if (currentRect.contains(lat2, lng1))
        {
            if (currentRect.contains(lat2, lng2))
            {
                lat2 = rectLat1;
                break;
            }
        }
        else if (currentRect.contains(lat2, lng2))
        {
            if (currentRect.contains(lat1, lng2))
            {
                lng2 = rectLng1;
                break;
            }
        }
        else if (currentRect.contains(lat1, lng2))
        {
            if (currentRect.contains(lat1, lng1))
            {
                lat1 = rectLat2;
                break;
            }
        }
    }

    const QRectF newRect(lat1, lng1, lat2 - lat1, lng2 - lng1);

    d->rectList.append(newRect);

    d->rectLevel.append(level);

    kDebug() << "Listing" << lat1 << lat2 << lng1 << lng2;

    DatabaseUrl u              = DatabaseUrl::fromAreaRange(lat1, lat2, lng1, lng2);
    KIO::Job* const currentJob = ImageLister::startListJob(u);

    currentJob->addMetaData("wantDirectQuery", "false");

    GPSMarkerTilerPrivate::InternalJobs currentJobInfo;

    currentJobInfo.kioJob = currentJob;
    currentJobInfo.level  = level;

    d->jobs.append(currentJobInfo);

    connect(currentJob, SIGNAL(result(KJob*)),
            this, SLOT(slotMapImagesJobResult(KJob*)));

    connect(currentJob, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(slotMapImagesJobData(KIO::Job*,QByteArray)));
}

/**
 * @brief Returns a pointer to a tile.
 * @param tileIndex The index of a tile.
 * @param stopIfEmpty Determines whether child tiles are also created for empty tiles.
 */
KGeoMap::AbstractMarkerTiler::Tile* GPSMarkerTiler::getTile(const KGeoMap::TileIndex& tileIndex, const bool stopIfEmpty)
{
    KGEOMAP_ASSERT(tileIndex.level() <= KGeoMap::TileIndex::MaxLevel);

    MyTile* tile = static_cast<MyTile*>(rootTile());

    for (int level = 0; level < tileIndex.indexCount(); ++level)
    {
        const int currentIndex = tileIndex.linearIndex(level);
        MyTile* childTile = 0;

        if (tile->childrenEmpty())
        {
            if (stopIfEmpty)
            {
                return 0;
            }

            for (int i = 0; i < tile->imagesId.count(); ++i)
            {
                const int currentImageId = tile->imagesId.at(i);
                const GPSImageInfo currentImageInfo = d->imagesHash[currentImageId];
                const KGeoMap::TileIndex markerTileIndex = KGeoMap::TileIndex::fromCoordinates(currentImageInfo.coordinates, level);
                const int newTileIndex = markerTileIndex.lastIndex();

                MyTile* newTile = static_cast<MyTile*>(tile->getChild(newTileIndex));

                if (newTile == 0)
                {
                    MyTile* newTile = static_cast<MyTile*>(tileNew());
                    newTile->imagesId.append(currentImageId);
                    tile->addChild(newTileIndex, newTile);
                }
                else
                {
                    if (!newTile->imagesId.contains(currentImageId))
                    {
                        newTile->imagesId.append(currentImageId);
                    }
                }
            }
        }

        childTile = static_cast<MyTile*>(tile->getChild(currentIndex));

        if (childTile == 0)
        {
            if (stopIfEmpty)
            {
                // there will be no markers in this tile, therefore stop
                return 0;
            }

            childTile = static_cast<MyTile*>(tileNew());
            tile->addChild(currentIndex, childTile);
        }

        tile = childTile;
    }

    return tile;
}

int GPSMarkerTiler::getTileMarkerCount(const KGeoMap::TileIndex& tileIndex)
{
    MyTile* tile = static_cast<MyTile*>(getTile(tileIndex));

    if (tile)
    {
        return tile->imagesId.count();
    }

    return 0;
}

int GPSMarkerTiler::getTileSelectedCount(const KGeoMap::TileIndex& tileIndex)
{
    Q_UNUSED(tileIndex)

    return 0;
}

/**
 @brief This function finds the best representative marker from a tile of markers.
 * @param tileIndex Index of the tile from which the best marker should be found.
 * @param sortKey Sets the criteria for selecting the representative thumbnail, a combination of the SortOptions bits.
 * @return Returns the internally used index of the marker.
 */
QVariant GPSMarkerTiler::getTileRepresentativeMarker(const KGeoMap::TileIndex& tileIndex, const int sortKey)
{
    MyTile* const tile = static_cast<MyTile*>(getTile(tileIndex, true));

    if (!tile)
    {
        return QVariant();
    }

    if (tile->imagesId.isEmpty())
    {
        return QVariant();
    }

    GPSImageInfo bestMarkerInfo = d->imagesHash.value(tile->imagesId.first());
    KGeoMap::KGeoMapGroupState bestMarkerGroupState = getImageState(bestMarkerInfo.id);

    for (int i = 1; i < tile->imagesId.count(); ++i)
    {
        const GPSImageInfo currentMarkerInfo = d->imagesHash.value(tile->imagesId.at(i));
        const KGeoMap::KGeoMapGroupState currentMarkerGroupState = getImageState(currentMarkerInfo.id);

        if (GPSImageInfoSorter::fitsBetter(bestMarkerInfo, bestMarkerGroupState, currentMarkerInfo, currentMarkerGroupState, getGlobalGroupState(), GPSImageInfoSorter::SortOptions(sortKey)))
        {
            bestMarkerInfo = currentMarkerInfo;
            bestMarkerGroupState = currentMarkerGroupState;
        }
    }

    const QPair<KGeoMap::TileIndex, int> returnedMarker(tileIndex, bestMarkerInfo.id);

    return QVariant::fromValue(returnedMarker);
}

/**
 @brief This function finds the best representative marker from a group of markers. This is needed to display a thumbnail for a marker group.
 * @param indices A list containing markers, obtained by getTileRepresentativeMarker.
 * @param sortKey Sets the criteria for selecting the representative thumbnail, a combination of the SortOptions bits.
 * @return Returns the internally used index of the marker.
 */
QVariant GPSMarkerTiler::bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey)
{
    if (indices.isEmpty())
    {
        return QVariant();
    }

    const QPair<KGeoMap::TileIndex, int> firstIndex = indices.first().value<QPair<KGeoMap::TileIndex, int> >();

    GPSImageInfo bestMarkerInfo = d->imagesHash.value(firstIndex.second);

    KGeoMap::KGeoMapGroupState bestMarkerGroupState = getImageState(firstIndex.second);

    KGeoMap::TileIndex bestMarkerTileIndex = firstIndex.first;

    for (int i = 1; i < indices.count(); ++i)
    {
        const QPair<KGeoMap::TileIndex, int> currentIndex = indices.at(i).value<QPair<KGeoMap::TileIndex, int> >();

        GPSImageInfo currentMarkerInfo = d->imagesHash.value(currentIndex.second);
        KGeoMap::KGeoMapGroupState currentMarkerGroupState = getImageState(currentIndex.second);

        if (GPSImageInfoSorter::fitsBetter(bestMarkerInfo, bestMarkerGroupState, currentMarkerInfo, currentMarkerGroupState, getGlobalGroupState(), GPSImageInfoSorter::SortOptions(sortKey)))
        {
            bestMarkerInfo = currentMarkerInfo;
            bestMarkerGroupState = currentMarkerGroupState;
            bestMarkerTileIndex = currentIndex.first;
        }
    }

    const QPair<KGeoMap::TileIndex, int> returnedMarker(bestMarkerTileIndex, bestMarkerInfo.id);

    return QVariant::fromValue(returnedMarker);
}

/**
 * @brief This function retrieves the thumbnail for an index.
 * @param index The marker's index.
 * @param size The size of the thumbnail.
 * @return If the thumbnail has been loaded in the ThumbnailLoadThread instance, it is returned. If not, a QPixmap is returned and ThumbnailLoadThread's signal named signalThumbnailLoaded is emitted when the thumbnail becomes available.
 */
QPixmap GPSMarkerTiler::pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size)
{
    QPair<KGeoMap::TileIndex, int> indexForPixmap = index.value<QPair<KGeoMap::TileIndex, int> >();

    QPixmap thumbnail;
    ImageInfo info(indexForPixmap.second);
    d->thumbnailMap.insert(info.id(), index);

    if (d->thumbnailLoadThread->find(info.thumbnailIdentifier(), thumbnail, qMax(size.width() + 2, size.height() + 2)))
    {
        // digikam returns thumbnails with a border around them, but libkgeomap expects them without a border
        return thumbnail.copy(1, 1, thumbnail.size().width() - 2, thumbnail.size().height() - 2);
    }
    else
    {
        return QPixmap();
    }
}

/**
 * @brief This function compares two marker indices.
 */
bool GPSMarkerTiler::indicesEqual(const QVariant& a, const QVariant& b) const
{
    QPair<KGeoMap::TileIndex, int> firstIndex = a.value<QPair<KGeoMap::TileIndex, int> >();
    QPair<KGeoMap::TileIndex, int> secondIndex = b.value<QPair<KGeoMap::TileIndex, int> >();

    QList<int> aIndicesList = firstIndex.first.toIntList();
    QList<int> bIndicesList = secondIndex.first.toIntList();

    if (firstIndex.second == secondIndex.second && aIndicesList == bIndicesList)
    {
        return true;
    }

    return false;
}

KGeoMap::KGeoMapGroupState GPSMarkerTiler::getTileGroupState(const KGeoMap::TileIndex& tileIndex)
{
    const bool haveGlobalSelection = (d->mapGlobalGroupState & (KGeoMap::KGeoMapFilteredPositiveMask | KGeoMap::KGeoMapRegionSelectedMask));

    if (!haveGlobalSelection)
    {
        return KGeoMap::KGeoMapSelectedNone;
    }

    /// @todo Store this state in the tiles!
    MyTile* tile = static_cast<MyTile*>(getTile(tileIndex, true));
    KGeoMap::KGeoMapGroupStateComputer tileStateComputer;

    for (int i = 0; i < tile->imagesId.count(); ++i)
    {
        const KGeoMap::KGeoMapGroupState imageState = getImageState(tile->imagesId.at(i));

        tileStateComputer.addState(imageState);
    }

    return tileStateComputer.getState();
}

/**
 * @brief The marker data is returned from the database in batches. This function takes and unites the batches.
 */
void GPSMarkerTiler::slotMapImagesJobData(KIO::Job* job, const QByteArray& data)
{
    if (data.isEmpty())
    {
        return;
    }

    QByteArray  di(data);
    QDataStream ds(&di, QIODevice::ReadOnly);

    GPSMarkerTilerPrivate::InternalJobs* internalJob = 0;

    for (int i = 0; i < d->jobs.count(); ++i)
    {
        if (job == d->jobs.at(i).kioJob)
        {
            /// @todo Is this really safe?
            internalJob = &d->jobs[i];
            break;
        }
    }

    if (!internalJob)
    {
        return;
    }

    if (!ImageListerRecord::checkStream(ImageListerRecord::ExtraValueFormat, ds))
    {
        kError() << "Binary stream from ioslave is not valid, rejecting";
        return;
    }

    GPSImageInfo::List newEntries;

    while (!ds.atEnd())
    {
        ImageListerRecord record(ImageListerRecord::ExtraValueFormat);
        ds >> record;

        if (record.extraValues.count() < 2)
        {
            // skip info without coordinates
            continue;
        }

        GPSImageInfo entry;

        entry.id           = record.imageID;
        entry.rating       = record.rating;
        entry.dateTime     = record.creationDate;
        entry.coordinates.setLatLon(record.extraValues.first().toDouble(), record.extraValues.last().toDouble());

        internalJob->dataFromDatabase << entry;
    }
}

/**
 * @brief Now, all the marker data has been retrieved from the database. Here, the markers are sorted into tiles.
 */
void GPSMarkerTiler::slotMapImagesJobResult(KJob* job)
{
    KIO::Job* const currentJob = qobject_cast<KIO::Job*>(job);

    int foundIndex = -1;

    for (int i = 0; i < d->jobs.count(); ++i)
    {
        if (currentJob == d->jobs.at(i).kioJob)
        {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex < 0)
    {
        // this should not happen, but ok...
        return;
    }

    if (job->error())
    {
        kWarning() << "Failed to list images in selected area:" << job->errorString();

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), job->errorString(),
                             DigikamApp::instance(), DigikamApp::instance()->windowTitle());
    }

    // get the results from the job:
    const QList<GPSImageInfo> returnedImageInfo = d->jobs.at(foundIndex).dataFromDatabase;
    /// @todo Currently, we ignore the wanted level and just add the images
    //     const int wantedLevel = d->jobs.at(foundIndex).level;

    // remove the finished job
    d->jobs[foundIndex].kioJob->kill();
    d->jobs[foundIndex].kioJob = 0;
    d->jobs.removeAt(foundIndex);

    if (returnedImageInfo.isEmpty())
    {
        return;
    }

    for (int i = 0; i < returnedImageInfo.count(); ++i)
    {
        const GPSImageInfo currentImageInfo = returnedImageInfo.at(i);

        if (!currentImageInfo.coordinates.hasCoordinates())
        {
            continue;
        }

        d->imagesHash.insert(currentImageInfo.id, currentImageInfo);

        const KGeoMap::TileIndex markerTileIndex = KGeoMap::TileIndex::fromCoordinates(currentImageInfo.coordinates, KGeoMap::TileIndex::MaxLevel);
        addMarkerToTileAndChildren(currentImageInfo.id, markerTileIndex, static_cast<MyTile*>(rootTile()), 0);
    }

    emit(signalTilesOrSelectionChanged());
}

/**
 * @brief Because of a call to pixmapFromRepresentativeIndex, some thumbnails are not yet loaded at the time of requesting. When each thumbnail loads, this slot is called and emits a signal that announces the map that the thumbnail is available.
 */
void GPSMarkerTiler::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumbnail)
{
    QVariant index = d->thumbnailMap.value(loadingDescription.thumbnailIdentifier().id);
    //    QPair<KGeoMap::TileIndex, int> indexForPixmap =
    //        index.value<QPair<KGeoMap::TileIndex, int> >();
    emit signalThumbnailAvailableForIndex(index, thumbnail.copy(1, 1, thumbnail.size().width() - 2, thumbnail.size().height() - 2));
}

/**
 * @brief Sets the map active/inactive
 * @param state New state of the map, true means active.
 */
void GPSMarkerTiler::setActive(const bool state)
{
    d->activeState = state;
}

KGeoMap::AbstractMarkerTiler::Tile* GPSMarkerTiler::tileNew()
{
    return new MyTile();
}

void GPSMarkerTiler::tileDelete(KGeoMap::AbstractMarkerTiler::Tile* const tile)
{
    delete static_cast<MyTile*>(tile);
}

/**
 * @brief Receives notifications from the database when images were changed and updates the tiler
 */
void GPSMarkerTiler::slotImageChange(const ImageChangeset& changeset)
{
    const DatabaseFields::Set changes = changeset.changes();
    //    const DatabaseFields::ImagePositions imagePositionChanges = changes;

    if (!((changes & DatabaseFields::LatitudeNumber)
          || (changes & DatabaseFields::LongitudeNumber)
          || (changes & DatabaseFields::Altitude)))
    {
        return;
    }

    foreach(const qlonglong& id, changeset.ids())
    {
        const ImageInfo newImageInfo(id);

        if (!newImageInfo.hasCoordinates())
        {
            // the image has no coordinates any more
            // remove it from the tiles and the image list
            const GPSImageInfo oldInfo = d->imagesHash.value(id);
            const KGeoMap::GeoCoordinates oldCoordinates = oldInfo.coordinates;
            const KGeoMap::TileIndex oldTileIndex = KGeoMap::TileIndex::fromCoordinates(oldCoordinates, KGeoMap::TileIndex::MaxLevel);

            removeMarkerFromTileAndChildren(id, oldTileIndex, static_cast<MyTile*>(rootTile()), 0, 0);

            d->imagesHash.remove(id);

            continue;
        }

        KGeoMap::GeoCoordinates newCoordinates(newImageInfo.latitudeNumber(), newImageInfo.longitudeNumber());

        if (newImageInfo.hasAltitude())
        {
            newCoordinates.setAlt(newImageInfo.altitudeNumber());
        }

        if (d->imagesHash.contains(id))
        {
            // the image id is known, therefore the image has already been sorted into tiles.
            // We assume that the coordinates of the image have changed.

            const GPSImageInfo oldInfo = d->imagesHash.value(id);
            const KGeoMap::GeoCoordinates oldCoordinates = oldInfo.coordinates;

            const GPSImageInfo currentImageInfo = GPSImageInfo::fromIdCoordinatesRatingDateTime(id, newCoordinates, newImageInfo.rating(), newImageInfo.dateTime());
            d->imagesHash.insert(id, currentImageInfo);

            const KGeoMap::TileIndex oldTileIndex = KGeoMap::TileIndex::fromCoordinates(oldCoordinates, KGeoMap::TileIndex::MaxLevel);
            const KGeoMap::TileIndex newTileIndex = KGeoMap::TileIndex::fromCoordinates(newCoordinates, KGeoMap::TileIndex::MaxLevel);

            // find out up to which level the tile indices are equal
            int separatorLevel = -1;

            for (int i = 0; i < KGeoMap::TileIndex::MaxLevel; ++i)
            {
                if (oldTileIndex.at(i) != newTileIndex.at(i))
                {
                    separatorLevel = i;
                    break;
                }
            }

            if (separatorLevel == -1)
            {
                // the tile index has not changed
                continue;
            }

            MyTile* currentTileOld = static_cast<MyTile*>(rootTile());
            MyTile* currentTileNew = currentTileOld;

            int level = 0;

            for (level = 0; level <= oldTileIndex.level(); ++level)
            {
                if (currentTileOld->childrenEmpty())
                {
                    break;
                }

                const int tileIndex = oldTileIndex.at(level);

                MyTile* childTileOld = static_cast<MyTile*>(currentTileOld->getChild(tileIndex));

                if (childTileOld == 0)
                {
                    break;
                }

                if (level < separatorLevel)
                {
                    currentTileOld = childTileOld;
                    currentTileNew = currentTileOld;
                }
                else
                {
                    removeMarkerFromTileAndChildren(id, oldTileIndex, childTileOld, level, currentTileOld);

                    break;
                }
            }

            addMarkerToTileAndChildren(id, newTileIndex, currentTileNew, level);
        }
        else
        {
            // the image is new, add it to the existing tiles
            const GPSImageInfo currentImageInfo = GPSImageInfo::fromIdCoordinatesRatingDateTime(id, newCoordinates, newImageInfo.rating(), newImageInfo.dateTime());
            d->imagesHash.insert(id, currentImageInfo);

            const KGeoMap::TileIndex newMarkerTileIndex = KGeoMap::TileIndex::fromCoordinates(currentImageInfo.coordinates, KGeoMap::TileIndex::MaxLevel);

            addMarkerToTileAndChildren(id, newMarkerTileIndex, static_cast<MyTile*>(rootTile()), 0);
        }
    }

    emit(signalTilesOrSelectionChanged());
}

/**
 * @brief Receives notifications from the album model about new items
 */
void GPSMarkerTiler::slotNewModelData(const QList<ImageInfo>& infoList)
{
    // We do not actually store the data from the model, we just want
    // to know that something was changed.
    /// @todo Also monitor removed, reset, etc. signals
    Q_UNUSED(infoList);

    emit(signalTilesOrSelectionChanged());
}

void GPSMarkerTiler::setRegionSelection(const KGeoMap::GeoCoordinates::Pair& sel)
{
    d->currentRegionSelection = sel;

    if (sel.first.hasCoordinates())
    {
        d->mapGlobalGroupState |= KGeoMap::KGeoMapRegionSelectedMask;
    }
    else
    {
        d->mapGlobalGroupState &= ~KGeoMap::KGeoMapRegionSelectedMask;
    }

    emit(signalTilesOrSelectionChanged());
}

void GPSMarkerTiler::removeCurrentRegionSelection()
{
    d->currentRegionSelection.first.clear();

    d->mapGlobalGroupState &= ~KGeoMap::KGeoMapRegionSelectedMask;

    emit(signalTilesOrSelectionChanged());
}

void GPSMarkerTiler::onIndicesClicked(const ClickInfo& clickInfo)
{
    /// @todo Also handle the representative index

    QList<qlonglong> clickedImagesId;
    Q_FOREACH(const KGeoMap::TileIndex & tileIndex, clickInfo.tileIndicesList)
    {
        clickedImagesId << getTileMarkerIds(tileIndex);
    }

    int repImageId = -1;

    if (clickInfo.representativeIndex.canConvert<QPair<KGeoMap::TileIndex, int> >())
    {
        repImageId = clickInfo.representativeIndex.value<QPair<KGeoMap::TileIndex, int> >().second;
    }

    if (clickInfo.currentMouseMode == KGeoMap::MouseModeSelectThumbnail && d->selectionModel)
    {
        /**
         * @todo This does not work properly, because not all images in a tile
         * may be selectable because some of them are outside of the region selection
         */
        const bool doSelect = (clickInfo.groupSelectionState & KGeoMap::KGeoMapSelectedMask) != KGeoMap::KGeoMapSelectedAll;

        const QItemSelectionModel::SelectionFlags selectionFlags =
            (doSelect ? QItemSelectionModel::Select : QItemSelectionModel::Deselect)
            | QItemSelectionModel::Rows;

        for (int i = 0; i < clickedImagesId.count(); ++i)
        {
            const QModelIndex currentIndex = d->imageFilterModel->indexForImageId(clickedImagesId.at(i));

            if (d->selectionModel->isSelected(currentIndex) != doSelect)
            {
                d->selectionModel->select(currentIndex, selectionFlags);
            }
        }

        if (repImageId >= 0)
        {
            const QModelIndex repImageIndex = d->imageFilterModel->indexForImageId(repImageId);

            if (repImageIndex.isValid())
            {
                d->selectionModel->setCurrentIndex(repImageIndex, selectionFlags);
            }
        }
    }
    else if (clickInfo.currentMouseMode == KGeoMap::MouseModeFilter)
    {
        setPositiveFilterIsActive(true);
        emit signalModelFilteredImages(clickedImagesId);
    }
}

QList<qlonglong> GPSMarkerTiler::getTileMarkerIds(const KGeoMap::TileIndex& tileIndex)
{
    KGEOMAP_ASSERT(tileIndex.level() <= KGeoMap::TileIndex::MaxLevel);
    const MyTile* const myTile = static_cast<MyTile*>(getTile(tileIndex, true));

    if (!myTile)
    {
        return QList<qlonglong>();
    }

    return myTile->imagesId;
}

KGeoMap::KGeoMapGroupState GPSMarkerTiler::getGlobalGroupState()
{
    return d->mapGlobalGroupState;
}

KGeoMap::KGeoMapGroupState GPSMarkerTiler::getImageState(const qlonglong imageId)
{
    KGeoMap::KGeoMapGroupState imageState;

    // is the image inside the region selection?
    if (d->mapGlobalGroupState & KGeoMap::KGeoMapRegionSelectedMask)
    {
        const QModelIndex imageAlbumModelIndex = d->imageAlbumModel->indexForImageId(imageId);

        if (imageAlbumModelIndex.isValid())
        {
            imageState |= KGeoMap::KGeoMapRegionSelectedAll;
        }
        else
        {
            // not inside region selection, therefore
            // no other flags can apply
            return KGeoMap::KGeoMapRegionSelectedNone;
        }
    }

    // is the image positively filtered?
    if (d->mapGlobalGroupState & KGeoMap::KGeoMapFilteredPositiveMask)
    {
        const QModelIndex imageIndexInFilterModel = d->imageFilterModel->indexForImageId(imageId);

        if (imageIndexInFilterModel.isValid())
        {
            imageState |= KGeoMap::KGeoMapFilteredPositiveAll;

            // is the image selected?
            if (d->selectionModel->hasSelection())
            {
                if (d->selectionModel->isSelected(imageIndexInFilterModel))
                {
                    imageState |= KGeoMap::KGeoMapSelectedAll;
                }
            }
        }
        else
        {
            // the image is not positively filtered, therefore it can
            // not be selected
            return imageState;
        }
    }
    else
    {
        // is the image selected?
        if (d->selectionModel->hasSelection())
        {
            const QModelIndex imageIndexInFilterModel = d->imageFilterModel->indexForImageId(imageId);

            if (d->selectionModel->isSelected(imageIndexInFilterModel))
            {
                imageState |= KGeoMap::KGeoMapSelectedAll;
            }
        }
    }

    return imageState;
}

void GPSMarkerTiler::setPositiveFilterIsActive(const bool state)
{
    if (state)
    {
        d->mapGlobalGroupState |= KGeoMap::KGeoMapFilteredPositiveMask;
    }
    else
    {
        d->mapGlobalGroupState &= ~KGeoMap::KGeoMapFilteredPositiveMask;
    }

    /// @todo Somehow, a delay is necessary before emitting this signal - probably the order in which the filtering is propagated to other parts of digikam is wrong or just takes too long
    QTimer::singleShot(100, this, SIGNAL(signalTilesOrSelectionChanged()));
    //     emit(signalTilesOrSelectionChanged());
}

void GPSMarkerTiler::slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    /// @todo Buffer this information, update the tiles, etc.
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    emit(signalTilesOrSelectionChanged());
}

void GPSMarkerTiler::removeMarkerFromTileAndChildren(const qlonglong imageId, const KGeoMap::TileIndex& markerTileIndex, MyTile* const startTile, const int startTileLevel, MyTile* const parentTile)
{
    MyTile* currentParentTile = parentTile;
    MyTile* currentTile = startTile;

    for (int level = startTileLevel; level <= markerTileIndex.level(); ++level)
    {
        if (!currentTile->imagesId.contains(imageId))
        {
            break;
        }

        currentTile->imagesId.removeOne(imageId);

        if (currentTile->imagesId.isEmpty())
        {
            if (currentTile == rootTile())
            {
                break;
            }

            // this tile can be deleted
            tileDeleteChild(currentParentTile, currentTile);
            break;
        }

        currentParentTile = currentTile;
        currentTile = static_cast<MyTile*>(currentParentTile->getChild(markerTileIndex.at(level)));

        if (!currentTile)
        {
            break;
        }
    }
}

void GPSMarkerTiler::addMarkerToTileAndChildren(const qlonglong imageId, const KGeoMap::TileIndex& markerTileIndex, MyTile* const startTile, const int startTileLevel)
{
    MyTile* currentTile = startTile;

    for (int level = startTileLevel; level <= markerTileIndex.level(); ++level)
    {
        /// @todo This could be possible until all code paths are checked
        if (!currentTile->imagesId.contains(imageId))
        {
            currentTile->imagesId.append(imageId);
        }

        if (currentTile->childrenEmpty())
        {
            break;
        }

        MyTile* nextTile = static_cast<MyTile*>(currentTile->getChild(markerTileIndex.at(level)));

        if (!nextTile)
        {
            nextTile = static_cast<MyTile*>(tileNew());
            currentTile->addChild(markerTileIndex.at(level), nextTile);
        }

        currentTile = nextTile;
    }
}

} // namespace Digikam
