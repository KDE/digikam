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

#include "facepipeline_p.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "facepreviewloader.h"
#include "parallelpipes.h"
#include "scanstatefilter.h"

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

void FacePipeline::Private::processBatch(const QList<ItemInfo>& infos)
{
    if (databaseFilter)
    {
        infosForFiltering += infos.size();
        databaseFilter->process(infos);
    }
    else
    {
        foreach (const ItemInfo& info, infos)
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
void FacePipeline::Private::skipFromFilter(const QList<ItemInfo>& infosForSkipping)
{
    infosForFiltering -= infosForSkipping.size();

    emit q->skipped(infosForSkipping);

    // everything skipped?
    checkFinished();
}

FacePipelineExtendedPackage::Ptr FacePipeline::Private::filterOrBuildPackage(const ItemInfo& info)
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

FacePipelineExtendedPackage::Ptr FacePipeline::Private::buildPackage(const ItemInfo& info)
{
    FacePipelineExtendedPackage::Ptr package(new FacePipelineExtendedPackage);
    package->info     = info;
    package->filePath = info.filePath();

    return package;
}

FacePipelineExtendedPackage::Ptr FacePipeline::Private::buildPackage(const ItemInfo& info,
                                                                     const FacePipelineFaceTagsIface& face,
                                                                     const DImg& image)
{
    FacePipelineFaceTagsIfaceList faces;
    faces << face;

    return buildPackage(info, faces, image);
}

FacePipelineExtendedPackage::Ptr FacePipeline::Private::buildPackage(const ItemInfo& info,
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
