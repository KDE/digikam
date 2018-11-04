/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-07
 * Description : Trash view
 *
 * Copyright (C) 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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
#include <QToolButton>
#include <QHeaderView>
#include <QListView>
#include <QMessageBox>
#include <QPainter>
#include <QAction>
#include <QMenu>

// KDE includes

#include "klocalizedstring.h"

// Local includes

#include "digikam_debug.h"
#include "dtrashiteminfo.h"
#include "dtrashitemmodel.h"
#include "iojobsmanager.h"
#include "thumbnailsize.h"
#include "scancontroller.h"

namespace Digikam
{

class Q_DECL_HIDDEN TrashView::Private
{

public:

    explicit Private()
        : model(0),
          thumbDelegate(0),
          mainLayout(0),
          btnsLayout(0),
          tableView(0),
          undoButton(0),
          restoreButton(0),
          deleteButton(0),
          deleteAction(0),
          deleteAllAction(0),
          thumbSize(ThumbnailSize::Large)
    {
    }

public:

    DTrashItemModel*           model;
    ThumbnailAligningDelegate* thumbDelegate;
    QVBoxLayout*               mainLayout;
    QHBoxLayout*               btnsLayout;
    QTableView*                tableView;
    QPushButton*               undoButton;
    QPushButton*               restoreButton;
    QToolButton*               deleteButton;
    QAction*                   deleteAction;
    QAction*                   deleteAllAction;

    QModelIndex                lastSelectedIndex;

    DTrashItemInfo             lastSelectedItem;
    QModelIndexList            selectedIndexesToRemove;
    ThumbnailSize              thumbSize;
};

TrashView::TrashView(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    // Layouts
    d->mainLayout    = new QVBoxLayout(this);
    d->btnsLayout    = new QHBoxLayout();

    // View and tools
    d->tableView     = new QTableView(this);
    d->model         = new DTrashItemModel(this);
    d->thumbDelegate = new ThumbnailAligningDelegate(this);

    // Table view settings
    d->tableView->setModel(d->model);
    d->tableView->setItemDelegateForColumn(0, d->thumbDelegate);
    d->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    d->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    d->tableView->verticalHeader()->setDefaultSectionSize(d->thumbSize.size());
    d->tableView->verticalHeader()->hide();
    d->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->tableView->setShowGrid(false);
    d->tableView->setSortingEnabled(true);
    d->tableView->sortByColumn(2, Qt::DescendingOrder);

    // Action Buttons
    d->undoButton      = new QPushButton(i18n("Undo"), this);
    d->restoreButton   = new QPushButton(i18n("Restore"), this);
    d->deleteButton    = new QToolButton(this);

    d->deleteAction    = new QAction(i18n("Delete Permanently"), this);
    d->deleteAllAction = new QAction(i18n("Delete All Permanently"), this);

    d->undoButton->setIcon(QIcon::fromTheme(QLatin1String("edit-undo")));
    d->restoreButton->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
    d->deleteAction->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    d->deleteAllAction->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));

    QMenu* const menu  = new QMenu(this);
    menu->addAction(d->deleteAllAction);

    d->deleteButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->deleteButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->deleteButton->setPopupMode(QToolButton::MenuButtonPopup);
    d->deleteButton->setDefaultAction(d->deleteAction);
    d->deleteButton->setMenu(menu);

    d->undoButton->setEnabled(false);
    d->restoreButton->setEnabled(false);
    d->deleteButton->setEnabled(false);

    // Adding widgets to layouts
    d->mainLayout->addWidget(d->tableView);

    d->btnsLayout->addWidget(d->undoButton);
    d->btnsLayout->addWidget(d->restoreButton);
    d->btnsLayout->addWidget(d->deleteButton);

    d->mainLayout->addLayout(d->btnsLayout);

    // Signals and Slots connections
    connect(d->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotSelectionChanged()));

    connect(d->undoButton, SIGNAL(released()),
            this, SLOT(slotUndoLastDeletedItems()));

    connect(d->restoreButton, SIGNAL(released()),
            this, SLOT(slotRestoreSelectedItems()));

    connect(d->deleteAction, SIGNAL(triggered()),
            this, SLOT(slotDeleteSelectedItems()));

    connect(d->deleteAllAction, SIGNAL(triggered()),
            this, SLOT(slotDeleteAllItems()));

    connect(d->model, SIGNAL(dataChange()),
            this, SLOT(slotDataChanged()));

    connect(d->tableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotChangeLastSelectedItem(QModelIndex,QModelIndex)));
}

TrashView::~TrashView()
{
    delete d;
}

DTrashItemModel* TrashView::model() const
{
    return d->model;
}

ThumbnailSize TrashView::getThumbnailSize() const
{
    return d->thumbSize;
}

void TrashView::slotSelectionChanged()
{
    if (d->tableView->selectionModel()->hasSelection())
    {
        d->restoreButton->setEnabled(true);
    }
    else
    {
        d->restoreButton->setEnabled(false);
    }
}

