/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-01
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

#include "scancontroller.h"

// Qt includes

#include <QStringList>
#include <QFileInfo>
#include <QPixmap>
#include <QIcon>
#include <QTime>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QTimer>
#include <QEventLoop>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "collectionscanner.h"
#include "collectionscannerhints.h"
#include "coredbaccess.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "filereadwritelock.h"
#include "coredbwatch.h"
#include "dprogressdlg.h"
#include "dmetadata.h"
#include "coredb.h"
#include "albummanager.h"
#include "album.h"
#include "coredbschemaupdater.h"

namespace Digikam
{

class SimpleCollectionScannerObserver : public CollectionScannerObserver
{
public:

    explicit SimpleCollectionScannerObserver(bool* const var)
        : m_continue(var)
    {
        *m_continue = true;
    }

    bool continueQuery()
    {
        return *m_continue;
    }

public:

    bool* m_continue;
};

// ------------------------------------------------------------------------------

class ScanController::Private
{
public:

    Private() :
        running(false),
        needsInitialization(false),
        needsCompleteScan(false),
        needsUpdateUniqueHash(false),
        idle(false),
        scanSuspended(0),
        deferFileScanning(false),
        finishScanAllowed(true),
        continueInitialization(false),
        continueScan(false),
        continuePartialScan(false),
        fileWatchInstalled(false),
        eventLoop(0),
        showTimer(0),
        relaxedTimer(0),
        hints(CollectionScanner::createHintContainer()),
        progressDialog(0),
        advice(ScanController::Success),
        needTotalFiles(false),
        totalFilesToScan(0)
    {
    }

    bool                            running;
    bool                            needsInitialization;
    bool                            needsCompleteScan;
    bool                            needsUpdateUniqueHash;
    bool                            idle;

    int                             scanSuspended;

    QStringList                     scanTasks;

    QStringList                     completeScanDeferredAlbums;
    bool                            deferFileScanning;
    bool                            finishScanAllowed;

    QMutex                          mutex;
    QWaitCondition                  condVar;

    bool                            continueInitialization;
    bool                            continueScan;
    bool                            continuePartialScan;

    bool                            fileWatchInstalled;

    QEventLoop*                     eventLoop;

    QTimer*                         showTimer;
    QTimer*                         relaxedTimer;

    QPixmap                         albumPix;
    QPixmap                         rootPix;
    QPixmap                         actionPix;
    QPixmap                         errorPix;

    CollectionScannerHintContainer* hints;

    QDateTime                       lastHintAdded;

    DProgressDlg*                   progressDialog;

    ScanController::Advice          advice;

    bool                            needTotalFiles;
    int                             totalFilesToScan;

public:

    QPixmap albumPixmap()
    {
        if (albumPix.isNull())
        {
            albumPix = QIcon::fromTheme(QLatin1String("folder-pictures")).pixmap(32);
        }

        return albumPix;
    }

    QPixmap rootPixmap()
    {
        if (rootPix.isNull())
        {
            rootPix = QIcon::fromTheme(QLatin1String("folder-open")).pixmap(32);
        }

        return rootPix;
    }

    QPixmap actionPixmap()
    {
        if (actionPix.isNull())
        {
            actionPix = QIcon::fromTheme(QLatin1String("system-run")).pixmap(32);
        }

        return actionPix;
    }

    QPixmap errorPixmap()
    {
        if (errorPix.isNull())
        {
            errorPix = QIcon::fromTheme(QLatin1String("dialog-error")).pixmap(32);
        }

        return errorPix;
    }

    QPixmap restartPixmap()
    {
        if (errorPix.isNull())
        {
            errorPix = QIcon::fromTheme(QLatin1String("view-refresh")).pixmap(32);
        }

        return errorPix;
    }

    void garbageCollectHints(bool setAccessTime)
    {
        QDateTime current = QDateTime::currentDateTime();

        if (idle                    &&
            lastHintAdded.isValid() &&
            lastHintAdded.secsTo(current) > (5*60))
        {
            hints->clear();
        }

        if (setAccessTime)
        {
            lastHintAdded = current;
        }
    }
};

// ------------------------------------------------------------------------------

class ScanControllerCreator
{
public:

