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

#include "fileactionmngr.moc"
#include "fileactionmngr_p.moc"

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
#include "jpegutils.h"
#include "dimg.h"

namespace Digikam
{

class FileActionMngrCreator
{
public:

    FileActionMngr object;
};

K_GLOBAL_STATIC(FileActionMngrCreator, metadataManagercreator)

FileActionMngr* FileActionMngr::instance()
{
    return &metadataManagercreator->object;
}

FileActionMngr::FileActionMngr()
    : d(new FileActionMngrPriv(this))
{
    connect(d, SIGNAL(progressMessageChanged(QString)),
            this, SIGNAL(progressMessageChanged(QString)));

    connect(d, SIGNAL(progressValueChanged(float)),
            this, SIGNAL(progressValueChanged(float)));

    connect(d, SIGNAL(progressFinished()),
            this, SIGNAL(progressFinished()));

    connect(d->fileWorker, SIGNAL(imageChangeFailed(QString, QStringList)),
            this, SIGNAL(imageChangeFailed(QString, QStringList)));
}

FileActionMngr::~FileActionMngr()
{
    shutDown();
    delete d;
}

bool FileActionMngr::requestShutDown()
{
    //TODO
    return true;
}

void FileActionMngr::shutDown()
{
    d->dbWorker->deactivate();
    d->fileWorker->deactivate();
}

void FileActionMngr::assignTags(const QList<qlonglong>& ids, const QList<int>& tagIDs)
{
    assignTags(ImageInfoList(ids), tagIDs);
}

void FileActionMngr::assignTag(const ImageInfo& info, int tagID)
{
    assignTags(QList<ImageInfo>() << info, QList<int>() << tagID);
}

void FileActionMngr::assignTag(const QList<ImageInfo>& infos, int tagID)
{
    assignTags(infos, QList<int>() << tagID);
}

void FileActionMngr::assignTags(const ImageInfo& info, const QList<int>& tagIDs)
{
    assignTags(QList<ImageInfo>() << info, tagIDs);
}

void FileActionMngr::assignTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs)
{
    d->schedulingForDB(infos.size());
    d->assignTags(infos, tagIDs);
}

void FileActionMngr::removeTag(const ImageInfo& info, int tagID)
{
    removeTags(QList<ImageInfo>() << info, QList<int>() << tagID);
}

void FileActionMngr::removeTag(const QList<ImageInfo>& infos, int tagID)
{
    removeTags(infos, QList<int>() << tagID);
}

void FileActionMngr::removeTags(const ImageInfo& info, const QList<int>& tagIDs)
{
    removeTags(QList<ImageInfo>() << info, tagIDs);
}

void FileActionMngr::removeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs)
{
    d->schedulingForDB(infos.size());
    d->removeTags(infos, tagIDs);
}

void FileActionMngr::assignPickLabel(const ImageInfo& info, int pickId)
{
    assignPickLabel(QList<ImageInfo>() << info, pickId);
}

void FileActionMngr::assignColorLabel(const ImageInfo& info, int colorId)
{
    assignColorLabel(QList<ImageInfo>() << info, colorId);
}

void FileActionMngr::assignPickLabel(const QList<ImageInfo>& infos, int pickId)
{
    d->schedulingForDB(infos.size());
    d->assignPickLabel(infos, pickId);
}

void FileActionMngr::assignColorLabel(const QList<ImageInfo>& infos, int colorId)
{
    d->schedulingForDB(infos.size());
    d->assignColorLabel(infos, colorId);
}

void FileActionMngr::assignRating(const ImageInfo& info, int rating)
{
    assignRating(QList<ImageInfo>() << info, rating);
}

void FileActionMngr::assignRating(const QList<ImageInfo>& infos, int rating)
{
    d->schedulingForDB(infos.size());
    d->assignRating(infos, rating);
}

void FileActionMngr::addToGroup(const ImageInfo& pick, const QList<ImageInfo>& infos)
{
    d->schedulingForDB(infos.size());
    d->editGroup(AddToGroup, pick, infos);
}

