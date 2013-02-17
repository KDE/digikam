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

TableViewModel::TableViewModel(TableViewColumnFactory* const tableViewColumnFactory, Digikam::ImageFilterModel* const sourceModel, QObject* parent)
  : QAbstractItemModel(parent),
    d(new Private())
{
    d->sourceModel = sourceModel;
    d->columnFactory = tableViewColumnFactory;

    d->columnObjects << d->columnFactory->getColumn(TableViewColumnConfiguration("thumbnail"));
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

QModelIndex TableViewModel::toImageFilterModelIndex(const QModelIndex& i) const
{
    if (i.isValid())
    {
        Q_ASSERT(i.model()==this);
    }

    const int rowNumber = i.row();

    if ( (rowNumber<0) || (rowNumber>=d->sourceModel->rowCount()) )
    {
        return QModelIndex();
    }

    const QModelIndex sourceIndex = d->sourceModel->index(rowNumber, 0);
    if (!sourceIndex.isValid())
    {
        return QModelIndex();
    }

    return sourceIndex;
}

QVariant TableViewModel::data(const QModelIndex& i, int role) const
{
    const int columnNumber = i.column();

    QModelIndex sourceIndex = toImageFilterModelIndex(i);
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

    addColumnAt(newConfiguration, targetColumn);
}

void TableViewModel::addColumnAt(const TableViewColumnConfiguration& configuration, const int targetColumn)
{
    TableViewColumn* const newColumn = d->columnFactory->getColumn(configuration);

    int newColumnIndex = targetColumn;
    if (targetColumn<0)
    {
        // a negative column index means "append after last column"
        newColumnIndex = d->columnObjects.count();
    }

    beginInsertColumns(QModelIndex(), newColumnIndex, newColumnIndex);
    if (newColumnIndex >= d->columnObjects.count())
    {
        d->columnObjects.append(newColumn);
    }
    else
    {
        d->columnObjects.insert(newColumnIndex, newColumn);
    }
    endInsertColumns();

    connect(newColumn, SIGNAL(signalDataChanged(QModelIndex)),
            this, SLOT(slotColumnDataChanged(QModelIndex)));
}

void TableViewModel::slotColumnDataChanged(const QModelIndex& sourceIndex)
{
    TableViewColumn* const senderColumn = qobject_cast<TableViewColumn*>(sender());

    /// @todo find a faster way to find the column number
    const int iColumn = d->columnObjects.indexOf(senderColumn);
    if (iColumn<0)
    {
        return;
    }

    QModelIndex changedIndex = index(sourceIndex.row(), iColumn, QModelIndex());
    emit(dataChanged(changedIndex, changedIndex));
}

void TableViewModel::removeColumnAt(const int columnIndex)
{
    beginRemoveColumns(QModelIndex(), columnIndex, columnIndex);
    TableViewColumn* removedColumn = d->columnObjects.takeAt(columnIndex);
    endRemoveColumns();

    delete removedColumn;
}

TableViewColumn* TableViewModel::getColumnObject(const int columnIndex)
{
    return d->columnObjects.at(columnIndex);
}

TableViewColumnProfile TableViewModel::getColumnProfile() const
{
    TableViewColumnProfile profile;

    for (int i=0; i<d->columnObjects.count(); ++i)
    {
        TableViewColumnConfiguration ic = d->columnObjects.at(i)->getConfiguration();
        profile.columnConfigurationList << ic;
    }

    return profile;
}

void TableViewModel::loadColumnProfile(const TableViewColumnProfile& columnProfile)
{
    while (!d->columnObjects.isEmpty())
    {
        removeColumnAt(0);
    }

    /// @todo disable updating of the model while this happens
    for (int i=0; i<columnProfile.columnConfigurationList.count(); ++i)
    {
        addColumnAt(columnProfile.columnConfigurationList.at(i), -1);
    }
}

} /* namespace Digikam */

