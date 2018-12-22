/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-05
 * Description : file action manager
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "fileactionmngr_p.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "thumbnailloadthread.h"
#include "loadingcacheinterface.h"

namespace Digikam
{

FileActionMngr::Private::Private(FileActionMngr* const qq)
    : q(qq)
{
    qRegisterMetaType<MetadataHub*>("MetadataHub*");
    qRegisterMetaType<FileActionItemInfoList>("FileActionItemInfoList");
    qRegisterMetaType<FileActionItemInfoList*>("FileActionItemInfoList*");
    qRegisterMetaType<QList<ItemInfo> >("QList<ItemInfo>");

    dbWorker   = new FileActionMngrDatabaseWorker(this);
    fileWorker = new ParallelAdapter<FileWorkerInterface>();

    while (!fileWorker->optimalWorkerCountReached())
    {
        fileWorker->add(new FileActionMngrFileWorker(this));
    }

    sleepTimer = new QTimer(this);
    sleepTimer->setSingleShot(true);
    sleepTimer->setInterval(1000);

    connectToDatabaseWorker();

    connectDatabaseToFileWorker();

    connect(this, SIGNAL(signalTransform(FileActionItemInfoList,int)),
            fileWorker, SLOT(transform(FileActionItemInfoList,int)),
            Qt::DirectConnection);

    fileWorker->connect(SIGNAL(imageDataChanged(QString,bool,bool)),
                        this, SLOT(slotImageDataChanged(QString,bool,bool)));

    connect(&dbProgress, SIGNAL(lastItemCompleted()),
            this, SLOT(slotLastProgressItemCompleted()));

    connect(&fileProgress, SIGNAL(lastItemCompleted()),
            this, SLOT(slotLastProgressItemCompleted()));

    connect(sleepTimer, SIGNAL(timeout()),
            this, SLOT(slotSleepTimer()));
}

void FileActionMngr::Private::connectToDatabaseWorker()
{

    WorkerObject::connectAndSchedule(this, SIGNAL(signalAddTags(FileActionItemInfoList,QList<int>)),
                                     dbWorker, SLOT(assignTags(FileActionItemInfoList,QList<int>)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalRemoveTags(FileActionItemInfoList,QList<int>)),
                                     dbWorker, SLOT(removeTags(FileActionItemInfoList,QList<int>)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalAssignPickLabel(FileActionItemInfoList,int)),
                                     dbWorker, SLOT(assignPickLabel(FileActionItemInfoList,int)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalAssignColorLabel(FileActionItemInfoList,int)),
                                     dbWorker, SLOT(assignColorLabel(FileActionItemInfoList,int)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalAssignRating(FileActionItemInfoList,int)),
                                     dbWorker, SLOT(assignRating(FileActionItemInfoList,int)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalEditGroup(int,ItemInfo,FileActionItemInfoList)),
                                     dbWorker, SLOT(editGroup(int,ItemInfo,FileActionItemInfoList)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalSetExifOrientation(FileActionItemInfoList,int)),
                                     dbWorker, SLOT(setExifOrientation(FileActionItemInfoList,int)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalApplyMetadata(FileActionItemInfoList,DisjointMetadata*)),
                                     dbWorker, SLOT(applyMetadata(FileActionItemInfoList,DisjointMetadata*)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalCopyAttributes(FileActionItemInfoList,QStringList)),
                                     dbWorker, SLOT(copyAttributes(FileActionItemInfoList,QStringList)));
}

void FileActionMngr::Private::connectDatabaseToFileWorker()
{
    connect(dbWorker, SIGNAL(writeMetadataToFiles(FileActionItemInfoList)),
            fileWorker, SLOT(writeMetadataToFiles(FileActionItemInfoList)),
            Qt::DirectConnection);

    connect(dbWorker, SIGNAL(writeMetadata(FileActionItemInfoList,int)),
            fileWorker, SLOT(writeMetadata(FileActionItemInfoList,int)),
            Qt::DirectConnection);

    connect(dbWorker, SIGNAL(writeOrientationToFiles(FileActionItemInfoList,int)),
            fileWorker, SLOT(writeOrientationToFiles(FileActionItemInfoList,int)),
            Qt::DirectConnection);

}

FileActionMngr::Private::~Private()
{
    delete dbWorker;
    delete fileWorker;
}

bool FileActionMngr::Private::isActive() const
{
    return dbProgress.activeProgressItems || fileProgress.activeProgressItems;
}

bool FileActionMngr::Private::shallSendForWriting(qlonglong id)
{
    QMutexLocker lock(&mutex);

    if (scheduledToWrite.contains(id))
    {
        return false;
    }

    scheduledToWrite << id;
    return true;
}

void FileActionMngr::Private::startingToWrite(const QList<ItemInfo>& infos)
{
    QMutexLocker lock(&mutex);

    foreach (const ItemInfo& info, infos)
    {
        scheduledToWrite.remove(info.id());
    }
}

void FileActionMngr::Private::slotSleepTimer()
{
    if (!dbProgress.activeProgressItems)
    {
        dbWorker->deactivate();
    }

    if (!fileProgress.activeProgressItems)
    {
        fileWorker->deactivate();
    }
}

void FileActionMngr::Private::slotLastProgressItemCompleted()
{
    if (!isActive())
    {
        emit signalTasksFinished();
    }
    sleepTimer->start();
}

void FileActionMngr::Private::slotImageDataChanged(const QString& path, bool removeThumbnails, bool notifyCache)
{
    // must be done from the UI thread, touches pixmaps
    if (removeThumbnails)
    {
        ThumbnailLoadThread::deleteThumbnail(path);
    }

    if (notifyCache)
    {
        LoadingCacheInterface::fileChanged(path);
    }
}

PrivateProgressItemCreator* FileActionMngr::Private::dbProgressCreator()
{
    return &dbProgress;
}

PrivateProgressItemCreator* FileActionMngr::Private::fileProgressCreator()
{
    return &fileProgress;
}

ProgressItem* PrivateProgressItemCreator::createProgressItem(const QString& action) const
{
    return new ProgressItem(0, ProgressManager::instance()->getUniqueID(), action, QString(), true, true);
}

void PrivateProgressItemCreator::addProgressItem(ProgressItem* const item)
{
    activeProgressItems.ref();

    connect(item, SIGNAL(progressItemCompleted(ProgressItem*)),
            this, SLOT(slotProgressItemCompleted()),
            Qt::DirectConnection);

    connect(item, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotProgressItemCanceled(ProgressItem*)),
            Qt::DirectConnection);

    ProgressManager::addProgressItem(item);
}

void PrivateProgressItemCreator::slotProgressItemCompleted()
{
    if (!activeProgressItems.deref())
    {
        emit lastItemCompleted();
    }
}

void PrivateProgressItemCreator::slotProgressItemCanceled(ProgressItem* item)
{
    FileActionMngr::instance()->shutDown();
    item->setComplete();
}

} // namespace Digikam
