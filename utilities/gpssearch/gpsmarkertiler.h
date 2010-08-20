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

#ifndef GPSMARKERTILER_H
#define GPSMARKERTILER_H

// Qt includes

#include <QByteArray>
#include <QMetaType>

// KDE includes

#include <kio/global.h>
#include <kio/job.h>

// libkmap includes

#include <libkmap/abstractmarkertiler.h>

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

enum SortOptions {
    
    SortYoungestFirst = 0,
    SortOldestFirst = 1,
    SortRating = 2
};

class GPSMarkerTiler : public KMap::AbstractMarkerTiler
{
    Q_OBJECT

public:

    class MyTile : public Tile
    {
    public:
        MyTile()
        : Tile(),
          imagesFromTileInfo()
        {
        }

        class ImageFromTileInfo
        {
        public:
            ImageFromTileInfo()
            :id(-2),
             url(),
             coordinate(),
             rating(),
             creationDate()
            {
            }

            ~ImageFromTileInfo()
            {
            }

            int                 id;
            KUrl                url;
            KMap::GeoCoordinates    coordinate;
            int                 rating;
            QDateTime           creationDate;
        };

        QList<ImageFromTileInfo>     imagesFromTileInfo;
    };

    GPSMarkerTiler(QObject* const parent = 0);
    ~GPSMarkerTiler();

    virtual Tile* tileNew();
    virtual void tileDelete(Tile* const tile);
    virtual void prepareTiles(const KMap::GeoCoordinates& upperLeft,const KMap::GeoCoordinates& lowerRight, int level);
    virtual void regenerateTiles();
    virtual KMap::AbstractMarkerTiler::Tile* getTile(const KMap::AbstractMarkerTiler::TileIndex& tileIndex, const bool stopIfEmpty = false);
    virtual int getTileMarkerCount(const KMap::AbstractMarkerTiler::TileIndex& tileIndex);
    virtual int getTileSelectedCount(const KMap::AbstractMarkerTiler::TileIndex& tileIndex);


    virtual QVariant getTileRepresentativeMarker(const KMap::AbstractMarkerTiler::TileIndex& tileIndex, const int sortKey);
    virtual QVariant bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey);
    virtual QPixmap pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size);
    virtual bool indicesEqual(const QVariant& a, const QVariant& b) const;
    virtual KMap::WMWSelectionState getTileSelectedState(const KMap::AbstractMarkerTiler::TileIndex& tileIndex);

    virtual void setActive(const bool state);

private Q_SLOTS:

    void slotMapImagesJobResult(KJob* job);
    void slotMapImagesJobData(KIO::Job* job, const QByteArray& data);
    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

private:

    class GPSMarkerTilerPrivate;
    GPSMarkerTilerPrivate* const d;
};

} // namespace Digikam

typedef QPair<KMap::AbstractMarkerTiler::TileIndex,int> MapPair;
Q_DECLARE_METATYPE(MapPair);

#endif //GPSMARKERTILER_H
