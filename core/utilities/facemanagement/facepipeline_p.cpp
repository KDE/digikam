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

#include "facepipeline.h"
#include "facepipeline_p.h"

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

FacePipelineExtendedPackage::Ptr PackageLoadingDescriptionList::take(const LoadingDescription& description)
{
    FacePipelineExtendedPackage::Ptr                  package;
    QList<FacePipelineExtendedPackage::Ptr>::iterator it;

    for (it = begin() ; it != end() ; ++it)
    {
        if (*(*it) == description)
        {
            package = *it;
            erase(it);
            break;
        }
    }

    return package;
}

// ----------------------------------------------------------------------------------------

ParallelPipes::ParallelPipes()
    : m_currentIndex(0)
{
}

ParallelPipes::~ParallelPipes()
{
    foreach (WorkerObject* const object, m_workers)
    {
        delete object;
    }
}

void ParallelPipes::schedule()
{
    foreach (WorkerObject* const object, m_workers)
    {
        object->schedule();
    }
}

void ParallelPipes::deactivate(WorkerObject::DeactivatingMode mode)
{
    foreach (WorkerObject* const object, m_workers)
    {
        object->deactivate(mode);
    }
}

void ParallelPipes::wait()
{
    foreach (WorkerObject* const object, m_workers)
    {
        object->wait();
    }
}

void ParallelPipes::setPriority(QThread::Priority priority)
{
    foreach (WorkerObject* const object, m_workers)
    {
        object->setPriority(priority);
    }
}

void ParallelPipes::add(WorkerObject* const worker)
{
    QByteArray normalizedSignature = QMetaObject::normalizedSignature("process(FacePipelineExtendedPackage::Ptr)");
    int methodIndex                = worker->metaObject()->indexOfMethod(normalizedSignature.constData());

    if (methodIndex == -1)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Object" << worker << "does not have a slot"
                                     << normalizedSignature << " - cannot use for processing.";
        return;
    }

    m_workers << worker;
    m_methods << worker->metaObject()->method(methodIndex);

    // collect the worker's signals and bundle them to our single signal, which is further connected
    connect(worker, SIGNAL(processed(FacePipelineExtendedPackage::Ptr)),
            this, SIGNAL(processed(FacePipelineExtendedPackage::Ptr)));
}

void ParallelPipes::process(FacePipelineExtendedPackage::Ptr package)
{
    // Here, we send the package to one of the workers, in turn
    m_methods.at(m_currentIndex).invoke(m_workers.at(m_currentIndex), Qt::QueuedConnection,
                                        Q_ARG(FacePipelineExtendedPackage::Ptr, package));

    if (++m_currentIndex == m_workers.size())
    {
        m_currentIndex = 0;
    }
}

// ----------------------------------------------------------------------------------------

ScanStateFilter::ScanStateFilter(FacePipeline::FilterMode mode, FacePipeline::Private* const d)
    : d(d),
      mode(mode)
{
    connect(this, SIGNAL(infosToDispatch()),
            this, SLOT(dispatch()));
}

FacePipelineExtendedPackage::Ptr ScanStateFilter::filter(const ImageInfo& info)
{
    FaceUtils utils;

    switch (mode)
    {
        case FacePipeline::ScanAll:
        {
            return d->buildPackage(info);
        }

        case FacePipeline::SkipAlreadyScanned:
        {
            if (!utils.hasBeenScanned(info))
            {
                return d->buildPackage(info);
            }

            break;
        }

        case FacePipeline::ReadUnconfirmedFaces:
        case FacePipeline::ReadFacesForTraining:
        case FacePipeline::ReadConfirmedFaces:
        {
            QList<FaceTagsIface> databaseFaces;

            if (mode == FacePipeline::ReadUnconfirmedFaces)
            {
                databaseFaces = utils.unconfirmedFaceTagsIfaces(info.id());

            }
            else if (mode == FacePipeline::ReadFacesForTraining)
            {
                databaseFaces = utils.databaseFacesForTraining(info.id());
            }
            else
            {
                databaseFaces = utils.confirmedFaceTagsIfaces(info.id());
            }

            if (!databaseFaces.isEmpty())
            {
                FacePipelineExtendedPackage::Ptr package = d->buildPackage(info);
                package->databaseFaces                   = databaseFaces;
                //qCDebug(DIGIKAM_GENERAL_LOG) << "Prepared package with" << databaseFaces.size();
                package->databaseFaces.setRole(FacePipelineFaceTagsIface::ReadFromDatabase);

                if (tasks)
                {
                    package->databaseFaces.setRole(tasks);
                }

                return package;
            }

            break;
        }
    }

    return FacePipelineExtendedPackage::Ptr();
}

