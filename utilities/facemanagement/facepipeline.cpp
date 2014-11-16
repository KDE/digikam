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

#include "facepipeline.moc"
#include "facepipeline_p.moc"

// Qt includes

#include <QMetaObject>
#include <QMutexLocker>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// libkface includes

#include <libkface/version.h>

// Local includes

#include "loadingdescription.h"
#include "metadatasettings.h"
#include "tagscache.h"
#include "threadmanager.h"

namespace Digikam
{

FacePipelineDatabaseFace::FacePipelineDatabaseFace()
    : roles(NoRole), assignedTagId(0)
{
}

FacePipelineDatabaseFace::FacePipelineDatabaseFace(const DatabaseFace& face)
    : DatabaseFace(face), roles(NoRole), assignedTagId(0)
{
}

FacePipelineDatabaseFaceList::FacePipelineDatabaseFaceList()
{
}

FacePipelineDatabaseFaceList::FacePipelineDatabaseFaceList(const QList<DatabaseFace>& faces)
{
    operator=(faces);
}

FacePipelineDatabaseFaceList& FacePipelineDatabaseFaceList::operator=(const QList<DatabaseFace>& faces)
{
    foreach(const DatabaseFace& face, faces)
    {
        operator<<(face);
    }

    return *this;
}

void FacePipelineDatabaseFaceList::setRole(FacePipelineDatabaseFace::Roles role)
{
    for (iterator it = begin(); it != end(); ++it)
    {
        it->roles |= role;
    }
}

void FacePipelineDatabaseFaceList::clearRole(FacePipelineDatabaseFace::Roles role)
{
    for (iterator it = begin(); it != end(); ++it)
    {
        it->roles &= ~role;
    }
}

void FacePipelineDatabaseFaceList::replaceRole(FacePipelineDatabaseFace::Roles remove, FacePipelineDatabaseFace::Roles add)
{
    for (iterator it = begin(); it != end(); ++it)
    {
        if (it->roles & remove)
        {
            it->roles &= ~remove;
            it->roles |= add;
        }
    }
}

QList<DatabaseFace> FacePipelineDatabaseFaceList::toDatabaseFaceList() const
{
    QList<DatabaseFace> faces;

    for (const_iterator it = constBegin(); it != constEnd(); ++it)
    {
        faces << *it;
    }

    return faces;
}

FacePipelineDatabaseFaceList FacePipelineDatabaseFaceList::facesForRole(FacePipelineDatabaseFace::Roles role) const
{
    FacePipelineDatabaseFaceList faces;

    for (const_iterator it = constBegin(); it != constEnd(); ++it)
    {
        if (it->roles & role)
        {
            faces << *it;
        }
    }

    return faces;
}

// ----------------------------------------------------------------------------------------

FacePipelineExtendedPackage::Ptr PackageLoadingDescriptionList::take(const LoadingDescription& description)
{
    FacePipelineExtendedPackage::Ptr                  package;
    QList<FacePipelineExtendedPackage::Ptr>::iterator it;

    for (it = begin(); it != end(); ++it)
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
    foreach(WorkerObject* const object, m_workers)
    {
        delete object;
    }
}

void ParallelPipes::schedule()
{
    foreach(WorkerObject* const object, m_workers)
    {
        object->schedule();
    }
}

void ParallelPipes::deactivate(WorkerObject::DeactivatingMode mode)
{
    foreach(WorkerObject* const object, m_workers)
    {
        object->deactivate(mode);
    }
}

void ParallelPipes::wait()
{
    foreach(WorkerObject* const object, m_workers)
    {
        object->wait();
    }
}

void ParallelPipes::setPriority(QThread::Priority priority)
{
    foreach(WorkerObject* const object, m_workers)
    {
        object->setPriority(priority);
    }
}

void ParallelPipes::add(WorkerObject* const worker)
{
    QByteArray normalizedSignature = QMetaObject::normalizedSignature("process(FacePipelineExtendedPackage::Ptr)");
    int methodIndex                = worker->metaObject()->indexOfMethod(normalizedSignature);

    if (methodIndex == -1)
    {
        kError() << "Object" << worker << "does not have a slot" << normalizedSignature << " - cannot use for processing.";
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
    : d(d), mode(mode)
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
            QList<DatabaseFace> databaseFaces;

            if (mode == FacePipeline::ReadUnconfirmedFaces)
            {
                databaseFaces = utils.unconfirmedDatabaseFaces(info.id());

            }
            else if (mode == FacePipeline::ReadFacesForTraining)
            {
                databaseFaces = utils.databaseFacesForTraining(info.id());
            }
            else
            {
                databaseFaces = utils.confirmedDatabaseFaces(info.id());
            }

            if (!databaseFaces.isEmpty())
            {
                FacePipelineExtendedPackage::Ptr package = d->buildPackage(info);
                package->databaseFaces = databaseFaces;
                //kDebug() << "Prepared package with" << databaseFaces.size();
                package->databaseFaces.setRole(FacePipelineDatabaseFace::ReadFromDatabase);

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
    //kDebug() << "Received" << infos.size() << "images for filtering";
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

            foreach(const ImageInfo& info, todo)
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

            //kDebug() << "Filtered" << todo.size() << "images, send" << send.size() << "skip" << skip.size();

            {
                QMutexLocker lock(threadMutex());
                toSend << send;
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

    //kDebug() << "Dispatching, sending" << send.size() << "skipping" << skip.size();
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
    //kDebug() << "sent out packages:" << d->packagesOnTheRoad - scheduledPackages.size() << "scheduled:" << scheduledPackages.size();
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

    kDebug() << "Found" << package->detectedFaces.size() << "faces in" << package->info.name()
             << package->image.size() << package->image.originalSize();

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
    params["accuracy"]    = accuracy;
    params["specificity"] = 0.8; //TODO: add UI for sensitivity - specificity
    detector.setParameters(params);
}

// ----------------------------------------------------------------------------------------

FaceImageRetriever::FaceImageRetriever(FacePipeline::Private* const d)
    : catcher(new ThumbnailImageCatcher(d->createThumbnailLoadThread(), d))
{
}

ThumbnailImageCatcher* FaceImageRetriever::thumbnailCatcher()
{
    return catcher;
}

void FaceImageRetriever::cancel()
{
    catcher->cancel();
}

QList<QImage> FaceImageRetriever::getDetails(const DImg& src, const QList<QRectF>& rects)
{
    QList<QImage> images;

    foreach (const QRectF& rect, rects)
    {
        images << src.copyQImage(rect);
    }

    return images;
}

QList<QImage> FaceImageRetriever::getDetails(const DImg& src, const QList<DatabaseFace>& faces)
{
    QList<QImage> images;

    foreach (const DatabaseFace& face, faces)
    {
        QRect rect = TagRegion::mapFromOriginalSize(src, face.region().toRect());
        images << src.copyQImage(rect);
    }

    return images;
}

QList<QImage> FaceImageRetriever::getThumbnails(const QString& filePath, const QList<DatabaseFace>& faces)
{
    Q_UNUSED(filePath)
    thumbnailCatcher()->setActive(true);

    foreach (const DatabaseFace& face, faces)
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
    database = KFaceIface::RecognitionDatabase::addDatabase();
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
        images = imageRetriever.getThumbnails(package->filePath, package->databaseFaces.toDatabaseFaceList());
    }

    package->recognitionResults  = database.recognizeFaces(images);
    package->processFlags       |= FacePipelinePackage::ProcessedByRecognizer;

    emit processed(package);
}

void RecognitionWorker::setThreshold(double threshold)
{
    database.setParameter("threshold", threshold);
}

void RecognitionWorker::aboutToDeactivate()
{
    imageRetriever.cancel();
}

// ----------------------------------------------------------------------------------------

DatabaseWriter::DatabaseWriter(FacePipeline::WriteMode mode, FacePipeline::Private* const d)
    : mode(mode), thumbnailLoadThread(d->createThumbnailLoadThread()), d(d)
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
            QList<DatabaseFace> oldEntries = utils.unconfirmedDatabaseFaces(package->info.id());
            kDebug() << "Removing old entries" << oldEntries;
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
            package->databaseFaces.setRole(FacePipelineDatabaseFace::DetectedFromImage);

            if (!package->image.isNull())
            {
                utils.storeThumbnails(thumbnailLoadThread, package->filePath,
                                      package->databaseFaces.toDatabaseFaceList(), package->image);
            }
        }
    }
    else if (package->processFlags & FacePipelinePackage::ProcessedByRecognizer)
    {
        FaceUtils utils;

        for (int i=0; i<package->databaseFaces.size(); i++)
        {
            if (package->databaseFaces[i].roles & FacePipelineDatabaseFace::ForRecognition)
            {
                // Allow to overwrite existing recognition with new, possibly valid, "not recognized" status
                int tagId = FaceTags::unknownPersonTagId();

                // NOTE: See bug #338485 : check if index is not outside of containe size.
                if (i < package->recognitionResults.size() &&
                    !package->recognitionResults[i].isNull())
                {
                    // Only perform this call if recognition as results, to prevent crash in QMap. See bug #335624

#if KFACE_VERSION >= 0x030500
                    tagId = FaceTags::getOrCreateTagForIdentity(package->recognitionResults[i].attributesMap());
#else
                    tagId = FaceTags::getOrCreateTagForIdentity(package->recognitionResults[i].attributes);
#endif
                }

                package->databaseFaces[i]        = utils.changeSuggestedName(package->databaseFaces[i], tagId);
                package->databaseFaces[i].roles &= ~FacePipelineDatabaseFace::ForRecognition;
           }
        }
    }
    else
    {
        // Editing database entries

        FaceUtils utils;
        FacePipelineDatabaseFaceList add;
        FacePipelineDatabaseFaceList::iterator it;

        for (it = package->databaseFaces.begin(); it != package->databaseFaces.end(); ++it)
        {
            if (it->roles & FacePipelineDatabaseFace::ForConfirmation)
            {
                FacePipelineDatabaseFace confirmed  = utils.confirmName(*it, it->assignedTagId, it->assignedRegion);
                confirmed.roles                    |= FacePipelineDatabaseFace::Confirmed | FacePipelineDatabaseFace::ForTraining;
                add << confirmed;
            }
            else if (it->roles & FacePipelineDatabaseFace::ForEditing)
            {
                if (it->isNull())
                {
                    // add Manually
                    DatabaseFace newFace = utils.unconfirmedEntry(package->info.id(), it->assignedTagId, it->assignedRegion);
                    utils.addManually(newFace);
                    add << newFace;
                }
                else if (it->assignedRegion.isValid())
                {
                    add << utils.changeRegion(*it, it->assignedRegion);
                    // not implemented: changing tag id
                }
                else
                {
                    utils.removeFace(*it);
                }

                it->roles &= ~FacePipelineDatabaseFace::ForEditing;
                it->roles |= FacePipelineDatabaseFace::Edited;
            }

            // Training is done by trainer
        }

        if (!package->image.isNull())
        {
            utils.storeThumbnails(thumbnailLoadThread, package->filePath, add.toDatabaseFaceList(), package->image);
        }

        package->databaseFaces << add;
    }

