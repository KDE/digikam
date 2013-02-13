/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-12
 * Description : Wrapper model for table view
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

#include "tableview_model.moc"

// local includes
#include "imagefiltermodel.h"
#include "tableview_columnfactory.h"
#include <valarray>

namespace Digikam
{

class TableViewModel::Private
{
public:
    ImageFilterModel* sourceModel;
    TableViewColumnFactory* columnFactory;
    QList<TableViewColumn*> columnObjects;
};

TableViewModel::TableViewModel(TableViewColumnFactory*const tableViewColumnFactory, Digikam::ImageFilterModel*const sourceModel, QObject* parent)
  : QAbstractItemModel(parent),
    d(new Private())
{
    d->sourceModel = sourceModel;
    d->columnFactory = tableViewColumnFactory;

    d->columnObjects << d->columnFactory->getColumn(TableViewColumnConfiguration("filename"));
    d->columnObjects << d->columnFactory->getColumn(TableViewColumnConfiguration("coordinates"));
}

TableViewModel::~TableViewModel()
{
    /// @todo delete d->columnObjects, d->tableViewColumnDataSource
}

int TableViewModel::columnCount(const QModelIndex& i) const
{
    return d->columnObjects.count();
}

QVariant TableViewModel::data(const QModelIndex& i, int role) const
{
    if (i.isValid())
    {
        Q_ASSERT(i.model()==this);
    }

    const int rowNumber = i.row();
    const int columnNumber = i.column();

    if ( (rowNumber<0) || (rowNumber>=d->sourceModel->rowCount()) )
    {
        return QVariant();
    }

    const QModelIndex sourceIndex = d->sourceModel->index(rowNumber, 0);
    if (!sourceIndex.isValid())
    {
        return QVariant();
    }

    TableViewColumn* const myColumn = d->columnObjects.at(columnNumber);
    return myColumn->data(sourceIndex, role);
}

QModelIndex TableViewModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        // there are no sub-items
        return QModelIndex();
    }

    /// @todo range-check on row, column

    return createIndex(row, column, 0);
}

QModelIndex TableViewModel::parent(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    // we only have top level items
    return QModelIndex();
}

int TableViewModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return d->sourceModel->rowCount();
}

QVariant TableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation!=Qt::Horizontal)||(role!=Qt::DisplayRole))
    {
        return QVariant();
    }

    TableViewColumn* const myColumn = d->columnObjects.at(section);
    return myColumn->getTitle();
}

void TableViewModel::addColumnAt(const TableViewColumnDescription& description, const int targetColumn)
{
    /// @todo take additional configuration data of the column into account
    TableViewColumnConfiguration newConfiguration(description.columnId);

    TableViewColumn* const newColumn = d->columnFactory->getColumn(newConfiguration);

    int newColumnIndex = targetColumn;
    if ( (targetColumn<0) || (targetColumn >= d->columnObjects.count()) )
    {
        newColumnIndex = d->columnObjects.count() - 1;
    }

    beginInsertColumns(QModelIndex(), newColumnIndex, newColumnIndex);
    d->columnObjects.insert(targetColumn, newColumn);
    endInsertColumns();
}

void TableViewModel::removeColumnAt(const int columnIndex)
{
    beginRemoveColumns(QModelIndex(), columnIndex, columnIndex);
    TableViewColumn* removedColumn = d->columnObjects.takeAt(columnIndex);
    endRemoveColumns();

    delete removedColumn;
}

} /* namespace Digikam */

