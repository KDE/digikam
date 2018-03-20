/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : Geolocation data container.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef GEO_DATA_CONTAINER_H
#define GEO_DATA_CONTAINER_H

#include <QStringList>
#include <QMetaType>

namespace Digikam
{

class GeoDataContainer
{
public:

    GeoDataContainer()
        : m_interpolated(false),
          m_altitude(0.0),
          m_latitude(0.0),
          m_longitude(0.0)
    {
    };

    GeoDataContainer(double altitude, double latitude,
                     double longitude, bool interpolated)
        : m_interpolated(interpolated),
          m_altitude(altitude),
          m_latitude(latitude),
          m_longitude(longitude)
    {
    };

    ~GeoDataContainer()
    {
    };

    GeoDataContainer& operator=(const GeoDataContainer& data)
    {
        m_interpolated = data.isInterpolated();
        m_altitude     = data.altitude();
        m_latitude     = data.latitude();
        m_longitude    = data.longitude();
        return *this;
    };

    // use this instead of '==', because '==' implies having the
    // same value for m_interpolated
    bool sameCoordinatesAs(const GeoDataContainer& a) const
    {
        return ( a.m_altitude == m_altitude ) &&
               ( a.m_latitude == m_latitude ) &&
               ( a.m_longitude == m_longitude);
    }

    void setInterpolated(bool ite)  { m_interpolated = ite; };
    void setAltitude(double alt)    { m_altitude     = alt; };
    void setLatitude(double lat)    { m_latitude     = lat; };
    void setLongitude(double lng)   { m_longitude    = lng; };

    bool   isInterpolated() const   { return m_interpolated; };
    double altitude()       const   { return m_altitude;     };
    double latitude()       const   { return m_latitude;     };
    double longitude()      const   { return m_longitude;    };

    QString altitudeString()  const { return QString::number(m_altitude,  'g', 12); }
    QString latitudeString()  const { return QString::number(m_latitude,  'g', 12); }
    QString longitudeString() const { return QString::number(m_longitude, 'g', 12); }

    QString geoUrl() const
    {
        return QString::fromLatin1("geo:%1,%2,%3")
                                   .arg(latitudeString())
                                   .arg(longitudeString())
                                   .arg(altitudeString());
    }

    static GeoDataContainer fromGeoUrl(const QString& url, bool* const parsedOkay)
    {
        // parse geo:-uri according to (only partially implemented):
        // http://tools.ietf.org/html/draft-ietf-geopriv-geo-uri-04
        // TODO: verify that we follow the spec fully!
        if (!url.startsWith(QLatin1String("geo:")))
        {
            // TODO: error
            if (parsedOkay)
                *parsedOkay = false;

            return GeoDataContainer();
        }

        const QStringList parts = url.mid(4).split(QLatin1Char(','));

        GeoDataContainer position;

        if ((parts.size() == 3) || (parts.size() == 2))
        {
            bool okay          = true;
            double ptLongitude = 0.0;
            double ptLatitude  = 0.0;
            double ptAltitude  = 0.0;
            ptLatitude         = parts[0].toDouble(&okay);

            if (okay)
                ptLongitude = parts[1].toDouble(&okay);

            if (okay&&(parts.size()==3))
                ptAltitude = parts[2].toDouble(&okay);

            if (!okay)
            {
                *parsedOkay = false;
                return GeoDataContainer();
            }

            position = GeoDataContainer(ptAltitude, ptLatitude, ptLongitude, false);
        }
        else
        {
            if (parsedOkay)
                *parsedOkay = false;

            return GeoDataContainer();
        }

        if (parsedOkay)
                *parsedOkay = true;

        return position;
    }

private:

    bool   m_interpolated;

    double m_altitude;
    double m_latitude;
    double m_longitude;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::GeoDataContainer)

#endif  // GEO_DATA_CONTAINER_H
