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

#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

// Qt includes

#include <QFlags>

// KDE includes

#include <kdebug.h>

// libkface includes

#include <libkface/kface.h>

// Local includes

#include "dimg.h"
#include "imageinfo.h"

namespace Digikam
{

class FacePipelinePackage
{
public:

    enum ProcessFlag
    {
        NotProcessed            = 0,
        PreviewImageLoaded      = 1 << 0,
        ProcessedByDetector     = 1 << 1,
        ProcessedByRecognizer   = 1 << 2,
        WrittenToDatabase       = 1 << 3
    };
    Q_DECLARE_FLAGS(ProcessFlags, ProcessFlag);

public:

    FacePipelinePackage() 
        : processFlags(NotProcessed)
    {
    }

    ImageInfo               info;
    DImg                    image;
    QList<KFaceIface::Face> faces;
    ProcessFlags            processFlags;
};

// ------------------------------------------------------------------------------------

class FacePipeline : public QObject
{
    Q_OBJECT

public:

    FacePipeline();
    ~FacePipeline();

    enum FilterMode
    {
        /// Will rescan any given image, removing existing unconfirmed face entries
        RescanAll,
        /// Will skip any image that is already marked as scanned
        SkipAlreadyScanned
    };

    /**
     * You can plug these four different steps in the working pipeline.
     * 1) Call any of the four plug...() methods. See below for supported combinations.
     * 2) Call construct() to set up the pipeline.
     *
     * - Database filter: Prepares database records and/or filters out items.
     *   See FilterMode for specification.
     * - Preview loader: If no preview loader is plugged, you must provide
     *   a DImg for face detection and recognition
     * - Face Detector: If no recognizer is plugged, all detected face are marked
     *   as the unknown person
     * - Face Recognizer: If no detector is plugged, only already scanned faces
     *   marked as unknown will be processed. They are implicitly read from the database.
     * - DatabaseWriter: Writes the detection and recognition results to the database.
     *   The trainer works on a completely different storage and is not affected by the database writer.
     *
     * PlugParallel: You can call this instead of the simple plugging method.
     * Depending on the number of processor cores of the machine and the memory cost,
     * more than one element may be plugged and process parallelly for this part of the pipeline.
     *
     * Supported combinations without providing a DImg:
     *  (Database Filter ->) Preview Loader -> Detector -> Recognizer (-> DatabaseWriter)
     *  (Database Filter ->) Preview Loader -> Detector (-> DatabaseWriter)
     *  (Database Filter ->) Preview Loader -> Recognizer (-> DatabaseWriter)
     *
     * Supported combinations providing a loaded DImg:
     *  (Database Filter ->) Detector -> Recognizer (-> DatabaseWriter)
     *  (Database Filter ->) Detector (-> DatabaseWriter)
     *  (Database Filter ->) Recognizer (-> DatabaseWriter)
     */

    void plugDatabaseFilter(FilterMode mode);
    void plugPreviewLoader();
    void plugFaceDetector();
    void plugParallelFaceDetectors();
    void plugFaceRecognizer();
    void plugDatabaseWriter();
    void construct();

    /** Cancels all processing */
    void cancel();

    bool hasFinished() const;

public Q_SLOTS:

    /**
     * Processes the given image info. If a filter is installed,
     * returns false if the info is skipped, or true if it is processed.
     * If no preview loader is plugged, you must provide a DImg.
     * Any of the signals below will only be emitted if true is returned.
     */
    bool process(const ImageInfo& info);
    bool process(const ImageInfo& info, const DImg& image);

    /**
     * Batch processing. If a filter is installed, the skipped() signal
     * will inform about skipped infos. Filtering is done in a thread, returns immediately.
     * Some of the signals below will be emitted in any case.
     */
    void process(const QList<ImageInfo>& infos);

Q_SIGNALS:

    /// Emitted when one package has finished processing
    void processed(const FacePipelinePackage& package);
    /// Emitted when the last package has finished processing
    void finished();
    /// Emitted when one or several packages were skipped, usually because they have already been scanned.
    void skipped(const QList<ImageInfo>& skippedInfos);

public:

    class FacePipelinePriv;

private:

    FacePipelinePriv* const d;
    friend class FacePipelinePriv;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::FacePipelinePackage)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::FacePipelinePackage::ProcessFlags)

#endif // FACEDETECTOR_H
