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

#include "faceworkers.h"

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
#include "metadatasettings.h"
#include "tagscache.h"
#include "threadmanager.h"
#include "facebenchmarkers.h"

namespace Digikam
{

DetectionWorker::DetectionWorker(FacePipeline::Private* const d)
    : d(d)
{
}

void DetectionWorker::process(FacePipelineExtendedPackage::Ptr package)
{
    QImage detectionImage  = scaleForDetection(package->image);
    package->detectedFaces = detector.detectFaces(detectionImage, package->image.originalSize());

    qCDebug(DIGIKAM_GENERAL_LOG) << "Found" << package->detectedFaces.size() << "faces in"
                                 << package->info.name() << package->image.size()
                                 << package->image.originalSize();

    package->processFlags |= FacePipelinePackage::ProcessedByDetector;

    emit processed(package);
}

QImage DetectionWorker::scaleForDetection(const DImg& image) const
{
    int recommendedSize = detector.recommendedImageSize(image.size());

    if (qMax(image.width(), image.height()) > (uint)recommendedSize)
    {
        return image.smoothScale(recommendedSize, recommendedSize, Qt::KeepAspectRatio).copyQImage();
    }

    return image.copyQImage();
}

void DetectionWorker::setAccuracy(double accuracy)
{
    QVariantMap params;
    params[QLatin1String("accuracy")]    = accuracy;
    params[QLatin1String("specificity")] = 0.8; //TODO: add UI for sensitivity - specificity
    detector.setParameters(params);
}

// ----------------------------------------------------------------------------------------

RecognitionWorker::RecognitionWorker(FacePipeline::Private* const d)
    : imageRetriever(d),
      d(d)
{
}

void RecognitionWorker::activeFaceRecognizer(RecognitionDatabase::RecognizeAlgorithm algorithmType)
{
    database.activeFaceRecognizer(algorithmType);
}

void RecognitionWorker::process(FacePipelineExtendedPackage::Ptr package)
{
    FaceUtils     utils;
    QList<QImage> images;

    if (package->processFlags & FacePipelinePackage::ProcessedByDetector)
    {
        // assume we have an image
        images = imageRetriever.getDetails(package->image, package->detectedFaces);
    }
    else if (!package->databaseFaces.isEmpty())
    {
        images = imageRetriever.getThumbnails(package->filePath, package->databaseFaces.toFaceTagsIfaceList());
    }

    package->recognitionResults  = database.recognizeFaces(images);
    package->processFlags       |= FacePipelinePackage::ProcessedByRecognizer;

    emit processed(package);
}

void RecognitionWorker::setThreshold(double threshold)
{
    database.setParameter(QLatin1String("threshold"), threshold);
}

void RecognitionWorker::aboutToDeactivate()
{
    imageRetriever.cancel();
}

} // namespace Digikam
