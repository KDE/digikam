/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-08-03
 * Description : digital camera thumbnails controller
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "cameracontroller.h"
#include "thumbnailsize.h"
#include "iccsettings.h"
#include "iccmanager.h"
#include "iccprofile.h"

#ifdef HAVE_MEDIAPLAYER
#   include "videothumbnailerjob.h"
#endif

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
        :
#ifdef HAVE_MEDIAPLAYER
          videoThumbs(0),
#endif
          controller(0)
    {
    }

    QCache<QUrl, CachedItem> cache;  // Camera info/thumb cache based on item url keys.

    QList<QUrl>              pendingItems;

#ifdef HAVE_MEDIAPLAYER
    VideoThumbnailerJob*     videoThumbs;
#endif

    CameraController*        controller;

    QHash<QUrl, CamItemInfo> videoJobHash;
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

#ifdef HAVE_MEDIAPLAYER

    d->videoThumbs    = new VideoThumbnailerJob(this);
    d->videoThumbs->setThumbnailSize(ThumbnailSize::Huge);
    d->videoThumbs->setCreateStrip(true);

    connect(d->videoThumbs, SIGNAL(signalThumbnailDone(QString,QImage)),
            this, SLOT(slotVideoThumbnailDone(QString,QImage)));

    connect(d->videoThumbs, SIGNAL(signalThumbnailFailed(QString)),
            this, SLOT(slotVideoThumbnailFailed(QString)));

    connect(d->videoThumbs, SIGNAL(signalThumbnailJobFinished()),
            this, SLOT(slotVideoThumbnailFinished()));

#endif
}

CameraThumbsCtrl::~CameraThumbsCtrl()
{
    clearCache();

#ifdef HAVE_MEDIAPLAYER
    delete d->videoThumbs;
#endif
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

    // Color Managed view rules.

    if (IccSettings::instance()->useManagedPreviews())
    {
        IccManager::transformForDisplay(thumbnail, static_d->profile);
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

#ifdef HAVE_MEDIAPLAYER
        d->videoJobHash.insert(info.url(), info);
        d->videoThumbs->addItems(QStringList() << info.url().toLocalFile());
#else
        qDebug(DIGIKAM_IMPORTUI_LOG) << "Cannot get video thumb for " << info.url().toLocalFile();
        qDebug(DIGIKAM_IMPORTUI_LOG) << "Video support is not available";
        slotVideoThumbnailFailed(info.url().toLocalFile());
#endif
    }
    else
    {
        QPixmap pix = d->controller->mimeTypeThumbnail(file).pixmap(ThumbnailSize::maxThumbsSize());
        putItemToCache(info.url(), info, pix);
        d->pendingItems.removeAll(info.url());
        emit signalThumbInfoReady(info);
    }
}

void CameraThumbsCtrl::slotVideoThumbnailDone(const QString& item, const QImage& img)
{
    procressVideoPreview(QUrl::fromLocalFile(item), QPixmap::fromImage(img));
}

void CameraThumbsCtrl::slotVideoThumbnailFailed(const QString& item)
{
    procressVideoPreview(QUrl::fromLocalFile(item), QPixmap());
}

void CameraThumbsCtrl::procressVideoPreview(const QUrl& item, const QPixmap& pix)
{
    CamItemInfo info = d->videoJobHash.value(item);

    if (info.isNull())
    {
        return;
    }

    QPixmap thumb;

    if (pix.isNull())
    {
        // This call must be run outside Camera Controller thread.
        thumb = d->controller->mimeTypeThumbnail(info.name).pixmap(ThumbnailSize::maxThumbsSize());
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed video thumb Preview : " << item.toLocalFile();
    }
    else
    {
        thumb = pix;
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Got video thumb Preview : " << item.toLocalFile();
    }

    putItemToCache(info.url(), info, thumb);
    d->pendingItems.removeAll(info.url());
    emit signalThumbInfoReady(info);
}

void CameraThumbsCtrl::slotVideoThumbnailFinished()
{
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
