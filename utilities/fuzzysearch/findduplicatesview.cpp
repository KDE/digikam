/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Find Duplicates View.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#define ICONSIZE 64

#include "findduplicatesview.h"
#include "findduplicatesview.moc"

// Qt includes.

#include <QHeaderView>
#include <QLayout>
#include <QPainter>
#include <QPushButton>
#include <QTreeWidget>

// KDE includes.

#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>

// Local includes.

#include "album.h"
#include "albummanager.h"
#include "albumdb.h"
#include "databaseaccess.h"
#include "imagelister.h"
#include "statusprogressbar.h"
#include "findduplicatesalbumitem.h"

namespace Digikam
{

class FindDuplicatesViewPriv
{

public:

    FindDuplicatesViewPriv()
    {
        listView                = 0;
        scanDuplicatesBtn       = 0;
        updateFingerPrtBtn      = 0;
        progressBar             = 0;
        thumbLoadThread         = 0;
        cancelFindDuplicates    = false;
    }

    bool                cancelFindDuplicates;

    QPushButton         *scanDuplicatesBtn;
    QPushButton         *updateFingerPrtBtn;

    QTreeWidget         *listView;

    StatusProgressBar   *progressBar;

    ThumbnailLoadThread *thumbLoadThread;
};

FindDuplicatesView::FindDuplicatesView(QWidget *parent)
                  : QWidget(parent), d(new FindDuplicatesViewPriv)
{
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();

    setAttribute(Qt::WA_DeleteOnClose);

    QGridLayout *grid = new QGridLayout(this);
    d->listView       = new QTreeWidget(this);
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setIconSize(QSize(ICONSIZE, ICONSIZE));
    d->listView->setSortingEnabled(true);
    d->listView->setColumnCount(2);
    d->listView->setHeaderLabels(QStringList() << i18n("Ref. images") << i18n("Items"));
    d->listView->header()->setResizeMode(QHeaderView::Stretch);
    d->listView->setWhatsThis(i18n("This shows all duplicate items found in whole collection."));

    d->updateFingerPrtBtn = new QPushButton(i18n("Update fingerprints"), this);
    d->updateFingerPrtBtn->setIcon(KIcon("run-build"));
    d->updateFingerPrtBtn->setWhatsThis(i18n("Use this button to scan the whole collection and find all "
                                              "duplicate items."));

    d->scanDuplicatesBtn = new QPushButton(i18n("Find duplicates"), this);
    d->scanDuplicatesBtn->setIcon(KIcon("system-search"));
    d->scanDuplicatesBtn->setWhatsThis(i18n("Use this button to scan whole collection to find all "
                                            "duplicate items."));

    d->progressBar = new StatusProgressBar(this);
    d->progressBar->progressBarMode(StatusProgressBar::TextMode);
    d->progressBar->setEnabled(false);

    grid->addWidget(d->listView,           0, 0, 1, 3);
    grid->addWidget(d->updateFingerPrtBtn, 1, 0, 1, 3);
    grid->addWidget(d->scanDuplicatesBtn,  2, 0, 1, 3);
    grid->addWidget(d->progressBar,        3, 0, 1, 3);
    grid->setRowStretch(0, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    connect(d->updateFingerPrtBtn, SIGNAL(clicked()),
            this, SIGNAL(signalUpdateFingerPrints()));

    connect(d->scanDuplicatesBtn, SIGNAL(clicked()),
            this, SLOT(slotFindDuplicates()));

    connect(d->listView, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(slotDuplicatesAlbumActived(QTreeWidgetItem*, int)));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription&, const QPixmap&)));

    connect(AlbumManager::instance(), SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(populateTreeView()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotClear()));

    connect(d->progressBar, SIGNAL(signalCancelButtonPressed()),
            this, SLOT(slotCancelButtonPressed()));
}

FindDuplicatesView::~FindDuplicatesView()
{
    delete d;
}

SAlbum* FindDuplicatesView::currentFindDuplicatesAlbum() const
{
    SAlbum *salbum = 0;

    FindDuplicatesAlbumItem* item = dynamic_cast<FindDuplicatesAlbumItem*>(d->listView->currentItem());
    if (item)
        salbum = item->album();

    return salbum;
}

void FindDuplicatesView::populateTreeView()
{
    const AlbumList& aList = AlbumManager::instance()->allSAlbums();

    for (AlbumList::const_iterator it = aList.begin(); it != aList.end(); ++it)
    {
        SAlbum* salbum = dynamic_cast<SAlbum*>(*it);
        if (salbum && salbum->isDuplicatesSearch() && !salbum->extraData(this))
        {
            FindDuplicatesAlbumItem *item = new FindDuplicatesAlbumItem(d->listView, salbum);
            salbum->setExtraData(this, item);
            ThumbnailLoadThread::defaultThread()->find(item->refUrl().path());
        }
    }

    d->listView->sortByColumn(1, Qt::DescendingOrder);
    d->listView->resizeColumnToContents(0);
}

