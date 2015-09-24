/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-03-21
 * @brief  Reverse geocoding data.
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef RG_INFO_H
#define RG_INFO_H

// Qt includes

#include <QPersistentModelIndex>
#include <QMap>

// GeoIface includes

#include "geoiface_types.h"

// Local includes

#include "gpsdatacontainer.h"

using namespace GeoIface;

namespace Digikam
{

/**
 * @class RGInfo
 *
 * @brief This class contains data needed in reverse geocoding process.
 */

class RGInfo
{
public:

    /**
     * Constructor
     */
    RGInfo();

    /**
     * Destructor
     */
    ~RGInfo();

public:
    /**
     * The image index.
     */
    QPersistentModelIndex  id;

    /**
     * The coordinates of current image.
     */
    GeoCoordinates         coordinates;

    /**
     * The address elements and their names.
     */
    QMap<QString, QString> rgData;
};

} /* namespace Digikam */

#endif /* RG_INFO_H */
