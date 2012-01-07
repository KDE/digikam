/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : database interface, also allowing easy manipulation of face tags
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

#include "faceiface.h"

// Qt includes

#include <QImage>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// Libkface Includes

#include <libkface/database.h>
#include <libkface/face.h>

// Local includes

#include "databaseaccess.h"
#include "databaseconstants.h"
#include "databaseoperationgroup.h"
#include "dimg.h"
#include "facetags.h"
#include "imageinfo.h"
#include "imagetagpair.h"
#include "metadatamanager.h"
#include "tagproperties.h"
#include "tagscache.h"
#include "tagregion.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

// --- Constructor / Destructor -------------------------------------------------------------------------------------

FaceIface::FaceIface()
{
}

FaceIface::~FaceIface()
{
}

// --- Mark for scanning and training -------------------------------------------------------------------------------

bool FaceIface::hasBeenScanned(qlonglong imageid) const
{
    return hasBeenScanned(ImageInfo(imageid));
}

bool FaceIface::hasBeenScanned(const ImageInfo& info) const
{
    return info.tagIds().contains(FaceTags::scannedForFacesTagId());
}

void FaceIface::markAsScanned(qlonglong imageid, bool hasBeenScanned) const
{
    return markAsScanned(ImageInfo(imageid), hasBeenScanned);
}

void FaceIface::markAsScanned(const ImageInfo& info, bool hasBeenScanned) const
{
    if (hasBeenScanned)
    {
        ImageInfo(info).setTag(FaceTags::scannedForFacesTagId());
    }
    else
    {
        ImageInfo(info).removeTag(FaceTags::scannedForFacesTagId());
    }
}

// --- Convert between KFaceIface::Face and DatabaseFace ---

QList<KFaceIface::Face> FaceIface::facesFromTags(qlonglong imageId) const
{
    return toFaces(databaseFaces(imageId));
}

QList<KFaceIface::Face> FaceIface::unconfirmedFacesFromTags(qlonglong imageId) const
{
    return toFaces(unconfirmedDatabaseFaces(imageId));
}

QList<KFaceIface::Face> FaceIface::toFaces(const QList<DatabaseFace>& databaseFaces) const
{
    QList<KFaceIface::Face> faceList;
    foreach(const DatabaseFace& databaseFace, databaseFaces)
    {
        QRect rect = databaseFace.region().toRect();

        if (!rect.isValid())
        {
            continue;
        }

        KFaceIface::Face f;
        f.setRect(rect);

        if (FaceTags::isTheUnknownPerson(databaseFace.tagId()))
        {
            f.setName(FaceTags::faceNameForTag(databaseFace.tagId()));
        }

        faceList += f;
    }

    return faceList;
}

QList<DatabaseFace> FaceIface::toDatabaseFaces(const DImg& image, qlonglong imageid,
                                    const QList<KFaceIface::Face>& faceList) const
{
    QList<DatabaseFace> faces;
    foreach(const KFaceIface::Face& face, faceList)
    {
        // We'll get the unknownPersonTagId if face.name() is null
        int tagId          = FaceTags::tagForFaceName(face.name());
        QRect fullSizeRect = TagRegion::mapToOriginalSize(image, face.toRect());

        if (!tagId || !fullSizeRect.isValid())
        {
            faces << DatabaseFace();
            continue;
        }

        //kDebug() << "New Entry" << fullSizeRect << tagId;
        faces << DatabaseFace(DatabaseFace::UnconfirmedName, imageid, tagId, TagRegion(fullSizeRect));
    }
    return faces;
}

// --- Images in faces and thumbnails ---

void FaceIface::fillImageInFaces(const DImg& image, QList<KFaceIface::Face>& faceList, const QSize& scaleSize) const
{
    for (int i = 0; i < faceList.size(); ++i)
    {
        fillImageInFace(image, faceList[i], scaleSize);
    }
}

void FaceIface::fillImageInFace(const DImg& image, KFaceIface::Face& face, const QSize& scaleSize) const
{
    QRect rect = TagRegion::mapFromOriginalSize(image, face.toRect());

    if (rect.isValid())
    {
        DImg detail = image.copy(rect);

        if (scaleSize.isValid())
        {
            detail = detail.smoothScale(scaleSize, Qt::IgnoreAspectRatio);
        }

        face.setImage(toImage(detail));
    }
}

