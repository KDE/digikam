/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : Faces tags editor allowing easy manipulation of face tags
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

#ifndef FACE_TAGS_EDITOR_H
#define FACE_TAGS_EDITOR_H

// Qt includes

#include <QFlags>
#include <QMap>
#include <QList>
#include <QRect>
#include <QString>
#include <QVariant>

// Local includes

#include "facetagsiface.h"
#include "digikam_export.h"
#include "facetags.h"

namespace Digikam
{

class ImageTagPair;

class DIGIKAM_DATABASE_EXPORT FaceTagsEditor
{
public:

    FaceTagsEditor();
    virtual ~FaceTagsEditor();

    // --- Read from database -----------------------------------------------------------------------------------------

    /**
     * Returns the number of faces present in an image.
     */
    int                  numberOfFaces(qlonglong imageid) const;

    /**
     * Returns the number of faces a particular person has in the specified image
     */
    int                  faceCountForPersonInImage(qlonglong imageid, int tagId) const;

    /**
     * Reads the FaceTagsIfaces for the given image id from the database
     */
    QList<FaceTagsIface> databaseFaces(qlonglong imageid) const;
    QList<FaceTagsIface> unconfirmedFaceTagsIfaces(qlonglong imageid) const;
    QList<FaceTagsIface> databaseFacesForTraining(qlonglong imageid) const;
    QList<FaceTagsIface> confirmedFaceTagsIfaces(qlonglong imageid) const;

    /**
     * Returns a list of all tag rectangles for the image. Unlike findAndTagFaces, this does not take a DImg,
     * because it returns only a QRect, not a Face, so no need of cropping a face rectangle.
     */
    QList<QRect>         getTagRects(qlonglong imageid) const;

    // --- Add / Confirm ---

    /**
     * Adds a new entry to the database.
     * The convenience wrapper will return the newly created entry.
     * If trainFace is true, the face will also be listed in the db as needing training.
     * The tag of the face will, if necessary, be converted to a person tag.
     */
    void          add(const FaceTagsIface& face, bool trainFace = true);
    FaceTagsIface add(qlonglong imageid, int tagId, const TagRegion& region, bool trainFace = true);
    FaceTagsIface addManually(const FaceTagsIface& face);

    /**
     * Switches an unknownPersonEntry or unconfirmedEntry to an unconfirmedEntry (with a different suggested name)
     */
    FaceTagsIface        changeSuggestedName(const FaceTagsIface& previousEntry, int unconfirmedNameTagId);

    /**
     * Assign the name tag for given face entry.
     * Pass the tagId if it changed or was newly assigned (UnknownName).
     * Pass the new, corrected region if it changed.
     * If the default values are passed, tag id or region are taken from the given face.
     * The given face should be an unchanged entry read from the database.
     * The confirmed tag will, if necessary, be converted to a person tag.
     * Returns the newly inserted entry.
     */
    FaceTagsIface        confirmName(const FaceTagsIface& face, int tagId = -1, const TagRegion& confirmedRegion = TagRegion());

    /**
     * Returns the entry that would be added if the given face is confirmed.
     */
    static FaceTagsIface confirmedEntry(const FaceTagsIface& face, int tagId = -1, const TagRegion& confirmedRegion = TagRegion());
    /**
     * Returns the entry that would be added if the given face is autodetected.
     * If tagId is -1, the unknown person will be taken.
     */
    static FaceTagsIface unconfirmedEntry(qlonglong imageId, int tagId, const TagRegion& region);
    static FaceTagsIface unknownPersonEntry(qlonglong imageId, const TagRegion& region);

    // --- Remove entries ---

    /**
     * Remove the given face.
     * If appropriate, the tag is also removed.
     */
    void                removeFace(const FaceTagsIface& face);
    void                removeFaces(const QList<FaceTagsIface>& faces);

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
    FaceTagsIface        changeRegion(const FaceTagsIface& face, const TagRegion& newRegion);

    // --- Utilities ---

    QList<FaceTagsIface> databaseFaces(qlonglong imageId, FaceTagsIface::TypeFlags flags)     const;
    QList<ImageTagPair>  faceImageTagPairs(qlonglong imageid, FaceTagsIface::TypeFlags flags) const;

protected:

    void addFaceAndTag(ImageTagPair& pair, const FaceTagsIface& face, const QStringList& properties, bool addTag);
    void removeFaceAndTag(ImageTagPair& pair, const FaceTagsIface& face, bool touchTags);

    virtual void addNormalTag(qlonglong imageid, int tagId);
    virtual void removeNormalTag(qlonglong imageid, int tagId);
    virtual void removeNormalTags(qlonglong imageid, QList<int> tagId);
};

}  // Namespace Digikam

#endif // FACE_TAGS_EDITOR_H
