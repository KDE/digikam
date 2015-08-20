/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : database worker interface
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "databaseworkeriface.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "collectionscanner.h"
#include "databaseoperationgroup.h"
#include "imageinfotasksplitter.h"
#include "fileactionmngr_p.h"
#include "scancontroller.h"
#include "disjointmetadata.h"

namespace Digikam
{

void FileActionMngrDatabaseWorker::assignTags(FileActionImageInfoList infos, const QList<int>& tagIDs)
{
    changeTags(infos, tagIDs, true);
}

void FileActionMngrDatabaseWorker::removeTags(FileActionImageInfoList infos, const QList<int>& tagIDs)
{
    changeTags(infos, tagIDs, false);
}

void FileActionMngrDatabaseWorker::changeTags(FileActionImageInfoList infos,
                                              const QList<int>& tagIDs, bool addOrRemove)
{
    DisjointMetadata    hub;
    QList<ImageInfo> forWriting;

    {
        //ScanController::instance()->suspendCollectionScan();
        DatabaseOperationGroup group;
        group.setMaximumTime(200);

        foreach(const ImageInfo& info, infos)
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            hub.load(info);

            for (QList<int>::const_iterator tagIt = tagIDs.constBegin(); tagIt != tagIDs.constEnd(); ++tagIt)
            {
                if(addOrRemove)
                    hub.setTag(*tagIt, DisjointMetadata::MetadataAvailable);
                else
                    hub.setTag(*tagIt, DisjointMetadata::MetadataInvalid);
            }

            hub.write(info, DisjointMetadata::PartialWrite);

            if (hub.willWriteMetadata(DisjointMetadata::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
            {
                forWriting << info;
            }

            infos.dbProcessedOne();
            group.allowLift();
        }
        //ScanController::instance()->resumeCollectionScan();
    }

    // send for writing file metadata
    if (!forWriting.isEmpty())
    {
        FileActionImageInfoList forWritingTaskList = FileActionImageInfoList::continueTask(forWriting, infos.progress());
        forWritingTaskList.schedulingForWrite(i18n("Writing metadata to files"), d->fileProgressCreator());

        qDebug() << "Scheduled to write";
        for (ImageInfoTaskSplitter splitter(forWritingTaskList); splitter.hasNext(); )
            emit writeMetadataToFiles(splitter.next());
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::assignPickLabel(FileActionImageInfoList infos, int pickId)
{
    DisjointMetadata      hub;
    QList<ImageInfo> forWriting;

    {
        //ScanController::instance()->suspendCollectionScan();
        DatabaseOperationGroup group;
        group.setMaximumTime(200);

        foreach(const ImageInfo& info, infos)
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            hub.load(info);
            hub.setPickLabel(pickId);
            hub.write(info, DisjointMetadata::PartialWrite);

            if (hub.willWriteMetadata(DisjointMetadata::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
            {
                forWriting << info;
            }

            infos.dbProcessedOne();
            group.allowLift();
        }
        //ScanController::instance()->resumeCollectionScan();
    }

    // send for writing file metadata
    if (!forWriting.isEmpty())
    {
        FileActionImageInfoList forWritingTaskList = FileActionImageInfoList::continueTask(forWriting, infos.progress());
        forWritingTaskList.schedulingForWrite(i18n("Writing metadata to files"), d->fileProgressCreator());

        for (ImageInfoTaskSplitter splitter(forWritingTaskList); splitter.hasNext(); )
            emit writeMetadataToFiles(splitter.next());
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::assignColorLabel(FileActionImageInfoList infos, int colorId)
{
    DisjointMetadata      hub;
    QList<ImageInfo> forWriting;

    {
        //ScanController::instance()->suspendCollectionScan();
        DatabaseOperationGroup group;
        group.setMaximumTime(200);

        foreach(const ImageInfo& info, infos)
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            hub.load(info);
            hub.setColorLabel(colorId);
            hub.write(info, DisjointMetadata::PartialWrite);

            if (hub.willWriteMetadata(DisjointMetadata::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
            {
                forWriting << info;
            }

            infos.dbProcessedOne();
            group.allowLift();
        }
        //ScanController::instance()->resumeCollectionScan();
    }

    // send for writing file metadata
    if (!forWriting.isEmpty())
    {
        FileActionImageInfoList forWritingTaskList = FileActionImageInfoList::continueTask(forWriting, infos.progress());
        forWritingTaskList.schedulingForWrite(i18n("Writing metadata to files"), d->fileProgressCreator());

        for (ImageInfoTaskSplitter splitter(forWritingTaskList); splitter.hasNext(); )
            emit writeMetadataToFiles(splitter.next());
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::assignRating(FileActionImageInfoList infos, int rating)
{
    rating = qMin(RatingMax, qMax(RatingMin, rating));
    DisjointMetadata      hub;
    QList<ImageInfo> forWriting;

    {
        //ScanController::instance()->suspendCollectionScan();
        DatabaseOperationGroup group;
        group.setMaximumTime(200);

        foreach(const ImageInfo& info, infos)
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            hub.load(info);
            hub.setRating(rating);
            hub.write(info, DisjointMetadata::PartialWrite);

            if (hub.willWriteMetadata(DisjointMetadata::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
            {
                forWriting << info;
            }

            infos.dbProcessedOne();
            group.allowLift();
        }
        //ScanController::instance()->resumeCollectionScan();
    }

    // send for writing file metadata
    if (!forWriting.isEmpty())
    {
        FileActionImageInfoList forWritingTaskList = FileActionImageInfoList::continueTask(forWriting, infos.progress());
        forWritingTaskList.schedulingForWrite(i18n("Writing metadata to files"), d->fileProgressCreator());

        for (ImageInfoTaskSplitter splitter(forWritingTaskList); splitter.hasNext(); )
            emit writeMetadataToFiles(splitter.next());
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::editGroup(int groupAction, const ImageInfo& pick, FileActionImageInfoList infos)
{
    {
        DatabaseOperationGroup group;
        group.setMaximumTime(200);

        foreach(const ImageInfo& constInfo, infos)
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

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

            infos.dbProcessedOne();
            group.allowLift();
        }
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::setExifOrientation(FileActionImageInfoList infos, int orientation)
{
    {
        DatabaseOperationGroup group;
        group.setMaximumTime(200);

        foreach (ImageInfo info, infos)
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            info.setOrientation(orientation);
        }
    }

    infos.dbProcessed(infos.count());
    infos.schedulingForWrite(infos.count(), i18n("Revising Exif Orientation tags"), d->fileProgressCreator());

    for (ImageInfoTaskSplitter splitter(infos); splitter.hasNext(); )
        emit writeOrientationToFiles(splitter.next(), orientation);

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::applyMetadata(FileActionImageInfoList infos, DisjointMetadata *hub)
{
    qDebug() << "Infos size" << infos.size();
    //ScanController::instance()->suspendCollectionScan();
    {
        DatabaseOperationGroup group;
        group.setMaximumTime(200);

        foreach(const ImageInfo& info, infos)
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            // apply to database
            hub->write(info);
            infos.dbProcessedOne();
            group.allowLift();
        }
    }
    //ScanController::instance()->resumeCollectionScan();

    if (hub->willWriteMetadata(DisjointMetadata::FullWriteIfChanged), Qt::DirectConnection)
    {
        // dont filter by shallSendForWriting here; we write from the hub, not from freshly loaded data
        infos.schedulingForWrite(infos.size(), i18n("Writing metadata to files"), d->fileProgressCreator());

        for (ImageInfoTaskSplitter splitter(infos); splitter.hasNext(); )
        {
            FileActionImageInfoList rez = splitter.next();
            emit writeMetadataToFiles(rez);
        }
    }

    delete hub;
    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::copyAttributes(FileActionImageInfoList infos, const QStringList& derivedPaths)
{
    if (infos.size() == 1)
    {
        foreach (const QString& path, derivedPaths)
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            ImageInfo dest = ScanController::instance()->scannedInfo(path);
            CollectionScanner::copyFileProperties(infos.first(), dest);
        }

        infos.dbProcessedOne();
    }

    infos.dbFinished();
}


} // namespace Digikam
