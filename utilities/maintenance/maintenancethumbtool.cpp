/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-16
 * Description : Maintenance tool using thumbnails load thread as items processor.
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

#include "maintenancethumbtool.moc"

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
#include "databaseaccess.h"
#include "dimg.h"
#include "imageinfo.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"
#include "knotificationwrapper.h"

namespace Digikam
{

class MaintenanceThumbTool::MaintenanceThumbToolPriv
{
public:

    MaintenanceThumbToolPriv() :
        thumbLoadThread(0)
    {
    }

    ThumbnailLoadThread* thumbLoadThread;
};

MaintenanceThumbTool::MaintenanceThumbTool(const QString& id, Mode mode, int albumId)
    : MaintenanceTool(id, mode, albumId),
      d(new MaintenanceThumbToolPriv)
{
    d->thumbLoadThread   = ThumbnailLoadThread::defaultThread();

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription, QPixmap)),
            this, SLOT(slotGotThumbnail(LoadingDescription, QPixmap)));
}

MaintenanceThumbTool::~MaintenanceThumbTool()
{
    delete d;
}

ThumbnailLoadThread* MaintenanceThumbTool::thumbsLoadThread() const
{
    return d->thumbLoadThread;
}

void MaintenanceThumbTool::slotGotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (cancel() || allPicturesPath().isEmpty())
    {
        return;
    }

    if (allPicturesPath().first() != desc.filePath)
    {
        return;
    }

    gotNewThumbnail(desc, pix);

    setThumbnail(pix);
    advance(1);

    if (!allPicturesPath().isEmpty())
    {
        allPicturesPath().removeFirst();
    }

    if (allPicturesPath().isEmpty())
    {
        complete();
    }
    else
    {
        processOne();
    }
}

}  // namespace Digikam
