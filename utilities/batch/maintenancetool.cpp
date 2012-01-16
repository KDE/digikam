/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-16
 * Description : Maintenance tool class
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

#include "maintenancetool.moc"

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
#include "knotificationwrapper.h"
#include "config-digikam.h"

namespace Digikam
{

class MaintenanceTool::MaintenanceToolPriv
{
public:

    MaintenanceToolPriv() :
        cancel(false),
        mode(MaintenanceTool::AllItems),
        albumId(-1),
        thumbLoadThread(0)
    {
        duration.start();
    }

    bool                  cancel;
    QTime                 duration;
    QStringList           allPicturesPath;

    MaintenanceTool::Mode mode;
    int                   albumId;

    ThumbnailLoadThread*  thumbLoadThread;
};

MaintenanceTool::MaintenanceTool(Mode mode, int albumId)
    : ProgressItem(0,
                   ProgressManager::getUniqueID(),
                   QString(),
                   QString(),
                   true,
                   true),
      d(new MaintenanceToolPriv)
{
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();
    d->mode            = mode;
    d->albumId         = albumId;

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotGotThumbnail(LoadingDescription,QPixmap)));

    connect(this, SIGNAL(progressItemCanceled(Digikam::ProgressItem*)),
            this, SLOT(slotCancel()));

    ProgressManager::addProgressItem(this);

    QTimer::singleShot(500, this, SLOT(slotRun()));
}

MaintenanceTool::~MaintenanceTool()
{
    delete d;
}

void MaintenanceTool::setTitle(const QString& title)
{
    QString label = QString("%1: ").arg(title);

    switch(d->mode)
    {
        case AllItems:
            label.append(i18n("process all items"));
            break;
        case MissingItems:
            label.append(i18n("process missing items"));
            break;
        case AlbumItems:
            label.append(i18n("process album items"));
            break;
    }

    setLabel(label);
}

QStringList& MaintenanceTool::allPicturePath()
{
    return d->allPicturesPath;
}

MaintenanceTool::Mode MaintenanceTool::mode()
{
    return d->mode;
}

ThumbnailLoadThread* MaintenanceTool::thumbsLoadThread() const
{
    return d->thumbLoadThread;
}

void MaintenanceTool::slotRun()
{
    // Get all digiKam albums collection pictures path.
    AlbumList palbumList;

    if (d->mode != AlbumItems)
    {
        palbumList  = AlbumManager::instance()->allPAlbums();
    }
    else
    {
        palbumList.append(AlbumManager::instance()->findPAlbum(d->albumId));
        kDebug() << d->albumId;
    }

    for (AlbumList::const_iterator it = palbumList.constBegin();
         !d->cancel && (it != palbumList.constEnd()); ++it )
    {
        if (!(*it))
        {
            continue;
        }

        d->allPicturesPath += DatabaseAccess().db()->getItemURLsInAlbum((*it)->id());
    }

    kDebug() << d->allPicturesPath;

    listItemstoProcess();

    if (d->allPicturesPath.isEmpty())
    {
        slotCancel();
        return;
    }

    setTotalItems(d->allPicturesPath.count());
    processOne();
}

void MaintenanceTool::processOne()
{
    if (d->cancel || d->allPicturesPath.isEmpty())
    {
        return;
    }
}

void MaintenanceTool::complete()
{
    setComplete();
    QTime now, t = now.addMSecs(d->duration.elapsed());
    // Pop-up a message to bring user when all is done.
    KNotificationWrapper(id(),
                         i18n("Process is done.\nDuration: %1", t.toString()),
                         kapp->activeWindow(), label());
    emit signalProcessDone();
}

void MaintenanceTool::slotGotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (d->cancel || d->allPicturesPath.isEmpty())
    {
        return;
    }

    if (d->allPicturesPath.first() != desc.filePath)
    {
        return;
    }

    setThumbnail(pix);
    advance(1);

    if (!d->allPicturesPath.isEmpty())
    {
        d->allPicturesPath.removeFirst();
    }

    if (d->allPicturesPath.isEmpty())
    {
        complete();
    }
    else
    {
        processOne();
    }
}

void MaintenanceTool::slotCancel()
{
    d->cancel = true;
    emit signalProcessDone();
    setComplete();
}

}  // namespace Digikam
