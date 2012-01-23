/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : database worker interface
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

// Local includes

#include "databaseoperationgroup.h"
#include "imageinfotasksplitter.h"
#include "fileactionmngr_p.h"

namespace Digikam
{

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
        for (ImageInfoTaskSplitter splitter(forWriting); splitter.hasNext(); )
            emit writeMetadataToFiles(splitter.next());
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
        for (ImageInfoTaskSplitter splitter(forWriting); splitter.hasNext(); )
            emit writeMetadataToFiles(splitter.next());
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
        for (ImageInfoTaskSplitter splitter(forWriting); splitter.hasNext(); )
            emit writeMetadataToFiles(splitter.next());
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
        for (ImageInfoTaskSplitter splitter(forWriting); splitter.hasNext(); )
            emit writeMetadataToFiles(splitter.next());
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
    {
        DatabaseOperationGroup group;
        group.setMaximumTime(200);
        foreach (ImageInfo info, infos)
        {
            info.setOrientation(orientation);
        }
    }
    d->dbProcessed(infos.count());
    d->schedulingForOrientationWrite(infos.count());
    for (ImageInfoTaskSplitter splitter(infos); splitter.hasNext(); )
        emit writeOrientationToFiles(splitter.next(), orientation);
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
    for (ImageInfoTaskSplitter splitter(infos); splitter.hasNext(); )
        emit writeMetadata(splitter.next(), hub);
    d->dbFinished(infos.size());
}

} // namespace Digikam
