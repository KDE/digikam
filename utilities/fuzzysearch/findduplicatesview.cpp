/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Find Duplicates View.
 *
 * Copyright (C) 2008-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "findduplicatesview.moc"

// Qt includes

#include <QHeaderView>
#include <QLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>

// KDE includes

#include <kapplication.h>
#include <kdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumselectors.h"
#include "databaseaccess.h"
#include "databasebackend.h"
#include "findduplicatesalbum.h"
#include "findduplicatesalbumitem.h"
#include "duplicatesfinder.h"
#include "fingerprintsgenerator.h"

namespace Digikam
{

class FindDuplicatesView::Private
{

public:

    Private()
    {
        includeAlbumsLabel = 0;
        listView           = 0;
        scanDuplicatesBtn  = 0;
        updateFingerPrtBtn = 0;
        progressItem       = 0;
        similarityLabel    = 0;
        similarity         = 0;
        albumSelectors     = 0;
    }

    QLabel*                      includeAlbumsLabel;
    QLabel*                      similarityLabel;

    QSpinBox*                    similarity;

    QPushButton*                 scanDuplicatesBtn;
    QPushButton*                 updateFingerPrtBtn;

    FindDuplicatesAlbum*         listView;

    ProgressItem*                progressItem;

    AlbumSelectors*              albumSelectors;
};

FindDuplicatesView::FindDuplicatesView(QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    // ---------------------------------------------------------------

    d->listView           = new FindDuplicatesAlbum();

    d->updateFingerPrtBtn = new QPushButton(i18n("Update fingerprints"));
    d->updateFingerPrtBtn->setIcon(KIcon("run-build"));
    d->updateFingerPrtBtn->setWhatsThis(i18n("Use this button to update all image fingerprints."));

    d->scanDuplicatesBtn  = new QPushButton(i18n("Find duplicates"));
    d->scanDuplicatesBtn->setIcon(KIcon("system-search"));
    d->scanDuplicatesBtn->setWhatsThis(i18n("Use this button to scan the selected albums for "
                                            "duplicate items."));

    // ---------------------------------------------------------------

    d->albumSelectors = new AlbumSelectors(i18nc("@label", "Search in:"), "Find Duplicates View");

    // ---------------------------------------------------------------

    d->similarity = new QSpinBox();
    d->similarity->setRange(0, 100);
    d->similarity->setValue(90);
    d->similarity->setSingleStep(1);
    d->similarity->setSuffix(QChar('%'));

    d->similarityLabel = new QLabel(i18n("Similarity:"));
    d->similarityLabel->setBuddy(d->similarity);

    // ---------------------------------------------------------------

    QGridLayout* const mainLayout = new QGridLayout();
    mainLayout->addWidget(d->listView,           0, 0, 1, -1);
    mainLayout->addWidget(d->albumSelectors,     1, 0, 1, -1);
    mainLayout->addWidget(d->similarityLabel,    2, 0, 1, 1);
    mainLayout->addWidget(d->similarity,         2, 2, 1, 1);
    mainLayout->addWidget(d->updateFingerPrtBtn, 3, 0, 1, -1);
    mainLayout->addWidget(d->scanDuplicatesBtn,  4, 0, 1, -1);
    mainLayout->setRowStretch(0, 10);
    mainLayout->setColumnStretch(1, 10);
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setSpacing(KDialog::spacingHint());
    setLayout(mainLayout);

    // ---------------------------------------------------------------

    connect(d->updateFingerPrtBtn, SIGNAL(clicked()),
            this, SLOT(slotUpdateFingerPrints()));

    connect(d->scanDuplicatesBtn, SIGNAL(clicked()),
            this, SLOT(slotFindDuplicates()));

    connect(d->listView, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotDuplicatesAlbumActived(QTreeWidgetItem*,int)));

    connect(d->albumSelectors, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotCheckForValidSettings()));

    connect(AlbumManager::instance(), SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(populateTreeView()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalSearchUpdated(SAlbum*)),
            this, SLOT(slotSearchUpdated(SAlbum*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotClear()));
}

FindDuplicatesView::~FindDuplicatesView()
{
    d->albumSelectors->saveState();
    delete d;
}