void FileActionMngr::removeFromGroup(const ImageInfo& info)
{
    removeFromGroup(QList<ImageInfo>() << info);
}

void FileActionMngr::removeFromGroup(const QList<ImageInfo>& infos)
{
    d->schedulingForDB(infos.size());
    d->editGroup(RemoveFromGroup, ImageInfo(), infos);
}

void FileActionMngr::ungroup(const ImageInfo& info)
{
    ungroup(QList<ImageInfo>() << info);
}

void FileActionMngr::ungroup(const QList<ImageInfo>& infos)
{
    d->schedulingForDB(infos.size());
    d->editGroup(Ungroup, ImageInfo(), infos);
}

void FileActionMngr::setExifOrientation(const QList<ImageInfo>& infos, int orientation)
{
    d->schedulingForDB(infos.size());
    d->setExifOrientation(infos, orientation);
}

void FileActionMngr::applyMetadata(const QList<ImageInfo>& infos, const MetadataHub& hub)
{
    d->schedulingForDB(infos.size());
    d->applyMetadata(infos, new MetadataHubOnTheRoad(hub, this));
}

void FileActionMngr::applyMetadata(const QList<ImageInfo>& infos, const MetadataHubOnTheRoad& hub)
{
    d->schedulingForDB(infos.size());
    d->applyMetadata(infos, new MetadataHubOnTheRoad(hub, this));
}

void FileActionMngr::rotate(const QList<ImageInfo>& infos, int orientation)
{
    d->rotate(infos, orientation);
}

// --------------------------------------------------------------------------------------

FileActionMngr::FileActionMngrPriv::FileActionMngrPriv(FileActionMngr* q)
    : q(q)
{
    dbWorker   = new FileActionMngrDatabaseWorker(this);
    fileWorker = new FileActionMngrFileWorker(this);

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

    WorkerObject::connectAndSchedule(this, SIGNAL(signalRotate(QList<ImageInfo>,int)),
                                     fileWorker, SLOT(rotate(QList<ImageInfo>,int)));

    connect(fileWorker, SIGNAL(imageDataChanged(QString,bool,bool)),
            this, SLOT(slotImageDataChanged(QString,bool,bool)));

    connect(this, SIGNAL(progressFinished()),
            sleepTimer, SLOT(start()));

    connect(sleepTimer, SIGNAL(timeout()),
            this, SLOT(slotSleepTimer()));
}

FileActionMngr::FileActionMngrPriv::~FileActionMngrPriv()
{
    delete dbWorker;
    delete fileWorker;
}

void FileActionMngr::FileActionMngrPriv::schedulingForDB(int numberOfInfos)
{
    dbTodo += numberOfInfos;
    updateProgress();
}

void FileActionMngr::FileActionMngrPriv::setDBAction(const QString& action)
{
    dbMessage = action;
    updateProgressMessage();
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

void FileActionMngr::FileActionMngrPriv::startingToWrite(const QList<ImageInfo>& infos)
{
    QMutexLocker lock(&mutex);
    foreach(const ImageInfo& info, infos)
    {
        scheduledToWrite.remove(info.id());
    }
}

void FileActionMngr::FileActionMngrPriv::writtenToOne()
{
    writerDone++;
    updateProgress();
}

void FileActionMngr::FileActionMngrPriv::finishedWriting(int numberOfInfos)
{
    writerTodo -= numberOfInfos;
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

// -------------------------------------------------------------------------------

void FileActionMngrDatabaseWorker::assignTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs)
{
    d->setDBAction(i18n("Assigning image tags. Please wait..."));
    changeTags(infos, tagIDs, true);
}

void FileActionMngrDatabaseWorker::removeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs)
{
    d->setDBAction(i18n("Removing image tags. Please wait..."));
    changeTags(infos, tagIDs, false);
}

