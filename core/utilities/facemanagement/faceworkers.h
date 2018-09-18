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

#ifndef DIGIKAM_FACE_WORKERS_H
#define DIGIKAM_FACE_WORKERS_H

// Qt includes

#include <QExplicitlySharedDataPointer>
#include <QMetaMethod>
#include <QMutex>
#include <QSharedData>
#include <QWaitCondition>

// Local includes

#include "facepipeline_p.h"
#include "facedetector.h"
#include "faceutils.h"
#include "previewloadthread.h"
#include "thumbnailloadthread.h"
#include "workerobject.h"

namespace Digikam
{

class Q_DECL_HIDDEN DetectionWorker : public WorkerObject
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

    FaceDetector                 detector;
    FacePipeline::Private* const d;
};

// ----------------------------------------------------------------------------------------

class Q_DECL_HIDDEN RecognitionWorker : public WorkerObject
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

    FaceImageRetriever           imageRetriever;
    RecognitionDatabase          database;
    FacePipeline::Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_FACE_WORKERS_H
