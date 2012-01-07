/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Find Duplicates View.
 *
 * Copyright (C) 2008-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
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
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>

// KDE includes

#include <kapplication.h>
#include <kdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "databaseaccess.h"
#include "databasebackend.h"
#include "findduplicatesalbumitem.h"
#include "imagelister.h"
#include "statusprogressbar.h"
#include "albumselectcombobox.h"
#include "abstractalbummodel.h"

namespace Digikam
{

class FindDuplicatesAlbum::FindDuplicatesAlbumPriv
{

public:

    FindDuplicatesAlbumPriv()
        : iconSize(64)
    {
        thumbLoadThread = 0;
    }

    const int            iconSize;

    ThumbnailLoadThread* thumbLoadThread;
};

FindDuplicatesAlbum::FindDuplicatesAlbum(QWidget* parent)
    : QTreeWidget(parent), d(new FindDuplicatesAlbumPriv)
{
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();

    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setIconSize(QSize(d->iconSize, d->iconSize));
    setSortingEnabled(true);
    setColumnCount(2);
    setHeaderLabels(QStringList() << i18n("Ref. images") << i18n("Items"));
    header()->setResizeMode(0, QHeaderView::Stretch);
    header()->setResizeMode(1, QHeaderView::ResizeToContents);
    setWhatsThis(i18n("This shows all found duplicate items."));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));
}

FindDuplicatesAlbum::~FindDuplicatesAlbum()
{
    delete d;
}

void FindDuplicatesAlbum::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        FindDuplicatesAlbumItem* item = dynamic_cast<FindDuplicatesAlbumItem*>(*it);

        if (item->refUrl().toLocalFile() == desc.filePath)
        {
            if (!pix.isNull())
            {
                item->setThumb(pix.scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio));
            }
        }

        ++it;
    }
}

void FindDuplicatesAlbum::drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
    FindDuplicatesAlbumItem* item = dynamic_cast<FindDuplicatesAlbumItem*>(itemFromIndex(index));

    if (item && !item->hasValidThumbnail())
    {
        d->thumbLoadThread->find(item->refUrl().toLocalFile());
    }

    QTreeWidget::drawRow(p, opt, index);
}

// ------------------------------------------------------------------------------------------------------

class FindDuplicatesView::FindDuplicatesViewPriv
{

public:

    FindDuplicatesViewPriv()
    {
        listView           = 0;
        scanDuplicatesBtn  = 0;
        updateFingerPrtBtn = 0;
        progressBar        = 0;
        includeAlbumsLabel = 0;
        similarityLabel    = 0;
        similarity         = 0;
        albumSelectCB      = 0;
        tagSelectCB        = 0;
        albumModel         = 0;
        tagModel           = 0;
        searchJob          = NULL;
    }

    KIO::Job*                    searchJob;

    QLabel*                      includeAlbumsLabel;
    QLabel*                      similarityLabel;

    QSpinBox*                    similarity;

    QPushButton*                 scanDuplicatesBtn;
    QPushButton*                 updateFingerPrtBtn;

    FindDuplicatesAlbum*         listView;

    StatusProgressBar*           progressBar;

    AlbumSelectComboBox*         albumSelectCB;
    AlbumSelectComboBox*         tagSelectCB;

    AbstractCheckableAlbumModel* albumModel;
    AbstractCheckableAlbumModel* tagModel;
};

