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

#ifndef METADATAMANAGER_P_H
#define METADATAMANAGER_P_H

// Qt includes

#include <QMutex>
#include <QSet>
#include <QTimer>

// Local includes

#include "imageinfo.h"
#include "workerobject.h"

namespace Digikam
{

class MetadataManagerDatabaseWorker;
class MetadataManagerFileWorker;

enum GroupAction
{
    AddToGroup,
    RemoveFromGroup,
    Ungroup
};

class MetadataManager::MetadataManagerPriv : public QObject
{
    Q_OBJECT

public:

    MetadataManagerPriv(MetadataManager* q);
    ~MetadataManagerPriv();

public:

    int                            dbTodo;
    int                            dbDone;
    int                            writerTodo;
    int                            writerDone;
    QSet<qlonglong>                scheduledToWrite;
    QString                        dbMessage;
    QString                        writerMessage;
    QMutex                         mutex;

    MetadataManager*               q;

    MetadataManagerDatabaseWorker* dbWorker;
    MetadataManagerFileWorker*     fileWorker;

    QTimer*                        sleepTimer;

public:

    // Signal-emitter glue code
    void assignTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs)
    {
        emit signalAddTags(infos, tagIDs);
    }
    void removeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs)
    {
        emit signalRemoveTags(infos, tagIDs);
    }
    void assignPickLabel(const QList<ImageInfo>& infos, int pickId)
    {
        emit signalAssignPickLabel(infos, pickId);
    }
    void assignColorLabel(const QList<ImageInfo>& infos, int colorId)
    {
        emit signalAssignColorLabel(infos, colorId);
    }
    void assignRating(const QList<ImageInfo>& infos, int rating)
    {
        emit signalAssignRating(infos, rating);
    }
    void editGroup(int groupAction, const ImageInfo& pick, const QList<ImageInfo>& infos)
    {
        emit signalEditGroup(groupAction, pick, infos);
    }
    void setExifOrientation(const QList<ImageInfo>& infos, int orientation)
    {
        emit signalSetExifOrientation(infos, orientation);
    }
    void applyMetadata(const QList<ImageInfo>& infos, MetadataHub* hub)
    {
        emit signalApplyMetadata(infos, hub);
    }

public:

    // -- Workflow controlling --

    // before sending to db worker
    void schedulingForDB(int numberOfInfos);
    // called by db worker to say what it is doing
    void setDBAction(const QString& action);
    // db worker will send info to file worker if returns true
    bool shallSendForWriting(qlonglong id);
    // db worker progress info
    void dbProcessedOne();
    void dbProcessed(int numberOfInfos);
    void dbFinished(int numberOfInfos);
    // db worker calls this before sending to file worker
    void schedulingForWrite(int numberOfInfos);
    void schedulingForOrientationWrite(int numberOfInfos);
    // called by file worker to say what it is doing
    void setWriterAction(const QString& action);
    // file worker calls this when receiving a task
    void startingToWrite(const QList<ImageInfo>& infos);
    // file worker calls this when finished
    void writtenToOne();
    void orientationWrittenToOne();
    void finishedWriting(int numberOfInfos);

    void updateProgress();
    void updateProgressMessage();

public Q_SLOTS:

    void slotImageDataChanged(const QString& path, bool removeThumbnails, bool notifyCache);
    void slotSleepTimer();

Q_SIGNALS:

    // connected to MetadataManager public signals
    void progressMessageChanged(const QString& descriptionOfAction);
    void progressValueChanged(float percent);
    void progressFinished();

    // inter-thread signals: connected to database worker slots
    void signalAddTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs);
    void signalRemoveTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs);
    void signalAssignPickLabel(const QList<ImageInfo>& infos, int pickId);
    void signalAssignColorLabel(const QList<ImageInfo>& infos, int colorId);
    void signalAssignRating(const QList<ImageInfo>& infos, int rating);
    void signalSetExifOrientation(const QList<ImageInfo>& infos, int orientation);
    void signalApplyMetadata(const QList<ImageInfo>& infos, MetadataHub* hub);
    void signalEditGroup(int groupAction, const ImageInfo& pick, const QList<ImageInfo>& infos);
};

// ---------------------------------------------------------------------------------------------

class MetadataManagerDatabaseWorker : public WorkerObject
{
    Q_OBJECT

public:

    MetadataManagerDatabaseWorker(MetadataManager::MetadataManagerPriv* d)
        : d(d) {}

public Q_SLOTS:

    void assignTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs);
    void removeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs);
    void assignPickLabel(const QList<ImageInfo>& infos, int pickId);
    void assignColorLabel(const QList<ImageInfo>& infos, int colorId);
    void assignRating(const QList<ImageInfo>& infos, int rating);
    void editGroup(int groupAction, const ImageInfo& pick, const QList<ImageInfo>& infos);
    void setExifOrientation(const QList<ImageInfo>& infos, int orientation);
    void applyMetadata(const QList<ImageInfo>& infos, MetadataHub* hub);

Q_SIGNALS:

    void writeMetadataToFiles(const QList<ImageInfo>& infos);
    void writeOrientationToFiles(const QList<ImageInfo>& infos, int orientation);
    void writeMetadata(const QList<ImageInfo>& infos, MetadataHub* hub);

private:

    void changeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs, bool addOrRemove);

    MetadataManager::MetadataManagerPriv* const d;
};

// ---------------------------------------------------------------------------------------------

class MetadataManagerFileWorker : public WorkerObject
{
    Q_OBJECT

public:

    MetadataManagerFileWorker(MetadataManager::MetadataManagerPriv* d)
        : d(d) {}

public Q_SLOTS:

    void writeOrientationToFiles(const QList<ImageInfo>& infos, int orientation);
    void writeMetadataToFiles(const QList<ImageInfo>& infos);
    void writeMetadata(const QList<ImageInfo>& infos, MetadataHub* hub);

Q_SIGNALS:

    void imageDataChanged(const QString& path, bool removeThumbnails, bool notifyCache);
    void orientationChangeFailed(const QStringList& failedFileNames);

private:

    MetadataManager::MetadataManagerPriv* const d;
};

} // namespace Digikam

#endif //METADATAMANAGER_P_H
