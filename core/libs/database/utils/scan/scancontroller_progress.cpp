/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-10-28
 * Description : scan item controller - progress operations.
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

void ScanController::slotTotalFilesToScan(int count)
{
    if (d->progressDialog)
    {
        d->progressDialog->incrementMaximum(count);
    }

    d->totalFilesToScan = count;
    emit totalFilesToScan(d->totalFilesToScan);
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
void ScanController::error(const QString& errorMessage)
{
    // not from main thread
    emit errorFromInitialization(errorMessage);
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

AlbumCopyMoveHint ScanController::hintForAlbum(const PAlbum* const album,
                                               int dstAlbumRootId,
                                               const QString& relativeDstPath,
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

    return AlbumCopyMoveHint(album->albumRootId(),
                             album->id(),
                             dstAlbumRootId,
                             dstAlbumPath);
}

QList<AlbumCopyMoveHint> ScanController::hintsForAlbum(const PAlbum* const album,
                                                       int dstAlbumRootId,
                                                       QString relativeDstPath,
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
        newHints << hintForAlbum(a,
                                 dstAlbumRootId,
                                 relativeDstPath,
                                 albumName + childAlbumPath.mid(parentAlbumPath.length()));
    }

    return newHints;
}

void ScanController::hintAtMoveOrCopyOfAlbum(const PAlbum* const album,
                                             const QString& dstPath,
                                             const QString& newAlbumName)
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

    QList<AlbumCopyMoveHint> newHints = hintsForAlbum(album,
                                                      location.id(),
                                                      relativeDstPath,
                                                      newAlbumName.isNull() ? album->title()
                                                                            : newAlbumName);

    //QMutexLocker lock(&d->mutex);
    //d->albumHints << newHints;
    d->hints->recordHints(newHints);
}

void ScanController::hintAtMoveOrCopyOfAlbum(const PAlbum* const album,
                                             const PAlbum* const dstAlbum,
                                             const QString& newAlbumName)
{
    QList<AlbumCopyMoveHint> newHints = hintsForAlbum(album,
                                                      dstAlbum->albumRootId(),
                                                      dstAlbum->albumPath(),
                                                      newAlbumName.isNull() ? album->title()
                                                                            : newAlbumName);

    //QMutexLocker lock(&d->mutex);
    //d->albumHints << newHints;
    d->hints->recordHints(newHints);
}

void ScanController::hintAtMoveOrCopyOfItems(const QList<qlonglong> ids,
                                             const PAlbum* const dstAlbum,
                                             const QStringList& itemNames)
{
    ItemCopyMoveHint hint(ids,
                          dstAlbum->albumRootId(),
                          dstAlbum->id(),
                          itemNames);

    d->garbageCollectHints(true);
    //d->itemHints << hint;
    d->hints->recordHints(QList<ItemCopyMoveHint>() << hint);
}

void ScanController::hintAtMoveOrCopyOfItem(qlonglong id,
                                            const PAlbum* const dstAlbum,
                                            const QString& itemName)
{
    ItemCopyMoveHint hint(QList<qlonglong>() << id,
                          dstAlbum->albumRootId(),
                          dstAlbum->id(),
                          QStringList() << itemName);

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

void ScanController::slotTriggerShowProgressDialog()
{
    if (d->progressDialog && !d->showTimer->isActive() && !d->progressDialog->isVisible())
    {
        d->showTimer->start(300);
    }
}

} // namespace Digikam
