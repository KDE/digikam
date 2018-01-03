/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Find Duplicates View.
 *
 * Copyright (C) 2016-2017 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "findduplicatesview.h"

// Qt includes

#include <QHeaderView>
#include <QLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "coredb.h"
#include "albummanager.h"
#include "albumselectors.h"
#include "coredbaccess.h"
#include "coredbbackend.h"
#include "findduplicatesalbum.h"
#include "findduplicatesalbumitem.h"
#include "duplicatesfinder.h"
#include "fingerprintsgenerator.h"
#include "applicationsettings.h"
#include "haariface.h"
#include "squeezedcombobox.h"
#include "drangebox.h"

namespace Digikam
{

class FindDuplicatesView::Private
{

public:

    Private()
    {
        includeAlbumsLabel      = 0;
        listView                = 0;
        scanDuplicatesBtn       = 0;
        updateFingerPrtBtn      = 0;
        progressItem            = 0;
        similarityLabel         = 0;
        similarityRange         = 0;
        restrictResultsLabel    = 0;
        albumTagRelationLabel   = 0;
        searchResultRestriction = 0;
        albumTagRelation        = 0;
        albumSelectors          = 0;
        settings                = 0;
        active                  = false;
    }

    QLabel*              includeAlbumsLabel;
    QLabel*              similarityLabel;
    QLabel*              restrictResultsLabel;
    QLabel*              albumTagRelationLabel;

    DIntRangeBox*        similarityRange;

    SqueezedComboBox*    searchResultRestriction;
    SqueezedComboBox*    albumTagRelation;

    QPushButton*         scanDuplicatesBtn;
    QPushButton*         updateFingerPrtBtn;

    FindDuplicatesAlbum* listView;

    ProgressItem*        progressItem;

    AlbumSelectors*      albumSelectors;

    ApplicationSettings* settings;

