/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : class GroupStateComputer
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
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

// Local includes

#include "groupstatecomputer.h"
#include "geoifacetypes.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN GroupStateComputer::Private
{
public:

    Private()
      : state(SelectedNone),
        stateMask(SelectedNone)
    {
    }

    GeoGroupState state;
    GeoGroupState stateMask;
};

GroupStateComputer::GroupStateComputer()
    : d(new Private)
{
}

GroupStateComputer::~GroupStateComputer()
{
}

GeoGroupState GroupStateComputer::getState() const
{
    return d->state;
}

void GroupStateComputer::clear()
{
    d->state     = SelectedNone;
    d->stateMask = SelectedNone;
}

void GroupStateComputer::addState(const GeoGroupState state)
{
    addSelectedState(state);
    addFilteredPositiveState(state);
    addRegionSelectedState(state);
}

void GroupStateComputer::addSelectedState(const GeoGroupState state)
{
    if (!(d->stateMask & SelectedMask))
    {
        d->state     |= state;
        d->stateMask |= SelectedMask;
    }
    else
    {
        if ((state&SelectedMask) == SelectedAll)
        {
            d->state |= SelectedAll;
        }
        else if ((d->state&SelectedMask) == SelectedAll)
        {
            d->state |= SelectedSome;
        }
        else
        {
            d->state |= state;
        }
    }
}

void GroupStateComputer::addFilteredPositiveState(const GeoGroupState state)
{
    if (!(d->stateMask & FilteredPositiveMask))
    {
        d->state     |= state;
        d->stateMask |= FilteredPositiveMask;
    }
    else
    {
        if ((state&FilteredPositiveMask) == FilteredPositiveAll)
        {
            d->state |= FilteredPositiveAll;
        }
        else if ((d->state&FilteredPositiveMask) == FilteredPositiveAll)
        {
            d->state |= FilteredPositiveSome;
        }
        else
        {
            d->state |= state;
        }
    }
}

void GroupStateComputer::addRegionSelectedState(const GeoGroupState state)
{
    if (!(d->stateMask & RegionSelectedMask))
    {
        d->state     |= state;
        d->stateMask |= RegionSelectedMask;
    }
    else
    {
        if ((state&RegionSelectedMask) == RegionSelectedAll)
        {
            d->state |= RegionSelectedAll;
        }
        else if ((d->state&RegionSelectedMask) == RegionSelectedAll)
        {
            d->state |= RegionSelectedSome;
        }
        else
        {
            d->state |= state;
        }
    }
}

} // namespace Digikam
