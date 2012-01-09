/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-05
 * Description : Metadata operations on images
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadatamanager.moc"
#include "metadatamanager_p.moc"

// Qt includes

#include <QMutexLocker>

// KDE includes

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "albumsettings.h"
#include "databaseoperationgroup.h"
#include "imageattributeswatch.h"
#include "loadingcacheinterface.h"
#include "metadatahub.h"
#include "metadatasettings.h"
#include "scancontroller.h"
#include "thumbnailloadthread.h"
#include "globals.h"

namespace Digikam
{

class MetadataManagerCreator
{
public:

    MetadataManager object;
};

K_GLOBAL_STATIC(MetadataManagerCreator, metadataManagercreator)

MetadataManager* MetadataManager::instance()
{
    return &metadataManagercreator->object;
}

MetadataManager::MetadataManager()
    : d(new MetadataManagerPriv(this))
{
    connect(d, SIGNAL(progressMessageChanged(QString)),
            this, SIGNAL(progressMessageChanged(QString)));

    connect(d, SIGNAL(progressValueChanged(float)),
            this, SIGNAL(progressValueChanged(float)));

    connect(d, SIGNAL(progressFinished()),
            this, SIGNAL(progressFinished()));

    connect(d->fileWorker, SIGNAL(orientationChangeFailed(QStringList)),
            this, SIGNAL(orientationChangeFailed(QStringList)));
}

MetadataManager::~MetadataManager()
{
    shutDown();
    delete d;
}

bool MetadataManager::requestShutDown()
{
    //TODO
    return true;
}

void MetadataManager::shutDown()
{
    d->dbWorker->deactivate();
    d->fileWorker->deactivate();
}

void MetadataManager::assignTags(const QList<qlonglong>& ids, const QList<int>& tagIDs)
{
    assignTags(ImageInfoList(ids), tagIDs);
}

void MetadataManager::assignTag(const ImageInfo& info, int tagID)
{
    assignTags(QList<ImageInfo>() << info, QList<int>() << tagID);
}

void MetadataManager::assignTag(const QList<ImageInfo>& infos, int tagID)
{
    assignTags(infos, QList<int>() << tagID);
}

void MetadataManager::assignTags(const ImageInfo& info, const QList<int>& tagIDs)
{
    assignTags(QList<ImageInfo>() << info, tagIDs);
}

void MetadataManager::assignTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs)
{
    d->schedulingForDB(infos.size());
    d->assignTags(infos, tagIDs);
}

void MetadataManager::removeTag(const ImageInfo& info, int tagID)
{
    removeTags(QList<ImageInfo>() << info, QList<int>() << tagID);
}

void MetadataManager::removeTag(const QList<ImageInfo>& infos, int tagID)
{
    removeTags(infos, QList<int>() << tagID);
}

void MetadataManager::removeTags(const ImageInfo& info, const QList<int>& tagIDs)
{
    removeTags(QList<ImageInfo>() << info, tagIDs);
}

void MetadataManager::removeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs)
{
    d->schedulingForDB(infos.size());
    d->removeTags(infos, tagIDs);
}

void MetadataManager::assignPickLabel(const ImageInfo& info, int pickId)
{
    assignPickLabel(QList<ImageInfo>() << info, pickId);
}

void MetadataManager::assignColorLabel(const ImageInfo& info, int colorId)
{
    assignColorLabel(QList<ImageInfo>() << info, colorId);
}

void MetadataManager::assignPickLabel(const QList<ImageInfo>& infos, int pickId)
{
    d->schedulingForDB(infos.size());
    d->assignPickLabel(infos, pickId);
}

void MetadataManager::assignColorLabel(const QList<ImageInfo>& infos, int colorId)
{
    d->schedulingForDB(infos.size());
    d->assignColorLabel(infos, colorId);
}

void MetadataManager::assignRating(const ImageInfo& info, int rating)
{
    assignRating(QList<ImageInfo>() << info, rating);
}