    bool                 active;
};

FindDuplicatesView::FindDuplicatesView(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    d->settings       = ApplicationSettings::instance();

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    // ---------------------------------------------------------------

    d->listView           = new FindDuplicatesAlbum();
    d->listView->setSortingEnabled(false);

    d->updateFingerPrtBtn = new QPushButton(i18n("Update fingerprints"));
    d->updateFingerPrtBtn->setIcon(QIcon::fromTheme(QLatin1String("run-build")));
    d->updateFingerPrtBtn->setWhatsThis(i18n("Use this button to update all image fingerprints."));

    d->scanDuplicatesBtn  = new QPushButton(i18n("Find duplicates"));
    d->scanDuplicatesBtn->setIcon(QIcon::fromTheme(QLatin1String("edit-find")));
    d->scanDuplicatesBtn->setWhatsThis(i18n("Use this button to scan the selected albums for "
                                            "duplicate items."));

    // ---------------------------------------------------------------

    d->albumSelectors  = new AlbumSelectors(i18nc("@label", "Search in:"), QLatin1String("Find Duplicates View"));

    // ---------------------------------------------------------------

    d->similarityRange = new DIntRangeBox();
    d->similarityRange->setSuffix(QLatin1String("%"));

    if (d->settings)
    {
        d->similarityRange->setRange(d->settings->getMinimumSimilarityBound(), 100);
        d->similarityRange->setInterval(d->settings->getDuplicatesSearchLastMinSimilarity(),
                                        d->settings->getDuplicatesSearchLastMaxSimilarity());
    }
    else
    {
        d->similarityRange->setRange(40, 100);
        d->similarityRange->setInterval(40, 100);
    }

    d->similarityLabel         = new QLabel(i18n("Similarity range:"));
    d->similarityLabel->setBuddy(d->similarityRange);

    d->restrictResultsLabel    = new QLabel(i18n("Restriction:"));
    d->restrictResultsLabel->setBuddy(d->searchResultRestriction);

    d->searchResultRestriction = new SqueezedComboBox();
    d->searchResultRestriction->addSqueezedItem(i18nc("@label:listbox", "None"),                                 HaarIface::DuplicatesSearchRestrictions::None);
    d->searchResultRestriction->addSqueezedItem(i18nc("@label:listbox", "Restrict to reference album"), HaarIface::DuplicatesSearchRestrictions::SameAlbum);
    d->searchResultRestriction->addSqueezedItem(i18nc("@label:listbox", "Exclude reference album"),     HaarIface::DuplicatesSearchRestrictions::DifferentAlbum);
    d->searchResultRestriction->setToolTip(i18n("Use this option to restrict the duplicate search "
                                                "with some criteria, as to limit search to the album "
                                                "of reference image, or to exclude the album of "
                                                "reference image of the search."));

    // Load the last choice from application settings.
    HaarIface::DuplicatesSearchRestrictions restrictions = (HaarIface::DuplicatesSearchRestrictions) d->settings->getDuplicatesSearchRestrictions();
    d->searchResultRestriction->setCurrentIndex(d->searchResultRestriction->findData(restrictions));

    d->albumTagRelationLabel    = new QLabel(i18n("Restrict to:"));
    d->albumTagRelationLabel->setBuddy(d->albumTagRelation);

    d->albumTagRelation = new SqueezedComboBox();
    d->albumTagRelation->addSqueezedItem(i18nc("@label:listbox", "Only selected tab") ,  HaarIface::AlbumTagRelation::NoMix);
    d->albumTagRelation->addSqueezedItem(i18nc("@label:listbox", "One of"),              HaarIface::AlbumTagRelation::Union);
    d->albumTagRelation->addSqueezedItem(i18nc("@label:listbox", "Both"),                HaarIface::AlbumTagRelation::Intersection);
    d->albumTagRelation->addSqueezedItem(i18nc("@label:listbox", "Albums but not tags"), HaarIface::AlbumTagRelation::AlbumExclusive);
    d->albumTagRelation->addSqueezedItem(i18nc("@label:listbox", "Tags but not albums"), HaarIface::AlbumTagRelation::TagExclusive);
    d->albumTagRelation->setCurrentIndex(ApplicationSettings::instance()->getDuplicatesAlbumTagRelation());

    d->albumTagRelation->setToolTip(i18n("Use this option to decide about the relation of the selected albums and tags.<br/>"
                                         "<i>One of</i> means that the images are either in the selected albums or tags.<br/>"
                                         "<i>Both</i> means that the images are both in the selected albums and tags.<br/>"
                                         "<i>Albums but not tags</i> means that images must be in the selected albums but not tags.<br/>"
                                         "<i>Tags but not albums</i> means that images must be in the selected tags but not albums.<br/>"
                                         "<i>Only selected tab</i> means that only the selected tab is used."));

    // Load the last choice from application settings.
    HaarIface::AlbumTagRelation relation = (HaarIface::AlbumTagRelation) d->settings->getDuplicatesAlbumTagRelation();
    d->albumTagRelation->setCurrentIndex(d->albumTagRelation->findData(relation));

    // ---------------------------------------------------------------

    QGridLayout* const mainLayout = new QGridLayout();
    mainLayout->addWidget(d->listView,                0, 0, 1, -1);
    mainLayout->addWidget(d->albumTagRelationLabel,   1, 0, 1,  2);
    mainLayout->addWidget(d->albumTagRelation,        1, 2, 1, -1);
    mainLayout->addWidget(d->albumSelectors,          2, 0, 1, -1);
    mainLayout->addWidget(d->similarityLabel,         3, 0, 1,  1);
    mainLayout->addWidget(d->similarityRange,         3, 2, 1,  1);
    mainLayout->addWidget(d->restrictResultsLabel,    4, 0, 1,  2);
    mainLayout->addWidget(d->searchResultRestriction, 4, 2, 1, -1);
    mainLayout->addWidget(d->updateFingerPrtBtn,      5, 0, 1, -1);
    mainLayout->addWidget(d->scanDuplicatesBtn,       6, 0, 1, -1);
    mainLayout->setRowStretch(0, 10);
    mainLayout->setColumnStretch(2, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    setLayout(mainLayout);

    // ---------------------------------------------------------------

    connect(d->updateFingerPrtBtn, SIGNAL(clicked()),
            this, SLOT(slotUpdateFingerPrints()));

    connect(d->scanDuplicatesBtn, SIGNAL(clicked()),
            this, SLOT(slotFindDuplicates()));

    connect(d->listView, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotDuplicatesAlbumActived()));

    connect(d->albumSelectors, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotCheckForValidSettings()));

