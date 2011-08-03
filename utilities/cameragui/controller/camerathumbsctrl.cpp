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

#include <QList>

// KDE includes

#include <kdebug.h>
#include <kurl.h>
#include <kio/previewjob.h>

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

    KUrl::List        pendingItems;

    CameraController* controller;
};

// --------------------------------------------------------

CameraThumbsCtrl::CameraThumbsCtrl(CameraController* ctrl, QObject* parent)
    : QObject(parent), d(new CameraThumbsCtrlPriv)
{
    d->controller = ctrl;

    connect(d->controller, SIGNAL(signalThumbInfo(QString, QString, CamItemInfo, QImage)),
            this, SLOT(slotThumbInfo(QString, QString, CamItemInfo, QImage)));

    connect(d->controller, SIGNAL(signalThumbInfoFailed(QString, QString, CamItemInfo)),
            this, SLOT(slotThumbInfoFailed(QString, QString, CamItemInfo)));
}

CameraThumbsCtrl::~CameraThumbsCtrl()
{
    delete d;
}

void CameraThumbsCtrl::getThumbsInfo(const CamItemInfoList& list)
{
    CamItemInfoList toProcess;

    foreach (CamItemInfo info, list)
    {
        if (!d->pendingItems.contains(info.url()))
        {
            toProcess.append(info);
            d->pendingItems << info.url();
            kDebug() << "Request thumbs from camera : " << info.url();
        }
    }

    if (!toProcess.isEmpty())
    {
        d->controller->getThumbsInfo(toProcess);
    }
}

void CameraThumbsCtrl::slotThumbInfo(const QString&, const QString& file, const CamItemInfo& info, const QImage& thumb)
{
    if (thumb.isNull())
    {
        emit signalThumbInfo(info, d->controller->mimeTypeThumbnail(file).toImage());
    }
    else
    {
        emit signalThumbInfo(info, thumb);
    }

    d->pendingItems.removeAll(info.url());
}

void CameraThumbsCtrl::slotThumbInfoFailed(const QString& folder, const QString& file, const CamItemInfo& info)
{
    if (d->controller->cameraDriverType() == DKCamera::UMSDriver)
    {
        emit signalInfo(folder, file, info);
        startKdePreviewJob(info.url());
    }
    else
    {
        emit signalThumbInfo(info, d->controller->mimeTypeThumbnail(file).toImage());
        d->pendingItems.removeAll(info.url());
    }
}

void CameraThumbsCtrl::startKdePreviewJob(const KUrl& url)
{

    KIO::PreviewJob* job = KIO::filePreview(KUrl::List() << url, ThumbnailSize::Huge);

    connect(job, SIGNAL(gotPreview(KFileItem, QPixmap)),
            this, SLOT(slotGotKDEPreview(KFileItem, QPixmap)));

    connect(job, SIGNAL(failed(KFileItem)),
            this, SLOT(slotFailedKDEPreview(KFileItem)));

    kDebug() << "pending thumbs from KDE Preview : " << url;
}

void CameraThumbsCtrl::slotGotKDEPreview(const KFileItem& item, const QPixmap& pix)
{
    procressKDEPreview(item, pix);
}

void CameraThumbsCtrl::slotFailedKDEPreview(const KFileItem& item)
{
    procressKDEPreview(item);
}

void CameraThumbsCtrl::procressKDEPreview(const KFileItem& item, const QPixmap& pix)
{
    QString file   = item.url().fileName();
    QString folder = item.url().toLocalFile().remove(QString("/") + file);
    QImage thumb;

    if (pix.isNull())
    {
        // This call must be run outside Camera Controller thread.
        thumb = d->controller->mimeTypeThumbnail(file).toImage();
        kDebug() << "Failed thumb from KDE Preview : " << item.url();
    }
    else
    {
        thumb = pix.toImage();
        kDebug() << "Got thumb from KDE Preview : " << item.url();
    }

    emit signalThumb(folder, file, thumb);
    d->pendingItems.removeAll(item.url());
}

}  // namespace Digikam
