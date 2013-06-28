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

#include "facetagseditor.h"

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// Local includes

#include "databaseaccess.h"
#include "databaseconstants.h"
#include "databaseoperationgroup.h"
#include "facetags.h"
#include "imageinfo.h"
#include "imagetagpair.h"
#include "tagproperties.h"
#include "tagscache.h"
#include "tagregion.h"

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

QList<DatabaseFace> FaceTagsEditor::databaseFaces(qlonglong imageId) const
{
    return databaseFaces(imageId, DatabaseFace::NormalFaces);
}

QList<DatabaseFace> FaceTagsEditor::unconfirmedDatabaseFaces(qlonglong imageId) const
{
    return databaseFaces(imageId, DatabaseFace::UnconfirmedTypes);
}

QList<DatabaseFace> FaceTagsEditor::databaseFacesForTraining(qlonglong imageId) const
{
    return databaseFaces(imageId, DatabaseFace::FaceForTraining);
}

QList<DatabaseFace> FaceTagsEditor::confirmedDatabaseFaces(qlonglong imageId) const
{
    return databaseFaces(imageId, DatabaseFace::ConfirmedName);
}

QList<DatabaseFace> FaceTagsEditor::databaseFaces(qlonglong imageid, DatabaseFace::TypeFlags flags) const
{
    QList<DatabaseFace> faces;
    QStringList         attributes = DatabaseFace::attributesForFlags(flags);

    foreach(const ImageTagPair& pair, faceImageTagPairs(imageid, flags))
    {
        foreach(const QString& attribute, attributes)
        {
            foreach(const QString& regionString, pair.values(attribute))
            {
                TagRegion region(regionString);
                //kDebug() << "rect found as "<< region << "for attribute" << attribute << "tag" << pair.tagId();

                if (!region.isValid())
                {
                    continue;
                }

                faces << DatabaseFace(attribute, imageid, pair.tagId(), region);
            }
        }
    }

    return faces;
}

