/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-01-29
 * Description : Database cleaner.
 *
 * Copyright (C) 2017      by Mario Frank <mario dot frank at uni minus potsdam dot de>
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

#include "dbcleaner.h"

// Qt includes

#include <QIcon>
#include <QMessageBox>
#include <QThread>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikamapp.h"
#include "maintenancethread.h"

namespace Digikam
{

class DbCleaner::Private
{
public:

    Private() :
        thread(0),
        cleanThumbsDb(false),
        cleanFacesDb(false),
        shrinkDatabases(false),
        databasesToAnalyseCount(1),
        databasesToShrinkCount(0),
        messageBox(0)
    {
    }
    
    ~Private()
    {
        delete messageBox;
    }

    MaintenanceThread*           thread;
    bool                         cleanThumbsDb;
    bool                         cleanFacesDb;
    bool                         shrinkDatabases;

    QList<qlonglong>             imagesToRemove;
    QList<int>                   staleThumbnails;
    QList<FacesEngine::Identity> staleIdentities;

    int                          databasesToAnalyseCount;
    int                          databasesToShrinkCount;

    QMessageBox*                 messageBox;

    QString                      coreDbStatus;
    QString                      thumbsDbStatus;
    QString                      recognitionDbStatus;
};

    QString DbCleaner::VACUUM_PENDING(QLatin1String("&#62;"));
    QString DbCleaner::VACUUM_DONE(QLatin1String("&#10003;"));

DbCleaner::DbCleaner(bool cleanThumbsDb, bool cleanFacesDb, bool shrinkDatabases, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("DbCleaner"), parent),
      d(new Private)
{
    // register the identity list as meta type to be able to use it in signal/slot connection
    qRegisterMetaType<QList<FacesEngine::Identity>>("QList<FacesEngine::Identity>");

    d->cleanThumbsDb   = cleanThumbsDb;
    if (cleanThumbsDb)
    {
        d->databasesToAnalyseCount = d->databasesToAnalyseCount + 1;
    }

    d->cleanFacesDb    = cleanFacesDb;
    if (cleanFacesDb)
    {
        d->databasesToAnalyseCount = d->databasesToAnalyseCount + 1;
    }

    d->shrinkDatabases = shrinkDatabases;

    if (shrinkDatabases)
    {
        d->databasesToShrinkCount = 3;
    }

    d->thread          = new MaintenanceThread(this);

    d->messageBox      = new QMessageBox(DigikamApp::instance());

    connect(d->thread, SIGNAL(signalAdvance()),
            this, SLOT(slotAdvance()));

}

DbCleaner::~DbCleaner()
{
    delete d;
}

void DbCleaner::slotStart()
{
    MaintenanceTool::slotStart();
    setLabel(i18n("Clean up the databases : ") + i18n("analysing databases"));
    setThumbnail(QIcon::fromTheme(QLatin1String("tools-wizard")).pixmap(22));
    ProgressManager::addProgressItem(this);

    // Set one item to make sure that the progress bar is shown.
    setTotalItems(d->databasesToAnalyseCount + d->databasesToShrinkCount);
    //qCDebug(DIGIKAM_GENERAL_LOG) << "Completed items at start: " << completedItems() << "/" << totalItems();

    connect(d->thread,SIGNAL(signalCompleted()),
            this, SLOT(slotCleanItems()));

    // Set the wiring from the data signal to the data slot.
    connect(d->thread,SIGNAL(signalData(QList<qlonglong>,QList<int>,QList<FacesEngine::Identity>)),
            this, SLOT(slotFetchedData(QList<qlonglong>,QList<int>,QList<FacesEngine::Identity>)));

    // Compute the database junk. This will lead to the call of the slot
    // slotFetchedData.
    d->thread->computeDatabaseJunk(d->cleanThumbsDb,d->cleanFacesDb);
    d->thread->start();
}

void DbCleaner::slotFetchedData(const QList<qlonglong>& staleImageIds,
                                const QList<int>& staleThumbIds,
                                const QList<FacesEngine::Identity>& staleIdentities)
{
    // We have data now. Store it and trigger the core db cleaning
    d->imagesToRemove  = staleImageIds;
    d->staleThumbnails = staleThumbIds;
    d->staleIdentities = staleIdentities;

    // If we have nothing to do, finish.
    // Signal done if no elements cleanup is necessary

    if (d->imagesToRemove.isEmpty() && d->staleThumbnails.isEmpty() && d->staleIdentities.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Nothing to do. Databases are clean.";
        if (d->shrinkDatabases)
        {
            disconnect(d->thread,SIGNAL(signalData(QList<qlonglong>,QList<int>,QList<FacesEngine::Identity>)),
                       this, SLOT(slotFetchedData(QList<qlonglong>,QList<int>,QList<FacesEngine::Identity>)));

            disconnect(d->thread,SIGNAL(signalCompleted()),
                        this, SLOT(slotCleanItems()));

            slotShrinkDatabases();
        }
        else
        {
            MaintenanceTool::slotDone();
            return;
        }
    }

    setTotalItems(totalItems() + d->imagesToRemove.size() + d->staleThumbnails.size() + d->staleIdentities.size());
    //qCDebug(DIGIKAM_GENERAL_LOG) << "Completed items after analysis: " << completedItems() << "/" << totalItems();
}

void DbCleaner::slotCleanItems()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Cleaning core db.";

