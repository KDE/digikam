/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-01
 * Description : scan pictures interface.
 *
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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
#include "moc_scancontroller.cpp"

// Qt includes

#include <QApplication>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QProgressBar>
#include <QTime>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QTimer>

// KDE includes

#include <kmessagebox.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

// Local includes

#include "collectionscanner.h"
#include "collectionscannerhints.h"
#include "databaseaccess.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "loadingcache.h"
#include "databasewatch.h"
#include "databasechangesets.h"
#include "imageinfo.h"
#include "dprogressdlg.h"
#include "dmetadata.h"
#include "albumsettings.h"
#include "albumdb.h"
#include "albummanager.h"
#include "album.h"
#include "schemaupdater.h"
#include "splashscreen.h"

namespace Digikam
{

class SimpleCollectionScannerObserver : public CollectionScannerObserver
{
public:

    SimpleCollectionScannerObserver(bool* var) : m_continue(var)
    {
        *m_continue = true;
    }

    bool continueQuery()
    {
        return *m_continue;
    }

    bool* m_continue;
};

// ------------------------------------------------------------------------------

class ScanController::ScanControllerPriv
{
public:

    ScanControllerPriv() :
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
        progressDialog(0),
        splash(0),
        advice(ScanController::Success),
        needTotalFiles(false),
        totalFilesToScan(0)
    {
    }

    bool                      running;
    bool                      needsInitialization;
    bool                      needsCompleteScan;
    bool                      needsUpdateUniqueHash;
    bool                      idle;

    int                       scanSuspended;

    QStringList               scanTasks;

    QStringList               completeScanDeferredAlbums;
    bool                      deferFileScanning;
    bool                      finishScanAllowed;

    QMutex                    mutex;
    QWaitCondition            condVar;

    bool                      continueInitialization;
    bool                      continueScan;
    bool                      continuePartialScan;

    bool                      fileWatchInstalled;

    QEventLoop*               eventLoop;

    QTimer*                   showTimer;
    QTimer*                   relaxedTimer;

    QPixmap                   albumPix;
    QPixmap                   rootPix;
    QPixmap                   actionPix;
    QPixmap                   errorPix;

    QList<AlbumCopyMoveHint>  albumHints;
    QList<ItemCopyMoveHint>   itemHints;
    QList<ItemChangeHint>     itemChangeHints;

    QDateTime                 lastHintAdded;

    DProgressDlg*             progressDialog;

    SplashScreen*             splash;

    ScanController::Advice    advice;

    bool                      needTotalFiles;
    int                       totalFilesToScan;

public:

    QPixmap albumPixmap()
    {
        if (albumPix.isNull())
        {
            albumPix = KIconLoader::global()->loadIcon("folder-image", KIconLoader::NoGroup, 32);
        }

        return albumPix;
    }

    QPixmap rootPixmap()
    {
        if (rootPix.isNull())
        {
            rootPix = KIconLoader::global()->loadIcon("folder-open", KIconLoader::NoGroup, 32);
        }

        return rootPix;
    }

    QPixmap actionPixmap()
    {
        if (actionPix.isNull())
        {
            actionPix = KIconLoader::global()->loadIcon("system-run", KIconLoader::NoGroup, 32);
        }

        return actionPix;
    }

    QPixmap errorPixmap()
    {
        if (errorPix.isNull())
        {
            errorPix = KIconLoader::global()->loadIcon("dialog-error", KIconLoader::NoGroup, 32);
        }

        return errorPix;
    }

    QPixmap restartPixmap()
    {
        if (errorPix.isNull())
        {
            errorPix = KIconLoader::global()->loadIcon("view-refresh", KIconLoader::NoGroup, 32);
        }

        return errorPix;
    }

