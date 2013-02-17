/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-11
 * Description : Table view
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

// Qt includes

#include <QTreeView>
#include <QWidget>
#include <QItemDelegate>

// KDE includes

#include "kcategorizedsortfilterproxymodel.h"

// local includes

/// @todo clean up includes and use forward-declarations where possible
#include "statesavingobject.h"
#include "digikam_export.h"
#include "imagealbummodel.h"
#include "thumbnailloadthread.h"
#include "imagefiltermodel.h"
#include "tableview_columnfactory.h"
#include "tableview_shared.h"

namespace Digikam
{

class TableViewItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    explicit TableViewItemDelegate(TableViewShared* const tableViewShared, QObject* parent = 0);
    virtual ~TableViewItemDelegate();

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& sortedIndex) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sortedIndex) const;

private:

    TableViewShared* const s;
};

class TableViewTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit TableViewTreeView(TableViewShared* const tableViewShared, QWidget* const parent = 0);
    virtual ~TableViewTreeView();

protected:

    virtual bool eventFilter(QObject* watched, QEvent* event);

private:

    void showHeaderContextMenu(QEvent* const event);

private Q_SLOTS:

    void slotHeaderContextMenuAddColumn();
    void slotHeaderContextMenuActionRemoveColumnTriggered();

private:

    class Private;
    const QScopedPointer<Private> d;
    TableViewShared* const s;
};

class TableView : public QWidget, public StateSavingObject
{
    Q_OBJECT

public:

    explicit TableView(
            QItemSelectionModel* const selectionModel,
            KCategorizedSortFilterProxyModel* const imageFilterModel,
            QWidget* const parent
        );
    virtual ~TableView();

protected:

    void doLoadState();
    void doSaveState();

private:

    class Private;
    const QScopedPointer<Private> d;
    const QScopedPointer<TableViewShared> s;
};

} /* namespace Digikam */

#endif // TABLEVIEW_H
