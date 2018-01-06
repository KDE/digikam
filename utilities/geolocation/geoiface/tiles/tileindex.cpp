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

#include "tileindex.h"
#include "geoifacecommon.h"

namespace Digikam
{

TileIndex::TileIndex()
    : m_indicesCount(0)
{
    for (int i = 0 ; i < MaxIndexCount ; ++i)
    {
        m_indices[i] = 0;
    }
}

TileIndex::~TileIndex()
{
}

int TileIndex::indexCount() const
{
    return m_indicesCount;
}

int TileIndex::level() const
{
    return m_indicesCount > 0 ? m_indicesCount - 1 : 0;
}

void TileIndex::clear()
{
    m_indicesCount = 0;
}

void TileIndex::appendLinearIndex(const int newIndex)
{
    GEOIFACE_ASSERT(m_indicesCount + 1 <= MaxIndexCount);
    m_indices[m_indicesCount] = newIndex;
    m_indicesCount++;
}

int TileIndex::linearIndex(const int getLevel) const
{
    GEOIFACE_ASSERT(getLevel<=level());
    return m_indices[getLevel];
}

int TileIndex::at(const int getLevel) const
{
    GEOIFACE_ASSERT(getLevel<=level());
    return m_indices[getLevel];
}

int TileIndex::lastIndex() const
{
    GEOIFACE_ASSERT(m_indicesCount>0);
    return m_indices[m_indicesCount-1];
}

int TileIndex::indexLat(const int getLevel) const
{
    return linearIndex(getLevel) / Tiling;
}

int TileIndex::indexLon(const int getLevel) const
{
    return linearIndex(getLevel) % Tiling;
}

QPoint TileIndex::latLonIndex(const int getLevel) const
{
    return QPoint(indexLon(getLevel), indexLat(getLevel));
}

void TileIndex::latLonIndex(const int getLevel, int* const latIndex, int* const lonIndex) const
{
    GEOIFACE_ASSERT(getLevel <= level());
    *latIndex = indexLat(getLevel);
    *lonIndex = indexLon(getLevel);
    GEOIFACE_ASSERT(*latIndex < Tiling);
    GEOIFACE_ASSERT(*lonIndex < Tiling);
}

void TileIndex::appendLatLonIndex(const int latIndex, const int lonIndex)
{
    appendLinearIndex(latIndex*Tiling + lonIndex);
}

QIntList TileIndex::toIntList() const
{
    QIntList result;

    for (int i = 0 ; i < m_indicesCount ; ++i)
    {
        result << m_indices[i];
    }

    return result;
}

TileIndex TileIndex::fromIntList(const QIntList& intList)
{
    TileIndex result;

    for (int i = 0 ; i < intList.count() ; ++i)
    {
        result.appendLinearIndex(intList.at(i));
    }

    return result;
}

bool TileIndex::indicesEqual(const TileIndex& a, const TileIndex& b, const int upToLevel)
{
    GEOIFACE_ASSERT(a.level() >= upToLevel);
    GEOIFACE_ASSERT(b.level() >= upToLevel);

    for (int i = 0 ; i <= upToLevel ; ++i)
    {
        if (a.linearIndex(i) != b.linearIndex(i))
        {
            return false;
        }
    }

    return true;
}

TileIndex TileIndex::mid(const int first, const int len) const
{
    GEOIFACE_ASSERT(first+(len-1) <= m_indicesCount);
    TileIndex result;

    for (int i = first ; i < first + len ; ++i)
    {
        result.appendLinearIndex(m_indices[i]);
    }

    return result;
}

void TileIndex::oneUp()
{
    GEOIFACE_ASSERT(m_indicesCount > 0);
    m_indicesCount--;
}

QList<QIntList> TileIndex::listToIntListList(const QList<TileIndex>& tileIndexList)
{
    QList<QIntList> result;

    for (int i = 0 ; i < tileIndexList.count() ; ++i)
    {
        result << tileIndexList.at(i).toIntList();
    }

    return result;
}

TileIndex TileIndex::fromCoordinates(const Digikam::GeoCoordinates& coordinate, const int getLevel)
{
    GEOIFACE_ASSERT(getLevel <= MaxLevel);

    if (!coordinate.hasCoordinates())
        return TileIndex();

    qreal tileLatBL     = -90.0;
    qreal tileLonBL     = -180.0;
    qreal tileLatHeight = 180.0;
    qreal tileLonWidth  = 360.0;

    TileIndex resultIndex;

    for (int l = 0 ; l <= getLevel ; ++l)
    {
        // how many tiles at this level?
        const qreal latDivisor = TileIndex::Tiling;
        const qreal lonDivisor = TileIndex::Tiling;

        const qreal dLat       = tileLatHeight / latDivisor;
        const qreal dLon       = tileLonWidth / lonDivisor;

        int latIndex           = int( (coordinate.lat() - tileLatBL ) / dLat );
        int lonIndex           = int( (coordinate.lon() - tileLonBL ) / dLon );

        // protect against invalid indices due to rounding errors
        bool haveRoundingErrors = false;

        if (latIndex < 0)
        {
            haveRoundingErrors = true;
            latIndex           = 0;
        }

        if (lonIndex < 0)
        {
            haveRoundingErrors = true;
            lonIndex           = 0;
        }

        if (latIndex >= latDivisor)
        {
            haveRoundingErrors = true;
            latIndex           = latDivisor-1;
        }

        if (lonIndex >= lonDivisor)
        {
            haveRoundingErrors = true;
            lonIndex           = lonDivisor-1;
        }

        if (haveRoundingErrors)
        {
//             qCDebug(DIGIKAM_GEOIFACE_LOG) << QString::fromLatin1("Rounding errors at level %1!").arg(l);
        }

        resultIndex.appendLatLonIndex(latIndex, lonIndex);

        // update the start position for the next tile:
        // TODO: rounding errors
        tileLatBL     += latIndex*dLat;
        tileLonBL     += lonIndex*dLon;
        tileLatHeight /= latDivisor;
        tileLonWidth  /= lonDivisor;
    }

    return resultIndex;
}

GeoCoordinates TileIndex::toCoordinates() const
{
    // TODO: safeguards against rounding errors!
    qreal tileLatBL     = -90.0;
    qreal tileLonBL     = -180.0;
    qreal tileLatHeight = 180.0;
    qreal tileLonWidth  = 360.0;

    for (int l = 0 ; l < m_indicesCount ; ++l)
    {
        // how many tiles are at this level?
        const qreal latDivisor = TileIndex::Tiling;
        const qreal lonDivisor = TileIndex::Tiling;

        const qreal dLat       = tileLatHeight / latDivisor;
        const qreal dLon       = tileLonWidth / lonDivisor;

        const int latIndex     = indexLat(l);
        const int lonIndex     = indexLon(l);

        // update the start position for the next tile:
        tileLatBL             += latIndex*dLat;
        tileLonBL             += lonIndex*dLon;
        tileLatHeight         /= latDivisor;
        tileLonWidth          /= lonDivisor;
    }

    return GeoCoordinates(tileLatBL, tileLonBL);
}


GeoCoordinates TileIndex::toCoordinates(const CornerPosition ofCorner) const
{
    // TODO: safeguards against rounding errors!
    qreal tileLatBL     = -90.0;
    qreal tileLonBL     = -180.0;
    qreal tileLatHeight = 180.0;
    qreal tileLonWidth  = 360.0;

    for (int l = 0 ; l < m_indicesCount ; ++l)
    {
        // how many tiles are at this level?
        const qreal latDivisor = TileIndex::Tiling;
        const qreal lonDivisor = TileIndex::Tiling;

        const qreal dLat       = tileLatHeight / latDivisor;
        const qreal dLon       = tileLonWidth / lonDivisor;

        const int latIndex     = indexLat(l);
        const int lonIndex     = indexLon(l);

        // update the start position for the next tile:
        if ((l+1) >= m_indicesCount)
        {
            if (ofCorner == CornerNW)
            {
                tileLatBL += latIndex*dLat;
                tileLonBL += lonIndex*dLon;
            }
            else if (ofCorner == CornerSW)
            {
                tileLatBL += (latIndex+1)*dLat;
                tileLonBL += lonIndex*dLon;
            }
            else if (ofCorner == CornerNE)
            {
                tileLatBL += latIndex*dLat;
                tileLonBL += (lonIndex+1)*dLon;
            }
            else if (ofCorner == CornerSE)
            {
                tileLatBL += (latIndex+1)*dLat;
                tileLonBL += (lonIndex+1)*dLon;
            }
        }
        else
        {
            // update the start position for the next tile:
            tileLatBL += latIndex*dLat;
            tileLonBL += lonIndex*dLon;
        }

        tileLatHeight /= latDivisor;
        tileLonWidth  /= lonDivisor;
    }

    return GeoCoordinates(tileLatBL, tileLonBL);
}

} // namespace Digikam

QDebug operator<<(QDebug debug, const Digikam::TileIndex& tileIndex)
{
    debug << tileIndex.toIntList();
    return debug;
}
