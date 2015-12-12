/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-10-10
 * Description : Lut3D color adjustment tool.
 *
 * Copyright (C) 2015 by Andrej Krutak <dev at andree dot sk>
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

#include "lut3dcontainer.h"
#include "filteraction.h"

namespace Digikam
{

Lut3DContainer::Lut3DContainer()
    : path(QString()),
      intensity(0)
{
}

Lut3DContainer::Lut3DContainer(const QString& _path, int _intensity)
    : path(_path),
      intensity(_intensity)
{
}

bool Lut3DContainer::operator==(const Lut3DContainer& other) const
{
    return (path == other.path) && (intensity == other.intensity);
}

void Lut3DContainer::writeToFilterAction(FilterAction& action, const QString& prefix) const
{
    action.addParameter(prefix + QLatin1String("path"),      path);
    action.addParameter(prefix + QLatin1String("intensity"), intensity);
}

Lut3DContainer Lut3DContainer::fromFilterAction(const FilterAction& action, const QString& prefix)
{
    Lut3DContainer settings;
    settings.path      = action.parameter(prefix + QLatin1String("path"), settings.path);
    settings.intensity = action.parameter(prefix + QLatin1String("intensity"), settings.intensity);

    return settings;
}

}  // namespace Digikam
