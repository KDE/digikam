/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-20
 * Description : GPS searck marker tiler
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

namespace Digikam
{

class EntryFromDatabase
{
    public:
        qlonglong id;
        int       rating;
        KMapIface::WMWGeoCoordinate coordinate;
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

    KIO::Job* kioJob;
    int level;


    QList<EntryFromDatabase> dataFromDatabase;
};

/**
 * @class GPSMarkerTiler
 * 
 * @brief Marker model for storing data needed to display markers on the map. The data is retrieved from Digikam's database.
 */

class GPSMarkerTiler::GPSMarkerTilerPrivate
{
public:

    GPSMarkerTilerPrivate()
        : returnedImageInfo(),
          levelList(),
          jobsList(),
          dataFromDatabaseList(),
          activeState(true)
    {
        mapImagesJob = 0;
    }

    KIO::TransferJob*                                              mapImagesJob;
    QList<KMapIface::AbstractMarkerTiler::Tile::ImageFromTileInfo> returnedImageInfo;
    QList<int>                                                     levelList;
    QList<KIO::Job*>                                               jobsList;
    QList<QList<QVariant> >                                        dataFromDatabaseList;
    QList<InternalJobs>                                            jobs;
    ThumbnailLoadThread*                                           thumbnailLoadThread;
    QMap<QString, QVariant>                                        thumbnailMap;
    QList<QRectF>                                                  rectList;
    QList<int>                                                     rectLevel;
    bool                                                           activeState; 
};

/**
 * Constructor
 * @param parent Parent object
 */
GPSMarkerTiler::GPSMarkerTiler(QObject* const parent)
              : KMapIface::AbstractMarkerTiler(parent), d(new GPSMarkerTilerPrivate())
{
    d->thumbnailLoadThread = new ThumbnailLoadThread();

    connect(d->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription&, const QPixmap&)));
}

/**
 * Destructor
 */
GPSMarkerTiler::~GPSMarkerTiler()
{
    delete d;
}

/**
 * This function returns false because it is not based on a model, but on database."
 */
bool GPSMarkerTiler::isItemModelBased() const
{
    return false;
}

QItemSelectionModel* GPSMarkerTiler::getSelectionModel() const
{
    return 0;
}

QAbstractItemModel* GPSMarkerTiler::getModel() const
{
    return 0;
}


QList<QPersistentModelIndex> GPSMarkerTiler::getTileMarkerIndices(const KMapIface::AbstractMarkerTiler::TileIndex& /*tileIndex*/)
{
    return QList<QPersistentModelIndex>();
}

void GPSMarkerTiler::regenerateTiles()
{
}

/**
 * @brief This function calls the database for the images found inside a rectangle defined by upperLeft and lowerRight points. The images 
 * are returned from database in batches.
 * @param upperLeft The North-West point.
 * @param lowerRight The South-East point.
 * @param level The current zoom level of the map
 */