    void garbageCollectHints(bool setAccessTime)
    {
        // called with locked mutex
        QDateTime current = QDateTime::currentDateTime();

        if (idle &&
            lastHintAdded.isValid() &&
            lastHintAdded.secsTo(current) > 5*60)
        {
            itemHints.clear();
            albumHints.clear();
            itemChangeHints.clear();
        }

        if (setAccessTime)
        {
            lastHintAdded = current;
        }
    }
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

// ------------------------------------------------------------------------------

// for ScanControllerLoadingCacheFileWatch
#include "scancontroller.moc"

class ScanControllerCreator
{
public:

    ScanController object;
};
K_GLOBAL_STATIC(ScanControllerCreator, creator)

// ------------------------------------------------------------------------------

ScanController* ScanController::instance()
{
    return &creator->object;
}

ScanController::ScanController()
    : d(new ScanControllerPriv)
{
    // create event loop
    d->eventLoop = new QEventLoop(this);

    connect(this, SIGNAL(databaseInitialized(bool)),
            d->eventLoop, SLOT(quit()));

    connect(this, SIGNAL(completeScanDone()),
            d->eventLoop, SLOT(quit()));

    // create timer to show progress dialog
    d->showTimer = new QTimer(this);
    d->showTimer->setSingleShot(true);

    connect(d->showTimer, SIGNAL(timeout()),
            this, SLOT(slotShowProgressDialog()));

    connect(this, SIGNAL(triggerShowProgressDialog()),
            this, SLOT(slotTriggerShowProgressDialog()));

    // create timer for relaxed scheduling
    d->relaxedTimer = new QTimer(this);
    d->relaxedTimer->setSingleShot(true);
    d->relaxedTimer->setInterval(250);

    connect(d->relaxedTimer, SIGNAL(timeout()),
            this, SLOT(slotRelaxedScanning()));

    // interthread connections
    connect(this, SIGNAL(errorFromInitialization(QString)),
            this, SLOT(slotErrorFromInitialization(QString)));

    connect(this, SIGNAL(progressFromInitialization(QString,int)),
            this, SLOT(slotProgressFromInitialization(QString,int)));

    // start thread
    d->running = true;
    start();
}

ScanController::~ScanController()
{
    shutDown();

    delete d->progressDialog;
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
        if (!d->splash || !CollectionScanner::databaseInitialScanDone())
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

    // setup file watch service for LoadingCache - now that we are sure we have a DatabaseWatch
    if (!d->fileWatchInstalled)
    {
        d->fileWatchInstalled = true; // once per application lifetime only
        LoadingCache* cache   = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        cache->setFileWatch(new ScanControllerLoadingCacheFileWatch);
    }

    delete d->progressDialog;
    d->progressDialog = 0;

    return d->advice;
}

void ScanController::completeCollectionScanDeferFiles(SplashScreen* splash)
{
    completeCollectionScan(splash, true);
}

void ScanController::allowToScanDeferredFiles()
{
    QMutexLocker lock(&d->mutex);
    d->finishScanAllowed = true;
    d->condVar.wakeAll();
}

void ScanController::completeCollectionScan(SplashScreen* splash, bool defer)
{
    d->splash = splash;
    createProgressDialog();
    // we only need to count the files in advance
    //if we show a progress percentage in progress dialog
    d->needTotalFiles = (!d->splash || !CollectionScanner::databaseInitialScanDone());

    {
        QMutexLocker lock(&d->mutex);
        d->needsCompleteScan = true;
        d->deferFileScanning = defer;
        d->condVar.wakeAll();
    }
    // loop is quit by signal
    d->eventLoop->exec();

    delete d->progressDialog;
    d->progressDialog = 0;
    // We do not delete Splashscreen here.
    d->splash         = 0;
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
    if (!d->relaxedTimer->isActive())
    {
        d->relaxedTimer->start();
    }

    QMutexLocker lock(&d->mutex);

    if (!d->scanTasks.contains(path))
    {
        d->scanTasks << path;
    }
}

void ScanController::slotRelaxedScanning()
{
    d->relaxedTimer->stop();

    QMutexLocker lock(&d->mutex);
    d->condVar.wakeAll();
}

void ScanController::scanFileDirectly(const QString& filePath)
{
    suspendCollectionScan();

    CollectionScanner scanner;
    scanner.recordHints(d->itemHints);
    scanner.recordHints(d->itemChangeHints);
    scanner.scanFile(filePath);

    resumeCollectionScan();
}

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
            bool success = DatabaseAccess::checkReadyForUse(this);

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

