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

#include "facetagseditor.h"

// Local includes

#include "coredbaccess.h"
#include "coredbconstants.h"
#include "coredboperationgroup.h"
#include "facetags.h"
#include "imageinfo.h"
#include "imagetagpair.h"
#include "tagproperties.h"
#include "tagscache.h"
#include "tagregion.h"
#include "digikam_debug.h"

namespace Digikam
{

// --- Constructor / Destructor -------------------------------------------------------------------------------------

FaceTagsEditor::FaceTagsEditor()
{
}

FaceTagsEditor::~FaceTagsEditor()
{
}

// --- Read from database -----------------------------------------------------------------------------------

int FaceTagsEditor::faceCountForPersonInImage(qlonglong imageid, int tagId ) const
{
    ImageTagPair pair(imageid, tagId);
    return pair.values(ImageTagPropertyName::tagRegion()).size();
}

QList<FaceTagsIface> FaceTagsEditor::databaseFaces(qlonglong imageId) const
{
    return databaseFaces(imageId, FaceTagsIface::NormalFaces);
}

QList<FaceTagsIface> FaceTagsEditor::unconfirmedFaceTagsIfaces(qlonglong imageId) const
{
    return databaseFaces(imageId, FaceTagsIface::UnconfirmedTypes);
}

QList<FaceTagsIface> FaceTagsEditor::databaseFacesForTraining(qlonglong imageId) const
{
    return databaseFaces(imageId, FaceTagsIface::FaceForTraining);
}

QList<FaceTagsIface> FaceTagsEditor::confirmedFaceTagsIfaces(qlonglong imageId) const
{
    return databaseFaces(imageId, FaceTagsIface::ConfirmedName);
}

QList<FaceTagsIface> FaceTagsEditor::databaseFaces(qlonglong imageid, FaceTagsIface::TypeFlags flags) const
{
    QList<FaceTagsIface> faces;
    QStringList         attributes = FaceTagsIface::attributesForFlags(flags);

    foreach(const ImageTagPair& pair, faceImageTagPairs(imageid, flags))
    {
        foreach(const QString& attribute, attributes)
        {
            foreach(const QString& regionString, pair.values(attribute))
            {
                TagRegion region(regionString);
                //qCDebug(DIGIKAM_DATABASE_LOG) << "rect found as "<< region << "for attribute" << attribute << "tag" << pair.tagId();

                if (!region.isValid())
                {
                    continue;
                }

                faces << FaceTagsIface(attribute, imageid, pair.tagId(), region);
            }
        }
    }

    return faces;
}

QList<ImageTagPair> FaceTagsEditor::faceImageTagPairs(qlonglong imageid, FaceTagsIface::TypeFlags flags) const
{
    QList<ImageTagPair> pairs;
    QStringList         attributes = FaceTagsIface::attributesForFlags(flags);

    foreach(const ImageTagPair& pair, ImageTagPair::availablePairs(imageid))
    {
        //qCDebug(DIGIKAM_DATABASE_LOG) << pair.tagId() << pair.properties();
        if (!FaceTags::isPerson(pair.tagId()))
        {
            continue;
        }

        // UnknownName and UnconfirmedName have the same attribute
        if (!(flags & FaceTagsIface::UnknownName) && FaceTags::isTheUnknownPerson(pair.tagId()))
        {
            continue;
        }

        if (!pair.hasAnyProperty(attributes))
        {
            continue;
        }

        pairs << pair;
    }

    return pairs;
}

QList< QRect > FaceTagsEditor::getTagRects(qlonglong imageid) const
{
    QList<QRect>        rectList;
    QList<ImageTagPair> pairs = ImageTagPair::availablePairs(imageid);

    foreach(const ImageTagPair& pair, pairs)
    {
        QStringList regions = pair.values(ImageTagPropertyName::tagRegion());

        foreach(const QString& region, regions)
        {
            QRect rect = TagRegion(region).toRect();

            if (rect.isValid())
            {
                rectList << rect;
            }
        }
    }

    return rectList;
}

int FaceTagsEditor::numberOfFaces(qlonglong imageid) const
{
    // Use case for this? Depending on a use case, we can think of an optimization
    int                 count = 0;
    QList<ImageTagPair> pairs = ImageTagPair::availablePairs(imageid);

    foreach(const ImageTagPair& pair, pairs)
    {
        QStringList regions = pair.values(ImageTagPropertyName::tagRegion());
        count += regions.size();
    }

    return count;
}

// --- Confirming and adding ---

FaceTagsIface FaceTagsEditor::unknownPersonEntry(qlonglong imageId, const TagRegion& region)
{
    return unconfirmedEntry(imageId, -1, region);
}

FaceTagsIface FaceTagsEditor::unconfirmedEntry(qlonglong imageId, int tagId, const TagRegion& region)
{
    return FaceTagsIface(FaceTagsIface::UnconfirmedName, imageId,
                        tagId == -1 ? FaceTags::unknownPersonTagId() : tagId, region);
}

FaceTagsIface FaceTagsEditor::confirmedEntry(const FaceTagsIface& face, int tagId, const TagRegion& confirmedRegion)
{
    return FaceTagsIface(FaceTagsIface::ConfirmedName, face.imageId(),
                        tagId == -1 ? face.tagId() : tagId,
                        confirmedRegion.isValid() ? confirmedRegion : face.region());
}

FaceTagsIface FaceTagsEditor::addManually(const FaceTagsIface& face)
{
    ImageTagPair pair(face.imageId(), face.tagId());
    addFaceAndTag(pair, face, FaceTagsIface::attributesForFlags(face.type()), false);
    return face;
}

FaceTagsIface FaceTagsEditor::changeSuggestedName(const FaceTagsIface& previousEntry, int unconfirmedNameTagId)
{
    if (previousEntry.isConfirmedName())
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Refusing to reset a confirmed name to an unconfirmed name";
        return previousEntry;
    }

