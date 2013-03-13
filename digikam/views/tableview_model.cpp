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

// C++ includes

#include <valarray>

// KDE includes

#include <kdebug.h>

// boost includes

#include <boost/config/posix_features.hpp>

// local includes

/// @todo Clean up include list

#include "albumdb.h"
#include "databaseaccess.h"
#include "databasechangesets.h"
#include "databasechangesets.h"
#include "databasefields.h"
#include "databasewatch.h"
#include "imagefiltermodel.h"
#include "imageinfo.h"
#include "imagelister.h"
#include "imageposition.h"
#include "modeltest/modeltest.h"
#include "tableview_columnfactory.h"
#include "tableview_selection_model_syncer.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class TableViewModelItem
{
public:

    TableViewModelItem()
      : imageId(0),
        imageFilterModelIndex(),
        parent(0),
        children()
    {
    }

    ~TableViewModelItem()
    {
        qDeleteAll(children);
    }

    void addChild(TableViewModelItem* const newChild)
    {
        newChild->parent = this;

        children << newChild;
    }

    void takeChild(TableViewModelItem* const oldChild)
    {
        children.removeOne(oldChild);
    }

    TableViewModelItem* findChildWithImageId(const qlonglong searchImageId)
    {
        if (imageId==searchImageId)
        {
            return this;
        }

        Q_FOREACH(TableViewModelItem* const item, children)
        {
            TableViewModelItem* const iItem = item->findChildWithImageId(searchImageId);
            if (iItem)
            {
                return iItem;
            }
        }

        return 0;
    }

    qlonglong imageId;
    QPersistentModelIndex imageFilterModelIndex;
    TableViewModelItem* parent;
    QList<TableViewModelItem*> children;
};

class TableViewModel::Private
{
public:

    Private()
      : columnObjects(),
        rootItem(0)
    {
    }

    QList<TableViewColumn*> columnObjects;
    TableViewModelItem* rootItem;
};

