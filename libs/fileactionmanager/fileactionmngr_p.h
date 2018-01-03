/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-05
 * Description : file action manager
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FILEACTIONMNGR_P_H
#define FILEACTIONMNGR_P_H

// Qt includes

#include <QMutex>
#include <QSet>
#include <QTimer>

// Local includes

#include "databaseworkeriface.h"
#include "fileactionmngr.h"
#include "fileworkeriface.h"
#include "fileactionimageinfolist.h"
#include "metadatahub.h"
#include "parallelworkers.h"

namespace Digikam
{

enum GroupAction
{
    AddToGroup,
    RemoveFromGroup,
    Ungroup
};

class PrivateProgressItemCreator : public QObject, public FileActionProgressItemCreator
{
    Q_OBJECT

public:

    ProgressItem* createProgressItem(const QString& action) const;
    void addProgressItem(ProgressItem* const item);

    QAtomicInt activeProgressItems;

Q_SIGNALS:

    void lastItemCompleted();

public Q_SLOTS:

    void slotProgressItemCompleted();
    void slotProgressItemCanceled(ProgressItem*);
};

// -----------------------------------------------------------------------------------------------------------

class FileActionMngr::Private : public QObject
{
    Q_OBJECT

public:

    explicit Private(FileActionMngr* const qq);
    ~Private();

Q_SIGNALS:

    void signalTasksFinished();

    // Inter-thread signals: connected to database worker slots
    void signalAddTags(const FileActionImageInfoList& infos, const QList<int>& tagIDs);
    void signalRemoveTags(const FileActionImageInfoList& infos, const QList<int>& tagIDs);
    void signalAssignPickLabel(const FileActionImageInfoList& infos, int pickId);
    void signalAssignColorLabel(const FileActionImageInfoList& infos, int colorId);
    void signalAssignRating(const FileActionImageInfoList& infos, int rating);
    void signalSetExifOrientation(const FileActionImageInfoList& infos, int orientation);
    void signalApplyMetadata(const FileActionImageInfoList& infos, DisjointMetadata* hub);
    void signalEditGroup(int groupAction, const ImageInfo& pick, const FileActionImageInfoList& infos);
    void signalTransform(const FileActionImageInfoList& infos, int orientation);
    void signalCopyAttributes(const FileActionImageInfoList& infos, const QStringList& derivedPaths);

    void signalTransformFinished();

public:

    // -- Signal-emitter glue code --

    void assignTags(const FileActionImageInfoList& infos, const QList<int>& tagIDs)
    {
        emit signalAddTags(infos, tagIDs);
    }

    void removeTags(const FileActionImageInfoList& infos, const QList<int>& tagIDs)
    {
        emit signalRemoveTags(infos, tagIDs);
    }

    void assignPickLabel(const FileActionImageInfoList& infos, int pickId)
    {
        emit signalAssignPickLabel(infos, pickId);
    }

    void assignColorLabel(const FileActionImageInfoList& infos, int colorId)
    {
        emit signalAssignColorLabel(infos, colorId);
    }

    void assignRating(const FileActionImageInfoList& infos, int rating)
    {
        emit signalAssignRating(infos, rating);
    }

    void editGroup(int groupAction, const ImageInfo& pick, const FileActionImageInfoList& infos)
    {
        emit signalEditGroup(groupAction, pick, infos);
    }

    void setExifOrientation(const FileActionImageInfoList& infos, int orientation)
    {
        emit signalSetExifOrientation(infos, orientation);
    }

    void applyMetadata(const FileActionImageInfoList& infos, DisjointMetadata* hub)
    {
        emit signalApplyMetadata(infos, hub);
    }

    void transform(const FileActionImageInfoList& infos, int orientation)
    {
        emit signalTransform(infos, orientation);
    }

    void copyAttributes(const FileActionImageInfoList& infos, const QStringList& derivedPaths)
    {
        emit signalCopyAttributes(infos, derivedPaths);
    }

public:

    // -- Workflow controlling --

    bool isActive() const;

    /// db worker will send info to file worker if returns true
    bool shallSendForWriting(qlonglong id);

    /// file worker calls this when receiving a task
    void startingToWrite(const QList<ImageInfo>& infos);

    void connectToDatabaseWorker();
    void connectDatabaseToFileWorker();

    PrivateProgressItemCreator* dbProgressCreator();
    PrivateProgressItemCreator* fileProgressCreator();

public Q_SLOTS:

    void slotImageDataChanged(const QString& path, bool removeThumbnails, bool notifyCache);
    void slotSleepTimer();
    void slotLastProgressItemCompleted();

public:

    QSet<qlonglong>                       scheduledToWrite;
    QString                               dbMessage;
    QString                               writerMessage;
    QMutex                                mutex;

    FileActionMngr*                       q;

    DatabaseWorkerInterface*              dbWorker;
    ParallelAdapter<FileWorkerInterface>* fileWorker;

    QTimer*                               sleepTimer;

    PrivateProgressItemCreator            dbProgress;
    PrivateProgressItemCreator            fileProgress;
};

} // namespace Digikam

#endif //FILEACTIONMNGR_P_H
