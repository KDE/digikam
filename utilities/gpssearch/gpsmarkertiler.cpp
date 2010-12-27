/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-20
 * Description : GPS search marker tiler
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
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

/// @todo Actually use this definition!
typedef QPair<KMap::AbstractMarkerTiler::TileIndex, int> MapPair;
Q_DECLARE_METATYPE(MapPair)

namespace Digikam
{

/**
 * @class GPSMarkerTiler
 *
 * @brief Marker model for storing data needed to display markers on the map. The data is retrieved from Digikam's database.
 */

class GPSMarkerTiler::GPSMarkerTilerPrivate
{
public:

    class EntryFromDatabase
    {
    public:
        qlonglong            id;
        int                  rating;
        QDateTime            creationDate;
        KMap::GeoCoordinates coordinate;
    };

    class InternalJobs
    {
    public:

        InternalJobs()
            : dataFromDatabase()
        {
            kioJob = 0;
            level = 0;
        }

        int                      level;
        KIO::Job*                kioJob;

        QList<EntryFromDatabase> dataFromDatabase;
    };

    GPSMarkerTilerPrivate()
        : mapImagesJob(0),
          dataFromDatabaseList(),
          activeState(true),
          imagesHash(),
          imageFilterModel(),
          imageAlbumModel(),
          currentMouseMode(KMap::MouseModePan),
          currentSelection(),
          selectionExists(false),
          existsFilterDatabaseBased(false),
          existsFilterModelBased(false),
          selectedImagesCoordinates(),
          modelDataProgStart(false)
    {
    }

    KIO::TransferJob*                      mapImagesJob;
    QList<QList<QVariant> >                dataFromDatabaseList;
    QList<InternalJobs>                    jobs;
    ThumbnailLoadThread*                   thumbnailLoadThread;
    QHash<QString, QVariant>               thumbnailMap;
    QList<QRectF>                          rectList;
    QList<int>                             rectLevel;
    bool                                   activeState;
    QHash<qlonglong, GPSImageInfo>         imagesHash;
    ImageFilterModel*                      imageFilterModel; // unused???
    ImageAlbumModel*                       imageAlbumModel;
    QItemSelectionModel*                   selectionModel;
    KMap::MouseMode                        currentMouseMode;
    KMap::GeoCoordinates::Pair             currentSelection;
    bool                                   selectionExists;
    bool                                   existsFilterDatabaseBased;
    bool                                   existsFilterModelBased;
    QHash<qlonglong, KMap::GeoCoordinates> selectedImagesCoordinates;
    bool                                   modelDataProgStart;
};

/**
 * @brief Constructor
 * @param parent Parent object
 */
GPSMarkerTiler::GPSMarkerTiler(QObject* const parent, ImageFilterModel* const imageFilterModel, QItemSelectionModel* const selectionModel)
    : KMap::AbstractMarkerTiler(parent), d(new GPSMarkerTilerPrivate())
{
    resetRootTile();

    d->thumbnailLoadThread = new ThumbnailLoadThread();
    d->imageFilterModel    = imageFilterModel;
    d->imageAlbumModel     = qobject_cast<ImageAlbumModel*>(imageFilterModel->sourceModel());
    d->selectionModel      = selectionModel;

    connect(d->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription&, const QPixmap&)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(const ImageChangeset&)),
            this, SLOT(slotImageChange(const ImageChangeset&)), Qt::QueuedConnection);

    connect(d->imageAlbumModel, SIGNAL(imageInfosAdded(const QList<ImageInfo>& )),
            this, SLOT(slotNewModelData(const QList<ImageInfo>& )));
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