            scanner.recordHints(d->albumHints);
            scanner.recordHints(d->itemHints);
            scanner.recordHints(d->itemChangeHints);

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

            scanner.recordHints(d->albumHints);
            scanner.recordHints(d->itemHints);
            scanner.recordHints(d->itemChangeHints);

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
            scanner.recordHints(d->albumHints);
            scanner.recordHints(d->itemHints);
            scanner.recordHints(d->itemChangeHints);
            //connectCollectionScanner(&scanner);
            SimpleCollectionScannerObserver observer(&d->continuePartialScan);
            scanner.setObserver(&observer);
            scanner.partialScan(task);
        }
        else if (doUpdateUniqueHash)
        {
            DatabaseAccess access;
            SchemaUpdater updater(access.db(), access.backend(), access.parameters());
            updater.setDatabaseAccess(&access);
            updater.setObserver(this);
            updater.updateUniqueHash();
            emit completeScanDone();
        }
    }
}

// (also implementing InitializationObserver)
void ScanController::connectCollectionScanner(CollectionScanner* scanner)
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

    QString message = i18n("Preparing collection scan...");

    if (d->splash)
    {
        d->splash->message(message);
    }

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
        d->progressDialog->addedAction(d->albumPixmap(), ' ' + album);
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

    if (d->splash)
    {
        d->splash->message(message);
    }
    else if (d->progressDialog)
    {
        d->progressDialog->addedAction(d->actionPixmap(), message);
    }
}

void ScanController::slotStartScanningAlbumRoots()
{
    QString message = i18n("Scanning images in individual albums...");

    if (d->splash)
    {
        d->splash->message(message);
    }
    else if (d->progressDialog)
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

    if (d->splash)
    {
        d->splash->message(message);
    }
    else if (d->progressDialog)
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

    if (d->splash)
    {
        d->splash->message(message);
    }
    else if (d->progressDialog)
    {
        d->progressDialog->addedAction(d->errorPixmap(), message);
    }

    KMessageBox::error(d->progressDialog, errorMessage);
}

void ScanController::setInitializationMessage()
{
    QString message = i18n("Initializing database...");

    if (d->splash)
    {
        d->splash->message(message);
    }

    if (d->progressDialog)
    {
        d->progressDialog->addedAction(d->restartPixmap(), message);
    }
}

static AlbumCopyMoveHint hintForAlbum(const PAlbum* album, int dstAlbumRootId, const QString& relativeDstPath,
                                      const QString& albumName)
{
    QString dstAlbumPath;

    if (relativeDstPath == "/")
    {
        dstAlbumPath = relativeDstPath + albumName;
    }
    else
    {
        dstAlbumPath = relativeDstPath + '/' + albumName;
    }

    return AlbumCopyMoveHint(album->albumRootId(), album->id(),
                             dstAlbumRootId, dstAlbumPath);
}

static QList<AlbumCopyMoveHint> hintsForAlbum(const PAlbum* album, int dstAlbumRootId, QString relativeDstPath,
                                              const QString& albumName)
{
    QList<AlbumCopyMoveHint> newHints;

    newHints << hintForAlbum(album, dstAlbumRootId, relativeDstPath, albumName);
    QString parentAlbumPath = album->albumPath();

    if (parentAlbumPath == "/")
    {
        parentAlbumPath.clear();    // do not cut away a "/" in mid() below
    }

    for (AlbumIterator it(const_cast<PAlbum*>(album)); *it; ++it)
    {
        PAlbum* a = (PAlbum*)*it;
        QString childAlbumPath = a->albumPath();
        newHints << hintForAlbum(a, dstAlbumRootId, relativeDstPath, albumName + childAlbumPath.mid(parentAlbumPath.length()));
    }

    return newHints;
}