    FaceTagsIface newEntry = unconfirmedEntry(previousEntry.imageId(), unconfirmedNameTagId, previousEntry.region());

    if (newEntry == previousEntry)
    {
        return previousEntry;
    }

    removeFace(previousEntry);

    QStringList attributesList = FaceTagsIface::attributesForFlags(FaceTagsIface::UnconfirmedName);

    ImageTagPair pair(newEntry.imageId(), newEntry.tagId());
    // UnconfirmedName and UnknownName have the same attribute
    addFaceAndTag(pair, newEntry, attributesList, false);

    // Add the image to the face to the unconfirmed tag, if it is not the unknown or unconfirmed tag.
    if (!FaceTags::isTheUnknownPerson(unconfirmedNameTagId) && !FaceTags::isTheUnconfirmedPerson(unconfirmedNameTagId))
    {
        ImageTagPair unconfirmedAssociation(newEntry.imageId(),FaceTags::unconfirmedPersonTagId());
        unconfirmedAssociation.addProperty(ImageTagPropertyName::autodetectedPerson(), newEntry.getAutodetectedPersonString());
    }

    return newEntry;
}

FaceTagsIface FaceTagsEditor::confirmName(const FaceTagsIface& face, int tagId, const TagRegion& confirmedRegion)
{
    FaceTagsIface newEntry = confirmedEntry(face, tagId, confirmedRegion);

    if (FaceTags::isTheUnknownPerson(newEntry.tagId()))
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Refusing to confirm unknownPerson tag on face";
        return face;
    }

    ImageTagPair pair(newEntry.imageId(), newEntry.tagId());

    // Remove entry from autodetection
    if (newEntry.tagId() == face.tagId())
    {
        removeFaceAndTag(pair, face, false);
    }
    else
    {
        ImageTagPair pairOldEntry(face.imageId(), face.tagId());
        removeFaceAndTag(pairOldEntry, face, true);
    }

    // Add new full entry
    addFaceAndTag(pair, newEntry,
                  FaceTagsIface::attributesForFlags(FaceTagsIface::ConfirmedName | FaceTagsIface::FaceForTraining),
                  true);

    return newEntry;
}

FaceTagsIface FaceTagsEditor::add(qlonglong imageId, int tagId, const TagRegion& region, bool trainFace)
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "Adding face with rectangle  " << region.toRect () << " to database";
    FaceTagsIface newEntry(FaceTagsIface::ConfirmedName, imageId, tagId, region);
    add(newEntry, trainFace);
    return newEntry;
}

void FaceTagsEditor::add(const FaceTagsIface& face, bool trainFace)
{
    ImageTagPair pair(face.imageId(), face.tagId());
    FaceTagsIface::TypeFlags flags = FaceTagsIface::ConfirmedName;

    if (trainFace)
    {
        flags |= FaceTagsIface::FaceForTraining;
    }

    addFaceAndTag(pair, face, FaceTagsIface::attributesForFlags(flags), true);
}

void FaceTagsEditor::addFaceAndTag(ImageTagPair& pair, const FaceTagsIface& face,
                    const QStringList& properties, bool addTag)
{
    FaceTags::ensureIsPerson(face.tagId());
    QString region = face.region().toXml();

    foreach(const QString& property, properties)
    {
        pair.addProperty(property, region);
    }

    if (addTag)
    {
        addNormalTag(face.imageId(), face.tagId());
    }
}

