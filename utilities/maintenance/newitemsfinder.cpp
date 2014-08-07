/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-20
 * Description : new items finder.
 *
 * Copyright (C) 2012-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "newitemsfinder.moc"

// Qt includes

#include <QApplication>
#include <QTimer>

// KDE includes

#include <klocale.h>
#include <kicon.h>
#include <kdebug.h>

// Local includes

#include "scancontroller.h"

namespace Digikam
{

class NewItemsFinder::Private
{
public:

    Private() :
        mode(CompleteCollectionScan)
    {
    }

    FinderMode  mode;
    QStringList foldersToScan;
    QStringList foldersScanned;
};

NewItemsFinder::NewItemsFinder(const FinderMode mode, const QStringList& foldersToScan, ProgressItem* const parent)
    : MaintenanceTool("NewItemsFinder", parent), d(new Private)
{
    setLabel(i18n("Find new items"));
    setThumbnail(KIcon("view-refresh").pixmap(22));
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
    if(mode == ScheduleCollectionScan && foldersToScan.isEmpty())
    {
        kWarning() << "NewItemsFinder called without any folders. Wrong call.";
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
            kDebug() << "scan mode: ScanDeferredFiles";

            connect(ScanController::instance(), SIGNAL(completeScanDone()),
                    this, SLOT(slotDone()));

            ScanController::instance()->completeCollectionScanInBackground(false);
            ScanController::instance()->allowToScanDeferredFiles();
            break;
        }

        case CompleteCollectionScan:
        {
            kDebug() << "scan mode: CompleteCollectionScan";

            ScanController::instance()->completeCollectionScanInBackground(false);

            connect(ScanController::instance(), SIGNAL(completeScanDone()),
                    this, SLOT(slotDone()));

            ScanController::instance()->allowToScanDeferredFiles();
            ScanController::instance()->completeCollectionScanInBackground(true);
            break;
        }

        case ScheduleCollectionScan:
        {
            kDebug() << "scan mode: ScheduleCollectionScan :: " << d->foldersToScan;
            d->foldersScanned.clear();

            foreach(const QString& folder, d->foldersToScan)
                ScanController::instance()->scheduleCollectionScan(folder);

            break;
        }
    }
}

void NewItemsFinder::slotScanStarted(const QString& info)
{
    kDebug() << info;
    setStatus(info);
}

void NewItemsFinder::slotTotalFilesToScan(int t)
{
    kDebug() << "total scan value : " << t;
    setTotalItems(t);
}

void NewItemsFinder::slotFilesScanned(int s)
{
    //kDebug() << "file scanned : " << s;
    advance(s);
}

void NewItemsFinder::slotCancel()
{
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