    ScanController object;
};

Q_GLOBAL_STATIC(ScanControllerCreator, creator)

// ------------------------------------------------------------------------------

ScanController* ScanController::instance()
{
    return &creator->object;
}

ScanController::ScanController()
    : d(new Private)
{
    // create event loop
    d->eventLoop = new QEventLoop(this);

    connect(this, SIGNAL(databaseInitialized(bool)),
            d->eventLoop, SLOT(quit()));

    connect(this, SIGNAL(completeScanDone()),
            d->eventLoop, SLOT(quit()));

    connect(this, SIGNAL(completeScanCanceled()),
            d->eventLoop, SLOT(quit()));

    // create timer to show progress dialog
    d->showTimer = new QTimer(this);
    d->showTimer->setSingleShot(true);

    connect(d->showTimer, &QTimer::timeout,
            this, &ScanController::slotShowProgressDialog);

    connect(this, &ScanController::triggerShowProgressDialog,
            this, &ScanController::slotTriggerShowProgressDialog);

    // create timer for relaxed scheduling
    d->relaxedTimer = new QTimer(this);
    d->relaxedTimer->setSingleShot(true);
    d->relaxedTimer->setInterval(500);

    connect(d->relaxedTimer, &QTimer::timeout,
            this, &ScanController::slotRelaxedScanning);

    // interthread connections
    connect(this, &ScanController::errorFromInitialization,
            this, &ScanController::slotErrorFromInitialization);

    connect(this, &ScanController::progressFromInitialization,
            this, &ScanController::slotProgressFromInitialization);

    // start thread
    d->running = true;
    start();
}

ScanController::~ScanController()
{
    shutDown();

    delete d->progressDialog;
    delete d->hints;
    delete d;
}

void ScanController::shutDown()
{
    if (!isRunning())
    {
        return;
    }

    d->running                = false;
    d->continueInitialization = false;
    d->continueScan           = false;
    d->continuePartialScan    = false;

    {
        QMutexLocker lock(&d->mutex);
        d->condVar.wakeAll();
    }

    wait();
}

void ScanController::createProgressDialog()
{
    if (d->progressDialog)
    {
        return;
    }

    d->progressDialog = new DProgressDlg(0);
    d->progressDialog->setLabel(i18n("<b>Scanning collections, please wait...</b>"));
    d->progressDialog->setWhatsThis(i18n("This shows the progress of the scan. "
                                         "During the scan, all files on disk "
                                         "are registered in a database."));

    d->progressDialog->setMaximum(1);
    d->progressDialog->setValue(0);

    connect(this, SIGNAL(incrementProgressDialog(int)),
            d->progressDialog, SLOT(incrementMaximum(int)));

    connect(d->progressDialog, SIGNAL(signalCancelPressed()),
            this, SLOT(slotCancelPressed()));
}

void ScanController::slotCancelPressed()
{
    abortInitialization();
    cancelCompleteScan();
}

void ScanController::slotTriggerShowProgressDialog()
{
    if (d->progressDialog && !d->showTimer->isActive() && !d->progressDialog->isVisible())
    {
        d->showTimer->start(300);
    }
}

void ScanController::slotShowProgressDialog()
{
    if (d->progressDialog)
    {
        //if (!CollectionScanner::databaseInitialScanDone())
        {
            d->progressDialog->show();
        }
    }
}

ScanController::Advice ScanController::databaseInitialization()
{
    d->advice = Success;
    createProgressDialog();
    setInitializationMessage();

    {
        QMutexLocker lock(&d->mutex);
        d->needsInitialization = true;
        d->condVar.wakeAll();
    }

    // loop is quit by signal
    d->eventLoop->exec();

    // setup file watch service for LoadingCache - now that we are sure we have a CoreDbWatch
    if (!d->fileWatchInstalled)
    {
        d->fileWatchInstalled     = true; // once per application lifetime only
        LoadingCache* const cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        cache->setFileWatch(new ScanControllerLoadingCacheFileWatch);
    }

    delete d->progressDialog;
    d->progressDialog = 0;

    return d->advice;
}

void ScanController::completeCollectionScanDeferFiles()
{
    completeCollectionScan(true);
}

void ScanController::allowToScanDeferredFiles()
{
    QMutexLocker lock(&d->mutex);
    d->finishScanAllowed = true;
    d->condVar.wakeAll();
}

void ScanController::completeCollectionScan(bool defer)
{
    createProgressDialog();

    // we only need to count the files in advance
    // if we show a progress percentage in progress dialog
    completeCollectionScanCore(!CollectionScanner::databaseInitialScanDone(), defer);

    delete d->progressDialog;
    d->progressDialog = 0;
}

void ScanController::completeCollectionScanInBackground(bool defer)
{
    completeCollectionScanCore(true, defer);
}

void ScanController::completeCollectionScanCore(bool needTotalFiles, bool defer)
{
    d->needTotalFiles = needTotalFiles;

    {
        QMutexLocker lock(&d->mutex);
        d->needsCompleteScan = true;
        d->deferFileScanning = defer;
        d->condVar.wakeAll();
    }

    // loop is quit by signal
    d->eventLoop->exec();

    d->needTotalFiles = false;
}

void ScanController::updateUniqueHash()
{
    createProgressDialog();
    // we only need to count the files in advance
    //if we show a progress percentage in progress dialog
    d->needTotalFiles = true;

    {
        QMutexLocker lock(&d->mutex);
        d->needsUpdateUniqueHash = true;
        d->condVar.wakeAll();
    }
    // loop is quit by signal
    d->eventLoop->exec();

    delete d->progressDialog;
    d->progressDialog = 0;
    d->needTotalFiles = false;
}

void ScanController::scheduleCollectionScan(const QString& path)
{
    QMutexLocker lock(&d->mutex);

    if (!d->scanTasks.contains(path))
    {
        d->scanTasks << path;
    }

    d->condVar.wakeAll();
}

void ScanController::scheduleCollectionScanRelaxed(const QString& path)
{
    d->relaxedTimer->start();

    QMutexLocker lock(&d->mutex);

    if (!d->scanTasks.contains(path))
    {
        d->scanTasks << path;
    }
}

void ScanController::slotRelaxedScanning()
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "Starting scan!";

