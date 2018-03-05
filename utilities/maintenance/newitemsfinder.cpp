/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-20
 * Description : new items finder.
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "newitemsfinder.h"

// Qt includes

#include <QApplication>
#include <QTimer>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "scancontroller.h"

namespace Digikam
{

class NewItemsFinder::Private
{
public:

    Private()
        : mode(CompleteCollectionScan),
          cancel(false)
    {
    }

    FinderMode  mode;

    bool        cancel;

    QStringList foldersToScan;
    QStringList foldersScanned;
};

NewItemsFinder::NewItemsFinder(const FinderMode mode, const QStringList& foldersToScan, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("NewItemsFinder"), parent),
      d(new Private)
{
    setLabel(i18n("Find new items"));
    setThumbnail(QIcon::fromTheme(QLatin1String("view-refresh")).pixmap(22));
    setShowAtStart(true);
    ProgressManager::addProgressItem(this);

    d->mode = mode;

    // Common conections to ScanController

    connect(ScanController::instance(), SIGNAL(collectionScanStarted(QString)),
            this, SLOT(slotScanStarted(QString)));

    connect(ScanController::instance(), SIGNAL(totalFilesToScan(int)),
            this, SLOT(slotTotalFilesToScan(int)));

    connect(ScanController::instance(), SIGNAL(filesScanned(int)),
            this, SLOT(slotFilesScanned(int)));

    // Connection and rules for ScheduleCollectionScan mode.

    connect(ScanController::instance(), SIGNAL(partialScanDone(QString)),
            this, SLOT(slotPartialScanDone(QString)));

    // If we are scanning for newly imported files, we need to have the folders for scanning...
    if (mode == ScheduleCollectionScan && foldersToScan.isEmpty())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "NewItemsFinder called without any folders. Wrong call.";
    }

    d->foldersToScan = foldersToScan;
    d->foldersToScan.sort();
}

NewItemsFinder::~NewItemsFinder()
{
    delete d;
}

void NewItemsFinder::slotStart()
{
    MaintenanceTool::slotStart();

    switch (d->mode)
    {
        case ScanDeferredFiles:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "scan mode: ScanDeferredFiles";

            connect(ScanController::instance(), SIGNAL(completeScanDone()),
                    this, SLOT(slotDone()));

            ScanController::instance()->completeCollectionScanInBackground(false);
            ScanController::instance()->allowToScanDeferredFiles();
            break;
        }

        case CompleteCollectionScan:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "scan mode: CompleteCollectionScan";

            ScanController::instance()->completeCollectionScanInBackground(false);

            if (d->cancel)
            {
                break;
            }

            connect(ScanController::instance(), SIGNAL(completeScanDone()),
                    this, SLOT(slotDone()));

            ScanController::instance()->allowToScanDeferredFiles();
            ScanController::instance()->completeCollectionScanInBackground(true);
            break;
        }

        case ScheduleCollectionScan:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "scan mode: ScheduleCollectionScan :: " << d->foldersToScan;
            d->foldersScanned.clear();

            foreach(const QString& folder, d->foldersToScan)
            {
                if (d->cancel)
                {
                    break;
                }

                ScanController::instance()->scheduleCollectionScan(folder);
            }

            break;
        }
    }
}

void NewItemsFinder::slotScanStarted(const QString& info)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << info;
    setStatus(info);
}

void NewItemsFinder::slotTotalFilesToScan(int t)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "total scan value : " << t;
    setTotalItems(t);
}

void NewItemsFinder::slotFilesScanned(int s)
{
    //qCDebug(DIGIKAM_GENERAL_LOG) << "file scanned : " << s;
    advance(s);
}

void NewItemsFinder::slotCancel()
{
    d->cancel = true;

    ScanController::instance()->cancelCompleteScan();
    MaintenanceTool::slotCancel();
}

void NewItemsFinder::slotPartialScanDone(const QString& path)
{
    // Check if path scanned is included in planed list.

    if (d->foldersToScan.contains(path) && !d->foldersScanned.contains(path))
    {
        d->foldersScanned.append(path);
        d->foldersScanned.sort();

        // Check if all planed scanning is done
        if (d->foldersScanned == d->foldersToScan)
        {
            slotDone();
        }
    }
}

}  // namespace Digikam
