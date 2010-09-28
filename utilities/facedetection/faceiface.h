/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : libkface interface, also allowing easy manipulation of face tags
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#ifndef FACEIFACE_H
#define FACEIFACE_H

// Qt includes

#include <QFlags>
#include <QMap>
#include <QList>
#include <QRect>
#include <QString>
#include <QVariant>

// Local includes

#include "databaseface.h"
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
class ImageInfo;
class ImageTagPair;

class FaceIface
{

public:

    enum FaceRecognitionSteps
    {
        DetectFaceRegions,
        DetectAndRecognize
    };

public:

    FaceIface();
    ~FaceIface();

    // --- Person tags -----------------------------------------------------------------------------------

    /**
     * Returns a boolean value indicating whether the given tagId represents a person
     */
    bool         isPerson(int tagId) const;

    /**
     * A method to return a list of all person tags in the DB
     */
    QList<int>          allPersonTags() const;

    /**
     * A method to return a list of all person tag names in the DB
     */
    QList<QString>      allPersonNames() const;

    /**
     * A method to return a list of all person tag paths in the DB
     */
    QList<QString>      allPersonPaths() const;

    /**
     * Looks for the given person name under the People tags tree, and returns an ID. Returns 0 if no name found.
     * Per default, fullName is the same as name.
     */
    int                 tagForPerson(const QString& name, const QString& fullName = QString()) const;

    /**
     * First, looks for the given person name in person tags, and returns an ID.
     * If not, creates a new tag.
     * Per default, fullName is the same as name.
     */
    int                 getOrCreateTagForPerson(const QString& name, const QString& fullName = QString()) const;

    QString             getNameForRect(qlonglong imageid, const QRect& faceRect) const;

    /**
     * Translate between the name set in a face, and the tag used by digikam
     */
    int                 tagForFaceName(const QString& kfaceId) const;

    QString             faceNameForTag(int tagId) const;

    // --- Read from database -----------------------------------------------------------------------------------------

    /**
     * Returns the number of faces present in an image.
     */
    int                 numberOfFaces(qlonglong imageid) const;

    /**
     * Returns the number of faces a particular person has in the specified image
     */
    int                 faceCountForPersonInImage(qlonglong imageid, int tagId) const;

    /**
     * Reads rect tags in the specified image and returns a list of faces, read from these tags.
     * Very fast compared to findAndTagFaces. It is "read-only".
     * findFacesFromTags returns all faces, findUnconfirmedFacesFromTags only unconfirmed ones.
     * Call fillImageInFaces() to set the face image.
     */
    QList<KFaceIface::Face> facesFromTags(qlonglong imageid) const;
    QList<KFaceIface::Face> unconfirmedFacesFromTags(qlonglong imageid) const;

    /**
     * Reads the DatabaseFaces for the given image id from the database
     */
    QList<DatabaseFace> databaseFaces(qlonglong imageid) const;
    QList<DatabaseFace> unconfirmedDatabaseFaces(qlonglong imageid) const;

    /**
     * Edits the given face(s): From the given DImg, the face regions are copied.
     */
    void                fillImageInFace(const DImg& image, KFaceIface::Face& face) const;
    void                fillImageInFaces(const DImg& image, QList<KFaceIface::Face>& faceList) const;

    /**
     * Returns a list of all tag rectangles for the image. Unlike findAndTagFaces, this does not take a DImg,
     * because it returns only a QRect, not a Face, so no need of cropping a face rectangle.
     */
    QList<QRect>        getTagRects(qlonglong imageid) const;

    /**
     * Convert image-lister extra value from ioslave to a DatabaseFace
     */
    DatabaseFace        databaseFaceFromListing(qlonglong imageid, const QList<QVariant>& extraValues);

    // --- Face detection and recognition ---

    /**
     * The given face list is a result of automatic detection and possibly recognition.
     * The results are written to the database.
     */
    void writeUnconfirmedResults(const DImg& image, qlonglong imageid, const QList<KFaceIface::Face>& faceList);

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


    // --- Confirmation ---

    /**
     * Assign the name tag for given name / tagId and rect.
     * Optionally, you can pass the rectangle for which this region was previously stored,
     * if the rectangle was adjusted.
     */
    DatabaseFace        confirmName(qlonglong imageid, const QString& name, const QRect& rect, const QRect& previousRect = QRect());
    DatabaseFace        confirmName(qlonglong imageid, int tagId, const QRect& rect, const QRect& previousRect = QRect());

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

    // --- Remove entries ---

    /**
     * Unassigns all face tags from the image and sets it's scanned property to false.
     */
    void                removeAllFaces(qlonglong imageid);

    /**
     * Remove a face or the face for a certain rect from an image.
     */
    void                removeFace(qlonglong imageid, const QRect& rect);
    void                removeFace(const DatabaseFace &face);

    /**
     * Reads configuration settings for detection accuracy and recognition suggestion threshold
     * which were set in the settings page. Default values are assumed if the settings are not found.
     * If any batch job is going on, it will immediately reflect the changes if they are made in the settings page.
     */
    void                readConfigSettings();

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

    /**
     * For display, it may be desirable to display a slightly larger region than the strict
     * face rectangle. This returns a pixel margin commonly used to increase the rectangle size
     * in all four directions.
     */
    static int          faceRectDisplayMargin();

    /**
     * Returns the DImg, if appropriate, scaled to the recommended size for face detection
     */
    static DImg         scaleForDetection(const DImg& image);

    /**
     * Converts the DImg to a KFaceIface::Image
     */
    static KFaceIface::Image toImage(const DImg& image);

    /**
     * Utilities
     */
    QList<KFaceIface::Face> toFaces(const QList<DatabaseFace>& databaseFaces) const;
    QList<DatabaseFace> databaseFaces(qlonglong imageId, DatabaseFace::TypeFlags flags) const;
    QList<ImageTagPair> faceImageTagPairs(qlonglong imageid, DatabaseFace::TypeFlags flags) const;

private:

    class FaceIfacePriv;
    FaceIfacePriv* const d;
} ;

}  // Namespace Digikam

#endif // FACEIFACE_H
