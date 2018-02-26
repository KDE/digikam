/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "panoactionthread.h"

// Qt includes

#include <QSharedPointer>
#include <QTemporaryDir>
#include <QString>

// KDE includes

#include <ThreadWeaver/Queue>
#include <ThreadWeaver/Sequence>
#include <ThreadWeaver/QObjectDecorator>
#include <threadweaver/debuggingaids.h>

// Local includes

#include "digikam_debug.h"
#include "panotasks.h"

using namespace ThreadWeaver;

namespace Digikam
{

class PanoActionThread::Private
{
public:

    explicit Private(QObject* const parent = 0)
      : threadQueue(new Queue(parent))
    {
        ThreadWeaver::setDebugLevel(true, 10);
    }

    ~Private()
    {
        threadQueue->dequeue();
        threadQueue->requestAbort();
        threadQueue->finish();
    }

    QSharedPointer<QTemporaryDir> preprocessingTmpDir;
    QSharedPointer<Queue>         threadQueue;
};

PanoActionThread::PanoActionThread(QObject* const parent)
    : QObject(parent),
      d(new Private(this))
{
    // PanoActionThread init
    qRegisterMetaType<PanoActionData>();

    d->threadQueue->setMaximumNumberOfThreads(qMax(QThread::idealThreadCount(), 1));
    qCDebug(DIGIKAM_GENERAL_LOG) << "Starting Main Thread";
}

PanoActionThread::~PanoActionThread()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Calling action thread destructor";

    delete d;
}

void PanoActionThread::cancel()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Cancel (PanoAction Thread)";
    d->threadQueue->dequeue();
    d->threadQueue->requestAbort();
}

void PanoActionThread::finish()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Finish (PanoAction Thread)";
    // Wait for all queued jobs to finish
    d->threadQueue->finish();
    d->threadQueue->resume();
}