    QMutexLocker lock(&d->mutex);
    d->condVar.wakeAll();
}

ImageInfo ScanController::scannedInfo(const QString& filePath)
{
    CollectionScanner scanner;
    scanner.setHintContainer(d->hints);

    ImageInfo info = ImageInfo::fromLocalFile(filePath);

    if (info.isNull())
    {
        qlonglong id = scanner.scanFile(filePath, CollectionScanner::NormalScan);
        return ImageInfo(id);
    }
    else
    {
        scanner.scanFile(info, CollectionScanner::NormalScan);
        return info;
    }
}

ScanController::FileMetadataWrite::FileMetadataWrite(const ImageInfo& info)
    : m_info(info),
      m_changed(false)
{
    ScanController::instance()->beginFileMetadataWrite(info);
}

void ScanController::FileMetadataWrite::changed(bool wasChanged)
{
    m_changed = m_changed || wasChanged;
}

ScanController::FileMetadataWrite::~FileMetadataWrite()
{
    ScanController::instance()->finishFileMetadataWrite(m_info, m_changed);
}

void ScanController::scanFileDirectly(const QString& filePath)
{
    suspendCollectionScan();

    CollectionScanner scanner;
    scanner.setHintContainer(d->hints);
    scanner.scanFile(filePath);

    resumeCollectionScan();
}

void ScanController::scanFileDirectlyNormal(const ImageInfo& info)
{
    CollectionScanner scanner;
    scanner.setHintContainer(d->hints);
    scanner.scanFile(info, CollectionScanner::NormalScan);
}

/*
/// This variant shall be used when a new file is created which is a version
/// of another image, and all relevant attributes shall be copied.
void scanFileDirectlyCopyAttributes(const QString& filePath, qlonglong parentVersion);

void ScanController::scanFileDirectlyCopyAttributes(const QString& filePath, qlonglong parentVersion)
{
    suspendCollectionScan();

    CollectionScanner scanner;
    scanner.recordHints(d->itemHints);
    scanner.recordHints(d->itemChangeHints);
    qlonglong id = scanner.scanFile(filePath);
    ImageInfo dest(id), source(parentVersion);
    scanner.copyFileProperties(source, dest);

    resumeCollectionScan();
}
*/

