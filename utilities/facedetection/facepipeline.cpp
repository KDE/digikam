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

#include "facepipeline.h"
#include "facepipeline_p.h"


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

// --- ParallelPipes ---

ParallelPipes::ParallelPipes()
    : m_currentIndex(0)
{
}

ParallelPipes::~ParallelPipes()
{
    foreach (WorkerObject *object, m_workers)
        delete object;
}

void ParallelPipes::schedule()
{
    foreach (WorkerObject *object, m_workers)
        object->schedule();
}

void ParallelPipes::deactivate(WorkerObject::DeactivatingMode mode)
{
    foreach (WorkerObject *object, m_workers)
        object->deactivate(mode);
}

void ParallelPipes::add(WorkerObject *worker)
{
    QByteArray normalizedSignature = QMetaObject::normalizedSignature("process(FacePipelineExtendedPackage::Ptr)");
    int methodIndex = worker->metaObject()->indexOfMethod(normalizedSignature);
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

// --- ScanStateFilter ---

ScanStateFilter::ScanStateFilter(FacePipeline::FilterMode mode, FacePipelinePriv *d)
    : d(d), mode(mode)
{
    connect(this, SIGNAL(infosToDispatch()),
            this, SLOT(dispatch()));
}

bool ScanStateFilter::filter(const ImageInfo& info)
{
    switch (mode)
    {
        case FacePipeline::RescanAll:
            //TODO: remove records from db (unconfirmed faces)
            return true;
        case FacePipeline::SkipAlreadyScanned:
            // Is iface thread-safe? Officially, not, but this method is.
            if (d->iface.hasBeenScanned(info))
                return false;
            //TODO: check if any records should be removed from db (unconfirmed faces)
            return true;
    }
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

        if (!todo.isEmpty())
        {
            QList<ImageInfo> send, skip;
            foreach (const ImageInfo& info, todo)
            {
                if (filter(info))
                    send << info;
                else
                    skip << info;
            }
            //kDebug() << "Filtered images: process" << send.size() << "and skip" << skip.size();

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
    QList<ImageInfo> send, skip;
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


// --- PreviewLoader ---

PreviewLoader::PreviewLoader(FacePipelinePriv* d)
    : d(d)
{
    // ideal thread count plus lubricant, but upper limit for memory cost
    maximumSentOutPackages = qMin(QThread::idealThreadCount() + 2, 5);

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
    //loadFastButLarge(package->filePath, 1600, MetadataSettings::instance()->settings().exifRotate);
    load(package->filePath, 1600, MetadataSettings::instance()->settings().exifRotate);
    //loadHighQuality(package->filePath, MetadataSettings::instance()->settings().exifRotate);
}

void PreviewLoader::slotImageLoaded(const LoadingDescription& loadingDescription, const DImg& img)
{
    FacePipelineExtendedPackage::Ptr package;
    QList<FacePipelineExtendedPackage::Ptr>::iterator it;
    for (it = scheduledPackages.begin(); it != scheduledPackages.end(); ++it)
    {
        if ( *(*it) == loadingDescription)
        {
            package = *it;
            scheduledPackages.erase(it);
            break;
        }
    }

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

// --- DetectionWorker ---

DetectionWorker::DetectionWorker(FacePipelinePriv* d)
    : d(d)
{
}

void DetectionWorker::process(FacePipelineExtendedPackage::Ptr package)
{
    KFaceIface::Image image(package->image.width(), package->image.height(),
                            package->image.sixteenBit(), package->image.hasAlpha(),
                            package->image.bits());
    package->faces = detector.detectFaces(image);
    kDebug() << "Found" << package->faces.size() << "faces in" << package->info.name() << package->image.size() << package->image.originalSize();

    package->processFlags |= FacePipelinePackage::ProcessedByDetector;
    emit processed(package);
}

void DetectionWorker::setAccuracy(int accuracy)
{
    detector.setAccuracy(accuracy);
}

// --- RecognitionWorker ---

RecognitionWorker::RecognitionWorker(FacePipelinePriv* d)
    : d(d)
{
    database = KFaceIface::RecognitionDatabase::addDatabase();
    recognitionThreshold = 10000000;
}


void RecognitionWorker::process(FacePipelineExtendedPackage::Ptr package)
{
    if (!package->processFlags & FacePipelinePackage::ProcessedByDetector && !package->info.isNull())
    {
        package->faces = d->iface.findUnconfirmedFacesFromTags(package->image, package->info.id());
    }

    QList<double> distances = database.recognizeFaces(package->faces);
    for (int i=0; i<distances.size(); i++)
    {
        kDebug() << "Recognition:" << package->info.id() << package->faces[i].toRect()
                 << "recognized as" << package->faces[i].id() << package->faces[i].name()
                 << "at distance" << distances[i]
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

// --- DatabaseWriter ---

DatabaseWriter::DatabaseWriter(FacePipelinePriv* d)
    : d(d)
{
}

void DatabaseWriter::process(FacePipelineExtendedPackage::Ptr package)
{
    d->iface.markAsScanned(package->info);
    if (!package->info.isNull() && !package->faces.isEmpty())
    {
        d->iface.writeUnconfirmedResults(package->image, package->info.id(), package->faces);
    }
    package->processFlags |= FacePipelinePackage::WrittenToDatabase;
    emit processed(package);
}

// --- FacePipelinePriv ---

FacePipelinePriv::FacePipelinePriv(FacePipeline *q)
    : q(q)
{
    alreadyScannedFilter = 0;
    previewThread        = 0;
    detectionWorker      = 0;
    parallelDetectors    = 0;
    recognitionWorker    = 0;
    databaseWriter       = 0;

    started              = false;
    infosForFiltering    = 0;
    packagesOnTheRoad    = 0;
}

void FacePipelinePriv::processBatch(const QList<ImageInfo>& infos)
{
    if (alreadyScannedFilter)
    {
        infosForFiltering += infos.size();
        alreadyScannedFilter->process(infos);
    }
    else
    {
        foreach (const ImageInfo& info, infos)
            send(buildPackage(info));
    }
}

// called by filter
void FacePipelinePriv::sendFromFilter(const QList<ImageInfo>& infos)
{
    infosForFiltering -= infos.size();
    foreach (const ImageInfo& info, infos)
        send(buildPackage(info));
}

// called by filter
void FacePipelinePriv::skipFromFilter(const QList<ImageInfo>& infosForSkipping)
{
    infosForFiltering -= infosForSkipping.size();
    emit q->skipped(infosForSkipping);
    // everything skipped?
    checkFinished();
}

FacePipelineExtendedPackage::Ptr FacePipelinePriv::buildPackage(const ImageInfo& info)
{
    FacePipelineExtendedPackage::Ptr package(new FacePipelineExtendedPackage);
    package->info = info;
    package->filePath = info.filePath();

    return package;
}

void FacePipelinePriv::send(FacePipelineExtendedPackage::Ptr package)
{
    start();
    packagesOnTheRoad++;
    emit startProcess(package);
}

void FacePipelinePriv::finishProcess(FacePipelineExtendedPackage::Ptr package)
{
    packagesOnTheRoad--;
    emit q->processed(*package);
    package = 0;

    if (previewThread)
        previewThread->checkRestart();

    checkFinished();
}

bool FacePipelinePriv::hasFinished()
{
    return !packagesOnTheRoad && !infosForFiltering;
}

void FacePipelinePriv::checkFinished()
{
    if (hasFinished())
    {
        emit q->finished();
        // stop threads
        stop();
    }
}

void FacePipelinePriv::start()
{
    if (started)
        return;

    if (parallelDetectors)
        parallelDetectors->schedule();
    else if (detectionWorker)
        detectionWorker->schedule();
    if (recognitionWorker)
        recognitionWorker->schedule();

    started = true;
}

void FacePipelinePriv::stop()
{
    if (!started)
        return;

    if (alreadyScannedFilter)
        alreadyScannedFilter->stop();
    if (previewThread)
        previewThread->cancel();
    if (parallelDetectors)
        parallelDetectors->deactivate();
    else if (detectionWorker)
        detectionWorker->deactivate();
    if (recognitionWorker)
        recognitionWorker->deactivate();

    started = false;
}

// --- FacePipeline ---

FacePipeline::FacePipeline()
    : d(new FacePipelinePriv(this))
{
    qRegisterMetaType<FacePipelineExtendedPackage::Ptr>("FacePipelineExtendedPackage::Ptr");
}

FacePipeline::~FacePipeline()
{
    delete d->alreadyScannedFilter;
    delete d->previewThread;
    delete d->detectionWorker;
    delete d->parallelDetectors;
    delete d->recognitionWorker;
    delete d->databaseWriter;
    delete d;
}

bool FacePipeline::hasFinished() const
{
    return d->hasFinished();
}

void FacePipeline::plugDatabaseFilter(FilterMode mode)
{
    d->alreadyScannedFilter = new ScanStateFilter(mode, d);
}

void FacePipeline::plugPreviewLoader()
{
    d->previewThread = new PreviewLoader(d);
}

void FacePipeline::plugFaceDetector()
{
    d->detectionWorker = new DetectionWorker(d);

    connect(d, SIGNAL(accuracyChanged(int)),
            d->detectionWorker, SLOT(setAccuracy(int)));
}

void FacePipeline::plugParallelFaceDetectors()
{
    if (QThread::idealThreadCount() <= 1)
        return plugFaceDetector();

    // limit number of parallel detectors to 3, because of memory cost (cascades)
    const int n = qMin(3, QThread::idealThreadCount());

    d->parallelDetectors = new ParallelPipes;

    for (int i=0; i<n; i++)
    {
        DetectionWorker *worker = new DetectionWorker(d);

        connect(d, SIGNAL(accuracyChanged(int)),
                worker, SLOT(setAccuracy(int)));

        d->parallelDetectors->add(worker);
    }
}

void FacePipeline::plugFaceRecognizer()
{
    d->recognitionWorker = new RecognitionWorker(d);

    connect(d, SIGNAL(thresholdChanged(double)),
            d->recognitionWorker, SLOT(setThreshold(double)));
}

void FacePipeline::plugDatabaseWriter()
{
    d->databaseWriter = new DatabaseWriter(d);
}

void FacePipeline::construct()
{
    QList<QObject*> pipeline;
    if (d->previewThread)
        pipeline << d->previewThread;
    if (d->parallelDetectors)
        pipeline << d->parallelDetectors;
    else if (d->detectionWorker)
        pipeline << d->detectionWorker;
    if (d->recognitionWorker)
        pipeline << d->recognitionWorker;
    if (d->databaseWriter)
        pipeline << d->databaseWriter;

    if (pipeline.isEmpty())
    {
        kWarning() << "Nothing plugged in. It's a noop.";
        return;
    }

    connect(d, SIGNAL(startProcess(FacePipelineExtendedPackage::Ptr)),
            pipeline.first(), SLOT(process(FacePipelineExtendedPackage::Ptr)));

    for (int i=0; i<pipeline.size()-1; i++)
    {
        connect(pipeline[i], SIGNAL(processed(FacePipelineExtendedPackage::Ptr)),
                pipeline[i+1], SLOT(process(FacePipelineExtendedPackage::Ptr)));
    }

    connect(pipeline.last(), SIGNAL(processed(FacePipelineExtendedPackage::Ptr)),
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

    if (d->alreadyScannedFilter)
    {
        if (!d->alreadyScannedFilter->filter(info))
            return false;
    }

    d->send(d->buildPackage(info));
    return true;
}

bool FacePipeline::process(const ImageInfo& info, const DImg& image)
{
    if (d->alreadyScannedFilter)
    {
        if (!d->alreadyScannedFilter->filter(info))
            return false;
    }

    FacePipelineExtendedPackage::Ptr package = d->buildPackage(info);
    package->image = image;
    d->send(package);

    return true;
}

void FacePipeline::process(const QList<ImageInfo>& infos)
{
    d->processBatch(infos);
}

}

