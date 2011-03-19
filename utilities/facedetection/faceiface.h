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
class ThumbnailLoadThread;
class ThumbnailImageCatcher;

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
    bool                isPerson(int tagId) const;

    bool                isTheUnknownPerson(int tagId) const;

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
     * The suggested parent tag for persons
     */
    int                 personParentTag() const;

    /**
     * Looks for the given person name under the People tags tree, and returns an ID. Returns 0 if no name found.
     * Per default, fullName is the same as name.
     * As parentId of -1 signals to look for any tag, a valid parentId will limit the search to direct children
     * of this tag. parentId of 0 means top-level tag.
     */
    int                 tagForPerson(const QString& name, int parentId = -1, const QString& fullName = QString()) const;

    /**
     * First, looks for the given person name in person tags, and returns an ID.
     * If not, creates a new tag.
     * Per default, fullName is the same as name.
     */
    int                 getOrCreateTagForPerson(const QString& name, int parentId = -1, const QString& fullName = QString()) const;

    /**
     * Ensure that the given tag is a person tag. If not, it will be converted.
     * Optionally, pass the full name. (tag name is not changed)
     */
    void                ensureIsPerson(int tagId, const QString& fullName = QString()) const;

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
    QList<DatabaseFace> databaseFacesForTraining(qlonglong imageid) const;
    QList<DatabaseFace> confirmedDatabaseFaces(qlonglong imageid) const;

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
    void                fillImageInFaces(ThumbnailImageCatcher* catcher, const QString& filePath,
                                         QList<KFaceIface::Face>& faceList, const QSize& scaleSize = QSize()) const;

    /**
     * Store the needed thumbnails for the given faces. This can be a huge optimization
     * when the has already been loaded anyway.
     */
    void                storeThumbnails(ThumbnailLoadThread* thread, const QString& filePath,
                                        const QList<DatabaseFace>& databaseFaces, const DImg& image);

    /**
     * Returns a list of all tag rectangles for the image. Unlike findAndTagFaces, this does not take a DImg,
     * because it returns only a QRect, not a Face, so no need of cropping a face rectangle.
     */
    QList<QRect>        getTagRects(qlonglong imageid) const;

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


    // --- Add / Confirm ---

    /**
     * Adds a new entry to the database.
     * The convenience wrapper will return the newly created entry.
     * If trainFace is true, the face will also be listed in the db as needing training.
     * The tag of the face will, if necessary, be converted to a person tag.
     */
    void add(const DatabaseFace& face, bool trainFace = true);
    DatabaseFace add(qlonglong imageid, int tagId, const TagRegion& region, bool trainFace = true);
    DatabaseFace addManually(const DatabaseFace& face);

    /**
     * Assign the name tag for given face entry.
     * Pass the tagId if it changed or was newly assigned (UnknownName).
     * Pass the new, corrected region if it changed.
     * If the default values are passed, tag id or region are taken from the given face.
     * The given face should be an unchanged entry read from the database.
     * The confirmed tag will, if necessary, be converted to a person tag.
     * Returns the newly inserted entry.
     */
    DatabaseFace        confirmName(const DatabaseFace& face, int tagId = -1, const TagRegion& confirmedRegion = TagRegion());

    /**
     * Returns the entry that would be added if the given face is confirmed.
     */
    static DatabaseFace confirmedEntry(const DatabaseFace& face, int tagId = -1, const TagRegion& confirmedRegion = TagRegion());
    /**
     * Returns the entry that would be added if the given face is autodetected.
     * If tagId is -1, the unknown person will be taken.
     */
    DatabaseFace unconfirmedEntry(qlonglong imageId, int tagId, const TagRegion& region);
    DatabaseFace unknownPersonEntry(qlonglong imageId, const TagRegion& region);

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
     * Remove the given face.
     * If appropriate, the tag is also removed.
     */
    void                removeFace(const DatabaseFace& face);
    void                removeFaces(const QList<DatabaseFace>& faces);

    /**
     * Unassigns all face tags from the image and sets it's scanned property to false.
     */
    void                removeAllFaces(qlonglong imageid);

    /**
     * Remove a face or the face for a certain rect from an image.
     */
    void                removeFace(qlonglong imageid, const QRect& rect);

    // --- Edit entry ---

    /**
     * Changes the region of the given entry. Returns the face with the new region set.
     */
    DatabaseFace        changeRegion(const DatabaseFace& face, const TagRegion& newRegion);

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
     * Converts the DImg to a KFaceIface::Image
     */
    static KFaceIface::Image toImage(const DImg& image);

    /**
     * Utilities
     */
    QList<KFaceIface::Face> toFaces(const QList<DatabaseFace>& databaseFaces) const;
    QList<DatabaseFace> databaseFaces(qlonglong imageId, DatabaseFace::TypeFlags flags) const;
    QList<ImageTagPair> faceImageTagPairs(qlonglong imageid, DatabaseFace::TypeFlags flags) const;
    QList<DatabaseFace> toDatabaseFaces(const DImg& image, qlonglong imageid, 
                                        const QList<KFaceIface::Face>& faces) const;

private:

    class FaceIfacePriv;
    FaceIfacePriv* const d;
} ;

}  // Namespace Digikam

#endif // FACEIFACE_H