void ScanController::abortInitialization()
{
    QMutexLocker lock(&d->mutex);
    d->needsInitialization    = false;
    d->continueInitialization = false;
}

void ScanController::cancelCompleteScan()
{
    QMutexLocker lock(&d->mutex);
    d->needsCompleteScan = false;
    d->continueScan      = false;
    emit completeScanCanceled();
}

void ScanController::cancelAllAndSuspendCollectionScan()
{
    QMutexLocker lock(&d->mutex);

    d->needsInitialization    = false;
    d->continueInitialization = false;

    d->needsCompleteScan      = false;
    d->continueScan           = false;

    d->scanTasks.clear();
    d->continuePartialScan    = false;

    d->relaxedTimer->stop();

    // like suspendCollectionScan
    d->scanSuspended++;

    while (!d->idle)
    {
        d->condVar.wait(&d->mutex, 20);
    }
}

void ScanController::suspendCollectionScan()
{
    QMutexLocker lock(&d->mutex);
    d->scanSuspended++;
}

void ScanController::resumeCollectionScan()
{
    QMutexLocker lock(&d->mutex);

    if (d->scanSuspended)
    {
        d->scanSuspended--;
    }

    if (!d->scanSuspended)
    {
        d->condVar.wakeAll();
    }
}

void ScanController::run()
{
    while (d->running)
    {
        bool doInit             = false;
        bool doScan             = false;
        bool doScanDeferred     = false;
        bool doFinishScan       = false;
        bool doPartialScan      = false;
        bool doUpdateUniqueHash = false;

        QString task;
        {
            QMutexLocker lock(&d->mutex);

            if (d->needsInitialization)
            {
                d->needsInitialization = false;
                doInit                 = true;
            }
            else if (d->needsCompleteScan)
            {
                d->needsCompleteScan = false;
                doScan               = true;
                doScanDeferred       = d->deferFileScanning;
            }
            else if (d->needsUpdateUniqueHash)
            {
                d->needsUpdateUniqueHash = false;
                doUpdateUniqueHash       = true;
            }
            else if (!d->completeScanDeferredAlbums.isEmpty() && d->finishScanAllowed && !d->scanSuspended)
            {
                // d->completeScanDeferredAlbums is only accessed from the thread, no need to copy
                doFinishScan             = true;
            }
            else if (!d->scanTasks.isEmpty() && !d->scanSuspended)
            {
                doPartialScan = true;
                task          = d->scanTasks.takeFirst();
            }
            else
            {
                d->idle = true;
                d->condVar.wait(&d->mutex);
                d->idle = false;
            }
        }

        if (doInit)
        {
            d->continueInitialization = true;
            // pass "this" as InitializationObserver
            bool success = CoreDbAccess::checkReadyForUse(this);

            // If d->advice has not been adjusted to a value indicating failure, do this here
            if (!success && d->advice == Success)
            {
                d->advice = ContinueWithoutDatabase;
            }

            emit databaseInitialized(success);
        }
        else if (doScan)
        {
            CollectionScanner scanner;
            connectCollectionScanner(&scanner);

            scanner.setNeedFileCount(d->needTotalFiles);
            scanner.setDeferredFileScanning(doScanDeferred);
            scanner.setHintContainer(d->hints);

            SimpleCollectionScannerObserver observer(&d->continueScan);
            scanner.setObserver(&observer);

            scanner.completeScan();

            emit completeScanDone();

            if (doScanDeferred)
            {
                d->completeScanDeferredAlbums = scanner.deferredAlbumPaths();
                d->finishScanAllowed = false;
            }
        }
        else if (doFinishScan)
        {
            if (d->completeScanDeferredAlbums.isEmpty())
            {
                continue;
            }

            CollectionScanner scanner;
            connectCollectionScanner(&scanner);

            emit collectionScanStarted(i18nc("@info:status", "Scanning collection"));
            //TODO: reconsider performance
            scanner.setNeedFileCount(true);//d->needTotalFiles);

            scanner.setHintContainer(d->hints);

            SimpleCollectionScannerObserver observer(&d->continueScan);
            scanner.setObserver(&observer);

            scanner.finishCompleteScan(d->completeScanDeferredAlbums);

            d->completeScanDeferredAlbums.clear();
            emit completeScanDone();
            emit collectionScanFinished();
        }
        else if (doPartialScan)
        {
            CollectionScanner scanner;
            scanner.setHintContainer(d->hints);
            //connectCollectionScanner(&scanner);
            SimpleCollectionScannerObserver observer(&d->continuePartialScan);
            scanner.setObserver(&observer);
            scanner.partialScan(task);
            emit partialScanDone(task);
        }
        else if (doUpdateUniqueHash)
        {
            CoreDbAccess access;
            CoreDbSchemaUpdater updater(access.db(), access.backend(), access.parameters());
            updater.setCoreDbAccess(&access);
            updater.setObserver(this);
            updater.updateUniqueHash();
            emit completeScanDone();
        }
    }
}