    package->processFlags |= FacePipelinePackage::WrittenToDatabase;
    emit processed(package);
}

// ----------------------------------------------------------------------------------------

DetectionBenchmarker::DetectionBenchmarker(FacePipeline::Private* const d)
    : totalImages(0),
      faces(0),
      totalPixels(0),
      facePixels(0),

      trueNegativeImages(0),
      falsePositiveImages(0),

      truePositiveFaces(0),
      falseNegativeFaces(0),
      falsePositiveFaces(0),
      d(d)
{
}

void DetectionBenchmarker::process(FacePipelineExtendedPackage::Ptr package)
{
    if (package->databaseFaces.isEmpty())
    {
        // Detection / Recognition
        kDebug() << "Benchmarking image" << package->info.name();

        FaceUtils utils;
        QList<DatabaseFace> groundTruth = utils.databaseFaces(package->info.id());

        QList<DatabaseFace> testedFaces = utils.toDatabaseFaces(package->info.id(), package->detectedFaces,
                                                                package->recognitionResults, package->image.originalSize());

        QList<DatabaseFace> unmatchedTrueFaces   = groundTruth;
        QList<DatabaseFace> unmatchedTestedFaces = testedFaces;
        QList<DatabaseFace> matchedTrueFaces;

        int trueFaces           = groundTruth.size();
        const double minOverlap = 0.75;

        kDebug() << "There are" << trueFaces << "faces to be detected. The detector found" << testedFaces.size();

        ++totalImages;
        faces       += trueFaces;
        totalPixels += package->image.originalSize().width() * package->image.originalSize().height();

        foreach(const DatabaseFace& trueFace, groundTruth)
        {
            ++faces;
            QRect rect  = trueFace.region().toRect();
            facePixels += rect.width() * rect.height();

            foreach(const DatabaseFace& testedFace, testedFaces)
            {
                if (trueFace.region().intersects(testedFace.region(), minOverlap))
                {
                    matchedTrueFaces << trueFace;
                    unmatchedTrueFaces.removeOne(trueFace);
                    break;
                }
            }
        }

        foreach(const DatabaseFace& testedFace, testedFaces)
        {
            foreach(const DatabaseFace& trueFace, groundTruth)
            {
                if (trueFace.region().intersects(testedFace.region(), minOverlap))
                {
                    unmatchedTestedFaces.removeOne(testedFace);
                    break;
                }
            }
        }

        if (groundTruth.isEmpty())
        {
            if (testedFaces.isEmpty())
            {
                ++trueNegativeImages;
            }
            else
            {
                kDebug() << "The image, truly without faces, is false-positive";
                ++falsePositiveImages;
            }
        }

        truePositiveFaces  += matchedTrueFaces.size();
        falseNegativeFaces += unmatchedTrueFaces.size();
        falsePositiveFaces += unmatchedTestedFaces.size();
        kDebug() << "Faces detected correctly:" << matchedTrueFaces.size() << ", faces missed:" << unmatchedTrueFaces.size()
                 << ", faces falsely detected:" << unmatchedTestedFaces.size();
    }

    package->processFlags  |= FacePipelinePackage::WrittenToDatabase;
    emit processed(package);
}

