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
#include <QObject>
#include <QSize>

// KDE includes

#include "kurl.h"

// libkmap includes

#include <libkmap/kmap_primitives.h>

namespace KMap
{
    class KMapWidget;
}

namespace Digikam
{

class ImageInfo;

class GPSImageInfo
{
public:

    /// @todo De-inline these functions?

    GPSImageInfo()
     : id(-2),
       coordinates(),
       rating(-1),
       dateTime(),
       url()
    {
    }

    ~GPSImageInfo()
    {
    }

    static GPSImageInfo fromIdCoordinatesRatingDateTime(const qlonglong p_id, const KMap::GeoCoordinates& p_coordinates,
                                                        const int p_rating, const QDateTime& p_creationDate)
    {
        GPSImageInfo info;
        info.id          = p_id;
        info.coordinates = p_coordinates;
        info.rating      = p_rating;
        info.dateTime    = p_creationDate;

        return info;
    }

    static bool fromImageInfo(const ImageInfo& imageInfo, GPSImageInfo* const gpsImageInfo);

    qlonglong               id;
    KMap::GeoCoordinates    coordinates;
    int                     rating;
    QDateTime               dateTime;
    KUrl                    url;

    typedef QList<GPSImageInfo> List;
};

class GPSImageInfoSorter : public QObject
{
Q_OBJECT

public:
    enum SortOption
    {
        SortYoungestFirst = 0,
        SortOldestFirst   = 1,
        SortRating        = 2
    };
    Q_DECLARE_FLAGS(SortOptions, SortOption);

    GPSImageInfoSorter(QObject* const parent);
    ~GPSImageInfoSorter();

    void addToKMapWidget(KMap::KMapWidget* const mapWidget);
    void setSortOptions(const SortOptions sortOptions);
    SortOptions getSortOptions() const;

    static bool fitsBetter(const GPSImageInfo& oldInfo, const KMap::KMapGroupState oldState,
                           const GPSImageInfo& newInfo, const KMap::KMapGroupState newState,
                           const KMap::KMapGroupState globalGroupState, const SortOptions sortOptions);

private Q_SLOTS:

    void slotSortOptionTriggered();

private:

    void initializeSortMenu();

    class Private;
    Private* const d;
};

} /* namespace Digikam */

Q_DECLARE_METATYPE(Digikam::GPSImageInfo);
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::GPSImageInfoSorter::SortOptions);

#endif /* DIGIKAM2KMAP_H */
