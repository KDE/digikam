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
#include "faceworkers.h"
#include "faceimageretriever.h"
#include "parallelpipes.h"
#include "scanstatefilter.h"

namespace Digikam
{

FacePipelineFaceTagsIface::FacePipelineFaceTagsIface()
    : roles(NoRole),
      assignedTagId(0)
{
}

FacePipelineFaceTagsIface::FacePipelineFaceTagsIface(const FaceTagsIface& face)
    : FaceTagsIface(face),
      roles(NoRole),
      assignedTagId(0)
{
}

// ----------------------------------------------------------------------------------------

FacePipelineFaceTagsIfaceList::FacePipelineFaceTagsIfaceList()
{
}

FacePipelineFaceTagsIfaceList::FacePipelineFaceTagsIfaceList(const QList<FaceTagsIface>& faces)
{
    operator=(faces);
}

FacePipelineFaceTagsIfaceList& FacePipelineFaceTagsIfaceList::operator=(const QList<FaceTagsIface>& faces)
{
    foreach (const FaceTagsIface& face, faces)
    {
        operator<<(FacePipelineFaceTagsIface(face));
    }

    return *this;
}

void FacePipelineFaceTagsIfaceList::setRole(FacePipelineFaceTagsIface::Roles role)
{
    for (iterator it = begin() ; it != end() ; ++it)
    {
        it->roles |= role;
    }
}

void FacePipelineFaceTagsIfaceList::clearRole(FacePipelineFaceTagsIface::Roles role)
{
    for (iterator it = begin() ; it != end() ; ++it)
    {
        it->roles &= ~role;
    }
}

void FacePipelineFaceTagsIfaceList::replaceRole(FacePipelineFaceTagsIface::Roles remove,
                                                FacePipelineFaceTagsIface::Roles add)
{
    for (iterator it = begin() ; it != end() ; ++it)
    {
        if (it->roles & remove)
        {
            it->roles &= ~remove;
            it->roles |= add;
        }
    }
}

QList<FaceTagsIface> FacePipelineFaceTagsIfaceList::toFaceTagsIfaceList() const
{
    QList<FaceTagsIface> faces;

    for (const_iterator it = constBegin() ; it != constEnd() ; ++it)
    {
        faces << *it;
    }

    return faces;
}

FacePipelineFaceTagsIfaceList FacePipelineFaceTagsIfaceList::facesForRole(FacePipelineFaceTagsIface::Roles role) const
{
    FacePipelineFaceTagsIfaceList faces;

    for (const_iterator it = constBegin() ; it != constEnd() ; ++it)
    {
        if (it->roles & role)
        {
            faces << *it;
        }
    }

    return faces;
}

// -----------------------------------------------------------------------------------------

FacePipelinePackage::FacePipelinePackage()
    : processFlags(NotProcessed)
{
}

FacePipelinePackage::~FacePipelinePackage()
{
}

// -----------------------------------------------------------------------------------------

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
    d->databaseFilter->tasks = FacePipelineFaceTagsIface::ForRecognition;
}

void FacePipeline::plugRetrainingDatabaseFilter()
{
    plugDatabaseFilter(ReadConfirmedFaces);
    d->databaseFilter->tasks = FacePipelineFaceTagsIface::ForTraining;
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

    for (int i = 0 ; i < n ; ++i)
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
        qCDebug(DIGIKAM_GENERAL_LOG) << "Face PipeLine: add preview thread";
    }

    if (d->parallelDetectors)
    {
        d->pipeline << d->parallelDetectors;
        qCDebug(DIGIKAM_GENERAL_LOG) << "Face PipeLine: add parallel thread detectors";
    }
    else if (d->detectionWorker)
    {
        d->pipeline << d->detectionWorker;
        qCDebug(DIGIKAM_GENERAL_LOG) << "Face PipeLine: add single thread detector";
    }

    if (d->recognitionWorker)
    {
        d->pipeline << d->recognitionWorker;
        qCDebug(DIGIKAM_GENERAL_LOG) << "Face PipeLine: add recognition worker";
    }

    if (d->detectionBenchmarker)
    {
        d->pipeline << d->detectionBenchmarker;
        qCDebug(DIGIKAM_GENERAL_LOG) << "Face PipeLine: add detection benchmaker";
    }

    if (d->recognitionBenchmarker)
    {
        d->pipeline << d->recognitionBenchmarker;
        qCDebug(DIGIKAM_GENERAL_LOG) << "Face PipeLine: add recognition benchmaker";
    }

    if (d->databaseWriter)
    {
        d->pipeline << d->databaseWriter;
        qCDebug(DIGIKAM_GENERAL_LOG) << "Face PipeLine: add database writer";
    }

    if (d->trainer)
    {
        d->pipeline << d->trainer;
        qCDebug(DIGIKAM_GENERAL_LOG) << "Face PipeLine: add faces trainer";
    }

    if (d->pipeline.isEmpty())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Nothing plugged in. It is a noop.";
        return;
    }

    connect(d, SIGNAL(startProcess(FacePipelineExtendedPackage::Ptr)),
            d->pipeline.first(), SLOT(process(FacePipelineExtendedPackage::Ptr)));

    for (int i = 0 ; i < d->pipeline.size() - 1 ; ++i)
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