FindDuplicatesView::FindDuplicatesView(QWidget* parent)
    : QWidget(parent), d(new FindDuplicatesViewPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);

    // ---------------------------------------------------------------

    d->listView           = new FindDuplicatesAlbum();

    d->updateFingerPrtBtn = new QPushButton(i18n("Update fingerprints"));
    d->updateFingerPrtBtn->setIcon(KIcon("run-build"));
    d->updateFingerPrtBtn->setWhatsThis(i18n("Use this button to update all image fingerprints."));

    d->scanDuplicatesBtn = new QPushButton(i18n("Find duplicates"));
    d->scanDuplicatesBtn->setIcon(KIcon("system-search"));
    d->scanDuplicatesBtn->setWhatsThis(i18n("Use this button to scan the selected albums for "
                                            "duplicate items."));

    d->progressBar = new StatusProgressBar();
    d->progressBar->progressBarMode(StatusProgressBar::TextMode);
    d->progressBar->setEnabled(false);

    // ---------------------------------------------------------------

    d->includeAlbumsLabel = new QLabel(i18n("Search in:"));

    d->albumSelectCB = new AlbumSelectComboBox();
    d->albumSelectCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QString albumSelectStr = i18n("Select all albums that should be included in the search.");
    d->albumSelectCB->setWhatsThis(albumSelectStr);
    d->albumSelectCB->setToolTip(albumSelectStr);

    d->tagSelectCB = new AlbumSelectComboBox();
    d->tagSelectCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QString tagSelectStr = i18n("Select all tags that should be included in the search.");
    d->tagSelectCB->setWhatsThis(tagSelectStr);
    d->tagSelectCB->setToolTip(tagSelectStr);

    // ---------------------------------------------------------------

    d->similarity = new QSpinBox();
    d->similarity->setRange(0, 100);
    d->similarity->setValue(90);
    d->similarity->setSingleStep(1);
    d->similarity->setSuffix(QChar('%'));

    d->similarityLabel = new QLabel(i18n("Similarity:"));
    d->similarityLabel->setBuddy(d->similarity);

    // ---------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->addWidget(d->listView,           0, 0, 1,-1);
    mainLayout->addWidget(d->includeAlbumsLabel, 1, 0, 1, 1);
    mainLayout->addWidget(d->albumSelectCB,      1, 1, 1,-1);
    mainLayout->addWidget(d->tagSelectCB,        2, 1, 1,-1);
    mainLayout->addWidget(d->similarityLabel,    3, 0, 1, 1);
    mainLayout->addWidget(d->similarity,         3, 2, 1, 1);
    mainLayout->addWidget(d->updateFingerPrtBtn, 4, 0, 1,-1);
    mainLayout->addWidget(d->scanDuplicatesBtn,  5, 0, 1,-1);
    mainLayout->addWidget(d->progressBar,        6, 0, 1,-1);
    mainLayout->setRowStretch(0, 10);
    mainLayout->setColumnStretch(1, 10);
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setSpacing(KDialog::spacingHint());
    setLayout(mainLayout);

    // ---------------------------------------------------------------

    connect(d->updateFingerPrtBtn, SIGNAL(clicked()),
            this, SIGNAL(signalUpdateFingerPrints()));

    connect(d->scanDuplicatesBtn, SIGNAL(clicked()),
            this, SLOT(slotFindDuplicates()));

    connect(d->listView, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotDuplicatesAlbumActived(QTreeWidgetItem*,int)));

    connect(d->progressBar, SIGNAL(signalCancelButtonPressed()),
            this, SLOT(slotCancelButtonPressed()));

    connect(AlbumManager::instance(), SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(populateTreeView()));

    connect(AlbumManager::instance(), SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(slotUpdateAlbumsAndTags()));

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
    delete d;
}

SAlbum* FindDuplicatesView::currentFindDuplicatesAlbum() const
{
    SAlbum* salbum = 0;

    FindDuplicatesAlbumItem* item = dynamic_cast<FindDuplicatesAlbumItem*>(d->listView->currentItem());

    if (item)
    {
        salbum = item->album();
    }

    return salbum;
}

void FindDuplicatesView::populateTreeView()
{
    const AlbumList& aList = AlbumManager::instance()->allSAlbums();

    for (AlbumList::const_iterator it = aList.constBegin(); it != aList.constEnd(); ++it)
    {
        SAlbum* salbum = dynamic_cast<SAlbum*>(*it);

        if (salbum && salbum->isDuplicatesSearch() && !salbum->extraData(this))
        {
            FindDuplicatesAlbumItem* item = new FindDuplicatesAlbumItem(d->listView, salbum);
            salbum->setExtraData(this, item);
        }
    }

    d->listView->sortByColumn(1, Qt::DescendingOrder);
    d->listView->resizeColumnToContents(0);
}