// (also implementing InitializationObserver)
void ScanController::connectCollectionScanner(CollectionScanner* const scanner)
{
    scanner->setSignalsEnabled(true);

    connect(scanner, SIGNAL(startCompleteScan()),
            this, SLOT(slotStartCompleteScan()));

    connect(scanner, SIGNAL(totalFilesToScan(int)),
            this, SLOT(slotTotalFilesToScan(int)));

    connect(scanner, SIGNAL(startScanningAlbum(QString,QString)),
            this, SLOT(slotStartScanningAlbum(QString,QString)));

    connect(scanner, SIGNAL(scannedFiles(int)),
            this, SLOT(slotScannedFiles(int)));

    connect(scanner, SIGNAL(startScanningAlbumRoot(QString)),
            this, SLOT(slotStartScanningAlbumRoot(QString)));

    connect(scanner, SIGNAL(startScanningForStaleAlbums()),
            this, SLOT(slotStartScanningForStaleAlbums()));

    connect(scanner, SIGNAL(startScanningAlbumRoots()),
            this, SLOT(slotStartScanningAlbumRoots()));
}

void ScanController::slotTotalFilesToScan(int count)
{
    if (d->progressDialog)
    {
        d->progressDialog->incrementMaximum(count);
    }

    d->totalFilesToScan = count;
    emit totalFilesToScan(d->totalFilesToScan);
}

void ScanController::slotStartCompleteScan()
{
    d->totalFilesToScan = 0;
    slotTriggerShowProgressDialog();

    QString message     = i18n("Preparing collection scan...");

    if (d->progressDialog)
    {
        d->progressDialog->addedAction(d->restartPixmap(), message);
    }
}

void ScanController::slotStartScanningAlbum(const QString& albumRoot, const QString& album)
{
    Q_UNUSED(albumRoot);

    if (d->progressDialog)
    {
        d->progressDialog->addedAction(d->albumPixmap(), QLatin1Char(' ') + album);
    }
}

void ScanController::slotScannedFiles(int scanned)
{
    if (d->progressDialog)
    {
        d->progressDialog->advance(scanned);
    }

    if (d->totalFilesToScan)
    {
        emit filesScanned(scanned);
        emit scanningProgress(double(scanned) / double(d->totalFilesToScan));
    }
}

void ScanController::slotStartScanningAlbumRoot(const QString& albumRoot)
{
    if (d->progressDialog)
    {
        d->progressDialog->addedAction(d->rootPixmap(), albumRoot);
    }
}

void ScanController::slotStartScanningForStaleAlbums()
{
    QString message = i18n("Scanning for removed albums...");

    if (d->progressDialog)
    {
        d->progressDialog->addedAction(d->actionPixmap(), message);
    }
}

void ScanController::slotStartScanningAlbumRoots()
{
    QString message = i18n("Scanning images in individual albums...");

    if (d->progressDialog)
    {
        d->progressDialog->addedAction(d->actionPixmap(), message);
    }
}

