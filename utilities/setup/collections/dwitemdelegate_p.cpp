/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-15
 * Description : widget item delegate for setup collection view
 *
 * Copyright (C) 2015      by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2008 by Rafael Fernández López <ereslibre at kde dot org>
 * Copyright (C) 2008      by Kevin Ottens <ervin at kde dot org>
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

#include "dwitemdelegate.h"
#include "dwitemdelegate_p.h"

// Qt includes

#include <QIcon>
#include <QSize>
#include <QStyle>
#include <QEvent>
#include <QHoverEvent>
#include <QFocusEvent>
#include <QCursor>
#include <QTimer>
#include <QBitmap>
#include <QLayout>
#include <QPainter>
#include <QScrollBar>
#include <QKeyEvent>
#include <QApplication>
#include <QStyleOption>
#include <QPaintEngine>
#include <QCoreApplication>
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QTreeView>

// Local includes

#include "dwitemdelegatepool.h"

namespace Digikam
{

DWItemDelegatePrivate::DWItemDelegatePrivate(DWItemDelegate* const q, QObject* const parent)
    : QObject(parent),
      itemView(0),
      widgetPool(new DWItemDelegatePool(q)),
      model(0),
      selectionModel(0),
      viewDestroyed(false),
      q(q)
{
}

DWItemDelegatePrivate::~DWItemDelegatePrivate()
{
    if (!viewDestroyed)
    {
        widgetPool->fullClear();
    }

    delete widgetPool;
}

void DWItemDelegatePrivate::slotDWRowsInserted(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(end);
    // We need to update the rows behind the inserted row as well because the widgets need to be
    // moved to their new position
    updateRowRange(parent, start, model->rowCount(parent), false);
}

void DWItemDelegatePrivate::slotDWRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    updateRowRange(parent, start, end, true);
}

void DWItemDelegatePrivate::slotDWRowsRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(end);
    // We need to update the rows that come behind the deleted rows because the widgets need to be
    // moved to the new position
    updateRowRange(parent, start, model->rowCount(parent), false);
}

void DWItemDelegatePrivate::slotDWDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    for (int i = topLeft.row(); i <= bottomRight.row(); ++i)
    {
        for (int j = topLeft.column(); j <= bottomRight.column(); ++j)
        {
            const QModelIndex index = model->index(i, j, topLeft.parent());
            widgetPool->findWidgets(index, optionView(index));
        }
    }
}

void DWItemDelegatePrivate::slotDWLayoutChanged()
{
    foreach (QWidget* const widget, widgetPool->invalidIndexesWidgets())
    {
        widget->setVisible(false);
    }

    QTimer::singleShot(0, this, SLOT(initializeModel()));
}

void DWItemDelegatePrivate::slotDWModelReset()
{
    widgetPool->fullClear();
    QTimer::singleShot(0, this, SLOT(initializeModel()));
}

void DWItemDelegatePrivate::slotDWSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    foreach (const QModelIndex& index, selected.indexes())
    {
        widgetPool->findWidgets(index, optionView(index));
    }

    foreach (const QModelIndex& index, deselected.indexes())
    {
        widgetPool->findWidgets(index, optionView(index));
    }
}

void DWItemDelegatePrivate::updateRowRange(const QModelIndex& parent, int start, int end, bool isRemoving)
{
    int i = start;

    while (i <= end)
    {
        for (int j = 0; j < model->columnCount(parent); ++j)
        {
            const QModelIndex index    = model->index(i, j, parent);
            QList<QWidget*> widgetList = widgetPool->findWidgets(index, optionView(index), isRemoving ? DWItemDelegatePool::NotUpdateWidgets
                                                                                                      : DWItemDelegatePool::UpdateWidgets);
            if (isRemoving)
            {
                widgetPool->d->allocatedWidgets.removeAll(widgetList);

                foreach (QWidget* const widget, widgetList)
                {
                    const QModelIndex idx = widgetPool->d->widgetInIndex[widget];
                    widgetPool->d->usedWidgets.remove(idx);
                    widgetPool->d->widgetInIndex.remove(widget);
                    delete widget;
                }
            }
        }

        i++;
    }
}

