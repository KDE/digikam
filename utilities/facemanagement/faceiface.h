/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : libkface interface, also allowing easy manipulation of face tags
 *
 * Copyright (C) 2010-2011 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#ifndef FACEIFACE_H
#define FACEIFACE_H

// Local includes

#include "facetagseditor.h"
#include "digikam_export.h"

namespace KFaceIface
{
    class Face;
    class Image;
}

class QImage;

namespace Digikam
{

class DImg;
class ThumbnailLoadThread;
class ThumbnailImageCatcher;

class FaceIface : public FaceTagsEditor
{

public:

    enum FaceRecognitionSteps
    {
        DetectFaceRegions,
        DetectAndRecognize
    };

public:

    FaceIface();
    virtual ~FaceIface();

    // --- Face detection and recognition ---

    /**
     * The given face list is a result of automatic detection and possibly recognition.
     * The results are written to the database and merged with existing entries.
     * The returned list contains the faces written to the database and has the same size as the given list.
     * If a face was skipped (because of an existing entry), a null DatabaseFace will be at this place.
     */
    QList<DatabaseFace> writeUnconfirmedResults(const DImg& image, qlonglong imageid, const QList<KFaceIface::Face>& faceList);

    /**
     * Detects faces from the image and returns a list of faces
     * @param image The DImg , from which face rectangles will be cropped out
     * @param imageid The image id from the database
     * @return A list of faces found in the given image. With cropped face images.
     */
    QList<KFaceIface::Face> findAndTagFaces(const DImg& image, qlonglong imageid, FaceRecognitionSteps steps = DetectAndRecognize);

    /**
     * Tries to recognize a Face, returns a string containing the name for the face.
     * Respects the match threshold.
     */
    QString             recognizeFace(const KFaceIface::Face& face);

    // --- Training ---

    /**
     * For the given images, train all faces marked for training.
     */
    void                trainImages(const QList<ImageInfo>& imageInfos);

    /**
     * Updates libkface's face database with a list of Face objects
     * Any faces that have a null name or image will be dropped.
     */
    void                trainFaces(const QList<KFaceIface::Face>& faceList);

    /**
     * Updates libkface's face database with a list of Face objects
     * Any faces that have a null name or image will be dropped.
     */
    void                trainFace(const KFaceIface::Face& face);

    // --- Status flags ---

    /**
     * Tells if the image has been scanned for faces or not
     */
    bool                hasBeenScanned(const ImageInfo& info) const;
    bool                hasBeenScanned(qlonglong imageid) const;

    /**
     * Marks the image as scanned for faces.
     */
    void                markAsScanned(qlonglong imageid, bool hasBeenScanned = true) const;
    void                markAsScanned(const ImageInfo& info, bool hasBeenScanned = true) const;

    // --- Utilities ---

    QList<KFaceIface::Face> facesFromTags(qlonglong imageId) const;
    QList<KFaceIface::Face> unconfirmedFacesFromTags(qlonglong imageId) const;

    /**
     * Edits the given face(s): From the given DImg, the face regions are copied.
     * If requested, the faces will be scaled to the given (fixed) size.
     */
    void                fillImageInFace(const DImg& image, KFaceIface::Face& face, const QSize& scaleSize = QSize()) const;
    void                fillImageInFaces(const DImg& image, QList<KFaceIface::Face>& faceList,
                                         const QSize& scaleSize = QSize()) const;
    /**
     * This uses a thumbnail load thread to load the image detail.
     * If requested, the faces will be scaled to the given (fixed) size.
     */
    void                fillImageInFaces(ThumbnailImageCatcher* const catcher, const QString& filePath,
                                         QList<KFaceIface::Face>& faceList, const QSize& scaleSize = QSize()) const;

    /**
     * Store the needed thumbnails for the given faces. This can be a huge optimization
     * when the has already been loaded anyway.
     */
    void                storeThumbnails(ThumbnailLoadThread* const thread, const QString& filePath,
                                        const QList<DatabaseFace>& databaseFaces, const DImg& image);

    /**
     * Converts the DImg to a KFaceIface::Image
     */
    static KFaceIface::Image toImage(const DImg& image);

    /**
     * Conversion
     */
    QList<KFaceIface::Face> toFaces(const QList<DatabaseFace>& databaseFaces) const;
    QList<DatabaseFace> toDatabaseFaces(const DImg& image, qlonglong imageid,
                                        const QList<KFaceIface::Face>& faces) const;

    /**
     * For display, it may be desirable to display a slightly larger region than the strict
     * face rectangle. This returns a pixel margin commonly used to increase the rectangle size
     * in all four directions.
     */
    static int          faceRectDisplayMargin();

protected:

    /* Reimplemented */
    virtual void addNormalTag(qlonglong imageid, int tagId);
    virtual void removeNormalTag(qlonglong imageid, int tagId);
    virtual void removeNormalTags(qlonglong imageid, QList<int> tagId);
};

}  // Namespace Digikam

#endif // FACEIFACE_H
