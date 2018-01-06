/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-09
 * Description : DTrash item info model
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "dtrashitemmodel.h"

// Qt includes

#include <QPixmap>
#include <QPersistentModelIndex>
#include <QTimer>

// KDE includes

#include "klocalizedstring.h"

// Local includes

#include "digikam_debug.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "iojobsmanager.h"

namespace Digikam
{

class DTrashItemModel::Private
{

public:

    Private() :
        thumbSize(ThumbnailSize::Large),
        sortColumn(2),
        sortOrder(Qt::DescendingOrder),
        itemsLoadingThread(0),
        thumbnailThread(0),
        timer(0)
    {
    }

public:

    int                  thumbSize;
    int                  sortColumn;
    Qt::SortOrder        sortOrder;
    IOJobsThread*        itemsLoadingThread;
    ThumbnailLoadThread* thumbnailThread;
    QTimer*              timer;
    DTrashItemInfoList   data;
};

DTrashItemModel::DTrashItemModel(QObject* parent)
    : QAbstractTableModel(parent), d(new Private)
{
    qRegisterMetaType<DTrashItemInfo>("DTrashItemInfo");
    d->thumbnailThread = ThumbnailLoadThread::defaultThread();

    d->timer = new QTimer();
    d->timer->setInterval(100);
    d->timer->setSingleShot(true);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(refreshLayout()));
}

DTrashItemModel::~DTrashItemModel()
{
    delete d;
}

int DTrashItemModel::rowCount(const QModelIndex&) const
{
    return d->data.count();
}

int DTrashItemModel::columnCount(const QModelIndex&) const
{
    return 3;
}

QVariant DTrashItemModel::data(const QModelIndex& index, int role) const
{
    if ( role != Qt::DisplayRole &&
         role != Qt::DecorationRole &&
         role != Qt::TextAlignmentRole &&
         role != Qt::ToolTipRole)
        return QVariant();

    const DTrashItemInfo& item = d->data[index.row()];

    if (role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;

    if (role == Qt::DecorationRole && index.column() == 0)
    {
        QPixmap pix;

        if (pixmapForItem(item.trashPath, pix))
        {
            return pix;
        }
        else
        {
            return QVariant(QVariant::Pixmap);
        }
    }

    if (role == Qt::ToolTipRole && index.column() == 1)
        return item.collectionRelativePath;

    switch (index.column())
    {
        case 1: return item.collectionRelativePath;
        case 2: return item.deletionTimestamp.toString();
        default: return QVariant();
    };
}

void DTrashItemModel::sort(int column, Qt::SortOrder order)
{
    d->sortColumn = column;
    d->sortOrder  = order;

    if (d->data.count() < 2)
    {
        return;
    }

    std::sort(d->data.begin(), d->data.end(),
                [column, order](const DTrashItemInfo& a, const DTrashItemInfo& b)
                {
                    if (column == 2 && a.deletionTimestamp != b.deletionTimestamp)
                    {
                        if (order == Qt::DescendingOrder)
                        {
                            return a.deletionTimestamp > b.deletionTimestamp;
                        }
                        else
                        {
                            return a.deletionTimestamp < b.deletionTimestamp;
                        }
                    }

                    if (order == Qt::DescendingOrder)
                    {
                        return a.collectionRelativePath > b.collectionRelativePath;
                    }

                    return a.collectionRelativePath < b.collectionRelativePath;
                });

    const QModelIndex topLeft     = index(0, 0, QModelIndex());
    const QModelIndex bottomRight = index(rowCount(QModelIndex())-1,
                                          columnCount(QModelIndex())-1, QModelIndex());
    dataChanged(topLeft, bottomRight);
}

bool DTrashItemModel::pixmapForItem(const QString& path, QPixmap& pix) const
{
    return d->thumbnailThread->find(ThumbnailIdentifier(path), pix, d->thumbSize);
}

QVariant DTrashItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (section)
    {
        case 0: return i18n("Thumbnail");
        case 1: return i18n("Relative Path");
        case 2: return i18n("Deletion Time");
        default: return QVariant();
    }
}

void DTrashItemModel::append(const DTrashItemInfo& itemInfo)
{
    if (d->itemsLoadingThread != sender())
        return;

    beginInsertRows(QModelIndex(), d->data.count(), d->data.count());
    d->data.append(itemInfo);
    endInsertRows();

    sort(d->sortColumn, d->sortOrder);
    emit dataChange();
}

void DTrashItemModel::removeItems(const QModelIndexList& indexes)
{
    QList<QPersistentModelIndex> persistentIndexes;

    foreach (const QModelIndex& index, indexes)
    {
        persistentIndexes << index;
    }

    layoutAboutToBeChanged();

    foreach (const QPersistentModelIndex& index, persistentIndexes)
    {
        if (!index.isValid())
            continue;

        beginRemoveRows(QModelIndex(), index.row(), index.row());
        removeRow(index.row());
        d->data.removeAt(index.row());
        endRemoveRows();
    }

    layoutChanged();
    emit dataChange();
}

void DTrashItemModel::refreshLayout()
{
    layoutAboutToBeChanged();
    layoutChanged();
}

void DTrashItemModel::clearCurrentData()
{
    beginResetModel();
    d->data.clear();
    endResetModel();
    emit dataChange();
}

void DTrashItemModel::loadItemsForCollection(const QString& colPath)
{
    clearCurrentData();

    d->itemsLoadingThread =
            IOJobsManager::instance()->startDTrashItemsListingForCollection(colPath);

    connect(d->itemsLoadingThread, SIGNAL(collectionTrashItemInfo(DTrashItemInfo)),
            this, SLOT(append(DTrashItemInfo)),
            Qt::QueuedConnection);
}

DTrashItemInfo DTrashItemModel::itemForIndex(const QModelIndex& index)
{
    if (!index.isValid())
        return DTrashItemInfo();

    return d->data.at(index.row());
}

DTrashItemInfoList DTrashItemModel::itemsForIndexes(const QList<QModelIndex>& indexes)
{
    DTrashItemInfoList items;

    foreach (const QModelIndex& index, indexes)
    {
        if (!index.isValid())
            continue;

        items << itemForIndex(index);
    }

    return items;
}

DTrashItemInfoList DTrashItemModel::allItems()
{
    return d->data;
}

bool DTrashItemModel::isEmpty()
{
    return d->data.isEmpty();
}

void DTrashItemModel::changeThumbSize(int size)
{
    d->thumbSize = size;

    if (isEmpty())
        return;

    const QModelIndex topLeft     = index(0, 0, QModelIndex());
    const QModelIndex bottomRight = index(rowCount(QModelIndex())-1, 0, QModelIndex());
    dataChanged(topLeft, bottomRight);

    d->timer->start();
}

} // namespace Digikam
