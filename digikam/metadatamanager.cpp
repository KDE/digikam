/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-05
 * Description : Metadata operations on images
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#include "metadatamanager.h"
#include "metadatamanager.moc"
#include "metadatamanager.h"
#include "metadatamanager_p.moc"

// Qt includes

#include <QMutexLocker>

// KDE includes

#include <kglobal.h>
#include <klocale.h>

// Local includes

#include "constants.h"
#include "databasetransaction.h"
#include "imageattributeswatch.h"
#include "metadatahub.h"
#include "scancontroller.h"

namespace Digikam
{

class MetadataManagerCreator { public: MetadataManager object; };
K_GLOBAL_STATIC(MetadataManagerCreator, creator)

MetadataManager* MetadataManager::instance()
{
    return &creator->object;
}

MetadataManager::MetadataManager()
              : d(new MetadataManagerPriv)
{
    connect(d, SIGNAL(progressMessageChanged(const QString &)),
            this, SIGNAL(progressMessageChanged(const QString &)));

    connect(d, SIGNAL(progressValueChanged(int)),
            this, SIGNAL(progressValueChanged(int)));

    connect(d, SIGNAL(progressFinished()),
            this, SIGNAL(progressFinished()));

    connect(d->fileWorker, SIGNAL(orientationChangeFailed(const QStringList &)),
            this, SIGNAL(orientationChangeFailed(const QStringList &)));
}

MetadataManager::~MetadataManager()
{
    delete d;
}

bool MetadataManager::requestShutDown()
{
    //TODO
    return true;
}

void MetadataManager::shutDown()
{
    d->dbWorker->shutDown();
    d->fileWorker->shutDown();
}

void MetadataManager::assignTags(const QList<ImageInfo> &infos, const QList<int> &tagIDs)
{
    d->schedulingForDB(infos.size());
    d->assignTags(infos, tagIDs);
}

void MetadataManager::removeTags(const QList<ImageInfo> &infos, const QList<int> &tagIDs)
{
    d->schedulingForDB(infos.size());
    d->removeTags(infos, tagIDs);
}

void MetadataManager::assignRating(const QList<ImageInfo> &infos, int rating)
{
    d->schedulingForDB(infos.size());
    d->assignRating(infos, rating);
}

void MetadataManager::setExifOrientation(const QList<ImageInfo> &infos, int orientation)
{
    d->schedulingForDB(infos.size());
    d->setExifOrientation(infos, orientation);
}

// -------------

MetadataManagerPriv::MetadataManagerPriv()
{
    dbWorker = new MetadataManagerDatabaseWorker(this);
    fileWorker = new MetadataManagerFileWorker(this);

    connect(this, SIGNAL(signalAddTags(const QList<ImageInfo> &, const QList<int> &)),
            dbWorker, SLOT(assignTags(const QList<ImageInfo> &, const QList<int> &)));

    connect(this, SIGNAL(signalRemoveTags(const QList<ImageInfo> &, const QList<int> &)),
            dbWorker, SLOT(removeTags(const QList<ImageInfo> &, const QList<int> &)));

    connect(this, SIGNAL(signalAssignRating(const QList<ImageInfo> &, int)),
            dbWorker, SLOT(assignRating(const QList<ImageInfo> &, int)));

    connect(this, SIGNAL(signalSetExifOrientation(const QList<ImageInfo> &, int)),
            dbWorker, SLOT(setExifOrientation(const QList<ImageInfo> &, int)));

    connect(dbWorker, SIGNAL(writeMetadataToFiles(const QList<ImageInfo> &, int)),
            fileWorker, SLOT(writeMetadataToFiles(const QList<ImageInfo> &, int)));

    connect(dbWorker, SIGNAL(writeOrientationToFiles(const QList<ImageInfo> &, int)),
            fileWorker, SLOT(writeOrientationToFiles(const QList<ImageInfo> &, int)));
}

void MetadataManagerPriv::schedulingForDB(int numberOfInfos)
{
    dbTodo += numberOfInfos;
    updateProgress();
}

void MetadataManagerPriv::setDBAction(const QString &action)
{
    dbMessage = action;
    updateProgressMessage();
}

bool MetadataManagerPriv::shallSendForWriting(qlonglong id)
{
    QMutexLocker lock(&mutex);
    if (scheduledToWrite.contains(id))
        return false;
    scheduledToWrite << id;
    return true;
}

void MetadataManagerPriv::dbProcessedOne()
{
    if ( (dbDone++ % 10) == 0)
        updateProgress();
}

void MetadataManagerPriv::dbProcessed(int numberOfInfos)
{
    dbDone += numberOfInfos;
    updateProgress();
}

void MetadataManagerPriv::dbFinished(int numberOfInfos)
{
    dbTodo -= numberOfInfos;
    updateProgress();
}

void MetadataManagerPriv::schedulingForWrite(int numberOfInfos)
{
    writerTodo += numberOfInfos;
    updateProgress();
}

void MetadataManagerPriv::schedulingForOrientationWrite(int numberOfInfos)
{
    schedulingForWrite(numberOfInfos);
}

void MetadataManagerPriv::setWriterAction(const QString &action)
{
    writerMessage = action;
    updateProgressMessage();
}

void MetadataManagerPriv::startingToWrite(const QList<ImageInfo> &infos)
{
    QMutexLocker lock(&mutex);
    foreach (const ImageInfo &info, infos)
        scheduledToWrite.remove(info.id());
}

void MetadataManagerPriv::writtenToOne()
{
    writerDone++;
    updateProgress();
}

void MetadataManagerPriv::orientationWrittenToOne()
{
    writtenToOne();
}

void MetadataManagerPriv::finishedWriting(int numberOfInfos)
{
    writerTodo -= numberOfInfos;
    updateProgress();
}

void MetadataManagerPriv::updateProgressMessage()
{
    QString message;
    if (dbTodo && writerTodo)
        message = dbMessage;
    else if (dbTodo)
        message = dbMessage;
    else if (writerTodo)
        message = writerMessage;
    emit progressMessageChanged(message);
}

void MetadataManagerPriv::updateProgress()
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

// -------------

void MetadataManagerDatabaseWorker::assignTags(const QList<ImageInfo> &infos, const QList<int> &tagIDs)
{
    d->setDBAction(i18n("Assigning image tags. Please wait..."));
    changeTags(infos, tagIDs, true);
}

void MetadataManagerDatabaseWorker::removeTags(const QList<ImageInfo> &infos, const QList<int> &tagIDs)
{
    d->setDBAction(i18n("Removing image tags. Please wait..."));
    changeTags(infos, tagIDs, false);
}

void MetadataManagerDatabaseWorker::changeTags(const QList<ImageInfo> &infos, const QList<int> &tagIDs, bool addOrRemove)
{
    MetadataHub hub;

    QList<ImageInfo> forWriting;

    ScanController::instance()->suspendCollectionScan();
    DatabaseTransaction transaction;
    foreach(const ImageInfo &info, infos)
    {

        hub.load(info);

        for (QList<int>::const_iterator tagIt = tagIDs.constBegin(); tagIt != tagIDs.constEnd(); ++tagIt)
        {
            hub.setTag(*tagIt, addOrRemove);
        }

        hub.write(info, MetadataHub::PartialWrite);

        if (hub.willWriteMetadata(MetadataHub::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
            forWriting << info;

        d->dbProcessedOne();
    }
    ScanController::instance()->resumeCollectionScan();

    // send for writing file metadata
    if (!forWriting.isEmpty())
    {
        d->schedulingForWrite(forWriting.size());
        emit writeMetadataToFiles(forWriting);
    }

    d->dbFinished(infos.size());
}

void MetadataManagerDatabaseWorker::assignRating(const QList<ImageInfo> &infos, int rating)
{
    d->setDBAction(i18n("Assigning image ratings. Please wait..."));

    rating = qMin(RatingMax, qMax(RatingMin, rating));
    MetadataHub hub;

    QList<ImageInfo> forWriting;

    ScanController::instance()->suspendCollectionScan();
    DatabaseTransaction transaction;
    foreach (const ImageInfo &info, infos)
    {
        hub.load(info);
        hub.setRating(rating);
        hub.write(info, MetadataHub::PartialWrite);

        if (hub.willWriteMetadata(MetadataHub::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
            forWriting << info;

        d->dbProcessedOne();
    }
    ScanController::instance()->resumeCollectionScan();

    // send for writing file metadata
    if (!forWriting.isEmpty())
    {
        d->schedulingForWrite(forWriting.size());
        emit writeMetadataToFiles(forWriting);
    }

    d->dbFinished(infos.size());
}

void MetadataManagerDatabaseWorker::setExifOrientation(const QList<ImageInfo> &infos, int orientation)
{
    d->setDBAction(i18n("Updating orientation in database. Please wait..."));
    //TODO: update db
    d->dbProcessed(infos.count());
    d->schedulingForOrientationWrite(infos.count());
    emit writeOrientationToFiles(infos, orientation);
    d->dbFinished(infos.size());
}

// -------------

void MetadataManagerFileWorker::writeOrientationToFiles(const QList<ImageInfo> &infos, int orientation)
{
    d->setWriterAction(i18n("Revising Exif Orientation tags. Please wait..."));

    QStringList failedItems;

    foreach (const ImageInfo &info, infos)
    {
        //kDebug(50003) << "Setting Exif Orientation tag to " << orientation << endl;

        QString path = info.filePath();
        DMetadata metadata(path);
        DMetadata::ImageOrientation o = (DMetadata::ImageOrientation)orientation;
        metadata.setImageOrientation(o);

        if (!metadata.applyChanges())
        {
            failedItems.append(info.name());
        }
        else
        {
            KUrl url = KUrl::fromPath(path);
            ImageAttributesWatch::instance()->fileMetadataChanged(url);
        }

        d->orientationWrittenToOne();
    }

    if (!failedItems.isEmpty())
        emit orientationChangeFailed(failedItems);

    d->finishedWriting(infos.size());
}

void MetadataManagerFileWorker::writeMetadataToFiles(const QList<ImageInfo> &infos)
{
    d->setWriterAction(i18n("Writing metadata to files. Please wait..."));
    d->startingToWrite(infos);

    MetadataHub hub;

    ScanController::instance()->suspendCollectionScan();
    foreach(const ImageInfo &info, infos)
    {

        hub.load(info);
        QString filePath = info.filePath();
        bool fileChanged = hub.write(filePath, MetadataHub::FullWriteIfChanged);
        if (fileChanged)
            ScanController::instance()->scanFileDirectly(filePath);

        d->writtenToOne();
    }
    ScanController::instance()->resumeCollectionScan();

    d->finishedWriting(infos.size());
}

} // namespace Digikam


