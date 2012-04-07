/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-20
 * Description : new items finder.
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QTimer>

// KDE includes

#include <klocale.h>
#include <kicon.h>
#include <kdebug.h>

// Local includes

#include "scancontroller.h"

namespace Digikam
{

class NewItemsFinder::NewItemsFinderPriv
{
public:

    NewItemsFinderPriv() :
        mode(CompleteCollectionScan)
    {
    }

    FinderMode  mode;
    QStringList foldersToScan;
};

NewItemsFinder::NewItemsFinder(FinderMode mode, const QStringList& foldersToScan, ProgressItem* parent)
    : MaintenanceTool("NewItemsFinder", parent), d(new NewItemsFinderPriv)
{
    connect(ScanController::instance(), SIGNAL(collectionScanStarted(QString)),
            this, SLOT(slotScanStarted(QString)));

    connect(ScanController::instance(), SIGNAL(totalFilesToScan(int)),
            this, SLOT(slotTotalFilesToScan(int)));

    connect(ScanController::instance(), SIGNAL(filesScanned(int)),
            this, SLOT(slotFilesScanned(int)));

    connect(ScanController::instance(), SIGNAL(completeScanDone()),
            this, SLOT(slotDone()));

    d->mode          = mode;
    d->foldersToScan = foldersToScan;
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

            ScanController::instance()->allowToScanDeferredFiles();
            break;
        }

        case CompleteCollectionScan:
        {
            kDebug() << "scan mode: CompleteCollectionScan";

            ScanController::instance()->completeCollectionScan();
            break;
        }

        case ScheduleCollectionScan:
        {
            kDebug() << "scan mode: ScheduleCollectionScan";

            foreach(const QString& folder, d->foldersToScan)
                ScanController::instance()->scheduleCollectionScan(folder);
            break;
        }
    }
}

void NewItemsFinder::slotScanStarted(const QString& info)
{
    ProgressManager::addProgressItem(this);
    setLabel(i18n("Find new items"));
    setStatus(info);
    setThumbnail(KIcon("view-refresh").pixmap(22));
}

void NewItemsFinder::slotTotalFilesToScan(int t)
{
    //kDebug() << "total scan value : " << t;
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

void NewItemsFinder::slotDone()
{
    //kDebug() << "fired!! ";
    MaintenanceTool::slotDone();
}

}  // namespace Digikam
