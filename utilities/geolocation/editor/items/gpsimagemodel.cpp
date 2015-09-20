/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-03-21
 * @brief  A model to hold information about images.
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "gpsimagemodel.h"

// KDE includes

#include <kimagecache.h>

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
        pixmapCache(0)
    {
    }

    QList<GPSImageItem*>                     items;
    int                                       columnCount;
    QMap<QPair<int, int>, QVariant>           headerData;
    KImageCache*                              pixmapCache;
    QList<QPair<QPersistentModelIndex, int> > requestedPixmaps;
};

GPSImageModel::GPSImageModel(QObject* const parent)
    : QAbstractItemModel(parent),
      d(new Private)
{
    // TODO: Find an appropriate name (is "digikam-geolocator" OK?)
    // TODO: Make cache size configurable.
    d->pixmapCache = new KImageCache(QStringLiteral("digikam-geolocator"), 5 * 1024 * 1024);
    d->pixmapCache->setPixmapCaching(false);
}

GPSImageModel::~GPSImageModel()
{
    // TODO: send a signal before deleting the items?
    qDeleteAll(d->items);
    delete d->pixmapCache;
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
        Q_ASSERT(index.model()==this);
    }

    const int rowNumber = index.row();

    if ((rowNumber<0)||(rowNumber>=d->items.count()))
    {
        return QVariant();
    }

    return d->items.at(rowNumber)->data(index.column(), role);
}

QModelIndex GPSImageModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        Q_ASSERT(parent.model()==this);
    }

//     qCDebug(DIGIKAM_GENERAL_LOG)<<row<<column<<parent;

    if (parent.isValid())
    {
        // there are no child items, only top level items
        return QModelIndex();
    }

    if ( (column<0) || (column>=d->columnCount) ||
         (row<0) || (row>=d->items.count()) )
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

    if (itemIndex<0)
        return;

    const QModelIndex itemModelIndexStart = createIndex(itemIndex, 0, (void*)0);
    const QModelIndex itemModelIndexEnd   = createIndex(itemIndex, d->columnCount - 1, (void*)0);
    emit(dataChanged(itemModelIndexStart, itemModelIndexEnd));
}

GPSImageItem* GPSImageModel::itemFromIndex(const QModelIndex& index) const
{
    if (index.isValid())
    {
        Q_ASSERT(index.model()==this);
    }

    if (!index.isValid())
        return 0;

    const int row = index.row();

    if ((row<0)||(row>=d->items.count()))
        return 0;

    return d->items.at(row);
}

int GPSImageModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        Q_ASSERT(parent.model()==this);
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
        Q_ASSERT(index.model()==this);
    }

    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
}

GPSImageItem* GPSImageModel::itemFromUrl(const QUrl& url) const
{
    for (int i=0; i<d->items.count(); ++i)
    {
        if (d->items.at(i)->url()==url)
            return d->items.at(i);
    }

    return 0;
}

QModelIndex GPSImageModel::indexFromUrl(const QUrl& url) const
{
    for (int i=0; i<d->items.count(); ++i)
    {
        if (d->items.at(i)->url()==url)
            return index(i, 0, QModelIndex());
    }

    return QModelIndex();
}

static QString CacheKeyFromSizeAndUrl(const int size, const QUrl& url)
{
    return QStringLiteral("%1-%3").arg(size).arg(url.url(QUrl::PreferLocalFile));
}

