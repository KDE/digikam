/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011      by Leif Huhn <leif at dkstat dot com>
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

#include "dmetadata.h"

// Local includes

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

DMetadata::DMetadata()
    : MetaEngine()
{
    registerMetadataSettings();
}

DMetadata::DMetadata(const QString& filePath)
    : MetaEngine()
{
    registerMetadataSettings();
    load(filePath);
}

DMetadata::DMetadata(const MetaEngineData& data)
    : MetaEngine(data)
{
    registerMetadataSettings();
}

DMetadata::~DMetadata()
{
}

void DMetadata::registerMetadataSettings()
{
    setSettings(MetaEngineSettings::instance()->settings());
}

void DMetadata::setSettings(const MetaEngineSettingsContainer& settings)
{
    setUseXMPSidecar4Reading(settings.useXMPSidecar4Reading);
    setWriteRawFiles(settings.writeRawFiles);
    setMetadataWritingMode(settings.metadataWritingMode);
    setUpdateFileTimeStamp(settings.updateFileTimeStamp);
}

} // namespace Digikam