// --- Removing faces ---

void FaceTagsEditor::removeAllFaces(qlonglong imageid)
{
    QList<int>  tagsToRemove;
    QStringList attributes = FaceTagsIface::attributesForFlags(FaceTagsIface::AllTypes);

    foreach(ImageTagPair pair, faceImageTagPairs(imageid, FaceTagsIface::AllTypes))
    {
        foreach(const QString& attribute, attributes)
        {
            pair.removeProperties(attribute);
        }

        if (pair.isAssigned())
        {
            tagsToRemove << pair.tagId();
        }
    }

    removeNormalTags(imageid, tagsToRemove);
}

void FaceTagsEditor::removeFace(qlonglong imageid, const QRect& rect)
{
    QList<int>          tagsToRemove;
    QStringList         attributes    = FaceTagsIface::attributesForFlags(FaceTagsIface::AllTypes);
    QList<ImageTagPair> pairs         = faceImageTagPairs(imageid, FaceTagsIface::AllTypes);

    for (int i=0; i<pairs.size(); ++i)
    {
        ImageTagPair& pair = pairs[i];

        foreach(const QString& attribute, attributes)
        {
            foreach(const QString& regionString, pair.values(attribute))
            {
                if (rect == TagRegion(regionString).toRect())
                {
                    pair.removeProperty(attribute, regionString);

                    if (pair.isAssigned())
                    {
                        tagsToRemove << pair.tagId();
                    }
                }
            }
        }
    }

    removeNormalTags(imageid, tagsToRemove);
}

void FaceTagsEditor::removeFace(const FaceTagsIface& face)
{
    if (face.isNull())
    {
        return;
    }

    ImageTagPair pair(face.imageId(), face.tagId());
    removeFaceAndTag(pair, face, true);
}

void FaceTagsEditor::removeFaces(const QList<FaceTagsIface>& faces)
{
    foreach(const FaceTagsIface& face, faces)
    {
        if (face.isNull())
        {
            continue;
        }

        ImageTagPair pair(face.imageId(), face.tagId());
        removeFaceAndTag(pair, face, true);
    }
}

void FaceTagsEditor::removeFaceAndTag(ImageTagPair& pair, const FaceTagsIface& face, bool touchTags)
{
    QString regionString = TagRegion(face.region().toRect()).toXml();
    pair.removeProperty(FaceTagsIface::attributeForType(face.type()), regionString);

    if (face.type() == FaceTagsIface::ConfirmedName)
    {
        pair.removeProperty(FaceTagsIface::attributeForType(FaceTagsIface::FaceForTraining), regionString);
    }

    // Remove the unconfirmed property for the image id and the unconfirmed tag with the original tag id and the confirmed region
    ImageTagPair unconfirmedAssociation(face.imageId(),FaceTags::unconfirmedPersonTagId());
    unconfirmedAssociation.removeProperty(ImageTagPropertyName::autodetectedPerson(),face.getAutodetectedPersonString());

    // Tag assigned and no other entry left?
    if (touchTags            &&
        pair.isAssigned()    &&
        !pair.hasProperty(FaceTagsIface::attributeForType(FaceTagsIface::ConfirmedName)))
    {
        removeNormalTag(face.imageId(), pair.tagId());
    }
}

FaceTagsIface FaceTagsEditor::changeRegion(const FaceTagsIface& face, const TagRegion& newRegion)
{
    if (face.isNull() || face.region() == newRegion)
    {
        return face;
    }

    ImageTagPair pair(face.imageId(), face.tagId());
    removeFaceAndTag(pair, face, false);

    FaceTagsIface newFace = face;
    newFace.setRegion(newRegion);
    addFaceAndTag(pair, newFace, FaceTagsIface::attributesForFlags(face.type()), false);
    return newFace;

    // todo: the Training entry is cleared.
}

// --- Editing normal tags ---

void FaceTagsEditor::addNormalTag(qlonglong imageId, int tagId)
{
    ImageInfo(imageId).setTag(tagId);
}

void FaceTagsEditor::removeNormalTag(qlonglong imageId, int tagId)
{
    ImageInfo(imageId).removeTag(tagId);
}

void FaceTagsEditor::removeNormalTags(qlonglong imageId, QList<int> tagIds)
{
    CoreDbOperationGroup group;
    group.setMaximumTime(200);
    ImageInfo info(imageId);

    foreach(int tagId, tagIds)
    {
        info.removeTag(tagId);
        group.allowLift();
    }
}

} // Namespace Digikam
