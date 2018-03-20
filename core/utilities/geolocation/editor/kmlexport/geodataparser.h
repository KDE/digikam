/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : GPS data file parser.
 *               (GPX format http://www.topografix.com/gpx.asp).
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

#ifndef GEO_DATA_PARSER_H
#define GEO_DATA_PARSER_H

// Qt includes

#include <QDateTime>
#include <QMap>
#include <QUrl>

// Local includes

#include "geodatacontainer.h"

namespace Digikam
{

class GeoDataParser
{

public:

    explicit GeoDataParser();
    ~GeoDataParser() {};

    bool loadGPXFile(const QUrl& url);

    void clear();
    int  numPoints() const;
    bool matchDate(const QDateTime& photoDateTime, int maxGapTime, int secondsOffset,
                   bool photoHasSystemTimeZone,
                   bool interpolate, int interpolationDstTime,
                   GeoDataContainer* const gpsData);

private:

    // Methods used to perform interpolation.
    QDateTime findNextDate(const QDateTime& dateTime, int secs);
    QDateTime findPrevDate(const QDateTime& dateTime, int secs);

protected:

    typedef QMap<QDateTime, GeoDataContainer> GeoDataMap;

    GeoDataMap                                m_GeoDataMap;
};

} // namespace Digikam

#endif // GEO_DATA_PARSER_H
