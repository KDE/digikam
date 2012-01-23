/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-05
 * Description : file action manager
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

// Local includes

#include "thumbnailloadthread.h"
#include "loadingcacheinterface.h"

namespace Digikam
{

FileActionMngr::FileActionMngrPriv::FileActionMngrPriv(FileActionMngr* q)
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

    dbTodo     = 0;
    dbDone     = 0;
    writerTodo = 0;
    writerDone = 0;

    connectToDatabaseWorker();

    connectDatabaseToFileWorker();

    connect(this, SIGNAL(signalTransform(QList<ImageInfo>,int)),
            fileWorker, SLOT(transform(QList<ImageInfo>,int)), Qt::DirectConnection);

    connect(fileWorker, SIGNAL(imageDataChanged(QString,bool,bool)),
            this, SLOT(slotImageDataChanged(QString,bool,bool)));

    connect(this, SIGNAL(progressFinished()),
            sleepTimer, SLOT(start()));

    connect(sleepTimer, SIGNAL(timeout()),
            this, SLOT(slotSleepTimer()));
}

void FileActionMngr::FileActionMngrPriv::connectToDatabaseWorker()
{

    WorkerObject::connectAndSchedule(this, SIGNAL(signalAddTags(QList<ImageInfo>,QList<int>)),
                                     dbWorker, SLOT(assignTags(QList<ImageInfo>,QList<int>)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalRemoveTags(QList<ImageInfo>,QList<int>)),
                                     dbWorker, SLOT(removeTags(QList<ImageInfo>,QList<int>)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalAssignPickLabel(QList<ImageInfo>,int)),
                                     dbWorker, SLOT(assignPickLabel(QList<ImageInfo>,int)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalAssignColorLabel(QList<ImageInfo>,int)),
                                     dbWorker, SLOT(assignColorLabel(QList<ImageInfo>,int)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalAssignRating(QList<ImageInfo>,int)),
                                     dbWorker, SLOT(assignRating(QList<ImageInfo>,int)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalEditGroup(int,ImageInfo,QList<ImageInfo>)),
                                     dbWorker, SLOT(editGroup(int,ImageInfo,QList<ImageInfo>)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalSetExifOrientation(QList<ImageInfo>,int)),
                                     dbWorker, SLOT(setExifOrientation(QList<ImageInfo>,int)));

    WorkerObject::connectAndSchedule(this, SIGNAL(signalApplyMetadata(QList<ImageInfo>,MetadataHub*)),
                                     dbWorker, SLOT(applyMetadata(QList<ImageInfo>,MetadataHub*)));
}

void FileActionMngr::FileActionMngrPriv::connectDatabaseToFileWorker()
{
    connect(dbWorker, SIGNAL(writeMetadataToFiles(QList<ImageInfo>)),
            fileWorker, SLOT(writeMetadataToFiles(QList<ImageInfo>)), Qt::DirectConnection);

    connect(dbWorker, SIGNAL(writeOrientationToFiles(QList<ImageInfo>,int)),
            fileWorker, SLOT(writeOrientationToFiles(QList<ImageInfo>,int)), Qt::DirectConnection);

    connect(dbWorker, SIGNAL(writeMetadata(QList<ImageInfo>,MetadataHub*)),
            fileWorker, SLOT(writeMetadata(QList<ImageInfo>,MetadataHub*)), Qt::DirectConnection);
}

FileActionMngr::FileActionMngrPriv::~FileActionMngrPriv()
{
    delete dbWorker;
    delete fileWorker;
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
    if (dbTodo == 0)
    {
        dbWorker->deactivate();
    }

    if (writerTodo == 0)
    {
        fileWorker->deactivate();
    }
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

    emit progressMessageChanged(message);
}

void FileActionMngr::FileActionMngrPriv::updateProgress()
{
    if (!q->isActive())
    {
        dbDone     = 0;
        writerDone = 0;
        emit progressFinished();
        return;
    }

    float dbPercent     = float(dbDone) / float(qMax(1, dbTodo));
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

    emit progressValueChanged(percent);
    emit progressValueChanged(int(percent*100));
}

} // namespace Digikam
