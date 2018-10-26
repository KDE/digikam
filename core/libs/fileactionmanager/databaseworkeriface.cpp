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
#include "coredboperationgroup.h"
#include "iteminfotasksplitter.h"
#include "fileactionmngr_p.h"
#include "scancontroller.h"
#include "disjointmetadata.h"

namespace Digikam
{

void FileActionMngrDatabaseWorker::assignTags(FileActionItemInfoList infos, const QList<int>& tagIDs)
{
    changeTags(infos, tagIDs, true);
}

void FileActionMngrDatabaseWorker::removeTags(FileActionItemInfoList infos, const QList<int>& tagIDs)
{
    changeTags(infos, tagIDs, false);
}

void FileActionMngrDatabaseWorker::changeTags(FileActionItemInfoList infos,
                                              const QList<int>& tagIDs, bool addOrRemove)
{
    DisjointMetadata hub;
    QList<ItemInfo> forWriting;

    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        foreach(const ItemInfo& info, infos)
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            hub.load(info);

            for (QList<int>::const_iterator tagIt = tagIDs.constBegin(); tagIt != tagIDs.constEnd(); ++tagIt)
            {
                if (addOrRemove)
                {
                    hub.setTag(*tagIt, DisjointMetadata::MetadataAvailable);
                }
                else
                {
                    hub.setTag(*tagIt, DisjointMetadata::MetadataInvalid);
                }
            }

            hub.write(info, DisjointMetadata::PartialWrite);

            if (hub.willWriteMetadata(DisjointMetadata::FullWriteIfChanged) && d->shallSendForWriting(info.id()))
            {
                forWriting << info;
            }

            infos.dbProcessedOne();
            group.allowLift();
        }
    }

    // send for writing file metadata
    if (!forWriting.isEmpty())
    {
        FileActionItemInfoList forWritingTaskList = FileActionItemInfoList::continueTask(forWriting, infos.progress());
        forWritingTaskList.schedulingForWrite(i18n("Writing metadata to files"), d->fileProgressCreator());

        qCDebug(DIGIKAM_GENERAL_LOG) << "Scheduled to write";

        for (ItemInfoTaskSplitter splitter(forWritingTaskList) ; splitter.hasNext() ; )
        {
            emit writeMetadata(FileActionItemInfoList(splitter.next()), MetadataHub::WRITE_TAGS);
        }
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::assignPickLabel(FileActionItemInfoList infos, int pickId)
{
    DisjointMetadata hub;
    QList<ItemInfo> forWriting;

    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        foreach(const ItemInfo& info, infos)
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
    }

    // send for writing file metadata
    if (!forWriting.isEmpty())
    {
        FileActionItemInfoList forWritingTaskList = FileActionItemInfoList::continueTask(forWriting, infos.progress());
        forWritingTaskList.schedulingForWrite(i18n("Writing metadata to files"), d->fileProgressCreator());

        for (ItemInfoTaskSplitter splitter(forWritingTaskList) ; splitter.hasNext() ; )
        {
            emit writeMetadata(FileActionItemInfoList(splitter.next()), MetadataHub::WRITE_PICKLABEL);
        }
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::assignColorLabel(FileActionItemInfoList infos, int colorId)
{
    DisjointMetadata hub;
    QList<ItemInfo> forWriting;

    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        foreach(const ItemInfo& info, infos)
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
    }

    // send for writing file metadata
    if (!forWriting.isEmpty())
    {
        FileActionItemInfoList forWritingTaskList = FileActionItemInfoList::continueTask(forWriting, infos.progress());
        forWritingTaskList.schedulingForWrite(i18n("Writing metadata to files"), d->fileProgressCreator());

        for (ItemInfoTaskSplitter splitter(forWritingTaskList) ; splitter.hasNext() ; )
        {
            emit writeMetadata(FileActionItemInfoList(splitter.next()), MetadataHub::WRITE_COLORLABEL);
        }
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::assignRating(FileActionItemInfoList infos, int rating)
{
    DisjointMetadata hub;
    QList<ItemInfo> forWriting;
    rating = qMin(RatingMax, qMax(RatingMin, rating));

    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        foreach(const ItemInfo& info, infos)
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
    }

    // send for writing file metadata
    if (!forWriting.isEmpty())
    {
        FileActionItemInfoList forWritingTaskList = FileActionItemInfoList::continueTask(forWriting, infos.progress());
        forWritingTaskList.schedulingForWrite(i18n("Writing metadata to files"), d->fileProgressCreator());

        for (ItemInfoTaskSplitter splitter(forWritingTaskList) ; splitter.hasNext() ; )
        {
            emit writeMetadata(FileActionItemInfoList(splitter.next()), MetadataHub::WRITE_RATING);
        }
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::editGroup(int groupAction, const ItemInfo& pick, FileActionItemInfoList infos)
{
    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        foreach(const ItemInfo& constInfo, infos)
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            ItemInfo info(constInfo);

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

void FileActionMngrDatabaseWorker::setExifOrientation(FileActionItemInfoList infos, int orientation)
{
    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        foreach(ItemInfo info, infos)
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

    for (ItemInfoTaskSplitter splitter(infos) ; splitter.hasNext() ; )
    {
        emit writeOrientationToFiles(FileActionItemInfoList(splitter.next()), orientation);
    }

    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::applyMetadata(FileActionItemInfoList infos, DisjointMetadata *hub)
{
    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        foreach(const ItemInfo& info, infos)
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

    if (hub->willWriteMetadata(DisjointMetadata::FullWriteIfChanged), Qt::DirectConnection)
    {
        int flags = hub->changedFlags();
        // don't filter by shallSendForWriting here; we write from the hub, not from freshly loaded data
        infos.schedulingForWrite(infos.size(), i18n("Writing metadata to files"), d->fileProgressCreator());

        for (ItemInfoTaskSplitter splitter(infos) ; splitter.hasNext() ; )
        {
            emit writeMetadata(FileActionItemInfoList(splitter.next()), flags);
        }
    }

    delete hub;
    infos.dbFinished();
}

void FileActionMngrDatabaseWorker::copyAttributes(FileActionItemInfoList infos, const QStringList& derivedPaths)
{
    if (infos.size() == 1)
    {
        foreach(const QString& path, derivedPaths)
        {
            if (state() == WorkerObject::Deactivating)
            {
                break;
            }

            ItemInfo dest = ScanController::instance()->scannedInfo(path);
            CollectionScanner::copyFileProperties(infos.first(), dest);
        }

        infos.dbProcessedOne();
    }

    infos.dbFinished();
}

} // namespace Digikam
