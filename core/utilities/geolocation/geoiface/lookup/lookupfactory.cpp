/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-05
 * Description : Factory to create instances of Lookup backends
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#include "lookupfactory.h"
#include "lookupaltitudegeonames.h"

namespace Digikam
{

LookupAltitude* LookupFactory::getAltitudeLookup(const QString& backendName,
                                                 QObject* const parent)
{
    if (backendName == QLatin1String("geonames"))
    {
        return new LookupAltitudeGeonames(parent);
    }

    return 0;
}

} // namespace Digikam
