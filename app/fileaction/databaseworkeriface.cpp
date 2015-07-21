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

#include "databaseworkeriface.moc"

// KDE includes

#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "collectionscanner.h"
#include "databaseoperationgroup.h"
#include "imageinfotasksplitter.h"
#include "fileactionmngr_p.h"
#include "scancontroller.h"

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
    MetadataHub      hub;
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
                hub.setTag(*tagIt, addOrRemove);
            }

            hub.write(info, MetadataHub::PartialWrite);

            if (hub.willWriteMetadata(MetadataHub::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
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

void FileActionMngrDatabaseWorker::assignPickLabel(FileActionImageInfoList infos, int pickId)
{
    MetadataHub      hub;
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
            hub.write(info, MetadataHub::PartialWrite);

            if (hub.willWriteMetadata(MetadataHub::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
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
    MetadataHub      hub;
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
            hub.write(info, MetadataHub::PartialWrite);

            if (hub.willWriteMetadata(MetadataHub::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
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
    MetadataHub      hub;
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
            hub.write(info, MetadataHub::PartialWrite);

            if (hub.willWriteMetadata(MetadataHub::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
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

void FileActionMngrDatabaseWorker::applyMetadata(FileActionImageInfoList infos, MetadataHub* hub)
{
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

    if (hub->willWriteMetadata(MetadataHub::FullWriteIfChanged))
    {
        // dont filter by shallSendForWriting here; we write from the hub, not from freshly loaded data
        infos.schedulingForWrite(infos.size(), i18n("Writing metadata to files"), d->fileProgressCreator());

        for (ImageInfoTaskSplitter splitter(infos); splitter.hasNext(); )
            emit writeMetadata(splitter.next(), hub->clone());
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
