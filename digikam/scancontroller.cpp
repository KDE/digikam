/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-01
 * Description : scan pictures interface.
 * 
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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
#include "scancontroller.h"
#include "scancontroller.moc"

namespace Digikam
{

class ScanControllerPriv
{
public:

    ScanControllerPriv()
    {
        progressDialog      = 0;
        running             = false;
        needsInitialization = false;
        needsCompleteScan   = false;
        eventLoop           = 0;
        advice              = ScanController::Success;
    }

    DProgressDlg        *progressDialog;

    bool                 running;
    bool                 needsInitialization;
    bool                 needsCompleteScan;
    QStringList          scanTasks;

    QMutex               mutex;
    QWaitCondition       condVar;

    QEventLoop          *eventLoop;

    QPixmap              albumPix;
    QPixmap              rootPix;
    QPixmap              actionPix;
    QPixmap              errorPix;

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
K_GLOBAL_STATIC(ScanControllerCreator, creator);

ScanController* ScanController::instance()
{
    return &creator->object;
}

ScanController::ScanController()
{
    d->eventLoop = new QEventLoop(this);

    connect(this, SIGNAL(databaseInitialized(bool)),
            d->eventLoop, SLOT(quit()));

    connect(this, SIGNAL(completeScanDone()),
            d->eventLoop, SLOT(quit()));
}


ScanController::~ScanController()
{
    d->running = false;
    {
        QMutexLocker lock(&d->mutex);
        d->condVar.wakeAll();
    }

    wait();

    delete d->progressDialog;
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

void ScanController::completeCollectionScan()
{
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
            bool success = DatabaseAccess::checkReadyForUse(this);
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
}

void ScanController::slotTotalFilesToScan(int count)
{
    d->progressDialog->incrementMaximum(count);
    if (count > 0)
        d->progressDialog->show();
}

void ScanController::slotStartScanningAlbum(const QString &albumRoot, const QString &album)
{
    Q_UNUSED(albumRoot);
    d->progressDialog->addedAction(d->albumPixmap(), " " + album);
}

void ScanController::slotFinishedScanningAlbum(const QString &, const QString &, int filesScanned)
{
    d->progressDialog->advance(filesScanned);
}

void ScanController::slotStartScanningAlbumRoot(const QString &albumRoot)
{
    d->progressDialog->addedAction(d->rootPixmap(), albumRoot);
}

void ScanController::slotStartScanningForStaleAlbums()
{
    d->progressDialog->addedAction(d->actionPixmap(), i18n("Scanning for removed albums"));
}

void ScanController::slotStartScanningAlbumRoots()
{
    d->progressDialog->addedAction(d->actionPixmap(), i18n("Scanning images in individual albums"));
}

void ScanController::moreSchemaUpdateSteps(int numberOfSteps)
{
    d->progressDialog->incrementMaximum(numberOfSteps);
}

void ScanController::schemaUpdateProgress(const QString &message, int numberOfSteps)
{
    d->progressDialog->addedAction(d->actionPixmap(), message);
    d->progressDialog->advance(numberOfSteps);
}

void ScanController::finishedSchemaUpdate(UpdateResult result)
{
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

void ScanController::error(const QString &errorMessage)
{
    d->progressDialog->addedAction(d->errorPixmap(), i18n("Error"));
    KMessageBox::error(d->progressDialog, errorMessage);
}


#if 0
void ScanController::findFoldersWhichDoNotExist()
{
    m_scanner.scanForStaleAlbums();
    QStringList toBeDeleted = m_scanner.formattedListOfStaleAlbums();

    if (!toBeDeleted.isEmpty())
    {
        int rc = KMessageBox::warningYesNoList(0,
            i18np("<p>There is an album in the database which does not appear to "
                  "be on disk. This album should be removed from the database, "
                  "however you may lose information because all images "
                  "associated with this album will be removed from the database "
                  "as well.<p>"
                  "digiKam cannot continue without removing the items "
                  "from the database because all views depend on the information "
                  "in the database. Do you want them to be removed from the "
                  "database?",
                  "<p>There are %1 albums in the database which do not appear to "
                  "be on disk. These albums should be removed from the database, "
                  "however you may lose information because all images "
                  "associated with these albums will be removed from the database "
                  "as well.<p>"
                  "digiKam cannot continue without removing the items "
                  "from the database because all views depend on the information "
                  "in the database. Do you want them to be removed from the "
                  "database?",
                  toBeDeleted.count()),
            toBeDeleted,
            i18n("Albums are Missing"));

        if (rc != KMessageBox::Yes)
            exit(0);

        m_scanner.removeStaleAlbums();
    }
}

void ScanController::findMissingItems()
{
    d->progressDialog->setAllowCancel(false);
    d->progressDialog->showCancelButton(false);
    d->progressDialog->setValue(0);
    d->progressDialog->setLabel(i18n("Scanning items, please wait..."));
    qApp->processEvents();

    m_scanner.scanAlbums();

    d->progressDialog->hide();
    kapp->processEvents();
}

void ScanController::slotScanningFile(const QString &)
{
    d->progressDialog->advance(1);
    if (d->progressDialog->value() % 30 == 0)
        qApp->processEvents();
}

void ScanController::updateItemsWithoutDate()
{
    d->progressDialog->setAllowCancel( false );
    d->progressDialog->showCancelButton (false );
    d->progressDialog->setValue(0);
    d->progressDialog->setLabel(i18n("Updating items, please wait..."));
    kapp->processEvents();

    m_scanner.updateItemsWithoutDate();

    d->progressDialog->hide();
    qApp->processEvents();
}

void ScanController::deleteStaleEntries()
{
    QStringList listToBeDeleted = m_scanner.formattedListOfStaleFiles();

    if ( !listToBeDeleted.isEmpty() )
    {
        int rc = KMessageBox::warningYesNoList(0,
          i18np("<p>There is an item in the database which does not "
                "appear to be on disk or is located in the root album of "
                "the path. This file should be removed from the "
                "database, however you may lose information.<p>"
                "digiKam cannot continue without removing the item from "
                "the database because all views depend on the information "
                "in the database. Do you want it to be removed from the "
                "database?",
                "<p>There are %1 items in the database which do not "
                "appear to be on disk or are located in the root album of "
                "the path. These files should be removed from the "
                "database, however you may lose information.<p>"
                "digiKam cannot continue without removing these items from "
                "the database because all views depend on the information "
                "in the database. Do you want them to be removed from the "
                "database?",
                listToBeDeleted.count()),
          listToBeDeleted,
          i18n("Files are Missing"));

        if (rc != KMessageBox::Yes)
            exit(0);

        m_scanner.removeStaleFiles();
    }
}
#endif

}  // namespace Digikam
