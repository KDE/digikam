/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-16-01
 * Description : white balance color correction.
 *
 * Copyright (C) 2007-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008      by Guillaume Castagnino <casta at xwing dot info>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "wbcontainer.h"
#include "wbfilter.h"

namespace Digikam
{

WBContainer::WBContainer()
{
    // Neutral color temperature settings.
    black          = 0.0;
    expositionMain = 0.0;
    expositionFine = 0.0;
    temperature    = 6500.0;
    green          = 1.0;
    dark           = 0.5;
    gamma          = 1.0;
    saturation     = 1.0;

    maxr           = -1;
    maxg           = -1;
    maxb           = -1;
}

bool WBContainer::isDefault() const
{
    return (*this == WBContainer());
}

bool WBContainer::operator==(const WBContainer& other) const
{
    return black          == other.black          &&
           expositionMain == other.expositionMain &&
           expositionFine == other.expositionFine &&
           temperature    == other.temperature    &&
           green          == other.green          &&
           dark           == other.dark           &&
           gamma          == other.gamma          &&
           saturation     == other.saturation;
}

void WBContainer::writeToFilterAction(FilterAction& action, const QString& prefix) const
{
    action.addParameter(prefix + "black",          black);
    action.addParameter(prefix + "expositionMain", expositionMain);
    action.addParameter(prefix + "expositionFine", expositionFine);
    action.addParameter(prefix + "temperature",    temperature);
    action.addParameter(prefix + "green",          green);
    action.addParameter(prefix + "dark",           dark);
    action.addParameter(prefix + "gamma",          gamma);
    action.addParameter(prefix + "saturation",     saturation);
}

WBContainer WBContainer::fromFilterAction(const FilterAction& action, const QString& prefix)
{
    WBContainer settings;
    settings.black          = action.parameter(prefix + "black",          settings.black);
    settings.expositionMain = action.parameter(prefix + "expositionMain", settings.expositionMain);
    settings.expositionFine = action.parameter(prefix + "expositionFine", settings.expositionFine);
    settings.temperature    = action.parameter(prefix + "temperature",    settings.temperature);
    settings.green          = action.parameter(prefix + "green",          settings.green);
    settings.dark           = action.parameter(prefix + "dark",           settings.dark);
    settings.gamma          = action.parameter(prefix + "gamma",          settings.gamma);
    settings.saturation     = action.parameter(prefix + "saturation",     settings.saturation);
    return settings;
}

}  // namespace Digikam