void MetadataManager::assignRating(const QList<ImageInfo>& infos, int rating)
{
    d->schedulingForDB(infos.size());
    d->assignRating(infos, rating);
}

void MetadataManager::addToGroup(const ImageInfo& pick, const QList<ImageInfo>& infos)
{
    d->schedulingForDB(infos.size());
    d->editGroup(AddToGroup, pick, infos);
}

void MetadataManager::removeFromGroup(const ImageInfo& info)
{
    removeFromGroup(QList<ImageInfo>() << info);
}

void MetadataManager::removeFromGroup(const QList<ImageInfo>& infos)
{
    d->schedulingForDB(infos.size());
    d->editGroup(RemoveFromGroup, ImageInfo(), infos);
}

void MetadataManager::ungroup(const ImageInfo& info)
{
    ungroup(QList<ImageInfo>() << info);
}

void MetadataManager::ungroup(const QList<ImageInfo>& infos)
{
    d->schedulingForDB(infos.size());
    d->editGroup(Ungroup, ImageInfo(), infos);
}

void MetadataManager::setExifOrientation(const QList<ImageInfo>& infos, int orientation)
{
    d->schedulingForDB(infos.size());
    d->setExifOrientation(infos, orientation);
}

void MetadataManager::applyMetadata(const QList<ImageInfo>& infos, const MetadataHub& hub)
{
    d->schedulingForDB(infos.size());
    d->applyMetadata(infos, new MetadataHubOnTheRoad(hub, this));
}

void MetadataManager::applyMetadata(const QList<ImageInfo>& infos, const MetadataHubOnTheRoad& hub)
{
    d->schedulingForDB(infos.size());
    d->applyMetadata(infos, new MetadataHubOnTheRoad(hub, this));
}

// --------------------------------------------------------------------------------------

MetadataManager::MetadataManagerPriv::MetadataManagerPriv(MetadataManager* q)
    : q(q)
{
    dbWorker   = new MetadataManagerDatabaseWorker(this);
    fileWorker = new MetadataManagerFileWorker(this);

    sleepTimer = new QTimer(this);
    sleepTimer->setSingleShot(true);
    sleepTimer->setInterval(1000);

    dbTodo     = 0;
    dbDone     = 0;
    writerTodo = 0;
    writerDone = 0;

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

    WorkerObject::connectAndSchedule(dbWorker, SIGNAL(writeMetadataToFiles(QList<ImageInfo>)),
                                     fileWorker, SLOT(writeMetadataToFiles(QList<ImageInfo>)));

    WorkerObject::connectAndSchedule(dbWorker, SIGNAL(writeOrientationToFiles(QList<ImageInfo>,int)),
                                     fileWorker, SLOT(writeOrientationToFiles(QList<ImageInfo>,int)));

    WorkerObject::connectAndSchedule(dbWorker, SIGNAL(writeMetadata(QList<ImageInfo>,MetadataHub*)),
                                     fileWorker, SLOT(writeMetadata(QList<ImageInfo>,MetadataHub*)));

    connect(fileWorker, SIGNAL(imageDataChanged(QString,bool,bool)),
            this, SLOT(slotImageDataChanged(QString,bool,bool)));

    connect(this, SIGNAL(progressFinished()),
            sleepTimer, SLOT(start()));

    connect(sleepTimer, SIGNAL(timeout()),
            this, SLOT(slotSleepTimer()));
}

MetadataManager::MetadataManagerPriv::~MetadataManagerPriv()
{
    delete dbWorker;
    delete fileWorker;
}

void MetadataManager::MetadataManagerPriv::schedulingForDB(int numberOfInfos)
{
    dbTodo += numberOfInfos;
    updateProgress();
}

void MetadataManager::MetadataManagerPriv::setDBAction(const QString& action)
{
    dbMessage = action;
    updateProgressMessage();
}

