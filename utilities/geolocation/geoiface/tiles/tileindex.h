/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : Tile index used in the tiling classes
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef TILE_INDEX_H
#define TILE_INDEX_H

// Qt includes

#include <QBitArray>
#include <QObject>
#include <QPoint>
#include <QDebug>

// Local includes

#include "geocoordinates.h"
#include "geoifacetypes.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT  TileIndex
{
public:

    enum Constants
    {
        MaxLevel       = 9,
        MaxIndexCount  = MaxLevel+1,
        Tiling         = 10,
        MaxLinearIndex = Tiling*Tiling
    };

    enum CornerPosition
    {
        CornerNW = 1,
        CornerSW = 2,
        CornerNE = 3,
        CornerSE = 4
    };

public:

    TileIndex();
    virtual ~TileIndex();

    int indexCount()                    const;
    int level()                         const;
    int linearIndex(const int getLevel) const;
    int at(const int getLevel)          const;
    int lastIndex()                     const;
    int indexLat(const int getLevel)    const;
    int indexLon(const int getLevel)    const;

    void clear();
    void appendLinearIndex(const int newIndex);

    QPoint latLonIndex(const int getLevel) const;

    void latLonIndex(const int getLevel, int* const latIndex, int* const lonIndex) const;
    void appendLatLonIndex(const int latIndex, const int lonIndex);

    QIntList toIntList() const;

    GeoCoordinates toCoordinates()                              const;
    GeoCoordinates toCoordinates(const CornerPosition ofCorner) const;

    TileIndex mid(const int first, const int len) const;
    void oneUp();

    static TileIndex fromCoordinates(const Digikam::GeoCoordinates& coordinate, const int getLevel);
    static TileIndex fromIntList(const QIntList& intList);
    static bool indicesEqual(const TileIndex& a, const TileIndex& b, const int upToLevel);
    static QList<QIntList> listToIntListList(const QList<TileIndex>& tileIndexList);

public:

    typedef QList<TileIndex> List;

private:

    int m_indicesCount;
    int m_indices[MaxIndexCount];
};

} // namespace Digikam

QDebug operator<<(QDebug debugOut, const Digikam::TileIndex& tileIndex);

Q_DECLARE_TYPEINFO(Digikam::TileIndex, Q_MOVABLE_TYPE);

#endif // TILE_INDEX_H