void TrashView::slotUndoLastDeletedItems()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Undo last deleted items from collection trash";
    DTrashItemInfoList items;
    d->undoButton->setEnabled(false);
    d->selectedIndexesToRemove.clear();
    QDateTime lastDateTime = QDateTime::fromMSecsSinceEpoch(0);

    foreach (const DTrashItemInfo& item, d->model->allItems())
    {
        if (item.deletionTimestamp > lastDateTime)
        {
            lastDateTime = item.deletionTimestamp;
        }
    }

    foreach (const DTrashItemInfo& item, d->model->allItems())
    {
        if (item.deletionTimestamp == lastDateTime)
        {
            QModelIndex index = d->model->indexForItem(item);

            if (index.isValid())
            {
                items << item;
                d->selectedIndexesToRemove << index;
            }
        }
    }

    if (items.isEmpty())
    {
        d->undoButton->setEnabled(true);
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Items to Restore:\n " << items;

    IOJobsThread* const thread = IOJobsManager::instance()->startRestoringDTrashItems(items);

    connect(thread, SIGNAL(finished()),
            this, SLOT(slotRestoreFinished()));
}

void TrashView::slotRestoreSelectedItems()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Restoring selected items from collection trash";

    d->selectedIndexesToRemove = d->tableView->selectionModel()->selectedRows();
    DTrashItemInfoList items   = d->model->itemsForIndexes(d->selectedIndexesToRemove);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Items to Restore:\n " << items;

    IOJobsThread* const thread = IOJobsManager::instance()->startRestoringDTrashItems(items);

    connect(thread, SIGNAL(finished()),
            this, SLOT(slotRestoreFinished()));
}

void TrashView::slotRestoreFinished()
{
    if (d->selectedIndexesToRemove.isEmpty())
    {
        return;
    }

    DTrashItemInfoList items = d->model->itemsForIndexes(d->selectedIndexesToRemove);

    foreach (const DTrashItemInfo& item, items)
    {
        QUrl url     = QUrl::fromLocalFile(item.collectionPath);
        QString path = url.adjusted(QUrl::RemoveFilename).toLocalFile();
        ScanController::instance()->scheduleCollectionScanRelaxed(path);
    }

    slotRemoveItemsFromModel();
}

void TrashView::slotDeleteSelectedItems()
{
    d->selectedIndexesToRemove = d->tableView->selectionModel()->selectedRows();

    if (d->selectedIndexesToRemove.isEmpty())
    {
        return;
    }

    QString title = i18n("Confirm Deletion");
    QString msg   = i18n("Are you sure you want to delete those items permanently?");
    int result    = QMessageBox::warning(this, title, msg, QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::No)
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Deleting selected items from collection trash";

    DTrashItemInfoList items = d->model->itemsForIndexes(d->selectedIndexesToRemove);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Items count: " << items.count();

    IOJobsThread* const thread = IOJobsManager::instance()->startDeletingDTrashItems(items);

    connect(thread, SIGNAL(finished()),
            this, SLOT(slotRemoveItemsFromModel()));
}

void TrashView::slotRemoveItemsFromModel()
{
    if (d->selectedIndexesToRemove.isEmpty())
    {
        return;
    }

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
    {
        return;
    }

    QString title = i18n("Confirm Deletion");
    QString msg   = i18n("Are you sure you want to delete ALL items permanently?");
    int result    = QMessageBox::warning(this, title, msg, QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::No)
        return;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Removing all item from trash permanently";

    IOJobsThread* const thread = IOJobsManager::instance()->startDeletingDTrashItems(d->model->allItems());

    connect(thread, SIGNAL(finished()),
            this, SLOT(slotRemoveAllItemsFromModel()));
}

void TrashView::slotDataChanged()
{
    selectLastSelected();

    if (d->model->isEmpty())
    {
        d->undoButton->setEnabled(false);
        d->deleteButton->setEnabled(false);
        return;
    }

    d->undoButton->setEnabled(true);
    d->deleteButton->setEnabled(true);
}

void TrashView::slotChangeLastSelectedItem(const QModelIndex& curr, const QModelIndex&)
{
    d->lastSelectedIndex = curr;
    d->lastSelectedItem  = d->model->itemForIndex(curr);
    emit selectionChanged();
}

void TrashView::setThumbnailSize(ThumbnailSize thumbSize)
{
    d->model->changeThumbSize(thumbSize.size());
    d->tableView->verticalHeader()->setDefaultSectionSize(thumbSize.size());
    d->thumbSize = thumbSize;
}

QUrl TrashView::lastSelectedItemUrl() const
{
    return QUrl::fromLocalFile(d->lastSelectedItem.trashPath);
}

void TrashView::selectLastSelected()
{
    if (d->model->isEmpty())
    {
        d->lastSelectedItem  = DTrashItemInfo();
        d->lastSelectedIndex = QModelIndex();
    }
    else if (!d->lastSelectedIndex.isValid())
    {
        d->tableView->selectRow(0);
        d->tableView->scrollTo(QModelIndex(), QAbstractItemView::EnsureVisible);
    }
    else
    {
        d->tableView->selectRow(d->lastSelectedIndex.row());
        d->tableView->scrollTo(d->lastSelectedIndex, QAbstractItemView::EnsureVisible);
    }

    emit selectionChanged();
}

// --------------------------------------------------

ThumbnailAligningDelegate::ThumbnailAligningDelegate(QObject* const parent)
    : QStyledItemDelegate(parent)
{
}

void ThumbnailAligningDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QPixmap pixmap;
    pixmap     = index.data(Qt::DecorationRole).value<QPixmap>();
    QPoint loc = option.rect.center() - pixmap.rect().center();

    painter->save();

    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
    }
    else if (option.state & QStyle::State_MouseOver)
    {
        painter->fillRect(option.rect, option.palette.brush(QPalette::Inactive, QPalette::Highlight));
    }

    painter->drawPixmap(loc, pixmap);
    painter->restore();
}

} // namespace Digikam