void PanoActionThread::preProcessFiles(const QList<QUrl>& urlList,
                                       PanoramaItemUrlsMap& preProcessedMap,
                                       QUrl& baseUrl,
                                       QUrl& cpFindPtoUrl,
                                       QUrl& cpCleanPtoUrl,
                                       bool celeste,
                                       PanoramaFileType fileType,
                                       bool gPano,
                                       const QString& huginVersion,
                                       const QString& cpCleanPath,
                                       const QString& cpFindPath)
{
    QString prefix = QDir::tempPath() + QLatin1Char('/') +
                     QLatin1String("digiKam-panorama-tmp-XXXXXX");

    d->preprocessingTmpDir = QSharedPointer<QTemporaryDir>(new QTemporaryDir(prefix));

    QSharedPointer<Sequence> jobSeq(new Sequence());

    QSharedPointer<Collection> preprocessJobs(new Collection());

    int id = 0;

    for (const QUrl& file : urlList)
    {
        preProcessedMap.insert(file, PanoramaPreprocessedUrls());

        QObjectDecorator* const t
            = new QObjectDecorator(new PreProcessTask(d->preprocessingTmpDir->path(),
                                                      id++,
                                                      preProcessedMap[file],
                                                      file));

        connect(t, SIGNAL(started(ThreadWeaver::JobPointer)),
                this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

        connect(t, SIGNAL(done(ThreadWeaver::JobPointer)),
                this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

        (*preprocessJobs) << JobPointer(t);
    }

    (*jobSeq) << preprocessJobs;

    QObjectDecorator* const pto
        = new QObjectDecorator(new CreatePtoTask(d->preprocessingTmpDir->path(),
                                                 fileType,
                                                 baseUrl,
                                                 urlList,
                                                 preProcessedMap,
                                                 gPano,
                                                 huginVersion));

    connect(pto, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(pto, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

    (*jobSeq) << JobPointer(pto);

    QObjectDecorator* const cpFind
        = new QObjectDecorator(new CpFindTask(d->preprocessingTmpDir->path(),
                                              baseUrl,
                                              cpFindPtoUrl,
                                              celeste,
                                              cpFindPath));

    connect(cpFind, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(cpFind, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

    (*jobSeq) << JobPointer(cpFind);

    QObjectDecorator* const cpClean
        = new QObjectDecorator(new CpCleanTask(d->preprocessingTmpDir->path(),
                                               cpFindPtoUrl,
                                               cpCleanPtoUrl,
                                               cpCleanPath));

    connect(cpClean, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(cpClean, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotDone(ThreadWeaver::JobPointer)));

    (*jobSeq) << JobPointer(cpClean);

    d->threadQueue->enqueue(jobSeq);
}

void PanoActionThread::optimizeProject(QUrl& ptoUrl,
                                       QUrl& optimizePtoUrl,
                                       QUrl& viewCropPtoUrl,
                                       bool levelHorizon,
                                       bool buildGPano,
                                       const QString& autooptimiserPath,
                                       const QString& panoModifyPath)
{
    QSharedPointer<Sequence> jobs(new Sequence());

    QObjectDecorator* const ot
        = new QObjectDecorator(new OptimisationTask(d->preprocessingTmpDir->path(),
                                                    ptoUrl,
                                                    optimizePtoUrl,
                                                    levelHorizon,
                                                    buildGPano,
                                                    autooptimiserPath));

    connect(ot, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(ot, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

    (*jobs) << ot;

    QObjectDecorator* const act
        = new QObjectDecorator(new AutoCropTask(d->preprocessingTmpDir->path(),
                                                optimizePtoUrl,
                                                viewCropPtoUrl,
                                                buildGPano,
                                                panoModifyPath));

    connect(act, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(act, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotDone(ThreadWeaver::JobPointer)));

    (*jobs) << act;

    d->threadQueue->enqueue(jobs);
}

void PanoActionThread::generatePanoramaPreview(QSharedPointer<const PTOType> ptoData,
                                               QUrl& previewPtoUrl,
                                               QUrl& previewMkUrl,
                                               QUrl& previewUrl,
                                               const PanoramaItemUrlsMap& preProcessedUrlsMap,
                                               const QString& makePath,
                                               const QString& pto2mkPath,
                                               const QString& huginExecutorPath,
                                               bool hugin2015,
                                               const QString& enblendPath,
                                               const QString& nonaPath)
{
    QSharedPointer<Sequence> jobs(new Sequence());

    QObjectDecorator* const ptoTask
        = new QObjectDecorator(new CreatePreviewTask(d->preprocessingTmpDir->path(),
                                                     ptoData,
                                                     previewPtoUrl,
                                                     preProcessedUrlsMap));

    connect(ptoTask, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(ptoTask, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

    (*jobs) << ptoTask;

    if (!hugin2015)
    {
        appendStitchingJobs(jobs,
                            previewPtoUrl,
                            previewMkUrl,
                            previewUrl,
                            preProcessedUrlsMap,
                            JPEG,
                            makePath,
                            pto2mkPath,
                            enblendPath,
                            nonaPath,
                            true);
    }
    else
    {
        QObjectDecorator* const huginExecutorTask
            = new QObjectDecorator(new HuginExecutorTask(d->preprocessingTmpDir->path(),
                                                         previewPtoUrl,
                                                         previewUrl,
                                                         JPEG,
                                                         huginExecutorPath,
                                                         true));

        connect(huginExecutorTask, SIGNAL(started(ThreadWeaver::JobPointer)),
                this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

        connect(huginExecutorTask, SIGNAL(done(ThreadWeaver::JobPointer)),
                this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

        (*jobs) << huginExecutorTask;
    }

    d->threadQueue->enqueue(jobs);
}

void PanoActionThread::compileProject(QSharedPointer<const PTOType> basePtoData,
                                      QUrl& panoPtoUrl,
                                      QUrl& mkUrl,
                                      QUrl& panoUrl,
                                      const PanoramaItemUrlsMap& preProcessedUrlsMap,
                                      PanoramaFileType fileType,
                                      const QRect& crop,
                                      const QString& makePath,
                                      const QString& pto2mkPath,
                                      const QString& huginExecutorPath,
                                      bool hugin2015,
                                      const QString& enblendPath,
                                      const QString& nonaPath)
{
    QSharedPointer<Sequence> jobs(new Sequence());

    QObjectDecorator* const ptoTask
        = new QObjectDecorator(new CreateFinalPtoTask(d->preprocessingTmpDir->path(),
                                                      basePtoData,
                                                      panoPtoUrl,
                                                      crop));

    connect(ptoTask, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(ptoTask, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

    (*jobs) << ptoTask;

    if (!hugin2015)
    {
        appendStitchingJobs(jobs,
                            panoPtoUrl,
                            mkUrl,
                            panoUrl,
                            preProcessedUrlsMap,
                            fileType,
                            makePath,
                            pto2mkPath,
                            enblendPath,
                            nonaPath,
                            false);
    }
    else
    {
        QObjectDecorator* const huginExecutorTask
            = new QObjectDecorator(new HuginExecutorTask(d->preprocessingTmpDir->path(),
                                                         panoPtoUrl,
                                                         panoUrl,
                                                         fileType,
                                                         huginExecutorPath,
                                                         false));

        connect(huginExecutorTask, SIGNAL(started(ThreadWeaver::JobPointer)),
                this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

        connect(huginExecutorTask, SIGNAL(done(ThreadWeaver::JobPointer)),
                this, SLOT(slotDone(ThreadWeaver::JobPointer)));

        (*jobs) << huginExecutorTask;
    }

    d->threadQueue->enqueue(jobs);
}

void PanoActionThread::copyFiles(const QUrl& ptoUrl,
                                 const QUrl& panoUrl,
                                 const QUrl& finalPanoUrl,
                                 const PanoramaItemUrlsMap& preProcessedUrlsMap,
                                 bool savePTO,
                                 bool addGPlusMetadata)
{
    QObjectDecorator* const t
        = new QObjectDecorator(new CopyFilesTask(d->preprocessingTmpDir->path(),
                                                 panoUrl,
                                                 finalPanoUrl,
                                                 ptoUrl,
                                                 preProcessedUrlsMap,
                                                 savePTO,
                                                 addGPlusMetadata));

    connect(t, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(t, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotDone(ThreadWeaver::JobPointer)));

    d->threadQueue->enqueue(JobPointer(t));
}

void PanoActionThread::slotStarting(JobPointer j)
{
    QSharedPointer<QObjectDecorator> dec = j.staticCast<QObjectDecorator>();
    PanoTask* const t                    = static_cast<PanoTask*>(dec->job());

    PanoActionData ad;
    ad.starting     = true;
    ad.action       = t->action;
    ad.id           = -1;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Starting (PanoAction Thread) (action):" << ad.action;

    if (t->action == PANO_NONAFILE)
    {
        CompileMKStepTask* const c = static_cast<CompileMKStepTask*>(t);
        ad.id                      = c->id;
    }
    else if (t->action == PANO_PREPROCESS_INPUT)
    {
        PreProcessTask* const p = static_cast<PreProcessTask*>(t);
        ad.id                   = p->id;
    }

    emit starting(ad);
}

void PanoActionThread::slotStepDone(JobPointer j)
{
    QSharedPointer<QObjectDecorator> dec = j.staticCast<QObjectDecorator>();
    PanoTask* const t                    = static_cast<PanoTask*>(dec->job());

    PanoActionData ad;
    ad.starting     = false;
    ad.action       = t->action;
    ad.id           = -1;
    ad.success      = t->success();
    ad.message      = t->errString;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Step done (PanoAction Thread) (action, success):" << ad.action << ad.success;

    if (t->action == PANO_NONAFILE)
    {
        CompileMKStepTask* const c = static_cast<CompileMKStepTask*>(t);
        ad.id                      = c->id;
    }
    else if (t->action == PANO_PREPROCESS_INPUT)
    {
        PreProcessTask* const p = static_cast<PreProcessTask*>(t);
        ad.id                   = p->id;
    }

    if (!ad.success)
    {
        d->threadQueue->dequeue();
    }

    emit stepFinished(ad);
}

void PanoActionThread::slotDone(JobPointer j)
{
    QSharedPointer<QObjectDecorator> dec = j.staticCast<QObjectDecorator>();
    PanoTask* const t                    = static_cast<PanoTask*>(dec->job());

    PanoActionData ad;
    ad.starting     = false;
    ad.action       = t->action;
    ad.id           = -1;
    ad.success      = t->success();
    ad.message      = t->errString;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Done (PanoAction Thread) (action, success):"
                                 << ad.action << ad.success;

    if (t->action == PANO_NONAFILE)
    {
        CompileMKStepTask* const c = static_cast<CompileMKStepTask*>(t);
        ad.id                      = c->id;
    }
    else if (t->action == PANO_PREPROCESS_INPUT)
    {
        PreProcessTask* const p = static_cast<PreProcessTask*>(t);
        ad.id                   = p->id;
    }

    emit jobCollectionFinished(ad);
}

void PanoActionThread::appendStitchingJobs(QSharedPointer<Sequence>& js,
                                           const QUrl& ptoUrl,
                                           QUrl& mkUrl,
                                           QUrl& outputUrl,
                                           const PanoramaItemUrlsMap& preProcessedUrlsMap,
                                           PanoramaFileType fileType,
                                           const QString& makePath,
                                           const QString& pto2mkPath,
                                           const QString& enblendPath,
                                           const QString& nonaPath,
                                           bool preview)
{
    QSharedPointer<Sequence> jobs(new Sequence());

    QObjectDecorator* const createMKTask
        = new QObjectDecorator(new CreateMKTask(d->preprocessingTmpDir->path(),
                                                ptoUrl,
                                                mkUrl,
                                                outputUrl,
                                                fileType,
                                                pto2mkPath,
                                                preview));

    connect(createMKTask, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(createMKTask, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

    (*jobs) << createMKTask;

    for (int i = 0 ; i < preProcessedUrlsMap.size() ; i++)
    {
        QObjectDecorator* const t
            = new QObjectDecorator(new CompileMKStepTask(d->preprocessingTmpDir->path(),
                                                         i,
                                                         mkUrl,
                                                         nonaPath,
                                                         enblendPath,
                                                         makePath,
                                                         preview));

        connect(t, SIGNAL(started(ThreadWeaver::JobPointer)),
                this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

        connect(t, SIGNAL(done(ThreadWeaver::JobPointer)),
                this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

        (*jobs) << t;
    }

    QObjectDecorator* const compileMKTask
        = new QObjectDecorator(new CompileMKTask(d->preprocessingTmpDir->path(),
                                                 mkUrl,
                                                 outputUrl,
                                                 nonaPath,
                                                 enblendPath,
                                                 makePath,
                                                 preview));

    connect(compileMKTask, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(compileMKTask, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotDone(ThreadWeaver::JobPointer)));

    (*jobs) << compileMKTask;

    (*js) << jobs;
}

}  // namespace Digikam
