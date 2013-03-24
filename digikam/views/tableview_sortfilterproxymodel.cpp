/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-03-11
 * Description : Sort filter proxy model for the table view model
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

#include "tableview_sortfilterproxymodel.moc"

// C++ includes

#include <valarray>

// KDE includes

#include <kdebug.h>

// local includes

/// @todo Clean up include list
#include "albumdb.h"
#include "databaseaccess.h"
#include "databasechangesets.h"
#include "databasefields.h"
#include "databasewatch.h"
#include "imagefiltermodel.h"
#include "imageinfo.h"
#include "imagelister.h"
#include "imageposition.h"
#include "modeltest/modeltest.h"
#include "tableview_columnfactory.h"
#include "tableview_model.h"
#include "tableview_selection_model_syncer.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class TableViewSortFilterProxyModel::Private
{
public:
    Private()
    {
    }

};

TableViewSortFilterProxyModel::TableViewSortFilterProxyModel(TableViewShared* const sPointer, QObject* parent)
  : QSortFilterProxyModel(parent),
    d(new Private()),
    s(sPointer)
{
    setSourceModel(s->tableViewModel);
    setDynamicSortFilter(true);
}

TableViewSortFilterProxyModel::~TableViewSortFilterProxyModel()
{

}

bool TableViewSortFilterProxyModel::lessThan(const QModelIndex& tableViewIndexLeft, const QModelIndex& tableViewIndexRight) const
{
    const int iColumn = tableViewIndexLeft.column();

    const TableViewColumn* columnObject = s->tableViewModel->getColumnObject(iColumn);

    if (!columnObject->getColumnFlags().testFlag(TableViewColumn::ColumnCustomSorting))
    {
        return QSortFilterProxyModel::lessThan(tableViewIndexLeft, tableViewIndexRight);
    }

    TableViewModel::Item* const itemA = s->tableViewModel->itemFromIndex(tableViewIndexLeft);
    TableViewModel::Item* const itemB = s->tableViewModel->itemFromIndex(tableViewIndexRight);
    const ImageInfo infoA = s->tableViewModel->infoFromItem(itemA);
    const ImageInfo infoB = s->tableViewModel->infoFromItem(itemB);

    TableViewColumn::ColumnCompareResult cmpResult = columnObject->compare(itemA, itemB);

    if (cmpResult==TableViewColumn::CmpEqual)
    {
        // compared items are equal, use the image id to enforce a repeatable sorting
        const qlonglong imageIdLeft = itemA->imageId;
        const qlonglong imageIdRight = itemB->imageId;

        return imageIdLeft < imageIdRight;
    }

    return cmpResult == TableViewColumn::CmpALessB;
}

class TableViewCurrentToSortedSyncer::Private
{
public:
    Private()
      : syncing(false)
    {
    }

    bool syncing;
};

TableViewCurrentToSortedSyncer::TableViewCurrentToSortedSyncer(
        TableViewShared* const sharedObject,
        QObject*const parent
    )
  : QObject(parent), d(new Private()), s(sharedObject)
{
    connect(s->sortSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotSortedModelCurrentChanged(QModelIndex,QModelIndex)));

    connect(s->tableViewSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
        this, SLOT(slotTableViewModelCurrentChanged(QModelIndex,QModelIndex)));
}

TableViewCurrentToSortedSyncer::~TableViewCurrentToSortedSyncer()
{

}

void TableViewCurrentToSortedSyncer::slotSortedModelCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous)

    if (d->syncing)
    {
        return;
    }
    d->syncing = true;

    const QModelIndex sortedIndexColumn0 = s->sortModel->index(current.row(), 0, QModelIndex());
    const QModelIndex tableViewColumn0 = s->sortModel->mapToSource(sortedIndexColumn0);

    s->tableViewSelectionModel->setCurrentIndex(tableViewColumn0, QItemSelectionModel::NoUpdate);

    d->syncing = false;
}

void TableViewCurrentToSortedSyncer::slotTableViewModelCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous)

    if (d->syncing)
    {
        return;
    }
    d->syncing = true;

    const QModelIndex tableViewColumn0 = s->tableViewModel->index(current.row(), 0, QModelIndex());
    const QModelIndex sortedIndexColumn0 = s->sortModel->mapFromSource(tableViewColumn0);

    s->sortSelectionModel->setCurrentIndex(sortedIndexColumn0, QItemSelectionModel::NoUpdate);

    d->syncing = false;
}

QModelIndex TableViewSortFilterProxyModel::toImageFilterModelIndex(const QModelIndex& index) const
{
    // "index" is a sortModel index. We have to map it to the TableViewModel:
    const QModelIndex tableViewModelIndex = mapToSource(index);

    // Map to ImageFilterModel:
    const QModelIndex imageFilterModelIndex = s->tableViewModel->toImageFilterModelIndex(tableViewModelIndex);

    return imageFilterModelIndex;
}


QModelIndex TableViewSortFilterProxyModel::toImageModelIndex(const QModelIndex& index) const
{
    const QModelIndex imageFilterModelIndex = toImageFilterModelIndex(index);

    // map to the source of ImageFilterModel: ImageModel
    const QModelIndex imageModelIndex = s->imageFilterModel->mapToSourceImageModel(imageFilterModelIndex);

    return imageModelIndex;
}

QMimeData* TableViewSortFilterProxyModel::mimeData(const QModelIndexList& indexes) const
{
    // we pack the mime data via ImageModel's drag-drop handler
    AbstractItemDragDropHandler* const ddHandler = s->imageModel->dragDropHandler();

    QModelIndexList imageModelIndexList;
    Q_FOREACH(const QModelIndex& i, indexes)
    {
        if (i.column()>0)
        {
            continue;
        }

        const QModelIndex imageModelIndex = toImageModelIndex(i);
        if (imageModelIndex.isValid())
        {
            imageModelIndexList << imageModelIndex;
        }
    }

    QMimeData* const imageModelMimeData = ddHandler->createMimeData(imageModelIndexList);

    return imageModelMimeData;
}

Qt::DropActions TableViewSortFilterProxyModel::supportedDropActions() const
{
    return Qt::CopyAction|Qt::MoveAction;
}

QStringList TableViewSortFilterProxyModel::mimeTypes() const
{
    AbstractItemDragDropHandler* const ddHandler = s->imageModel->dragDropHandler();

    if (ddHandler)
    {
        return ddHandler->mimeTypes();
    }

    return QStringList();
}

bool TableViewSortFilterProxyModel::dropMimeData(
        const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    Q_UNUSED(data)
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)

    // the drop is handled by the drag-drop handler, therefore we return false here
    return false;
}

} /* namespace Digikam */