QString DetectionBenchmarker::result() const
{
    kDebug() << "Per-image:" << trueNegativeImages << falsePositiveFaces;
    kDebug() << "Per-face:" << truePositiveFaces << falseNegativeFaces << falsePositiveFaces; // 26 7 1
    int negativeImages = trueNegativeImages + falsePositiveImages;
    int trueFaces      = truePositiveFaces  + falseNegativeFaces;
    QString specificityWarning, sensitivityWarning;

    if (negativeImages < 0.2 * totalImages)
    {
        specificityWarning = QString("<p><b>Note:</b><br/> "
                                     "Only %1 of the %2 test images have <i>no</i> depicted faces. "
                                     "This means the result is cannot be representative; "
                                     "it can only be used to compare preselected collections, "
                                     "and the specificity and false-positive rate have little meaning. </p>")
                .arg(negativeImages).arg(totalImages);
        negativeImages     = qMax(negativeImages, 1);
    }

    if (trueFaces == 0)
    {
        sensitivityWarning = QString("<p><b>Note:</b><br/> "
                                     "No picture in the test collection contained a face. "
                                     "This means that sensitivity and PPV have no meaning and will be zero. </p>");
        trueFaces          = 1;
    }

    // collection properties
    double pixelCoverage     = facePixels / totalPixels;
    // per-image
    double specificity       = double(trueNegativeImages) / negativeImages;
    double falsePositiveRate = double(falsePositiveImages) / negativeImages;
    // per-face
    double sensitivity       = double(truePositiveFaces) / trueFaces;
    double ppv               = double(truePositiveFaces) / (truePositiveFaces + falsePositiveFaces);

    return QString("<p>"
                   "<u>Collection Properties:</u><br/>"
                   "%1 Images <br/>"
                   "%2 Faces <br/>"
                   "%3% of pixels covered by faces <br/>"
                   "</p>"
                   "%8"
                   "%9"
                   "<p>"
                   "<u>Per-Image Performance:</u> <br/>"
                   "Specificity: %4% <br/>"
                   "False-Positive Rate: %5%"
                   "</p>"
                   "<p>"
                   "<u>Per-Face Performance:</u> <br/>"
                   "Sensitivity: %6% <br/>"
                   "Positive Predictive Value: %7% <br/>"
                   "</p>"
                   "<p>"
                   "In other words, if a face is detected as face, it will "
                   "with a probability of %7% truly be a face. "
                   "Of all true faces, %6% will be detected. "
                   "Given face with no images on it, the detector will with a probability "
                   "of %5% falsely find a face on it. "
                   "</p>")
            .arg(totalImages).arg(faces).arg(pixelCoverage * 100, 0, 'f', 1)
            .arg(specificity * 100, 0, 'f', 1).arg(falsePositiveRate * 100, 0, 'f', 1)
            .arg(sensitivity * 100, 0, 'f', 1).arg(ppv * 100, 0, 'f', 1)
            .arg(specificityWarning).arg(sensitivityWarning);
}