void ScanController::hintAtMoveOrCopyOfAlbum(const PAlbum* album, const QString& dstPath, const QString& newAlbumName)
{
    // get album root and album from dst path
    CollectionLocation location = CollectionManager::instance()->locationForPath(dstPath);

    if (location.isNull())
    {
        kWarning() << "hintAtMoveOrCopyOfAlbum: Destination path" << dstPath
                   << "does not point to an available location.";
        return;
    }

    QString relativeDstPath = CollectionManager::instance()->album(location, dstPath);

    QList<AlbumCopyMoveHint> newHints = hintsForAlbum(album, location.id(), relativeDstPath,
                                                      newAlbumName.isNull() ? album->title() : newAlbumName);

    QMutexLocker lock(&d->mutex);
    d->albumHints << newHints;
}

void ScanController::hintAtMoveOrCopyOfAlbum(const PAlbum* album, const PAlbum* dstAlbum, const QString& newAlbumName)
{
    QList<AlbumCopyMoveHint> newHints = hintsForAlbum(album, dstAlbum->albumRootId(), dstAlbum->albumPath(),
                                                      newAlbumName.isNull() ? album->title() : newAlbumName);

    QMutexLocker lock(&d->mutex);
    d->albumHints << newHints;
}

void ScanController::hintAtMoveOrCopyOfItems(const QList<qlonglong> ids, const PAlbum* dstAlbum,
                                             const QStringList& itemNames)
{
    ItemCopyMoveHint hint(ids, dstAlbum->albumRootId(), dstAlbum->id(), itemNames);

    QMutexLocker lock(&d->mutex);
    d->garbageCollectHints(true);
    d->itemHints << hint;
}

void ScanController::hintAtMoveOrCopyOfItem(qlonglong id, const PAlbum* dstAlbum, const QString& itemName)
{
    ItemCopyMoveHint hint(QList<qlonglong>() << id, dstAlbum->albumRootId(), dstAlbum->id(), QStringList() << itemName);

    QMutexLocker lock(&d->mutex);
    d->garbageCollectHints(true);
    d->itemHints << hint;
}

void ScanController::hintAtModificationOfItems(const QList<qlonglong> ids)
{
    ItemChangeHint hint(ids, ItemChangeHint::ItemModified);

    QMutexLocker lock(&d->mutex);
    d->garbageCollectHints(true);
    d->itemChangeHints << hint;
}

void ScanController::hintAtModificationOfItem(qlonglong id)
{
    ItemChangeHint hint(QList<qlonglong>() << id, ItemChangeHint::ItemModified);

    QMutexLocker lock(&d->mutex);
    d->garbageCollectHints(true);
    d->itemChangeHints << hint;
}

// --------------------------------------------------------------------------------------------

ScanControllerLoadingCacheFileWatch::ScanControllerLoadingCacheFileWatch()
{
    DatabaseWatch* dbwatch = DatabaseAccess::databaseWatch();

    // we opt for a queued connection to make stuff a bit relaxed
    connect(dbwatch, SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotImageChanged(ImageChangeset)),
            Qt::QueuedConnection);
}

void ScanControllerLoadingCacheFileWatch::slotImageChanged(const ImageChangeset& changeset)
{
    DatabaseAccess access;

    foreach(const qlonglong& imageId, changeset.ids())
    {
        DatabaseFields::Set changes = changeset.changes();

        if (changes & DatabaseFields::ModificationDate
            || changes & DatabaseFields::Orientation)
        {
            ImageInfo info(imageId);
            //kDebug() << imageId << info.filePath();
            notifyFileChanged(info.filePath());
        }
    }
}

}  // namespace Digikam