void FileActionMngrDatabaseWorker::changeTags(const QList<ImageInfo>& infos,
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

void FileActionMngrDatabaseWorker::assignPickLabel(const QList<ImageInfo>& infos, int pickId)
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

void FileActionMngrDatabaseWorker::assignColorLabel(const QList<ImageInfo>& infos, int colorId)
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

void FileActionMngrDatabaseWorker::assignRating(const QList<ImageInfo>& infos, int rating)
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

void FileActionMngrDatabaseWorker::editGroup(int groupAction, const ImageInfo& pick, const QList<ImageInfo>& infos)
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

void FileActionMngrDatabaseWorker::setExifOrientation(const QList<ImageInfo>& infos, int orientation)
{
    d->setDBAction(i18n("Updating orientation in database. Please wait..."));
    //TODO: update db
    d->dbProcessed(infos.count());
    d->schedulingForOrientationWrite(infos.count());
    emit writeOrientationToFiles(infos, orientation);
    d->dbFinished(infos.size());
}

void FileActionMngrDatabaseWorker::applyMetadata(const QList<ImageInfo>& infos, MetadataHub* hub)
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

void FileActionMngrFileWorker::writeOrientationToFiles(const QList<ImageInfo>& infos, int orientation)
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

        d->writtenToOne();
    }

    if (!failedItems.isEmpty())
    {
        emit imageChangeFailed(i18n("Failed to revise Exif orientation these files:"), failedItems);
    }

    d->finishedWriting(infos.size());
}

void FileActionMngrFileWorker::writeMetadataToFiles(const QList<ImageInfo>& infos)
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

void FileActionMngrFileWorker::writeMetadata(const QList<ImageInfo>& infos, MetadataHub* hub)
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

void FileActionMngrFileWorker::rotate(const QList<ImageInfo>& infos, int orientation)
{
    d->setWriterAction(i18n("Rotate items. Please wait..."));
    d->startingToWrite(infos);

    bool useExif = (orientation == -1);
    QStringList failedItems;
    ScanController::instance()->suspendCollectionScan();

    foreach(const ImageInfo& info, infos)
    {
        KUrl url = info.fileUrl();

        if (isJpegImage(url.toLocalFile()))
        {
            if (useExif)
            {
                if (!exifTransform(url.toLocalFile(), url.fileName(), url.toLocalFile(), Auto))
                {
                    failedItems.append(info.name());
                }
            }
            else
            {
                switch (orientation)
                {
                    case DImg::ROT90:
                        if (!exifTransform(url.toLocalFile(), url.fileName(), url.toLocalFile(), Rotate90))
                            failedItems.append(info.name());
                        break;
                    case DImg::ROT180:
                        if (!exifTransform(url.toLocalFile(), url.fileName(), url.toLocalFile(), Rotate180))
                            failedItems.append(info.name());
                        break;
                    case DImg::ROT270:
                        if (!exifTransform(url.toLocalFile(), url.fileName(), url.toLocalFile(), Rotate270))
                            failedItems.append(info.name());
                        break;
                    default:
                        break;
                }
            }
        }
        else
        {
            // Non-JPEG image: DImg
            DImg image;

            if (!image.load(url.toLocalFile()))
            {
                failedItems.append(info.name());
            }
            else
            {
                if (useExif)
                {
                    DMetadata meta(url.toLocalFile());
                    image.rotateAndFlip(meta.getImageOrientation());
                }
                else
                {
                    image.rotate((DImg::ANGLE)orientation);
                }

                if (!image.save(url.toLocalFile(), DImg::fileFormat(url.toLocalFile())))
                {
                    failedItems.append(info.name());
                }
            }
        }

        if (!failedItems.contains(info.name()))
        {
            emit imageDataChanged(url.toLocalFile(), true, true);
            ImageAttributesWatch::instance()->fileMetadataChanged(url);
        }

        d->writtenToOne();
    }

    if (!failedItems.isEmpty())
    {
        emit imageChangeFailed(i18n("Failed to rotate these files:"), failedItems);
    }

    ScanController::instance()->resumeCollectionScan();
    d->finishedWriting(infos.size());
}

} // namespace Digikam
