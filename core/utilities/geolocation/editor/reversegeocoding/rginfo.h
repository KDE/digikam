/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-21
 * Description : Reverse geocoding data.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef RG_INFO_H
#define RG_INFO_H

// Qt includes

#include <QPersistentModelIndex>
#include <QMap>

// Local includes

#include "geoifacetypes.h"
#include "gpsdatacontainer.h"
#include "digikam_export.h"

namespace Digikam
{

/**
 * @class RGInfo
 *
 * @brief This class contains data needed in reverse geocoding process.
 */

class DIGIKAM_EXPORT RGInfo
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

} // namespace Digikam

#endif // RG_INFO_H
