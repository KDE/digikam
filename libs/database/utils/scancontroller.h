/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-28
 * Description : scan pictures interface.
 *
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef SCANCONTROLLER_H
#define SCANCONTROLLER_H

// Qt includes

#include <QThread>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "collectionscannerobserver.h"
#include "imageinfo.h"
#include "loadingcache.h"
#include "coredbchangesets.h"

namespace Digikam
{

class CollectionScanner;
class PAlbum;

class DIGIKAM_EXPORT ScanController : public QThread, public InitializationObserver
{
    Q_OBJECT

public:

    enum Advice
    {
        Success,
        ContinueWithoutDatabase,
        AbortImmediately
    };

public:

    static ScanController* instance();

    /** Wait for the thread to finish. Returns after all tasks are done. */
    void shutDown();


    /**
     * Calls CoreDbAccess::checkReadyForUse(), providing progress
     * feedback if schema updating occurs.
     * Synchronous, returns when ready.
     */
    Advice databaseInitialization();

    /**
     * Carries out a complete collection scan, providing progress feedback.
     * Synchronous, returns when ready.
     * The database will be locked while the scan is running.
     * With the DeferFiles variant, deep files scanning (new files), the part
     * which can take long, will be done during the time after the method returns,
     * shortening the synchronous wait. After completeCollectionScanDeferFiles, you
     * need to call allowToScanDeferredFiles() once to enable scanning the deferred files.
     */
    void completeCollectionScan(bool defer = false);
    void completeCollectionScanDeferFiles();
    void allowToScanDeferredFiles();

    /**
     * Scan Whole collection without to display a progress dialog or to manage splashscreen, as for NewItemsFinder tool.
     */
    void completeCollectionScanInBackground(bool defer);

    /**
     * Carries out a complete collection scan, at the same time updating
     * the unique hash in the database and thumbnail database.
     * Synchronous, returns when ready.
     * The database will be locked while the scan is running.
     */
    void updateUniqueHash();

    /**
     * Schedules a scan of the specified part of the collection.
     * Asynchronous, returns immediately.
     */
    void scheduleCollectionScan(const QString& path);

    /**
     * Schedules a scan of the specified part of the collection.
     * Asynchronous, returns immediately.
     * A small delay may be introduced before the actual scanning starts,
     * so that you can call this often without checking for duplicates.
     * This method must only be used from the main thread.
     */
    void scheduleCollectionScanRelaxed(const QString& path);

    /**
     * If necessary (modified or newly created, scans the file directly
     * Returns the up-to-date ImageInfo.
     */
    ImageInfo scannedInfo(const QString& filePath);

    /**
     * When writing metadata to the file, the file content on disk changes,
     * but the information is taken from the database; therefore,
     * the resulting scanning process can be optimized.
     *
     * Thus, if you write metadata of an ImageInfo from the database to disk,
     * do this in the scope of a FileMetadataWrite object.
     */
    class FileMetadataWrite
    {
    public:

        explicit FileMetadataWrite(const ImageInfo& info);
        ~FileMetadataWrite();

        void changed(bool wasChanged);

    protected:

        ImageInfo m_info;
        bool      m_changed;
    };

    /** If the controller is currently processing a database update
     *  (typically after first run),
     *  cancel this hard and as soon as possible. Any progress may be lost. */
    void abortInitialization();

    /** If the controller is currently doing a complete scan
     *  (typically at startup), stop this operation.
     *  It can be resumed later. */
    void cancelCompleteScan();

    /** Cancels all running or scheduled operations and suspends scanning.
     *  This method returns when all scanning has stopped.
     *  This includes a call to suspendCollectionScan().
     *  Restart with resumeCollectionScan. */
    void cancelAllAndSuspendCollectionScan();

    /** Temporarily suspend collection scanning.
     *  All scheduled scanning tasks are queued
     *  and will be done when resumeCollectionScan()
     *  has been called.
     *  Calling these methods is recursive, you must resume
     *  as often as you called suspend.
     */
    void suspendCollectionScan();
    void resumeCollectionScan();

