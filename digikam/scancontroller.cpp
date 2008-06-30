/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-01
 * Description : scan pictures interface.
 * 
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

// Qt includes.

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

// KDE includes.

#include <kmessagebox.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>

// Local includes.

#include "ddebug.h"
#include "collectionscanner.h"
#include "databaseaccess.h"
#include "dprogressdlg.h"
#include "dmetadata.h"
#include "albumsettings.h"
#include "albumdb.h"
#include "albummanager.h"
#include "splashscreen.h"
#include "scancontroller.h"
#include "scancontroller.moc"

namespace Digikam
{

class ScanControllerPriv
{
public:

    ScanControllerPriv()
    {
        splash              = 0;
        progressDialog      = 0;
        running             = false;
        needsInitialization = false;
        needsCompleteScan   = false;
        eventLoop           = 0;
        showTimer           = 0;
        advice              = ScanController::Success;
    }

    bool                 running;
    bool                 needsInitialization;
    bool                 needsCompleteScan;

    QStringList          scanTasks;

    QMutex               mutex;
    QWaitCondition       condVar;

    QEventLoop          *eventLoop;

    QTimer              *showTimer;

    QPixmap              albumPix;
    QPixmap              rootPix;
    QPixmap              actionPix;
    QPixmap              errorPix;

    DProgressDlg        *progressDialog;

    SplashScreen        *splash;

    ScanController::Advice advice;

    QPixmap albumPixmap()
    {
        if (albumPix.isNull())
            albumPix = KIconLoader::global()->loadIcon("folder-image", KIconLoader::NoGroup, 32);
        return albumPix;
    }

    QPixmap rootPixmap()
    {
        if (rootPix.isNull())
            rootPix = KIconLoader::global()->loadIcon("folder-open", KIconLoader::NoGroup, 32);
        return rootPix;
    }

    QPixmap actionPixmap()
    {
        if (actionPix.isNull())
            actionPix = KIconLoader::global()->loadIcon("system-run", KIconLoader::NoGroup, 32);
        return actionPix;
    }