void FindDuplicatesView::slotUpdateAlbumsAndTags()
{
    updateAlbumsBox();
    updateTagsBox();
    checkForValidSettings();
}

void FindDuplicatesView::updateAlbumsBox()
{
    if (d->albumModel)
    {
        disconnect(d->albumModel, 0, this, 0);
    }

    d->albumSelectCB->setDefaultAlbumModel();
    d->albumModel = d->albumSelectCB->model();
    d->albumSelectCB->view()->expandToDepth(0);
    d->albumSelectCB->setNoSelectionText(i18n("No albums selected"));

    connect(d->albumModel, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SLOT(slotAlbumSelectionChanged(Album*,Qt::CheckState)));
}

void FindDuplicatesView::updateTagsBox()
{
    if (d->tagModel)
    {
        disconnect(d->tagModel, 0, this, 0);
    }


    d->tagSelectCB->setDefaultTagModel();
    d->tagModel = d->tagSelectCB->model();
    d->tagSelectCB->view()->expandToDepth(0);
    d->tagSelectCB->setNoSelectionText(i18n("No tags selected"));

    connect(d->tagModel, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SLOT(slotTagSelectionChanged(Album*,Qt::CheckState)));
}

void FindDuplicatesView::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
    {
        return;
    }

    SAlbum* salbum  = static_cast<SAlbum*>(a);

    if (!salbum->isDuplicatesSearch())
    {
        return;
    }

    if (!salbum->extraData(this))
    {
        FindDuplicatesAlbumItem* item = new FindDuplicatesAlbumItem(d->listView, salbum);
        salbum->setExtraData(this, item);
    }
}

void FindDuplicatesView::slotAlbumDeleted(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
    {
        return;
    }

    SAlbum* album = static_cast<SAlbum*>(a);

    FindDuplicatesAlbumItem* item = static_cast<FindDuplicatesAlbumItem*>(album->extraData(this));

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
        SAlbum* salbum = static_cast<FindDuplicatesAlbumItem*>(*it)->album();

        if (salbum)
        {
            salbum->removeExtraData(this);
        }
    }

    d->listView->clear();
}

void FindDuplicatesView::enableControlWidgets(bool val)
{
    d->scanDuplicatesBtn->setEnabled(val && checkForValidSettings());
    d->updateFingerPrtBtn->setEnabled(val);
    d->includeAlbumsLabel->setEnabled(val);
    d->albumSelectCB->setEnabled(val);
    d->tagSelectCB->setEnabled(val);
    d->similarityLabel->setEnabled(val);
    d->similarity->setEnabled(val);

    d->progressBar->progressBarMode(val ? StatusProgressBar::TextMode
                                    : StatusProgressBar::CancelProgressBarMode);
    d->progressBar->setProgressValue(0);
    d->progressBar->setEnabled(!val);
}

void FindDuplicatesView::slotFindDuplicates()
{
    slotClear();
    enableControlWidgets(false);

    QStringList albumsIdList;
    QStringList tagsIdList;
    foreach(const Album* album, d->albumModel->checkedAlbums())
    {
        albumsIdList << QString::number(album->id());
    }
    foreach(const Album* album, d->tagModel->checkedAlbums())
    {
        tagsIdList << QString::number(album->id());
    }

    // --------------------------------------------------------

    double thresh = d->similarity->value() / 100.0;

    KIO::Job* job = ImageLister::startListJob(DatabaseUrl::searchUrl(-1));
    job->addMetaData("duplicates", "normal");
    job->addMetaData("albumids",   albumsIdList.join(","));
    job->addMetaData("tagids",     tagsIdList.join(","));
    job->addMetaData("threshold",  QString::number(thresh));
    d->searchJob = job;

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotDuplicatesSearchResult(KJob*)));

    connect(job, SIGNAL(totalAmount(KJob*,KJob::Unit,qulonglong)),
            this, SLOT(slotDuplicatesSearchTotalAmount(KJob*,KJob::Unit,qulonglong)));

    connect(job, SIGNAL(processedAmount(KJob*,KJob::Unit,qulonglong)),
            this, SLOT(slotDuplicatesSearchProcessedAmount(KJob*,KJob::Unit,qulonglong)));
}

