/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-03-02
 * Description : Table view: Tree view subelement
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

#ifndef TABLEVIEW_TREEVIEW_DELEGATE_H
#define TABLEVIEW_TREEVIEW_DELEGATE_H

// Qt includes

#include <QTreeView>
#include <QWidget>
#include <QItemDelegate>

// KDE includes

#include "kcategorizedsortfilterproxymodel.h"
#include "kdialog.h"

// local includes

/// @todo clean up includes and use forward-declarations where possible
#include "statesavingobject.h"
#include "digikam_export.h"
#include "imagealbummodel.h"
#include "thumbnailloadthread.h"
#include "imagefiltermodel.h"
#include "tableview_columnfactory.h"
#include "tableview_shared.h"

class KMenu;
class QContextMenuEvent;

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

} /* namespace Digikam */

#endif // TABLEVIEW_TREEVIEW_DELEGATE_H
