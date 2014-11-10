/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-06
 * Description : Helper functions for libkgeomap interaction
 *
 * Copyright (C) 2011 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef DIGIKAM2KGEOMAP_DATABASE_H
#define DIGIKAM2KGEOMAP_DATABASE_H

#include "digikam2kgeomap.h"

// local includes

#include "imageinfo.h"
#include "imageposition.h"

namespace Digikam
{

/**
 * @todo This function should not be in a header, but that can be solved later
 */
inline bool GPSImageInfo::fromImageInfo(const ImageInfo& imageInfo, GPSImageInfo* const gpsImageInfo)
{
    const ImagePosition pos = imageInfo.imagePosition();

    if (pos.isEmpty())
    {
        return false;
    }

    gpsImageInfo->coordinates.setLatLon(pos.latitudeNumber(), pos.longitudeNumber());

    if (pos.hasAltitude())
    {
        gpsImageInfo->coordinates.setAlt(pos.altitude());
    }

    gpsImageInfo->dateTime  = imageInfo.dateTime();
    gpsImageInfo->rating    = imageInfo.rating();
    gpsImageInfo->url       = imageInfo.fileUrl();
    gpsImageInfo->id        = imageInfo.id();

    return true;
}

} // namespace Digikam

#endif /* DIGIKAM2KGEOMAP_DATABASE_H */
