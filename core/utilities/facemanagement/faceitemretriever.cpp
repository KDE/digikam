/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-03
 * Description : Integrated, multithread face detection / recognition
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "faceitemretriever.h"

// Qt includes

#include <QMetaObject>
#include <QMutexLocker>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "loadingdescription.h"
#include "metaenginesettings.h"
#include "tagscache.h"
#include "threadmanager.h"
#include "facebenchmarkers.h"
#include "faceworkers.h"

namespace Digikam
{

FacePreviewLoader::FacePreviewLoader(FacePipeline::Private* const d)
    : d(d)
{
    // upper limit for memory cost
    maximumSentOutPackages = qMin(QThread::idealThreadCount(), 5);

    // this is crucial! Per default, only the last added image will be loaded
    setLoadingPolicy(PreviewLoadThread::LoadingPolicySimpleAppend);

    connect(this, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
            this, SLOT(slotImageLoaded(LoadingDescription,DImg)));
}

void FacePreviewLoader::cancel()
{
    stopAllTasks();
    scheduledPackages.clear();
}

void FacePreviewLoader::process(FacePipelineExtendedPackage::Ptr package)
{
    if (!package->image.isNull())
    {
        emit processed(package);
        return;
    }

    scheduledPackages << package;
    loadFastButLarge(package->filePath, 1600);
    //load(package->filePath, 800, MetaEngineSettings::instance()->settings().exifRotate);
    //loadHighQuality(package->filePath, MetaEngineSettings::instance()->settings().exifRotate);

    checkRestart();
}

void FacePreviewLoader::slotImageLoaded(const LoadingDescription& loadingDescription, const DImg& img)
{
    FacePipelineExtendedPackage::Ptr package = scheduledPackages.take(loadingDescription);

    if (!package)
    {
        return;
    }

    // Avoid congestion before detection or recognition by pausing the thread.
    // We are throwing around serious amounts of memory!
/*
    //qCDebug(DIGIKAM_GENERAL_LOG) << "sent out packages:"
                                   << d->packagesOnTheRoad - scheduledPackages.size()
                                   << "scheduled:" << scheduledPackages.size();
*/
    if (sentOutLimitReached() && !scheduledPackages.isEmpty())
    {
        stop();
    }

    if (img.isNull())
    {
        d->finishProcess(package);
        return;
    }

    package->image         = img;
    package->processFlags |= FacePipelinePackage::PreviewImageLoaded;
    emit processed(package);
}

bool FacePreviewLoader::sentOutLimitReached()
{
    int packagesInTheFollowingPipeline = d->packagesOnTheRoad - scheduledPackages.size();

    return (packagesInTheFollowingPipeline > maximumSentOutPackages);
}

void FacePreviewLoader::checkRestart()
{
    if (!sentOutLimitReached() && !scheduledPackages.isEmpty())
    {
        start();
    }
}

// ----------------------------------------------------------------------------------------

FaceItemRetriever::FaceItemRetriever(FacePipeline::Private* const d)
    : catcher(new ThumbnailImageCatcher(d->createThumbnailLoadThread(), d))
{
}

ThumbnailImageCatcher* FaceItemRetriever::thumbnailCatcher() const
{
    return catcher;
}

void FaceItemRetriever::cancel()
{
    catcher->cancel();
}

QList<QImage> FaceItemRetriever::getDetails(const DImg& src, const QList<QRectF>& rects) const
{
    QList<QImage> images;

    foreach (const QRectF& rect, rects)
    {
        images << src.copyQImage(rect);
    }

    return images;
}

QList<QImage> FaceItemRetriever::getDetails(const DImg& src, const QList<FaceTagsIface>& faces) const
{
    QList<QImage> images;

    foreach (const FaceTagsIface& face, faces)
    {
        QRect rect = TagRegion::mapFromOriginalSize(src, face.region().toRect());
        images << src.copyQImage(rect);
    }

    return images;
}

QList<QImage> FaceItemRetriever::getThumbnails(const QString& filePath, const QList<FaceTagsIface>& faces) const
{
    Q_UNUSED(filePath)
    thumbnailCatcher()->setActive(true);

    foreach (const FaceTagsIface& face, faces)
    {
        QRect rect = face.region().toRect();
        catcher->thread()->find(ItemInfo::thumbnailIdentifier(face.imageId()), rect);
        catcher->enqueue();
    }

    QList<QImage> images = catcher->waitForThumbnails();
    thumbnailCatcher()->setActive(false);

    return images;
}

} // namespace Digikam
