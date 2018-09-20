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

#ifndef DIGIKAM_FACE_BENCH_MARKERS_H
#define DIGIKAM_FACE_BENCH_MARKERS_H

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

class Q_DECL_HIDDEN DetectionBenchmarker : public WorkerObject
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

class Q_DECL_HIDDEN RecognitionBenchmarker : public WorkerObject
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

    class Q_DECL_HIDDEN Statistics
    {
    public:

        Statistics();

    public:

        int knownFaces;
        int correctlyRecognized;
    };

protected:

    QMap<int, Statistics>        results;

    FacePipeline::Private* const d;
    RecognitionDatabase          database;
};

} // namespace Digikam

#endif // DIGIKAM_FACE_BENCH_MARKERS_H