GPSMarkerTiler::GPSImageInfo::~GPSImageInfo()
{
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
void GPSMarkerTiler::prepareTiles(const KMap::GeoCoordinates& upperLeft, const KMap::GeoCoordinates& lowerRight, int level)
{
    qreal lat1 = upperLeft.lat();
    qreal lng1 = upperLeft.lon();
    qreal lat2 = lowerRight.lat();
    qreal lng2 = lowerRight.lon();
    QRect requestedRect(lat1, lng1, lat2-lat1, lng2-lng1);

    for (int i=0; i<d->rectList.count(); ++i)
    {
        if (level != d->rectLevel.at(i))
        {
            continue;
        }

        qreal rectLat1, rectLng1, rectLat2, rectLng2;
        QRectF currentRect = d->rectList.at(i);
        currentRect.getCoords(&rectLat1, &rectLng1, &rectLat2, &rectLng2);

        //do nothing if this rectangle was already requested
        if (currentRect.contains(requestedRect))
        {
            return;
        }

        if (currentRect.contains(lat1,lng1))
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
                lat2 = rectLng1;
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

    QRectF newRect(lat1, lng1, lat2-lat1, lng2-lng1);
    d->rectList.append(newRect);
    d->rectLevel.append(level);

    kDebug() << "Listing" << lat1 << lat2 << lng1 << lng2;
    DatabaseUrl u = DatabaseUrl::fromAreaRange(lat1, lat2, lng1, lng2);
    KIO::Job* currentJob = ImageLister::startListJob(u);
    currentJob->addMetaData("wantDirectQuery", "false");

    GPSMarkerTilerPrivate::InternalJobs currentJobInfo;
    currentJobInfo.kioJob = currentJob;
    currentJobInfo.level  = level;

    d->jobs.append(currentJobInfo);

    connect(currentJob, SIGNAL(result(KJob*)),
            this, SLOT(slotMapImagesJobResult(KJob*)));

    connect(currentJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotMapImagesJobData(KIO::Job*, const QByteArray&)));
}

/**
 * @brief Returns a pointer to a tile.
 * @param tileIndex The index of a tile.
 * @param stopIfEmpty Determines whether child tiles are also created for empty tiles.
 */
KMap::AbstractMarkerTiler::Tile* GPSMarkerTiler::getTile(const KMap::AbstractMarkerTiler::TileIndex& tileIndex, const bool stopIfEmpty)
{
    //   if (isDirty())
    //   {
    //       regenerateTiles();
    //   }

    KMAP_ASSERT(tileIndex.level()<=TileIndex::MaxLevel);

    MyTile* tile = static_cast<MyTile*>(rootTile());

    for (int level = 0; level < tileIndex.indexCount(); ++level)
    {
        const int currentIndex = tileIndex.linearIndex(level);
        MyTile* childTile = 0;

        if (tile->children.isEmpty())
        {
            tile->prepareForChildren(KMap::QIntPair(TileIndex::Tiling, TileIndex::Tiling));

            for (int i=0; i<tile->imagesId.count(); ++i)
            {
                int currentImageId = tile->imagesId.at(i);
                GPSImageInfo currentImageInfo = d->imagesHash[currentImageId];
                const TileIndex markerTileIndex = TileIndex::fromCoordinates(currentImageInfo.coordinate, level);
                const int newTileIndex = markerTileIndex.toIntList().last();

                MyTile* newTile = static_cast<MyTile*>(tile->children.at(newTileIndex));

                if (newTile == 0)
                {
                    MyTile* newTile = static_cast<MyTile*>(tileNew());
                    newTile->imagesId.append(currentImageId);
                    tile->addChild(newTileIndex, newTile);
                }
                else
                {
                    bool found = false;

                    for (int j=0; j<newTile->imagesId.count(); ++j)
                    {
                        if (newTile->imagesId.at(j) == currentImageId)
                        {
                            found = true;
                        }

                        if (!found)
                        {
                            newTile->imagesId.append(currentImageId);
                        }
                    }
                }
            }

            //return 0;
        }

        childTile = static_cast<MyTile*>(tile->children.at(currentIndex));

        if (childTile==0)
        {
            if (stopIfEmpty)
            {
                // there will be no markers in this tile, therefore stop
                return 0;
            }

            //childTile = new KMap::AbstractMarkerTiler::Tile();
            //tile->addChild(currentIndex, childTile);
        }

        tile = childTile;
    }

    return tile;
}

int GPSMarkerTiler::getTileMarkerCount(const KMap::AbstractMarkerTiler::TileIndex& tileIndex)
{
    MyTile* tile = static_cast<MyTile*>(getTile(tileIndex));

    if (tile)
    {
        return tile->imagesId.count();
    }

    return 0;
}

int GPSMarkerTiler::getTileSelectedCount(const KMap::AbstractMarkerTiler::TileIndex& tileIndex)
{
    Q_UNUSED(tileIndex)

    return 0;
}

/**
 @brief This function finds the best representative marker from a group of markers. This is needed to display a thumbnail for a marker group.
 * @param indices A list containing markers.
 * @param sortKey Sets the criteria for selecting the representative thumbnail. When sortKey == 0 the most youngest thumbnail is chosen, when sortKey == 1 the most oldest thumbnail is chosen and when sortKey == 2 the thumbnail with the highest rating is chosen(if 2 thumbnails have the same rating, the youngest one is chosen).
 * @return Returns the index of the marker.
 */
QVariant GPSMarkerTiler::getTileRepresentativeMarker(const KMap::AbstractMarkerTiler::TileIndex& tileIndex, const int sortKey)
{
    MyTile*                                          tile = static_cast<MyTile*>(getTile(tileIndex, true));
    QPair<KMap::AbstractMarkerTiler::TileIndex, int> bestRep;
    QVariant                                         v;

    if (tile != NULL)
    {
        if (tile->imagesId.isEmpty())
        {
            return QVariant();
        }

        GPSImageInfo bestMarkerInfo = d->imagesHash.value(tile->imagesId.first());
        GPSImageInfo bestFilteredMarkerInfo, bestUnFilteredMarkerInfo;

        for (int i=0; i<tile->imagesId.count(); ++i)
        {
            GPSImageInfo currentMarkerInfo = d->imagesHash.value(tile->imagesId.at(i));

            if (sortKey == SortYoungestFirst)
            {
                if (d->existsFilterDatabaseBased || d->existsFilterModelBased || d->selectionExists)
                {
                    if (d->selectedImagesCoordinates.contains(currentMarkerInfo.id))
                    {
                        if (bestFilteredMarkerInfo.id == -2)
                        {
                            bestFilteredMarkerInfo = currentMarkerInfo;
                        }
                        else if (currentMarkerInfo.creationDate < bestFilteredMarkerInfo.creationDate)
                        {
                            bestFilteredMarkerInfo = currentMarkerInfo;
                        }
                    }
                    else
                    {
                        if ( bestUnFilteredMarkerInfo.id == -2)
                        {
                            bestUnFilteredMarkerInfo = currentMarkerInfo;
                        }
                        else if (currentMarkerInfo.creationDate < bestUnFilteredMarkerInfo.creationDate)
                        {
                            bestUnFilteredMarkerInfo = currentMarkerInfo;
                        }
                    }
                }
                else if (currentMarkerInfo.creationDate < bestMarkerInfo.creationDate)
                {
                    bestMarkerInfo = currentMarkerInfo;
                }
            }
            else if (sortKey == SortOldestFirst)
            {
                if (d->existsFilterDatabaseBased || d->existsFilterModelBased)
                {
                    if (d->selectedImagesCoordinates.contains(currentMarkerInfo.id))
                    {
                        if (bestFilteredMarkerInfo.id == -2)
                        {
                            bestFilteredMarkerInfo = currentMarkerInfo;
                        }
                        else if (currentMarkerInfo.creationDate > bestFilteredMarkerInfo.creationDate)
                        {
                            bestFilteredMarkerInfo = currentMarkerInfo;
                        }
                    }
                    else
                    {
                        if (bestUnFilteredMarkerInfo.id == -2)
                        {
                            bestUnFilteredMarkerInfo = currentMarkerInfo;
                        }
                        else if (currentMarkerInfo.creationDate > bestUnFilteredMarkerInfo.creationDate)
                        {
                            bestUnFilteredMarkerInfo = currentMarkerInfo;
                        }
                    }
                }
                else if (currentMarkerInfo.creationDate > bestMarkerInfo.creationDate)
                {
                    bestMarkerInfo = currentMarkerInfo;
                }
            }
            else
            {
                if (currentMarkerInfo.rating > bestMarkerInfo.rating)
                {
                    bestMarkerInfo = currentMarkerInfo;
                }
                else if (currentMarkerInfo.rating == bestMarkerInfo.rating)
                {
                    if (currentMarkerInfo.creationDate < bestMarkerInfo.creationDate)
                    {
                        bestMarkerInfo = currentMarkerInfo;
                    }
                }
            }
        }

        bestRep.first                              = tileIndex;

        if (d->existsFilterModelBased || d->existsFilterDatabaseBased || d->selectionExists)
        {
            if (bestFilteredMarkerInfo.id != -2)
            {
                bestRep.second = bestFilteredMarkerInfo.id;
            }
            else if (bestUnFilteredMarkerInfo.id != -2)
            {
                bestRep.second = bestUnFilteredMarkerInfo.id;
            }
            else
            {
                return QVariant();
            }
        }
        else
        {
            bestRep.second                             = bestMarkerInfo.id;
        }

        const QPair<TileIndex, int> returnedMarker = bestRep;

        v.setValue(bestRep);

        return v;
    }

    return QVariant();
}

/**
 * @brief This function finds the best representative marker from a group of markers. This is needed to display a thumbnail for a marker group.
 * @param indices A list containing markers.
 * @param sortKey Sets the criteria for selecting the representative thumbnail. When sortKey == 0 the most youngest thumbnail is chosen, when sortKey == 1 the most oldest thumbnail is chosen and when sortKey == 2 the thumbnail with the highest rating is chosen(if 2 thumbnails have the same rating, the youngest one is chosen).
 * @return Returns the index of the marker.
 */
QVariant GPSMarkerTiler::bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey)
{
    QVariant              v;
    QPair<TileIndex, int> bestRep;

    if (indices.count() == 0)
    {
        return QVariant();
    }

    QPair<TileIndex, int> currentIndex = indices.first().value<QPair<TileIndex, int> >();
    bestRep                            = currentIndex;
    GPSImageInfo bestMarkerInfo        = d->imagesHash.value(currentIndex.second);

    for (int i=1; i<indices.count(); ++i)
    {
        QPair<TileIndex, int> currentIndex = indices.at(i).value<QPair<TileIndex, int> >();

        MyTile* tile                   = static_cast<MyTile*>(getTile(currentIndex.first, true));
        GPSImageInfo currentMarkerInfo = d->imagesHash.value(currentIndex.second);
        Q_UNUSED(tile);

        if (sortKey == SortYoungestFirst)
        {
            if (currentMarkerInfo.creationDate < bestMarkerInfo.creationDate)
            {
                bestRep        = currentIndex;
                bestMarkerInfo = currentMarkerInfo;
            }
        }
        else if (sortKey == SortOldestFirst)
        {
            if (currentMarkerInfo.creationDate > bestMarkerInfo.creationDate)
            {
                bestRep        = currentIndex;
                bestMarkerInfo = currentMarkerInfo;
            }
        }
        else
        {
            if (currentMarkerInfo.rating > bestMarkerInfo.rating)
            {
                bestRep        = currentIndex;
                bestMarkerInfo = currentMarkerInfo;
            }
            else if (currentMarkerInfo.rating == bestMarkerInfo.rating)
            {
                if (currentMarkerInfo.creationDate < bestMarkerInfo.creationDate)
                {
                    bestRep        = currentIndex;
                    bestMarkerInfo = currentMarkerInfo;
                }
            }
        }
    }

    v.setValue(bestRep);
    return QVariant(v);
}

