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

#include "batchthumbsgenerator.moc"

// Qt includes

#include <QString>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QPixmap>

// KDE includes

#include <kapplication.h>
#include <kcodecs.h>
#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albuminfo.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "databaseaccess.h"
#include "imageinfo.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"
#include "config-digikam.h"

namespace Digikam
{

BatchThumbsGenerator::BatchThumbsGenerator(Mode mode, int albumId)
    : MaintenanceTool(mode, albumId)
{
    setTitle(i18n("Thumbs"));
}

BatchThumbsGenerator::~BatchThumbsGenerator()
{
}

void BatchThumbsGenerator::listItemstoProcess()
{
    QStringList& all = allPicturePath();

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

void BatchThumbsGenerator::processOne()
{
    if (!checkToContinue()) return;

    QString path = allPicturePath().first();
    thumbsLoadThread()->deleteThumbnail(path);
    thumbsLoadThread()->find(path);
}

}  // namespace Digikam
