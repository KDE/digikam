/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-01
 * Description : GPSSync common functions and structures
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef GPS_COMMON_H
#define GPS_COMMON_H

// Qt includes

#include <QString>
#include <QUrl>

// Local includes

#include "geocoordinates.h"

namespace Digikam
{

enum MapLayout
{
    MapLayoutOne        = 0,
    MapLayoutHorizontal = 1,
    MapLayoutVertical   = 2
};

QString getUserAgentName();

void coordinatesToClipboard(const GeoCoordinates& coordinates,
                            const QUrl& url,
                            const QString& title);

bool checkSidecarSettings();

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::MapLayout)

#endif // GPS_COMMON_H
