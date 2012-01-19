/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata with digiKam database
 *
 * Copyright (C) 2007-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "batchalbumssyncmetadata.moc"

// Qt includes

#include <QString>

// KDE includes

#include <kicon.h>
#include <kapplication.h>
#include <klocale.h>

// Local includes

#include "metadatahub.h"

namespace Digikam
{

BatchAlbumsSyncMetadata::BatchAlbumsSyncMetadata()
    : MaintenanceImgInfJobTool("BatchAlbumsSyncMetadata")
{
}

BatchAlbumsSyncMetadata::~BatchAlbumsSyncMetadata()
{
}

void BatchAlbumsSyncMetadata::gotNewImageInfoList(const ImageInfoList& list)
{
    setTitle(i18n("Sync metadata"));
    setThumbnail(KIcon("run-build-file").pixmap(22));

    MetadataHub fileHub;
    foreach(const ImageInfo& info, list)
    {
        kapp->processEvents();
        if (cancel()) return;

        // read in from database
        fileHub.load(info);

        kapp->processEvents();
        if (cancel()) return;

        // write out to file DMetadata
        fileHub.write(info.filePath());
    }
}

}  // namespace Digikam
