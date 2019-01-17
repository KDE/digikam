/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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

#include "facepreviewloader.h"

// Local includes

#include "digikam_debug.h"

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

} // namespace Digikam