// implementing InitializationObserver
void ScanController::moreSchemaUpdateSteps(int numberOfSteps)
{
    // not from main thread
    emit triggerShowProgressDialog();
    emit incrementProgressDialog(numberOfSteps);
}

// implementing InitializationObserver
void ScanController::schemaUpdateProgress(const QString& message, int numberOfSteps)
{
    // not from main thread
    emit progressFromInitialization(message, numberOfSteps);
}

void ScanController::slotProgressFromInitialization(const QString& message, int numberOfSteps)
{
    // main thread

    if (d->progressDialog)
    {
        d->progressDialog->addedAction(d->actionPixmap(), message);
        d->progressDialog->advance(numberOfSteps);
    }
}

// implementing InitializationObserver
void ScanController::finishedSchemaUpdate(UpdateResult result)
{
    // not from main thread
    switch (result)
    {
        case InitializationObserver::UpdateSuccess:
            d->advice = Success;
            break;
        case InitializationObserver::UpdateError:
            d->advice = ContinueWithoutDatabase;
            break;
        case InitializationObserver::UpdateErrorMustAbort:
            d->advice = AbortImmediately;
            break;
    }
}

// implementing InitializationObserver
void ScanController::error(const QString& errorMessage)
{
    // not from main thread
    emit errorFromInitialization(errorMessage);
}

// implementing InitializationObserver
bool ScanController::continueQuery()
{
    // not from main thread
    return d->continueInitialization;
}

void ScanController::slotErrorFromInitialization(const QString& errorMessage)
{
    // main thread
    QString message = i18n("Error");

    if (d->progressDialog)
    {
        d->progressDialog->addedAction(d->errorPixmap(), message);
    }

    QMessageBox::critical(d->progressDialog, qApp->applicationName(), errorMessage);
}

void ScanController::setInitializationMessage()
{
    QString message = i18n("Initializing database...");

    if (d->progressDialog)
    {
        d->progressDialog->addedAction(d->restartPixmap(), message);
    }
}

static AlbumCopyMoveHint hintForAlbum(const PAlbum* const album, int dstAlbumRootId, const QString& relativeDstPath,
                                      const QString& albumName)
{
    QString dstAlbumPath;

    if (relativeDstPath == QLatin1String("/"))
    {
        dstAlbumPath = relativeDstPath + albumName;
    }
    else
    {
        dstAlbumPath = relativeDstPath + QLatin1Char('/') + albumName;
    }

    return AlbumCopyMoveHint(album->albumRootId(), album->id(),
                             dstAlbumRootId, dstAlbumPath);
}

static QList<AlbumCopyMoveHint> hintsForAlbum(const PAlbum* const album, int dstAlbumRootId, QString relativeDstPath,
                                              const QString& albumName)
{
    QList<AlbumCopyMoveHint> newHints;

    newHints << hintForAlbum(album, dstAlbumRootId, relativeDstPath, albumName);
    QString parentAlbumPath = album->albumPath();

    if (parentAlbumPath == QLatin1String("/"))
    {
        parentAlbumPath.clear();    // do not cut away a "/" in mid() below
    }

    for (AlbumIterator it(const_cast<PAlbum*>(album)); *it; ++it)
    {
        PAlbum* const a        = (PAlbum*)*it;
        QString childAlbumPath = a->albumPath();
        newHints << hintForAlbum(a, dstAlbumRootId, relativeDstPath, albumName + childAlbumPath.mid(parentAlbumPath.length()));
    }

    return newHints;
}

void ScanController::hintAtMoveOrCopyOfAlbum(const PAlbum* const album, const QString& dstPath, const QString& newAlbumName)
{
    // get album root and album from dst path
    CollectionLocation location = CollectionManager::instance()->locationForPath(dstPath);

    if (location.isNull())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "hintAtMoveOrCopyOfAlbum: Destination path" << dstPath
                   << "does not point to an available location.";
        return;
    }

    QString relativeDstPath           = CollectionManager::instance()->album(location, dstPath);

    QList<AlbumCopyMoveHint> newHints = hintsForAlbum(album, location.id(), relativeDstPath,
                                                      newAlbumName.isNull() ? album->title() : newAlbumName);

    //QMutexLocker lock(&d->mutex);
    //d->albumHints << newHints;
    d->hints->recordHints(newHints);
}

