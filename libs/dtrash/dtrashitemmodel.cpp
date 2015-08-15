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

namespace Digikam
{

DTrashItemModel::DTrashItemModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    qRegisterMetaType<DTrashItemInfo>("DTrashItemInfo");
    m_thumbnailThread = new ThumbnailLoadThread(this);
    m_thumbSize = ThumbnailSize::Large;
    m_timer = new QTimer();
    m_timer->setInterval(100);
    m_timer->setSingleShot(true);

    connect(m_timer, SIGNAL(timeout()),
            this, SLOT(refreshLayout()));
}

DTrashItemModel::~DTrashItemModel()
{
    m_thumbnailThread->cleanUp();
    delete m_thumbnailThread;
}

QVariant DTrashItemModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::DecorationRole && role != Qt::TextAlignmentRole)
        return QVariant();

    const DTrashItemInfo& item = m_data[index.row()];

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

    switch (index.column())
    {
        case 1: return item.collectionRelativePath;
        case 2: return item.deletionTimestamp.toString();
        default: return QVariant();
    };
}

bool DTrashItemModel::pixmapForItem(const QString &path, QPixmap &pix) const
{
    return m_thumbnailThread->find(ThumbnailIdentifier(path), pix, m_thumbSize);
}

QVariant DTrashItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) return QVariant();
    if (role != Qt::DisplayRole) return QVariant();

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
    beginInsertRows(QModelIndex(), m_data.count(), m_data.count());
    m_data.append(itemInfo);
    endInsertRows();
    dataChange();
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
        m_data.removeAt(index.row());
        endRemoveRows();
    }

    layoutChanged();
    dataChange();
}

void DTrashItemModel::refreshLayout()
{
    layoutAboutToBeChanged();
    layoutChanged();
}

void DTrashItemModel::clearCurrentData()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
    dataChange();
}

DTrashItemInfoList DTrashItemModel::itemsForIndexes(QList<QModelIndex> indexes)
{
    DTrashItemInfoList items;

    foreach (const QModelIndex& index, indexes)
    {
        if (!index.isValid())
            continue;

        items << m_data.at(index.row());
    }

    return items;
}

DTrashItemInfoList DTrashItemModel::allItems()
{
    return m_data;
}

bool DTrashItemModel::isEmpty()
{
    return m_data.isEmpty();
}

void DTrashItemModel::changeThumbSize(int size)
{
    m_thumbSize = size;

    if (isEmpty())
        return;

    const QModelIndex topLeft = index(0, 0, QModelIndex());
    const QModelIndex bottomRight = index(rowCount(QModelIndex())-1, 0, QModelIndex());
    dataChanged(topLeft, bottomRight);

    m_timer->start();
}

} // namespace Digikam
