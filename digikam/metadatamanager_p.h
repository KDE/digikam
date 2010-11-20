/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-05
 * Description : Metadata operations on images
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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
#include <QThread>

// Local includes

#include "imageinfo.h"

namespace Digikam
{

class MetadataManagerDatabaseWorker;
class MetadataManagerFileWorker;

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

public:

    // Signal-emitter glue code
    void assignTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs)
        { emit signalAddTags(infos, tagIDs); }
    void removeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs)
        { emit signalRemoveTags(infos, tagIDs); }
    void assignRating(const QList<ImageInfo>& infos, int rating)
        { emit signalAssignRating(infos, rating); }
    void setExifOrientation(const QList<ImageInfo>& infos, int orientation)
        { emit signalSetExifOrientation(infos, orientation); }
    void applyMetadata(const QList<ImageInfo>& infos, MetadataHub *hub)
        { emit signalApplyMetadata(infos, hub); }

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

    void slotImageDataChanged(const QString &path, bool removeThumbnails, bool notifyCache);

Q_SIGNALS:

    // connected to MetadataManager public signals
    void progressMessageChanged(const QString& descriptionOfAction);
    void progressValueChanged(float percent);
    void progressFinished();

    // inter-thread signals: connected to database worker slots
    void signalAddTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs);
    void signalRemoveTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs);
    void signalAssignRating(const QList<ImageInfo>& infos, int rating);
    void signalSetExifOrientation(const QList<ImageInfo>& infos, int orientation);
    void signalApplyMetadata(const QList<ImageInfo>& infos, MetadataHub *hub);
};

class MetadataManagerWorker : public QObject
{
    Q_OBJECT

public:

    MetadataManagerWorker(MetadataManager::MetadataManagerPriv* d)
        : d(d) // do not install d as QObject parent, moveToThread wont work then
    {
        thread = new Thread(this);
        moveToThread(thread);
        thread->start();
    }

    void shutDown()
    {
        thread->quit(); 
        thread->wait();
    }

protected:

    class Thread : public QThread
    {
        public:
            Thread(QObject* parent = 0)
                : QThread(parent)
                {
                }

            virtual void run() { exec(); }
    };

    Thread*                               thread;

    MetadataManager::MetadataManagerPriv* d;
};

class MetadataManagerDatabaseWorker : public MetadataManagerWorker
{
    Q_OBJECT

public:

    MetadataManagerDatabaseWorker(MetadataManager::MetadataManagerPriv* d)
        : MetadataManagerWorker(d) {}

public Q_SLOTS:

    void assignTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs);
    void removeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs);
    void assignRating(const QList<ImageInfo>& infos, int rating);
    void setExifOrientation(const QList<ImageInfo>& infos, int orientation);
    void applyMetadata(const QList<ImageInfo>& infos, MetadataHub *hub);

Q_SIGNALS:

    void writeMetadataToFiles(const QList<ImageInfo>& infos);
    void writeOrientationToFiles(const QList<ImageInfo>& infos, int orientation);
    void writeMetadata(const QList<ImageInfo>& infos, MetadataHub *hub);

protected:

    void changeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs, bool addOrRemove);
};

class MetadataManagerFileWorker : public MetadataManagerWorker
{
    Q_OBJECT

public:

    MetadataManagerFileWorker(MetadataManager::MetadataManagerPriv* d)
        : MetadataManagerWorker(d) {}

public Q_SLOTS:

    void writeOrientationToFiles(const QList<ImageInfo>& infos, int orientation);
    void writeMetadataToFiles(const QList<ImageInfo>& infos);
    void writeMetadata(const QList<ImageInfo>& infos, MetadataHub* hub);

Q_SIGNALS:

    void imageDataChanged(const QString& path, bool removeThumbnails, bool notifyCache);
    void orientationChangeFailed(const QStringList& failedFileNames);
};

} // namespace Digikam

#endif //METADATAMANAGER_P_H
