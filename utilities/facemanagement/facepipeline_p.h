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

#ifndef FACEPIPELINE_P_H
#define FACEPIPELINE_P_H

#include "facepipeline.h"

// Qt includes

#include <QExplicitlySharedDataPointer>
#include <QMetaMethod>
#include <QMutex>
#include <QSharedData>
#include <QWaitCondition>

// Local includes

#include "facedetector.h"
#include "faceutils.h"
#include "previewloadthread.h"
#include "thumbnailloadthread.h"
#include "workerobject.h"

namespace Digikam
{

class FacePipelineExtendedPackage : public FacePipelinePackage, public QSharedData
{
public:

    QString                                                           filePath;
    DImg                                                              detectionImage; // image scaled to about 0.5 Mpx
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

    PackageLoadingDescriptionList()
    {
    }

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
    void wait();

    void add(WorkerObject* const worker);
    void setPriority(QThread::Priority priority);

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

    ScanStateFilter(FacePipeline::FilterMode mode, FacePipeline::Private* const d);

    void process(const QList<ImageInfo>& infos);
    void process(const ImageInfo& info);

    FacePipelineExtendedPackage::Ptr filter(const ImageInfo& info);

public:

    FacePipeline::Private* const    d;
    FacePipeline::FilterMode        mode;
    FacePipelineFaceTagsIface::Roles tasks;

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

    explicit PreviewLoader(FacePipeline::Private* const d);

    void cancel();
    bool sentOutLimitReached();
    void checkRestart();

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);
    void slotImageLoaded(const LoadingDescription& loadingDescription, const DImg& img);

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    PackageLoadingDescriptionList scheduledPackages;
    int                           maximumSentOutPackages;
    FacePipeline::Private* const  d;
};

// ----------------------------------------------------------------------------------------

class DetectionWorker : public WorkerObject
{
    Q_OBJECT

public:

    explicit DetectionWorker(FacePipeline::Private* const d);
    ~DetectionWorker()
    {
        wait();    // protect detector
    }

    QImage scaleForDetection(const DImg& image) const;

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);
    void setAccuracy(double value);

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    FaceDetector     detector;
    FacePipeline::Private* const d;
};

// ----------------------------------------------------------------------------------------

class FaceImageRetriever
{
public:

    FaceImageRetriever(FacePipeline::Private* const d);
    void cancel();

    ThumbnailImageCatcher* thumbnailCatcher();
    QList<QImage> getDetails(const DImg& src, const QList<QRectF>& rects);
    QList<QImage> getDetails(const DImg& src, const QList<FaceTagsIface>& faces);
    QList<QImage> getThumbnails(const QString& filePath, const QList<FaceTagsIface>& faces);

protected:

    ThumbnailImageCatcher* catcher;
};

// ----------------------------------------------------------------------------------------

class RecognitionWorker : public WorkerObject
{
    Q_OBJECT

public:

    explicit RecognitionWorker(FacePipeline::Private* const d);
    ~RecognitionWorker()
    {
        wait();    // protect database
    }

    /**
     * Set the face recognition algorithm type
     */
    void activeFaceRecognizer(RecognitionDatabase::RecognizeAlgorithm  algorithmType);

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);
    void setThreshold(double threshold);

protected:

    virtual void aboutToDeactivate();

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    FaceImageRetriever               imageRetriever;
    RecognitionDatabase database;
    FacePipeline::Private* const     d;
};

// ----------------------------------------------------------------------------------------

class DatabaseWriter : public WorkerObject
{
    Q_OBJECT

public:

    DatabaseWriter(FacePipeline::WriteMode mode, FacePipeline::Private* const d);

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    FacePipeline::WriteMode      mode;
    ThumbnailLoadThread*         thumbnailLoadThread;
    FacePipeline::Private* const d;
};

// ----------------------------------------------------------------------------------------

class Trainer : public WorkerObject
{
    Q_OBJECT

public:

    explicit Trainer(FacePipeline::Private* const d);
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

    RecognitionDatabase database;
    FaceImageRetriever               imageRetriever;
    FacePipeline::Private* const     d;
};

// ----------------------------------------------------------------------------------------

class DetectionBenchmarker : public WorkerObject
{
    Q_OBJECT

public:

    explicit DetectionBenchmarker(FacePipeline::Private* const d);
    QString result() const;

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    int                          totalImages;
    int                          faces;
    double                       totalPixels;
    double                       facePixels;

    int                          trueNegativeImages;
    int                          falsePositiveImages;

    int                          truePositiveFaces;
    int                          falseNegativeFaces;
    int                          falsePositiveFaces;

    FacePipeline::Private* const d;
};

// ----------------------------------------------------------------------------------------

class RecognitionBenchmarker : public WorkerObject
{
    Q_OBJECT

public:

    explicit RecognitionBenchmarker(FacePipeline::Private* const d);
    QString result() const;

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    class Statistics
    {
    public:

        Statistics();
        int knownFaces;
        int correctlyRecognized;
    };

    QMap<int, Statistics>        results;

    FacePipeline::Private* const d;
    RecognitionDatabase          database;
};

// ----------------------------------------------------------------------------------------

class FacePipeline::Private : public QObject
{
    Q_OBJECT

public:

    explicit Private(FacePipeline* q);

    void processBatch(const QList<ImageInfo>& infos);
    void sendFromFilter(const QList<FacePipelineExtendedPackage::Ptr>& packages);
    void skipFromFilter(const QList<ImageInfo>& infosForSkipping);
    void send(FacePipelineExtendedPackage::Ptr package);
    bool senderFlowControl(FacePipelineExtendedPackage::Ptr package);
    void receiverFlowControl();
    FacePipelineExtendedPackage::Ptr buildPackage(const ImageInfo& info);
    FacePipelineExtendedPackage::Ptr buildPackage(const ImageInfo& info,
                                                  const FacePipelineFaceTagsIface&, const DImg& image);
    FacePipelineExtendedPackage::Ptr buildPackage(const ImageInfo& info,
                                                  const FacePipelineFaceTagsIfaceList& faces, const DImg& image);
    FacePipelineExtendedPackage::Ptr filterOrBuildPackage(const ImageInfo& info);

    bool hasFinished();
    void checkFinished();
    void start();
    void stop();
    void wait();
    void applyPriority();

    ThumbnailLoadThread* createThumbnailLoadThread();

public:

    ScanStateFilter*                        databaseFilter;
    PreviewLoader*                          previewThread;
    DetectionWorker*                        detectionWorker;
    ParallelPipes*                          parallelDetectors;
    RecognitionWorker*                      recognitionWorker;
    DatabaseWriter*                         databaseWriter;
    Trainer*                                trainer;
    DetectionBenchmarker*                   detectionBenchmarker;
    RecognitionBenchmarker*                 recognitionBenchmarker;

    QList<QObject*>                         pipeline;
    QThread::Priority                       priority;

    QList<ThumbnailLoadThread*>             thumbnailLoadThreads;
    bool                                    started;
    int                                     infosForFiltering;
    int                                     packagesOnTheRoad;
    int                                     maxPackagesOnTheRoad;
    int                                     totalPackagesAdded;

    QList<FacePipelineExtendedPackage::Ptr> delayedPackages;

public Q_SLOTS:

    void finishProcess(FacePipelineExtendedPackage::Ptr package);

Q_SIGNALS:

    friend class FacePipeline;
    void startProcess(FacePipelineExtendedPackage::Ptr package);

    void accuracyChanged(double accuracy);
    void thresholdChanged(double threshold);

private:

    FacePipeline* const q;
};

} // namespace Digikam

#endif // FACEPIPELINE_P_H
