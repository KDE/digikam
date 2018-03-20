/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : Primitive datatypes for geolocation interface
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

#ifndef GEO_IFACE_TYPES_H
#define GEO_IFACE_TYPES_H

// Qt includes

#include <QPersistentModelIndex>

Q_DECLARE_METATYPE(QPersistentModelIndex)

namespace Digikam
{

enum GeoMouseMode
{
    MouseModePan                     = 1,
    MouseModeRegionSelection         = 2,
    MouseModeRegionSelectionFromIcon = 4,
    MouseModeFilter                  = 8,
    MouseModeSelectThumbnail         = 16,
    MouseModeZoomIntoGroup           = 32,
    MouseModeLast                    = 32
};

Q_DECLARE_FLAGS(GeoMouseModes, GeoMouseMode)

enum GeoExtraAction
{
    ExtraActionSticky = 1
};

Q_DECLARE_FLAGS(GeoExtraActions, GeoExtraAction)

typedef QList<int> QIntList;

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::GeoMouseModes)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::GeoExtraActions)

Q_DECLARE_METATYPE(Digikam::GeoMouseModes)

#endif // GEO_IFACE_TYPES_H