    disconnect(d->thread, SIGNAL(signalCompleted()),
               this, SLOT(slotCleanItems()));

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotCleanedItems()));

    if (d->imagesToRemove.size() > 0)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Found " << d->imagesToRemove.size() << " obsolete image entries.";

        setLabel(i18n("Clean up the databases : ") + i18n("cleaning core db"));

        // GO!
        d->thread->cleanCoreDb(d->imagesToRemove);
        d->thread->start();
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Core DB is clean.";
        slotCleanedItems();
    }
}

void DbCleaner::slotCleanedItems()
{
    // We cleaned the items. Now clean the thumbs db
    disconnect(d->thread, SIGNAL(signalCompleted()),
               this, SLOT(slotCleanedItems()));

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotCleanedThumbnails()));

    if (d->cleanThumbsDb)
    {
        if (d->staleThumbnails.size() > 0)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Found " << d->staleThumbnails.size() << " stale thumbnails.";
            setLabel(i18n("Clean up the databases : ") + i18n("cleaning thumbnails db"));

            // GO!
            d->thread->cleanThumbsDb(d->staleThumbnails);
            d->thread->start();
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Thumbnail DB is clean.";
            slotCleanedThumbnails();
        }
    }
    else
    {
        slotCleanedThumbnails();
    }
}

void DbCleaner::slotCleanedThumbnails()
{
    // We cleaned the thumbnails. Now clean the recognition db
    disconnect(d->thread, SIGNAL(signalCompleted()),
               this, SLOT(slotCleanedThumbnails()));

    if (d->cleanFacesDb)
    {
        if (d->staleIdentities.count() > 0)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Found " << d->staleIdentities.size() << " stale face identities.";
            setLabel(i18n("Clean up the databases : ") + i18n("cleaning recognition db"));

            // GO! and don't forget the signal!
            connect(d->thread, SIGNAL(signalCompleted()),
                    this, SLOT(slotCleanedFaces()));

            // We cleaned the thumbs db. Now clean the faces db.
            d->thread->cleanFacesDb(d->staleIdentities);
            d->thread->start();
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Faces DB is clean.";
            slotCleanedFaces();
        }
    }
    else
    {
        slotCleanedFaces();
    }
}

void DbCleaner::slotCleanedFaces()
{
    // We cleaned the recognition db. We are done.
    if (d->shrinkDatabases)
    {
        slotShrinkDatabases();
    }
    slotDone();
}

void DbCleaner::slotShrinkDatabases()
{
    setLabel(i18n("Clean up the databases : ") + i18n("shrinking databases"));

    disconnect(d->thread, SIGNAL(signalCompleted()),
               this, SLOT(slotCleanedFaces()));

    connect(d->thread, SIGNAL(signalStarted()),
                d->messageBox, SLOT(exec()));

    connect(d->thread, SIGNAL(signalAdvance()),
                this, SLOT(slotShrinkNextDBInfo()));

    connect(d->thread, SIGNAL(signalCompleted()),
                this, SLOT(slotDone()));

    d->thread->shrinkDatabases();

    //qCDebug(DIGIKAM_GENERAL_LOG) << "Completed items before vacuum: " << completedItems() << "/" << totalItems();

    d->messageBox->setText(i18n("Database shrinking in progress."));

    slotShrinkNextDBInfo();
    d->messageBox->setStandardButtons(QMessageBox::NoButton);

    d->thread->start();
}

void DbCleaner::slotAdvance()
{
    advance(1);
}

void DbCleaner::slotShrinkNextDBInfo()
{
    switch(d->databasesToShrinkCount)
    {
        case 3:
            d->coreDbStatus        = VACUUM_PENDING;
            break;
        case 2:
            d->coreDbStatus        = VACUUM_DONE;
            d->thumbsDbStatus      = VACUUM_PENDING;
            break;
        case 1:
            d->thumbsDbStatus      = VACUUM_DONE;
            d->recognitionDbStatus = VACUUM_PENDING;
            break;
        case 0:
            d->recognitionDbStatus = VACUUM_DONE;
            break;
    }

    d->databasesToShrinkCount = d->databasesToShrinkCount - 1;

    QString filledText = i18n("Currently, your database(s) are shrinked.<br/>"
            "This will take some time - depending on your database(s) size.<br/><br/>"
            "We have to freeze digiKam in order to prevent database corruption.<br/>"
            "This info box will vanish when the shrinking process is finished.<br/>"
            "Current Status: <ul>"
            "<li>%1 Core DB</li>"
            "<li>%2 Thumbs DB</li>"
            "<li>%3 Recognition DB</li>"
            "</ul>",d->coreDbStatus,d->thumbsDbStatus,d->recognitionDbStatus);

    d->messageBox->setInformativeText(filledText);

}

void DbCleaner::setUseMultiCoreCPU(bool b)
{
    d->thread->setUseMultiCore(b);
}

void DbCleaner::slotCancel()
{
    d->thread->cancel();
    MaintenanceTool::slotCancel();
}

void DbCleaner::slotDone()
{
    d->messageBox->hide();
    MaintenanceTool::slotDone();
}

} // namespace Digikam
