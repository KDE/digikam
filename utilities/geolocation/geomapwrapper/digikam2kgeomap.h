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

#ifndef DIGIKAM2KGEOMAP_H
#define DIGIKAM2KGEOMAP_H

// Qt includes

#include <QDateTime>
#include <QObject>
#include <QSize>
#include <QUrl>

// libkgeomap includes

#include <KGeoMap/GeoCoordinates>
#include <KGeoMap/GroupState>

namespace KGeoMap
{
    class MapWidget;
}

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
          url()
    {
    }

    ~GPSImageInfo()
    {
    }

public:

    static GPSImageInfo fromIdCoordinatesRatingDateTime(const qlonglong p_id, const KGeoMap::GeoCoordinates& p_coordinates,
                                                        const int p_rating, const QDateTime& p_creationDate)
    {
        GPSImageInfo info;
        info.id          = p_id;
        info.coordinates = p_coordinates;
        info.rating      = p_rating;
        info.dateTime    = p_creationDate;

        return info;
    }

public:

    qlonglong                   id;
    KGeoMap::GeoCoordinates     coordinates;
    int                         rating;
    QDateTime                   dateTime;
    QUrl                        url;

    typedef QList<GPSImageInfo> List;
};

// --------------------------------------------------------------------------------------------------------------------------

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

    void addToMapWidget(KGeoMap::MapWidget* const mapWidget);
    void setSortOptions(const SortOptions sortOptions);
    SortOptions getSortOptions() const;

    static bool fitsBetter(const GPSImageInfo& oldInfo, const KGeoMap::GroupState oldState,
                           const GPSImageInfo& newInfo, const KGeoMap::GroupState newState,
                           const KGeoMap::GroupState globalGroupState, const SortOptions sortOptions);

private Q_SLOTS:

    void slotSortOptionTriggered();

private:

    void initializeSortMenu();

    class Private;
    Private* const d;
};

} /* namespace Digikam */

Q_DECLARE_METATYPE(Digikam::GPSImageInfo)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::GPSImageInfoSorter::SortOptions)

#endif /* DIGIKAM2KGEOMAP_H */
