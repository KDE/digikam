/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-31
 * Description : maintenance manager settings
 *
 * Copyright (C) 2012-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MAINTENANCESETTINGS_H
#define MAINTENANCESETTINGS_H

// Local includes

#include "facescansettings.h"

namespace Digikam
{

class MaintenanceSettings
{

public:

    MaintenanceSettings()
    {
        newItems         = false;
        thumbnails       = false;
        scanThumbs       = false;
        fingerPrints     = false;
        scanFingerPrints = false;
        duplicates       = false;
        similarity       = 90;
        metadata         = false;
    };

    virtual ~MaintenanceSettings()
    {
    };

public:

    /// Find new items on whole collection
    bool             newItems;

    /// Generate thumbnails
    bool             thumbnails;
    /// Rebuild all thumbnails or only scan missing items.
    bool             scanThumbs;

    /// Generate finger-prints
    bool             fingerPrints;
    /// Rebuild all fingerprints or only scan missing items.
    bool             scanFingerPrints;

    /// Scan for new items
    bool             duplicates;
    /// Similarity between items to compare, in percents.
    int              similarity;

    /// Sync image metadata with DB
    bool             metadata;

    /// Scan for faces
    bool             faceDetection;
    /// Face detection settings
    FaceScanSettings faceSettings;
};

}  // namespace Digikam

#endif  // MAINTENANCESETTINGS_H
