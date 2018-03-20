/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-01-29
 * Description : Database cleaner.
 *
 * Copyright (C) 2017-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
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
#include <QLabel>
#include <QMessageBox>
#include <QThread>
#include <QVBoxLayout>

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
        cleanSimilarityDb(false),
        shrinkDatabases(false),
        databasesToAnalyseCount(1),
        databasesToShrinkCount(0),
        shrinkDlg(0)
    {
    }

    ~Private()
    {
        delete shrinkDlg;
    }

    MaintenanceThread*           thread;
    bool                         cleanThumbsDb;
    bool                         cleanFacesDb;
    bool                         cleanSimilarityDb;
    bool                         shrinkDatabases;

    QList<qlonglong>             imagesToRemove;
    QList<int>                   staleThumbnails;
    QList<Identity>              staleIdentities;
    QList<qlonglong>             staleImageSimilarities;

    int                          databasesToAnalyseCount;
    int                          databasesToShrinkCount;

    DbShrinkDialog*              shrinkDlg;
};

DbCleaner::DbCleaner(bool cleanThumbsDb,
                     bool cleanFacesDb,
                     bool cleanSimilarityDb,
                     bool shrinkDatabases,
                     ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("DbCleaner"), parent),
      d(new Private)
{
    // register the identity list as meta type to be able to use it in signal/slot connection
    qRegisterMetaType<QList<Identity>>("QList<Identity>");

    d->cleanThumbsDb     = cleanThumbsDb;

    if (cleanThumbsDb)
    {
        d->databasesToAnalyseCount = d->databasesToAnalyseCount + 1;
    }

    d->cleanFacesDb      = cleanFacesDb;

    if (cleanFacesDb)
    {
        d->databasesToAnalyseCount = d->databasesToAnalyseCount + 1;
    }

    d->cleanSimilarityDb = cleanSimilarityDb;

    if (cleanSimilarityDb)
    {
        d->databasesToAnalyseCount = d->databasesToAnalyseCount + 1;
    }

    d->shrinkDatabases = shrinkDatabases;

    if (shrinkDatabases)
    {
        d->databasesToShrinkCount = 4;
        d->shrinkDlg = new DbShrinkDialog(DigikamApp::instance());
    }

    d->thread          = new MaintenanceThread(this);

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

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotCleanItems()));

    connect(d->thread,SIGNAL(signalAddItemsToProcess(int)),
            this, SLOT(slotAddItemsToProcess(int)));

    // Set the wiring from the data signal to the data slot.
    connect(d->thread,SIGNAL(signalData(QList<qlonglong>,QList<int>,QList<Identity>,QList<qlonglong>)),
            this, SLOT(slotFetchedData(QList<qlonglong>,QList<int>,QList<Identity>,QList<qlonglong>)));

    // Compute the database junk. This will lead to the call of the slot
    // slotFetchedData.
    d->thread->computeDatabaseJunk(d->cleanThumbsDb,d->cleanFacesDb,d->cleanSimilarityDb);
    d->thread->start();
}

void DbCleaner::slotAddItemsToProcess(int count)
{
    setTotalItems(totalItems() + count);
}

void DbCleaner::slotFetchedData(const QList<qlonglong>& staleImageIds,
                                const QList<int>& staleThumbIds,
                                const QList<Identity>& staleIdentities,
                                const QList<qlonglong>& staleImageSimilarities)
{
    // We have data now. Store it and trigger the core db cleaning
    d->imagesToRemove         = staleImageIds;
    d->staleThumbnails        = staleThumbIds;
    d->staleIdentities        = staleIdentities;
    d->staleImageSimilarities = staleImageSimilarities;

    // If we have nothing to do, finish.
    // Signal done if no elements cleanup is necessary

    if (d->imagesToRemove.isEmpty() && d->staleThumbnails.isEmpty() && d->staleIdentities.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Nothing to do. Databases are clean.";

        if (d->shrinkDatabases)
        {
            disconnect(d->thread, SIGNAL(signalData(QList<qlonglong>, QList<int>, QList<Identity>)),
                       this, SLOT(slotFetchedData(QList<qlonglong>, QList<int>, QList<Identity>)));

            disconnect(d->thread, SIGNAL(signalCompleted()),
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
        if (!d->staleThumbnails.empty())
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
    // We cleaned the recognition db. Now clean the similarity db
    disconnect(d->thread, SIGNAL(signalCompleted()),
               this, SLOT(slotCleanedFaces()));

    if (d->cleanSimilarityDb)
    {
        // TODO: implement similarity db cleanup
        if (!d->staleImageSimilarities.empty())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Found " << d->staleImageSimilarities.size() << " image ids that are referenced in similarity db but not used.";
            setLabel(i18n("Clean up the databases : ") + i18n("cleaning similarity db"));

            // GO! and don't forget the signal!
            connect(d->thread, SIGNAL(signalCompleted()),
                    this, SLOT(slotCleanedSimilarity()));

            // We cleaned the thumbs db. Now clean the faces db.
            d->thread->cleanSimilarityDb(d->staleImageSimilarities);
            d->thread->start();
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Similarity DB is clean.";
            slotCleanedSimilarity();
        }
    }
    else
    {
        slotCleanedSimilarity();
    }

    slotDone();
}

void DbCleaner::slotCleanedSimilarity()
{
    // We cleaned the similarity db. We are done.
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
            d->shrinkDlg, SLOT(exec()));

    connect(d->thread, SIGNAL(signalFinished(bool,bool)),
            this, SLOT(slotShrinkNextDBInfo(bool, bool)));

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotDone()));

    d->thread->shrinkDatabases();

    //qCDebug(DIGIKAM_GENERAL_LOG) << "Completed items before vacuum: " << completedItems() << "/" << totalItems();

