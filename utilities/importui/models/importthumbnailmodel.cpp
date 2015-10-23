/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-06-13
 * Description : Qt item model for camera thumbnails entries
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

#include "importthumbnailmodel.h"

// Qt includes

#include <QCache>
#include <QReadWriteLock>

// Local includes

#include "digikam_debug.h"
#include "cameracontroller.h"

namespace Digikam
{

class ImportThumbnailModel::Private
{
public:

    Private() :
        thumbsCtrl(0),
        thumbSize(0),
        lastGlobalThumbSize(0),
        emitDataChanged(true)
    {
    }

    CameraThumbsCtrl* thumbsCtrl;

    ThumbnailSize     thumbSize;
    ThumbnailSize     lastGlobalThumbSize;
    bool              emitDataChanged;
};

ImportThumbnailModel::ImportThumbnailModel(QObject* const parent)
    : ImportImageModel(parent), d(new Private)
{
    setKeepsFileUrlCache(true);
}

ImportThumbnailModel::~ImportThumbnailModel()
{
    delete d;
}

void ImportThumbnailModel::setCameraThumbsController(CameraThumbsCtrl* const thumbsCtrl)
{
    d->thumbsCtrl = thumbsCtrl;

    connect(d->thumbsCtrl, SIGNAL(signalThumbInfoReady(const CamItemInfo&)),
            this, SLOT(slotThumbInfoReady(const CamItemInfo&)));

    ImportImageModel::setCameraThumbsController(d->thumbsCtrl);
}

ThumbnailSize ImportThumbnailModel::thumbnailSize() const
{
    return d->thumbSize;
}

void ImportThumbnailModel::setEmitDataChanged(bool emitSignal)
{
    d->emitDataChanged = emitSignal;
}

QVariant ImportThumbnailModel::data(const QModelIndex& index, int role) const
{
    if (role == ThumbnailRole && d->thumbsCtrl && index.isValid())
    {
        CamItemInfo info = camItemInfo(index);
        QString     path = info.url().toLocalFile();
        CachedItem  item;

        // use mimetype thumbnail also if the mime is set to something else than to image
        // this is to avoid querying the device for previews with unsupported file formats
        // at least gphoto2 doesn't really like it and will error a lot and slow down
        if (info.isNull() || path.isEmpty() || !info.previewPossible)
        {
            return QVariant(d->thumbsCtrl->cameraController()->mimeTypeThumbnail(path).pixmap(d->thumbSize.size()));
        }

        if (d->thumbsCtrl->getThumbInfo(info, item))
        {
            return QVariant(item.second.scaled(d->thumbSize.size(), d->thumbSize.size(), Qt::KeepAspectRatio));
        }

        return QVariant(d->thumbsCtrl->cameraController()->mimeTypeThumbnail(path).pixmap(d->thumbSize.size()));
    }

    return ImportImageModel::data(index, role);
}

bool ImportThumbnailModel::setData(const QModelIndex& index, const QVariant& value, int role)
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
                    d->thumbSize           = value.toInt();
                }
                break;

            default:
                break;
        }
    }

    return ImportImageModel::setData(index, value, role);
}

void ImportThumbnailModel::slotThumbInfoReady(const CamItemInfo& info)
{
    CachedItem  item;
    d->thumbsCtrl->getThumbInfo(info, item);

    // In case of multiple occurrence, we currently do not know which thumbnail is this. Signal change on all.
    foreach(const QModelIndex& index, indexesForUrl(info.url()))
    {
        if (item.second.isNull())
        {
            emit thumbnailFailed(index, d->thumbSize.size());
        }
        else
        {
            emit thumbnailAvailable(index, d->thumbSize.size());

            if (d->emitDataChanged)
            {
                emit dataChanged(index, index);
            }
        }
    }
}

} // namespace Digikam
