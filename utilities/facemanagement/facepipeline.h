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

#ifndef FACEPIPELINE_H
#define FACEPIPELINE_H

// Qt includes

#include <QFlags>
#include <QThread>

// Local includes

#include "identity.h"
#include "digikam_debug.h"
#include "facetagsiface.h"
#include "dimg.h"
#include "imageinfo.h"
#include "recognitiondatabase.h"

namespace Digikam
{

class FacePipelineFaceTagsIface : public FaceTagsIface
{
public:

    enum Role
    {
        NoRole             = 0,

        /// Source
        GivenAsArgument    = 1 << 0,
        ReadFromDatabase   = 1 << 1,
        DetectedFromImage  = 1 << 2,

        /// Task
        ForRecognition     = 1 << 10,
        ForConfirmation    = 1 << 11,
        ForTraining        = 1 << 12,
        ForEditing         = 1 << 13, // Add, Change or remove

        /// Executed action (task is cleared)
        Confirmed          = 1 << 20,
        Trained            = 1 << 21,
        Edited             = 1 << 22
    };
    Q_DECLARE_FLAGS(Roles, Role)

public:

    FacePipelineFaceTagsIface();
    FacePipelineFaceTagsIface(const FaceTagsIface& face);

public:

    Roles     roles;
    int       assignedTagId;
    TagRegion assignedRegion;
};

// ------------------------------------------------------------------------------------

class FacePipelineFaceTagsIfaceList : public QList<FacePipelineFaceTagsIface>
{
public:

    FacePipelineFaceTagsIfaceList();
    FacePipelineFaceTagsIfaceList(const QList<FaceTagsIface>& faces);

    FacePipelineFaceTagsIfaceList& operator=(const QList<FaceTagsIface>& faces);

    void setRole(FacePipelineFaceTagsIface::Roles role);
    void clearRole(FacePipelineFaceTagsIface::Roles role);
    void replaceRole(FacePipelineFaceTagsIface::Roles remove, FacePipelineFaceTagsIface::Roles add);

    QList<FaceTagsIface> toFaceTagsIfaceList() const;

    FacePipelineFaceTagsIfaceList facesForRole(FacePipelineFaceTagsIface::Roles role) const;
};

// ------------------------------------------------------------------------------------

class FacePipelinePackage
{
public:

    enum ProcessFlag
    {
        NotProcessed            = 0,
        PreviewImageLoaded      = 1 << 0,
        ProcessedByDetector     = 1 << 1,
        ProcessedByRecognizer   = 1 << 2,
        WrittenToDatabase       = 1 << 3,
        ProcessedByTrainer      = 1 << 4
    };
    Q_DECLARE_FLAGS(ProcessFlags, ProcessFlag)

public:

    FacePipelinePackage()
        : processFlags(NotProcessed)
    {
    }

public:

    ImageInfo                     info;
    DImg                          image;
    QList<QRectF>                 detectedFaces;
    QList<Identity>               recognitionResults;
    FacePipelineFaceTagsIfaceList databaseFaces;

    ProcessFlags                  processFlags;
};

// ------------------------------------------------------------------------------------

class FacePipeline : public QObject
{
    Q_OBJECT

public:

    enum FilterMode
    {
        /// Will read any given image
        ScanAll,
        /// Will skip any image that is already marked as scanned
        SkipAlreadyScanned,
        /// Will read unconfirmed faces for recognition
        ReadUnconfirmedFaces,
        /// Will read faces marked for training
        ReadFacesForTraining,
        /// Will read faces which are confirmed
        ReadConfirmedFaces
    };

    enum WriteMode
    {
        /// Write results. Merge with existing entries.
        NormalWrite,
        /// Add new results. Previous unconfirmed results will be cleared.
        OverwriteUnconfirmed
    };

public:

