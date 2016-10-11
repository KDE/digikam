/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-08-22
 * Description : Reimplemented QListView for Tags Manager, with support for
 *               drag-n-drop
 *
 * Copyright (C) 2013-2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "namespacelistview.h"

// Qt includes

#include <QDrag>
#include <QDropEvent>
#include <QMimeData>
#include <QItemSelectionModel>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QStandardItem>
#include <QStandardItemModel>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

NamespaceListView::NamespaceListView(QWidget* const parent)
    : QListView(parent)
{
    setAlternatingRowColors(true);
    setAcceptDrops(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void NamespaceListView::startDrag(Qt::DropActions supportedActions)
{
    QListView::startDrag(supportedActions);
}

QModelIndexList NamespaceListView::mySelectedIndexes()
{
    return this->selectedIndexes();
}

void NamespaceListView::dropEvent(QDropEvent* e)
{
    QListView::dropEvent(e);
    emit signalItemsChanged();
}

QModelIndex NamespaceListView::indexVisuallyAt(const QPoint& p)
{
    if (viewport()->rect().contains(p))
    {
        QModelIndex index = indexAt(p);

        if (index.isValid() && visualRect(index).contains(p))
        {
            return index;
        }
    }

    return QModelIndex();
}

//void NamespaceListView::contextMenuEvent(QContextMenuEvent* event)
//{
//    Q_UNUSED(event);

//    QMenu popmenu(this);
//    ContextMenuHelper cmhelper(&popmenu);

//    TagList* const tagList = dynamic_cast<TagList*>(this->parent());

//    if (!tagList)
//    {
//        return;
//    }

//    QAction* const delAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash")), i18n("Delete Selected from List"),this);
//    cmhelper.addAction(delAction, tagList, SLOT(slotDeleteSelected()),false);

//    QModelIndexList sel = this->selectionModel()->selectedIndexes();
//
//    if (sel.size() == 1 && sel.first().row() == 0)
//        delAction->setDisabled(true);

//    cmhelper.exec(QCursor::pos());
//}

void NamespaceListView::slotDeleteSelected()
{
    QModelIndexList sel = this->selectionModel()->selectedIndexes();

    if (sel.isEmpty())
    {
        return;
    }

    QStandardItemModel* const model = dynamic_cast<QStandardItemModel*>(this->model());

    if (!model)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error! no model available!";
        return;
    }

    foreach (const QModelIndex& index, sel)
    {
        QStandardItem* const root = model->invisibleRootItem();
        root->removeRow(index.row());
    }

    emit signalItemsChanged();
}

void NamespaceListView::slotMoveItemUp()
{
    QModelIndexList sel = this->selectionModel()->selectedIndexes();

    if (sel.isEmpty())
    {
        return;
    }

    QStandardItemModel* const model = dynamic_cast<QStandardItemModel*>(this->model());

    if (!model)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error! no model available!";
        return;
    }

    QModelIndex index = sel.first();

    if (index.row() == 0)
    {
        return;
    }

    QStandardItem* const root    = model->invisibleRootItem();
    int savedRow                 = index.row();
    QStandardItem* const item    = root->child(index.row());
    QStandardItem* const newCopy = item->clone();

    root->removeRow(index.row());
    root->insertRow(savedRow - 1, newCopy);

    this->setCurrentIndex(model->index(index.row() - 1, index.column(), index.parent()));

    emit signalItemsChanged();
}

void NamespaceListView::slotMoveItemDown()
{
    QModelIndexList sel = this->selectionModel()->selectedIndexes();

    if (sel.isEmpty())
    {
        return;
    }

    QStandardItemModel* const model = dynamic_cast<QStandardItemModel*>(this->model());

    if (!model)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error! no model available!";
        return;
    }

    QModelIndex index = sel.first();

    QStandardItem* const root = model->invisibleRootItem();

    if (index.row() == root->rowCount() - 1)
    {
        return;
    }

    int savedRow                 = index.row();
    QStandardItem* const item    = root->child(index.row());
    QStandardItem* const newCopy = item->clone();


    root->removeRow(index.row());
    root->insertRow(savedRow + 1, newCopy);

    this->setCurrentIndex(model->index(index.row() + 1, index.column(), index.parent()));

    emit signalItemsChanged();
}

} // namespace Digikam
