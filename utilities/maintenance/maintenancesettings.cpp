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

#include "maintenancesettings.h"

namespace Digikam
{

MaintenanceSettings::MaintenanceSettings()
{
    newItems         = false;
    thumbnails       = false;
    scanThumbs       = false;
    fingerPrints     = false;
    scanFingerPrints = false;
    duplicates       = false;
    similarity       = 90;
    metadata         = false;
    syncDirection    = MetadataSynchronizer::WriteFromDatabaseToFile;
    faceDetection    = false;
};

MaintenanceSettings::~MaintenanceSettings()
{
}

//! kDebug() stream operator. Writes property @a s to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const MaintenanceSettings& s)
{
    dbg.nospace() << "newItems            : " << s.newItems;
    dbg.nospace() << "thumbnails          : " << s.thumbnails;
    dbg.nospace() << "scanThumbs          : " << s.scanThumbs;
    dbg.nospace() << "fingerPrints        : " << s.fingerPrints;
    dbg.nospace() << "scanFingerPrints    : " << s.scanFingerPrints;
    dbg.nospace() << "duplicates          : " << s.duplicates;
    dbg.nospace() << "similarity          : " << s.similarity;
    dbg.nospace() << "metadata            : " << s.metadata;
    dbg.nospace() << "syncDirection       : " << s.syncDirection;
    dbg.nospace() << "facedetection       : " << s.faceDetection;
    dbg.nospace() << "faceScannedHandling : " << s.faceSettings.alreadyScannedHandling;
    return dbg.space();
}

}  // namespace Digikam