/**
 * @brief This function retrieves the thumbnail for an index.
 * @param index The marker's index.
 * @param size The size of the thumbnail.
 * @return If the thumbnail has been loaded in the ThumbnailLoadThread instance, it is returned. If not, a QPixmap is returned and ThumbnailLoadThread's signal named signalThumbnailLoaded is emited when the thumbnail becomes available.
 */
QPixmap GPSMarkerTiler::pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size)
{
    QPair<TileIndex, int> indexForPixmap = index.value<QPair<TileIndex,int> >();

    QPixmap thumbnail;
    ImageInfo info(indexForPixmap.second);
    QString path = info.filePath();
    d->thumbnailMap.insert(path, index);

    if (d->thumbnailLoadThread->find(path, thumbnail, qMax(size.width(), size.height())))
    {
        if ((d->existsFilterDatabaseBased || d->selectionExists) && !d->selectedImagesCoordinates.contains(info.id()))
        {
            QPixmap alphaPixmap(thumbnail.size());
            alphaPixmap.fill(QColor::fromRgb(0x80, 0x80, 0x80));
            thumbnail.setAlphaChannel(alphaPixmap);
        }

        return thumbnail;
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
    QPair<TileIndex, int> firstIndex = a.value<QPair<TileIndex,int> >();
    QPair<TileIndex, int> secondIndex = b.value<QPair<TileIndex,int> >();

    QList<int> aIndicesList = firstIndex.first.toIntList();
    QList<int> bIndicesList = secondIndex.first.toIntList();

    if (firstIndex.second == secondIndex.second && aIndicesList == bIndicesList)
    {
        return true;
    }

    return false;
}

KMap::KMapSelectionState GPSMarkerTiler::getTileSelectedState(const KMap::AbstractMarkerTiler::TileIndex& tileIndex)
{
    Q_UNUSED(tileIndex)

    return KMap::KMapSelectionState();
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

    QList<GPSMarkerTilerPrivate::EntryFromDatabase> newEntries;

    while (!ds.atEnd())
    {
        ImageListerRecord record(ImageListerRecord::ExtraValueFormat);
        ds >> record;

        GPSMarkerTilerPrivate::EntryFromDatabase entry;

        entry.id           = record.imageID;
        entry.rating       = record.rating;
        entry.creationDate = record.creationDate;

        if (!record.extraValues.count() < 2)
        {
            entry.coordinate.setLatLon(record.extraValues.first().toDouble(), record.extraValues.last().toDouble());
        }

        newEntries << entry;
    }

    for (int i=0; i<d->jobs.count(); ++i)
    {
        if (job == d->jobs.at(i).kioJob)
        {
            d->jobs[i].dataFromDatabase << newEntries;
        }
    }
}

/**
 * @brief Now, all the marker data has been retrieved from the database. Here, the markers are sorted into tiles.
 */
void GPSMarkerTiler::slotMapImagesJobResult(KJob* job)
{
    if (job->error())
    {
        kWarning() << "Failed to list images in selected area:" << job->errorString();
        return;
    }

    KIO::Job* currentJob = qobject_cast<KIO::Job*>(job);
    QList<GPSImageInfo> returnedImageInfo;

    int foundIndex = -1;

    for (int i=0; i<d->jobs.count(); ++i)
    {
        if (currentJob == d->jobs.at(i).kioJob)
        {
            foundIndex = i;

            foreach (const GPSMarkerTilerPrivate::EntryFromDatabase& entry, d->jobs.at(i).dataFromDatabase)
            {
                //TODO: What if image coordinates have changed?
                if (d->imagesHash.contains(entry.id))
                {
                    continue;
                }

                GPSImageInfo info;

                info.id               = entry.id;
                info.rating           = entry.rating;
                info.creationDate     = entry.creationDate;
                info.coordinate       = entry.coordinate;

                returnedImageInfo << info;
                d->imagesHash.insert(entry.id, info);
            }
        }
    }

    if (foundIndex != -1 && !returnedImageInfo.isEmpty())
    {
        int wantedLevel = d->jobs.at(foundIndex).level;

        for (int i=0; i<returnedImageInfo.count(); ++i)
        {
            MyTile* currentTile = static_cast<MyTile*>(rootTile());

            GPSImageInfo currentImageInfo = returnedImageInfo.at(i);

            for (int currentLevel = 0; currentLevel <= wantedLevel+1; ++currentLevel)
            {
                bool found = false;

                for (int counter = 0; counter < currentTile->imagesId.count(); ++counter)
                {
                    if (currentImageInfo.id == currentTile->imagesId.at(counter))
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    currentTile->imagesId.append(currentImageInfo.id);
                }

                if (currentTile->children.isEmpty())
                {
                    currentTile->prepareForChildren(KMap::QIntPair(TileIndex::Tiling,
                                                    TileIndex::Tiling));
                }

                const TileIndex markerTileIndex = TileIndex::fromCoordinates(currentImageInfo.coordinate, currentLevel);

                const int newTileIndex          = markerTileIndex.toIntList().last();

                MyTile* newTile = static_cast<MyTile*>(currentTile->children.at(newTileIndex));

                if (newTile == 0)
                {
                    newTile = static_cast<MyTile*>(tileNew());

                    if (currentLevel == wantedLevel+1)
                    {
                        newTile->imagesId.append(currentImageInfo.id);
                    }

                    currentTile->addChild(newTileIndex, newTile);
                    currentTile = newTile;
                }
                else
                {
                    currentTile = newTile;
                }
            }
        }

        d->jobs[foundIndex].kioJob->kill();
        d->jobs[foundIndex].kioJob = 0;
        d->jobs.removeAt(foundIndex);
    }
}

/**
 * @brief Because of a call to pixmapFromRepresentativeIndex, some thumbnails are not yet loaded at the time of requesting. When each thumbnail loads, this slot is called and emits a signal that announces the map that the thumbnail is available.
 */
void GPSMarkerTiler::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumbnail)
{
    QVariant index = d->thumbnailMap.value(loadingDescription.filePath);
    QPair<KMap::AbstractMarkerTiler::TileIndex, int> indexForPixmap =
        index.value<QPair<KMap::AbstractMarkerTiler::TileIndex, int> >();
    emit signalThumbnailAvailableForIndex(index, thumbnail);
}

