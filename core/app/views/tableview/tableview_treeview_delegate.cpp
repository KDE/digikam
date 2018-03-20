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

#include "tableview_treeview_delegate.h"

// Qt includes

#include <QTreeView>

// Local includes

#include "digikam_debug.h"
#include "contextmenuhelper.h"
#include "coredbfields.h"
#include "coredbwatch.h"
#include "fileactionmngr.h"
#include "imageinfo.h"
#include "imagemodel.h"
#include "imageposition.h"
#include "importfiltermodel.h"
#include "importimagemodel.h"
#include "importui.h"
#include "tableview_column_configuration_dialog.h"
#include "tableview_columnfactory.h"
#include "tableview_model.h"
#include "tableview_selection_model_syncer.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

TableViewItemDelegate::TableViewItemDelegate(TableViewShared* const tableViewShared, QObject* const parent)
    : QItemDelegate(parent),
      s(tableViewShared)
{
}

TableViewItemDelegate::~TableViewItemDelegate()
{
}

void TableViewItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& tableViewModelIndex) const
{
    const int columnIndex               = tableViewModelIndex.column();
    const int columnCount               = s->tableViewModel->columnCount(QModelIndex());

    if (columnIndex < 0 || columnIndex >= columnCount)
    {
        QItemDelegate::paint(painter, option, tableViewModelIndex);
        return;
    }

    TableViewColumn* const columnObject = s->tableViewModel->getColumnObject(columnIndex);
    bool useDefaultPainter              = !columnObject->getColumnFlags().testFlag(TableViewColumn::ColumnCustomPainting);

    if (!useDefaultPainter)
    {
        TableViewModel::Item* const item = s->tableViewModel->itemFromIndex(tableViewModelIndex);
        useDefaultPainter                = !columnObject->paint(painter, option, item);
    }

    if (useDefaultPainter)
    {
        QItemDelegate::paint(painter, option, tableViewModelIndex);
    }
}

QSize TableViewItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& tableViewModelIndex) const
{
    const int columnIndex = tableViewModelIndex.column();

    /// we have to take the maximum of all columns for the height
    /// @todo somehow cache this calculation
    /// @todo check column flags
    const int columnCount            = s->tableViewModel->columnCount(QModelIndex());
    TableViewModel::Item* const item = s->tableViewModel->itemFromIndex(tableViewModelIndex);
    int maxHeight                    = 0;

    if (columnIndex < 0 || columnIndex >= columnCount)
    {
        return QItemDelegate::sizeHint(option, tableViewModelIndex);
    }

    for (int i = 0; i < columnCount ; ++i)
    {
        TableViewColumn* const iColumnObject = s->tableViewModel->getColumnObject(i);

        if (iColumnObject && item)
        {
            const QSize iColumnSize = iColumnObject->sizeHint(option, item);

            if (iColumnSize.isValid())
            {
                maxHeight = qMax(maxHeight, iColumnSize.height());
            }
        }
    }

    QSize columnSize;
    TableViewColumn* const columnObject = s->tableViewModel->getColumnObject(columnIndex);

    if (columnObject && item)
        columnSize = columnObject->sizeHint(option, item);

    if (!columnSize.isValid())
    {
        columnSize = QItemDelegate::sizeHint(option, tableViewModelIndex);
        /// @todo we have to incorporate the height given by QItemDelegate for the other columns, too
        maxHeight  = qMax(maxHeight, columnSize.height());
    }

    return QSize(columnSize.width(), maxHeight);
}

} // namespace Digikam
