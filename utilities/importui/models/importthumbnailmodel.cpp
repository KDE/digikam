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

#include "importthumbnailmodel.moc"
#include "importthumbnailmodel.h"

// Qt includes

#include <QCache>

// KDE includes

#include <kio/previewjob.h>
#include <KDebug>
#include <kdeversion.h>

namespace Digikam
{

class ImportThumbnailModel::Private
{
public:

    Private() :
        controller(0),
        thumbSize(0),
        lastGlobalThumbSize(0),
        emitDataChanged(true)
    {
    }

    CameraController*        controller;

    KUrl::List               pendingItems;
    QCache<KUrl, CachedItem> cache;  // Camera info/thumb cache based on item url keys.

    QList<CamItemInfo>       kdeTodo;
    QHash<KUrl, CamItemInfo> kdeJobHash;
    KIO::PreviewJob*         kdeJob;

    ThumbnailSize            thumbSize;
    ThumbnailSize            lastGlobalThumbSize;
    bool                     emitDataChanged;
};

ImportThumbnailModel::ImportThumbnailModel(QObject* const parent)
    : ImportImageModel(parent), d(new Private)
{
    setKeepsFileUrlCache(true);
    setCacheSize(200);
}

ImportThumbnailModel::~ImportThumbnailModel()
{
    clearCache();
    delete d;
}

void ImportThumbnailModel::setCameraController(CameraController* const controller)
{
    d->controller = controller;

    connect(d->controller, SIGNAL(signalThumbInfo(QString,QString,CamItemInfo,QImage)),
            this, SLOT(slotThumbInfoLoaded(QString,QString,CamItemInfo,QImage)));

    connect(d->controller, SIGNAL(signalThumbInfoFailed(QString,QString,CamItemInfo)),
            this, SLOT(slotThumbInfoFailed(QString,QString,CamItemInfo)));

    ImportImageModel::setCameraController(controller);
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
    if (role == ThumbnailRole && d->controller && index.isValid())
    {
        CamItemInfo info = camItemInfo(index);
        QString     path = info.url().prettyUrl();
        CachedItem  item;

        if (info.isNull() || path.isEmpty())
        {
            return QVariant(d->controller->mimeTypeThumbnail(path));
        }

        bool thumbChanged = false;
        if(d->thumbSize != d->lastGlobalThumbSize)
        {
            thumbChanged = true;
        }

        if (getThumbInfo(info, item, d->thumbSize, thumbChanged))
        {
            return QVariant(item.second);
        }

        return QVariant(d->controller->mimeTypeThumbnail(path));
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
                    d->thumbSize = value.toInt();
                }
                break;

            default:
                break;
        }
    }

    return ImportImageModel::setData(index, value, role);
}

bool ImportThumbnailModel::getThumbInfo(const CamItemInfo& info, CachedItem& item, ThumbnailSize thumbSize, bool thumbChanged) const
{
    // If thumbSize changed clear cache and reload thumbs for items.
    if(thumbChanged)
    {
        d->cache.clear();
        d->pendingItems.clear();
    }

    // We look if items are not in cache.

    if (hasItemFromCache(info.url()))
    {
        item = *retrieveItemFromCache(info.url());
        return true;
        // kDebug() << "Found in cache: " << info.url();
    }

    // We look if items are not in pending list.

    else if (!d->pendingItems.contains(info.url()))
    {
        d->pendingItems << info.url();
        // kDebug() << "Request thumbs from camera : " << info.url();
        d->controller->getThumbsInfo(CamItemInfoList() << info, thumbSize);
    }

    item = CachedItem(info, d->controller->mimeTypeThumbnail(info.name));
    return false;
}