/**
 * @brief Sets the map active/inactive
 * @param state New state of the map, true means active.
 */
void GPSMarkerTiler::setActive(const bool state)
{
    d->activeState = state;
}

KMap::AbstractMarkerTiler::Tile* GPSMarkerTiler::tileNew()
{
    return new MyTile();
}

void GPSMarkerTiler::tileDelete(KMap::AbstractMarkerTiler::Tile* const tile)
{
    if (tile)
    {
        delete static_cast<MyTile*>(tile);
    }
}

void GPSMarkerTiler::slotImageChange(const ImageChangeset& changeset)
{
    const DatabaseFields::Set changes = changeset.changes();
    const DatabaseFields::ImagePositions imagePositionChanges = changes;

    if (   ( changes & DatabaseFields::LatitudeNumber )
        || ( changes & DatabaseFields::LongitudeNumber )
        || ( changes & DatabaseFields::Altitude ) )
    {
        foreach (const qlonglong& id, changeset.ids())
        {
            ImageInfo newImageInfo(id);

            if (!newImageInfo.hasCoordinates())
                //TODO: Case when the image had coordinates before and now it doesn't have coordinates anymore
            {
                continue;
            }

            KMap::GeoCoordinates newCoordinates(newImageInfo.latitudeNumber(), newImageInfo.longitudeNumber());

            if (newImageInfo.hasAltitude())
            {
                newCoordinates.setAlt(newImageInfo.altitudeNumber());
            }

            if (d->imagesHash.contains(id))
            {
                GPSImageInfo oldInfo = d->imagesHash.value(id);
                KMap::GeoCoordinates oldCoordinates = oldInfo.coordinate;

                GPSImageInfo currentImageInfo = gpsData(id, newCoordinates, newImageInfo.rating(), newImageInfo.dateTime());
                d->imagesHash.insert(id, currentImageInfo);

                TileIndex oldTileIndex = TileIndex::fromCoordinates(oldCoordinates, TileIndex::MaxLevel);
                TileIndex newTileIndex = TileIndex::fromCoordinates(newCoordinates, TileIndex::MaxLevel);

                QList<int> oldTileIndexList = oldTileIndex.toIntList();
                QList<int> newTileIndexList = newTileIndex.toIntList();

                int separatorLevel = -1;

                for (int i=0; i<TileIndex::MaxLevel; ++i)
                {
                    if (oldTileIndexList.at(i) != newTileIndexList.at(i))
                    {
                        separatorLevel = i;
                        break;
                    }
                }

                if (separatorLevel != -1)
                {
                    MyTile* currentTileOld = static_cast<MyTile*>(rootTile());
                    MyTile* currentTileNew = currentTileOld;

                    for (int level=0; level<oldTileIndexList.count(); ++level)
                    {
                        const int tileIndex = oldTileIndexList.at(level);

                        if (currentTileOld->children.isEmpty())
                        {
                            break;
                        }

                        MyTile* childTileOld = static_cast<MyTile*>(currentTileOld->children.at(tileIndex));

                        if (childTileOld == 0)
                        {
                            break;
                        }

                        if (level<separatorLevel)
                        {
                            currentTileOld = childTileOld;
                            currentTileNew = currentTileOld;
                        }
                        else
                        {
                            int idIndex = childTileOld->imagesId.indexOf(id);

                            if (idIndex != -1)
                            {
                                childTileOld->imagesId.removeAt(idIndex);
                            }

                            if (childTileOld->imagesId.isEmpty())
                            {
                                int childTileListIndex = oldTileIndexList.at(level);

                                if (childTileListIndex != -1)
                                {
                                    currentTileOld->children[childTileListIndex] = 0;
                                }
                            }

                            currentTileOld = childTileOld;

                            //adds the new child tile to currentTileNew

                            bool found = currentTileNew->imagesId.contains(id);

                            if (!found)
                            {
                                currentTileNew->imagesId.append(currentImageInfo.id);
                            }

                            if (currentTileNew->children.isEmpty())
                            {
                                currentTileNew->prepareForChildren(
                                        KMap::QIntPair(TileIndex::Tiling, TileIndex::Tiling)
                                    );
                            }

                            const int newTileIndex = newTileIndexList.at(level);

                            MyTile* childTileNew = static_cast<MyTile*>(currentTileNew->children.at(newTileIndex));

                            if (childTileNew == 0)
                            {
                                childTileNew = static_cast<MyTile*>(tileNew());
                                childTileNew->imagesId.append(currentImageInfo.id);
                                currentTileNew->addChild(newTileIndex, childTileNew);
                                currentTileNew = childTileNew;
                            }
                            else
                            {
                                currentTileNew = childTileNew;
                            }
                        }
                    }
                }
            }
            else
            {
                // code that adds the image

                GPSImageInfo currentImageInfo = gpsData(id, newCoordinates, newImageInfo.rating(), newImageInfo.dateTime());
                d->imagesHash.insert(id, currentImageInfo);

                MyTile* currentTile = static_cast<MyTile*>(rootTile());

                for (int currentLevel = 0; currentLevel <= TileIndex::MaxLevel; ++currentLevel)
                {
                    const bool found = currentTile->imagesId.contains(currentImageInfo.id);

                    if (!found)
                    {
                        currentTile->imagesId.append(currentImageInfo.id);
                    }

                    if (currentTile->children.isEmpty())
                    {
                        currentTile->prepareForChildren(
                                KMap::QIntPair(TileIndex::Tiling, TileIndex::Tiling)
                            );
                    }

                    const TileIndex markerTileIndex = TileIndex::fromCoordinates(currentImageInfo.coordinate, currentLevel);

                    const int newTileIndex          = markerTileIndex.toIntList().last();

                    MyTile* newTile = static_cast<MyTile*>(currentTile->children.at(newTileIndex));

                    if (newTile == 0)
                    {
                        newTile = static_cast<MyTile*>(tileNew());
                        currentTile->addChild(newTileIndex, newTile);
                        currentTile = newTile;
                    }
                    else
                    {
                        currentTile = newTile;
                    }
                }
            }
        }
    }
}

