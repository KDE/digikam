/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-22
 * Description : Qt item model for Showfoto thumbnails entries
 *
 * Copyright (C) 2013 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#include "showfotothumbnailmodel.moc"

#include "QDebug"

namespace ShowFoto {

class ShowfotoThumbnailModel::Private
{
public:
    Private() :
        loader(0),
        thumbSize(0),
        lastGlobalThumbSize(0),
        emitDataChanged(true)
    {
    }

    ShowfotoItemLoader* loader;
    ThumbnailSize       thumbSize;
    ThumbnailSize       lastGlobalThumbSize;
    bool                emitDataChanged;
};

ShowfotoThumbnailModel::ShowfotoThumbnailModel(QObject *parent)
    : ShowfotoImageModel(parent), d(new Private)
{
}

ShowfotoThumbnailModel::~ShowfotoThumbnailModel()
{
    delete d;
}

ThumbnailSize ShowfotoThumbnailModel::thumbnailSize() const
{
    return d->thumbSize;
}

void ShowfotoThumbnailModel::setEmitDataChanged(bool emitSignal)
{
    d->emitDataChanged = emitSignal;
}

void ShowfotoThumbnailModel::setLoader(ShowfotoItemLoader* Loader)
{
    d->loader = Loader;
}

QVariant ShowfotoThumbnailModel::data(const QModelIndex& index, int role) const
{
    if (role == ThumbnailRole && d->loader && index.isValid())
    {
        ShowfotoItemInfo info = showfotoItemInfo(index);
        QString          path = info.url.prettyUrl();
        CachedItem       item;

        if (info.isNull() || path.isEmpty())
        {
            return QVariant(d->loader->mimeTypeThumbnail(path, d->thumbSize.size()));
        }

        bool thumbChanged = false;
        if(d->thumbSize != d->lastGlobalThumbSize)
        {
            thumbChanged = true;
        }

        if (d->loader->loadThumbnailForItem(info, item, d->thumbSize, thumbChanged))
        {
            return QVariant(item.second);
        }

        return QVariant(d->loader->mimeTypeThumbnail(path, d->thumbSize.size()));
    }

    return ShowfotoImageModel::data(index, role);
}

bool ShowfotoThumbnailModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == ThumbnailRole)
    {
        switch (value.type())
        {
            case QVariant::Invalid:
                d->thumbSize  = d->lastGlobalThumbSize;
                break;

            case QVariant::Int:

                if (value.isNull())
                {
                    d->thumbSize = d->lastGlobalThumbSize;
                }
                else
                {
                    d->lastGlobalThumbSize = d->thumbSize;
                    d->thumbSize = value.toInt();
                }
                break;

            default:
                break;
        }
    }

    return ShowfotoImageModel::setData(index, value, role);
}


void ShowfotoThumbnailModel::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb)
{
    qDebug() << "got desc and thumb";
    qDebug() << loadingDescription.filePath;
    if (thumb.isNull())
    {
        return;
    }

    // In case of multiple occurrence, we currently do not know which thumbnail is this. Signal change on all.
    foreach(const QModelIndex& index, indexesForUrl(loadingDescription.filePath))
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


} // namespace ShowFoto
