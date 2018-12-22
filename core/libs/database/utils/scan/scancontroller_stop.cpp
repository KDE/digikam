/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-10-28
 * Description : scan item controller - stop operations.
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

void ScanController::slotCancelPressed()
{
    abortInitialization();
    cancelCompleteScan();
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

void ScanController::finishFileMetadataWrite(const ItemInfo& info, bool changed)
{
    QFileInfo fi(info.filePath());
    d->hints->recordHint(ItemMetadataAdjustmentHint(info.id(),
                                                    changed ? ItemMetadataAdjustmentHint::MetadataEditingFinished :
                                                              ItemMetadataAdjustmentHint::MetadataEditingAborted,
                                                    fi.lastModified(),
                                                    fi.size()));

    scanFileDirectlyNormal(info);
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

void ScanController::slotRelaxedScanning()
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "Starting scan!";
    d->externalTimer->stop();
    d->relaxedTimer->stop();

    QMutexLocker lock(&d->mutex);
    d->condVar.wakeAll();
}

} // namespace Digikam
