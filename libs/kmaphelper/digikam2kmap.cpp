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

#include "digikam2kmap.h"

namespace Digikam
{

bool GPSImageInfoSorter::fitsBetter(const GPSImageInfo& oldInfo, const KMap::KMapGroupState oldState,
                                    const GPSImageInfo& newInfo, const KMap::KMapGroupState newState,
                                    const KMap::KMapGroupState globalGroupState, const SortOptions sortOptions)
{
    // the best index for a tile is determined like this:
    // region selected? -> prefer region selected markers
    // positive filtering on? - > prefer positively filtered markers
    // next -> depending on sortkey, prefer better rated ones
    // next -> depending on sortkey, prefer older or younger ones
    // next -> if the image has a URL, prefer the one with the 'lower' URL
    // next -> prefer the image with the higher image id

    // region selection part:
    if (globalGroupState & KMap::KMapRegionSelectedMask)
    {
        const bool oldIsRegionSelected = oldState & KMap::KMapRegionSelectedMask;
        const bool newIsRegionSelected = newState & KMap::KMapRegionSelectedMask;

        if (oldIsRegionSelected != newIsRegionSelected)
        {
            return newIsRegionSelected;
        }
    }

    // positive filtering part:
    if (globalGroupState & KMap::KMapFilteredPositiveMask)
    {
        const bool oldIsFilteredPositive = oldState & KMap::KMapFilteredPositiveMask;
        const bool newIsFilteredPositive = newState & KMap::KMapFilteredPositiveMask;

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

} /* Digikam */