GPSMarkerTiler::GPSImageInfo GPSMarkerTiler::gpsData(const qlonglong id, const KMap::GeoCoordinates& coordinates, const int rating, const QDateTime& creationDate)
{
    /// @todo This function could be a constructor of GPSImageInfo?
    GPSImageInfo currentImageInfo;
    currentImageInfo.id           = id;
    currentImageInfo.coordinate   = coordinates;
    currentImageInfo.rating       = rating;
    currentImageInfo.creationDate = creationDate;
    return currentImageInfo;
}

void GPSMarkerTiler::mouseModeChanged(const KMap::MouseMode currentMouseMode)
{
    d->currentMouseMode = currentMouseMode;
}

void GPSMarkerTiler::slotNewModelData(const QList<ImageInfo>& infos)
{
    if (!d->modelDataProgStart)
    {
        d->modelDataProgStart = true;
        //slotRemoveCurrentSelection();
        //emit signalRemoveCurrentSelection();
        emit signalClearImages();
        return;
    }

    if (d->activeState)
    {
        for (int i=0; i<infos.length(); ++i)
        {
            qlonglong currentImageId = infos.at(i).id();
            d->selectedImagesCoordinates.insert(currentImageId, d->imagesHash.value(currentImageId).coordinate);
        }
    }

    emit signalRefreshMap();
}

