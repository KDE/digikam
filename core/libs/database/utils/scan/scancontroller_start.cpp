/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-10-28
 * Description : scan item controller - start operations.
 *
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "scancontroller_p.h"

namespace Digikam
{

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
    d->progressDialog = nullptr;

    return d->advice;
}

void ScanController::completeCollectionScanDeferFiles()
{
    completeCollectionScan(true);
}

void ScanController::completeCollectionScan(bool defer)
{
    createProgressDialog();

    // we only need to count the files in advance
    // if we show a progress percentage in progress dialog
    completeCollectionScanCore(!CollectionScanner::databaseInitialScanDone(), defer);

    delete d->progressDialog;
    d->progressDialog = nullptr;
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

void ScanController::scheduleCollectionScanExternal(const QString& path)
{
    d->externalTimer->start();

    QMutexLocker lock(&d->mutex);

    if (!d->scanTasks.contains(path))
    {
        d->scanTasks << path;
    }
}

void ScanController::scanFileDirectly(const QString& filePath)
{
    suspendCollectionScan();

    CollectionScanner scanner;
    scanner.setHintContainer(d->hints);
    scanner.scanFile(filePath);

    resumeCollectionScan();
}

void ScanController::scanFileDirectlyNormal(const ItemInfo& info)
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
    ItemInfo dest(id), source(parentVersion);
    scanner.copyFileProperties(source, dest);

    resumeCollectionScan();
}
*/

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

void ScanController::beginFileMetadataWrite(const ItemInfo& info)
{
    {
        // throw in a lock to synchronize with all parallel writing
        FileReadLocker locker(info.filePath());
    }

    QFileInfo fi(info.filePath());
    d->hints->recordHint(ItemMetadataAdjustmentHint(info.id(),
                                                    ItemMetadataAdjustmentHint::AboutToEditMetadata,
                                                    fi.lastModified(),
                                                    fi.size()));
}

} // namespace Digikam