void GPSMarkerTiler::prepareTiles(const KMapIface::WMWGeoCoordinate& upperLeft,const KMapIface::WMWGeoCoordinate& lowerRight, int level)
{

    qreal lat1 = upperLeft.lat();
    qreal lng1 = upperLeft.lon();
    qreal lat2 = lowerRight.lat();
    qreal lng2 = lowerRight.lon();
    QRect requestedRect(lat1, lng1, lat2-lat1, lng2-lng1);

    for(int i=0; i<d->rectList.count(); ++i)
    {
        if(level != d->rectLevel.at(i))
            continue;

        qreal rectLat1, rectLng1, rectLat2, rectLng2;
        QRectF currentRect = d->rectList.at(i);
        currentRect.getCoords(&rectLat1, &rectLng1, &rectLat2, &rectLng2);
 
        //do nothing if this rectangle was already requested
        if(currentRect.contains(requestedRect))
            return;

        if(currentRect.contains(lat1,lng1))
        {
            if(currentRect.contains(lat2, lng1))
            {
                lng1 = rectLng2;
                break;
            }
        }
        else if(currentRect.contains(lat2, lng1))
        {
            if(currentRect.contains(lat2, lng2))
            {
                lat2 = rectLng1;
                break;
            }
        }
        else if(currentRect.contains(lat2, lng2))
        {
            if(currentRect.contains(lat1, lng2))
            {
                lng2 = rectLng1;
                break;
            }
        }
        else if(currentRect.contains(lat1, lng2))
        {
            if(currentRect.contains(lat1, lng1))
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

    InternalJobs currentJobInfo;
    currentJobInfo.kioJob           = currentJob;
    currentJobInfo.level            = level;

    d->jobs.append(currentJobInfo);

    connect(currentJob, SIGNAL(result(KJob*)),
            this, SLOT(slotMapImagesJobResult(KJob*)));

    connect(currentJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotMapImagesJobData(KIO::Job*, const QByteArray&)));
}

/**
 * @brief Returns information about a tile.
 * @param tileIndex The index of a tile.
 * @param stopIfEmpty The search of a tile is done recursively. If the search founds an empty tile, checks stopIfEmpty. If stopIfEmpty is true, the search stops.
 */
KMapIface::AbstractMarkerTiler::Tile* GPSMarkerTiler::getTile(const KMapIface::AbstractMarkerTiler::TileIndex& tileIndex, const bool stopIfEmpty)
{
//   if (isDirty())
//   {
//       regenerateTiles();
//   }

    KMAP_ASSERT(tileIndex.level()<=KMapIface::AbstractMarkerTiler::TileIndex::MaxLevel);

    KMapIface::AbstractMarkerTiler::Tile* tile = KMapIface::AbstractMarkerTiler::rootTile();
    for (int level = 0; level < tileIndex.indexCount(); ++level)
    {
        const int currentIndex = tileIndex.linearIndex(level);

        KMapIface::AbstractMarkerTiler::Tile* childTile = 0;
        if (tile->children.isEmpty())
        {
            tile->prepareForChildren(KMapIface::QIntPair(KMapIface::AbstractMarkerTiler::TileIndex::Tiling, KMapIface::AbstractMarkerTiler::TileIndex::Tiling));

            for(int i=0; i<tile->imagesFromTileInfo.count(); ++i)
            {
                KMapIface::AbstractMarkerTiler::Tile::ImageFromTileInfo currentImageInfo = tile->imagesFromTileInfo.at(i); 
                const KMapIface::AbstractMarkerTiler::TileIndex markerTileIndex = KMapIface::AbstractMarkerTiler::TileIndex::fromCoordinates(currentImageInfo.coordinate, level);
                const int newTileIndex = markerTileIndex.toIntList().last();

                KMapIface::AbstractMarkerTiler::Tile* newTile = tile->children.at(newTileIndex);

                if(newTile == 0)
                {
                    KMapIface::AbstractMarkerTiler::Tile* newTile = new KMapIface::AbstractMarkerTiler::Tile();
                    newTile->imagesFromTileInfo.append(currentImageInfo);
                    tile->addChild(newTileIndex, newTile);
                }
                else
                {
                    bool found = false;
                    for(int j=0; j<newTile->imagesFromTileInfo.count(); ++j)
                    {
                        if(newTile->imagesFromTileInfo.at(j).id == currentImageInfo.id)
                            found = true;

                        if(!found)
                            newTile->imagesFromTileInfo.append(currentImageInfo);
                    }
                }
            }

            //return 0;
        }
        childTile = tile->children.at(currentIndex);

        if (childTile==0)
        {
            if (stopIfEmpty)
            {
                // there will be no markers in this tile, therefore stop
                return 0;
            }
            //childTile = new KMapIface::AbstractMarkerTiler::Tile();
            //tile->addChild(currentIndex, childTile);
        }
        tile = childTile;
    }
    return tile;
}

int GPSMarkerTiler::getTileMarkerCount(const KMapIface::AbstractMarkerTiler::TileIndex& tileIndex)
{
    KMapIface::AbstractMarkerTiler::Tile* tile = getTile(tileIndex);
    if(tile)
        return tile->imagesFromTileInfo.count();

    return 0;
}

int GPSMarkerTiler::getTileSelectedCount(const KMapIface::AbstractMarkerTiler::TileIndex& /*tileIndex*/)
{
    return 0;
}

QVariant GPSMarkerTiler::getTileRepresentativeMarker(const KMapIface::AbstractMarkerTiler::TileIndex& tileIndex, const int /*sortKey*/)
{
    //TODO: sort the markers using sortKey

    KMapIface::AbstractMarkerTiler::Tile* tile = getTile(tileIndex, true);
    QPair<KMapIface::AbstractMarkerTiler::TileIndex, int> bestRep;
    QVariant v;

    if(tile != NULL)
    {
        bestRep.first  = tileIndex;
        bestRep.second = tile->imagesFromTileInfo.first().id;
        const QPair<KMapIface::AbstractMarkerTiler::TileIndex, int> returnedMarker = bestRep;
        v.setValue(bestRep);
        return v;
    }
    return QVariant();
}

QVariant GPSMarkerTiler::bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int /*sortKey*/)
{
    //TODO: sort the markers using sortKey
    QVariant v;
    QPair<KMapIface::AbstractMarkerTiler::TileIndex, int> bestRep;
    int bestRating = -2;

    for(int i=0; i<indices.count(); ++i)
    {
        QPair<KMapIface::AbstractMarkerTiler::TileIndex, int> currentIndex = indices.at(i).value<QPair<KMapIface::AbstractMarkerTiler::TileIndex, int> >();

        KMapIface::AbstractMarkerTiler::Tile* tile = getTile(currentIndex.first, true);

        for(int j=0; j<tile->imagesFromTileInfo.count(); ++j)
        {
            if(tile->imagesFromTileInfo.at(j).id == currentIndex.second)
            {
                if(bestRating == -2)
                {
                    bestRep = currentIndex;
                    bestRating = tile->imagesFromTileInfo.at(j).rating;
                }
                else
                {
                    if(tile->imagesFromTileInfo.at(j).rating < bestRating)
                    {
                        bestRep = currentIndex;
                        bestRating = tile->imagesFromTileInfo.at(j).rating;
                    }
                }
                break;
            }
        }
    }

    v.setValue(bestRep);
    return QVariant(v);
}

