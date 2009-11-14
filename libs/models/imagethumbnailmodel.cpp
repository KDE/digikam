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

// #include "imagethumbnailmodel.h"
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
        thread    = 0;
        thumbSize = 0;
    }

    ThumbnailLoadThread       *thread;
    ThumbnailSize              thumbSize;
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

    if (d->thumbSize.size())
        d->thread->setThumbnailSize(d->thumbSize.size());
}

ThumbnailSize ImageThumbnailModel::thumbnailSize() const
{
    return d->thumbSize;
}

void ImageThumbnailModel::setThumbnailSize(const ThumbnailSize& size)
{
    if (d->thumbSize == size)
        return;
    d->thumbSize = size;
    d->thread->setThumbnailSize(d->thumbSize.size());
}

void ImageThumbnailModel::prepareThumbnails(const QList<QModelIndex>& indexesToPrepare)
{
    if (!d->thread)
        return;

    QStringList filePaths;
    foreach(const QModelIndex& index, indexesToPrepare)
    {
        filePaths << imageInfoRef(index).filePath();
    }
    d->thread->findGroup(filePaths);
}

QVariant ImageThumbnailModel::data(const QModelIndex& index, int role) const
{
    if (d->thread && role == ThumbnailRole && index.isValid())
    {
        QPixmap thumbnail;
        ImageInfo info = imageInfoRef(index);
        QString path = info.filePath();
        if (d->thread->find(path, thumbnail))
            return thumbnail;
        else
        {
            return QVariant(QVariant::Pixmap);
        }
    }
    return ImageModel::data(index, role);
}

void ImageThumbnailModel::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb)
{
    if (thumb.isNull())
        return;
    QModelIndex changed = indexForPath(loadingDescription.filePath);
    if (changed.isValid())
        emit dataChanged(changed, changed);
}

}

