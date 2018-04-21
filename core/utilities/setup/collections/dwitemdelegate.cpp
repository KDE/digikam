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

#include <QSize>
#include <QStyle>
#include <QEvent>
#include <QHoverEvent>
#include <QFocusEvent>
#include <QTimer>
#include <QStyleOption>
#include <QPaintEngine>
#include <QApplication>
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QTreeView>

// Local includes

#include "dwitemdelegatepool.h"

namespace Digikam
{

DWItemDelegate::DWItemDelegate(QAbstractItemView* const itemView, QObject* const parent)
    : QAbstractItemDelegate(parent),
      d(new DWItemDelegatePrivate(this))
{
    Q_ASSERT(itemView);

    itemView->setMouseTracking(true);
    itemView->viewport()->setAttribute(Qt::WA_Hover);

    d->itemView = itemView;

    itemView->viewport()->installEventFilter(d); // mouse events
    itemView->installEventFilter(d);             // keyboard events

    if (qobject_cast<QTreeView*>(itemView))
    {
        connect(itemView,  SIGNAL(collapsed(QModelIndex)),
                d, SLOT(initializeModel()));

        connect(itemView,  SIGNAL(expanded(QModelIndex)),
                d, SLOT(initializeModel()));
    }
}

DWItemDelegate::~DWItemDelegate()
{
    delete d;
}

QAbstractItemView* DWItemDelegate::itemView() const
{
    return d->itemView;
}

QPersistentModelIndex DWItemDelegate::focusedIndex() const
{
    const QPersistentModelIndex idx = d->widgetPool->d->widgetInIndex.value(QApplication::focusWidget());

    if (idx.isValid())
    {
        return idx;
    }

    // Use the mouse position, if the widget refused to take keyboard focus.
    const QPoint pos = d->itemView->viewport()->mapFromGlobal(QCursor::pos());

    return d->itemView->indexAt(pos);
}

void DWItemDelegate::setBlockedEventTypes(QWidget* const widget, QList<QEvent::Type> types) const
{
    widget->setProperty("DigiKam:blockedEventTypes", qVariantFromValue(types));
}

QList<QEvent::Type> DWItemDelegate::blockedEventTypes(QWidget* const widget) const
{
    return widget->property("Digikam:blockedEventTypes").value<QList<QEvent::Type> >();
}

} // namespace Digikam
