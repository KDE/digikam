/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries with support for thumbnail loading
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemthumbnailmodel.h"

// Qt includes

#include <QHash>
#include <QApplication>
#include <QDesktopWidget>

// Local includes

#include "digikam_debug.h"
#include "digikam_export.h"
#include "digikam_globals.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemThumbnailModel::Private
{
public:

    explicit Private()
      : thread(nullptr),
        preloadThread(nullptr),
        thumbSize(0),
        lastGlobalThumbSize(0),
        preloadThumbSize(0),
        emitDataChanged(true)
    {
        staticListContainingThumbnailRole << ItemModel::ThumbnailRole;
    }

    ThumbnailLoadThread*   thread;
    ThumbnailLoadThread*   preloadThread;
    ThumbnailSize          thumbSize;
    ThumbnailSize          lastGlobalThumbSize;
    ThumbnailSize          preloadThumbSize;
    QRect                  detailRect;
    QVector<int>           staticListContainingThumbnailRole;

    bool                   emitDataChanged;

    int preloadThumbnailSize() const
    {
        if (preloadThumbSize.size())
        {
            return preloadThumbSize.size();
        }

        return thumbSize.size();
    }
};

ItemThumbnailModel::ItemThumbnailModel(QObject* const parent)
    : ItemModel(parent),
      d(new Private)
{
    setKeepsFilePathCache(true);
}

ItemThumbnailModel::~ItemThumbnailModel()
{
    delete d->preloadThread;
    delete d;
}

void ItemThumbnailModel::setThumbnailLoadThread(ThumbnailLoadThread* const thread)
{
    d->thread = thread;

    connect(d->thread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));
}

ThumbnailLoadThread* ItemThumbnailModel::thumbnailLoadThread() const
{
    return d->thread;
}

ThumbnailSize ItemThumbnailModel::thumbnailSize() const
{
    return d->thumbSize;
}

void ItemThumbnailModel::setThumbnailSize(const ThumbnailSize& size)
{
    d->lastGlobalThumbSize = size;
    d->thumbSize = size;
}

void ItemThumbnailModel::setPreloadThumbnailSize(const ThumbnailSize& size)
{
    d->preloadThumbSize = size;
}

void ItemThumbnailModel::setEmitDataChanged(bool emitSignal)
{
    d->emitDataChanged = emitSignal;
}

void ItemThumbnailModel::setPreloadThumbnails(bool preload)
{
    if (preload)
    {
        if (!d->preloadThread)
        {
            d->preloadThread = new ThumbnailLoadThread;
            d->preloadThread->setPixmapRequested(false);
            d->preloadThread->setPriority(QThread::LowestPriority);
        }

        connect(this, SIGNAL(allRefreshingFinished()),
                this, SLOT(preloadAllThumbnails()));
    }
    else
    {
        delete d->preloadThread;
        d->preloadThread = nullptr;

        disconnect(this, SIGNAL(allRefreshingFinished()),
                   this, SLOT(preloadAllThumbnails()));
    }
}

void ItemThumbnailModel::prepareThumbnails(const QList<QModelIndex>& indexesToPrepare)
{
    prepareThumbnails(indexesToPrepare, d->thumbSize);
}

void ItemThumbnailModel::prepareThumbnails(const QList<QModelIndex>& indexesToPrepare, const ThumbnailSize& thumbSize)
{
    if (!d->thread)
    {
        return;
    }

    QList<ThumbnailIdentifier> ids;

    foreach (const QModelIndex& index, indexesToPrepare)
    {
        ids << imageInfoRef(index).thumbnailIdentifier();
    }

    d->thread->findGroup(ids, thumbSize.size());
}

void ItemThumbnailModel::preloadThumbnails(const QList<ItemInfo>& infos)
{
    if (!d->preloadThread)
    {
        return;
    }

    QList<ThumbnailIdentifier> ids;

    foreach (const ItemInfo& info, infos)
    {
        ids << info.thumbnailIdentifier();
    }

    d->preloadThread->pregenerateGroup(ids, d->preloadThumbnailSize());
}

void ItemThumbnailModel::preloadThumbnails(const QList<QModelIndex>& indexesToPreload)
{
    if (!d->preloadThread)
    {
        return;
    }

    QList<ThumbnailIdentifier> ids;

    foreach (const QModelIndex& index, indexesToPreload)
    {
        ids << imageInfoRef(index).thumbnailIdentifier();
    }

    d->preloadThread->stopAllTasks();
    d->preloadThread->pregenerateGroup(ids, d->preloadThumbnailSize());
}

void ItemThumbnailModel::preloadAllThumbnails()
{
    preloadThumbnails(imageInfos());
}

void ItemThumbnailModel::imageInfosCleared()
{
    if (d->preloadThread)
    {
        d->preloadThread->stopAllTasks();
    }
}

QVariant ItemThumbnailModel::data(const QModelIndex& index, int role) const
{
    if (role == ThumbnailRole && d->thread && index.isValid())
    {
        QPixmap   thumbnail;
        ItemInfo info = imageInfo(index);

        if (info.isNull())
        {
            return QVariant(QVariant::Pixmap);
        }

        double ratio  = QApplication::desktop()->devicePixelRatioF();
        int thumbSize = qMin(qRound((double)d->thumbSize.size() * ratio), (int)ThumbnailSize::HD);

        if (!d->detailRect.isNull())
        {
            if (d->thread->find(info.thumbnailIdentifier(), d->detailRect, thumbnail, thumbSize))
            {
                return thumbnail;
            }
        }
        else
        {
            if (d->thread->find(info.thumbnailIdentifier(), thumbnail, thumbSize))
            {
                return thumbnail;
            }
        }

        return QVariant(QVariant::Pixmap);
    }

    return ItemModel::data(index, role);
}

bool ItemThumbnailModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == ThumbnailRole)
    {
        switch (value.type())
        {
            case QVariant::Invalid:
                d->thumbSize  = d->lastGlobalThumbSize;
                d->detailRect = QRect();
                break;

            case QVariant::Int:

                if (value.isNull())
                {
                    d->thumbSize = d->lastGlobalThumbSize;
                }
                else
                {
                    d->thumbSize = ThumbnailSize(value.toInt());
                }
                break;

            case QVariant::Rect:

                if (value.isNull())
                {
                    d->detailRect = QRect();
                }
                else
                {
                    d->detailRect = value.toRect();
                }
                break;

            default:
                break;
        }
    }

    return ItemModel::setData(index, value, role);
}

void ItemThumbnailModel::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb)
{
    if (thumb.isNull())
    {
        return;
    }

    // In case of multiple occurrence, we currently do not know which thumbnail is this. Signal change on all.
    QModelIndexList indexes;
    ThumbnailIdentifier thumbId = loadingDescription.thumbnailIdentifier();

    if (thumbId.filePath.isEmpty())
    {
        indexes = indexesForImageId(thumbId.id);
    }
    else
    {
        indexes = indexesForPath(thumbId.filePath);
    }

    foreach (const QModelIndex& index, indexes)
    {
        if (thumb.isNull())
        {
            emit thumbnailFailed(index, loadingDescription.previewParameters.size);
        }
        else
        {
            emit thumbnailAvailable(index, loadingDescription.previewParameters.size);

            if (d->emitDataChanged)
            {
                emit dataChanged(index, index, d->staticListContainingThumbnailRole);
            }
        }
    }
}

} // namespace Digikam