bool MetadataManager::MetadataManagerPriv::shallSendForWriting(qlonglong id)
{
    QMutexLocker lock(&mutex);

    if (scheduledToWrite.contains(id))
    {
        return false;
    }

    scheduledToWrite << id;
    return true;
}

void MetadataManager::MetadataManagerPriv::dbProcessedOne()
{
    if ( (dbDone++ % 10) == 0)
    {
        updateProgress();
    }
}

void MetadataManager::MetadataManagerPriv::dbProcessed(int numberOfInfos)
{
    dbDone += numberOfInfos;
    updateProgress();
}

void MetadataManager::MetadataManagerPriv::dbFinished(int numberOfInfos)
{
    dbTodo -= numberOfInfos;
    updateProgress();
}

void MetadataManager::MetadataManagerPriv::schedulingForWrite(int numberOfInfos)
{
    writerTodo += numberOfInfos;
    updateProgress();
}

void MetadataManager::MetadataManagerPriv::schedulingForOrientationWrite(int numberOfInfos)
{
    schedulingForWrite(numberOfInfos);
}

void MetadataManager::MetadataManagerPriv::setWriterAction(const QString& action)
{
    writerMessage = action;
    updateProgressMessage();
}

void MetadataManager::MetadataManagerPriv::startingToWrite(const QList<ImageInfo>& infos)
{
    QMutexLocker lock(&mutex);
    foreach(const ImageInfo& info, infos)
    {
        scheduledToWrite.remove(info.id());
    }
}

void MetadataManager::MetadataManagerPriv::writtenToOne()
{
    writerDone++;
    updateProgress();
}

void MetadataManager::MetadataManagerPriv::orientationWrittenToOne()
{
    writtenToOne();
}

void MetadataManager::MetadataManagerPriv::finishedWriting(int numberOfInfos)
{
    writerTodo -= numberOfInfos;
    updateProgress();
}

void MetadataManager::MetadataManagerPriv::updateProgressMessage()
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

void MetadataManager::MetadataManagerPriv::updateProgress()
{
    if (dbTodo == 0 && writerTodo == 0)
    {
        emit progressFinished();
        return;
    }

    // we use a weighting factor of 10 for file writing
    float allTodo = dbTodo + 10*writerTodo;
    float allDone = dbDone + 10*writerDone;
    float percent = allDone / allTodo;
    emit progressValueChanged(percent);
}

void MetadataManager::MetadataManagerPriv::slotImageDataChanged(const QString& path, bool removeThumbnails, bool notifyCache)
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

void MetadataManager::MetadataManagerPriv::slotSleepTimer()
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

// -------------------------------------------------------------------------------

void MetadataManagerDatabaseWorker::assignTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs)
{
    d->setDBAction(i18n("Assigning image tags. Please wait..."));
    changeTags(infos, tagIDs, true);
}

void MetadataManagerDatabaseWorker::removeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs)
{
    d->setDBAction(i18n("Removing image tags. Please wait..."));
    changeTags(infos, tagIDs, false);
}

