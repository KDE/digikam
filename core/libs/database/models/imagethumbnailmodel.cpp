/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries with support for thumbnail loading
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagethumbnailmodel.h"

// Qt includes

#include <QHash>

// Local includes

#include "digikam_debug.h"
#include "thumbnailloadthread.h"
#include "digikam_export.h"
#include "digikam_globals.h"

namespace Digikam
{

class ImageThumbnailModel::ImageThumbnailModelPriv
{
public:

    ImageThumbnailModelPriv() :
        thread(0),
        preloadThread(0),
        thumbSize(0),
        lastGlobalThumbSize(0),
        preloadThumbSize(0),
        emitDataChanged(true)
    {
        staticListContainingThumbnailRole << ImageModel::ThumbnailRole;
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

ImageThumbnailModel::ImageThumbnailModel(QObject* parent)
    : ImageModel(parent), d(new ImageThumbnailModelPriv)
{
    setKeepsFilePathCache(true);
}

ImageThumbnailModel::~ImageThumbnailModel()
{
    delete d->preloadThread;
    delete d;
}

void ImageThumbnailModel::setThumbnailLoadThread(ThumbnailLoadThread* thread)
{
    d->thread = thread;

    connect(d->thread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));
}

ThumbnailLoadThread* ImageThumbnailModel::thumbnailLoadThread() const
{
    return d->thread;
}

ThumbnailSize ImageThumbnailModel::thumbnailSize() const
{
    return d->thumbSize;
}

void ImageThumbnailModel::setThumbnailSize(const ThumbnailSize& size)
{
    d->lastGlobalThumbSize = size;
    d->thumbSize = size;
}

void ImageThumbnailModel::setPreloadThumbnailSize(const ThumbnailSize& size)
{
    d->preloadThumbSize = size;
}

void ImageThumbnailModel::setEmitDataChanged(bool emitSignal)
{
    d->emitDataChanged = emitSignal;
}

void ImageThumbnailModel::setPreloadThumbnails(bool preload)
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
        d->preloadThread = 0;
        disconnect(this, SIGNAL(allRefreshingFinished()),
                   this, SLOT(preloadAllThumbnails()));
    }
}

void ImageThumbnailModel::prepareThumbnails(const QList<QModelIndex>& indexesToPrepare)
{
    prepareThumbnails(indexesToPrepare, d->thumbSize);
}

void ImageThumbnailModel::prepareThumbnails(const QList<QModelIndex>& indexesToPrepare, const ThumbnailSize& thumbSize)
{
    if (!d->thread)
    {
        return;
    }

    QList<ThumbnailIdentifier> ids;
    foreach(const QModelIndex& index, indexesToPrepare)
    {
        ids << imageInfoRef(index).thumbnailIdentifier();
    }
    d->thread->findGroup(ids, thumbSize.size());
}

void ImageThumbnailModel::preloadThumbnails(const QList<ImageInfo>& infos)
{
    if (!d->preloadThread)
    {
        return;
    }

    QList<ThumbnailIdentifier> ids;
    foreach(const ImageInfo& info, infos)
    {
        ids << info.thumbnailIdentifier();
    }
    d->preloadThread->pregenerateGroup(ids, d->preloadThumbnailSize());
}

void ImageThumbnailModel::preloadThumbnails(const QList<QModelIndex>& indexesToPreload)
{
    if (!d->preloadThread)
    {
        return;
    }

    QList<ThumbnailIdentifier> ids;
    foreach(const QModelIndex& index, indexesToPreload)
    {
        ids << imageInfoRef(index).thumbnailIdentifier();
    }
    d->preloadThread->stopAllTasks();
    d->preloadThread->pregenerateGroup(ids, d->preloadThumbnailSize());
}

void ImageThumbnailModel::preloadAllThumbnails()
{
    preloadThumbnails(imageInfos());
}

void ImageThumbnailModel::imageInfosCleared()
{
    if (d->preloadThread)
    {
        d->preloadThread->stopAllTasks();
    }
}

QVariant ImageThumbnailModel::data(const QModelIndex& index, int role) const
{
    if (role == ThumbnailRole && d->thread && index.isValid())
    {
        QPixmap   thumbnail;
        ImageInfo info = imageInfo(index);
        QString   path = info.filePath();

        if (info.isNull())
        {
            return QVariant(QVariant::Pixmap);
        }

        if (!d->detailRect.isNull())
        {
            if (d->thread->find(info.thumbnailIdentifier(), d->detailRect, thumbnail, d->thumbSize.size()))
            {
                return thumbnail;
            }
        }
        else
        {
            if (d->thread->find(info.thumbnailIdentifier(), thumbnail, d->thumbSize.size()))
            {
                return thumbnail;
            }
        }

        return QVariant(QVariant::Pixmap);
    }

    return ImageModel::data(index, role);
}

bool ImageThumbnailModel::setData(const QModelIndex& index, const QVariant& value, int role)
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
                    d->thumbSize = value.toInt();
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

    return ImageModel::setData(index, value, role);
}

void ImageThumbnailModel::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb)
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
    foreach(const QModelIndex& index, indexes)
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
