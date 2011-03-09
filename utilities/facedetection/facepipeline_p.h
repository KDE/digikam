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

// Qt includes

#include <QExplicitlySharedDataPointer>
#include <QMetaMethod>
#include <QMutex>
#include <QSharedData>
#include <QWaitCondition>

// KDE includes

// libkface includes

#include <libkface/facedetector.h>
#include <libkface/recognitiondatabase.h>

// Local includes

#include "faceiface.h"
#include "previewloadthread.h"
#include "thumbnailloadthread.h"
#include "workerobject.h"

namespace Digikam
{

class FacePipelineExtendedPackage : public FacePipelinePackage, public QSharedData
{
public:

    QString filePath;
    DImg    detectionImage; // image scaled to about 0.5 Mpx
    typedef QExplicitlySharedDataPointer<FacePipelineExtendedPackage> Ptr;

public:

    bool operator==(const LoadingDescription& description) const
    {
        return filePath == description.filePath;
    }
};

// ----------------------------------------------------------------------------------------

class PackageLoadingDescriptionList : public QList<FacePipelineExtendedPackage::Ptr>
{
public:

    PackageLoadingDescriptionList() {}
    FacePipelineExtendedPackage::Ptr take(const LoadingDescription& description);
};

// ----------------------------------------------------------------------------------------

class ParallelPipes : public QObject
{
    Q_OBJECT

public:

    ParallelPipes();
    ~ParallelPipes();

    void schedule();
    void deactivate(WorkerObject::DeactivatingMode mode = WorkerObject::FlushSignals);

    void add(WorkerObject* worker);

public:

    QList<WorkerObject*> m_workers;

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    QList<QMetaMethod> m_methods;
    int                m_currentIndex;
};

// ----------------------------------------------------------------------------------------

class ScanStateFilter : public DynamicThread
{
    Q_OBJECT

public:

    ScanStateFilter(FacePipeline::FilterMode mode, FacePipeline::FacePipelinePriv* d);

    void process(const QList<ImageInfo>& infos);
    void process(const ImageInfo& info);

    FacePipelineExtendedPackage::Ptr filter(const ImageInfo& info);

public:

    FacePipeline::FacePipelinePriv* const d;
    FacePipeline::FilterMode              mode;
    FacePipelineDatabaseFace::Roles       tasks;

protected Q_SLOTS:

    void dispatch();

Q_SIGNALS:

    void infosToDispatch();

protected:

    virtual void run();

protected:

    QList<ImageInfo>                        toFilter;
    QList<FacePipelineExtendedPackage::Ptr> toSend;
    QList<ImageInfo>                        toBeSkipped;
};

// ----------------------------------------------------------------------------------------

class PreviewLoader : public PreviewLoadThread
{
    Q_OBJECT

public:

    PreviewLoader(FacePipeline::FacePipelinePriv* d);

    void cancel();
    bool sentOutLimitReached();
    void checkRestart();

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);
    void slotImageLoaded(const LoadingDescription& loadingDescription, const DImg& img);

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    PackageLoadingDescriptionList         scheduledPackages;
    int                                   maximumSentOutPackages;
    FacePipeline::FacePipelinePriv* const d;
};

// ----------------------------------------------------------------------------------------

class DetectionWorker : public WorkerObject
{
    Q_OBJECT

public:

    DetectionWorker(FacePipeline::FacePipelinePriv* d);
    ~DetectionWorker()
    {
        wait();    // protect detector
    }
    DImg scaleForDetection(const DImg& image) const;

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);
    void setAccuracy(double value);
    void setSpecificity(double value);

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    KFaceIface::FaceDetector              detector;
    FacePipeline::FacePipelinePriv* const d;
};

// ----------------------------------------------------------------------------------------

class RecognitionWorker : public WorkerObject
{
    Q_OBJECT

public:

    RecognitionWorker(FacePipeline::FacePipelinePriv* d);
    ~RecognitionWorker()
    {
        wait();    // protect database
    }

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);
    void setThreshold(double threshold);

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    KFaceIface::RecognitionDatabase       database;
    double                                recognitionThreshold;
    FacePipeline::FacePipelinePriv* const d;
};

// ----------------------------------------------------------------------------------------

class DatabaseWriter : public WorkerObject
{
    Q_OBJECT

public:

    DatabaseWriter(FacePipeline::WriteMode mode, FacePipeline::FacePipelinePriv* d);

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    FacePipeline::WriteMode               mode;
    FacePipeline::FacePipelinePriv* const d;
};

// ----------------------------------------------------------------------------------------

class Trainer : public WorkerObject
{
    Q_OBJECT

public:

    Trainer(FacePipeline::FacePipelinePriv* d);
    ~Trainer()
    {
        wait();    // protect detector
    }

protected:

    virtual void aboutToDeactivate();

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    ThumbnailImageCatcher*                catcher;
    KFaceIface::RecognitionDatabase       database;
    FacePipeline::FacePipelinePriv* const d;
};

// ----------------------------------------------------------------------------------------

class FacePipeline::FacePipelinePriv : public QObject
{
    Q_OBJECT

public:

    FacePipelinePriv(FacePipeline* q);

    void processBatch(const QList<ImageInfo>& infos);
    void sendFromFilter(const QList<FacePipelineExtendedPackage::Ptr>& packages);
    void skipFromFilter(const QList<ImageInfo>& infosForSkipping);
    void send(FacePipelineExtendedPackage::Ptr package);
    FacePipelineExtendedPackage::Ptr buildPackage(const ImageInfo& info);
    FacePipelineExtendedPackage::Ptr buildPackage(const ImageInfo& info,
                                                  const FacePipelineDatabaseFace&, const DImg& image);
    FacePipelineExtendedPackage::Ptr buildPackage(const ImageInfo& info,
                                                  const FacePipelineDatabaseFaceList& faces, const DImg& image);
    FacePipelineExtendedPackage::Ptr filterOrBuildPackage(const ImageInfo& info);

    bool hasFinished();
    void checkFinished();
    void start();
    void stop();

    void createThumbnailLoadThread();

public:

    ScanStateFilter*     databaseFilter;
    PreviewLoader*       previewThread;
    DetectionWorker*     detectionWorker;
    ParallelPipes*       parallelDetectors;
    RecognitionWorker*   recognitionWorker;
    DatabaseWriter*      databaseWriter;
    Trainer*             trainer;

    QList<QObject*>      pipeline;

    FaceIface*           iface;
    ThumbnailLoadThread* thumbnailLoadThread;
    bool                 started;
    int                  infosForFiltering;
    int                  packagesOnTheRoad;

public Q_SLOTS:

    void finishProcess(FacePipelineExtendedPackage::Ptr package);

Q_SIGNALS:

    friend class FacePipeline;
    void startProcess(FacePipelineExtendedPackage::Ptr package);

    void accuracyChanged(double accuracy);
    void specificityChanged(double accuracy);
    void thresholdChanged(double threshold);

private:

    FacePipeline* const q;
};

} // namespace Digikam