void FindDuplicatesView::populateTreeView()
{
    const AlbumList& aList = AlbumManager::instance()->allSAlbums();

    for (AlbumList::const_iterator it = aList.constBegin(); it != aList.constEnd(); ++it)
    {
        SAlbum* const salbum = dynamic_cast<SAlbum*>(*it);

        if (salbum && salbum->isDuplicatesSearch() && !salbum->extraData(this))
        {
            FindDuplicatesAlbumItem* const item = new FindDuplicatesAlbumItem(d->listView, salbum);
            salbum->setExtraData(this, item);
        }
    }

    d->listView->sortByColumn(1, Qt::DescendingOrder);
    d->listView->resizeColumnToContents(0);

    d->albumSelectors->loadState();
}

SAlbum* FindDuplicatesView::currentFindDuplicatesAlbum() const
{
    SAlbum* salbum = 0;

    FindDuplicatesAlbumItem* const item = dynamic_cast<FindDuplicatesAlbumItem*>(d->listView->currentItem());

    if (item)
    {
        salbum = item->album();
    }

    return salbum;
}

void FindDuplicatesView::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
    {
        return;
    }

    SAlbum* const salbum  = static_cast<SAlbum*>(a);

    if (!salbum->isDuplicatesSearch())
    {
        return;
    }

    if (!salbum->extraData(this))
    {
        FindDuplicatesAlbumItem* const item = new FindDuplicatesAlbumItem(d->listView, salbum);
        salbum->setExtraData(this, item);
    }
}

void FindDuplicatesView::slotAlbumDeleted(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
    {
        return;
    }

    SAlbum* const album = static_cast<SAlbum*>(a);

    FindDuplicatesAlbumItem* const item = static_cast<FindDuplicatesAlbumItem*>(album->extraData(this));

    if (item)
    {
        a->removeExtraData(this);
        delete item;
    }
}

void FindDuplicatesView::slotSearchUpdated(SAlbum* a)
{
    if (!a->isDuplicatesSearch())
    {
        return;
    }

    slotAlbumDeleted(a);
    slotAlbumAdded(a);
}

void FindDuplicatesView::slotClear()
{
    for (QTreeWidgetItemIterator it(d->listView); *it; ++it)
    {
        SAlbum* const salbum = static_cast<FindDuplicatesAlbumItem*>(*it)->album();

        if (salbum)
        {
            salbum->removeExtraData(this);
        }
    }

    d->listView->clear();
}

void FindDuplicatesView::enableControlWidgets(bool val)
{
    d->scanDuplicatesBtn->setEnabled(val);
    slotCheckForValidSettings();

    d->updateFingerPrtBtn->setEnabled(val);
    d->albumSelectors->setEnabled(val);
    d->similarityLabel->setEnabled(val);
    d->similarity->setEnabled(val);
}

void FindDuplicatesView::slotFindDuplicates()
{
    d->albumSelectors->saveState();
    slotClear();
    enableControlWidgets(false);

    DuplicatesFinder* const finder = new DuplicatesFinder(d->albumSelectors->selectedPAlbums(), d->albumSelectors->selectedTAlbums(), d->similarity->value());

    connect(finder, SIGNAL(signalComplete()),
            this, SLOT(slotComplete()));

    finder->start();
}

void FindDuplicatesView::slotComplete()
{
    enableControlWidgets(true);
    populateTreeView();
}

void FindDuplicatesView::slotDuplicatesAlbumActived(QTreeWidgetItem* item, int)
{
    FindDuplicatesAlbumItem* const sitem = dynamic_cast<FindDuplicatesAlbumItem*>(item);

    if (sitem)
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << sitem->album());
    }
}

void FindDuplicatesView::slotCheckForValidSettings()
{
    bool valid = d->albumSelectors->selectedPAlbums().count() || d->albumSelectors->selectedTAlbums().count();
    d->scanDuplicatesBtn->setEnabled(valid);
}

void FindDuplicatesView::slotUpdateFingerPrints()
{
    FingerPrintsGenerator* const tool = new FingerPrintsGenerator(false);
    tool->start();
}

void FindDuplicatesView::slotSetSelectedAlbum(Album* album)
{
    if (!album)
    {
        return;
    }

    resetAlbumsAndTags();
    d->albumSelectors->setPAlbumSelected(album, true);
    slotCheckForValidSettings();
}

void FindDuplicatesView::slotSetSelectedTag(Album* album)
{
    if (!album)
    {
        return;
    }

    resetAlbumsAndTags();
    d->albumSelectors->setTAlbumSelected(album, true);
    slotCheckForValidSettings();
}

void FindDuplicatesView::resetAlbumsAndTags()
{
    d->albumSelectors->resetSelection();
    slotCheckForValidSettings();
}

}  // namespace Digikam