// ----------------------------------------------------------------------------------------

RecognitionBenchmarker::Statistics::Statistics()
: knownFaces(0), correctlyRecognized(0)
{}

RecognitionBenchmarker::RecognitionBenchmarker(FacePipeline::Private* const d)
    : d(d)
{
    database = KFaceIface::RecognitionDatabase::addDatabase();
}

QString RecognitionBenchmarker::result() const
{
    int totalImages = 0;
    foreach (const Statistics& stat, results)
    {
        totalImages += stat.knownFaces;
    }

    QString s = QString("<p>"
                        "<u>Collection Properties:</u><br/>"
                        "%1 Images <br/>"
                        "%2 Identities <br/>"
                        "</p><p>").arg(totalImages).arg(results.size());
    for (QMap<int, Statistics>::const_iterator it = results.begin(); it != results.end(); ++it)
    {
        const Statistics& stat = it.value();
        double correctRate = double(stat.correctlyRecognized) / stat.knownFaces;
        s += TagsCache::instance()->tagName(it.key());
        s += QString(": %1 faces, %2 (%3%) correctly recognized<br/>")
            .arg(stat.knownFaces).arg(stat.correctlyRecognized).arg(correctRate * 100);
    }
    s += "</p>";
    return s;
}

void RecognitionBenchmarker::process(FacePipelineExtendedPackage::Ptr package)
{
    FaceUtils utils;

    for (int i=0; i<package->databaseFaces.size(); i++)
    {
        KFaceIface::Identity identity = utils.identityForTag(package->databaseFaces[i].tagId(), database);
        Statistics& result = results[package->databaseFaces[i].tagId()];
        result.knownFaces++;
        if (identity == package->recognitionResults[i])
        {
            result.correctlyRecognized++;
        }
    }
    emit processed(package);
}


