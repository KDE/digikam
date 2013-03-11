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

#include "tableview_treeview_delegate.moc"

// Qt includes

#include <QContextMenuEvent>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QTreeView>
#include <QVBoxLayout>

// KDE includes

#include <kmenu.h>
#include <kaction.h>
#include <klinkitemselectionmodel.h>

// local includes

/// @todo clean up includes
#include "imageposition.h"
#include "imageinfo.h"
#include "imagemodel.h"
#include "importfiltermodel.h"
#include "importimagemodel.h"
#include "databasewatch.h"
#include "databasefields.h"
#include "digikam2kgeomap_database.h"
#include "importui.h"
#include "thumbnailloadthread.h"
#include "tableview_model.h"
#include "tableview_columnfactory.h"
#include "tableview_column_configuration_dialog.h"
#include "tableview_selection_model_syncer.h"
#include "contextmenuhelper.h"
#include "fileactionmngr.h"
#include "tableview_sortfilterproxymodel.h"

namespace Digikam
{

class ImageAlbumModel;
class ImageFilterModel;

TableViewItemDelegate::TableViewItemDelegate(TableViewShared* const tableViewShared, QObject* parent)
  : QItemDelegate(parent),
    s(tableViewShared)
{

}

TableViewItemDelegate::~TableViewItemDelegate()
{

}

void TableViewItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& sortedIndex) const
{
    const int columnIndex = sortedIndex.column();
    TableViewColumn* const columnObject = s->tableViewModel->getColumnObject(columnIndex);

    bool useDefaultPainter = !columnObject->getColumnFlags().testFlag(TableViewColumn::ColumnCustomPainting);

    if (!useDefaultPainter)
    {
        const QModelIndex tableViewIndex = s->sortModel->mapToSource(sortedIndex);
        const QModelIndex sourceIndex = s->tableViewModel->toImageFilterModelIndex(tableViewIndex);

        useDefaultPainter = !columnObject->paint(painter, option, sourceIndex);
    }

    if (useDefaultPainter)
    {
        QItemDelegate::paint(painter, option, sortedIndex);
    }
}

QSize TableViewItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sortedIndex) const
{
    const int columnIndex = sortedIndex.column();
    TableViewColumn* const columnObject = s->tableViewModel->getColumnObject(columnIndex);
    const QModelIndex tableViewIndex = s->sortModel->mapToSource(sortedIndex);
    const QModelIndex sourceIndex = s->tableViewModel->toImageFilterModelIndex(tableViewIndex);

    /// we have to take the maximum of all columns for the height
    /// @todo somehow cache this calculation
    /// @todo check column flags
    const int columnCount = s->tableViewModel->columnCount(QModelIndex());
    int maxHeight = 0;
    for (int i=0; i<columnCount; ++i)
    {
        TableViewColumn* const iColumnObject = s->tableViewModel->getColumnObject(i);
        const QSize iColumnSize = iColumnObject->sizeHint(option, sourceIndex);
        if (iColumnSize.isValid())
        {
            maxHeight = qMax(maxHeight, iColumnSize.height());
        }
    }

    QSize columnSize = columnObject->sizeHint(option, sourceIndex);
    if (!columnSize.isValid())
    {
        columnSize = QItemDelegate::sizeHint(option, sortedIndex);
        /// @todo we have to incorporate the height given by QItemDelegate for the other columns, too
        maxHeight = qMax(maxHeight, columnSize.height());
    }

    return QSize(columnSize.width(), maxHeight);
}

} /* namespace Digikam */