void GPSMarkerTiler::newSelectionFromMap(const KMap::GeoCoordinates::Pair& sel)
{
    d->currentSelection = sel;
    d->selectionExists  = true;
}

void GPSMarkerTiler::removeCurrentSelection()
{
    d->selectedImagesCoordinates.clear();

    if (d->selectionExists)
    {
        d->selectionExists           = false;
    }

    if (d->existsFilterDatabaseBased)
    {
        d->existsFilterDatabaseBased = false;
    }
}

void GPSMarkerTiler::onIndicesClicked(const KMap::AbstractMarkerTiler::TileIndex::List& tileIndicesList, const KMap::KMapSelectionState& groupSelectionState, KMap::MouseMode currentMouseMode)
{
    QList<qlonglong> clickedImagesId;

    for (int i=0; i<tileIndicesList.count(); ++i)
    {
        const KMap::AbstractMarkerTiler::TileIndex tileIndex = tileIndicesList.at(i);
        clickedImagesId << getTileMarkerIds(tileIndex);
    }

    if (currentMouseMode == KMap::MouseModeSelectThumbnail)
    {
        const bool doSelect = groupSelectionState != KMap::KMapSelectedAll;

        if (d->selectionModel)
        {
            for (int i=0; i<clickedImagesId.count(); ++i)
            {
                QModelIndex currentIndex = d->imageAlbumModel->indexForImageId(clickedImagesId.at(i));

                if (d->selectionModel->isSelected(currentIndex) != doSelect)
                {
                    d->selectionModel->select(
                        currentIndex,
                        (doSelect ? QItemSelectionModel::Select : QItemSelectionModel::Deselect)
                        | QItemSelectionModel::Rows
                    );
                }
            }
        }
    }
    else if (currentMouseMode == KMap::MouseModeFilter)
    {
        emit signalModelFilteredImages(clickedImagesId);
    }
}

void GPSMarkerTiler::newMapFilter(const KMap::FilterMode& newFilter)
{
    if (newFilter == KMap::DatabaseFilter)
    {
        d->existsFilterDatabaseBased = true;
    }
    else
    {
        d->existsFilterModelBased    = true;
    }
}

void GPSMarkerTiler::removeCurrentMapFilter(const KMap::FilterMode& removedFilter)
{
    if (removedFilter == KMap::DatabaseFilter)
    {
        d->existsFilterDatabaseBased = false;
    }
    else
    {
        d->existsFilterModelBased    = false;
    }
}

QList<qlonglong> GPSMarkerTiler::getTileMarkerIds(const KMap::AbstractMarkerTiler::TileIndex& tileIndex)
{
    KMAP_ASSERT(tileIndex.level() <= KMap::AbstractMarkerTiler::TileIndex::MaxLevel);
    const MyTile* const myTile = static_cast<MyTile*>(getTile(tileIndex, true));

    if (!myTile)
    {
        return QList<qlonglong>();
    }

    return myTile->imagesId;
}

} // namespace Digikam
