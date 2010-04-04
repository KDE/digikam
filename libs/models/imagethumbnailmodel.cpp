/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries with support for thumbnail loading
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes

#include "thumbnailloadthread.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageThumbnailModelPriv
{
public:

    ImageThumbnailModelPriv()
    {
        thread              = 0;
        thumbSize           = 0;
        lastGlobalThumbSize = 0;
        emitDataChanged     = true;
    }

    ThumbnailLoadThread       *thread;
    ThumbnailSize              thumbSize;
    ThumbnailSize              lastGlobalThumbSize;
    bool                       emitDataChanged;
};

ImageThumbnailModel::ImageThumbnailModel(QObject *parent)
    : ImageModel(parent), d(new ImageThumbnailModelPriv)
{
    setKeepsFilePathCache(true);
}

ImageThumbnailModel::~ImageThumbnailModel()
{
    delete d;
}

void ImageThumbnailModel::setThumbnailLoadThread(ThumbnailLoadThread *thread)
{
    d->thread = thread;

    connect(d->thread, SIGNAL(signalThumbnailLoaded(const LoadingDescription &, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription &, const QPixmap&)));
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

void ImageThumbnailModel::setEmitDataChanged(bool emitSignal)
{
    d->emitDataChanged = emitSignal;
}

void ImageThumbnailModel::prepareThumbnails(const QList<QModelIndex>& indexesToPrepare)
{
    prepareThumbnails(indexesToPrepare, d->thumbSize);
}

void ImageThumbnailModel::prepareThumbnails(const QList<QModelIndex>& indexesToPrepare, const ThumbnailSize& thumbSize)
{
    if (!d->thread)
        return;

    QStringList filePaths;
    foreach(const QModelIndex& index, indexesToPrepare)
    {
        filePaths << imageInfoRef(index).filePath();
    }
    d->thread->findGroup(filePaths, thumbSize.size());
}

QVariant ImageThumbnailModel::data(const QModelIndex& index, int role) const
{
    if (role == ThumbnailRole && d->thread && index.isValid())
    {
        QPixmap thumbnail;
        ImageInfo info = imageInfoRef(index);
        QString path = info.filePath();
        if (d->thread->find(path, thumbnail, d->thumbSize.size()))
            return thumbnail;
        else
        {
            return QVariant(QVariant::Pixmap);
        }
    }
    return ImageModel::data(index, role);
}

bool ImageThumbnailModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == ThumbnailRole && d->thread)
    {
        if (value.isNull())
            d->thumbSize = d->lastGlobalThumbSize;
        else
            d->thumbSize = value.toInt();
    }
    return ImageModel::setData(index, value, role);
}

void ImageThumbnailModel::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb)
{
    if (thumb.isNull())
        return;
    QModelIndex changed = indexForPath(loadingDescription.filePath);
    if (changed.isValid())
    {
        emit thumbnailAvailable(changed, loadingDescription.previewParameters.size);
        if (d->emitDataChanged)
            emit dataChanged(changed, changed);
    }
}

}