TableViewModel::TableViewModel(TableViewShared* const sharedObject, QObject* parent)
  : QAbstractItemModel(parent),
    s(sharedObject),
    d(new Private())
{
    d->rootItem = new TableViewModelItem();

    connect(s->imageFilterModel, SIGNAL(modelAboutToBeReset()),
            this, SLOT(slotSourceModelAboutToBeReset()));
    connect(s->imageFilterModel, SIGNAL(modelReset()),
            this, SLOT(slotSourceModelReset()));
    connect(s->imageFilterModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(slotSourceRowsAboutToBeInserted(QModelIndex,int,int)));
    connect(s->imageFilterModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotSourceRowsInserted(QModelIndex,int,int)));
    connect(s->imageFilterModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(slotSourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(s->imageFilterModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(slotSourceRowsRemoved(QModelIndex,int,int)));
    connect(s->imageFilterModel, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(slotSourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(s->imageFilterModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(slotSourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(s->imageFilterModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(slotSourceLayoutAboutToBeChanged()));
    connect(s->imageFilterModel, SIGNAL(layoutChanged()),
            this, SLOT(slotSourceLayoutChanged()));
    connect(s->imageFilterModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotSourceDataChanged(QModelIndex,QModelIndex)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotDatabaseImageChanged(ImageChangeset)), Qt::QueuedConnection);

    new ModelTest(this, this);

    beginResetModel();
    slotPopulateModel();
    endResetModel();
}

TableViewModel::~TableViewModel()
{
    delete d->rootItem;
}

int TableViewModel::columnCount(const QModelIndex& i) const
{
    return d->columnObjects.count();
}

QModelIndex TableViewModel::toImageFilterModelIndex(const QModelIndex& i) const
{
    TableViewModelItem* const item = itemFromIndex(i);
    if (!item)
    {
        return QModelIndex();
    }

    return item->imageFilterModelIndex;
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
        /// @todo there are no sub-items yet
        return QModelIndex();
    }

    // test for valid row/column values
    if ( (row<0) || (column<0) ||
         (column>=d->columnObjects.count()) || (row>=d->rootItem->children.count())
       )
    {
        return QModelIndex();
    }

    TableViewModelItem* const itemPointer = d->rootItem->children.at(row);
    return createIndex(row, column, itemPointer);
}

QModelIndex TableViewModel::parent(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    /// @todo we only have top level items for now
    return QModelIndex();
}

int TableViewModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return d->rootItem->children.count();
}

QVariant TableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // test for valid input ranges
    if ( (section<0) || (section>=d->columnObjects.count()) )
    {
        return QVariant();
    }

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
    TableViewColumnConfiguration newConfiguration = description.toConfiguration();

    addColumnAt(newConfiguration, targetColumn);
}

void TableViewModel::addColumnAt(const TableViewColumnConfiguration& configuration, const int targetColumn)
{
    TableViewColumn* const newColumn = s->columnFactory->getColumn(configuration);
    if (!newColumn)
    {
        return;
    }

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

    connect(newColumn, SIGNAL(signalAllDataChanged()),
            this, SLOT(slotColumnAllDataChanged()));
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

    const QModelIndex changedIndex = index(sourceIndex.row(), iColumn, QModelIndex());
    emit(dataChanged(changedIndex, changedIndex));
}

void TableViewModel::slotColumnAllDataChanged()
{
    TableViewColumn* const senderColumn = qobject_cast<TableViewColumn*>(sender());

    /// @todo find a faster way to find the column number
    const int iColumn = d->columnObjects.indexOf(senderColumn);
    if (iColumn<0)
    {
        return;
    }

    const QModelIndex changedIndexTopLeft = index(0, iColumn, QModelIndex());
    const QModelIndex changedIndexBottomRight = index(rowCount(QModelIndex())-1, iColumn, QModelIndex());
    emit(dataChanged(changedIndexTopLeft, changedIndexBottomRight));
}

void TableViewModel::removeColumnAt(const int columnIndex)
{
    beginRemoveColumns(QModelIndex(), columnIndex, columnIndex);
    TableViewColumn* const removedColumn = d->columnObjects.takeAt(columnIndex);
    endRemoveColumns();

    delete removedColumn;
}

TableViewColumn* TableViewModel::getColumnObject(const int columnIndex)
{
    /// @todo Debug output to find OSX crash
    if (columnIndex>=d->columnObjects.count())
    {
        kDebug()<<"------ CRASH AHEAD: columnObjects.count(): "<<d->columnObjects.count()<<", columnIndex: "<<columnIndex;
    }
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

void TableViewModel::slotSourceModelAboutToBeReset()
{
    // the source model is about to be reset. Propagate that change:
    beginResetModel();
}

void TableViewModel::slotSourceModelReset()
{
    // the source model is done resetting.
    slotPopulateModel();
    endResetModel();
}

void TableViewModel::slotSourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end)
{
    beginInsertRows(parent, start, end);
}

void TableViewModel::slotSourceRowsInserted(const QModelIndex& parent, int start, int end)
{
    for (int i = start; i<=end; ++i)
    {
        const QModelIndex sourceIndex = s->imageFilterModel->index(i, 0, parent);

        addSourceModelIndex(sourceIndex);
    }

    endInsertRows();
}

void TableViewModel::slotSourceRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    beginRemoveRows(parent, start, end);

    for (int i=start; i<=end; ++i)
    {
        const QModelIndex sourceIndex = s->imageFilterModel->index(i, 0, parent);
        const qlonglong imageId = s->imageFilterModel->imageId(sourceIndex);

        TableViewModelItem* const item = itemFromImageId(imageId);
        item->parent->takeChild(item);

        /// @todo do proper row removing for this item
        delete item;
    }
}

void TableViewModel::slotSourceRowsRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)

    endRemoveRows();
}

void TableViewModel::slotSourceRowsAboutToBeMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd,
                                                  const QModelIndex& destinationParent, int destinationRow)
{
//     beginMoveRows(sourceParent, sourceStart, sourceEnd, destinationParent, destinationRow);

    /// @todo For our items, moving stuff around does not matter
}

void TableViewModel::slotSourceRowsMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd,
                                         const QModelIndex& destinationParent, int destinationRow)
{
    /// @todo For our items, moving stuff around does not matter
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destinationParent)
    Q_UNUSED(destinationRow)

//     endMoveRows();
}