    connect(AlbumManager::instance(), SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(populateTreeView()));

    connect(AlbumManager::instance(), SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(initAlbumUpdateConnections()));

    connect(d->settings, SIGNAL(setupChanged()),
            this, SLOT(slotApplicationSettingsChanged()));
}

FindDuplicatesView::~FindDuplicatesView()
{
    d->albumSelectors->saveState();
    delete d;
}

void FindDuplicatesView::initAlbumUpdateConnections()
{
    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalSearchUpdated(SAlbum*)),
            this, SLOT(slotSearchUpdated(SAlbum*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotClear()));

    connect(AlbumManager::instance(),SIGNAL(signalUpdateDuplicatesAlbums(QList<SAlbum*>, QList<qlonglong>)),
            this,SLOT(slotUpdateDuplicates(QList<SAlbum*>,QList<qlonglong>)));

}

void FindDuplicatesView::setActive(bool val)
{
    d->active = val;
}

void FindDuplicatesView::populateTreeView()
{
    const AlbumList& aList = AlbumManager::instance()->allSAlbums();

    for (AlbumList::const_iterator it = aList.constBegin() ; it != aList.constEnd() ; ++it)
    {
        SAlbum* const salbum = dynamic_cast<SAlbum*>(*it);

        if (salbum && salbum->isDuplicatesSearch() && !salbum->extraData(this))
        {
            FindDuplicatesAlbumItem* const item = new FindDuplicatesAlbumItem(d->listView, salbum);
            salbum->setExtraData(this, item);
        }
    }

    d->listView->setSortingEnabled(true);
    d->listView->sortByColumn(1, Qt::DescendingOrder);
    d->listView->resizeColumnToContents(0);

    d->albumSelectors->loadState();

    d->listView->selectFirstItem();
}

QList<SAlbum*> FindDuplicatesView::currentFindDuplicatesAlbums() const
{
    QList<QTreeWidgetItem*> selectedItems = d->listView->selectedItems();

    if (selectedItems.isEmpty())
    {
        QTreeWidgetItem* const item = d->listView->firstItem();

        if (item)
        {
            selectedItems << item;
        }
    }

    QList<SAlbum*> albumList;

    foreach(QTreeWidgetItem* const item, selectedItems)
    {
        FindDuplicatesAlbumItem* const albumItem = dynamic_cast<FindDuplicatesAlbumItem*>(item);

        if (albumItem)
        {
            albumList << albumItem->album();
        }
    }

    return albumList;
}

void FindDuplicatesView::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
    {
        return;
    }

    SAlbum* const salbum = static_cast<SAlbum*>(a);

    if (!salbum->isDuplicatesSearch())
    {
        return;
    }

    if (!d->active)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Duplicates view is not active, returning";
        return;
    }

    if (!salbum->extraData(this))
    {
        FindDuplicatesAlbumItem* const item = new FindDuplicatesAlbumItem(d->listView, salbum);
        salbum->setExtraData(this, item);
    }

    d->similarityRange->setInterval(d->settings->getDuplicatesSearchLastMinSimilarity(),
                                    d->settings->getDuplicatesSearchLastMaxSimilarity());
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

    d->similarityRange->setInterval(d->settings->getDuplicatesSearchLastMinSimilarity(),
                                    d->settings->getDuplicatesSearchLastMaxSimilarity());
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
    for (QTreeWidgetItemIterator it(d->listView) ; *it ; ++it)
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
    d->searchResultRestriction->setEnabled(val);
    d->updateFingerPrtBtn->setEnabled(val);
    d->scanDuplicatesBtn->setEnabled(val);
    d->albumTagRelation->setEnabled(val);
    d->similarityRange->setEnabled(val);
    d->albumSelectors->setEnabled(val);

    d->albumTagRelationLabel->setEnabled(val);
    d->restrictResultsLabel->setEnabled(val);
    d->similarityLabel->setEnabled(val);
}

