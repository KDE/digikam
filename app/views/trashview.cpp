/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-07
 * Description : Trash view
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "trashview.h"

// Qt includes

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QPushButton>
#include <QHeaderView>
#include <QListView>
#include <QMessageBox>

// KDE includes

#include "klocalizedstring.h"

// Local includes

#include "dtrashiteminfo.h"
#include "dtrashitemmodel.h"
#include "thumbnailsize.h"
#include "iojobsmanager.h"
#include "iojobsthread.h"
#include "digikam_debug.h"

namespace Digikam
{

class TrashView::Private
{

public:

    Private()
        : model(0),
          mainLayout(0),
          btnsLayout(0),
          tableView(0),
          restoreButton(0),
          deleteButton(0),
          deleteAllButton(0),
          thumbSize(ThumbnailSize::Large)
    {
    }

public:

    DTrashItemModel* model;
    QVBoxLayout*     mainLayout;
    QHBoxLayout*     btnsLayout;
    QTableView*      tableView;
    QPushButton*     restoreButton;
    QPushButton*     deleteButton;
    QPushButton*     deleteAllButton;
    IOJobsThread*    itemsLoadingThread;
    QModelIndexList  selectedIndexesToRemove;
    ThumbnailSize    thumbSize;
};

TrashView::TrashView(QWidget* parent)
    : QWidget(parent), d(new Private)
{
    d->mainLayout = new QVBoxLayout(this);
    d->btnsLayout = new QHBoxLayout();

    d->tableView = new QTableView(this);

    d->model = new DTrashItemModel(this);

    d->tableView->setModel(d->model);
    d->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    d->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    d->tableView->verticalHeader()->setDefaultSectionSize(d->thumbSize.size());
    d->tableView->verticalHeader()->hide();
    d->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->tableView->setShowGrid(false);

    connect(d->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotSelectionChanged()));

    d->restoreButton   = new QPushButton(i18n("Restore"));
    d->deleteButton    = new QPushButton(i18n("Delete Permanently"));
    d->deleteAllButton = new QPushButton(i18n("Delete All Permanently"));

    d->restoreButton->setEnabled(false);
    d->deleteButton->setEnabled(false);
    d->deleteAllButton->setEnabled(false);

    d->mainLayout->addWidget(d->tableView);

    d->btnsLayout->addWidget(d->restoreButton);
    d->btnsLayout->addWidget(d->deleteButton);
    d->btnsLayout->addWidget(d->deleteAllButton);

    d->mainLayout->addLayout(d->btnsLayout);

    connect(d->restoreButton, SIGNAL(released()),
            this, SLOT(slotRestoreSelectedItems()));

    connect(d->deleteButton, SIGNAL(released()),
            this, SLOT(slotDeleteSelectedItems()));

    connect(d->deleteAllButton, SIGNAL(released()),
            this, SLOT(slotDeleteAllItems()));

    connect(d->model, SIGNAL(dataChange()),
            this, SLOT(slotDataChanged()));
}

TrashView::~TrashView()
{
    delete d;
}

DTrashItemModel* TrashView::model()
{
    return d->model;
}

void TrashView::showTrashItemsForCollection(const QString& collectionPath)
{
    d->model->clearCurrentData();
    d->itemsLoadingThread = IOJobsManager::instance()->startDTrashItemsListingForCollection(collectionPath);

    connect(d->itemsLoadingThread, SIGNAL(collectionTrashItemInfo(DTrashItemInfo)),
            d->model, SLOT(append(DTrashItemInfo)));
}

ThumbnailSize TrashView::getThumbnailSize()
{
    return d->thumbSize;
}

void TrashView::slotSelectionChanged()
{
    if (d->tableView->selectionModel()->hasSelection())
    {
        d->deleteButton->setEnabled(true);
        d->restoreButton->setEnabled(true);
    }
    else
    {
        d->deleteButton->setEnabled(false);
        d->restoreButton->setEnabled(false);
    }
}

void TrashView::slotRestoreSelectedItems()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Restoring selected items from collection trash";

    d->selectedIndexesToRemove = d->tableView->selectionModel()->selectedRows();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Number of selected items to restore: "
                                 << d->selectedIndexesToRemove;

    DTrashItemInfoList items = d->model->itemsForIndexes(d->selectedIndexesToRemove);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Items to Restore:\n " << items;

    IOJobsThread* const thread = IOJobsManager::instance()->startRestoringDTrashItems(items);

    connect(thread, SIGNAL(finished()),
            this, SLOT(slotRemoveItemsFromModel()));
}

void TrashView::slotDeleteSelectedItems()
{
    QString title = i18n("Confirm Deletion");
    QString msg = i18n("Are you sure you want to delete those items permanently?");

    int result = QMessageBox::warning(this, title, msg, QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::No)
        return;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Deleting selected items from collection trash";

    d->selectedIndexesToRemove = d->tableView->selectionModel()->selectedRows();
    DTrashItemInfoList items = d->model->itemsForIndexes(d->selectedIndexesToRemove);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Items count: " << items.count();

    IOJobsThread* const thread = IOJobsManager::instance()->startDeletingDTrashItems(items);

    connect(thread, SIGNAL(finished()),
            this, SLOT(slotRemoveItemsFromModel()));
}

void TrashView::slotRemoveItemsFromModel()
{
    if (d->selectedIndexesToRemove.isEmpty())
        return;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Removing deleted items from view";

    d->model->removeItems(d->selectedIndexesToRemove);
    d->selectedIndexesToRemove.clear();
}

void TrashView::slotRemoveAllItemsFromModel()
{
    d->model->clearCurrentData();
}

void TrashView::slotDeleteAllItems()
{
    if (d->model->isEmpty())
        return;

    QString title = i18n("Confirm Deletion");
    QString msg = i18n("Are you sure you want to delete ALL items permanently?");

    int result = QMessageBox::warning(this, title, msg, QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::No)
        return;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Removing all item from trash permanently";

    IOJobsThread* const thread =
            IOJobsManager::instance()->startDeletingDTrashItems(d->model->allItems());

    connect(thread,SIGNAL(finished()),
            this, SLOT(slotRemoveAllItemsFromModel()));
}

void TrashView::slotDataChanged()
{
    if (d->model->isEmpty())
    {
        d->deleteAllButton->setEnabled(false);
        return;
    }
    d->deleteAllButton->setEnabled(true);
}

void TrashView::setThumbnailSize(ThumbnailSize thumbSize)
{
    d->model->changeThumbSize(thumbSize.size());
    d->tableView->verticalHeader()->setDefaultSectionSize(thumbSize.size());
    d->thumbSize = thumbSize;
}

} // namespace Digikam