// ----------------------------------------------------------------------------------------

class MapListTrainingDataProvider : public KFaceIface::TrainingDataProvider
{
public:

    MapListTrainingDataProvider()
    {
    }

    KFaceIface::ImageListProvider* newImages(const KFaceIface::Identity& identity)
    {
#if KFACE_VERSION >= 0x030500
        if (imagesToTrain.contains(identity.id()))
        {
            KFaceIface::QListImageListProvider& provider = imagesToTrain[identity.id()];
#else
        if (imagesToTrain.contains(identity.id))
        {
            KFaceIface::QListImageListProvider& provider = imagesToTrain[identity.id];
#endif
            provider.reset();
            return &provider;
        }

        return &empty;
    }

    KFaceIface::ImageListProvider* images(const KFaceIface::Identity&)
    {
        // Unimplemented. Would be needed if we use a backend with a "holistic" approach that needs all images to train.
        return &empty;
    }

    KFaceIface::EmptyImageListProvider            empty;
    QMap<int, KFaceIface::QListImageListProvider> imagesToTrain;
};


Trainer::Trainer(FacePipeline::Private* const d)
    : imageRetriever(d),
      d(d)
{
    database = KFaceIface::RecognitionDatabase::addDatabase();
}

void Trainer::process(FacePipelineExtendedPackage::Ptr package)
{
    //kDebug() << "Trainer: processing one package";
    // Get a list of faces with type FaceForTraining (probably type is ConfirmedFace)

    QList<DatabaseFace> toTrain;
    QList<int> identities;
    QList<KFaceIface::Identity> identitySet;
    FaceUtils utils;

    foreach(const FacePipelineDatabaseFace& face, package->databaseFaces)
    {
        if (face.roles & FacePipelineDatabaseFace::ForTraining)
        {
            DatabaseFace dbFace = face;
            dbFace.setType(DatabaseFace::FaceForTraining);
            toTrain << dbFace;

            KFaceIface::Identity identity = utils.identityForTag(dbFace.tagId(), database);

#if KFACE_VERSION >= 0x030500
            identities  << identity.id();
#else
            identities  << identity.id;
#endif

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
        for (int i = 0; i<toTrain.size(); ++i)
        {
            provider.imagesToTrain[identities[i]].list << images[i];
        }

        database.train(identitySet, &provider, "digikam");
    }

    utils.removeFaces(toTrain);
    package->databaseFaces.replaceRole(FacePipelineDatabaseFace::ForTraining, FacePipelineDatabaseFace::Trained);

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
    databaseFilter       = 0;
    previewThread        = 0;
    detectionWorker      = 0;
    parallelDetectors    = 0;
    recognitionWorker    = 0;
    databaseWriter       = 0;
    trainer              = 0;
    detectionBenchmarker = 0;
    recognitionBenchmarker = 0;
    priority             = QThread::LowPriority;
    started              = false;
    infosForFiltering    = 0;
    packagesOnTheRoad    = 0;
    maxPackagesOnTheRoad = 50;
    totalPackagesAdded   = 0;
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
        foreach(const ImageInfo& info, infos)
        {
            send(buildPackage(info));
        }
    }
}

// called by filter
void FacePipeline::Private::sendFromFilter(const QList<FacePipelineExtendedPackage::Ptr>& packages)
{
    infosForFiltering -= packages.size();

    foreach(const FacePipelineExtendedPackage::Ptr& package, packages)
    {
        send(package);
    }
}

// called by filter
void FacePipeline::Private::skipFromFilter(const QList<ImageInfo>& infosForSkipping)
{
    infosForFiltering -= infosForSkipping.size();

    emit(q->skipped(infosForSkipping));

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

FacePipelineExtendedPackage::Ptr FacePipeline::Private::buildPackage(const ImageInfo& info, const FacePipelineDatabaseFace& face, const DImg& image)
{
    FacePipelineDatabaseFaceList faces;
    faces << face;
    return buildPackage(info, faces, image);
}

FacePipelineExtendedPackage::Ptr FacePipeline::Private::buildPackage(const ImageInfo& info, const FacePipelineDatabaseFaceList& faces, const DImg& image)
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
    emit(q->processing(*package));

    if (senderFlowControl(package))
    {
        ++packagesOnTheRoad;
        emit startProcess(package);
    }
}

void FacePipeline::Private::finishProcess(FacePipelineExtendedPackage::Ptr package)
{
    packagesOnTheRoad--;
    emit(q->processed(*package));
    emit(q->progressValueChanged(float(packagesOnTheRoad + delayedPackages.size()) / totalPackagesAdded));
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
    kDebug() << "Check for finish: " << packagesOnTheRoad << "packages,"
             << infosForFiltering << "infos to filter, hasFinished()" << hasFinished();

    if (hasFinished())
    {
        totalPackagesAdded = 0;
        emit(q->finished());
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

    emit(q->scheduled());

    WorkerObject*  workerObject = 0;
    ParallelPipes* pipes        = 0;

    foreach(QObject* const element, pipeline)
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
    emit(q->started(i18n("Applying face changes")));
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

    foreach (ThumbnailLoadThread* thread, thumbnailLoadThreads)
    {
        thread->stopAllTasks();
    }

    WorkerObject* workerObject = 0;
    ParallelPipes* pipes       = 0;
    DynamicThread* thread      = 0;

    foreach(QObject* const element, pipeline)
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

    foreach (ThumbnailLoadThread* thread, thumbnailLoadThreads)
    {
        thread->wait();
    }

    WorkerObject* workerObject = 0;
    ParallelPipes* pipes       = 0;
    DynamicThread* thread      = 0;

    foreach(QObject* const element, pipeline)
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

    foreach(QObject* const element, pipeline)
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

    foreach (ThumbnailLoadThread* thread, thumbnailLoadThreads)
    {
        thread->setPriority(priority);
    }
}

ThumbnailLoadThread* FacePipeline::Private::createThumbnailLoadThread()
{
    ThumbnailLoadThread* thumbnailLoadThread = new ThumbnailLoadThread;
    thumbnailLoadThread->setPixmapRequested(false);
    thumbnailLoadThread->setThumbnailSize(ThumbnailLoadThread::maximumThumbnailSize());
    // KFaceIface::Image::recommendedSizeForRecognition()
    thumbnailLoadThread->setPriority(priority);

    thumbnailLoadThreads << thumbnailLoadThread;
    return thumbnailLoadThread;
}

// ----------------------------------------------------------------------------------------

FacePipeline::FacePipeline()
    : d(new Private(this))
{
    qRegisterMetaType<FacePipelineExtendedPackage::Ptr>("FacePipelineExtendedPackage::Ptr");
}

FacePipeline::~FacePipeline()
{
    shutDown();

    delete d->databaseFilter;
    delete d->previewThread;
    delete d->detectionWorker;
    delete d->parallelDetectors;
    delete d->recognitionWorker;
    delete d->databaseWriter;
    delete d->trainer;
    qDeleteAll(d->thumbnailLoadThreads);
    delete d->detectionBenchmarker;
    delete d->recognitionBenchmarker;
    delete d;
}

void FacePipeline::shutDown()
{
    cancel();
    d->wait();
}

bool FacePipeline::hasFinished() const
{
    return d->hasFinished();
}

QString FacePipeline::benchmarkResult() const
{
    if (d->detectionBenchmarker)
    {
        return d->detectionBenchmarker->result();
    }
    if (d->recognitionBenchmarker)
    {
        return d->recognitionBenchmarker->result();
    }
    return QString();
}

void FacePipeline::plugDatabaseFilter(FilterMode mode)
{
    d->databaseFilter = new ScanStateFilter(mode, d);
}

void FacePipeline::plugRerecognizingDatabaseFilter()
{
    plugDatabaseFilter(ReadUnconfirmedFaces);
    d->databaseFilter->tasks = FacePipelineDatabaseFace::ForRecognition;
}

void FacePipeline::plugRetrainingDatabaseFilter()
{
    plugDatabaseFilter(ReadConfirmedFaces);
    d->databaseFilter->tasks = FacePipelineDatabaseFace::ForTraining;
}

void FacePipeline::plugPreviewLoader()
{
    d->previewThread = new PreviewLoader(d);
}

void FacePipeline::plugFaceDetector()
{
    d->detectionWorker = new DetectionWorker(d);

    connect(d, SIGNAL(accuracyChanged(double)),
            d->detectionWorker, SLOT(setAccuracy(double)));
}

void FacePipeline::plugParallelFaceDetectors()
{
    if (QThread::idealThreadCount() <= 1)
    {
        return plugFaceDetector();
    }

    // limit number of parallel detectors to 3, because of memory cost (cascades)
    const int n          = qMin(3, QThread::idealThreadCount());
    d->parallelDetectors = new ParallelPipes;

    for (int i = 0; i < n; ++i)
    {
        DetectionWorker* const worker = new DetectionWorker(d);

        connect(d, SIGNAL(accuracyChanged(double)),
                worker, SLOT(setAccuracy(double)));

        d->parallelDetectors->add(worker);
    }
}

void FacePipeline::plugFaceRecognizer()
{
    d->recognitionWorker = new RecognitionWorker(d);
    d->createThumbnailLoadThread();

    connect(d, SIGNAL(accuracyChanged(double)),
            d->recognitionWorker, SLOT(setThreshold(double)));
}

void FacePipeline::plugDatabaseWriter(WriteMode mode)
{
    d->databaseWriter = new DatabaseWriter(mode, d);
    d->createThumbnailLoadThread();
}

void FacePipeline::plugTrainer()
{
    d->trainer = new Trainer(d);
    d->createThumbnailLoadThread();
}

void FacePipeline::plugDetectionBenchmarker()
{
    d->detectionBenchmarker = new DetectionBenchmarker(d);
    d->createThumbnailLoadThread();
}

void FacePipeline::plugRecognitionBenchmarker()
{
    d->recognitionBenchmarker = new RecognitionBenchmarker(d);
}

void FacePipeline::plugDatabaseEditor()
{
    plugDatabaseWriter(NormalWrite);
    d->createThumbnailLoadThread();
}

void FacePipeline::construct()
{
    if (d->previewThread)
    {
        d->pipeline << d->previewThread;
    }

    if (d->parallelDetectors)
    {
        d->pipeline << d->parallelDetectors;
    }
    else if (d->detectionWorker)
    {
        d->pipeline << d->detectionWorker;
    }

    if (d->recognitionWorker)
    {
        d->pipeline << d->recognitionWorker;
    }

    if (d->detectionBenchmarker)
    {
        d->pipeline << d->detectionBenchmarker;
    }

    if (d->recognitionBenchmarker)
    {
        d->pipeline << d->recognitionBenchmarker;
    }

    if (d->databaseWriter)
    {
        d->pipeline << d->databaseWriter;
    }

    if (d->trainer)
    {
        d->pipeline << d->trainer;
    }

    if (d->pipeline.isEmpty())
    {
        kWarning() << "Nothing plugged in. It's a noop.";
        return;
    }

    connect(d, SIGNAL(startProcess(FacePipelineExtendedPackage::Ptr)),
            d->pipeline.first(), SLOT(process(FacePipelineExtendedPackage::Ptr)));

    for (int i = 0; i < d->pipeline.size() - 1; ++i)
    {
        connect(d->pipeline.at(i), SIGNAL(processed(FacePipelineExtendedPackage::Ptr)),
                d->pipeline.at(i + 1), SLOT(process(FacePipelineExtendedPackage::Ptr)));
    }

    connect(d->pipeline.last(), SIGNAL(processed(FacePipelineExtendedPackage::Ptr)),
            d, SLOT(finishProcess(FacePipelineExtendedPackage::Ptr)));

    d->applyPriority();
}

void FacePipeline::setPriority(QThread::Priority priority)
{
    if (d->priority == priority)
    {
        return;
    }

    d->priority = priority;
    d->applyPriority();
}

QThread::Priority FacePipeline::priority() const
{
    return d->priority;
}

void FacePipeline::cancel()
{
    d->stop();
}

bool FacePipeline::process(const ImageInfo& info)
{
    QString filePath = info.filePath();

    if (filePath.isNull())
    {
        kWarning() << "ImageInfo has no valid file path. Skipping.";
        return false;
    }

    FacePipelineExtendedPackage::Ptr package = d->filterOrBuildPackage(info);

    if (!package)
    {
        return false;
    }

    d->send(package);
    return true;
}

bool FacePipeline::process(const ImageInfo& info, const DImg& image)
{
    FacePipelineExtendedPackage::Ptr package = d->filterOrBuildPackage(info);

    if (!package)
    {
        return false;
    }

    package->image = image;
    d->send(package);

    return true;
}

/*
bool FacePipeline::add(const ImageInfo& info, const QRect& rect, const DImg& image)
{
    FacePipelineExtendedPackage::Ptr package = d->buildPackage(info);
    package->image                           = image;
    package->detectionImage                  = image;
    package->faces << KFaceIface::Face(rect);
    d->send(package);
}
*/

void FacePipeline::train(const ImageInfo& info, const QList<DatabaseFace>& databaseFaces)
{
    train(info, databaseFaces, DImg());
}

void FacePipeline::train(const ImageInfo& info, const QList<DatabaseFace>& databaseFaces, const DImg& image)
{
    FacePipelineExtendedPackage::Ptr package = d->buildPackage(info, databaseFaces, image);
    package->databaseFaces.setRole(FacePipelineDatabaseFace::ForTraining);
    d->send(package);
}

DatabaseFace FacePipeline::confirm(const ImageInfo& info, const DatabaseFace& databaseFace,
                                   int assignedTagId, const TagRegion& assignedRegion)
{
    return confirm(info, databaseFace, DImg(), assignedTagId, assignedRegion);
}

DatabaseFace FacePipeline::confirm(const ImageInfo& info, const DatabaseFace& databaseFace,
                                   const DImg& image, int assignedTagId, const TagRegion& assignedRegion)
{
    FacePipelineDatabaseFace face             = databaseFace;
    face.assignedTagId                        = assignedTagId;
    face.assignedRegion                       = assignedRegion;
    face.roles                               |= FacePipelineDatabaseFace::ForConfirmation;
    FacePipelineExtendedPackage::Ptr package  = d->buildPackage(info, face, image);

    d->send(package);

    return FaceTagsEditor::confirmedEntry(face, assignedTagId, assignedRegion);
}

DatabaseFace FacePipeline::addManually(const ImageInfo& info, const DImg& image, const TagRegion& assignedRegion)
{
    FacePipelineDatabaseFace face; // giving a null face => no existing face yet, add it
    face.assignedTagId                        = -1;
    face.assignedRegion                       = assignedRegion;
    face.roles                               |= FacePipelineDatabaseFace::ForEditing;
    FacePipelineExtendedPackage::Ptr package  = d->buildPackage(info, face, image);

    package->databaseFaces.setRole(FacePipelineDatabaseFace::ForEditing);
    d->send(package);

    return FaceTagsEditor::unconfirmedEntry(info.id(), face.assignedTagId, face.assignedRegion);
}

DatabaseFace FacePipeline::editRegion(const ImageInfo& info, const DImg& image,
                                      const DatabaseFace& databaseFace,
                                      const TagRegion& newRegion)
{
    FacePipelineDatabaseFace face             = databaseFace;
    face.assignedTagId                        = -1;
    face.assignedRegion                       = newRegion;
    face.roles                               |= FacePipelineDatabaseFace::ForEditing;
    FacePipelineExtendedPackage::Ptr package  = d->buildPackage(info, face, image);

    package->databaseFaces.setRole(FacePipelineDatabaseFace::ForEditing);
    d->send(package);

    face.setRegion(newRegion);
    return face;
}

void FacePipeline::remove(const ImageInfo& info, const DatabaseFace& databaseFace)
{
    FacePipelineExtendedPackage::Ptr package = d->buildPackage(info, databaseFace, DImg());
    package->databaseFaces.setRole(FacePipelineDatabaseFace::ForEditing);
    d->send(package);
}

void FacePipeline::process(const QList<ImageInfo>& infos)
{
    d->processBatch(infos);
}

void FacePipeline::setDetectionAccuracy(double value)
{
    emit(d->accuracyChanged(value));
}

} // namespace Digikam
