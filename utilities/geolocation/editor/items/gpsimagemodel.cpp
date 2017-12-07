/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-21
 * Description : A model to hold information about images.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Michael G. Hansen <mike at mghansen dot de>
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

#include "gpsimagemodel.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class GPSImageModel::Private
{
public:

    Private()
      : items(),
        columnCount(0),
        thumbnailLoadThread(0)
    {
    }

    QList<GPSImageItem*>            items;
    int                             columnCount;
    QMap<QPair<int, int>, QVariant> headerData;
    ThumbnailLoadThread*            thumbnailLoadThread;
};

GPSImageModel::GPSImageModel(QObject* const parent)
    : QAbstractItemModel(parent),
      d(new Private)
{
    d->thumbnailLoadThread = new ThumbnailLoadThread(this);

    connect(d->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription, QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription, QPixmap)));
}

GPSImageModel::~GPSImageModel()
{
    // TODO: send a signal before deleting the items?
    qDeleteAll(d->items);
    delete d;
}

int GPSImageModel::columnCount(const QModelIndex& /*parent*/) const
{
    return d->columnCount;
}

QVariant GPSImageModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        Q_ASSERT(index.model() == this);
    }

    const int rowNumber = index.row();

    if ((rowNumber < 0) || (rowNumber >= d->items.count()))
    {
        return QVariant();
    }

    return d->items.at(rowNumber)->data(index.column(), role);
}

QModelIndex GPSImageModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        Q_ASSERT(parent.model() == this);
    }

//     qCDebug(DIGIKAM_GENERAL_LOG)<<row<<column<<parent;

    if (parent.isValid())
    {
        // there are no child items, only top level items
        return QModelIndex();
    }

    if ( (column < 0)               ||
         (column >= d->columnCount) ||
         (row < 0)                  ||
         (row >= d->items.count())
       )
        return QModelIndex();

    return createIndex(row, column, (void*)0);
}

QModelIndex GPSImageModel::parent(const QModelIndex& /*index*/) const
{
    // we have only top level items
    return QModelIndex();
}

void GPSImageModel::addItem(GPSImageItem* const newItem)
{
    beginInsertRows(QModelIndex(), d->items.count(), d->items.count());
    newItem->setModel(this);
    d->items << newItem;
    endInsertRows();
}

void GPSImageModel::setColumnCount(const int nColumns)
{
    emit(layoutAboutToBeChanged());

    d->columnCount = nColumns;

    emit(layoutChanged());
}

void GPSImageModel::itemChanged(GPSImageItem* const changedItem)
{
    const int itemIndex = d->items.indexOf(changedItem);

    if (itemIndex < 0)
        return;

    const QModelIndex itemModelIndexStart = createIndex(itemIndex, 0, (void*)0);
    const QModelIndex itemModelIndexEnd   = createIndex(itemIndex, d->columnCount - 1, (void*)0);

    emit(dataChanged(itemModelIndexStart, itemModelIndexEnd));
}

GPSImageItem* GPSImageModel::itemFromIndex(const QModelIndex& index) const
{
    if (index.isValid())
    {
        Q_ASSERT(index.model() == this);
    }

    if (!index.isValid())
        return 0;

    const int row = index.row();

    if ((row < 0) || (row >= d->items.count()))
        return 0;

    return d->items.at(row);
}

int GPSImageModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        Q_ASSERT(parent.model() == this);
    }

    if (parent.isValid())
        return 0;

    return d->items.count();
}

bool GPSImageModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
    if ((section >= d->columnCount) || (orientation != Qt::Horizontal))
        return false;

    const QPair<int, int> headerIndex = QPair<int, int>(section, role);
    d->headerData[headerIndex]        = value;

    return true;
}

QVariant GPSImageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((section >= d->columnCount) || (orientation != Qt::Horizontal))
        return false;

    const QPair<int, int> headerIndex = QPair<int, int>(section, role);

    return d->headerData.value(headerIndex);
}

bool GPSImageModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);

    return false;
}

Qt::ItemFlags GPSImageModel::flags(const QModelIndex& index) const
{
    if (index.isValid())
    {
        Q_ASSERT(index.model() == this);
    }

    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
}

GPSImageItem* GPSImageModel::itemFromUrl(const QUrl& url) const
{
    for (int i = 0 ; i < d->items.count() ; ++i)
    {
        if (d->items.at(i)->url() == url)
            return d->items.at(i);
    }

    return 0;
}

QModelIndex GPSImageModel::indexFromUrl(const QUrl& url) const
{
    for (int i = 0 ; i < d->items.count() ; ++i)
    {
        if (d->items.at(i)->url() == url)
            return index(i, 0, QModelIndex());
    }

    return QModelIndex();
}

QPixmap GPSImageModel::getPixmapForIndex(const QPersistentModelIndex& itemIndex, const int size)
{
    if (itemIndex.isValid())
    {
        Q_ASSERT(itemIndex.model() == this);
    }

    // TODO: should we cache the pixmap on our own here or does the interface usually cache it for us?
    // TODO: do we need to make sure we do not request the same pixmap twice in a row?
    // construct the key under which we stored the pixmap in the cache:
    GPSImageItem* const imageItem = itemFromIndex(itemIndex);

    if (!imageItem)
        return QPixmap();

    QPixmap thumbnail;

    if (d->thumbnailLoadThread->find(ThumbnailIdentifier(imageItem->url().toLocalFile()), thumbnail, size))
    {
        return thumbnail.copy(1, 1, thumbnail.size().width()-2, thumbnail.size().height()-2);
    }

    return QPixmap();
}

void GPSImageModel::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb)
{
    if (thumb.isNull())
    {
        return;
    }

    const QModelIndex currentIndex = indexFromUrl(QUrl::fromLocalFile(loadingDescription.filePath));

    if (currentIndex.isValid())
    {
        QPersistentModelIndex goodIndex(currentIndex);
        emit(signalThumbnailForIndexAvailable(goodIndex, thumb.copy(1, 1, thumb.size().width()-2, thumb.size().height()-2)));
    }
}

Qt::DropActions GPSImageModel::supportedDragActions() const
{
    return Qt::CopyAction;
}

} // namespace Digikam
