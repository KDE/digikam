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

// local includes

#include "imagelister.h"
#include "databaseaccess.h"
#include "albumdb.h"
#include "imageinfo.h"
#include "thumbnailloadthread.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"

namespace Digikam
{

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
    QList<QVariant> dataFromDatabase;
};

class GPSMarkerTiler::GPSMarkerTilerPrivate
{
public:

    GPSMarkerTilerPrivate()
        : returnedImageInfo(),
          levelList(),
          jobsList(),
          dataFromDatabaseList()
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
};

GPSMarkerTiler::GPSMarkerTiler(QObject* const parent)
              : KMapIface::AbstractMarkerTiler(parent), d(new GPSMarkerTilerPrivate())
{
    d->thumbnailLoadThread = new ThumbnailLoadThread();

 /*   ThumbnailDatabaseAccess thumbAccess;
    QHash<QString, int> filePathsHash = thumbAccess.db()->getFilePathsWithThumbnail();
    const QList<QString> filePaths = filePathsHash.keys();

    for(int i=0; i<filePaths.count(); i++)
    {
        d->thumbnailLoadThread->find(filePaths.at(i));
    }
 */
}

GPSMarkerTiler::~GPSMarkerTiler()
{
    delete d;
}

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

void GPSMarkerTiler::prepareTiles(const KMapIface::WMWGeoCoordinate& upperLeft,const KMapIface::WMWGeoCoordinate& lowerRight, int level)
{

    kDebug()<<"Started tiles prepairing";

    DatabaseUrl u = DatabaseUrl::mapImagesUrl();

    qreal lat1 = upperLeft.lat();
    qreal lng1 = upperLeft.lon();
    qreal lat2 = lowerRight.lat();
    qreal lng2 = lowerRight.lon();

    QByteArray baLat1 = QString("%1").arg(lat1).toAscii();
    QByteArray baLat2 = QString("%1").arg(lat2).toAscii();
    QByteArray baLng1 = QString("%1").arg(lng1).toAscii();
    QByteArray baLng2 = QString("%1").arg(lng2).toAscii();

    KIO::Job* currentJob = ImageLister::startListJob(u);

    currentJob->addMetaData("lat1", baLat1.constData());
    currentJob->addMetaData("lat2", baLat2.constData());
    currentJob->addMetaData("lng1", baLng1.constData());
    currentJob->addMetaData("lng2", baLng2.constData());
    currentJob->addMetaData("wantDirectQuery", "true");

    InternalJobs currentJobInfo;
    currentJobInfo.kioJob           = currentJob;
    QList<QVariant> currentList;
    currentJobInfo.dataFromDatabase = currentList;
    currentJobInfo.level            = level;

    d->jobs.append(currentJobInfo);

    connect(currentJob, SIGNAL(result(KJob*)),
            this, SLOT(slotMapImagesJobResult(KJob*)));

    connect(currentJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotMapImagesJobData(KIO::Job*, const QByteArray&)));
}

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
            //shoud be return tile; instead of return 0; ?
            return 0;
        }
        childTile = tile->children.at(currentIndex);

        if (childTile==0)
        {
            if (stopIfEmpty)
            {
                // there will be no markers in this tile, therefore stop
                return 0;
            }
            //childTile = new Tile();
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

int GPSMarkerTiler::getTileSelectedCount(const KMapIface::AbstractMarkerTiler::TileIndex& tileIndex)
{
    return 0;
}

QVariant GPSMarkerTiler::getTileRepresentativeMarker(const KMapIface::AbstractMarkerTiler::TileIndex& /*tileIndex*/, const int /*sortKey*/)
{
    //TODO: sort the markers using sortKey

    KMapIface::AbstractMarkerTiler::Tile* tile = getTile(tileIndex, true);
    QPair<KMapIface::AbstractMarkerTiler::TileIndex, int> bestRep;
    QVariant v;

    if(tile != NULL)
    {
        bestRep.first = tileIndex;
        bestRep.second = tile->imagesFromTileInfo.first().id;
        //int bestId = tile->imagesFromTileInfo.first().id;
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

    if(d->thumbnailLoadThread->find(path, thumbnail, qMax(size.width(), size.height())))
        return thumbnail;
    else
        return QPixmap();
}

bool GPSMarkerTiler::indicesEqual(const QVariant& /*a*/, const QVariant& /*b*/) const
{
    return false;
}

KMapIface::WMWSelectionState GPSMarkerTiler::getTileSelectedState(const KMapIface::AbstractMarkerTiler::TileIndex& /*tileIndex*/)
{
    return KMapIface::WMWSelectionState();
}

/*
void GPSMarkerTiler::secondTestDatabase(qreal lat1, qreal lat2, qreal lng1, qreal lng2)
{
    if(d->mapImagesJob)
    {
        d->mapImagesJob->kill();
        d->mapImagesJob = 0;
    }

    kDebug() << "We now make the test with wantDirectQuery=true";

    DatabaseUrl u = DatabaseUrl::mapImagesUrl();

    QByteArray baLat1 = QString("%1").arg(lat1).toAscii();
    QByteArray baLat2 = QString("%1").arg(lat2).toAscii();
    QByteArray baLng1 = QString("%1").arg(lng1).toAscii();
    QByteArray baLng2 = QString("%1").arg(lng2).toAscii();

    d->mapImagesJob = ImageLister::startListJob(u);
    d->mapImagesJob->addMetaData("lat1", baLat1.constData());
    d->mapImagesJob->addMetaData("lat2", baLat2.constData());
    d->mapImagesJob->addMetaData("lng1", baLng1.constData());
    d->mapImagesJob->addMetaData("lng2", baLng2.constData());
    d->mapImagesJob->addMetaData("wantDirectQuery", "true");

    connect(d->mapImagesJob, SIGNAL(result(KJob*)),
            this, SLOT(slotMapImagesJobResult(KJob*)));

    connect(d->mapImagesJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotMapImagesJobData(KIO::Job*, const QByteArray&)));
}

void GPSMarkerTiler::secondTestDatabase(int lat1, int lat2, int lng1, int lng2)
{
    if(d->mapImagesJob)
    {
        d->mapImagesJob->kill();
        d->mapImagesJob = 0;
    }

    kDebug()<<"We now make the test with wantDirectQuery=false";

    DatabaseUrl u = DatabaseUrl::fromAreaRange(lat1, lat2, lng1, lng2);
    d->mapImagesJob = ImageLister::startListJob(u);
    d->mapImagesJob->addMetaData("wantDirectQuery", "false");

    connect(d->mapImagesJob, SIGNAL(result(KJob*)),
            this, SLOT(slotMapImagesJobResult(KJob*)));

    connect(d->mapImagesJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotMapImagesJobData(KIO::Job*, const QByteArray&)));
}
*/

void GPSMarkerTiler::slotMapImagesJobData(KIO::Job* job, const QByteArray& data)
{
    if(data.isEmpty())
    {
        kDebug()<<"We Have Empty Data.";
        return;
    }

    QList<QVariant> currentData;
    QByteArray di(data);
    QDataStream ds(&di, QIODevice::ReadOnly);
    ds>>currentData;

    for(int i=0; i<d->jobs.count(); ++i)
    {
        if(job == d->jobs.at(i).kioJob)
        {
            d->jobs[i].dataFromDatabase.append(currentData);
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

            for(QList<QVariant>::const_iterator it = d->jobs.at(i).dataFromDatabase.constBegin(); it!= d->jobs.at(i).dataFromDatabase.constEnd();)
            {
                KMapIface::AbstractMarkerTiler::Tile::ImageFromTileInfo info;
                info.id               = (*it).toInt();
                ++it;
                info.rating           = (*it).toInt();
                ++it;
                qreal latitudeNumber  = (*it).toDouble();
                ++it;
                qreal longitudeNumber = (*it).toDouble();
                ++it;

                KMapIface::WMWGeoCoordinate coordinate;
                coordinate.setLatLon(latitudeNumber, longitudeNumber);
                info.coordinate       = coordinate;

                currentReturnedImageInfo<<info;
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

        for(int currentLevel = 0; currentLevel <= wantedLevel; ++currentLevel)
        {
            bool found = false;
            for(int counter = 0; counter < currentTile->imagesFromTileInfo.count(); ++counter)
                if(currentImageInfo.id == currentTile->imagesFromTileInfo.at(counter).id)
                    found = true;

            if(!found)
                currentTile->imagesFromTileInfo.append(currentImageInfo);


            if(currentTile->children.isEmpty())
                currentTile->prepareForChildren(KMapIface::QIntPair(KMapIface::AbstractMarkerTiler::TileIndex::Tiling, KMapIface::AbstractMarkerTiler::TileIndex::Tiling));

            const KMapIface::AbstractMarkerTiler::TileIndex markerTileIndex = KMapIface::AbstractMarkerTiler::TileIndex::fromCoordinates(currentImageInfo.coordinate, currentLevel);
            const int newTileIndex = markerTileIndex.toIntList().last();

            KMapIface::AbstractMarkerTiler::Tile* newTile = currentTile->children.at(newTileIndex);

            if(newTile == 0)
            {
                newTile = new KMapIface::AbstractMarkerTiler::Tile();
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

} // namespace Digikam