//    slotShrinkNextDBInfo(true,true);
//    qCDebug(DIGIKAM_GENERAL_LOG) << "Is timer active before start():"
//                                 << d->progressTimer->isActive();

    d->thread->start();
//    qCDebug(DIGIKAM_GENERAL_LOG) << "Is timer active after start():"
//                                 << d->progressTimer->isActive();
//    d->progressTimer->start(300);
}

void DbCleaner::slotAdvance()
{
    advance(1);
}

void DbCleaner::slotShrinkNextDBInfo(bool done, bool passed)
{
    --d->databasesToShrinkCount;

    QIcon statusIcon = QIcon::fromTheme(QLatin1String("dialog-cancel"));

    if (done)
    {
        if (passed)
        {
            statusIcon = QIcon::fromTheme(QLatin1String("dialog-ok-apply"));
        }
        else
        {
            statusIcon = QIcon::fromTheme(QLatin1String("script-error"));
        }
    }

    switch(d->databasesToShrinkCount)
    {
        case 3:
            d->shrinkDlg->setIcon(0, statusIcon);
            d->shrinkDlg->setActive(1);
            break;

        case 2:
            d->shrinkDlg->setIcon(1, statusIcon);
            d->shrinkDlg->setActive(2);
            break;

        case 1:
            d->shrinkDlg->setIcon(2, statusIcon);
            d->shrinkDlg->setActive(3);
            break;

        case 0:
            d->shrinkDlg->setIcon(3, statusIcon);
            d->shrinkDlg->setActive(-1);
            break;
    }
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
    if (d->shrinkDlg)
    {
        d->shrinkDlg->hide();
    }

    MaintenanceTool::slotDone();
}

//----------------------------------------------------------------------------

DbShrinkDialog::DbShrinkDialog(QWidget* const parent)
    : QDialog(parent),
      active(-1),
      progressPix(DWorkingPixmap()),
      progressTimer(new QTimer(parent)),
      progressIndex(1),
      statusList(new QListWidget(this))
{
    QVBoxLayout* statusLayout = new QVBoxLayout(this);

    QLabel* infos = new QLabel(i18n("<p>Database shrinking in progress.</p>"
                                    "<p>Currently, your databases are being shrunk. "
                                    "This will take some time - depending on "
                                    "your databases size.</p>"
                                    "<p>We have to freeze digiKam in order to "
                                    "prevent database corruption. This info box "
                                    "will vanish when the shrinking process is "
                                    "finished.</p>"
                                    "Current Status:"),
                               this);
    infos->setWordWrap(true);
    statusLayout->addWidget(infos);

    statusList->addItem(i18n("Core DB"));
    statusList->addItem(i18n("Thumbnails DB"));
    statusList->addItem(i18n("Face Recognition DB"));
    statusList->addItem(i18n("Similarity DB"));

    for (int i = 0 ; i < 4 ; ++i)
    {
        statusList->item(i)->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
    }
//    statusList->setMinimumSize(0, 0);
//    statusList->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
//    statusList->adjustSize();
    statusList->setMaximumHeight(4 * statusList->sizeHintForRow(0)
                                 + 2 * statusList->frameWidth());
    statusLayout->addWidget(statusList);

    connect(progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

DbShrinkDialog::~DbShrinkDialog()
{
    progressTimer->stop();
}

void DbShrinkDialog::setActive(const int pos)
{
    active = pos;

    if (progressTimer->isActive())
    {
        if (active < 0)
        {
            progressTimer->stop();
        }
    }
    else
    {
        if (active >= 0)
        {
            statusList->item(active)->setIcon(progressPix.frameAt(0));
            progressTimer->start(300);
            progressIndex = 1;
        }
    }
}

void DbShrinkDialog::setIcon(const int pos, const QIcon& icon)
{
    if (active == pos)
    {
        active = -1;
    }

    statusList->item(pos)->setIcon(icon);
}

int DbShrinkDialog::exec()
{
    active = 0;
    progressTimer->start(300);
    return QDialog::exec();
}

void DbShrinkDialog::slotProgressTimerDone()
{
    if (active < 0)
    {
        return;
    }

    if (progressIndex == progressPix.frameCount())
    {
        progressIndex = 0;
    }

    statusList->item(active)->setIcon(progressPix.frameAt(progressIndex));
    ++progressIndex;
}

} // namespace Digikam
