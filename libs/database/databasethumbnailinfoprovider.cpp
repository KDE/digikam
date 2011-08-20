/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 200
 * Description : database thumbnail interface.
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "databasethumbnailinfoprovider.h"

// KDE includes

#include <kurl.h>

// Local includes

#include "albumdb.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "databaseaccess.h"
#include "imageinfo.h"

namespace Digikam
{

ThumbnailInfo DatabaseThumbnailInfoProvider::thumbnailInfo(const QString& path)
{
    // If code here proves to be a bottleneck we can add custom queries to albumdb to retrieve info all-in-one
    ImageInfo imageinfo(KUrl::fromPath(path));

    if (imageinfo.isNull())
    {
        return ThumbnailCreator::fileThumbnailInfo(path);
    }

    ThumbnailInfo thumbinfo;
    QVariantList values;

    thumbinfo.filePath = path;
    thumbinfo.isAccessible = CollectionManager::instance()->locationForAlbumRootId(imageinfo.albumRootId()).isAvailable();

    DatabaseAccess access;
    values = access.db()->getImagesFields(imageinfo.id(),
                                          DatabaseFields::ModificationDate | DatabaseFields::FileSize | DatabaseFields::UniqueHash);

    if (!values.isEmpty())
    {
        thumbinfo.modificationDate = values.at(0).toDateTime();
        thumbinfo.fileSize = values.at(1).toLongLong();
        thumbinfo.uniqueHash = values.at(2).toString();
    }

    values = access.db()->getImageInformation(imageinfo.id(), DatabaseFields::Orientation);

    if (!values.isEmpty())
    {
        thumbinfo.orientationHint = values.first().toInt();
    }

    return thumbinfo;
}


}  // namespace Digikam
