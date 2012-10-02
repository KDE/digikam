/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-05
 * Description : file action manager
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "fileactionmngr_p.moc"

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// Local includes

#include "thumbnailloadthread.h"
#include "loadingcacheinterface.h"

namespace Digikam
{

FileActionMngr::FileActionMngrPriv::FileActionMngrPriv(FileActionMngr* const q)
    : q(q)
{
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

    connect(this, SIGNAL(signalTransform(FileActionImageInfoList, int)),
            fileWorker, SLOT(transform(FileActionImageInfoList, int)), Qt::DirectConnection);

    connect(fileWorker, SIGNAL(imageDataChanged(QString, bool, bool)),
            this, SLOT(slotImageDataChanged(QString, bool, bool)));

    connect(&dbProgress, SIGNAL(lastItemCompleted()),
            this, SLOT(slotLastProgressItemCompleted()));

    connect(&fileProgress, SIGNAL(lastItemCompleted()),
            this, SLOT(slotLastProgressItemCompleted()));

    connect(sleepTimer, SIGNAL(timeout()),
            this, SLOT(slotSleepTimer()));
}

void FileActionMngr::FileActionMngrPriv::connectToDatabaseWorker()
{

    WorkerObject::connectAndSchedule(this, SIGNAL(signalAddTags(FileActionImageInfoList, QList<int>)),
                                     dbWorker, SLOT(assignTags(FileActionImageInfoList, QList<int>)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalRemoveTags(FileActionImageInfoList, QList<int>)),
                                     dbWorker, SLOT(removeTags(FileActionImageInfoList, QList<int>)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalAssignPickLabel(FileActionImageInfoList, int)),
                                     dbWorker, SLOT(assignPickLabel(FileActionImageInfoList, int)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalAssignColorLabel(FileActionImageInfoList, int)),
                                     dbWorker, SLOT(assignColorLabel(FileActionImageInfoList, int)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalAssignRating(FileActionImageInfoList, int)),
                                     dbWorker, SLOT(assignRating(FileActionImageInfoList, int)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalEditGroup(int, ImageInfo, FileActionImageInfoList)),
                                     dbWorker, SLOT(editGroup(int, ImageInfo, FileActionImageInfoList)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalSetExifOrientation(FileActionImageInfoList, int)),
                                     dbWorker, SLOT(setExifOrientation(FileActionImageInfoList, int)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalApplyMetadata(FileActionImageInfoList, MetadataHub*)),
                                     dbWorker, SLOT(applyMetadata(FileActionImageInfoList, MetadataHub*)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalCopyAttributes(FileActionImageInfoList, QStringList)),
                                     dbWorker, SLOT(copyAttributes(FileActionImageInfoList, QStringList)));
}

void FileActionMngr::FileActionMngrPriv::connectDatabaseToFileWorker()
{
    connect(dbWorker, SIGNAL(writeMetadataToFiles(FileActionImageInfoList)),
            fileWorker, SLOT(writeMetadataToFiles(FileActionImageInfoList)), Qt::DirectConnection);

    connect(dbWorker, SIGNAL(writeOrientationToFiles(FileActionImageInfoList, int)),
            fileWorker, SLOT(writeOrientationToFiles(FileActionImageInfoList, int)), Qt::DirectConnection);

    connect(dbWorker, SIGNAL(writeMetadata(FileActionImageInfoList, MetadataHub*)),
            fileWorker, SLOT(writeMetadata(FileActionImageInfoList, MetadataHub*)), Qt::DirectConnection);
}

FileActionMngr::FileActionMngrPriv::~FileActionMngrPriv()
{
    delete dbWorker;
    delete fileWorker;
}

bool FileActionMngr::FileActionMngrPriv::isActive() const
{
    return dbProgress.activeProgressItems || fileProgress.activeProgressItems;
}

bool FileActionMngr::FileActionMngrPriv::shallSendForWriting(qlonglong id)
{
    QMutexLocker lock(&mutex);

    if (scheduledToWrite.contains(id))
    {
        return false;
    }

    scheduledToWrite << id;
    return true;
}

void FileActionMngr::FileActionMngrPriv::startingToWrite(const QList<ImageInfo>& infos)
{
    QMutexLocker lock(&mutex);
    foreach(const ImageInfo& info, infos)
    {
        scheduledToWrite.remove(info.id());
    }
}

void FileActionMngr::FileActionMngrPriv::slotSleepTimer()
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

void FileActionMngr::FileActionMngrPriv::slotLastProgressItemCompleted()
{
    if (!isActive())
    {
        emit signalTasksFinished();
    }
    sleepTimer->start();
}

void FileActionMngr::FileActionMngrPriv::slotImageDataChanged(const QString& path, bool removeThumbnails, bool notifyCache)
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

FileActionMngrPrivProgressItemCreator* FileActionMngr::FileActionMngrPriv::dbProgressCreator()
{
    return &dbProgress;
}

FileActionMngrPrivProgressItemCreator* FileActionMngr::FileActionMngrPriv::fileProgressCreator()
{
    return &fileProgress;
}

ProgressItem* FileActionMngrPrivProgressItemCreator::createProgressItem(const QString& action) const
{
    return new ProgressItem(0, ProgressManager::instance()->getUniqueID(), action, QString(), true, true);
    /*
    if (!parentProgressItems.first())
    {
        parentProgressItems.createFirstItem(i18n("Editing Database"));
    }
    return parentProgressItems.first();
    */
    /*
    if (!parentProgressItems.second())
    {
        parentProgressItems.createSecondItem(i18n("Writing to Files"));
    }
    return parentProgressItems.second();
    */
}

void FileActionMngrPrivProgressItemCreator::addProgressItem(ProgressItem* const item)
{
    activeProgressItems.ref();

    connect(item, SIGNAL(progressItemCompleted(ProgressItem*)),
            this, SLOT(slotProgressItemCompleted()),
            Qt::DirectConnection
           );

    ProgressManager::addProgressItem(item);
}

void FileActionMngrPrivProgressItemCreator::slotProgressItemCompleted()
{
    activeProgressItems.deref();
}

/*
void FileActionMngr::FileActionMngrPriv::setDBAction(const QString& action)
{
    dbMessage = action;
    updateProgressMessage();
}

void FileActionMngr::FileActionMngrPriv::schedulingForDB(int numberOfInfos)
{
    dbTodo += numberOfInfos;
    updateProgress();
}

void FileActionMngr::FileActionMngrPriv::dbProcessedOne()
{
    if ( (dbDone++ % 10) == 0)
    {
        updateProgress();
    }
}

void FileActionMngr::FileActionMngrPriv::dbProcessed(int numberOfInfos)
{
    dbDone += numberOfInfos;
    updateProgress();
}

void FileActionMngr::FileActionMngrPriv::dbFinished(int numberOfInfos)
{
    dbTodo -= numberOfInfos;
    dbDone -= numberOfInfos;
    updateProgress();
}

void FileActionMngr::FileActionMngrPriv::schedulingForWrite(int numberOfInfos)
{
    writerTodo += numberOfInfos;
    updateProgress();
}

void FileActionMngr::FileActionMngrPriv::schedulingForOrientationWrite(int numberOfInfos)
{
    schedulingForWrite(numberOfInfos);
}

void FileActionMngr::FileActionMngrPriv::setWriterAction(const QString& action)
{
    writerMessage = action;
    updateProgressMessage();
}

void FileActionMngr::FileActionMngrPriv::writtenToOne()
{
    writerDone++;
    updateProgress();
}

void FileActionMngr::FileActionMngrPriv::finishedWriting(int numberOfInfos)
{
    writerTodo -= numberOfInfos;
    writerDone -= numberOfInfos;
    updateProgress();
}

void FileActionMngr::FileActionMngrPriv::updateProgressMessage()
{
    QString message;

    if (dbTodo && writerTodo)
    {
        message = dbMessage;
    }
    else if (dbTodo)
    {
        message = dbMessage;
    }
    else if (writerTodo)
    {
        message = writerMessage;
    }

    emit signalProgressMessageChanged(message);
}

void FileActionMngr::FileActionMngrPriv::updateProgress()
{
    if (!q->isActive())
    {
        dbDone     = 0;
        writerDone = 0;
        emit signalProgressFinished();
        return;
    }

    float dbPercent     = float(dbDone)     / float(qMax(1, dbTodo));
    float writerPercent = float(writerDone) / float(qMax(1, writerTodo));
    float percent;

    if (dbTodo && writerTodo)
    {
        // we use a weighting factor of 10 for file writing
        percent = 0.1 * dbPercent + 0.9 * writerPercent;
    }
    else
    {
        percent = dbPercent + writerPercent;
    }

    emit signalProgressValueChanged(percent);
}
*/

} // namespace Digikam