void ScanStateFilter::process(const QList<ImageInfo>& infos)
{
    QMutexLocker lock(threadMutex());
    toFilter << infos;
    //qCDebug(DIGIKAM_GENERAL_LOG) << "Received" << infos.size() << "images for filtering";
    start(lock);
}

void ScanStateFilter::process(const ImageInfo& info)
{
    QMutexLocker lock(threadMutex());
    toFilter << info;
    start(lock);
}

void ScanStateFilter::run()
{
    while (runningFlag())
    {
        // get todo list
        QList<ImageInfo> todo;
        {
            QMutexLocker lock(threadMutex());

            if (!toFilter.isEmpty())
            {
                todo = toFilter;
                toFilter.clear();
            }
            else
            {
                stop(lock);
            }
        }

        // process list
        if (!todo.isEmpty())
        {
            QList<FacePipelineExtendedPackage::Ptr> send;
            QList<ImageInfo> skip;

            foreach (const ImageInfo& info, todo)
            {
                FacePipelineExtendedPackage::Ptr package = filter(info);

                if (package)
                {
                    send << package;
                }
                else
                {
                    skip << info;
                }
            }

            //qCDebug(DIGIKAM_GENERAL_LOG) << "Filtered" << todo.size() << "images, send" << send.size() << "skip" << skip.size();

            {
                QMutexLocker lock(threadMutex());
                toSend      << send;
                toBeSkipped << skip;
            }

            emit infosToDispatch();
        }
    }
}

void ScanStateFilter::dispatch()
{
    QList<FacePipelineExtendedPackage::Ptr> send;

    QList<ImageInfo> skip;
    {
        QMutexLocker lock(threadMutex());
        send = toSend;
        toSend.clear();
        skip = toBeSkipped;
        toBeSkipped.clear();
    }

    //qCDebug(DIGIKAM_GENERAL_LOG) << "Dispatching, sending" << send.size() << "skipping" << skip.size();
    if (!skip.isEmpty())
    {
        d->skipFromFilter(skip);
    }

    if (!send.isEmpty())
    {
        d->sendFromFilter(send);
    }
}

// ----------------------------------------------------------------------------------------

PreviewLoader::PreviewLoader(FacePipeline::Private* const d)
    : d(d)
{
    // upper limit for memory cost
    maximumSentOutPackages = qMin(QThread::idealThreadCount(), 5);

    // this is crucial! Per default, only the last added image will be loaded
    setLoadingPolicy(PreviewLoadThread::LoadingPolicySimpleAppend);

    connect(this, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
            this, SLOT(slotImageLoaded(LoadingDescription,DImg)));
}

void PreviewLoader::cancel()
{
    stopAllTasks();
    scheduledPackages.clear();
}

void PreviewLoader::process(FacePipelineExtendedPackage::Ptr package)
{
    if (!package->image.isNull())
    {
        emit processed(package);
        return;
    }

    scheduledPackages << package;
    loadFastButLarge(package->filePath, 1600);
    //load(package->filePath, 800, MetadataSettings::instance()->settings().exifRotate);
    //loadHighQuality(package->filePath, MetadataSettings::instance()->settings().exifRotate);

    checkRestart();
}

