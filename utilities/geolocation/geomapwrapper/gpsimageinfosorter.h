/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-06
 * Description : Helper class for geomap interaction
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

#ifndef GPS_IMAGE_INFO_SORTER_H
#define GPS_IMAGE_INFO_SORTER_H

// Qt includes

#include <QDateTime>
#include <QObject>
#include <QSize>
#include <QUrl>

// Local includes

#include "geocoordinates.h"
#include "geogroupstate.h"
#include "gpsimageinfo.h"
#include "mapwidget.h"

namespace Digikam
{

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
    Q_DECLARE_FLAGS(SortOptions, SortOption)

public:

    explicit GPSImageInfoSorter(QObject* const parent);
    ~GPSImageInfoSorter();

    void addToMapWidget(MapWidget* const mapWidget);
    void setSortOptions(const SortOptions sortOptions);
    SortOptions getSortOptions() const;

public:

    static bool fitsBetter(const GPSImageInfo& oldInfo, const GeoGroupState oldState,
                           const GPSImageInfo& newInfo, const GeoGroupState newState,
                           const GeoGroupState globalGroupState, const SortOptions sortOptions);

private Q_SLOTS:

    void slotSortOptionTriggered();

private:

    void initializeSortMenu();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::GPSImageInfoSorter::SortOptions)

#endif // GPS_IMAGE_INFO_SORTER_H