void FaceIface::fillImageInFaces(ThumbnailImageCatcher* catcher, const QString& filePath,
                                 QList<KFaceIface::Face>& faces, const QSize& scaleSize) const
{
    foreach(const KFaceIface::Face& face, faces)
    {
        QRect rect = face.toRect();

        if (!rect.isValid())
        {
            continue;
        }

        catcher->thread()->find(filePath, rect);
        catcher->enqueue();
    }

    QList<QImage> details = catcher->waitForThumbnails();
    kDebug() << details.size();

    for (int i=0; i<faces.size(); ++i)
    {
        KFaceIface::Face& face = faces[i];
        QImage detail          = details[i];
        kDebug() << "Loaded detail" << detail.size();

        if (scaleSize.isValid())
        {
            detail = detail.scaled(scaleSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }

        face.setImage(detail);
    }
}

KFaceIface::Image FaceIface::toImage(const DImg& image)
{
    return KFaceIface::Image(image.width(), image.height(),
                             image.sixteenBit(), image.hasAlpha(),
                             image.bits());
}

void FaceIface::storeThumbnails(ThumbnailLoadThread* thread, const QString& filePath,
                                const QList<DatabaseFace>& databaseFaces, const DImg& image)
{
    foreach(const DatabaseFace& face, databaseFaces)
    {
        QList<QRect> rects;
        rects << face.region().toRect();
        const int margin = faceRectDisplayMargin();
        rects << face.region().toRect().adjusted(-margin, -margin, margin, margin);

        foreach(const QRect& rect, rects)
        {
            QRect mapped = TagRegion::mapFromOriginalSize(image, rect);
            QImage detail = image.copyQImage(mapped);
            thread->storeDetailThumbnail(filePath, rect, detail, true);
        }
    }
}

// --- Face detection: merging results ------------------------------------------------------------------------------------

QList<DatabaseFace> FaceIface::writeUnconfirmedResults(const DImg& image, qlonglong imageid,
                                                       const QList<KFaceIface::Face>& faceList)
{
    // Build list of new entries
    QList<DatabaseFace> newFaces = toDatabaseFaces(image, imageid, faceList);

    if (newFaces.isEmpty())
    {
        return newFaces;
    }

    // list of existing entries
    QList<DatabaseFace> currentFaces = databaseFaces(imageid);

    // merge new with existing entries
    for (int i=0; i<newFaces.size(); ++i)
    {
        DatabaseFace& newFace = newFaces[i];

        QList<DatabaseFace> overlappingEntries;
        foreach(const DatabaseFace& oldFace, currentFaces)
        {
            double minOverlap = oldFace.isConfirmedName() ? 0.25 : 0.5;

            if (oldFace.region().intersects(newFace.region(), minOverlap))
            {
                overlappingEntries << oldFace;
                kDebug() << "Entry" << oldFace.region() << oldFace.tagId()
                         << "overlaps" << newFace.region() << newFace.tagId() << ", skipping";
            }
        }

        // The purpose if the next scope is to merge entries:
        // A confirmed face will never be overwritten.
        // If a name is set to an old face, it will only be replaced by a new face with a name.
        if (!overlappingEntries.isEmpty())
        {
            if (newFace.isUnknownName())
            {
                // we have no name in the new face. Do we have one in the old faces?
                for (int i=0; i<overlappingEntries.size(); ++i)
                {
                    const DatabaseFace& oldFace = overlappingEntries.at(i);

                    if (oldFace.isUnknownName())
                    {
                        // remove old face
                    }
                    else
                    {
                        // skip new entry if any overlapping face has a name, and we do not
                        newFace = DatabaseFace();
                        break;
                    }
                }
            }
            else
            {
                // we have a name in the new face. Do we have names in overlapping faces?
                for (int i=0; i<overlappingEntries.size(); ++i)
                {
                    DatabaseFace& oldFace = overlappingEntries[i];

                    if (oldFace.isUnknownName())
                    {
                        // remove old face
                    }
                    else if (oldFace.isUnconfirmedName())
                    {
                        if (oldFace.tagId() == newFace.tagId())
                        {
                            // remove smaller face
                            if (oldFace.region().intersects(newFace.region(), 1))
                            {
                                newFace = DatabaseFace();
                                break;
                            }

                            // else remove old face
                        }
                        else
                        {
                            // assume new recognition is more trained, remove older face
                        }
                    }
                    else if (oldFace.isConfirmedName())
                    {
                        // skip new entry, confirmed has of course priority
                        newFace = DatabaseFace();
                    }
                }
            }
        }

        // if we did not decide to skip this face, add is to the db now
        if (!newFace.isNull())
        {
            // list will contain all old entries that should still be removed
            removeFaces(overlappingEntries);

            ImageTagPair pair(imageid, newFace.tagId());
            // UnconfirmedName and UnknownName have the same attribute
            addFaceAndTag(pair, newFace, DatabaseFace::attributesForFlags(DatabaseFace::UnconfirmedName), false);
        }
    }

    return newFaces;
}

// --- Editing normal tags, reimplemented with MetadataManager ---

void FaceIface::addNormalTag(qlonglong imageid, int tagId)
{
    MetadataManager::instance()->assignTag(ImageInfo(imageid), tagId);
}

void FaceIface::removeNormalTag(qlonglong imageId, int tagId)
{
    MetadataManager::instance()->removeTag(ImageInfo(imageId), tagId);
}

void FaceIface::removeNormalTags(qlonglong imageId, QList<int> tagIds)
{
    MetadataManager::instance()->removeTags(ImageInfo(imageId), tagIds);
}

// --- Utilities ---

int FaceIface::faceRectDisplayMargin()
{
    /*
     * Do not change that value unless you know what you do.
     * There are a lot of pregenerated thumbnails in user's databases,
     * expensive to regenerate, depending on this very value.
     */
    return 70;
}

} // Namespace Digikam
