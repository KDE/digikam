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

#include <QItemDelegate>
#include <QTreeView>

// Local includes

#include "digikam_export.h"
#include "imagealbummodel.h"
#include "imagefiltermodel.h"
#include "statesavingobject.h"
#include "tableview_columnfactory.h"
#include "tableview_shared.h"
#include "thumbnailloadthread.h"

class QMenu;
class QContextMenuEvent;

namespace Digikam
{

class TableViewItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    explicit TableViewItemDelegate(TableViewShared* const tableViewShared, QObject* const parent = 0);
    virtual ~TableViewItemDelegate();

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& tableViewModelIndex) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& tableViewModelIndex) const;

private:

    TableViewShared* const s;
};

} /* namespace Digikam */

#endif // TABLEVIEW_TREEVIEW_DELEGATE_H