    QPixmap errorPixmap()
    {
        if (errorPix.isNull())
            errorPix = KIconLoader::global()->loadIcon("dialog-error", KIconLoader::NoGroup, 32);
        return errorPix;
    }

};

class ScanControllerCreator { public: ScanController object; };
K_GLOBAL_STATIC(ScanControllerCreator, creator)

ScanController* ScanController::instance()
{
    return &creator->object;
}

ScanController::ScanController()
{
    d = new ScanControllerPriv;

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

    // interthread connections
    connect(this, SIGNAL(errorFromInitialization(const QString &)),
            this, SLOT(slotErrorFromInitialization(const QString &)));

    connect(this, SIGNAL(progressFromInitialization(const QString &, int)),
            this, SLOT(slotProgressFromInitialization(const QString &, int)));

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
        return;

    d->running = false;
    {
        QMutexLocker lock(&d->mutex);
        d->condVar.wakeAll();
    }

    wait();
}

void ScanController::createProgressDialog()
{
    if (d->progressDialog)
        return;

    d->progressDialog = new DProgressDlg(0);
    d->progressDialog->setInitialSize(QSize(500, 100));
    d->progressDialog->setActionListVSBarVisible(false);
    d->progressDialog->setWhatsThis( i18n("This shows the progress of the scan. "
                                      "During the scan, all files on disk "
                                      "are registered in a database.") );

    d->progressDialog->setMaximum(1);
    d->progressDialog->setValue(1);

    connect(this, SIGNAL(incrementProgressDialog(int)),
            d->progressDialog, SLOT(incrementMaximum(int)));
}

void ScanController::slotTriggerShowProgressDialog()
{
    if (d->progressDialog && !d->showTimer->isActive() && !d->progressDialog->isVisible())
        d->showTimer->start(300);
}

void ScanController::slotShowProgressDialog()
{
    if (d->progressDialog && !d->splash)
        d->progressDialog->show();
}

ScanController::Advice ScanController::databaseInitialization()
{
    d->advice = Success;
    createProgressDialog();
    {
        QMutexLocker lock(&d->mutex);
        d->needsInitialization = true;
        d->condVar.wakeAll();
    }
    // loop is quit by signal
    d->eventLoop->exec();

    delete d->progressDialog;
    d->progressDialog = 0;

    return d->advice;
}

void ScanController::completeCollectionScan(SplashScreen *splash)
{
    d->splash = splash;
    createProgressDialog();
    {
        QMutexLocker lock(&d->mutex);
        d->needsCompleteScan = true;
        d->condVar.wakeAll();
    }
    // loop is quit by signal
    d->eventLoop->exec();

    delete d->progressDialog;
    d->progressDialog = 0;
    // We do not delete Splashscreen here.
    d->splash         = 0;
}

void ScanController::scheduleCollectionScan(const QString &path)
{
    QMutexLocker lock(&d->mutex);
    if (!d->scanTasks.contains(path))
        d->scanTasks << path;
    d->condVar.wakeAll();
}

void ScanController::run()
{
    while (d->running)
    {
        bool doInit = false, doScan = false, doPartialScan = false;
        QString task;
        {
            QMutexLocker lock(&d->mutex);
            if (d->needsInitialization)
            {
                d->needsInitialization = false;
                doInit = true;
            }
            else if (d->needsCompleteScan)
            {
                d->needsCompleteScan = false;
                doScan = true;
            }
            else if (!d->scanTasks.isEmpty())
            {
                doPartialScan = true;
                task = d->scanTasks.takeFirst();
            }
            else
                d->condVar.wait(&d->mutex);
        }

        if (doInit)
        {
            // pass "this" as InitializationObserver
            bool success = DatabaseAccess::checkReadyForUse(this);
            // If d->advice has not been adjusted to a value indicating failure, do this here
            if (!success && d->advice == Success)
                d->advice = ContinueWithoutDatabase;
            emit databaseInitialized(success);
        }
        else if (doScan)
        {
            CollectionScanner scanner;
            connectCollectionScanner(&scanner);
            scanner.completeScan();
            emit completeScanDone();
        }
        else if (doPartialScan)
        {
            CollectionScanner scanner;
            connectCollectionScanner(&scanner);
            scanner.partialScan(task);
        }
    }
}

// (also implementing InitializationObserver)
void ScanController::connectCollectionScanner(CollectionScanner *scanner)
{
    scanner->setSignalsEnabled(true);

    connect(scanner, SIGNAL(totalFilesToScan(int)),
            this, SLOT(slotTotalFilesToScan(int)));

    connect(scanner, SIGNAL(startScanningAlbum(const QString&, const QString&)),
            this, SLOT(slotStartScanningAlbum(const QString&, const QString&)));

    connect(scanner, SIGNAL(finishedScanningAlbum(const QString&, const QString&, int)),
            this, SLOT(slotFinishedScanningAlbum(const QString&, const QString&, int)));

    connect(scanner, SIGNAL(startScanningAlbumRoot(const QString&)),
            this, SLOT(slotStartScanningAlbumRoot(const QString&)));

    connect(scanner, SIGNAL(startScanningForStaleAlbums()),
            this, SLOT(slotStartScanningForStaleAlbums()));

    connect(scanner, SIGNAL(startScanningAlbumRoots()),
            this, SLOT(slotStartScanningAlbumRoots()));

    connect(scanner, SIGNAL(startCompleteScan()),
            this, SLOT(slotTriggerShowProgressDialog()));
}

void ScanController::slotTotalFilesToScan(int count)
{
    if (d->progressDialog)
        d->progressDialog->incrementMaximum(count);
}

void ScanController::slotStartScanningAlbum(const QString &albumRoot, const QString &album)
{
    Q_UNUSED(albumRoot);
    if (d->progressDialog)
        d->progressDialog->addedAction(d->albumPixmap(), " " + album);
}

void ScanController::slotFinishedScanningAlbum(const QString &, const QString &, int filesScanned)
{
    if (d->progressDialog)
        d->progressDialog->advance(filesScanned);
}

void ScanController::slotStartScanningAlbumRoot(const QString &albumRoot)
{
    if (d->progressDialog)
        d->progressDialog->addedAction(d->rootPixmap(), albumRoot);
}

void ScanController::slotStartScanningForStaleAlbums()
{
    QString message = i18n("Scanning for removed albums");
    if (d->splash)
        d->splash->message(message, Qt::AlignLeft, Qt::white);
    else if (d->progressDialog)
        d->progressDialog->addedAction(d->actionPixmap(), message);
}

void ScanController::slotStartScanningAlbumRoots()
{
    QString message = i18n("Scanning images in individual albums");
    if (d->splash)
        d->splash->message(message, Qt::AlignLeft, Qt::white);
    else if (d->progressDialog)
        d->progressDialog->addedAction(d->actionPixmap(), message);
}

// implementing InitializationObserver
void ScanController::moreSchemaUpdateSteps(int numberOfSteps)
{
    // not from main thread
    emit triggerShowProgressDialog();
    emit incrementProgressDialog(numberOfSteps);
}

// implementing InitializationObserver
void ScanController::schemaUpdateProgress(const QString &message, int numberOfSteps)
{
    // not from main thread
    emit progressFromInitialization(message, numberOfSteps);
}

void ScanController::slotProgressFromInitialization(const QString& message, int numberOfSteps)
{
    // main thread

    if (d->splash)
        d->splash->message(message, Qt::AlignLeft, Qt::white);
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
    switch(result)
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
void ScanController::error(const QString &errorMessage)
{
    // not from main thread
    emit errorFromInitialization(errorMessage);
}

void ScanController::slotErrorFromInitialization(const QString &errorMessage)
{
    // main thread
    QString message = i18n("Error");
    if (d->splash)
        d->splash->message(message, Qt::AlignLeft, Qt::white);
    else if (d->progressDialog)
        d->progressDialog->addedAction(d->errorPixmap(), message);

    KMessageBox::error(d->progressDialog, errorMessage);
}


}  // namespace Digikam