QPixmap GPSImageModel::getPixmapForIndex(const QPersistentModelIndex& itemIndex, const int size)
{
    if (itemIndex.isValid())
    {
        Q_ASSERT(itemIndex.model()==this);
    }

    // TODO: should we cache the pixmap on our own here or does the interface usually cache it for us?
    // TODO: do we need to make sure we do not request the same pixmap twice in a row?
    // construct the key under which we stored the pixmap in the cache:
    GPSImageItem* const imageItem = itemFromIndex(itemIndex);

    if (!imageItem)
        return QPixmap();

    const QString itemKeyString  = CacheKeyFromSizeAndUrl(size, imageItem->url());
    QPixmap thumbnailPixmap;
    const bool havePixmapInCache = d->pixmapCache->findPixmap(itemKeyString, &thumbnailPixmap);
//     qCDebug(DIGIKAM_GENERAL_LOG)<<imageItem->url()<<size<<havePixmapInCache<<d->pixmapCache->isEnabled();

    if (havePixmapInCache)
        return thumbnailPixmap;

    // did we already request this pixmap at this size?
    for (int i=0; i<d->requestedPixmaps.count(); ++i)
    {
        if (d->requestedPixmaps.at(i).first==itemIndex)
        {
            if (d->requestedPixmaps.at(i).second==size)
            {
                // the pixmap has already been requested, at this size
                return QPixmap();
            }
        }
    }

    // remember at which size the pixmap was ordered:
    d->requestedPixmaps << QPair<QPersistentModelIndex, int>(itemIndex, size);

/* FIXME : port to ThumbLoadThread

    if (d->interface)
    {
        d->interface->thumbnails(QList<QUrl>()<<imageItem->url(), size);
    }
    else
    {
        KIO::PreviewJob *job = KIO::filePreview(urls, DEFAULTSIZE);

        connect(job, SIGNAL(gotPreview(KFileItem,QPixmap)),
                this, SLOT(slotKDEPreview(KFileItem,QPixmap)));

        connect(job, SIGNAL(failed(KFileItem)),
                this, SLOT(slotKDEPreviewFailed(KFileItem)));
    }
*/
    return QPixmap();
}

//FIXME : port to ThumbLoadThread
void GPSImageModel::slotThumbnailFromInterface(const QUrl& url, const QPixmap& pixmap)
{
    qCDebug(DIGIKAM_GENERAL_LOG)<<url<<pixmap.size();

    if (pixmap.isNull())
        return;

    const int effectiveSize = qMax(pixmap.size().width(), pixmap.size().height());

    // find the item corresponding to the URL:
    const QModelIndex imageIndex = indexFromUrl(url);
    qCDebug(DIGIKAM_GENERAL_LOG)<<url<<imageIndex.isValid();

    if (imageIndex.isValid())
    {
        // this is tricky: some kipi interfaces return pixmaps at the requested size, others do not.
        // therefore we check whether a pixmap of this size has been requested. If so, we send it on.
        // If a pixmap of this size has not been requested, we rescale it to fulfill all other requests.

        // index, size
        QList<QPair<int, int> > openRequests;

        for (int i=0; i<d->requestedPixmaps.count(); ++i)
        {
            if (d->requestedPixmaps.at(i).first==imageIndex)
            {
                const int requestedSize = d->requestedPixmaps.at(i).second;

                if (requestedSize==effectiveSize)
                {
                    // match, send it out.
                    d->requestedPixmaps.removeAt(i);
                    qCDebug(DIGIKAM_GENERAL_LOG)<<i;

                    // save the pixmap:
                    const QString itemKeyString = CacheKeyFromSizeAndUrl(effectiveSize, url);
                    d->pixmapCache->insertPixmap(itemKeyString, pixmap);

                    emit(signalThumbnailForIndexAvailable(imageIndex, pixmap));
                    return;
                }
                else
                {
                    openRequests << QPair<int, int>(i, requestedSize);
                }
            }
        }

        // the pixmap was not requested at this size, fulfill all requests:
        for (int i=openRequests.count()-1; i>=0; --i)
        {
            const int targetSize = openRequests.at(i).second;
            d->requestedPixmaps.removeAt(openRequests.at(i).first);
            qCDebug(DIGIKAM_GENERAL_LOG)<<i<<targetSize;

            QPixmap scaledPixmap = pixmap.scaled(targetSize, targetSize, Qt::KeepAspectRatio);

            // save the pixmap:
            const QString itemKeyString = CacheKeyFromSizeAndUrl(targetSize, url);
            d->pixmapCache->insertPixmap(itemKeyString, scaledPixmap);

            emit(signalThumbnailForIndexAvailable(imageIndex, scaledPixmap));
        }
    }
}

Qt::DropActions GPSImageModel::supportedDragActions() const
{
    return Qt::CopyAction;
}

} /* Digikam */
