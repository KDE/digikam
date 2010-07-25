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

#include <QPersistentModelIndex>
#include <QByteArray>
#include <QMetaType>

// KDE includes

#include <kio/global.h>
#include <kio/job.h>

// libkmap includes

#include <libkmap/kmap.h>
#include <libkmap/abstractmarkertiler.h>

namespace Digikam
{

class GPSMarkerTiler : public KMapIface::AbstractMarkerTiler
{
    Q_OBJECT

public:

    GPSMarkerTiler(QObject* const parent = 0);
    ~GPSMarkerTiler();

    virtual bool isItemModelBased() const;
    virtual QItemSelectionModel* getSelectionModel() const;
    virtual QAbstractItemModel* getModel() const;
    virtual QList<QPersistentModelIndex> getTileMarkerIndices(const KMapIface::AbstractMarkerTiler::TileIndex& tileIndex);

    virtual void prepareTiles(const KMapIface::WMWGeoCoordinate& upperLeft,const KMapIface::WMWGeoCoordinate& lowerRight, int level);
    virtual void regenerateTiles();
    virtual KMapIface::AbstractMarkerTiler::Tile* getTile(const KMapIface::AbstractMarkerTiler::TileIndex& tileIndex, const bool stopIfEmpty = false);
    virtual int getTileMarkerCount(const KMapIface::AbstractMarkerTiler::TileIndex& tileIndex);
    virtual int getTileSelectedCount(const KMapIface::AbstractMarkerTiler::TileIndex& tileIndex);


    virtual QVariant getTileRepresentativeMarker(const KMapIface::AbstractMarkerTiler::TileIndex& tileIndex, const int sortKey);
    virtual QVariant bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey);
    virtual QPixmap pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size);
    virtual bool indicesEqual(const QVariant& a, const QVariant& b) const;
    virtual KMapIface::WMWSelectionState getTileSelectedState(const KMapIface::AbstractMarkerTiler::TileIndex& tileIndex);

    void secondTestDatabase(qreal, qreal, qreal, qreal);

private Q_SLOTS:

    void slotMapImagesJobResult(KJob* job);
    void slotMapImagesJobData(KIO::Job* job, const QByteArray& data);

private:

    class GPSMarkerTilerPrivate;
    GPSMarkerTilerPrivate* const d;
};

} // namespace Digikam

typedef QPair<KMapIface::AbstractMarkerTiler::TileIndex,int> MapPair;
Q_DECLARE_METATYPE(MapPair);

#endif //GPSMARKERTILER_H
