/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-06-01
 * @brief  GPSSync common functions and structures
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
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

#ifndef GPS_COMMON_H
#define GPS_COMMON_H

// Qt includes

#include <QString>
#include <QUrl>

// Libkgeomap includes

#include <KGeoMap/GeoCoordinates>

namespace Digikam
{

enum MapLayout
{
    MapLayoutOne        = 0,
    MapLayoutHorizontal = 1,
    MapLayoutVertical   = 2
};

QString getUserAgentName();

void coordinatesToClipboard(const KGeoMap::GeoCoordinates& coordinates, const QUrl& url, const QString& title);

bool checkSidecarSettings();

} /* namespace Digikam */

Q_DECLARE_METATYPE(Digikam::MapLayout)

#endif /* GPS_COMMON_H */
