/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-08-03
 * Description : digital camera thumbnails controller
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "camerathumbsctrl.moc"

// Qt includes

#include <QCache>
#include <QPair>

// KDE includes

#include <kdebug.h>
#include <kurl.h>
#include <kio/previewjob.h>
#include <kdeversion.h>

// Local includes

#include "cameracontroller.h"
#include "thumbnailsize.h"

namespace Digikam
{

class CameraThumbsCtrl::CameraThumbsCtrlPriv
{

public:

    CameraThumbsCtrlPriv()
      : controller(0)
    {}

    QCache<KUrl, CachedItem> cache;  // Camera info/thumb cache based on item url keys.

    KUrl::List               pendingItems;

    CameraController*        controller;

    QList<CamItemInfo>       kdeTodo;
    QHash<KUrl, CamItemInfo> kdeJobHash;
    KIO::PreviewJob*         kdeJob;

};

// --------------------------------------------------------

CameraThumbsCtrl::CameraThumbsCtrl(CameraController* ctrl, QObject* parent)
    : QObject(parent), d(new CameraThumbsCtrlPriv)
{
    d->controller = ctrl;

    connect(d->controller, SIGNAL(signalThumbInfo(QString,QString,CamItemInfo,QImage)),
            this, SLOT(slotThumbInfo(QString,QString,CamItemInfo,QImage)));

    connect(d->controller, SIGNAL(signalThumbInfoFailed(QString,QString,CamItemInfo)),
            this, SLOT(slotThumbInfoFailed(QString,QString,CamItemInfo)));

    setCacheSize(200);
}

CameraThumbsCtrl::~CameraThumbsCtrl()
{
    clearCache();
    delete d;
}

CachedItem CameraThumbsCtrl::getThumbInfo(const CamItemInfo& info) const
{
    // We look if items are not in cache.

    if (hasItemFromCache(info.url()))
    {
        return *retrieveItemFromCache(info.url());
        // kDebug() << "Found in cache: " << info.url();
    }

    // We look if items are not in pending list.

    else if (!d->pendingItems.contains(info.url()))
    {
        d->pendingItems << info.url();
        // kDebug() << "Request thumbs from camera : " << info.url();
        d->controller->getThumbsInfo(CamItemInfoList() << info);
    }

    return CachedItem(info, d->controller->mimeTypeThumbnail(info.name));
}

void CameraThumbsCtrl::slotThumbInfo(const QString&, const QString& file, const CamItemInfo& info, const QImage& thumb)
{
    QImage thumbnail = thumb;

    if (thumb.isNull())
    {
        thumbnail = d->controller->mimeTypeThumbnail(file).toImage();
    }

    putItemToCache(info.url(), info, QPixmap::fromImage(thumbnail));
    d->pendingItems.removeAll(info.url());
    emit signalThumbInfoReady(info);
}

void CameraThumbsCtrl::slotThumbInfoFailed(const QString& /*folder*/, const QString& file, const CamItemInfo& info)
{
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
        emit signalThumbInfoReady(info);
    }
}

void CameraThumbsCtrl::loadWithKDE(const CamItemInfo& info)
{
    d->kdeTodo << info;
    startKdePreviewJob();
}

void CameraThumbsCtrl::startKdePreviewJob()
{
    if (d->kdeJob || d->kdeTodo.isEmpty())
    {
        return;
    }

    d->kdeJobHash.clear();
    KUrl::List list;
    foreach (const CamItemInfo& info, d->kdeTodo)
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

void CameraThumbsCtrl::slotGotKDEPreview(const KFileItem& item, const QPixmap& pix)
{
    procressKDEPreview(item, pix);
}

void CameraThumbsCtrl::slotFailedKDEPreview(const KFileItem& item)
{
    procressKDEPreview(item, QPixmap());
}

void CameraThumbsCtrl::procressKDEPreview(const KFileItem& item, const QPixmap& pix)
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
    emit signalThumbInfoReady(info);
}

void CameraThumbsCtrl::slotKdePreviewFinished(KJob*)
{
    d->kdeJob = 0;
    startKdePreviewJob();
}

// -- Cache management methods ------------------------------------------------------------

const CachedItem* CameraThumbsCtrl::retrieveItemFromCache(const KUrl& url) const
{
    return d->cache[url];
}

bool CameraThumbsCtrl::hasItemFromCache(const KUrl& url) const
{
    return d->cache.contains(url);
}

void CameraThumbsCtrl::putItemToCache(const KUrl& url, const CamItemInfo& info, const QPixmap& thumb)
{
    int infoCost  = sizeof(info);
    int thumbCost = thumb.width() * thumb.height() * thumb.depth() / 8;
    d->cache.insert(url,
                    new CachedItem(info, thumb),
                    infoCost + thumbCost);
}

void CameraThumbsCtrl::removeItemFromCache(const KUrl& url)
{
    d->cache.remove(url);
}

void CameraThumbsCtrl::clearCache()
{
    d->cache.clear();
}

// NOTE: Marcel, how to compute cost for CamItemInfo container. I set 2 Kb : it's fine ?

void CameraThumbsCtrl::setCacheSize(int numberOfItems)
{
    d->cache.setMaxCost( (numberOfItems * 256 * 256 * QPixmap::defaultDepth() / 8) +
                         (numberOfItems * 1024 * 2));
}

}  // namespace Digikam