void FacePipeline::activeFaceRecognizer(RecognitionDatabase::RecognizeAlgorithm algorithmType)
{
    if (d->recognitionWorker != 0)
    {
        d->recognitionWorker->activeFaceRecognizer(algorithmType);
    }
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
        qCWarning(DIGIKAM_GENERAL_LOG) << "ImageInfo has no valid file path. Skipping.";
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

bool FacePipeline::process(const ImageInfo& info,
                           const DImg& image)
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
bool FacePipeline::add(const ImageInfo& info,
                       const QRect& rect,
                       const DImg& image)
{
    FacePipelineExtendedPackage::Ptr package = d->buildPackage(info);
    package->image                           = image;
    package->detectionImage                  = image;
    package->faces << Face(rect);
    d->send(package);
}
*/

void FacePipeline::train(const ImageInfo& info,
                         const QList<FaceTagsIface>& databaseFaces)
{
    train(info, databaseFaces, DImg());
}

void FacePipeline::train(const ImageInfo& info,
                         const QList<FaceTagsIface>& databaseFaces,
                         const DImg& image)
{
    FacePipelineExtendedPackage::Ptr package = d->buildPackage(info,
                                                               FacePipelineFaceTagsIfaceList(databaseFaces),
                                                               image);
    package->databaseFaces.setRole(FacePipelineFaceTagsIface::ForTraining);
    d->send(package);
}

FaceTagsIface FacePipeline::confirm(const ImageInfo& info,
                                    const FaceTagsIface& databaseFace,
                                    int assignedTagId,
                                    const TagRegion& assignedRegion)
{
    return confirm(info, databaseFace, DImg(), assignedTagId, assignedRegion);
}

FaceTagsIface FacePipeline::confirm(const ImageInfo& info,
                                    const FaceTagsIface& databaseFace,
                                    const DImg& image,
                                    int assignedTagId,
                                    const TagRegion& assignedRegion)
{
    FacePipelineFaceTagsIface face            = FacePipelineFaceTagsIface(databaseFace);
    face.assignedTagId                        = assignedTagId;
    face.assignedRegion                       = assignedRegion;
    face.roles                               |= FacePipelineFaceTagsIface::ForConfirmation;
    FacePipelineExtendedPackage::Ptr package  = d->buildPackage(info, face, image);

    d->send(package);

    return FaceTagsEditor::confirmedEntry(face, assignedTagId, assignedRegion);
}

FaceTagsIface FacePipeline::addManually(const ImageInfo& info,
                                        const DImg& image,
                                        const TagRegion& assignedRegion)
{
    FacePipelineFaceTagsIface face; // giving a null face => no existing face yet, add it
    face.assignedTagId                        = -1;
    face.assignedRegion                       = assignedRegion;
    face.roles                               |= FacePipelineFaceTagsIface::ForEditing;
    FacePipelineExtendedPackage::Ptr package  = d->buildPackage(info, face, image);

    package->databaseFaces.setRole(FacePipelineFaceTagsIface::ForEditing);
    d->send(package);

    return FaceTagsEditor::unconfirmedEntry(info.id(), face.assignedTagId, face.assignedRegion);
}

FaceTagsIface FacePipeline::editRegion(const ImageInfo& info,
                                       const DImg& image,
                                       const FaceTagsIface& databaseFace,
                                       const TagRegion& newRegion)
{
    FacePipelineFaceTagsIface face            = FacePipelineFaceTagsIface(databaseFace);
    face.assignedTagId                        = -1;
    face.assignedRegion                       = newRegion;
    face.roles                               |= FacePipelineFaceTagsIface::ForEditing;
    FacePipelineExtendedPackage::Ptr package  = d->buildPackage(info, face, image);

    package->databaseFaces.setRole(FacePipelineFaceTagsIface::ForEditing);
    d->send(package);

    face.setRegion(newRegion);

    return face;
}

void FacePipeline::remove(const ImageInfo& info,
                          const FaceTagsIface& databaseFace)
{
    FacePipelineExtendedPackage::Ptr package = d->buildPackage(info,
                                                               FacePipelineFaceTagsIface(databaseFace),
                                                               DImg());
    package->databaseFaces.setRole(FacePipelineFaceTagsIface::ForEditing);
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

} // namespace Digikam
