/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : batch thumbnails generator
 *
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbnailsgenerator.moc"

// Qt includes

#include <QString>

// KDE includes

#include <klocale.h>

// Local includes

#include "thumbnailloadthread.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"
#include "config-digikam.h"

namespace Digikam
{

ThumbnailsGenerator::ThumbnailsGenerator(Mode mode, int albumId)
    : MaintenanceTool("ThumbnailsGenerator", mode, albumId)
{
    setTitle(i18n("Thumbs"));
}

ThumbnailsGenerator::~ThumbnailsGenerator()
{
}

void ThumbnailsGenerator::listItemstoProcess()
{
    QStringList& all = allPicturesPath();

#ifdef USE_THUMBS_DB

    if (mode() == MaintenanceTool::MissingItems)
    {
        QHash<QString, int> filePaths = ThumbnailDatabaseAccess().db()->getFilePathsWithThumbnail();

        QStringList::iterator it = all.begin();

        while (it != all.end())
        {
            if (filePaths.contains(*it))
            {
                it = all.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

#endif

}

void ThumbnailsGenerator::processOne()
{
    if (!checkToContinue()) return;

    QString path = allPicturesPath().first();
    thumbsLoadThread()->deleteThumbnail(path);
    thumbsLoadThread()->find(path);
}

}  // namespace Digikam
