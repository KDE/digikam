/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-16
 * Description : fingerprints generator
 *
 * Copyright (C) 2008-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "fingerprintsgenerator.moc"

// Qt includes

#include <QString>

// KDE includes

#include <klocale.h>

// Local includes

#include "dimg.h"
#include "albumdb.h"
#include "databaseaccess.h"
#include "previewloadthread.h"
#include "loadingdescription.h"
#include "haar.h"

namespace Digikam
{

FingerPrintsGenerator::FingerPrintsGenerator(Mode mode, int albumId)
    : MaintenanceTool(mode, albumId)
{
    setTitle(i18n("Fingerprints"));
}

FingerPrintsGenerator::~FingerPrintsGenerator()
{
}

void FingerPrintsGenerator::listItemstoProcess()
{
    if (mode() == MaintenanceTool::MissingItems)
    {
        allPicturePath() = DatabaseAccess().db()->getDirtyOrMissingFingerprintURLs();
    }
}

void FingerPrintsGenerator::processOne()
{
    if (!checkToContinue()) return;

    QString path = allPicturePath().first();
    LoadingDescription description(path, HaarIface::preferredSize(), LoadingDescription::ConvertToSRGB);
    description.rawDecodingSettings.rawPrm.sixteenBitsImage = false;
    previewLoadThread()->load(description);
}

void FingerPrintsGenerator::gotNewPreview(const LoadingDescription& desc, const DImg& img)
{
    if (!img.isNull())
    {
        // compute Haar fingerprint
        m_haarIface.indexImage(desc.filePath, img);
    }
}

}  // namespace Digikam