void FindDuplicatesView::slotFindDuplicates()
{
    d->albumSelectors->saveState();
    slotClear();
    enableControlWidgets(false);

    AlbumList albums;
    AlbumList tags;

    if (d->albumTagRelation->itemData(d->albumTagRelation->currentIndex()).toInt() == HaarIface::AlbumTagRelation::NoMix)
    {
        if (d->albumSelectors->typeSelection() == AlbumSelectors::AlbumType::PhysAlbum)
        {
            albums = d->albumSelectors->selectedAlbums();
        }
        else if (d->albumSelectors->typeSelection() == AlbumSelectors::AlbumType::TagsAlbum)
        {
            tags  = d->albumSelectors->selectedTags();
        }
    }
    else
    {
        albums = d->albumSelectors->selectedAlbums();
        tags   = d->albumSelectors->selectedTags();
    }

    DuplicatesFinder* const finder = new DuplicatesFinder(albums, tags,
                                                          d->albumTagRelation->itemData(d->albumTagRelation->currentIndex()).toInt(),
                                                          d->similarityRange->minValue(), d->similarityRange->maxValue(),
                                                          d->searchResultRestriction->itemData(d->searchResultRestriction->currentIndex()).toInt());

    connect(finder, SIGNAL(signalComplete()),
            this, SLOT(slotComplete()));

    finder->start();
}

void FindDuplicatesView::slotUpdateDuplicates(const QList<SAlbum*>& sAlbumsToRebuild,
                                              const QList<qlonglong>& deletedImages)
{
    d->listView->updateDuplicatesAlbumItems(sAlbumsToRebuild, deletedImages);
}

void FindDuplicatesView::slotApplicationSettingsChanged()
{
    d->similarityRange->setRange(d->settings->getMinimumSimilarityBound(),100);
}

void FindDuplicatesView::slotComplete()
{
    enableControlWidgets(true);
    slotCheckForValidSettings();
    populateTreeView();
}

void FindDuplicatesView::slotDuplicatesAlbumActived()
{
    QList<Album*> albums;

    foreach(QTreeWidgetItem* const item, d->listView->selectedItems())
    {
        FindDuplicatesAlbumItem* const albumItem = dynamic_cast<FindDuplicatesAlbumItem*>(item);

        if (albumItem)
        {
            albums << albumItem->album();
        }
    }

    if (!albums.empty())
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << albums);
    }
}

void FindDuplicatesView::slotCheckForValidSettings()
{
    bool valid = d->albumSelectors->selectedAlbums().count() || d->albumSelectors->selectedTags().count();
    d->scanDuplicatesBtn->setEnabled(valid);
}

void FindDuplicatesView::slotUpdateFingerPrints()
{
    FingerPrintsGenerator* const tool = new FingerPrintsGenerator(false);
    tool->start();
}

void FindDuplicatesView::slotSetSelectedAlbum(PAlbum* album)
{
    if (!album)
    {
        return;
    }

    resetAlbumsAndTags();

    // @ODD : Why is singleton set to true? resetAlbumsAndTags already clears the selection.
    d->albumSelectors->setAlbumSelected(album, true);
    d->albumSelectors->setTypeSelection(AlbumSelectors::AlbumType::PhysAlbum);
    slotCheckForValidSettings();
}

void FindDuplicatesView::slotSetSelectedAlbums(QList<PAlbum*> albums)
{
    // @ODD : Why is singleton set to true? resetAlbumsAndTags already clears the selection.
    resetAlbumsAndTags();

    foreach(PAlbum* const album, albums)
    {
        d->albumSelectors->setAlbumSelected(album, false);
    }

    d->albumSelectors->setTypeSelection(AlbumSelectors::AlbumType::PhysAlbum);
    slotCheckForValidSettings();
}

void FindDuplicatesView::slotSetSelectedAlbums(QList<TAlbum*> albums)
{
    resetAlbumsAndTags();

    foreach(TAlbum* const album, albums)
    {
        d->albumSelectors->setTagSelected(album, false);
    }

    d->albumSelectors->setTypeSelection(AlbumSelectors::AlbumType::TagsAlbum);
    slotCheckForValidSettings();
}

void FindDuplicatesView::resetAlbumsAndTags()
{
    d->albumSelectors->resetSelection();
    slotCheckForValidSettings();
}

}  // namespace Digikam