void TableViewModel::slotSourceLayoutAboutToBeChanged()
{
    /// @todo Emitting layoutAboutToBeChanged and layoutChanged is tricky,
    ///       because we do not know what will change.
    ///       It looks like ImageFilterModel emits layoutAboutToBeChanged and layoutChanged
    ///       even when the resulting dataset will be empty, and ModelTest does not like that.
    ///       For now, the easiest workaround is resetting the model
//     emit(layoutAboutToBeChanged());
    beginResetModel();
}

void TableViewModel::slotSourceLayoutChanged()
{
    /// @todo See note in TableViewModel#slotSourceLayoutAboutToBeChanged

    slotPopulateModel();

    endResetModel();
}

void TableViewModel::slotSourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    /// @todo Not sure when ImageFilterModel will emit this signal and whether
    ///       it will be enough to monitor DatabaseAccess::databaseWatch()::imageChange(ImageChangeset)

    const int topRow = topLeft.row();
    const int bottomRow = bottomRight.row();

    const int nColumns = d->columnObjects.count();

    const QModelIndex myTopLeft = index(topRow, 0, QModelIndex());
    const QModelIndex myBottomRight = index(bottomRow, nColumns-1, QModelIndex());

    emit(dataChanged(myTopLeft, myBottomRight));
}

void TableViewModel::slotDatabaseImageChanged(const ImageChangeset& imageChangeset)
{
//     const DatabaseFields::Set changes = imageChangeset.changes();

    /// @todo Decide which changes are relevant here or
    ///       let the TableViewColumn object decide which are relevant

    foreach(const qlonglong& id, imageChangeset.ids())
    {
        const QModelIndex& changedIndex = s->imageFilterModel->indexForImageId(id);
        if (changedIndex.isValid())
        {
            emit(dataChanged(changedIndex, changedIndex));
        }
    }
}

Qt::ItemFlags TableViewModel::flags(const QModelIndex& index) const
{
    const Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    /// @todo Handle read-only files etc. which can not be moved
    if (index.isValid())
    {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    }

    return Qt::ItemIsDropEnabled | defaultFlags;
}

QList< TableViewColumn* > TableViewModel::getColumnObjects()
{
    return d->columnObjects;
}

void TableViewModel::slotPopulateModel()
{
    if (d->rootItem)
    {
        delete d->rootItem;
    }

    d->rootItem = new TableViewModelItem();

    const int sourceRowCount = s->imageFilterModel->rowCount(QModelIndex());
    for (int i=0; i<sourceRowCount; ++i)
    {
        const QModelIndex sourceModelIndex = s->imageFilterModel->index(i, 0);
        addSourceModelIndex(sourceModelIndex);
    }
}

TableViewModelItem* TableViewModel::createItemFromSourceIndex(const QModelIndex& imageFilterModelIndex)
{
    TableViewModelItem* const item = new TableViewModelItem();
    item->imageFilterModelIndex = imageFilterModelIndex;
    item->imageId = s->imageFilterModel->imageId(imageFilterModelIndex);

    return item;
}

void TableViewModel::addSourceModelIndex(const QModelIndex& imageFilterModelIndex)
{
    /// @todo Filter out grouped items here

    TableViewModelItem* item = createItemFromSourceIndex(imageFilterModelIndex);

    d->rootItem->addChild(item);
}

TableViewModelItem* TableViewModel::itemFromImageId(const qlonglong imageId) const
{
    return d->rootItem->findChildWithImageId(imageId);
}

TableViewModelItem* TableViewModel::itemFromIndex(const QModelIndex& i) const
{
    if (!i.isValid())
    {
        return 0;
    }

    Q_ASSERT(i.model()==this);

    TableViewModelItem* const item = static_cast<TableViewModelItem*>(i.internalPointer());

    return item;
}

QModelIndex TableViewModel::fromImageFilterModelIndex(const QModelIndex& imageFilterModelIndex)
{
    const qlonglong imageId = s->imageFilterModel->imageId(imageFilterModelIndex);
    if (!imageId)
    {
        return QModelIndex();
    }

    TableViewModelItem* const item = itemFromImageId(imageId);
    if (!item)
    {
        return QModelIndex();
    }
    TableViewModelItem* const parentItem = item->parent;

    /// @todo This is a waste of time because itemFromImageId already did this search.
    ///       We should modify it to also give the row index.
    const int rowIndex = parentItem->children.indexOf(item);

    return createIndex(rowIndex, 0, item);
}

} /* namespace Digikam */