QPixmap GPSMarkerTiler::pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size)
{
    QPair<KMapIface::AbstractMarkerTiler::TileIndex, int> indexForPixmap = index.value<QPair<KMapIface::AbstractMarkerTiler::TileIndex,int> >();

    QPixmap thumbnail;
    ImageInfo info(indexForPixmap.second);
    QString path = info.filePath();
    d->thumbnailMap.insert(path, index);

    if(d->thumbnailLoadThread->find(path, thumbnail, qMax(size.width(), size.height())))
    {
        return thumbnail;
    }
    else
    {
        return QPixmap();
    }
}

bool GPSMarkerTiler::indicesEqual(const QVariant& a, const QVariant& b) const
{
    QPair<KMapIface::AbstractMarkerTiler::TileIndex, int> firstIndex = a.value<QPair<KMapIface::AbstractMarkerTiler::TileIndex,int> >();
    QPair<KMapIface::AbstractMarkerTiler::TileIndex, int> secondIndex = b.value<QPair<KMapIface::AbstractMarkerTiler::TileIndex,int> >();

    QList<int> aIndicesList = firstIndex.first.toIntList();
    QList<int> bIndicesList = secondIndex.first.toIntList();

    if(firstIndex.second == secondIndex.second && aIndicesList == bIndicesList)   
        return true;

    return false;
}

KMapIface::WMWSelectionState GPSMarkerTiler::getTileSelectedState(const KMapIface::AbstractMarkerTiler::TileIndex& /*tileIndex*/)
{
    return KMapIface::WMWSelectionState();
}

