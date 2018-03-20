/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-05
 * Description : Merges tiles into groups
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#include "tilegrouper.h"

// C++ includes

#include <cmath>

// local includes

#include "abstractmarkertiler.h"
#include "mapbackend.h"
#include "digikam_debug.h"

namespace Digikam
{

class TileGrouper::Private
{
public:

    Private()
        : clustersDirty(true),
          currentBackend(0)
    {
    }

    bool        clustersDirty;
    MapBackend* currentBackend;
};

TileGrouper::TileGrouper(const QExplicitlySharedDataPointer<GeoIfaceSharedData>& sharedData,
                         QObject* const parent)
    : QObject(parent),
      d(new Private),
      s(sharedData)
{
    qRegisterMetaType<QVector<int>>("QVector<int>");
}

TileGrouper::~TileGrouper()
{
}

void TileGrouper::setClustersDirty()
{
    d->clustersDirty = true;
}

bool TileGrouper::getClustersDirty() const
{
    return d->clustersDirty;
}

void TileGrouper::setCurrentBackend(MapBackend* const backend)
{
    d->currentBackend = backend;
}

bool TileGrouper::currentBackendReady()
{
    if (!d->currentBackend)
    {
        return false;
    }

    return d->currentBackend->isReady();
}

void TileGrouper::updateClusters()
{
    if (!s->markerModel)
    {
        return;
    }

    if (s->haveMovingCluster)
    {
        // do not re-cluster while a cluster is being moved
        return;
    }

    if (!currentBackendReady())
    {
        return;
    }

    if (!d->clustersDirty)
    {
       return;
    }

    d->clustersDirty = false;

    // constants for clusters
    const int ClusterRadius          = s->showThumbnails ? s->thumbnailGroupingRadius : s->markerGroupingRadius;
//    const QSize ClusterDefaultSize   = QSize(2*ClusterRadius, 2*ClusterRadius);
    const int ClusterGridSizeScreen  = 4*ClusterRadius;
//    const QSize ClusterMaxPixmapSize = QSize(ClusterGridSizeScreen, ClusterGridSizeScreen);

//     qCDebug(DIGIKAM_GEOIFACE_LOG)<<"updateClusters starting...";

    s->clusterList.clear();

    const int markerLevel                                   = d->currentBackend->getMarkerModelLevel();
    QList<QPair<GeoCoordinates, GeoCoordinates> > mapBounds = d->currentBackend->getNormalizedBounds();

//     // debug output for tile level diagnostics:
//     QIntList tile1;
//     tile1<<520;
//     QIntList tile2 = tile1;
//     for (int i = 1; i <= s->markerModel->maxLevel()-1; ++i)
//     {
//         tile2 = tile1;
//         tile2<<0;
//         tile1<<1;
//         const GeoCoordinates tile1Coordinate = s->markerModel->tileIndexToCoordinate(tile1);
//         const GeoCoordinates tile2Coordinate = s->markerModel->tileIndexToCoordinate(tile2);
//         QPoint tile1Point, tile2Point;
//         d->currentBackend->screenCoordinates(tile1Coordinate, &tile1Point);
//         d->currentBackend->screenCoordinates(tile2Coordinate, &tile2Point);
//         qCDebug(DIGIKAM_GEOIFACE_LOG)<<i<<tile1Point<<tile2Point<<(tile1Point-tile2Point);
//     }

    const int gridSize   = ClusterGridSizeScreen;
    const QSize mapSize  = d->currentBackend->mapSize();
    const int gridWidth  = mapSize.width();
    const int gridHeight = mapSize.height();
    QVector<QList<TileIndex> > pixelNonEmptyTileIndexGrid(gridWidth*gridHeight, QList<TileIndex>());
    QVector<int> pixelCountGrid(gridWidth*gridHeight, 0);
    QList<QPair<QPoint, QPair<int, QList<TileIndex> > > > leftOverList;

    /// @todo Iterate only over the visible part of the map
    int debugCountNonEmptyTiles = 0;
    int debugTilesSearched      = 0;

    /// @todo Review this
    for(int i = 0 ; i < mapBounds.count() ; ++i)
    {
        s->markerModel->prepareTiles(mapBounds.at(i).first, mapBounds.at(i).second, markerLevel);
    }

    for (AbstractMarkerTiler::NonEmptyIterator tileIterator(s->markerModel, markerLevel, mapBounds) ;
         !tileIterator.atEnd() ; tileIterator.nextIndex())
    {
        const TileIndex tileIndex           = tileIterator.currentIndex();

        // find out where the tile is on the map:
        const GeoCoordinates tileCoordinate = tileIndex.toCoordinates();
        debugTilesSearched++;
        QPoint tilePoint;

        if (!d->currentBackend->screenCoordinates(tileCoordinate, &tilePoint))
        {
            continue;
        }

        // make sure we are in the grid (in case there are rounding errors somewhere in the backend
        if ((tilePoint.x() < 0) || (tilePoint.y() < 0) || (tilePoint.x() >= gridWidth) || (tilePoint.y() >= gridHeight))
            continue;

        debugCountNonEmptyTiles++;
        const int linearIndex        = tilePoint.x() + tilePoint.y()*gridWidth;
        pixelNonEmptyTileIndexGrid[linearIndex] << tileIndex;
        pixelCountGrid[linearIndex] += s->markerModel->getTileMarkerCount(tileIndex);

//         qCDebug(DIGIKAM_GEOIFACE_LOG) << QString::fromLatin1("pixel at: %1, %2 (%3): %4 markers").arg(tilePoint.x()).arg(tilePoint.y()).arg(linearIndex).arg(pixelCountGrid[linearIndex]);
    }

    /// @todo Cleanup this list every ... iterations in the next loop, too
    QIntList nonEmptyPixelIndices;

    for (int i = 0 ; i < gridWidth*gridHeight ; ++i)
    {
        if (pixelCountGrid.at(i) > 0)
            nonEmptyPixelIndices << i;
    }

    // re-add the markers to clusters:
//     int lastTooCloseClusterIndex = 0;
    Q_FOREVER
    {
        // here we store candidates for clusters:
        int markerMax             = 0;
        int markerX               = 0;
        int markerY               = 0;
        int pixelGridMetaIndexMax = 0;

        for (int pixelGridMetaIndex = 0; pixelGridMetaIndex < nonEmptyPixelIndices.size(); ++pixelGridMetaIndex)
        {
            const int index = nonEmptyPixelIndices.at(pixelGridMetaIndex);

            if (index < 0)
                continue;

            if (pixelCountGrid.at(index) == 0)
            {
                /// @todo Also remove this entry from the list to speed up the loop!
                nonEmptyPixelIndices[pixelGridMetaIndex] = -1;
                continue;
            }

            if (pixelCountGrid.at(index) > markerMax)
            {
                // calculate x,y from the linear index:
                const int x = index % gridWidth;
                const int y = (index-x)/gridWidth;
                const QPoint markerPosition(x, y);

                // only use this as a candidate for a cluster if it is not too close to another cluster:
                bool tooClose = false;

                /// @todo Check the cluster that was a problem last time first:
//                 if (lastTooCloseClusterIndex<s->clusterList.size())
//                 {
//                     tooClose = QPointSquareDistance(s->clusterList.at(lastTooCloseClusterIndex).pixelPos, markerPosition) < pow(ClusterGridSizeScreen/2, 2);
//                 }

                // now check all other clusters:
                for (int i = 0 ; (!tooClose) && (i < s->clusterList.size()) ; ++i)
                {
                    if (i == index)
                        continue;

                    tooClose = QPointSquareDistance(s->clusterList.at(i).pixelPos, markerPosition) < pow(ClusterGridSizeScreen/2, 2);
//                     if (tooClose)
//                         lastTooCloseClusterIndex = i;
                }

                if (tooClose)
                {
                    // move markers into leftover list
                    leftOverList << QPair<QPoint, QPair<int, QList<TileIndex> > >(QPoint(x,y),
                                    QPair<int, QList<TileIndex> >(pixelCountGrid.at(index), pixelNonEmptyTileIndexGrid.at(index)));
                    pixelCountGrid[index] = 0;
                    pixelNonEmptyTileIndexGrid[index].clear();
                    nonEmptyPixelIndices[pixelGridMetaIndex] = -1;
                }
                else
                {
                    markerMax=pixelCountGrid.at(index);
                    markerX=x;
                    markerY=y;
                    pixelGridMetaIndexMax = pixelGridMetaIndex;
                }
            }
        }

        if (markerMax == 0)
            break;

        GeoCoordinates clusterCoordinates = pixelNonEmptyTileIndexGrid.at(markerX+markerY*gridWidth).first().toCoordinates();
        GeoIfaceCluster cluster;
        cluster.coordinates               = clusterCoordinates;
        cluster.pixelPos                  = QPoint(markerX, markerY);
        cluster.tileIndicesList           = pixelNonEmptyTileIndexGrid.at(markerX+markerY*gridWidth);
        cluster.markerCount               = pixelCountGrid.at(markerX+markerY*gridWidth);

        // mark the pixel as done:
        pixelCountGrid[markerX+markerY*gridWidth]   = 0;
        pixelNonEmptyTileIndexGrid[markerX+markerY*gridWidth].clear();
        nonEmptyPixelIndices[pixelGridMetaIndexMax] = -1;

        // absorb all markers around it:
        // Now we only remove the markers from the pixelgrid. They will be cleared from the
        // pixelGridIndices in the loop above
        // make sure we do not go over the grid boundaries:
        const int eatRadius = gridSize/4;
        const int xStart    = qMax( (markerX-eatRadius), 0);
        const int yStart    = qMax( (markerY-eatRadius), 0);
        const int xEnd      = qMin( (markerX+eatRadius), gridWidth-1);
        const int yEnd      = qMin( (markerY+eatRadius), gridHeight-1);

        for (int indexX = xStart ; indexX <= xEnd ; ++indexX)
        {
            for (int indexY = yStart ; indexY <= yEnd ; ++indexY)
            {
                const int index       = indexX + indexY*gridWidth;
                cluster.tileIndicesList << pixelNonEmptyTileIndexGrid.at(index);
                pixelNonEmptyTileIndexGrid[index].clear();
                cluster.markerCount  += pixelCountGrid.at(index);
                pixelCountGrid[index] = 0;
            }
        }

        qCDebug(DIGIKAM_GEOIFACE_LOG) << QString::fromLatin1("created cluster %1: %2 tiles")
                                         .arg(s->clusterList.size())
                                         .arg(cluster.tileIndicesList.count());

        s->clusterList << cluster;
    }

    // now move all leftover markers into clusters:
    for (QList<QPair<QPoint, QPair<int, QList<TileIndex> > > >::const_iterator it = leftOverList.constBegin();
         it != leftOverList.constEnd() ; ++it)
    {
        const QPoint markerPosition = it->first;

        // find the closest cluster:
        int closestSquareDistance   = 0;
        int closestIndex            = -1;

        for (int i=0; i<s->clusterList.size(); ++i)
        {
            const int squareDistance = QPointSquareDistance(s->clusterList.at(i).pixelPos, markerPosition);

            if ((closestIndex < 0) || (squareDistance < closestSquareDistance))
            {
                closestSquareDistance = squareDistance;
                closestIndex          = i;
            }
        }

        if (closestIndex >= 0)
        {
            s->clusterList[closestIndex].markerCount += it->second.first;
            s->clusterList[closestIndex].tileIndicesList << it->second.second;
        }
    }

    // determine the selected states of the clusters:
    for (int i = 0 ; i < s->clusterList.count() ; ++i)
    {
        GeoIfaceCluster& cluster = s->clusterList[i];
        int clusterSelectedCount = 0;
        GroupStateComputer clusterStateComputer;

        for (int iTile = 0 ; (iTile < cluster.tileIndicesList.count()) ; ++iTile)
        {
            const TileIndex tileIndex              = cluster.tileIndicesList.at(iTile);
            const GeoGroupState tileGroupState = s->markerModel->getTileGroupState(tileIndex);
            clusterStateComputer.addState(tileGroupState);
            clusterSelectedCount                  += s->markerModel->getTileSelectedCount(tileIndex);
        }

        cluster.markerSelectedCount = clusterSelectedCount;
        cluster.groupState          = clusterStateComputer.getState();
    }

//     qCDebug(DIGIKAM_GEOIFACE_LOG) << s->clusterList.size();
    qCDebug(DIGIKAM_GEOIFACE_LOG) << QString::fromLatin1("level %1: %2 non empty tiles sorted into %3 clusters (%4 searched)")
                                     .arg(markerLevel)
                                     .arg(debugCountNonEmptyTiles)
                                     .arg(s->clusterList.count())
                                     .arg(debugTilesSearched);

    d->currentBackend->updateClusters();
}

} // namespace Digikam
