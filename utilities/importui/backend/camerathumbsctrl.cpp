/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-08-03
 * Description : digital camera thumbnails controller
 *
 * Copyright (C) 2011-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "camerathumbsctrl.h"

// Qt includes

#include <QCache>
#include <QPair>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "cameracontroller.h"
#include "thumbnailsize.h"
#include "iccsettings.h"
#include "iccmanager.h"
#include "iccprofile.h"
#include "kiowrapper.h"

namespace Digikam
{

class CameraThumbsCtrlStaticPriv
{
public:

    CameraThumbsCtrlStaticPriv()
    {
        profile = IccProfile::sRGB();
    }

public:

    IccProfile profile;
};

Q_GLOBAL_STATIC(CameraThumbsCtrlStaticPriv, static_d)

// ------------------------------------------------------------------------------------------

class CameraThumbsCtrl::Private
{

public:

    Private()
        : controller(0),
          kioWrapper(0)
    {
    }

    QCache<QUrl, CachedItem> cache;  // Camera info/thumb cache based on item url keys.

    QList<QUrl>              pendingItems;

    CameraController*        controller;

    QList<CamItemInfo>       kdeTodo;
    QHash<QUrl, CamItemInfo> kdeJobHash;
    KIOWrapper*              kioWrapper;
};

// --------------------------------------------------------

CameraThumbsCtrl::CameraThumbsCtrl(CameraController* const ctrl, QWidget* const parent)
    : QObject(parent),
      d(new Private)
{
    d->controller     = ctrl;
    static_d->profile = IccManager::displayProfile(parent);

    connect(d->controller, SIGNAL(signalThumbInfo(QString,QString,CamItemInfo,QImage)),
            this, SLOT(slotThumbInfo(QString,QString,CamItemInfo,QImage)));

    connect(d->controller, SIGNAL(signalThumbInfoFailed(QString,QString,CamItemInfo)),
            this, SLOT(slotThumbInfoFailed(QString,QString,CamItemInfo)));

    setCacheSize(200);
}

CameraThumbsCtrl::~CameraThumbsCtrl()
{
    clearCache();
    delete d->kioWrapper;
}

CameraController* CameraThumbsCtrl::cameraController() const
{
    return d->controller;
}

bool CameraThumbsCtrl::getThumbInfo(const CamItemInfo& info, CachedItem& item) const
{
    if (hasItemFromCache(info.url()))
    {
        // We look if items are not in cache.

        item = *retrieveItemFromCache(info.url());

        // Color Managed view rules.

        if (IccSettings::instance()->useManagedPreviews())
        {
            QImage img  = item.second.toImage();
            IccManager::transformForDisplay(img, static_d->profile);
            item.second = QPixmap::fromImage(img);
        }

        return true;
    }
    else if (!d->pendingItems.contains(info.url()))
    {
        // We look if items are not in pending list.

        d->pendingItems << info.url();
        d->controller->getThumbsInfo(CamItemInfoList() << info, ThumbnailSize::maxThumbsSize());
    }

    item = CachedItem(info, d->controller->mimeTypeThumbnail(info.name).pixmap(ThumbnailSize::maxThumbsSize()));

    return false;
}

void CameraThumbsCtrl::updateThumbInfoFromCache(const CamItemInfo& info)
{
    removeItemFromCache(info.url());
    CachedItem item;
    getThumbInfo(info, item);
}

void CameraThumbsCtrl::slotThumbInfo(const QString&, const QString& file, const CamItemInfo& info, const QImage& thumb)
{
    QImage thumbnail = thumb;

    if (thumb.isNull())
    {
        thumbnail = d->controller->mimeTypeThumbnail(file).pixmap(ThumbnailSize::maxThumbsSize()).toImage();
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
        QPixmap pix = d->controller->mimeTypeThumbnail(file).pixmap(ThumbnailSize::maxThumbsSize());
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
    if (d->kioWrapper || d->kdeTodo.isEmpty())
    {
        return;
    }

    d->kdeJobHash.clear();
    QList<QUrl> list;

    foreach(const CamItemInfo& info, d->kdeTodo)
    {
        QUrl url = info.url();
        list << url;
        d->kdeJobHash[url] = info;
    }

    d->kdeTodo.clear();

    KFileItemList items;

    for (QList<QUrl>::const_iterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        if ((*it).isValid())
            items.append(KFileItem(*it));
    }

    d->kioWrapper = new KIOWrapper();
    d->kioWrapper->filePreview(list, QSize(ThumbnailSize::Huge, ThumbnailSize::Huge));

    connect(d->kioWrapper, SIGNAL(gotPreview(QUrl,QPixmap)),
            this, SLOT(slotGotKDEPreview(QUrl,QPixmap)));

    connect(d->kioWrapper, SIGNAL(previewJobFailed(QUrl)),
            this, SLOT(slotFailedKDEPreview(QUrl)));

    connect(d->kioWrapper, SIGNAL(previewJobFinished()),
            this, SLOT(slotKdePreviewFinished()));
}

void CameraThumbsCtrl::slotGotKDEPreview(const QUrl& item, const QPixmap& pix)
{
    procressKDEPreview(item, pix);
}

void CameraThumbsCtrl::slotFailedKDEPreview(const QUrl& item)
{
    procressKDEPreview(item, QPixmap());
}

void CameraThumbsCtrl::procressKDEPreview(const QUrl& item, const QPixmap& pix)
{
    CamItemInfo info = d->kdeJobHash.value(item);
    QUrl url         = info.url();

    if (info.isNull())
    {
        return;
    }

    QString file = item.fileName();
    QPixmap thumb;

    if (pix.isNull())
    {
        // This call must be run outside Camera Controller thread.
        thumb = d->controller->mimeTypeThumbnail(file).pixmap(ThumbnailSize::maxThumbsSize());
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed thumb from KDE Preview : " << item;
    }
    else
    {
        thumb = pix;
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Got thumb from KDE Preview : " << item;
    }

    putItemToCache(url, info, thumb);
    d->pendingItems.removeAll(url);
    emit signalThumbInfoReady(info);
}

void CameraThumbsCtrl::slotKdePreviewFinished()
{
    delete d->kioWrapper;
    d->kioWrapper = 0;

    startKdePreviewJob();
}

// -- Cache management methods ------------------------------------------------------------

const CachedItem* CameraThumbsCtrl::retrieveItemFromCache(const QUrl& url) const
{
    return d->cache[url];
}

bool CameraThumbsCtrl::hasItemFromCache(const QUrl& url) const
{
    return d->cache.contains(url);
}

void CameraThumbsCtrl::putItemToCache(const QUrl& url, const CamItemInfo& info, const QPixmap& thumb)
{
    int infoCost  = sizeof(info);
    int thumbCost = thumb.width() * thumb.height() * thumb.depth() / 8;
    d->cache.insert(url, new CachedItem(info, thumb), infoCost + thumbCost);
}

void CameraThumbsCtrl::removeItemFromCache(const QUrl& url)
{
    d->cache.remove(url);
}

void CameraThumbsCtrl::clearCache()
{
    d->cache.clear();
}

void CameraThumbsCtrl::setCacheSize(int numberOfItems)
{
    d->cache.setMaxCost(numberOfItems * (ThumbnailSize::maxThumbsSize() * ThumbnailSize::maxThumbsSize() *
                                         QPixmap(1, 1).depth() / 8) + (numberOfItems * sizeof(CamItemInfo)));
}

}  // namespace Digikam
