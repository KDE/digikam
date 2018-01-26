/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-09
 * Description : Thread actions manager for maintenance tools.
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2017-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
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

#ifndef MAINTENANCE_THREAD_H
#define MAINTENANCE_THREAD_H

// Local includes

#include "actionthreadbase.h"
#include "metadatasynchronizer.h"
#include "imageinfo.h"
#include "identity.h"

class QImage;

namespace Digikam
{

class ImageQualitySettings;
class MaintenanceData;

class MaintenanceThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit MaintenanceThread(QObject* const parent);
    ~MaintenanceThread();

    void setUseMultiCore(const bool b);

    void syncMetadata(const ImageInfoList& items, MetadataSynchronizer::SyncDirection dir, bool tagsOnly);
    void generateThumbs(const QStringList& paths);
    void generateFingerprints(const QStringList& paths);
    void sortByImageQuality(const QStringList& paths, const ImageQualitySettings& quality);

    void computeDatabaseJunk(bool thumbsDb=false, bool facesDb=false, bool similarityDb=false);
    void cleanCoreDb(const QList<qlonglong>& imageIds);
    void cleanThumbsDb(const QList<int>& thumbnailIds);
    void cleanFacesDb(const QList<Identity>& staleIdentities);
    void cleanSimilarityDb(const QList<qlonglong>& imageIds);
    void shrinkDatabases();

    void cancel();

    QString getThumbFingerprintPath();

Q_SIGNALS:

    /** Emit when the task has started it's work.
     */
    void signalStarted();

    /** Emit when an item have been processed. QImage can be used to pass item thumbnail processed.
     */
    void signalAdvance(const QImage&);

    /** Emit when an itam was processed and on additional information is necessary.
     */
    void signalAdvance();

    /** Emit when a items list have been fully processed.
     */
    void signalCompleted();

    /** Signal to emit to sub-tasks to cancel processing.
     */
    void signalCanceled();

    /** Signal to emit junk data for db cleaner.
     */
    void signalData(const QList<qlonglong>& staleImageIds,
                    const QList<int>& staleThumbIds,
                    const QList<Identity>& staleIdentities,
                    const QList<qlonglong>& staleSimilarityImageIds);

    /**
     * Signal to emit the count of additional items to process.
     */
    void signalAddItemsToProcess(int count);

    /**
     * Signal to emit after processing with info if the processing was done
     * and if yes, without errors.
     */
    void signalFinished(bool done, bool errorFree);

private Q_SLOTS:

    void slotThreadFinished();

private:

    /**
     * This function generates from the given list
     * a list of lists with each embedded list having size
     * of at most chunkSize. If chunkSize is zero, the original
     * list is the only chunk.
     * @param toChunk The list to chunk
     * @param chunkSize The chunk size (0 for take everything)
     */
    /*
    template<typename T>
    QList<QList<T>> chunkList(const QList<T>& toChunk, int chunkSize=0)
    {
        QList<QList<T>> chunks;

        // Chunk size 0 means all
        if (chunkSize == 0)
        {
            chunks << toChunk;
            return chunks;
        }

        // Buffer the input list
        QList<T> toChunkList = toChunk;
        QList<T> currentChunk;
        while (!toChunkList.isEmpty())
        {
            // Set the current chunk to the first n elements
            currentChunk = toChunkList.mid(0,chunkSize);
            // Set the buffer list to the rest, i.e.
            // start at position n and take all from this position
            // If n is bigger than the size, an empty list is returned.
            // see qarraydata.cpp in Qt implementation.
            toChunkList  = toChunkList.mid(chunkSize);
            chunks << currentChunk;
        }
        return chunks;
    }

    int getChunkSize(int elementCount);
    */

    MaintenanceData* const data;
};

} // namespace Digikam

#endif // MAINTENANCE_THREAD_H