    explicit FacePipeline();
    ~FacePipeline();

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
     * - DatabaseEditor: Can confirm or reject faces
     *
     * PlugParallel: You can call this instead of the simple plugging method.
     * Depending on the number of processor cores of the machine and the memory cost,
     * more than one element may be plugged and process parallelly for this part of the pipeline.
     *
     * Supported combinations:
     *  (Database Filter ->) (Preview Loader ->) Detector -> Recognizer (-> DatabaseWriter)
     *  (Database Filter ->) (Preview Loader ->) Detector (-> DatabaseWriter)
     *  (Database Filter ->) (Preview Loader ->) Recognizer (-> DatabaseWriter)
     *  DatabaseEditor
     *  Trainer
     *  DatabaseEditor -> Trainer
     */
    void plugDatabaseFilter(FilterMode mode);
    void plugRerecognizingDatabaseFilter();
    void plugRetrainingDatabaseFilter();
    void plugPreviewLoader();
    void plugFaceDetector();
    void plugParallelFaceDetectors();
    void plugFaceRecognizer();
    void plugDatabaseWriter(WriteMode mode);
    void plugDatabaseEditor();
    void plugTrainer();
    void plugDetectionBenchmarker();
    void plugRecognitionBenchmarker();
    void construct();

    /**
     * Set the face recognition algorithm type
     */
    void activeFaceRecognizer(RecognitionDatabase::RecognizeAlgorithm algorithmType);

    /**
     * Cancels all processing
     */
    void cancel();

    /**
     * Cancels and waits for the pipeline to finish
     */
    void shutDown();

    bool hasFinished() const;
    QString benchmarkResult() const;

    /**
     * Set the priority of the threads used by this pipeline.
     * The default setting is QThread::LowPriority.
     */
    void setPriority(QThread::Priority priority);
    QThread::Priority priority() const;

public Q_SLOTS:

    /**
     * Processes the given image info. If a filter is installed,
     * returns false if the info is skipped, or true if it is processed.
     * If no preview loader is plugged, you must provide a DImg for detection or recognition.
     * Any of the signals below will only be emitted if true is returned.
     */
    bool process(const ImageInfo& info);
    bool process(const ImageInfo& info, const DImg& image);

    /**
     * Confirm the face. Pass the original face, and additionally tag id or region
     * if they changed. Returns the confirmed face entry immediately purely for convenience,
     * it is not yet in the database (connect to signal processed() to react when the processing finished).
     * If a trainer is plugged, the face will be trained.
     */
    FaceTagsIface confirm(const ImageInfo& info, const FaceTagsIface& face,
                         int assignedTagId = 0, const TagRegion& assignedRegion = TagRegion());
    FaceTagsIface confirm(const ImageInfo& info, const FaceTagsIface& face, const DImg& image,
                         int assignedTagId = 0, const TagRegion& assignedRegion = TagRegion());
    /**
     * Train the given faces.
     */
    void train(const ImageInfo& info, const QList<FaceTagsIface>& faces);
    void train(const ImageInfo& info, const QList<FaceTagsIface>& faces, const DImg& image);

    /**
     * Remove the given face.
     */
    void remove(const ImageInfo& info, const FaceTagsIface& face);

    /**
     * Add an entry manually.
     */
    FaceTagsIface addManually(const ImageInfo& info, const DImg& image, const TagRegion& assignedRegion);

    /**
     * Change the given face's region to newRegion.
     * Does not care for training atm.
     */
    FaceTagsIface editRegion(const ImageInfo& info, const DImg& image,
                            const FaceTagsIface& databaseFace, const TagRegion& newRegion);

    /**
     * Batch processing. If a filter is installed, the skipped() signal
     * will inform about skipped infos. Filtering is done in a thread, returns immediately.
     * Some of the signals below will be emitted in any case.
     */
    void process(const QList<ImageInfo>& infos);

    void setDetectionAccuracy(double accuracy);

Q_SIGNALS:

    /// Emitted when processing is scheduled.
    void scheduled();

    /// Emitted when processing has started
    void started(const QString& message);

    /// Emitted when one package begins processing
    void processing(const FacePipelinePackage& package);
    /// Emitted when one package has finished processing
    void processed(const FacePipelinePackage& package);
    void progressValueChanged(float progress);

    /// Emitted when the last package has finished processing
    void finished();

    /// Emitted when one or several packages were skipped, usually because they have already been scanned.
    void skipped(const QList<ImageInfo>& skippedInfos);

public:

    class Private;

private:

    Private* const d;
    friend class Private;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::FacePipelinePackage)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::FacePipelineFaceTagsIface::Roles)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::FacePipelinePackage::ProcessFlags)

#endif // FACEPIPELINE_H