void FindDuplicatesView::slotCancelButtonPressed()
{
    if (d->searchJob)
    {
        d->searchJob->kill();
        d->searchJob = NULL;

        enableControlWidgets(true);
        populateTreeView();
    }
}

void FindDuplicatesView::slotDuplicatesSearchTotalAmount(KJob*, KJob::Unit, qulonglong amount)
{
    d->progressBar->setProgressValue(0);
    d->progressBar->setProgressTotalSteps(amount);
}

void FindDuplicatesView::slotDuplicatesSearchProcessedAmount(KJob*, KJob::Unit, qulonglong amount)
{
    d->progressBar->setProgressValue(amount);
}

void FindDuplicatesView::slotDuplicatesSearchResult(KJob*)
{
    d->searchJob = NULL;

    enableControlWidgets(true);
    populateTreeView();
}

void FindDuplicatesView::slotDuplicatesAlbumActived(QTreeWidgetItem* item, int)
{
    FindDuplicatesAlbumItem* sitem = dynamic_cast<FindDuplicatesAlbumItem*>(item);

    if (sitem)
    {
        AlbumManager::instance()->setCurrentAlbum(sitem->album());
    }
}

void FindDuplicatesView::slotAlbumSelectionChanged(Album* album, Qt::CheckState checkState)
{
    QModelIndex index = d->albumModel->indexForAlbum(album);

    if (index.isValid() && d->albumModel->hasChildren(index))
    {
        AlbumIterator it(album);

        while (it.current())
        {
            d->albumModel->setCheckState(it.current(), checkState);
            ++it;
        }
    }

    checkForValidSettings();
}

void FindDuplicatesView::slotTagSelectionChanged(Album* album, Qt::CheckState checkState)
{
    QModelIndex index = d->tagModel->indexForAlbum(album);

    if (index.isValid() && d->tagModel->hasChildren(index))
    {
        AlbumIterator it(album);

        while (it.current())
        {
            d->tagModel->setCheckState(it.current(), checkState);
            ++it;
        }
    }

    checkForValidSettings();
}

void FindDuplicatesView::slotSetSelectedAlbum(Album* album)
{
    if (!album)
    {
        return;
    }

    resetAlbumsAndTags();
    d->albumModel->setChecked(album, true);
    slotAlbumSelectionChanged(album, Qt::Checked);
}

void FindDuplicatesView::slotSetSelectedTag(Album* album)
{
    if (!album)
    {
        return;
    }

    resetAlbumsAndTags();
    d->tagModel->setChecked(album, true);
    slotTagSelectionChanged(album, Qt::Checked);
}

bool FindDuplicatesView::checkForValidSettings()
{
    bool valid = false;
    valid      = validAlbumSettings() || validTagSettings();

    d->scanDuplicatesBtn->setEnabled(valid);
    return valid;
}

bool FindDuplicatesView::validAlbumSettings()
{
    bool valid = false;

    if (d->albumModel)
    {
        valid = d->albumModel->checkedAlbums().count();
    }

    return valid;
}

bool FindDuplicatesView::validTagSettings()
{
    bool valid = false;

    if (d->tagModel)
    {
        valid = d->tagModel->checkedAlbums().count();
    }

    return valid;
}

void FindDuplicatesView::resetAlbumsAndTags()
{
    d->albumModel->resetCheckedAlbums();
    d->tagModel->resetCheckedAlbums();
    checkForValidSettings();
}

}  // namespace Digikam
