/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-06
 * Description : Helper functions for libkmap interaction
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

#ifndef DIGIKAM2KMAP_H
#define DIGIKAM2KMAP_H

// Qt includes

#include <QDateTime>
#include <QSize>

// KDE includes

#include "kurl.h"

// libkmap includes

#include <libkmap/kmap_primitives.h>

namespace Digikam
{

class GPSImageInfo
{
public:

    GPSImageInfo()
     : id(-2),
       coordinates(),
       rating(-1),
       dateTime(),
       url(),
       dimensions()
    {
    }

    ~GPSImageInfo()
    {
    }

    static GPSImageInfo fromIdCoordinatesRatingDateTime(const qlonglong p_id, const KMap::GeoCoordinates& p_coordinates, const int p_rating, const QDateTime& p_creationDate)
    {
        GPSImageInfo info;
        info.id = p_id;
        info.coordinates = p_coordinates;
        info.rating = p_rating;
        info.dateTime = p_creationDate;

        return info;
    }

    qlonglong               id;
    KMap::GeoCoordinates    coordinates;
    int                     rating;
    QDateTime               dateTime;
    KUrl                    url;
    /// @todo What are these used for?
    QSize                   dimensions;

    typedef QList<GPSImageInfo> List;
};

class GPSImageInfoSorter
{
public:
    enum SortOptions
    {
        SortYoungestFirst = 0,
        SortOldestFirst   = 1,
        SortRating        = 2
    };

    static bool fitsBetter(const GPSImageInfo& oldInfo, const KMap::KMapGroupState oldState,
                           const GPSImageInfo& newInfo, const KMap::KMapGroupState newState,
                           const KMap::KMapGroupState globalGroupState, const SortOptions sortOptions);
};

} /* Digikam */

Q_DECLARE_METATYPE(Digikam::GPSImageInfo)

#endif /* DIGIKAM2KMAP_H */

