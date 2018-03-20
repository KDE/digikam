/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-06
 * Description : Helper functions for geolocation interface interaction
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

#include "gpsimageinfosorter.h"

// Qt includes

#include <QList>
#include <QMenu>
#include <QPointer>
#include <QAction>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "mapwidget.h"

namespace Digikam
{

class GPSImageInfoSorter::Private
{
public:

    Private()
      : mapWidgets(),
        sortOrder(GPSImageInfoSorter::SortYoungestFirst),
        sortMenu(0),
        sortActionOldestFirst(0),
        sortActionYoungestFirst(0),
        sortActionRating(0)
    {
    }

    QList<QPointer<MapWidget> > mapWidgets;
    GPSImageInfoSorter::SortOptions      sortOrder;
    QPointer<QMenu>                      sortMenu;
    QAction*                             sortActionOldestFirst;
    QAction*                             sortActionYoungestFirst;
    QAction*                             sortActionRating;

};

GPSImageInfoSorter::GPSImageInfoSorter(QObject* const parent)
    : QObject(parent),
      d(new Private())
{
}

GPSImageInfoSorter::~GPSImageInfoSorter()
{
    if (d->sortMenu)
    {
        delete d->sortMenu;
    }

    delete d;
}

bool GPSImageInfoSorter::fitsBetter(const GPSImageInfo& oldInfo, const GeoGroupState oldState,
                                    const GPSImageInfo& newInfo, const GeoGroupState newState,
                                    const GeoGroupState globalGroupState, const SortOptions sortOptions)
{
    // the best index for a tile is determined like this:
    // region selected? -> prefer region selected markers
    // positive filtering on? - > prefer positively filtered markers
    // next -> depending on sortkey, prefer better rated ones
    // next -> depending on sortkey, prefer older or younger ones
    // next -> if the image has a URL, prefer the one with the 'lower' URL
    // next -> prefer the image with the higher image id

    // region selection part:
    if (globalGroupState & RegionSelectedMask)
    {
        const bool oldIsRegionSelected = oldState & RegionSelectedMask;
        const bool newIsRegionSelected = newState & RegionSelectedMask;

        if (oldIsRegionSelected != newIsRegionSelected)
        {
            return newIsRegionSelected;
        }
    }

    // positive filtering part:
    if (globalGroupState & FilteredPositiveMask)
    {
        const bool oldIsFilteredPositive = oldState & FilteredPositiveMask;
        const bool newIsFilteredPositive = newState & FilteredPositiveMask;

        if (oldIsFilteredPositive != newIsFilteredPositive)
        {
            return newIsFilteredPositive;
        }
    }

    // care about rating, if requested
    if (sortOptions & SortRating)
    {
        const bool oldHasRating = oldInfo.rating > 0;
        const bool newHasRating = newInfo.rating > 0;

        if (oldHasRating != newHasRating)
        {
            return newHasRating;
        }

        if ( (oldHasRating && newHasRating) && (oldInfo.rating != newInfo.rating) )
        {
            return oldInfo.rating < newInfo.rating;
        }

        // ratings are equal or both have no rating, therefore fall through to the next level
    }

    // finally, decide by date:
    const bool oldHasDate = oldInfo.dateTime.isValid();
    const bool newHasDate = newInfo.dateTime.isValid();

    if (oldHasDate != newHasDate)
    {
        return newHasDate;
    }

    if (oldHasDate && newHasDate)
    {
        if (oldInfo.dateTime != newInfo.dateTime)
        {
            if (sortOptions & SortOldestFirst)
            {
                return oldInfo.dateTime > newInfo.dateTime;
            }
            else
            {
                return oldInfo.dateTime < newInfo.dateTime;
            }
        }
    }

    // compare the image URL
    if (oldInfo.url.isValid() && newInfo.url.isValid())
    {
        return oldInfo.url.url() > newInfo.url.url();
    }

    // last resort: use the image id for reproducibility
    return oldInfo.id > newInfo.id;
}

void GPSImageInfoSorter::addToMapWidget(MapWidget* const mapWidget)
{
    initializeSortMenu();

    d->mapWidgets << QPointer<MapWidget>(mapWidget);
    mapWidget->setSortOptionsMenu(d->sortMenu);
}

void GPSImageInfoSorter::initializeSortMenu()
{
    if (d->sortMenu)
    {
        return;
    }

    d->sortMenu                            = new QMenu();
    d->sortMenu->setTitle(i18n("Sorting"));
    QActionGroup* const sortOrderExclusive = new QActionGroup(d->sortMenu);
    sortOrderExclusive->setExclusive(true);

    connect(sortOrderExclusive, SIGNAL(triggered(QAction*)),
            this, SLOT(slotSortOptionTriggered()));

    d->sortActionOldestFirst = new QAction(i18n("Show oldest first"), sortOrderExclusive);
    d->sortActionOldestFirst->setCheckable(true);
    d->sortMenu->addAction(d->sortActionOldestFirst);

    d->sortActionYoungestFirst = new QAction(i18n("Show youngest first"), sortOrderExclusive);
    d->sortActionYoungestFirst->setCheckable(true);
    d->sortMenu->addAction(d->sortActionYoungestFirst);

    d->sortActionRating = new QAction(i18n("Sort by rating"), this);
    d->sortActionRating->setCheckable(true);
    d->sortMenu->addAction(d->sortActionRating);

    connect(d->sortActionRating, SIGNAL(triggered(bool)),
            this, SLOT(slotSortOptionTriggered()));

    /// @todo Should we initialize the checked state already or wait for a call to setSortOptions?
}

void GPSImageInfoSorter::setSortOptions(const SortOptions sortOptions)
{
    d->sortOrder = sortOptions;

    for (int i = 0; i < d->mapWidgets.count(); ++i)
    {
        if (d->mapWidgets.at(i))
        {
            d->mapWidgets.at(i)->setSortKey(d->sortOrder);
        }
    }

    d->sortActionRating->setChecked(d->sortOrder & GPSImageInfoSorter::SortRating);
    d->sortActionOldestFirst->setChecked(d->sortOrder & GPSImageInfoSorter::SortOldestFirst);
    d->sortActionYoungestFirst->setChecked(!(d->sortOrder & GPSImageInfoSorter::SortOldestFirst));
}

GPSImageInfoSorter::SortOptions GPSImageInfoSorter::getSortOptions() const
{
    return d->sortOrder;
}

void GPSImageInfoSorter::slotSortOptionTriggered()
{
    SortOptions newSortKey = SortYoungestFirst;

    if (d->sortActionOldestFirst->isChecked())
    {
        newSortKey = SortOldestFirst;
    }

    if (d->sortActionRating->isChecked())
    {
        newSortKey|= SortRating;
    }

    d->sortOrder = newSortKey;

    for (int i = 0; i < d->mapWidgets.count(); ++i)
    {
        if (d->mapWidgets.at(i))
        {
            d->mapWidgets.at(i)->setSortKey(d->sortOrder);
        }
    }
}

} // namespace Digikam