    /** Hint at the imminent copy, move or rename of an album, so that the
     *  collection scanner is informed about this.
     *  If the album is renamed, give the new name in newAlbumName.
     *  DstAlbum is the new parent album /
     *  dstPath is the new parent directory of the album, so
     *  do not include the album name to dstPath.
     */
    void hintAtMoveOrCopyOfAlbum(const PAlbum* const album, const PAlbum* const dstAlbum, const QString& newAlbumName = QString());
    void hintAtMoveOrCopyOfAlbum(const PAlbum* const album, const QString& dstPath, const QString& newAlbumName = QString());

    /** Hint at the imminent copy, move or rename of items, so that the
     *  collection scanner is informed about this.
     *  Give the list of existing items, specify the destination with dstAlbum,
     *  and give the names at destination in itemNames (Unless for rename, names wont usually change.
     *  Give them nevertheless.)
     */
    void hintAtMoveOrCopyOfItems(const QList<qlonglong> ids, const PAlbum* const dstAlbum, const QStringList& itemNames);
    void hintAtMoveOrCopyOfItem(qlonglong id, const PAlbum* const dstAlbum, const QString& itemName);

    /** Hint at the fact that an item may have changed, although its modification date may not have changed.
     *  Note that a scan of the containing directory will need to be triggered nonetheless for the hints to take effect. */
    void hintAtModificationOfItems(const QList<qlonglong> ids);
    void hintAtModificationOfItem(qlonglong id);

    /**
     * Implementation of FileMetadataWrite, see there. Calling these methods is equivalent.
     */
    void beginFileMetadataWrite(const ImageInfo& info);
    void finishFileMetadataWrite(const ImageInfo& info, bool changed);

Q_SIGNALS:

    void databaseInitialized(bool success);
    void completeScanDone();
    void completeScanCanceled();
    void triggerShowProgressDialog();
    void incrementProgressDialog(int);
    void errorFromInitialization(const QString&);
    void progressFromInitialization(const QString&, int);

    void totalFilesToScan(int);
    void filesScanned(int);

    void collectionScanStarted(const QString& message);
    void scanningProgress(float progress);
    void collectionScanFinished();
    void partialScanDone(const QString& path);

private Q_SLOTS:

    void slotStartCompleteScan();
    void slotTotalFilesToScan(int count);
    void slotStartScanningAlbum(const QString& albumRoot, const QString& album);
    void slotScannedFiles(int scanned);
    void slotStartScanningAlbumRoot(const QString& albumRoot);
    void slotStartScanningForStaleAlbums();
    void slotStartScanningAlbumRoots();

    void slotShowProgressDialog();
    void slotTriggerShowProgressDialog();
    void slotCancelPressed();

    void slotProgressFromInitialization(const QString& message, int numberOfSteps);
    void slotErrorFromInitialization(const QString& errorMessage);

    void slotRelaxedScanning();

protected:

    virtual void run();

private:

    /**
     * The file pointed to by file path will be scanned.
     * The scan is finished when returning from the method.
     */
    void scanFileDirectly(const QString& filePath);
    void scanFileDirectlyNormal(const ImageInfo& info);

    void createProgressDialog();
    void setInitializationMessage();

    void completeCollectionScanCore(bool needTotalFiles, bool defer);

    virtual void moreSchemaUpdateSteps(int numberOfSteps);
    virtual void schemaUpdateProgress(const QString& message, int numberOfSteps);
    virtual void finishedSchemaUpdate(UpdateResult result);
    virtual void connectCollectionScanner(CollectionScanner* const scanner);
    virtual void error(const QString& errorMessage);
    virtual bool continueQuery();

private:

    ScanController();
    ~ScanController();

    friend class ScanControllerCreator;

private:

    class Private;
    Private* const d;
};

// ------------------------------------------------------------------------------

class ScanControllerLoadingCacheFileWatch : public ClassicLoadingCacheFileWatch
{
    Q_OBJECT

    /* This class is derived from the ClassicLoadingCacheFileWatch,
       which means it has the full functionality of the class
       and only extends it by listening to CollectionScanner information
    */

public:

    ScanControllerLoadingCacheFileWatch();

private Q_SLOTS:

    void slotImageChanged(const ImageChangeset& changeset);
};

} // namespace Digikam

#endif // SCANCONTROLLER_H