void MetadataManagerDatabaseWorker::changeTags(const QList<ImageInfo>& infos,
                                               const QList<int>& tagIDs, bool addOrRemove)
{
    MetadataHub      hub;
    QList<ImageInfo> forWriting;

    {
        //ScanController::instance()->suspendCollectionScan();
        DatabaseOperationGroup group;
        group.setMaximumTime(200);
        foreach(const ImageInfo& info, infos)
        {

            hub.load(info);

            for (QList<int>::const_iterator tagIt = tagIDs.constBegin(); tagIt != tagIDs.constEnd(); ++tagIt)
            {
                hub.setTag(*tagIt, addOrRemove);
            }

            hub.write(info, MetadataHub::PartialWrite);

            if (hub.willWriteMetadata(MetadataHub::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
            {
                forWriting << info;
            }

            d->dbProcessedOne();
            group.allowLift();
        }
        //ScanController::instance()->resumeCollectionScan();
    }

    // send for writing file metadata
    if (!forWriting.isEmpty())
    {
        d->schedulingForWrite(forWriting.size());
        emit writeMetadataToFiles(forWriting);
    }

    d->dbFinished(infos.size());
}

void MetadataManagerDatabaseWorker::assignPickLabel(const QList<ImageInfo>& infos, int pickId)
{
    d->setDBAction(i18n("Assigning image pick label. Please wait..."));

    MetadataHub      hub;
    QList<ImageInfo> forWriting;

    {
        //ScanController::instance()->suspendCollectionScan();
        DatabaseOperationGroup group;
        group.setMaximumTime(200);
        foreach(const ImageInfo& info, infos)
        {
            hub.load(info);
            hub.setPickLabel(pickId);
            hub.write(info, MetadataHub::PartialWrite);

            if (hub.willWriteMetadata(MetadataHub::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
            {
                forWriting << info;
            }

            d->dbProcessedOne();
            group.allowLift();
        }
        //ScanController::instance()->resumeCollectionScan();
    }

    // send for writing file metadata
    if (!forWriting.isEmpty())
    {
        d->schedulingForWrite(forWriting.size());
        emit writeMetadataToFiles(forWriting);
    }

    d->dbFinished(infos.size());
}

void MetadataManagerDatabaseWorker::assignColorLabel(const QList<ImageInfo>& infos, int colorId)
{
    d->setDBAction(i18n("Assigning image color label. Please wait..."));

    MetadataHub      hub;
    QList<ImageInfo> forWriting;

    {
        //ScanController::instance()->suspendCollectionScan();
        DatabaseOperationGroup group;
        group.setMaximumTime(200);
        foreach(const ImageInfo& info, infos)
        {
            hub.load(info);
            hub.setColorLabel(colorId);
            hub.write(info, MetadataHub::PartialWrite);

            if (hub.willWriteMetadata(MetadataHub::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
            {
                forWriting << info;
            }

            d->dbProcessedOne();
            group.allowLift();
        }
        //ScanController::instance()->resumeCollectionScan();
    }

    // send for writing file metadata
    if (!forWriting.isEmpty())
    {
        d->schedulingForWrite(forWriting.size());
        emit writeMetadataToFiles(forWriting);
    }

    d->dbFinished(infos.size());
}

void MetadataManagerDatabaseWorker::assignRating(const QList<ImageInfo>& infos, int rating)
{
    d->setDBAction(i18n("Assigning image ratings. Please wait..."));

    rating = qMin(RatingMax, qMax(RatingMin, rating));
    MetadataHub      hub;
    QList<ImageInfo> forWriting;

    {
        //ScanController::instance()->suspendCollectionScan();
        DatabaseOperationGroup group;
        group.setMaximumTime(200);
        foreach(const ImageInfo& info, infos)
        {
            hub.load(info);
            hub.setRating(rating);
            hub.write(info, MetadataHub::PartialWrite);

            if (hub.willWriteMetadata(MetadataHub::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
            {
                forWriting << info;
            }

            d->dbProcessedOne();
            group.allowLift();
        }
        //ScanController::instance()->resumeCollectionScan();
    }

    // send for writing file metadata
    if (!forWriting.isEmpty())
    {
        d->schedulingForWrite(forWriting.size());
        emit writeMetadataToFiles(forWriting);
    }

    d->dbFinished(infos.size());
}

void MetadataManagerDatabaseWorker::editGroup(int groupAction, const ImageInfo& pick, const QList<ImageInfo>& infos)
{
    d->setDBAction(i18n("Editing group. Please wait..."));

    {
        DatabaseOperationGroup group;
        group.setMaximumTime(200);
        foreach(const ImageInfo& constInfo, infos)
        {
            ImageInfo info(constInfo);

            switch (groupAction)
            {
                case AddToGroup:
                    info.addToGroup(pick);
                    break;
                case RemoveFromGroup:
                    info.removeFromGroup();
                    break;
                case Ungroup:
                    info.clearGroup();
                    break;
            }

            d->dbProcessedOne();
            group.allowLift();
        }
    }
    d->dbFinished(infos.size());
}

void MetadataManagerDatabaseWorker::setExifOrientation(const QList<ImageInfo>& infos, int orientation)
{
    d->setDBAction(i18n("Updating orientation in database. Please wait..."));
    //TODO: update db
    d->dbProcessed(infos.count());
    d->schedulingForOrientationWrite(infos.count());
    emit writeOrientationToFiles(infos, orientation);
    d->dbFinished(infos.size());
}

void MetadataManagerDatabaseWorker::applyMetadata(const QList<ImageInfo>& infos, MetadataHub* hub)
{
    d->setDBAction(i18n("Applying metadata. Please wait..."));

    //ScanController::instance()->suspendCollectionScan();
    {
        DatabaseOperationGroup group;
        group.setMaximumTime(200);

        foreach(const ImageInfo& info, infos)
        {
            // apply to database
            hub->write(info);
            d->dbProcessedOne();
            group.allowLift();
        }
    }
    //ScanController::instance()->resumeCollectionScan();

    d->schedulingForWrite(infos.size());
    emit writeMetadata(infos, hub);
    d->dbFinished(infos.size());
}

// ----------------------------------------------------------------------

void MetadataManagerFileWorker::writeOrientationToFiles(const QList<ImageInfo>& infos, int orientation)
{
    d->setWriterAction(i18n("Revising Exif Orientation tags. Please wait..."));

    QStringList failedItems;

    foreach(const ImageInfo& info, infos)
    {
        //kDebug() << "Setting Exif Orientation tag to " << orientation;

        QString path                  = info.filePath();
        DMetadata metadata(path);
        DMetadata::ImageOrientation o = (DMetadata::ImageOrientation)orientation;
        metadata.setImageOrientation(o);

        if (!metadata.applyChanges())
        {
            failedItems.append(info.name());
        }
        else
        {
            emit imageDataChanged(path, true, true);
            KUrl url = KUrl::fromPath(path);
            ImageAttributesWatch::instance()->fileMetadataChanged(url);
        }

        d->orientationWrittenToOne();
    }

    if (!failedItems.isEmpty())
    {
        emit orientationChangeFailed(failedItems);
    }

    d->finishedWriting(infos.size());
}

void MetadataManagerFileWorker::writeMetadataToFiles(const QList<ImageInfo>& infos)
{
    d->setWriterAction(i18n("Writing metadata to files. Please wait..."));
    d->startingToWrite(infos);

    MetadataHub hub;

    ScanController::instance()->suspendCollectionScan();
    foreach(const ImageInfo& info, infos)
    {

        hub.load(info);
        QString filePath = info.filePath();
        bool fileChanged = hub.write(filePath, MetadataHub::FullWrite);

        if (fileChanged)
        {
            ScanController::instance()->scanFileDirectly(filePath);
        }

        // hub emits fileMetadataChanged

        d->writtenToOne();
    }
    ScanController::instance()->resumeCollectionScan();

    d->finishedWriting(infos.size());
}

void MetadataManagerFileWorker::writeMetadata(const QList<ImageInfo>& infos, MetadataHub* hub)
{
    d->setWriterAction(i18n("Writing metadata to files. Please wait..."));
    d->startingToWrite(infos);

    MetadataSettingsContainer writeSettings = MetadataSettings::instance()->settings();

    ScanController::instance()->suspendCollectionScan();
    foreach(const ImageInfo& info, infos)
    {
        QString filePath = info.filePath();

        // apply to file metadata
        bool fileChanged = hub->write(filePath, MetadataHub::FullWrite, writeSettings);

        // trigger db scan (to update file size etc.)
        if (fileChanged)
        {
            ScanController::instance()->scanFileDirectly(filePath);
        }

        // hub emits fileMetadataChanged

        d->writtenToOne();
    }
    ScanController::instance()->resumeCollectionScan();

    d->finishedWriting(infos.size());
}

} // namespace Digikam