void PreviewLoader::slotImageLoaded(const LoadingDescription& loadingDescription, const DImg& img)
{
    FacePipelineExtendedPackage::Ptr package = scheduledPackages.take(loadingDescription);

    if (!package)
    {
        return;
    }

    // Avoid congestion before detection or recognition by pausing the thread.
    // We are throwing around serious amounts of memory!
    //qCDebug(DIGIKAM_GENERAL_LOG) << "sent out packages:" << d->packagesOnTheRoad - scheduledPackages.size() << "scheduled:" << scheduledPackages.size();
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

bool PreviewLoader::sentOutLimitReached()
{
    int packagesInTheFollowingPipeline = d->packagesOnTheRoad - scheduledPackages.size();

    return (packagesInTheFollowingPipeline > maximumSentOutPackages);
}

void PreviewLoader::checkRestart()
{
    if (!sentOutLimitReached() && !scheduledPackages.isEmpty())
    {
        start();
    }
}

// ----------------------------------------------------------------------------------------

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

FaceImageRetriever::FaceImageRetriever(FacePipeline::Private* const d)
    : catcher(new ThumbnailImageCatcher(d->createThumbnailLoadThread(), d))
{
}

ThumbnailImageCatcher* FaceImageRetriever::thumbnailCatcher() const
{
    return catcher;
}

void FaceImageRetriever::cancel()
{
    catcher->cancel();
}

QList<QImage> FaceImageRetriever::getDetails(const DImg& src, const QList<QRectF>& rects) const
{
    QList<QImage> images;

    foreach (const QRectF& rect, rects)
    {
        images << src.copyQImage(rect);
    }

    return images;
}

QList<QImage> FaceImageRetriever::getDetails(const DImg& src, const QList<FaceTagsIface>& faces) const
{
    QList<QImage> images;

    foreach (const FaceTagsIface& face, faces)
    {
        QRect rect = TagRegion::mapFromOriginalSize(src, face.region().toRect());
        images << src.copyQImage(rect);
    }

    return images;
}

QList<QImage> FaceImageRetriever::getThumbnails(const QString& filePath, const QList<FaceTagsIface>& faces) const
{
    Q_UNUSED(filePath)
    thumbnailCatcher()->setActive(true);

    foreach (const FaceTagsIface& face, faces)
    {
        QRect rect = face.region().toRect();
        catcher->thread()->find(ImageInfo::thumbnailIdentifier(face.imageId()), rect);
        catcher->enqueue();
    }

    QList<QImage> images = catcher->waitForThumbnails();
    thumbnailCatcher()->setActive(false);

    return images;
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

        // OverwriteUnconfirmed means that a new scan discared unconfirmed results of previous scans
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

                // NOTE: See bug #338485 : check if index is not outside of containe size.
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
        // Unimplemented. Would be needed if we use a backend with a "holistic" approach that needs all images to train.
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
    QList<int> identities;
    QList<Identity> identitySet;
    FaceUtils utils;

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

FacePipeline::Private::Private(FacePipeline* const q)
    : q(q)
{
    databaseFilter         = 0;
    previewThread          = 0;
    detectionWorker        = 0;
    parallelDetectors      = 0;
    recognitionWorker      = 0;
    databaseWriter         = 0;
    trainer                = 0;
    detectionBenchmarker   = 0;
    recognitionBenchmarker = 0;
    priority               = QThread::LowPriority;
    started                = false;
    infosForFiltering      = 0;
    packagesOnTheRoad      = 0;
    maxPackagesOnTheRoad   = 50;
    totalPackagesAdded     = 0;
}

void FacePipeline::Private::processBatch(const QList<ImageInfo>& infos)
{
    if (databaseFilter)
    {
        infosForFiltering += infos.size();
        databaseFilter->process(infos);
    }
    else
    {
        foreach (const ImageInfo& info, infos)
        {
            send(buildPackage(info));
        }
    }
}

// called by filter
void FacePipeline::Private::sendFromFilter(const QList<FacePipelineExtendedPackage::Ptr>& packages)
{
    infosForFiltering -= packages.size();

    foreach (const FacePipelineExtendedPackage::Ptr& package, packages)
    {
        send(package);
    }
}

// called by filter
void FacePipeline::Private::skipFromFilter(const QList<ImageInfo>& infosForSkipping)
{
    infosForFiltering -= infosForSkipping.size();

    emit q->skipped(infosForSkipping);

    // everything skipped?
    checkFinished();
}

FacePipelineExtendedPackage::Ptr FacePipeline::Private::filterOrBuildPackage(const ImageInfo& info)
{
    if (databaseFilter)
    {
        return databaseFilter->filter(info);
    }
    else
    {
        return buildPackage(info);
    }
}

FacePipelineExtendedPackage::Ptr FacePipeline::Private::buildPackage(const ImageInfo& info)
{
    FacePipelineExtendedPackage::Ptr package(new FacePipelineExtendedPackage);
    package->info     = info;
    package->filePath = info.filePath();

    return package;
}

FacePipelineExtendedPackage::Ptr FacePipeline::Private::buildPackage(const ImageInfo& info,
                                                                     const FacePipelineFaceTagsIface& face,
                                                                     const DImg& image)
{
    FacePipelineFaceTagsIfaceList faces;
    faces << face;
    return buildPackage(info, faces, image);
}

FacePipelineExtendedPackage::Ptr FacePipeline::Private::buildPackage(const ImageInfo& info,
                                                                     const FacePipelineFaceTagsIfaceList& faces,
                                                                     const DImg& image)
{
    FacePipelineExtendedPackage::Ptr package = buildPackage(info);
    package->databaseFaces                   = faces;
    package->image                           = image;
    return package;
}

void FacePipeline::Private::send(FacePipelineExtendedPackage::Ptr package)
{
    start();
    ++totalPackagesAdded;
    emit q->processing(*package);

    if (senderFlowControl(package))
    {
        ++packagesOnTheRoad;
        emit startProcess(package);
    }
}

void FacePipeline::Private::finishProcess(FacePipelineExtendedPackage::Ptr package)
{
    packagesOnTheRoad--;
    emit q->processed(*package);
    emit q->progressValueChanged(float(packagesOnTheRoad + delayedPackages.size()) / totalPackagesAdded);
    package = 0;

    if (previewThread)
    {
        previewThread->checkRestart();
    }

    receiverFlowControl();

    checkFinished();
}

bool FacePipeline::Private::senderFlowControl(FacePipelineExtendedPackage::Ptr package)
{
    if (packagesOnTheRoad > maxPackagesOnTheRoad)
    {
        delayedPackages << package;
        return false;
    }

    return true;
}

void FacePipeline::Private::receiverFlowControl()
{
    if (!delayedPackages.isEmpty() && packagesOnTheRoad <= maxPackagesOnTheRoad)
    {
        --totalPackagesAdded; // do not add twice
        send(delayedPackages.takeFirst());
    }
}

bool FacePipeline::Private::hasFinished()
{
    return !packagesOnTheRoad && !infosForFiltering;
}

void FacePipeline::Private::checkFinished()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Check for finish: " << packagesOnTheRoad << "packages,"
                                 << infosForFiltering << "infos to filter, hasFinished()" << hasFinished();

    if (hasFinished())
    {
        totalPackagesAdded = 0;
        emit q->finished();
        // stop threads
        stop();
    }
}

void FacePipeline::Private::start()
{
    if (started)
    {
        return;
    }

    emit q->scheduled();

    WorkerObject*  workerObject = 0;
    ParallelPipes* pipes        = 0;

    foreach (QObject* const element, pipeline)
    {
        if ((workerObject = qobject_cast<WorkerObject*>(element)))
        {
            workerObject->schedule();
        }
        else if ((pipes = qobject_cast<ParallelPipes*>(element)))
        {
            pipes->schedule();
        }
    }

    started = true;
    emit q->started(i18n("Applying face changes"));
}

void FacePipeline::Private::stop()
{
    if (!started)
    {
        return;
    }

    if (previewThread)
    {
        previewThread->cancel();
    }

    foreach (ThumbnailLoadThread* const thread, thumbnailLoadThreads)
    {
        thread->stopAllTasks();
    }

    WorkerObject* workerObject = 0;
    ParallelPipes* pipes       = 0;
    DynamicThread* thread      = 0;

    foreach (QObject* const element, pipeline)
    {
        if ((workerObject = qobject_cast<WorkerObject*>(element)))
        {
            workerObject->deactivate();
        }
        else if ((pipes = qobject_cast<ParallelPipes*>(element)))
        {
            pipes->deactivate();
        }
        else if ((thread = qobject_cast<DynamicThread*>(element)))
        {
            thread->stop();
        }
    }

    started = false;
}

void FacePipeline::Private::wait()
{
    if (!started)
    {
        return;
    }

    if (previewThread)
    {
        previewThread->wait();
    }

    foreach (ThumbnailLoadThread* const thread, thumbnailLoadThreads)
    {
        thread->wait();
    }

    WorkerObject* workerObject = 0;
    ParallelPipes* pipes       = 0;
    DynamicThread* thread      = 0;

    foreach (QObject* const element, pipeline)
    {
        if ((workerObject = qobject_cast<WorkerObject*>(element)))
        {
            workerObject->wait();
        }
        else if ((pipes = qobject_cast<ParallelPipes*>(element)))
        {
            pipes->wait();
        }
        else if ((thread = qobject_cast<DynamicThread*>(element)))
        {
            thread->wait();
        }
    }

    started = false;
}

void FacePipeline::Private::applyPriority()
{
    WorkerObject*  workerObject = 0;
    ParallelPipes* pipes        = 0;

    foreach (QObject* const element, pipeline)
    {
        if ((workerObject = qobject_cast<WorkerObject*>(element)))
        {
            workerObject->setPriority(priority);
        }
        else if ((pipes = qobject_cast<ParallelPipes*>(element)))
        {
            pipes->setPriority(priority);
        }
    }

    foreach (ThumbnailLoadThread* const thread, thumbnailLoadThreads)
    {
        thread->setPriority(priority);
    }
}

ThumbnailLoadThread* FacePipeline::Private::createThumbnailLoadThread()
{
    ThumbnailLoadThread* const thumbnailLoadThread = new ThumbnailLoadThread;
    thumbnailLoadThread->setPixmapRequested(false);
    thumbnailLoadThread->setThumbnailSize(ThumbnailLoadThread::maximumThumbnailSize());
    // Image::recommendedSizeForRecognition()
    thumbnailLoadThread->setPriority(priority);

    thumbnailLoadThreads << thumbnailLoadThread;
    return thumbnailLoadThread;
}

} // namespace Digikam