void ScanController::hintAtMoveOrCopyOfAlbum(const PAlbum* const album, const PAlbum* const dstAlbum, const QString& newAlbumName)
{
    QList<AlbumCopyMoveHint> newHints = hintsForAlbum(album, dstAlbum->albumRootId(), dstAlbum->albumPath(),
                                                      newAlbumName.isNull() ? album->title() : newAlbumName);

    //QMutexLocker lock(&d->mutex);
    //d->albumHints << newHints;
    d->hints->recordHints(newHints);
}

void ScanController::hintAtMoveOrCopyOfItems(const QList<qlonglong> ids, const PAlbum* const dstAlbum,
                                             const QStringList& itemNames)
{
    ItemCopyMoveHint hint(ids, dstAlbum->albumRootId(), dstAlbum->id(), itemNames);

    d->garbageCollectHints(true);
    //d->itemHints << hint;
    d->hints->recordHints(QList<ItemCopyMoveHint>() << hint);
}

void ScanController::hintAtMoveOrCopyOfItem(qlonglong id, const PAlbum* const dstAlbum, const QString& itemName)
{
    ItemCopyMoveHint hint(QList<qlonglong>() << id, dstAlbum->albumRootId(), dstAlbum->id(), QStringList() << itemName);

    d->garbageCollectHints(true);
    //d->itemHints << hint;
    d->hints->recordHints(QList<ItemCopyMoveHint>() << hint);
}

void ScanController::hintAtModificationOfItems(const QList<qlonglong> ids)
{
    ItemChangeHint hint(ids, ItemChangeHint::ItemModified);

    d->garbageCollectHints(true);
    //d->itemHints << hint;
    d->hints->recordHints(QList<ItemChangeHint>() << hint);
}

void ScanController::hintAtModificationOfItem(qlonglong id)
{
    ItemChangeHint hint(QList<qlonglong>() << id, ItemChangeHint::ItemModified);

    d->garbageCollectHints(true);
    //d->itemHints << hint;
    d->hints->recordHints(QList<ItemChangeHint>() << hint);
}

void ScanController::beginFileMetadataWrite(const ImageInfo& info)
{
    {
        // throw in a lock to synchronize with all parallel writing
        FileReadLocker locker(info.filePath());
    }
    QFileInfo fi(info.filePath());
    d->hints->recordHint(ItemMetadataAdjustmentHint(info.id(), ItemMetadataAdjustmentHint::AboutToEditMetadata,
                                                    fi.lastModified(), fi.size()));
}

void ScanController::finishFileMetadataWrite(const ImageInfo& info, bool changed)
{
    QFileInfo fi(info.filePath());
    d->hints->recordHint(ItemMetadataAdjustmentHint(info.id(),
                                                    changed ? ItemMetadataAdjustmentHint::MetadataEditingFinished :
                                                              ItemMetadataAdjustmentHint::MetadataEditingAborted,
                                                    fi.lastModified(), fi.size()));

    scanFileDirectlyNormal(info);
}

// --------------------------------------------------------------------------------------------

ScanControllerLoadingCacheFileWatch::ScanControllerLoadingCacheFileWatch()
{
    CoreDbWatch* const dbwatch = CoreDbAccess::databaseWatch();

    // we opt for a queued connection to make stuff a bit relaxed
    connect(dbwatch, SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotImageChanged(ImageChangeset)),
            Qt::QueuedConnection);
}

void ScanControllerLoadingCacheFileWatch::slotImageChanged(const ImageChangeset& changeset)
{
    foreach(const qlonglong& imageId, changeset.ids())
    {
        DatabaseFields::Set changes = changeset.changes();

        if (changes & DatabaseFields::ModificationDate || changes & DatabaseFields::Orientation)
        {
            ImageInfo info(imageId);
            //qCDebug(DIGIKAM_DATABASE_LOG) << imageId << info.filePath();
            notifyFileChanged(info.filePath());
        }
    }
}

}  // namespace Digikam
