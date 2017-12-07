/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-16
 * Description : GeoCoordinates class
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef GEO_COORDINATES_H
#define GEO_COORDINATES_H

// Qt includes

#include <QString>
#include <QStringList>
#include <QDebug>

// Local includes

#include "digikam_export.h"

// forward declaration only to declare interoperability operators
namespace Marble
{
    class GeoDataCoordinates;
}

namespace Digikam
{

class DIGIKAM_EXPORT GeoCoordinates
{
public:

    enum HasFlag
    {
        HasNothing     = 0,
        HasLatitude    = 1,
        HasLongitude   = 2,
        HasCoordinates = 3,
        HasAltitude    = 4
    };

    Q_DECLARE_FLAGS(HasFlags, HasFlag)

    typedef QList<GeoCoordinates>                 List;
    typedef QPair<GeoCoordinates, GeoCoordinates> Pair;
    typedef QList<GeoCoordinates::Pair>           PairList;

public:

    GeoCoordinates();
    GeoCoordinates(const double inLat, const double inLon);
    GeoCoordinates(const double inLat, const double inLon, const double inAlt);
    ~GeoCoordinates();

    bool operator==(const GeoCoordinates& other) const;

    double lat() const;
    double lon() const ;
    double alt() const;

    bool hasCoordinates() const;
    bool hasLatitude()    const;
    bool hasLongitude()   const;

    void setLatLon(const double inLat, const double inLon);

    bool hasAltitude()  const;
    HasFlags hasFlags() const;

    void setAlt(const double inAlt);

    void clearAlt();
    void clear();

    QString altString() const;
    QString latString() const;
    QString lonString() const;
    QString geoUrl()    const;

    bool sameLonLatAs(const GeoCoordinates& other)   const;
    Marble::GeoDataCoordinates toMarbleCoordinates() const;

    static GeoCoordinates fromGeoUrl(const QString& url, bool* const parsedOkay = 0);
    static GeoCoordinates fromMarbleCoordinates(const Marble::GeoDataCoordinates& marbleCoordinates);
    static Pair makePair(const qreal lat1, const qreal lon1, const qreal lat2, const qreal lon2);

private:

    double   m_lat;
    double   m_lon;
    double   m_alt;
    HasFlags m_hasFlags;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::GeoCoordinates::HasFlags)

Q_DECLARE_TYPEINFO(Digikam::GeoCoordinates, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(Digikam::GeoCoordinates)
Q_DECLARE_METATYPE(Digikam::GeoCoordinates::Pair)
Q_DECLARE_METATYPE(Digikam::GeoCoordinates::PairList)

DIGIKAM_EXPORT QDebug operator<<(QDebug debug, const Digikam::GeoCoordinates& coordinate);

#endif // GEO_COORDINATES_H
