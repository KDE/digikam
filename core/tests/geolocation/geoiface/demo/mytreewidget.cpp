/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-06
 * Description : sub class of QTreeWidget for drag-and-drop support
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
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

#include "mytreewidget.h"

// Qt includes

#include <QApplication>
#include <QMouseEvent>
#include <QDrag>

// geoiface includes

#include "geoifacetypes.h"

// local includes

#include "mydragdrophandler.h"

class MyTreeWidget::Private
{
public:

    Private()
        : dragStartPos()
    {
    }

    QPoint dragStartPos;
};

MyTreeWidget::MyTreeWidget(QWidget* const parent)
    : QTreeWidget(parent),
      d(new Private())
{
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
}

MyTreeWidget::~MyTreeWidget()
{
    delete d;
}

void MyTreeWidget::startDrag(Qt::DropActions /*supportedActions*/)
{
    QMimeData* const dragMimeData = mimeData(selectionModel()->selectedIndexes());
    QDrag* const drag             = new QDrag(this);
    drag->setMimeData(dragMimeData);
    drag->exec(Qt::CopyAction);
}

QMimeData* MyTreeWidget::mimeData(const QList<QTreeWidgetItem*> items) const
{
    return QTreeWidget::mimeData(items);
}

QMimeData* MyTreeWidget::mimeData(const QModelIndexList itemsToDrag) const
{
    MyDragData* const mimeData = new MyDragData;

    // TODO: determine the indices of the items to drag!
    for (int i = 0; i < itemsToDrag.count(); ++i)
    {
        mimeData->draggedIndices << QPersistentModelIndex(itemsToDrag.at(i));
    }

    return mimeData;
}