inline QStyleOptionViewItem DWItemDelegatePrivate::optionView(const QModelIndex& index)
{
    QStyleOptionViewItem optionView;
    optionView.initFrom(itemView->viewport());
    optionView.rect           = itemView->visualRect(index);
    optionView.decorationSize = itemView->iconSize();
    return optionView;
}

void DWItemDelegatePrivate::initializeModel(const QModelIndex& parent)
{
    if (!model)
    {
        return;
    }

    for (int i = 0; i < model->rowCount(parent); ++i)
    {
        for (int j = 0; j < model->columnCount(parent); ++j)
        {
            const QModelIndex index = model->index(i, j, parent);

            if (index.isValid())
            {
                widgetPool->findWidgets(index, optionView(index));
            }
        }
        // Check if we need to go recursively through the children of parent (if any) to initialize
        // all possible indexes that are shown.
        const QModelIndex index = model->index(i, 0, parent);

        if (index.isValid() && model->hasChildren(index))
        {
            initializeModel(index);
        }
    }
}

bool DWItemDelegatePrivate::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Destroy)
    {
        // we care for the view since it deletes the widgets (parentage).
        // if the view hasn't been deleted, it might be that just the
        // delegate is removed from it, in which case we need to remove the widgets
        // manually, otherwise they still get drawn.
        if (watched == itemView)
        {
            viewDestroyed = true;
        }

        return false;
    }

    Q_ASSERT(itemView);

    if (model != itemView->model())
    {
        if (model)
        {
            disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                       q, SLOT(slotDWRowsInserted(QModelIndex,int,int)));

            disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                       q, SLOT(slotDWRowsAboutToBeRemoved(QModelIndex,int,int)));

            disconnect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                       q, SLOT(slotDWRowsRemoved(QModelIndex,int,int)));

            disconnect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                       q, SLOT(slotDWDataChanged(QModelIndex,QModelIndex)));

            disconnect(model, SIGNAL(layoutChanged()),
                       q, SLOT(slotDWLayoutChanged()));

            disconnect(model, SIGNAL(modelReset()),
                       q, SLOT(slotDWModelReset()));
        }

        model = itemView->model();

        connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                q, SLOT(slotDWRowsInserted(QModelIndex,int,int)));

        connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                q, SLOT(slotDWRowsAboutToBeRemoved(QModelIndex,int,int)));

        connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                q, SLOT(slotDWRowsRemoved(QModelIndex,int,int)));

        connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                q, SLOT(slotDWDataChanged(QModelIndex,QModelIndex)));

        connect(model, SIGNAL(layoutChanged()),
                q, SLOT(slotDWLayoutChanged()));

        connect(model, SIGNAL(modelReset()),
                q, SLOT(slotDWModelReset()));

        QTimer::singleShot(0, this, SLOT(initializeModel()));
    }

    if (selectionModel != itemView->selectionModel())
    {
        if (selectionModel)
        {
            disconnect(selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                       q, SLOT(slotDWSelectionChanged(QItemSelection,QItemSelection)));
        }

        selectionModel = itemView->selectionModel();

        connect(selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                q, SLOT(slotDWSelectionChanged(QItemSelection,QItemSelection)));

        QTimer::singleShot(0, this, SLOT(initializeModel()));
    }

    switch (event->type())
    {
        case QEvent::Polish:
        case QEvent::Resize:
            if (!qobject_cast<QAbstractItemView*>(watched))
            {
                QTimer::singleShot(0, this, SLOT(initializeModel()));
            }
            break;
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            if (qobject_cast<QAbstractItemView*>(watched))
            {
                foreach (const QModelIndex& index, selectionModel->selectedIndexes())
                {
                    if (index.isValid())
                    {
                        widgetPool->findWidgets(index, optionView(index));
                    }
                }
            }
        default:
            break;
    }

    return QObject::eventFilter(watched, event);
}

} // namespace Digikam