void GPSMarkerTiler::slotMapImagesJobData(KIO::Job* job, const QByteArray& data)
{
    if(data.isEmpty())
    {
        return;
    }

    QByteArray di(data);
    QDataStream ds(&di, QIODevice::ReadOnly);

    QList<EntryFromDatabase> newEntries;
    while (!ds.atEnd())
    {
        ImageListerRecord record(ImageListerRecord::ExtraValueFormat);
        ds >> record;

        EntryFromDatabase entry;

        entry.id = record.imageID;
        entry.rating = record.rating;
        if (!record.extraValues.count() < 2)
            entry.coordinate.setLatLon(record.extraValues.first().toDouble(), record.extraValues.last().toDouble());

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

void GPSMarkerTiler::slotMapImagesJobResult(KJob* job)
{
    if(job->error())
    {
        kWarning()<<"Failed to list images in selected area:"<<job->errorString();
        return;
    }

    KIO::Job* currentJob = qobject_cast<KIO::Job*>(job);
    QList<KMapIface::AbstractMarkerTiler::Tile::ImageFromTileInfo> currentReturnedImageInfo;

    int foundIndex = -1;
    for(int i=0; i<d->jobs.count(); ++i)
    {
        if(currentJob == d->jobs.at(i).kioJob)
        {
            foundIndex = i;

            foreach (const EntryFromDatabase& entry, d->jobs.at(i).dataFromDatabase)
            {
                KMapIface::AbstractMarkerTiler::Tile::ImageFromTileInfo info;

                info.id               = entry.id;
                info.rating           = entry.rating;
                info.coordinate       = entry.coordinate;

                currentReturnedImageInfo << info;
            }
        }
    }

    if(foundIndex != -1)
    {

    int wantedLevel = d->jobs.at(foundIndex).level;
    QList<KMapIface::AbstractMarkerTiler::Tile::ImageFromTileInfo> resultedImages = currentReturnedImageInfo;
    currentReturnedImageInfo.clear();

    for(int i=0; i<resultedImages.count(); ++i)
    {
        KMapIface::AbstractMarkerTiler::Tile* currentTile = KMapIface::AbstractMarkerTiler::rootTile();
        //for now, info contains id,coordinates and rating
        KMapIface::AbstractMarkerTiler::Tile::ImageFromTileInfo currentImageInfo = resultedImages.at(i);

        for(int currentLevel = 0; currentLevel <= wantedLevel+1; ++currentLevel)
        {
            bool found = false;
            for(int counter = 0; counter < currentTile->imagesFromTileInfo.count(); ++counter)
                if(currentImageInfo.id == currentTile->imagesFromTileInfo.at(counter).id)
                    found = true;

            if(!found)
                currentTile->imagesFromTileInfo.append(currentImageInfo);

            if(currentTile->children.isEmpty())
                currentTile->prepareForChildren(KMapIface::QIntPair(KMapIface::AbstractMarkerTiler::TileIndex::Tiling, 
								    KMapIface::AbstractMarkerTiler::TileIndex::Tiling));

            const KMapIface::AbstractMarkerTiler::TileIndex markerTileIndex = 
                         KMapIface::AbstractMarkerTiler::TileIndex::fromCoordinates(currentImageInfo.coordinate, currentLevel);
            const int newTileIndex = markerTileIndex.toIntList().last();

            KMapIface::AbstractMarkerTiler::Tile* newTile = currentTile->children.at(newTileIndex);

            if(newTile == 0)
            {
                newTile = new KMapIface::AbstractMarkerTiler::Tile();

                if(currentLevel == wantedLevel+1)
                {
                    newTile->imagesFromTileInfo.append(currentImageInfo);
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

void GPSMarkerTiler::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumbnail)
{
    QVariant index = d->thumbnailMap.value(loadingDescription.filePath);
    QPair<KMapIface::AbstractMarkerTiler::TileIndex, int> indexForPixmap = 
                              index.value<QPair<KMapIface::AbstractMarkerTiler::TileIndex,int> >();    
    emit signalThumbnailAvailableForIndex(index, thumbnail);
}

void GPSMarkerTiler::setActive(const bool state)
{
    d->activeState = state;
}

} // namespace Digikam
