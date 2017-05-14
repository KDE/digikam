/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-22
 * Description : Qt item model for Showfoto thumbnails entries
 *
 * Copyright (C) 2013 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "showfotothumbnailmodel.h"

// Local includes

#include "drawdecoder.h"
#include "digikam_debug.h"
#include "dmetadata.h"
#include "imagescanner.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "loadingdescription.h"

using namespace Digikam;

namespace ShowFoto
{

class ShowfotoThumbnailModel::Private
{
public:

        Private() :
        thread(0),
        preloadThread(0),
        thumbSize(0),
        lastGlobalThumbSize(0),
        preloadThumbSize(0),
        emitDataChanged(true)
    {
        maxThumbSize = ThumbnailSize::Huge;
    }

    int preloadThumbnailSize() const
    {
        if (preloadThumbSize.size())
        {
            return preloadThumbSize.size();
        }

        return thumbSize.size();
    }

public:

    ThumbnailLoadThread* thread;
    ThumbnailLoadThread* preloadThread;
    ThumbnailSize        thumbSize;
    ThumbnailSize        lastGlobalThumbSize;
    ThumbnailSize        preloadThumbSize;
    QRect                detailRect;
    int                  maxThumbSize;
    bool                 emitDataChanged;
};

ShowfotoThumbnailModel::ShowfotoThumbnailModel(QObject* const parent)
    : ShowfotoImageModel(parent),
      d(new Private)
{
    connect(this, &ShowfotoThumbnailModel::signalThumbInfo, this, &ShowfotoThumbnailModel::slotThumbInfoLoaded);
}

ShowfotoThumbnailModel::~ShowfotoThumbnailModel()
{
    delete d->preloadThread;
    delete d;
}

void ShowfotoThumbnailModel::setThumbnailLoadThread(ThumbnailLoadThread* thread)
{
    d->thread = thread;

    connect(d->thread, &ThumbnailLoadThread::signalThumbnailLoaded,
            this, &ShowfotoThumbnailModel::slotThumbnailLoaded);
}

ThumbnailLoadThread* ShowfotoThumbnailModel::thumbnailLoadThread() const
{
    return d->thread;
}

ThumbnailSize ShowfotoThumbnailModel::thumbnailSize() const
{
    return d->thumbSize;
}

void ShowfotoThumbnailModel::setThumbnailSize(const ThumbnailSize& size)
{
    d->lastGlobalThumbSize = size;
    d->thumbSize           = size;
}

void ShowfotoThumbnailModel::setPreloadThumbnailSize(const ThumbnailSize& size)
{
    d->preloadThumbSize = size;
}

void ShowfotoThumbnailModel::setEmitDataChanged(bool emitSignal)
{
    d->emitDataChanged = emitSignal;
}

void ShowfotoThumbnailModel::showfotoItemInfosCleared()
{
    if (d->preloadThread)
    {
        d->preloadThread->stopAllTasks();
    }
}

QVariant ShowfotoThumbnailModel::data(const QModelIndex& index, int role) const
{

    if (role == ThumbnailRole && d->thread && index.isValid())
    {
        QImage    thumbnailImage;
        QPixmap   pixmap;
        ShowfotoItemInfo info = showfotoItemInfo(index);
        QString url           = info.url.toDisplayString();
        QString path          = info.folder + QLatin1String("/") + info.name;

        if (info.isNull() || url.isEmpty())
        {
            return QVariant(QVariant::Pixmap);
        }

        if(pixmapForItem(path,pixmap))
        {
            return pixmap;
        }

        //if pixmapForItem Failed
        if(getThumbnail(info,thumbnailImage))
        {
            thumbnailImage = thumbnailImage.scaled(d->thumbSize.size(),d->thumbSize.size(),Qt::KeepAspectRatio);
            emit signalThumbInfo(info,thumbnailImage);
            return thumbnailImage;
        }

        return QVariant(QVariant::Pixmap);
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

    return ShowfotoImageModel::setData(index, value, role);
}

void ShowfotoThumbnailModel::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb)
{
    if (thumb.isNull())
    {
        return;
    }

    // In case of multiple occurrence, we currently do not know which thumbnail is this. Signal change on all.
    foreach(const QModelIndex& index, indexesForUrl(QUrl::fromLocalFile(loadingDescription.filePath)))
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

bool ShowfotoThumbnailModel::getThumbnail(const ShowfotoItemInfo& itemInfo, QImage& thumbnail) const
{
    QString path = itemInfo.folder + QLatin1String("/") + itemInfo.name;

    // Try to get preview from Exif data (good quality). Can work with Raw files

    DMetadata metadata(path);
    metadata.getImagePreview(thumbnail);

    if (!thumbnail.isNull())
    {
        return true;
    }

    // RAW files : try to extract embedded thumbnail using RawEngine

    DRawDecoder::loadRawPreview(thumbnail, path);

    if (!thumbnail.isNull())
    {
        return true;
    }

    KSharedConfig::Ptr config  = KSharedConfig::openConfig();
    KConfigGroup group         = config->group(QLatin1String("Camera Settings"));
    bool turnHighQualityThumbs = group.readEntry(QLatin1String("TurnHighQualityThumbs"), false);

    // Try to get thumbnail from Exif data (poor quality).
    if (!turnHighQualityThumbs)
    {
        thumbnail = metadata.getExifThumbnail(true);

        if (!thumbnail.isNull())
        {
            return true;
        }
    }

    // THM files: try to get thumbnail from '.thm' files if we didn't manage to get
    // thumbnail from Exif. Any cameras provides *.thm files like JPEG files with RAW files.
    // Using this way is always speed up than ultimate loading using DImg.
    // Note: the thumbnail extracted with this method can be in poor quality.
    // 2006/27/01 - Gilles - Tested with my Minolta Dynax 5D USM camera.

    QFileInfo fi(path);

    if (thumbnail.load(itemInfo.folder + QLatin1String("/") + fi.baseName() + QLatin1String(".thm")))        // Lowercase
    {
        if (!thumbnail.isNull())
        {
            return true;
        }
    }
    else if (thumbnail.load(itemInfo.folder + QLatin1String("/") + fi.baseName() + QLatin1String(".THM")))   // Uppercase
    {
        if (!thumbnail.isNull())
        {
            return true;
        }
    }

    // Finally, we trying to get thumbnail using DImg API (slow).

//    qCDebug(DIGIKAM_SHOWFOTO_LOG) << "Use DImg loader to get thumbnail from : " << path;

//    DImg dimgThumb(path);

//    if (!dimgThumb.isNull())
//    {
//        thumbnail = dimgThumb.copyQImage();
//        return true;
//    }

    return false;
}

bool ShowfotoThumbnailModel::pixmapForItem(QString url, QPixmap& pix) const
{
    if (d->thumbSize.size() > d->maxThumbSize)
    {
        //TODO: Install a widget maximum size to prevent this situation

        bool hasPixmap = d->thread->find(ThumbnailIdentifier(url), pix, d->maxThumbSize);

        if (hasPixmap)
        {

            qCWarning(DIGIKAM_GENERAL_LOG) << "Thumbbar: Requested thumbnail size" << d->thumbSize.size()
                                           << "is larger than the maximum thumbnail size" << d->maxThumbSize
                                           << ". Returning a scaled-up image.";

            pix = pix.scaled(d->thumbSize.size(), d->thumbSize.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return d->thread->find(ThumbnailIdentifier(url), pix, d->thumbSize.size());
    }
}

void ShowfotoThumbnailModel::slotThumbInfoLoaded(const ShowfotoItemInfo& info, const QImage& thumbnailImage)
{
    QImage thumbnail = thumbnailImage;

    if (thumbnail.isNull())
    {
        thumbnail = QImage();
    }

    foreach(const QModelIndex& index, indexesForUrl(info.url))
    {
        if (thumbnail.isNull())
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

} // namespace ShowFoto
