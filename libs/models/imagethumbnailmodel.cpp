/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries with support for thumbnail loading
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C)      2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagethumbnailmodel.moc"

// Qt includes

#include <QHash>

// KDE includes

#include <kdebug.h>

// Local includes

#include "thumbnailloadthread.h"
#include "digikam_export.h"
#include "globals.h"

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
        emitDataChanged(true),
        exifRotate(true)
    {
    }

    ThumbnailLoadThread*   thread;
    ThumbnailLoadThread*   preloadThread;
    ThumbnailSize          thumbSize;
    ThumbnailSize          lastGlobalThumbSize;
    ThumbnailSize          preloadThumbSize;
    QRect                  detailRect;

    bool                   emitDataChanged;
    bool                   exifRotate;

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
            d->preloadThread->setExifRotate(d->exifRotate);
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

void ImageThumbnailModel::setExifRotate(bool rotate)
{
    d->exifRotate = rotate;

    if (d->thread)
    {
        d->thread->setExifRotate(rotate);
    }

    if (d->preloadThread)
    {
        d->preloadThread->setExifRotate(rotate);
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

    QStringList filePaths;
    foreach(const QModelIndex& index, indexesToPrepare)
    {
        filePaths << imageInfoRef(index).filePath();
    }
    d->thread->findGroup(filePaths, thumbSize.size());
}

void ImageThumbnailModel::preloadThumbnails(const QList<ImageInfo>& infos)
{
    if (!d->preloadThread)
    {
        return;
    }

    QStringList filePaths;
    foreach(const ImageInfo& info, infos)
    {
        filePaths << info.filePath();
    }
    d->preloadThread->stopAllTasks();
    d->preloadThread->pregenerateGroup(filePaths, d->preloadThumbnailSize());
}

void ImageThumbnailModel::preloadThumbnails(const QList<QModelIndex>& infos)
{
    if (!d->preloadThread)
    {
        return;
    }

    QStringList filePaths;
    foreach(const QModelIndex& index, infos)
    {
        filePaths << imageInfoRef(index).filePath();
    }
    d->preloadThread->stopAllTasks();
    d->preloadThread->pregenerateGroup(filePaths, d->preloadThumbnailSize());
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

        if (info.isNull() || path.isEmpty())
        {
            return QVariant(QVariant::Pixmap);
        }

        if (!d->detailRect.isNull())
        {
            if (d->thread->find(path, d->detailRect, thumbnail, d->thumbSize.size()))
            {
                return thumbnail;
            }
        }
        else
        {
            if (d->thread->find(path, thumbnail, d->thumbSize.size()))
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

    // In case of multiple occurrence, we currently dont know which thumbnail is this. Signal change on all.
    foreach (const QModelIndex& index, indexesForPath(loadingDescription.filePath))
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
                emit dataChanged(index, index);
            }
        }
    }
}

} // namespace Digikam
