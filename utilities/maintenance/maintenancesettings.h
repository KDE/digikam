/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-31
 * Description : maintenance manager settings
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MAINTENANCE_SETTINGS_H
#define MAINTENANCE_SETTINGS_H

// Qt includes

#include <QDebug>

// Local includes

#include "album.h"
#include "facescansettings.h"
#include "haariface.h"
#include "imagequalitysettings.h"
#include "metadatasynchronizer.h"
#include "imagequalitysorter.h"

namespace Digikam
{

class MaintenanceSettings
{

public:

    MaintenanceSettings();
    virtual ~MaintenanceSettings();

public:

    bool                                    wholeAlbums;
    bool                                    wholeTags;

    AlbumList                               albums;
    AlbumList                               tags;

    /// Use Multi-core CPU to process items.
    bool                                    useMutiCoreCPU;

    /// Find new items on whole collection.
    bool                                    newItems;

    /// Generate thumbnails
    bool                                    thumbnails;
    /// Rebuild all thumbnails or only scan missing items.
    bool                                    scanThumbs;

    /// Generate finger-prints
    bool                                    fingerPrints;
    /// Rebuild all fingerprints or only scan missing items.
    bool                                    scanFingerPrints;

    /// Scan for new items
    bool                                    duplicates;
    /// Minimal similarity between items to compare, in percents.
    int                                     minSimilarity;
    /// Maximal similarity between items to compare, in percents.
    int                                     maxSimilarity;
    /// The type of restrictions to apply on duplicates search results.
    HaarIface::DuplicatesSearchRestrictions duplicatesRestriction;

    /// Scan for faces.
    bool                                    faceManagement;
    /// Face detection settings.
    FaceScanSettings                        faceSettings;

    /// Perform Image Quality Sorting.
    bool                                    qualitySort;
    /// Mode to assign Pick Labels to items.
    int                                     qualityScanMode;
    /// Image Quality Sorting Settings.
    ImageQualitySettings                    quality;

    /// Sync metadata and DB.
    bool                                    metadataSync;
    /// Sync direction (image metadata <-> DB).
    int                                     syncDirection;

    /// Perform database cleanup
    bool                                    databaseCleanup;
    bool                                    cleanThumbDb;
    bool                                    cleanFacesDb;
    bool                                    shrinkDatabases;
};

//! qDebug() stream operator. Writes property @a s to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const MaintenanceSettings& s);

}  // namespace Digikam

#endif  // MAINTENANCE_SETTINGS_H