QList<ImageTagPair> FaceTagsEditor::faceImageTagPairs(qlonglong imageid, DatabaseFace::TypeFlags flags) const
{
    QList<ImageTagPair> pairs;
    QStringList         attributes = DatabaseFace::attributesForFlags(flags);

    foreach(const ImageTagPair& pair, ImageTagPair::availablePairs(imageid))
    {
        //kDebug() << pair.tagId() << pair.properties();
        if (!FaceTags::isPerson(pair.tagId()))
        {
            continue;
        }

        // UnknownName and UnconfirmedName have the same attribute
        if (!(flags & DatabaseFace::UnknownName) && FaceTags::isTheUnknownPerson(pair.tagId()))
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

DatabaseFace FaceTagsEditor::unknownPersonEntry(qlonglong imageId, const TagRegion& region)
{
    return unconfirmedEntry(imageId, -1, region);
}

DatabaseFace FaceTagsEditor::unconfirmedEntry(qlonglong imageId, int tagId, const TagRegion& region)
{
    return DatabaseFace(DatabaseFace::UnconfirmedName, imageId,
                        tagId == -1 ? FaceTags::unknownPersonTagId() : tagId, region);
}

DatabaseFace FaceTagsEditor::confirmedEntry(const DatabaseFace& face, int tagId, const TagRegion& confirmedRegion)
{
    return DatabaseFace(DatabaseFace::ConfirmedName, face.imageId(),
                        tagId == -1 ? face.tagId() : tagId,
                        confirmedRegion.isValid() ? confirmedRegion : face.region());
}

DatabaseFace FaceTagsEditor::addManually(const DatabaseFace& face)
{
    ImageTagPair pair(face.imageId(), face.tagId());
    addFaceAndTag(pair, face, DatabaseFace::attributesForFlags(face.type()), false);
    return face;
}

DatabaseFace FaceTagsEditor::changeSuggestedName(const DatabaseFace& previousEntry, int unconfirmedNameTagId)
{
    if (previousEntry.isConfirmedName())
    {
        kDebug() << "Refusing to reset a confirmed name to an unconfirmed name";
        return previousEntry;
    }

    DatabaseFace newEntry = unconfirmedEntry(previousEntry.imageId(), unconfirmedNameTagId, previousEntry.region());

    if (newEntry == previousEntry)
    {
        return previousEntry;
    }

    removeFace(previousEntry);

    ImageTagPair pair(newEntry.imageId(), newEntry.tagId());
    // UnconfirmedName and UnknownName have the same attribute
    addFaceAndTag(pair, newEntry, DatabaseFace::attributesForFlags(DatabaseFace::UnconfirmedName), false);
    return newEntry;
}

DatabaseFace FaceTagsEditor::confirmName(const DatabaseFace& face, int tagId, const TagRegion& confirmedRegion)
{
    DatabaseFace newEntry = confirmedEntry(face, tagId, confirmedRegion);

    if (FaceTags::isTheUnknownPerson(newEntry.tagId()))
    {
        kDebug() << "Refusing to confirm unknownPerson tag on face";
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
                  DatabaseFace::attributesForFlags(DatabaseFace::ConfirmedName | DatabaseFace::FaceForTraining),
                  true);

    return newEntry;
}

DatabaseFace FaceTagsEditor::add(qlonglong imageId, int tagId, const TagRegion& region, bool trainFace)
{
    kDebug () << "Adding face with rectangle  " << region.toRect () << " to database";
    DatabaseFace newEntry(DatabaseFace::ConfirmedName, imageId, tagId, region);
    add(newEntry, trainFace);
    return newEntry;
}

void FaceTagsEditor::add(const DatabaseFace& face, bool trainFace)
{
    ImageTagPair pair(face.imageId(), face.tagId());
    DatabaseFace::TypeFlags flags = DatabaseFace::ConfirmedName;

    if (trainFace)
    {
        flags |= DatabaseFace::FaceForTraining;
    }

    addFaceAndTag(pair, face, DatabaseFace::attributesForFlags(flags), true);
}

void FaceTagsEditor::addFaceAndTag(ImageTagPair& pair, const DatabaseFace& face,
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
    QStringList attributes = DatabaseFace::attributesForFlags(DatabaseFace::AllTypes);

    foreach(ImageTagPair pair, faceImageTagPairs(imageid, DatabaseFace::AllTypes))
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
    QStringList         attributes    = DatabaseFace::attributesForFlags(DatabaseFace::AllTypes);
    QList<ImageTagPair> pairs         = faceImageTagPairs(imageid, DatabaseFace::AllTypes);

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

void FaceTagsEditor::removeFace(const DatabaseFace& face)
{
    if (face.isNull())
    {
        return;
    }

    ImageTagPair pair(face.imageId(), face.tagId());
    removeFaceAndTag(pair, face, true);
}

void FaceTagsEditor::removeFaces(const QList<DatabaseFace>& faces)
{
    foreach(const DatabaseFace& face, faces)
    {
        if (face.isNull())
        {
            continue;
        }

        ImageTagPair pair(face.imageId(), face.tagId());
        removeFaceAndTag(pair, face, true);
    }
}

void FaceTagsEditor::removeFaceAndTag(ImageTagPair& pair, const DatabaseFace& face, bool touchTags)
{
    QString regionString = TagRegion(face.region().toRect()).toXml();
    pair.removeProperty(DatabaseFace::attributeForType(face.type()), regionString);

    if (face.type() == DatabaseFace::ConfirmedName)
    {
        pair.removeProperty(DatabaseFace::attributeForType(DatabaseFace::FaceForTraining), regionString);
    }

    // Tag assigned and no other entry left?
    if (touchTags            &&
        pair.isAssigned()    &&
        !pair.hasProperty(DatabaseFace::attributeForType(DatabaseFace::ConfirmedName)))
    {
        removeNormalTag(face.imageId(), pair.tagId());
    }
}

DatabaseFace FaceTagsEditor::changeRegion(const DatabaseFace& face, const TagRegion& newRegion)
{
    if (face.isNull() || face.region() == newRegion)
    {
        return face;
    }

    ImageTagPair pair(face.imageId(), face.tagId());
    removeFaceAndTag(pair, face, false);

    DatabaseFace newFace = face;
    newFace.setRegion(newRegion);
    addFaceAndTag(pair, newFace, DatabaseFace::attributesForFlags(face.type()), false);
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
    DatabaseOperationGroup group;
    group.setMaximumTime(200);
    ImageInfo info(imageId);

    foreach(int tagId, tagIds)
    {
        info.removeTag(tagId);
        group.allowLift();
    }
}

} // Namespace Digikam
