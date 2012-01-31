/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-31
 * Description : maintenance manager settings
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

namespace Digikam
{

class MaintenanceSettings
{

public:

    MaintenanceSettings()
    {
        rebuildAllThumbs       = true;
        rebuildAllFingerPrints = true;
    };

    virtual ~MaintenanceSettings() {};

public:

    /// Rebuild all thumbnails or only scan missing items.
    bool rebuildAllThumbs;

    /// Rebuild all fingerprints or only scan missing items.
    bool rebuildAllFingerPrints;
};

}  // namespace Digikam

#endif  // MAINTENANCESETTINGS_H
