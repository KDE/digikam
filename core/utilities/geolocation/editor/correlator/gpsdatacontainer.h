/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-19
 * Description : A class to hold the GPS related data
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef GPS_DATA_CONTAINER_H
#define GPS_DATA_CONTAINER_H

// Local includes

#include "geocoordinates.h"

namespace Digikam
{

class GPSDataContainer
{
public:

    GPSDataContainer()
      : m_hasFlags(0),
        m_coordinates(),
        m_nSatellites(-1),
        m_dop(-1),
        m_fixType(-1),
        m_speed(0)
    {
    }

    enum HasFlagsEnum
    {
        HasCoordinates    = 1,
        HasAltitude       = 2,
        HasIsInterpolated = 4,
        HasNSatellites    = 8,
        HasDop            = 16,
        HasFixType        = 32,
        HasSpeed          = 64
    };
    Q_DECLARE_FLAGS(HasFlags, HasFlagsEnum)

private:

    HasFlags       m_hasFlags;
    GeoCoordinates m_coordinates;
    int            m_nSatellites;
    qreal          m_dop;
    int            m_fixType;
    qreal          m_speed;

public:

    // general

    bool operator==(const GPSDataContainer& b) const
    {
        if (m_hasFlags != b.m_hasFlags)
            return false;

        if (m_hasFlags.testFlag(HasCoordinates))
        {
            if (!(m_coordinates == b.m_coordinates))
                return false;
        }

        if (hasNSatellites())
        {
            if (m_nSatellites != b.m_nSatellites)
                return false;
        }

        if (hasDop())
        {
            if (m_dop != b.m_dop)
                return false;
        }

        if (hasFixType())
        {
            if (m_fixType != b.m_fixType)
                return false;
        }

        if (hasSpeed())
        {
            if (m_speed != b.m_speed)
                return false;
        }

        return true;
    }

    inline HasFlags flags() const
    {
        return m_hasFlags;
    }

    inline void clear()
    {
        m_hasFlags = 0;
        m_coordinates.clear();
    }

    inline void clearNonCoordinates()
    {
        m_hasFlags&= ~(HasNSatellites | HasDop | HasFixType | HasSpeed);
    }

    // coordinates

    inline GeoCoordinates getCoordinates() const
    {
        return m_coordinates;
    }

    inline void setCoordinates(const GeoCoordinates& coordinates)
    {
        m_coordinates = coordinates;

        if (coordinates.hasCoordinates())
        {
            m_hasFlags |= HasCoordinates;
        }
        else
        {
            m_hasFlags &= ~HasCoordinates;
        }
        if (coordinates.hasAltitude())
        {
            m_hasFlags |= HasAltitude;
        }
        else
        {
            m_hasFlags &= ~HasAltitude;
        }

        clearNonCoordinates();
    }

    inline void setAltitude(const qreal alt)
    {
        m_coordinates.setAlt(alt);
        m_hasFlags |= HasAltitude;
    }

    inline bool hasAltitude() const
    {
        return m_hasFlags.testFlag(HasAltitude);
    }

    inline void setLatLon(const qreal lat, const qreal lon)
    {
        m_coordinates.setLatLon(lat, lon);
        m_hasFlags |= HasCoordinates;

        clearNonCoordinates();
    }

    inline void clearAltitude()
    {
        m_hasFlags &= ~HasAltitude;
        m_coordinates.clearAlt();
    }

    inline bool hasCoordinates() const
    {
        return m_hasFlags.testFlag(HasCoordinates);
    }

    // NSatellites

    inline int getNSatellites() const
    {
        return m_nSatellites;
    }

    inline bool hasNSatellites() const
    {
        return m_hasFlags.testFlag(HasNSatellites);
    }

    inline void clearNSatellites()
    {
        m_hasFlags &= ~HasNSatellites;
    }

    inline void setNSatellites(const int nSatellites)
    {
        m_nSatellites = nSatellites;
        m_hasFlags |= HasNSatellites;
    }

    // DOP

    inline bool hasDop() const
    {
        return m_hasFlags.testFlag(HasDop);
    }

    inline void clearDop()
    {
        m_hasFlags &= ~HasDop;
    }

    inline void setDop(const qreal dop)
    {
        m_dop       = dop;
        m_hasFlags |= HasDop;
    }

    inline qreal getDop() const
    {
        return m_dop;
    }

    // fix type

    inline bool hasFixType() const
    {
        return m_hasFlags.testFlag(HasFixType);
    }

    inline void setFixType(const int fixType)
    {
        m_fixType   = fixType;
        m_hasFlags |= HasFixType;
    }

    inline qreal getFixType() const
    {
        return m_fixType;
    }

    inline void clearFixType()
    {
        m_hasFlags &= ~HasFixType;
    }

    // speed

    /**
     * @brief Return the speed in m/s
     */
    inline qreal getSpeed() const
    {
        return m_speed;
    }

    inline bool hasSpeed() const
    {
        return m_hasFlags.testFlag(HasSpeed);
    }

    /**
     * @brief Set the speed in m/s
     */
    inline void setSpeed(const qreal speed)
    {
        m_hasFlags |= HasSpeed;
        m_speed     = speed;
    }

    inline void clearSpeed()
    {
        m_hasFlags &= ~HasSpeed;
    }
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::GPSDataContainer::HasFlags)

#endif // GPS_DATA_CONTAINER_H
