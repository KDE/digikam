/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : FacesEngine interface, also allowing easy manipulation of face tags
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

#ifndef FACEUTILS_H
#define FACEUTILS_H

// Qt includes

#include <QStringList>

// Local includes

#include "identity.h"
#include "recognitiondatabase.h"
#include "imageinfo.h"
#include "facetagseditor.h"
#include "digikam_export.h"

class QImage;

namespace Digikam
{

class DImg;
class ThumbnailLoadThread;
class ThumbnailImageCatcher;

class FaceUtils : public FaceTagsEditor
{

public:

    enum FaceRecognitionSteps
    {
        DetectFaceRegions,
        DetectAndRecognize
    };

public:

    FaceUtils();
    virtual ~FaceUtils();

    // --- Face detection and recognition ---

    /**
     * The given face list is a result of automatic detection and possibly recognition.
     * The results are written to the database and merged with existing entries.
     * The returned list contains the faces written to the database and has the same size as the given list.
     * If a face was skipped (because of an existing entry), a null FaceTagsIface will be at this place.
     */
    QList<FaceTagsIface> writeUnconfirmedResults(qlonglong imageid,
                                                const QList<QRectF>& detectedFaces,
                                                const QList<Identity> recognitionResults,
                                                const QSize& fullSize);

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

    /**
     * This uses a thumbnail load thread to load the image detail.
     * If requested, the faces will be scaled to the given (fixed) size.
     */
/*
    void                fillImageInFaces(ThumbnailImageCatcher* const catcher, const QString& filePath,
                                         QList<Face>& faceList, const QSize& scaleSize = QSize()) const;
*/

    /**
     * Store the needed thumbnails for the given faces. This can be a huge optimization
     * when the has already been loaded anyway.
     */
    void                storeThumbnails(ThumbnailLoadThread* const thread, const QString& filePath,
                                        const QList<FaceTagsIface>& databaseFaces, const DImg& image);

    /**
     * Conversion
     */
    QList<FaceTagsIface> toFaceTagsIfaces(qlonglong imageid,
                                        const QList<QRectF>& detectedFaces,
                                        const QList<Identity> recognitionResults,
                                        const QSize& fullSize) const;

    /**
     * For display, it may be desirable to display a slightly larger region than the strict
     * face rectangle. This returns a pixel margin commonly used to increase the rectangle size
     * in all four directions.
     */
    static int          faceRectDisplayMargin();

    Identity identityForTag(int tagId, RecognitionDatabase& db) const;
    int                  tagForIdentity(const Identity& identity) const;

protected:

    /* Reimplemented */
    virtual void addNormalTag(qlonglong imageid, int tagId);
    virtual void removeNormalTag(qlonglong imageid, int tagId);
    virtual void removeNormalTags(qlonglong imageid, QList<int> tagId);
};

}  // Namespace Digikam

#endif // FACEUTILS_H
