/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata from all Albums
 *               with digiKam database
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

#include "maintenanceimginfjobtool.moc"

// Qt includes

#include <QTimer>
#include <QDateTime>
#include <QPixmap>

// KDE includes

#include <kdebug.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "imageinfojob.h"
#include "metadatahub.h"

namespace Digikam
{

enum BatchAlbumsStatus
{
    NotRunning,
    LoadingAlbum,
    ReceivingImageInfos,
    CompletedAlbum
};

class MaintenanceImgInfJobTool::MaintenanceImgInfJobToolPriv
{
public:

    MaintenanceImgInfJobToolPriv() :
        imageInfoJob(0),
        status(NotRunning)
    {
    }

    ImageInfoJob*       imageInfoJob;

    AlbumList           palbumList;
    AlbumList::Iterator albumsIt;

    BatchAlbumsStatus   status;
};

MaintenanceImgInfJobTool::MaintenanceImgInfJobTool(const QString& id, Mode mode, int albumId)
    : MaintenanceTool(id, mode, albumId),
      d(new MaintenanceImgInfJobToolPriv)
{
    d->imageInfoJob = new ImageInfoJob();

    connect(d->imageInfoJob, SIGNAL(signalItemsInfo(ImageInfoList)),
            this, SLOT(slotAlbumItemsInfo(ImageInfoList)));

    connect(d->imageInfoJob, SIGNAL(signalCompleted()),
            this, SLOT(slotOneAlbumComplete()));
}

MaintenanceImgInfJobTool::~MaintenanceImgInfJobTool()
{
    delete d;
}

bool MaintenanceImgInfJobTool::isEmpty() const
{
    return d->palbumList.empty();
}

void MaintenanceImgInfJobTool::populateItemsToProcess()
{
    d->palbumList = AlbumManager::instance()->allPAlbums();

    setTotalItems(d->palbumList.count());

    d->albumsIt = d->palbumList.begin();
    processOne();
}

void MaintenanceImgInfJobTool::processOne()
{
    if (!checkToContinue()) return;

    if (d->status == CompletedAlbum)
    {
        advance(1);
        d->albumsIt++;
    }

    if (d->albumsIt == d->palbumList.end())     // All is done.
    {
        complete();
        return;
    }

    if ((*d->albumsIt)->isRoot())
    {
        d->status = CompletedAlbum;
        processOne();
        return;
    }

    d->status = LoadingAlbum;
    d->imageInfoJob->allItemsFromAlbum(*d->albumsIt);
    kDebug() << "Sync Items from Album :" << (*d->albumsIt)->title();
}

void MaintenanceImgInfJobTool::slotAlbumItemsInfo(const ImageInfoList& list)
{
    if (d->status == LoadingAlbum)
    {
        d->status = ReceivingImageInfos;
    }

    gotNewImageInfoList(list);
}

void MaintenanceImgInfJobTool::slotOneAlbumComplete()
{
    d->status = CompletedAlbum;
    processOne();
}

}  // namespace Digikam