void FindDuplicatesView::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum *salbum  = (SAlbum*)a;

    if (!salbum->isDuplicatesSearch())
        return;

    if (!salbum->extraData(this))
    {
        FindDuplicatesAlbumItem *item = new FindDuplicatesAlbumItem(d->listView, salbum);
        salbum->setExtraData(this, item);
        ThumbnailLoadThread::defaultThread()->find(item->refUrl().path());
    }
}

void FindDuplicatesView::slotAlbumDeleted(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum* album = (SAlbum*)a;

    FindDuplicatesAlbumItem* item = (FindDuplicatesAlbumItem*) album->extraData(this);
    if (item)
    {
        a->removeExtraData(item);
        delete item;
    }
}

void FindDuplicatesView::slotClear()
{
    for(QTreeWidgetItemIterator it(d->listView); *it; ++it)
    {
        SAlbum *salbum = static_cast<FindDuplicatesAlbumItem*>(*it)->album();
        if (salbum)
            salbum->removeExtraData(this);
    }
    d->listView->clear();
}

void FindDuplicatesView::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    QTreeWidgetItemIterator it(d->listView);
    while (*it)
    {
        FindDuplicatesAlbumItem* item = dynamic_cast<FindDuplicatesAlbumItem*>(*it);
        if (item->refUrl().path() == desc.filePath)
        {
            if (pix.isNull())
                item->setThumb(SmallIcon("image-x-generic", ICONSIZE, KIconLoader::DisabledState));
            else
                item->setThumb(pix.scaled(ICONSIZE, ICONSIZE, Qt::KeepAspectRatio));

            return;
        }
        ++it;
    }
}

void FindDuplicatesView::enableControlWidgets(bool val)
{

    if (val)
    {
        d->scanDuplicatesBtn->setEnabled(true);
        d->updateFingerPrtBtn->setEnabled(true);
        d->progressBar->progressBarMode(StatusProgressBar::TextMode);
        d->progressBar->setProgressValue(0);
        d->progressBar->setEnabled(false);
    }
    else
    {
        d->scanDuplicatesBtn->setEnabled(false);
        d->updateFingerPrtBtn->setEnabled(false);
        d->progressBar->progressBarMode(StatusProgressBar::CancelProgressBarMode);
        d->progressBar->setProgressValue(0);
        d->progressBar->setEnabled(true);
    }
}

void FindDuplicatesView::slotFindDuplicates()
{
    slotClear();
    enableControlWidgets(false);

    AlbumList albums = AlbumManager::instance()->allPAlbums();
    QStringList idsStringList;
    foreach(Album *a, albums)
        idsStringList << QString::number(a->id());

    KIO::Job *job = ImageLister::startListJob(DatabaseUrl::searchUrl(-1));
    job->addMetaData("duplicates", "true");
    job->addMetaData("albumids", idsStringList.join(","));
    job->addMetaData("threshold", QString::number(0.9));

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotDuplicatesSearchResult(KJob*)));

    connect(job, SIGNAL(totalAmount(KJob *, KJob::Unit, qulonglong)),
            this, SLOT(slotDuplicatesSearchTotalAmount(KJob *, KJob::Unit, qulonglong)));

    connect(job, SIGNAL(processedAmount(KJob *, KJob::Unit, qulonglong)),
            this, SLOT(slotDuplicatesSearchProcessedAmount(KJob *, KJob::Unit, qulonglong)));
}

void FindDuplicatesView::cancelFindDuplicates(KJob* job)
{
    if (!job)
        return;

    job->kill();
    d->cancelFindDuplicates = false;

    enableControlWidgets(true);
    kapp->restoreOverrideCursor();

    populateTreeView();
}

void FindDuplicatesView::slotCancelButtonPressed()
{
    d->cancelFindDuplicates = true;
    kapp->setOverrideCursor(Qt::WaitCursor);
}

void FindDuplicatesView::slotDuplicatesSearchTotalAmount(KJob*, KJob::Unit, qulonglong amount)
{
    d->progressBar->setProgressValue(0);
    d->progressBar->setProgressTotalSteps(amount);
}

void FindDuplicatesView::slotDuplicatesSearchProcessedAmount(KJob* job, KJob::Unit, qulonglong amount)
{
    d->progressBar->setProgressValue(amount);

    if (d->cancelFindDuplicates)
        cancelFindDuplicates(job);
}

void FindDuplicatesView::slotDuplicatesSearchResult(KJob*)
{
    enableControlWidgets(true);
    populateTreeView();
}

void FindDuplicatesView::slotDuplicatesAlbumActived(QTreeWidgetItem* item, int)
{
    FindDuplicatesAlbumItem* sitem = dynamic_cast<FindDuplicatesAlbumItem*>(item);
    if (sitem)
        AlbumManager::instance()->setCurrentAlbum(sitem->album());
}

}  // namespace Digikam
