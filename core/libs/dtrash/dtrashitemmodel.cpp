/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-09
 * Description : DTrash item info model
 *
 * Copyright (C) 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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

#include <QIcon>
#include <QTimer>
#include <QPixmap>
#include <QMimeType>
#include <QMimeDatabase>
#include <QPersistentModelIndex>

// KDE includes

#include "klocalizedstring.h"

// Local includes

#include "digikam_debug.h"
#include "thumbnailsize.h"
#include "iojobsmanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN DTrashItemModel::Private
{

public:

    explicit Private()
      : thumbSize(ThumbnailSize::Large),
        sortColumn(2),
        sortOrder(Qt::DescendingOrder),
        itemsLoadingThread(0),
        thumbnailThread(0)
    {
    }

public:

    int                  thumbSize;
    int                  sortColumn;

    Qt::SortOrder        sortOrder;

    IOJobsThread*        itemsLoadingThread;
    ThumbnailLoadThread* thumbnailThread;

    QList<QString>       failedThumbnails;
    DTrashItemInfoList   data;
};

DTrashItemModel::DTrashItemModel(QObject* const parent)
    : QAbstractTableModel(parent),
      d(new Private)
{
    qRegisterMetaType<DTrashItemInfo>("DTrashItemInfo");
    d->thumbnailThread = new ThumbnailLoadThread;
    d->thumbnailThread->setSendSurrogatePixmap(false);

    connect(d->thumbnailThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(refreshThumbnails(LoadingDescription,QPixmap)));
}

DTrashItemModel::~DTrashItemModel()
{
    delete d->thumbnailThread;
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
    if ( role != Qt::DisplayRole       &&
         role != Qt::DecorationRole    &&
         role != Qt::TextAlignmentRole &&
         role != Qt::ToolTipRole)
        return QVariant();

    const DTrashItemInfo& item = d->data[index.row()];

    if (role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;

    if (role == Qt::DecorationRole && index.column() == 0)
    {
        QPixmap pix;
        QString thumbPath;

        if (!d->failedThumbnails.contains(item.collectionPath))
        {
            thumbPath = item.collectionPath;
        }
        else
        {
            thumbPath = item.trashPath;
        }

        if (pixmapForItem(thumbPath, pix))
        {
            if (pix.isNull())
            {
                QMimeType mimeType = QMimeDatabase().mimeTypeForFile(item.trashPath);

                if (mimeType.isValid())
                {
                    pix = QIcon::fromTheme(mimeType.genericIconName()).pixmap(128);
                }
            }

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
        case 1:
            return item.collectionRelativePath;
        case 2:
        {
            QString dateTimeFormat = QLocale().dateTimeFormat();

            if (!dateTimeFormat.contains(QLatin1String("yyyy")))
            {
                dateTimeFormat.replace(QLatin1String("yy"),
                                       QLatin1String("yyyy"));
            }

            return item.deletionTimestamp.toString(dateTimeFormat);
        }
        default:
            return QVariant();
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

    const QModelIndex topLeft     = index(0, 0);
    const QModelIndex bottomRight = index(rowCount(QModelIndex())-1,
                                          columnCount(QModelIndex())-1);
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
        case 0:
            return i18n("Thumbnail");
        case 1:
            return i18n("Relative Path");
        case 2:
            return i18n("Deletion Time");
        default:
            return QVariant();
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

        const DTrashItemInfo& item = d->data[index.row()];

        d->failedThumbnails.removeAll(item.collectionPath);
        d->failedThumbnails.removeAll(item.trashPath);

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
    const QModelIndex topLeft     = index(0, 0);
    const QModelIndex bottomRight = index(rowCount(QModelIndex())-1, 0);
    dataChanged(topLeft, bottomRight);
    layoutAboutToBeChanged();
    layoutChanged();
}

void DTrashItemModel::refreshThumbnails(const LoadingDescription& desc, const QPixmap& pix)
{
    if (pix.isNull())
    {
        if (!d->failedThumbnails.contains(desc.filePath))
        {
            d->failedThumbnails << desc.filePath;
        }
    }

    const QModelIndex topLeft     = index(0, 0);
    const QModelIndex bottomRight = index(rowCount(QModelIndex())-1, 0);
    dataChanged(topLeft, bottomRight);
}

void DTrashItemModel::clearCurrentData()
{
    d->failedThumbnails.clear();
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

QModelIndex DTrashItemModel::indexForItem(const DTrashItemInfo& itemInfo) const
{
    int index = d->data.indexOf(itemInfo);

    if (index != -1)
    {
        return createIndex(index, 0);
    }

    return QModelIndex();
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

    QTimer::singleShot(100, this, SLOT(refreshLayout()));
}

} // namespace Digikam
