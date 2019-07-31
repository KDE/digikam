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

#include "faceworkers.h"

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DetectionWorker::DetectionWorker(FacePipeline::Private* const d)
    : d(d)
{
}

void DetectionWorker::process(FacePipelineExtendedPackage::Ptr package)
{
/*
    QImage detectionImage  = scaleForDetection(package->image);
    package->detectedFaces = detector.detectFaces(detectionImage, package->image.originalSize());
*/
    package->detectedFaces = detector.detectFaces(package->filePath);

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

// ----------------------------------------------------------------------------------------

class Q_DECL_HIDDEN MapListTrainingDataProvider : public TrainingDataProvider
{
public:

    MapListTrainingDataProvider()
    {
    }

    ImageListProvider* newImages(const Identity& identity)
    {
        if (imagesToTrain.contains(identity.id()))
        {
            QListImageListProvider& provider = imagesToTrain[identity.id()];
            provider.reset();

            return &provider;
        }

        return &empty;
    }

    ImageListProvider* images(const Identity&)
    {
        // Not implemented. Would be needed if we use a backend with a "holistic" approach that needs all images to train.
        return &empty;
    }

public:

    EmptyImageListProvider            empty;
    QMap<int, QListImageListProvider> imagesToTrain;
};

// ----------------------------------------------------------------------------------------

Trainer::Trainer(FacePipeline::Private* const d)
    : imageRetriever(d),
      d(d)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Face Detection Dialog"));

    RecognitionDatabase::RecognizeAlgorithm algo =
            (RecognitionDatabase::RecognizeAlgorithm)group.readEntry(QLatin1String("Recognize Algorithm"),
                                                                     (int)RecognitionDatabase::RecognizeAlgorithm::LBP);
    database.activeFaceRecognizer(algo);
}

void Trainer::process(FacePipelineExtendedPackage::Ptr package)
{
    //qCDebug(DIGIKAM_GENERAL_LOG) << "Trainer: processing one package";
    // Get a list of faces with type FaceForTraining (probably type is ConfirmedFace)

    QList<FaceTagsIface> toTrain;
    QList<int>           identities;
    QList<Identity>      identitySet;
    FaceUtils            utils;

    foreach (const FacePipelineFaceTagsIface& face, package->databaseFaces)
    {
        if (face.roles & FacePipelineFaceTagsIface::ForTraining)
        {
            FaceTagsIface dbFace = face;
            dbFace.setType(FaceTagsIface::FaceForTraining);
            toTrain << dbFace;

            Identity identity    = utils.identityForTag(dbFace.tagId(), database);

            identities  << identity.id();

            if (!identitySet.contains(identity))
            {
                identitySet << identity;
            }
        }
    }

    if (!toTrain.isEmpty())
    {
        QList<QImage> images;

        if (package->image.isNull())
        {
            images = imageRetriever.getThumbnails(package->filePath, toTrain);
        }
        else
        {
            images = imageRetriever.getDetails(package->image, toTrain);
        }

        MapListTrainingDataProvider provider;

        // Group images by identity
        for (int i = 0 ; i < toTrain.size() ; ++i)
        {
            provider.imagesToTrain[identities[i]].list << images[i];
        }

        database.train(identitySet, &provider, QLatin1String("digikam"));
    }

    utils.removeFaces(toTrain);
    package->databaseFaces.replaceRole(FacePipelineFaceTagsIface::ForTraining, FacePipelineFaceTagsIface::Trained);

    package->processFlags |= FacePipelinePackage::ProcessedByTrainer;
    emit processed(package);
}

void Trainer::aboutToDeactivate()
{
    imageRetriever.cancel();
}

// ----------------------------------------------------------------------------------------

DatabaseWriter::DatabaseWriter(FacePipeline::WriteMode mode, FacePipeline::Private* const d)
    : mode(mode),
      thumbnailLoadThread(d->createThumbnailLoadThread()),
      d(d)
{
}

