/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-03
 * Description : Integrated, multithread face detection / recognition
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes

#include "loadingdescription.h"
#include "metadatasettings.h"
#include "threadmanager.h"

namespace Digikam
{

FacePipelineExtendedPackage::Ptr PackageLoadingDescriptionList::take(const LoadingDescription& description)
{
    FacePipelineExtendedPackage::Ptr package;
    QList<FacePipelineExtendedPackage::Ptr>::iterator it;
    for (it = begin(); it != end(); ++it)
    {
        if ( *(*it) == description)
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
    foreach (WorkerObject* object, m_workers)
        delete object;
}

void ParallelPipes::schedule()
{
    foreach (WorkerObject* object, m_workers)
        object->schedule();
}

void ParallelPipes::deactivate(WorkerObject::DeactivatingMode mode)
{
    foreach (WorkerObject* object, m_workers)
        object->deactivate(mode);
}

void ParallelPipes::add(WorkerObject *worker)
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
    m_methods[m_currentIndex].invoke(m_workers[m_currentIndex], Qt::QueuedConnection,
                                     Q_ARG(FacePipelineExtendedPackage::Ptr, package));

    if (++m_currentIndex == m_workers.size())
        m_currentIndex = 0;
}

// ----------------------------------------------------------------------------------------

ScanStateFilter::ScanStateFilter(FacePipeline::FilterMode mode, FacePipeline::FacePipelinePriv* d)
               : d(d), mode(mode)
{
    connect(this, SIGNAL(infosToDispatch()),
            this, SLOT(dispatch()));
}

FacePipelineExtendedPackage::Ptr ScanStateFilter::filter(const ImageInfo& info)
{
    switch (mode)
    {
        case FacePipeline::ScanAll:
        {
            return d->buildPackage(info);
        }
        case FacePipeline::SkipAlreadyScanned:
        {
            if (!d->iface->hasBeenScanned(info))
                return d->buildPackage(info);
            break;
        }
        case FacePipeline::ReadUnconfirmedFaces:
        case FacePipeline::ReadFacesForTraining:
        {
            QList<DatabaseFace> databaseFaces;
            if (mode == FacePipeline::ReadUnconfirmedFaces)
                databaseFaces = d->iface->unconfirmedDatabaseFaces(info.id());
            else
                databaseFaces = d->iface->databaseFacesForTraining(info.id());

            if (!databaseFaces.isEmpty())
            {
                FacePipelineExtendedPackage::Ptr package = d->buildPackage(info);
                package->databaseFaces = databaseFaces;
                package->faces         = d->iface->toFaces(databaseFaces);
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
                stop(lock);
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
                    send << package;
                else
                    skip << info;
            }

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
    if (!skip.isEmpty())
        d->skipFromFilter(skip);
    if (!send.isEmpty())
        d->sendFromFilter(send);
}

// ----------------------------------------------------------------------------------------

PreviewLoader::PreviewLoader(FacePipeline::FacePipelinePriv* d)
             : d(d)
{
    // upper limit for memory cost
    maximumSentOutPackages = qMin(QThread::idealThreadCount(), 5);

    // this is crucial! Per default, only the last added image will be loaded
    setLoadingPolicy(PreviewLoadThread::LoadingPolicySimpleAppend);

    connect(this, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
            this, SLOT(slotImageLoaded(const LoadingDescription&, const DImg&)));
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
    loadFastButLarge(package->filePath, 1600, MetadataSettings::instance()->settings().exifRotate);
    //load(package->filePath, 800, MetadataSettings::instance()->settings().exifRotate);
    //loadHighQuality(package->filePath, MetadataSettings::instance()->settings().exifRotate);
}

void PreviewLoader::slotImageLoaded(const LoadingDescription& loadingDescription, const DImg& img)
{
    FacePipelineExtendedPackage::Ptr package = scheduledPackages.take(loadingDescription);
    if (!package)
        return;

    // Avoid congestion before detection or recognition by pausing the thread.
    // We are throwing around serious amounts of memory!
    //kDebug() << "sent out packages:" << d->packagesOnTheRoad - scheduledPackages.size() << "scheduled:" << scheduledPackages.size();
    if (sentOutLimitReached() && !scheduledPackages.isEmpty())
        stop();

    if (img.isNull())
        return d->finishProcess(package);

    package->image = img;
    package->processFlags |= FacePipelinePackage::PreviewImageLoaded;
    emit processed(package);
}

bool PreviewLoader::sentOutLimitReached()
{
    return d->packagesOnTheRoad - scheduledPackages.size() > maximumSentOutPackages;
}

void PreviewLoader::checkRestart()
{
    if (!sentOutLimitReached() && !scheduledPackages.isEmpty())
        start();
}

// ----------------------------------------------------------------------------------------

DetectionWorker::DetectionWorker(FacePipeline::FacePipelinePriv* d)
               : d(d)
{
}

void DetectionWorker::process(FacePipelineExtendedPackage::Ptr package)
{
    package->detectionImage = scaleForDetection(package->image);
    KFaceIface::Image image = FaceIface::toImage(package->detectionImage);
    image.setOriginalSize(package->image.originalSize());

    package->faces = detector.detectFaces(image);

    kDebug() << "Found" << package->faces.size() << "faces in" << package->info.name() 
             << package->image.size() << package->image.originalSize();

    package->processFlags |= FacePipelinePackage::ProcessedByDetector;
    emit processed(package);
}

DImg DetectionWorker::scaleForDetection(const DImg& image) const
{
    int recommendedSize = detector.recommendedImageSize(image.size());
    if (qMax(image.width(), image.height()) > (uint)recommendedSize)
    {
        return image.smoothScale(recommendedSize, recommendedSize, Qt::KeepAspectRatio);
    }
    return image;
}

void DetectionWorker::setAccuracy(double accuracy)
{
    detector.setAccuracy(accuracy);
}

void DetectionWorker::setSpecificity(double specificity)
{
    detector.setSpecificity(specificity);
}

// ----------------------------------------------------------------------------------------

RecognitionWorker::RecognitionWorker(FacePipeline::FacePipelinePriv* d)
                 : d(d)
{
    database             = KFaceIface::RecognitionDatabase::addDatabase();
    recognitionThreshold = 10000000;
}

void RecognitionWorker::process(FacePipelineExtendedPackage::Ptr package)
{
    QSize size = database.recommendedImageSize(package->image.size());
    d->iface->fillImageInFaces(package->image, package->faces, size);

    QList<double> distances = database.recognizeFaces(package->faces);

    for (int i=0; i<distances.size(); i++)
    {
        kDebug() << "Recognition:"  << package->info.id()     << package->faces[i].toRect()
                 << "recognized as" << package->faces[i].id() << package->faces[i].name()
                 << "at distance"   << distances[i]
                 << ((distances[i] > recognitionThreshold) ? "(discarded)" : "(accepted)");
        if (distances[i] > recognitionThreshold)
        {
            package->faces[i].clearRecognition();
        }
    }
    package->processFlags |= FacePipelinePackage::ProcessedByRecognizer;
    emit processed(package);
}

void RecognitionWorker::setThreshold(double threshold)
{
    recognitionThreshold = threshold;
}

// ----------------------------------------------------------------------------------------

DatabaseWriter::DatabaseWriter(FacePipeline::WriteMode mode, FacePipeline::FacePipelinePriv* d)
              : mode(mode), d(d)
{
}

void DatabaseWriter::process(FacePipelineExtendedPackage::Ptr package)
{
    if (mode == FacePipeline::OverwriteUnconfirmed
        && package->processFlags & FacePipelinePackage::ProcessedByDetector)
    {
        QList<DatabaseFace> oldEntries = d->iface->unconfirmedDatabaseFaces(package->info.id());
        kDebug() << "Removing old entries" << oldEntries;
        d->iface->removeFaces(oldEntries);
    }

    d->iface->markAsScanned(package->info);

    if (!package->info.isNull() && !package->faces.isEmpty())
    {
        package->databaseFaces =
            d->iface->writeUnconfirmedResults(package->detectionImage, package->info.id(), package->faces);

        if (!package->image.isNull())
            d->iface->storeThumbnails(d->thumbnailLoadThread, package->filePath, package->databaseFaces, package->image);
    }

    package->processFlags |= FacePipelinePackage::WrittenToDatabase;
    emit processed(package);
}

// ----------------------------------------------------------------------------------------

Trainer::Trainer(FacePipeline::FacePipelinePriv* d)
    : d(d)
{
    database            = KFaceIface::RecognitionDatabase::addDatabase();
}

void Trainer::process(FacePipelineExtendedPackage::Ptr package)
{
    QSize size = database.recommendedImageSize(package->image.size());
    if (package->image.isNull())
        d->iface->fillImageInFaces(d->thumbnailLoadThread, package->filePath, package->faces, size);
    else
        d->iface->fillImageInFaces(package->image, package->faces, size);

    database.updateFaces(package->faces);

    // These are faces for training
    d->iface->removeFaces(package->databaseFaces);

    package->processFlags |= FacePipelinePackage::ProcessedByTrainer;
    emit processed(package);
}

// ----------------------------------------------------------------------------------------

FacePipeline::FacePipelinePriv::FacePipelinePriv(FacePipeline* q)
                              : q(q)
{
    databaseFilter       = 0;
    previewThread        = 0;
    detectionWorker      = 0;
    parallelDetectors    = 0;
    recognitionWorker    = 0;
    databaseWriter       = 0;
    trainer              = 0;
    iface                = 0;
    thumbnailLoadThread  = 0;

    started              = false;
    infosForFiltering    = 0;
    packagesOnTheRoad    = 0;
}

void FacePipeline::FacePipelinePriv::processBatch(const QList<ImageInfo>& infos)
{
    if (databaseFilter)
    {
        infosForFiltering += infos.size();
        databaseFilter->process(infos);
    }
    else
    {
        foreach (const ImageInfo& info, infos)
            send(buildPackage(info));
    }
}

// called by filter
void FacePipeline::FacePipelinePriv::sendFromFilter(const QList<FacePipelineExtendedPackage::Ptr>& packages)
{
    infosForFiltering -= packages.size();
    foreach (const FacePipelineExtendedPackage::Ptr& package, packages)
        send(package);
}

// called by filter
void FacePipeline::FacePipelinePriv::skipFromFilter(const QList<ImageInfo>& infosForSkipping)
{
    infosForFiltering -= infosForSkipping.size();
    emit q->skipped(infosForSkipping);
    // everything skipped?
    checkFinished();
}

FacePipelineExtendedPackage::Ptr FacePipeline::FacePipelinePriv::filterOrBuildPackage(const ImageInfo& info)
{
    if (databaseFilter)
        return databaseFilter->filter(info);
    else
        return buildPackage(info);
}

FacePipelineExtendedPackage::Ptr FacePipeline::FacePipelinePriv::buildPackage(const ImageInfo& info)
{
    FacePipelineExtendedPackage::Ptr package(new FacePipelineExtendedPackage);
    package->info     = info;
    package->filePath = info.filePath();

    return package;
}

void FacePipeline::FacePipelinePriv::send(FacePipelineExtendedPackage::Ptr package)
{
    start();
    packagesOnTheRoad++;
    emit startProcess(package);
}

void FacePipeline::FacePipelinePriv::finishProcess(FacePipelineExtendedPackage::Ptr package)
{
    packagesOnTheRoad--;
    emit q->processed(*package);
    package = 0;

    if (previewThread)
        previewThread->checkRestart();

    checkFinished();
}

bool FacePipeline::FacePipelinePriv::hasFinished()
{
    return !packagesOnTheRoad && !infosForFiltering;
}

void FacePipeline::FacePipelinePriv::checkFinished()
{
    if (hasFinished())
    {
        emit q->finished();
        // stop threads
        stop();
    }
}

void FacePipeline::FacePipelinePriv::start()
{
    if (started)
        return;

    WorkerObject  *workerObject;
    ParallelPipes *pipes;
    foreach (QObject *element, pipeline)
    {
        if ( (workerObject = qobject_cast<WorkerObject*>(element)) )
            workerObject->schedule();
        else if ( (pipes = qobject_cast<ParallelPipes*>(element)) )
            pipes->schedule();
    }

    started = true;
}

void FacePipeline::FacePipelinePriv::stop()
{
    if (!started)
        return;

    if (previewThread)
        previewThread->cancel();

    WorkerObject  *workerObject;
    ParallelPipes *pipes;
    DynamicThread *thread;
    foreach (QObject *element, pipeline)
    {
        if ( (workerObject = qobject_cast<WorkerObject*>(element)) )
            workerObject->deactivate();
        else if ( (pipes = qobject_cast<ParallelPipes*>(element)) )
            pipes->deactivate();
        else if ( (thread = qobject_cast<DynamicThread*>(element)) )
            thread->stop();
    }

    started = false;
}

void FacePipeline::FacePipelinePriv::createThumbnailLoadThread()
{
    if (!thumbnailLoadThread)
    {
        thumbnailLoadThread = new ThumbnailLoadThread;
        thumbnailLoadThread->setPixmapRequested(false);
        thumbnailLoadThread->setThumbnailSize(ThumbnailLoadThread::maximumThumbnailSize());
        // KFaceIface::Image::recommendedSizeForRecognition()
        thumbnailLoadThread->setExifRotate(MetadataSettings::instance()->settings().exifRotate);
    }
    //FaceIface::faceRectDisplayMargin()
}

// ----------------------------------------------------------------------------------------

FacePipeline::FacePipeline()
            : d(new FacePipelinePriv(this))
{
    d->iface = new FaceIface;

    qRegisterMetaType<FacePipelineExtendedPackage::Ptr>("FacePipelineExtendedPackage::Ptr");
}

FacePipeline::~FacePipeline()
{
    delete d->databaseFilter;
    delete d->previewThread;
    delete d->detectionWorker;
    delete d->parallelDetectors;
    delete d->recognitionWorker;
    delete d->databaseWriter;
    delete d->trainer;
    delete d->iface;
    delete d->thumbnailLoadThread;
    delete d;
}

bool FacePipeline::hasFinished() const
{
    return d->hasFinished();
}

void FacePipeline::plugDatabaseFilter(FilterMode mode)
{
    d->databaseFilter = new ScanStateFilter(mode, d);
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

    connect(d, SIGNAL(specificityChanged(double)),
            d->detectionWorker, SLOT(setSpecificity(double)));
}

void FacePipeline::plugParallelFaceDetectors()
{
    if (QThread::idealThreadCount() <= 1)
        return plugFaceDetector();

    // limit number of parallel detectors to 3, because of memory cost (cascades)
    const int n          = qMin(3, QThread::idealThreadCount());
    d->parallelDetectors = new ParallelPipes;

    for (int i=0; i<n; i++)
    {
        DetectionWorker *worker = new DetectionWorker(d);

        connect(d, SIGNAL(accuracyChanged(double)),
                worker, SLOT(setAccuracy(double)));

        connect(d, SIGNAL(specificityChanged(double)),
                worker, SLOT(setSpecificity(double)));

        d->parallelDetectors->add(worker);
    }
}

void FacePipeline::plugFaceRecognizer()
{
    d->recognitionWorker = new RecognitionWorker(d);

    connect(d, SIGNAL(thresholdChanged(double)),
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

void FacePipeline::construct()
{
    if (d->previewThread)
        d->pipeline << d->previewThread;

    if (d->parallelDetectors)
        d->pipeline << d->parallelDetectors;
    else if (d->detectionWorker)
        d->pipeline << d->detectionWorker;

    if (d->recognitionWorker)
        d->pipeline << d->recognitionWorker;

    if (d->databaseWriter)
        d->pipeline << d->databaseWriter;

    if (d->pipeline.isEmpty())
    {
        kWarning() << "Nothing plugged in. It's a noop.";
        return;
    }

    connect(d, SIGNAL(startProcess(FacePipelineExtendedPackage::Ptr)),
            d->pipeline.first(), SLOT(process(FacePipelineExtendedPackage::Ptr)));

    for (int i = 0; i < d->pipeline.size()-1; i++)
    {
        connect(d->pipeline[i], SIGNAL(processed(FacePipelineExtendedPackage::Ptr)),
                d->pipeline[i+1], SLOT(process(FacePipelineExtendedPackage::Ptr)));
    }

    connect(d->pipeline.last(), SIGNAL(processed(FacePipelineExtendedPackage::Ptr)),
            d, SLOT(finishProcess(FacePipelineExtendedPackage::Ptr)));
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
        return false;
    d->send(package);
    return true;
}

bool FacePipeline::process(const ImageInfo& info, const DImg& image)
{
    FacePipelineExtendedPackage::Ptr package = d->filterOrBuildPackage(info);
    if (!package)
        return false;
    package->image = image;
    d->send(package);

    return true;
}

void FacePipeline::train(const QList<DatabaseFace> &databaseFaces, const ImageInfo& info)
{
    train(databaseFaces, info, DImg());
}

void FacePipeline::train(const QList<DatabaseFace> &databaseFaces, const ImageInfo& info, const DImg& image)
{
    FacePipelineExtendedPackage::Ptr package = d->buildPackage(info);
    package->image = image;
    package->databaseFaces = databaseFaces;
    package->faces = d->iface->toFaces(databaseFaces);

    // Only need to set the right type
    for (int i=0; i<package->databaseFaces.size(); i++)
    {
        DatabaseFace &face = package->databaseFaces[i];
        if (face.isForTraining())
            continue;
        else if (face.isConfirmedName())
            face.setType(DatabaseFace::FaceForTraining);
        else
            face = DatabaseFace();
    }

    d->send(package);
}

void FacePipeline::process(const QList<ImageInfo>& infos)
{
    d->processBatch(infos);
}

void FacePipeline::setDetectionAccuracy(double value)
{
    emit d->accuracyChanged(value);
}

void FacePipeline::setDetectionSpecificity(double value)
{
    emit d->specificityChanged(value);
}

} // namespace Digikam
