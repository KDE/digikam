/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-06
 * Description : Helper class for geomap interaction
 *
 * Copyright (C) 2011      by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "gpsiteminfo.h"

namespace Digikam
{

GPSItemInfo::GPSItemInfo()
    : id(-2),
      coordinates(),
      rating(-1),
      dateTime(),
      url()
{
}

GPSItemInfo::~GPSItemInfo()
{
}

GPSItemInfo GPSItemInfo::fromIdCoordinatesRatingDateTime(const qlonglong p_id,
                                                         const GeoCoordinates& p_coordinates,
                                                         const int p_rating,
                                                         const QDateTime& p_creationDate)
{
    GPSItemInfo info;
    info.id          = p_id;
    info.coordinates = p_coordinates;
    info.rating      = p_rating;
    info.dateTime    = p_creationDate;

    return info;
}

} // namespace Digikam