void ImportThumbnailModel::slotThumbInfoLoaded(const QString& folder, const QString& file, const CamItemInfo& info, const QImage& thumb)
{
    Q_UNUSED(folder);

    QImage thumbnail = thumb;

    if (thumb.isNull())
    {
        thumbnail = d->controller->mimeTypeThumbnail(file).toImage();
    }

    putItemToCache(info.url(), info, QPixmap::fromImage(thumbnail));
    d->pendingItems.removeAll(info.url());

    // In case of multiple occurrence, we currently do not know which thumbnail is this. Signal change on all.
    foreach(const QModelIndex& index, indexesForUrl(info.url()))
    {
        if (thumb.isNull())
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

void ImportThumbnailModel::slotThumbInfoFailed(const QString& folder, const QString& file, const CamItemInfo& info)
{
    Q_UNUSED(folder);

    if (d->controller->cameraDriverType() == DKCamera::UMSDriver)
    {
        putItemToCache(info.url(), info, QPixmap());
        loadWithKDE(info);
    }
    else
    {
        QPixmap pix = d->controller->mimeTypeThumbnail(file);
        putItemToCache(info.url(), info, pix);
        d->pendingItems.removeAll(info.url());

        // In case of multiple occurrence, we currently do not know which thumbnail is this. Signal change on all.
        foreach(const QModelIndex& index, indexesForUrl(info.url()))
        {
            if (pix.isNull())
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
}

void ImportThumbnailModel::loadWithKDE(const CamItemInfo& info)
{
    d->kdeTodo << info;
    startKdePreviewJob();
}

void ImportThumbnailModel::startKdePreviewJob()
{
    if (d->kdeJob || d->kdeTodo.isEmpty())
    {
        return;
    }

    d->kdeJobHash.clear();
    KUrl::List list;
    foreach(const CamItemInfo& info, d->kdeTodo)
    {
        KUrl url           = info.url();
        list << url;
        d->kdeJobHash[url] = info;
    }
    d->kdeTodo.clear();

#if KDE_IS_VERSION(4,7,0)
    KFileItemList items;
    for (KUrl::List::ConstIterator it = list.begin() ; it != list.end() ; ++it)
    {
        if ((*it).isValid())
            items.append(KFileItem(KFileItem::Unknown, KFileItem::Unknown, *it, true));
    }
    d->kdeJob = KIO::filePreview(items, QSize(ThumbnailSize::Huge, ThumbnailSize::Huge));
#else
    d->kdeJob = KIO::filePreview(list, ThumbnailSize::Huge);
#endif

    connect(d->kdeJob, SIGNAL(gotPreview(KFileItem,QPixmap)),
            this, SLOT(slotGotKDEPreview(KFileItem,QPixmap)));

    connect(d->kdeJob, SIGNAL(failed(KFileItem)),
            this, SLOT(slotFailedKDEPreview(KFileItem)));

    connect(d->kdeJob, SIGNAL(finished(KJob*)),
            this, SLOT(slotKdePreviewFinished(KJob*)));
}

void ImportThumbnailModel::slotGotKDEPreview(const KFileItem& item, const QPixmap& pix)
{
    procressKDEPreview(item, pix);
}

void ImportThumbnailModel::slotFailedKDEPreview(const KFileItem& item)
{
    procressKDEPreview(item, QPixmap());
}

void ImportThumbnailModel::procressKDEPreview(const KFileItem& item, const QPixmap& pix)
{
    CamItemInfo info = d->kdeJobHash.value(item.url());
    QUrl url         = info.url();

    if (info.isNull())
    {
        return;
    }

    QString file   = item.url().fileName();
    QString folder = item.url().toLocalFile().remove(QString("/") + file);
    QPixmap thumb;

    if (pix.isNull())
    {
        // This call must be run outside Camera Controller thread.
        thumb = d->controller->mimeTypeThumbnail(file);
        kDebug() << "Failed thumb from KDE Preview : " << item.url();
    }
    else
    {
        thumb = pix;
        kDebug() << "Got thumb from KDE Preview : " << item.url();
    }

    putItemToCache(url, info, thumb);
    d->pendingItems.removeAll(url);

    // In case of multiple occurrence, we currently do not know which thumbnail is this. Signal change on all.
    foreach(const QModelIndex& index, indexesForUrl(info.url()))
    {
        if (pix.isNull())
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

void ImportThumbnailModel::slotKdePreviewFinished(KJob*)
{
    d->kdeJob = 0;
    startKdePreviewJob();
}

// -- Cache management methods ------------------------------------------------------------

const CachedItem* ImportThumbnailModel::retrieveItemFromCache(const KUrl& url) const
{
    return d->cache[url];
}

bool ImportThumbnailModel::hasItemFromCache(const KUrl& url) const
{
    return d->cache.contains(url);
}

void ImportThumbnailModel::putItemToCache(const KUrl& url, const CamItemInfo& info, const QPixmap& thumb)
{
    int infoCost  = sizeof(info);
    int thumbCost = thumb.width() * thumb.height() * thumb.depth() / 8;
    d->cache.insert(url,
                    new CachedItem(info, thumb),
                    infoCost + thumbCost);
}

void ImportThumbnailModel::removeItemFromCache(const KUrl& url)
{
    d->cache.remove(url);
}

void ImportThumbnailModel::clearCache()
{
    d->cache.clear();
}

void ImportThumbnailModel::setCacheSize(int numberOfItems)
{
    d->cache.setMaxCost((numberOfItems * 256 * 256 * QPixmap::defaultDepth() / 8) +
                        (numberOfItems * 1024 * 2));
}

} // namespace Digikam