void DatabaseWriter::process(FacePipelineExtendedPackage::Ptr package)
{
    if (package->databaseFaces.isEmpty())
    {
        // Detection / Recognition
        FaceUtils utils;

        // OverwriteUnconfirmed means that a new scan discarded unconfirmed results of previous scans
        // (assuming at least equal or new, better methodology is in use compared to the previous scan)
        if (mode == FacePipeline::OverwriteUnconfirmed && (package->processFlags & FacePipelinePackage::ProcessedByDetector))
        {
            QList<FaceTagsIface> oldEntries = utils.unconfirmedFaceTagsIfaces(package->info.id());
            qCDebug(DIGIKAM_GENERAL_LOG) << "Removing old entries" << oldEntries;
            utils.removeFaces(oldEntries);
        }

        // mark the whole image as scanned-for-faces
        utils.markAsScanned(package->info);

        if (!package->info.isNull() && !package->detectedFaces.isEmpty())
        {
            package->databaseFaces = utils.writeUnconfirmedResults(package->info.id(),
                                                                   package->detectedFaces,
                                                                   package->recognitionResults,
                                                                   package->image.originalSize());
            package->databaseFaces.setRole(FacePipelineFaceTagsIface::DetectedFromImage);

            if (!package->image.isNull())
            {
                utils.storeThumbnails(thumbnailLoadThread, package->filePath,
                                      package->databaseFaces.toFaceTagsIfaceList(), package->image);
            }
        }
    }
    else if (package->processFlags & FacePipelinePackage::ProcessedByRecognizer)
    {
        FaceUtils utils;

        for (int i = 0 ; i < package->databaseFaces.size() ; ++i)
        {
            if (package->databaseFaces[i].roles & FacePipelineFaceTagsIface::ForRecognition)
            {
                // Allow to overwrite existing recognition with new, possibly valid, "not recognized" status
                int tagId = FaceTags::unknownPersonTagId();

                // NOTE: See bug #338485 : check if index is not outside of container size.
                if (i < package->recognitionResults.size() &&
                    !package->recognitionResults[i].isNull())
                {
                    // Only perform this call if recognition as results, to prevent crash in QMap. See bug #335624

                    tagId = FaceTags::getOrCreateTagForIdentity(package->recognitionResults[i].attributesMap());
                }

                package->databaseFaces[i]        = FacePipelineFaceTagsIface(utils.changeSuggestedName(package->databaseFaces[i], tagId));
                package->databaseFaces[i].roles &= ~FacePipelineFaceTagsIface::ForRecognition;
           }
        }
    }
    else
    {
        // Editing database entries

        FaceUtils utils;
        FacePipelineFaceTagsIfaceList add;
        FacePipelineFaceTagsIfaceList::iterator it;

        for (it = package->databaseFaces.begin() ; it != package->databaseFaces.end() ; ++it)
        {
            if (it->roles & FacePipelineFaceTagsIface::ForConfirmation)
            {
                FacePipelineFaceTagsIface confirmed = FacePipelineFaceTagsIface(utils.confirmName(*it, it->assignedTagId, it->assignedRegion));
                confirmed.roles                    |= FacePipelineFaceTagsIface::Confirmed | FacePipelineFaceTagsIface::ForTraining;
                add << confirmed;
            }
            else if (it->roles & FacePipelineFaceTagsIface::ForEditing)
            {
                if (it->isNull())
                {
                    // add Manually
                    FaceTagsIface newFace = utils.unconfirmedEntry(package->info.id(), it->assignedTagId, it->assignedRegion);
                    utils.addManually(newFace);
                    add << FacePipelineFaceTagsIface(newFace);
                }
                else if (it->assignedRegion.isValid())
                {
                    add << FacePipelineFaceTagsIface(utils.changeRegion(*it, it->assignedRegion));
                    // not implemented: changing tag id
                }
                else
                {
                    utils.removeFace(*it);
                }

                it->roles &= ~FacePipelineFaceTagsIface::ForEditing;
                it->roles |= FacePipelineFaceTagsIface::Edited;
            }

            // Training is done by trainer
        }

        if (!package->image.isNull())
        {
            utils.storeThumbnails(thumbnailLoadThread, package->filePath, add.toFaceTagsIfaceList(), package->image);
        }

        package->databaseFaces << add;
    }

    package->processFlags |= FacePipelinePackage::WrittenToDatabase;

    emit processed(package);
}

} // namespace Digikam
