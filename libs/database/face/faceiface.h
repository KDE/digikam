/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : libkface interface, also allowing easy manipulation of face tags
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#include <QString>
#include <QMap>
#include <QList>

// Libkface includes

#include <libkface/database.h>
#include <libkface/kface.h>

// Local includes

#include "digikam_export.h"

using namespace KFaceIface;

class QImage;

namespace Digikam
{

class DImg;
class FaceIfacePriv;

class DIGIKAM_DATABASE_EXPORT FaceIface
{

public:
    
    FaceIface();
    ~FaceIface();

    /**
     * Tells if the image has been scanned for faces or not
     */
    bool                hasBeenScanned(qlonglong imageid);
    
    /**
     * Detects faces from the image and returns a list of faces
     * @param image The DImg , from which face rectangles will be cropped out
     * @param imageid The image id from the database
     * @return A list of faces found in the given image. With cropped face images.
     */
    QList<Face>         findAndTagFaces(DImg& image, qlonglong imageid);
    
    /**
     * Reads rect tags in the specified image and returns a list of faces, read from these tags.
     * Very fast compared to findAndTagFaces. It is "read-only".
     */
    QList<Face>         findFacesFromTags(DImg& image, qlonglong imageid);
    
    /**
     * Unassigns all face tags from the image and sets it's scanned property to false.
     */
    void                forgetFaceTags(qlonglong imageid);
    
    /**
     * Returns a list of all tag rectangles for the image. Unlike findAndTagFaces, this does not take a DImg, 
     * because it returns only a QRect, not a Face, so no need of cropping a face rectangle.
     */
    QList<QRect>        getTagRects(qlonglong imageid);
    
    /**
     * Returns a list of image ids of all images in the DB which have a specified person within.
     * @param tagId The person's id. Or tag id for the person tag. Same thing.
     * @param repeat If repeat is specified as true, then if person with id tagId is found n times in an image, 
     * that image's id will be pushed into the returned list n times. Useful for thumbnailing as I see it.
     */
    QList<qlonglong>    imagesWithPerson(int tagId, bool repeat = false);
    
    /** 
     * Returns the number of faces present in an image.
     */
    int                 numberOfFaces(qlonglong imageid);
    
    /**
     * Returns a boolean value indicating whether the given tagId represents a person
     */
    bool                isPerson(int tagId);
    
    QString rectToString(const QRect& rect)        const;
    QRect   stringToRect(const QString& string)   const;
private:
    
    FaceIfacePriv* const d;

};

}  // Namespace Digikam


#endif // FACEIFACE_H
