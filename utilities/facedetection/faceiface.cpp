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
#include <kstandarddirs.h>

// Libkface Includes

#include <libkface/database.h>
#include <libkface/face.h>

// Local includes

#include "databaseaccess.h"
#include "databaseconstants.h"
#include "dimg.h"
#include "imageinfo.h"
#include "imagetagpair.h"
#include "metadatamanager.h"
#include "tagproperties.h"
#include "tagscache.h"
#include "tagregion.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

// --- FaceIfacePriv ----------------------------------------------------------------------------------------

class FaceIface::FaceIfacePriv
{
public:

    FaceIfacePriv(FaceIface* q) : q(q)
    {
        setupTags();
    }

    ~FaceIfacePriv()
    {
    }

public:

    int      scannedForFacesTagId() const;
    int      peopleTagId() const;
    int      unknownPeopleTagId() const;

    void     setupTags();
    QString  tagPath(const QString& name, int parentId) const;
    void     makeFaceTag(int tagId, const QString& fullName) const;
    int      findFirstTagWithProperty(const QString& property, const QString& value = QString()) const;
    int      tagForName(const QString& name, int tagId, int parentId,
                        const QString& givenFullName, bool convert, bool create) const;
    void     add(ImageTagPair& pair, const DatabaseFace& face, const QStringList& properties, bool addTag);
    void     remove(ImageTagPair& pair, const DatabaseFace& face, bool touchTags);

private:

    FaceIface* const q;
};

// --- Private methods, face tag utility methods --------------------------------------------------------

int FaceIface::FaceIfacePriv::scannedForFacesTagId() const
{
    return TagsCache::instance()->getOrCreateInternalTag(InternalTagName::scannedForFaces()); // no i18n
}

int FaceIface::FaceIfacePriv::peopleTagId() const
{
    return TagsCache::instance()->getOrCreateTag(i18nc("People on your photos", "People"));
}

int FaceIface::FaceIfacePriv::unknownPeopleTagId() const
{
    QList<int> ids = TagsCache::instance()->tagsWithPropertyCached(TagPropertyName::unknownPerson());

    if (!ids.isEmpty())
    {
        return ids.first();
    }

    return 0;
}

void FaceIface::FaceIfacePriv::setupTags()
{
    // Internal tag
    unknownPeopleTagId();

    // Faces parent tag
    peopleTagId();

    // Unknown people tag
    if (!unknownPeopleTagId())
    {
        int unknownPeopleTagId = TagsCache::instance()->getOrCreateTag(tagPath(i18nc(
                                 "The list of detected faces from the collections but not recognized",
                                 "Unknown"), peopleTagId()));
        TagProperties props(unknownPeopleTagId);
        props.setProperty(TagPropertyName::person(), QString()); // no name associated
        props.setProperty(TagPropertyName::unknownPerson(), QString()); // special property
    }
}

int FaceIface::FaceIfacePriv::findFirstTagWithProperty(const QString& property, const QString& value) const
{
    QList<int> candidates = TagsCache::instance()->tagsWithProperty(property, value);

    if (!candidates.isEmpty())
    {
        return candidates.first();
    }

    return 0;
}

QString FaceIface::FaceIfacePriv::tagPath(const QString& name, int parentId) const
{
    QString faceParentTagName = TagsCache::instance()->tagName(parentId);
    return faceParentTagName + '/' + name;
}

void FaceIface::FaceIfacePriv::makeFaceTag(int tagId, const QString& fullName) const
{
    QString kfaceId  = fullName;
    /*
     *    // find a unique kfaceId
     *    for (int i=0; d->findFirstTagWithProperty(TagPropertyName::kfaceId(), kfaceId); ++i)
     *        kfaceId = fullName + QString(" (%1)").arg(i);
     */
    TagProperties props(tagId);
    props.setProperty(TagPropertyName::person(), fullName);
    props.setProperty(TagPropertyName::kfaceId(), kfaceId);
}

int FaceIface::FaceIfacePriv::tagForName(const QString& name, int tagId, int parentId, const QString& givenFullName,
                                         bool convert, bool create) const
{
    if (name.isEmpty() && givenFullName.isEmpty() && !tagId)
    {
        return unknownPeopleTagId();
    }

    QString fullName = givenFullName.isNull() ? name : givenFullName;

    if (tagId)
    {
        if (q->isPerson(tagId))
        {
            //kDebug() << "Proposed tag is already a person";
            return tagId;
        }
        else if (convert)
        {
            if (fullName.isNull())
            {
                fullName = TagsCache::instance()->tagName(tagId);
            }

            kDebug() << "Converting proposed tag to person, full name" << fullName;
            makeFaceTag(tagId, fullName);
            return tagId;
        }

        return 0;
    }

    // First attempt: Find by full name in "person" attribute
    QList<int> candidates = TagsCache::instance()->tagsWithProperty(TagPropertyName::person(), fullName);
    foreach (int id, candidates)
    {
        kDebug() << "Candidate with set full name:" << id << fullName;

        if (parentId == -1)
        {
            return id;
        }
        else if (TagsCache::instance()->parentTag(id) == parentId)
        {
            return id;
        }
    }

    // Second attempt: Find by tag name
    if (parentId == -1)
    {
        candidates = TagsCache::instance()->tagsForName(name);
    }
    else
    {
        tagId = TagsCache::instance()->tagForName(name, parentId);
        candidates.clear();

        if (tagId)
        {
            candidates << tagId;
        }
    }

    foreach (int id, candidates)
    {
        // Is this tag already a person tag?
        if (q->isPerson(id))
        {
            kDebug() << "Found tag with name" << name << "is already a person." << id;
            return id;
        }
        else if (convert)
        {
            kDebug() << "Converting tag with name" << name << "to a person." << id;
            makeFaceTag(id, fullName);
            return id;
        }
    }

    // Third: If desired, create a new tag
    if (create)
    {
        kDebug() << "Creating new tag for name" << name << "fullName" << fullName;

        if (parentId == -1)
        {
            parentId = peopleTagId();
        }

        tagId = TagsCache::instance()->getOrCreateTag(tagPath(name, parentId));
        makeFaceTag(tagId, fullName);
        return tagId;
    }

    return 0;
}

// --- Constructor / Destructor -------------------------------------------------------------------------------------

FaceIface::FaceIface()
    : d( new FaceIfacePriv(this) )
{
}

FaceIface::~FaceIface()
{
    delete d;
}

// --- Mark for scanning and training -------------------------------------------------------------------------------

bool FaceIface::hasBeenScanned(qlonglong imageid) const
{
    return hasBeenScanned(ImageInfo(imageid));
}

bool FaceIface::hasBeenScanned(const ImageInfo& info) const
{
    return info.tagIds().contains(d->scannedForFacesTagId());
}

void FaceIface::markAsScanned(qlonglong imageid, bool hasBeenScanned) const
{
    return markAsScanned(ImageInfo(imageid), hasBeenScanned);
}

void FaceIface::markAsScanned(const ImageInfo& info, bool hasBeenScanned) const
{
    if (hasBeenScanned)
    {
        ImageInfo(info).setTag(d->scannedForFacesTagId());
    }
    else
    {
        ImageInfo(info).removeTag(d->scannedForFacesTagId());
    }
}

// --- Face tags ---------------------------------------------------------------------------------------------------

int FaceIface::tagForFaceName(const QString& kfaceId) const
{
    if (kfaceId.isNull())
    {
        return d->unknownPeopleTagId();
    }

    // Find in "kfaceId" attribute
    int tagId = d->findFirstTagWithProperty(TagPropertyName::kfaceId(), kfaceId);

    if (tagId)
    {
        return tagId;
    }

    // First find by full name or name. If not, id is in libface's database, but not in ours, so create.
    return getOrCreateTagForPerson(kfaceId);
}

QString FaceIface::faceNameForTag(int tagId) const
{
    if (!TagsCache::instance()->hasTag(tagId))
    {
        return QString();
    }

    QString id = TagsCache::instance()->propertyValue(tagId, TagPropertyName::kfaceId());

    if (id.isNull())
    {
        id = TagsCache::instance()->propertyValue(tagId, TagPropertyName::person());
    }

    if (id.isNull())
    {
        id = TagsCache::instance()->tagName(tagId);
    }

    return id;
}

int FaceIface::personParentTag() const
{
    return d->peopleTagId();
}

QList<int> FaceIface::allPersonTags() const
{
    return TagsCache::instance()->tagsWithProperty(TagPropertyName::person());
}

QList<QString> FaceIface::allPersonNames() const
{
    return TagsCache::instance()->tagNames(allPersonTags());
}

QList<QString> FaceIface::allPersonPaths() const
{
    return TagsCache::instance()->tagPaths(allPersonTags());
}

int FaceIface::tagForPerson(const QString& name, int parentId, const QString& fullName) const
{
    return d->tagForName(name, 0, parentId, fullName, false, false);
}

int FaceIface::getOrCreateTagForPerson(const QString& name, int parentId, const QString& fullName) const
{
    return d->tagForName(name, 0, parentId, fullName, true, true);
}

void FaceIface::ensureIsPerson(int tagId, const QString& fullName) const
{
    d->tagForName(QString(), tagId, 0, fullName, true, false);
}

bool FaceIface::isPerson(int tagId) const
{
    return TagsCache::instance()->hasProperty(tagId, TagPropertyName::person());
}

bool FaceIface::isTheUnknownPerson(int tagId) const
{
    return d->unknownPeopleTagId() == tagId;
}

// --- Read from database -----------------------------------------------------------------------------------

int FaceIface::faceCountForPersonInImage(qlonglong imageid, int tagId ) const
{
    ImageTagPair pair(imageid, tagId);
    return pair.values(ImageTagPropertyName::tagRegion()).size();
}

QList<KFaceIface::Face> FaceIface::facesFromTags(qlonglong imageId) const
{
    return toFaces(databaseFaces(imageId));
}

QList<KFaceIface::Face> FaceIface::unconfirmedFacesFromTags(qlonglong imageId) const
{
    return toFaces(unconfirmedDatabaseFaces(imageId));
}

QList<DatabaseFace> FaceIface::databaseFaces(qlonglong imageId) const
{
    return databaseFaces(imageId, DatabaseFace::NormalFaces);
}

QList<DatabaseFace> FaceIface::unconfirmedDatabaseFaces(qlonglong imageId) const
{
    return databaseFaces(imageId, DatabaseFace::UnconfirmedTypes);
}

QList<DatabaseFace> FaceIface::databaseFacesForTraining(qlonglong imageId) const
{
    return databaseFaces(imageId, DatabaseFace::FaceForTraining);
}

QList<DatabaseFace> FaceIface::confirmedDatabaseFaces(qlonglong imageId) const
{
    return databaseFaces(imageId, DatabaseFace::ConfirmedName);
}

QList<KFaceIface::Face> FaceIface::toFaces(const QList<DatabaseFace>& databaseFaces) const
{
    QList<KFaceIface::Face> faceList;
    foreach (const DatabaseFace& databaseFace, databaseFaces)
    {
        QRect rect = databaseFace.region().toRect();

        if (!rect.isValid())
        {
            continue;
        }

        KFaceIface::Face f;
        f.setRect(rect);

        if (databaseFace.tagId() != d->unknownPeopleTagId())
        {
            f.setName(faceNameForTag(databaseFace.tagId()));
        }

        faceList += f;
    }

    return faceList;
}

QList<DatabaseFace> FaceIface::databaseFaces(qlonglong imageid, DatabaseFace::TypeFlags flags) const
{
    QList<DatabaseFace> faces;
    QStringList attributes = DatabaseFace::attributesForFlags(flags);
    foreach (const ImageTagPair& pair, faceImageTagPairs(imageid, flags))
    {
        foreach (const QString& attribute, attributes)
        {
            foreach (const QString& regionString, pair.values(attribute))
            {
                TagRegion region(regionString);
                kDebug() << "rect found as "<< region << "for attribute" << attribute << "tag" << pair.tagId();

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

QList<ImageTagPair> FaceIface::faceImageTagPairs(qlonglong imageid, DatabaseFace::TypeFlags flags) const
{
    QList<ImageTagPair> pairs;

    QStringList attributes = DatabaseFace::attributesForFlags(flags);
    foreach (const ImageTagPair& pair, ImageTagPair::availablePairs(imageid))
    {
        //kDebug() << pair.tagId() << pair.properties();
        if (!isPerson(pair.tagId()))
        {
            continue;
        }

        // UnknownName and UnconfirmedName have the same attribute
        if (!(flags & DatabaseFace::UnknownName) && pair.tagId() == d->unknownPeopleTagId())
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

QList< QRect > FaceIface::getTagRects(qlonglong imageid) const
{
    QList< QRect > rectList;

    QList<ImageTagPair> pairs = ImageTagPair::availablePairs(imageid);
    foreach (const ImageTagPair& pair, pairs)
    {
        QStringList regions = pair.values(ImageTagPropertyName::tagRegion());
        foreach (const QString& region, regions)
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

int FaceIface::numberOfFaces(qlonglong imageid) const
{
    // Use case for this? Depending on a use case, we can think of an optimization
    int count = 0;

    QList<ImageTagPair> pairs = ImageTagPair::availablePairs(imageid);
    foreach (const ImageTagPair& pair, pairs)
    {
        QStringList regions = pair.values(ImageTagPropertyName::tagRegion());
        count += regions.size();
    }

    return count;
}

// --- Images in faces and thumbnails ---

void FaceIface::fillImageInFaces(const DImg& image, QList<KFaceIface::Face>& faceList, const QSize& scaleSize) const
{
    for (int i = 0; i < faceList.size(); i++)
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
    foreach (const KFaceIface::Face& face, faces)
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

    for (int i=0; i<faces.size(); i++)
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
    foreach (const DatabaseFace& face, databaseFaces)
    {
        QList<QRect> rects;
        rects << face.region().toRect();
        const int margin = faceRectDisplayMargin();
        rects << face.region().toRect().adjusted(-margin, -margin, margin, margin);

        foreach (const QRect& rect, rects)
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
    QList<DatabaseFace> newFaces;

    // Build list of new entries
    foreach (const KFaceIface::Face& face, faceList)
    {
        // We'll get the unknownPeopleTagId if face.name() is null
        int tagId          = tagForFaceName(face.name());
        QRect fullSizeRect = TagRegion::mapToOriginalSize(image, face.toRect());

        if (!tagId || !fullSizeRect.isValid())
        {
            newFaces << DatabaseFace();
            continue;
        }

        kDebug() << "New Entry" << fullSizeRect << tagId;
        newFaces << DatabaseFace(DatabaseFace::UnconfirmedName, imageid, tagId, TagRegion(fullSizeRect));
    }

    if (newFaces.isEmpty())
    {
        return newFaces;
    }

    // list of existing entries
    QList<DatabaseFace> currentFaces = databaseFaces(imageid);

    // merge new with existing entries
    for (int i=0; i<newFaces.size(); i++)
    {
        DatabaseFace& newFace = newFaces[i];

        QList<DatabaseFace> overlappingEntries;
        foreach (const DatabaseFace& oldFace, currentFaces)
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
                for (int i=0; i<overlappingEntries.size(); i++)
                {
                    DatabaseFace& oldFace = overlappingEntries[i];

                    if (oldFace.isUnknownName())
                    {
                        // remove old face
                    }
                    else
                    {
                        // skip new entry if any overlapping face has a name, and we dont
                        newFace = DatabaseFace();
                        break;
                    }
                }
            }
            else
            {
                // we have a name in the new face. Do we have names in overlapping faces?
                for (int i=0; i<overlappingEntries.size(); i++)
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
            d->add(pair, newFace, DatabaseFace::attributesForFlags(DatabaseFace::UnconfirmedName), false);
        }
    }

    return newFaces;
}

DatabaseFace FaceIface::addUnknownManually(const DImg& image, qlonglong imageid, const QRect& rect)
{
    DatabaseFace newFace;

    int tagId          = d->unknownPeopleTagId();
    QRect fullSizeRect = TagRegion::mapToOriginalSize(image, rect);

    if (!tagId || !fullSizeRect.isValid())
    {
        return newFace;
    }

    kDebug() << "New Entry" << fullSizeRect << tagId;
    newFace = DatabaseFace(DatabaseFace::UnconfirmedName, imageid, tagId, TagRegion(fullSizeRect));


    ImageTagPair pair(imageid, newFace.tagId());
    // UnconfirmedName and UnknownName have the same attribute
    d->add(pair, newFace, DatabaseFace::attributesForFlags(DatabaseFace::UnconfirmedName), false);

    return newFace;
}

// --- Confirming and adding ---

DatabaseFace FaceIface::confirmedEntry(const DatabaseFace& face, int tagId, const TagRegion& confirmedRegion)
{
    return DatabaseFace(DatabaseFace::ConfirmedName, face.imageId(),
                        tagId == -1 ? face.tagId() : tagId,
                        confirmedRegion.isValid() ? confirmedRegion : face.region());
}

DatabaseFace FaceIface::confirmName(const DatabaseFace& face, int tagId, const TagRegion& confirmedRegion)
{
    DatabaseFace newEntry = confirmedEntry(face, tagId, confirmedRegion);

    if (newEntry.tagId() == d->unknownPeopleTagId())
    {
        kDebug() << "Refusing to confirm unknownPerson tag on face";
        return face;
    }

    ImageTagPair pair(newEntry.imageId(), newEntry.tagId());

    // Remove entry from autodetection
    if (newEntry.tagId() == face.tagId())
    {
        d->remove(pair, face, false);
    }
    else
    {
        ImageTagPair pairOldEntry(face.imageId(), face.tagId());
        d->remove(pairOldEntry, face, true);
    }

    // Add new full entry
    d->add(pair, newEntry,
           DatabaseFace::attributesForFlags(DatabaseFace::ConfirmedName | DatabaseFace::FaceForTraining),
           true);

    return newEntry;
}

DatabaseFace FaceIface::add(qlonglong imageId, int tagId, const TagRegion& region, bool trainFace)
{
    DatabaseFace newEntry(DatabaseFace::ConfirmedName, imageId, tagId, region);
    add(newEntry, trainFace);
    return newEntry;
}

void FaceIface::add(const DatabaseFace& face, bool trainFace)
{
    ImageTagPair pair(face.imageId(), face.tagId());
    DatabaseFace::TypeFlags flags = DatabaseFace::ConfirmedName;

    if (trainFace)
    {
        flags |= DatabaseFace::FaceForTraining;
    }

    d->add(pair, face, DatabaseFace::attributesForFlags(flags), true);
}

void FaceIface::FaceIfacePriv::add(ImageTagPair& pair, const DatabaseFace& face,
                                   const QStringList& properties, bool addTag)
{
    q->ensureIsPerson(face.tagId());

    QString region = face.region().toXml();
    foreach (const QString& property, properties)
    {
        pair.addProperty(property, region);
    }

    if (addTag)
    {
        MetadataManager::instance()->assignTag(ImageInfo(face.imageId()), face.tagId());
    }
}

// --- Removing faces ---

void FaceIface::removeAllFaces(qlonglong imageid)
{
    ImageInfo info(imageid);

    //markAsScanned(info, false);

    QList<int> tagsToRemove;
    QStringList attributes = DatabaseFace::attributesForFlags(DatabaseFace::AllTypes);
    foreach (ImageTagPair pair, faceImageTagPairs(imageid, DatabaseFace::AllTypes))
    {
        foreach (const QString& attribute, attributes)
        {
            pair.removeProperties(attribute);
        }

        if (pair.isAssigned())
        {
            tagsToRemove << pair.tagId();
        }
    }

    MetadataManager::instance()->removeTags(info, tagsToRemove);
}

void FaceIface::removeFace(qlonglong imageid, const QRect& rect)
{
    ImageInfo info(imageid);

    QList<int> tagsToRemove;
    QStringList attributes    = DatabaseFace::attributesForFlags(DatabaseFace::AllTypes);
    QList<ImageTagPair> pairs = faceImageTagPairs(imageid, DatabaseFace::AllTypes);

    for (int i=0; i<pairs.size(); i++)
    {
        ImageTagPair& pair = pairs[i];
        foreach (const QString& attribute, attributes)
        {
            foreach (const QString& regionString, pair.values(attribute))
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

    MetadataManager::instance()->removeTags(ImageInfo(imageid), tagsToRemove);
}

void FaceIface::removeFace(const DatabaseFace& face)
{
    if (face.isNull())
    {
        return;
    }

    ImageTagPair pair(face.imageId(), face.tagId());
    d->remove(pair, face, true);
}

void FaceIface::removeFaces(const QList<DatabaseFace>& faces)
{
    foreach (const DatabaseFace& face, faces)
    {
        if (face.isNull())
        {
            continue;
        }

        ImageTagPair pair(face.imageId(), face.tagId());
        d->remove(pair, face, true);
    }
}

void FaceIface::FaceIfacePriv::remove(ImageTagPair& pair, const DatabaseFace& face, bool touchTags)
{
    QString regionString = TagRegion(face.region().toRect()).toXml();
    pair.removeProperty(DatabaseFace::attributeForType(face.type()), regionString);

    if (face.type() == DatabaseFace::ConfirmedName)
    {
        pair.removeProperty(DatabaseFace::attributeForType(DatabaseFace::FaceForTraining), regionString);
    }

    // Tag assigned and no other entry left?
    if (touchTags
        && pair.isAssigned()
        && !pair.hasProperty(DatabaseFace::attributeForType(DatabaseFace::ConfirmedName)))
    {
        MetadataManager::instance()->removeTag(ImageInfo(face.imageId()), pair.tagId());
    }
}

// --- Utilities ---

int FaceIface::faceRectDisplayMargin()
{
    /*
     * Dont change that value unless you know what you do.
     * There are a lot of pregenerated thumbnails in user's databases,
     * expensive to regenerate, depending on this very value.
     */
    return 70;
}


/*
QList< Face > FaceIface::findAndTagFaces(const DImg& image, qlonglong imageid, FaceRecognitionSteps todo)
{
    readConfigSettings(); //FIXME: do by signal

    kDebug() << "Image" << image.attribute("originalFilePath") << "dimensions" << image.size() << "original size" << image.originalSize();
    KFaceIface::Image fimg(image.width(), image.height(), image.sixteenBit(), image.hasAlpha(), image.bits());

    // -- Detection --

    QList<KFaceIface::Face> faceList = d->database()->detectFaces(fimg);
    // Mark the image as scanned.
    markAsScanned(imageid);

    // -- Recognition --

    QList<double> distances;
    if (d->database()->peopleCount() && todo >= DetectAndRecognize)
    {
        distances = d->database()->recognizeFaces(faceList);
        for (int i=0; i<faceList.size(); i++)
        {
            if (distances[i] > 100)
            {
                // Dismissing
                faceList[i].clearRecognition();
            }
        }
    }

    // -- Apply tags --

    writeUnconfirmedResults(image, imageid, faceList);

    return faceList;
}
*/
/*
QString FaceIface::recognizeFace(const KFaceIface::Face& face)
{
    if(d->database()->peopleCount() == 0)
        return QString();

    QList<Face> f;
    f.append(face);

    if( face.image().isNull() )
    {
        return QString();
    }
    else
    {
        double distance = d->database()->recognizeFaces(f).at(0)/100000;
        kDebug() << "Possible : " << f[0].name();
        kDebug() << "Distance is " << distance;

        if(distance > 100)
        {
            kDebug() << "Dismissing ";
            return QString();
        }
        else
        {
            return f[0].name();
        }
    }
}
*/
/*
DatabaseFace FaceIface::confirmName( qlonglong imageid, const QString& name, const QRect& rect, const QRect& previousRect)
{
    int nameTagId = getOrCreateTagForPerson(name);
    return confirmName(imageid, nameTagId, rect, previousRect);
}

DatabaseFace FaceIface::confirmName(qlonglong imageid, int tagId, const QRect& rect, const QRect& previousRect)
{
    ensureIsPerson(tagId);

    QString region         = TagRegion(rect).toXml();
    QString regionToRemove = previousRect.isNull() ? region : TagRegion(previousRect).toXml();

    ImageTagPair pairUnknown ( imageid, d->unknownPeopleTagId());
    ImageTagPair pairNamed   ( imageid, tagId );

    pairUnknown.removeProperty(ImageTagPropertyName::autodetectedFace(), regionToRemove);

    pairNamed.setProperty(ImageTagPropertyName::tagRegion(), region);
    pairNamed.setProperty(ImageTagPropertyName::faceToTrain(), region);

    MetadataManager::instance()->assignTag(ImageInfo(imageid), tagId);

    return DatabaseFace(DatabaseFace::ConfirmedName, imageid, tagId, rect);
}
*/
// --- Training ---------------------------------------------------------------------------------------------------

/*
void FaceIface::trainImages(const QList<ImageInfo>& imageInfos)
{
    ThumbnailImageCatcher catcher(d->thumbnailLoadThread);

    typedef QHash<ImageInfo, QList<DatabaseFace> > ImageInfoToDatabaseFaceHash;
    ImageInfoToDatabaseFaceHash                    regionsTrained;
    QList<Face>                                    facesToTrain;

    foreach (const ImageInfo& info, imageInfos)
    {
        QString filePath = info.filePath();
        if (filePath.isNull())
            continue;

        QList<DatabaseFace> regions = databaseFaces(info.id(), DatabaseFace::FaceForTraining);
        QList<Face> faces           = toFaces(regions);

        for (int i=0; i<faces.size(); ++i)
        {
            Face &face   = faces[i];
            d->thumbnailLoadThread->find(filePath, face.toRect(), ThumbnailLoadThread::maximumThumbnailSize());
            QImage image = catcher.waitForThumbnail(filePath);
            face.setImage(image);
            facesToTrain << face;
            regionsTrained[info] << regions[i];
        }
    }

    trainFaces(facesToTrain);

    // Ok, what's that? We have a list of images. For each image, we have a list of face
    // regions, each with a tag id and a region. For each region (with associated image id and tag id)
    // we remove the faceToTrain() image tag property.
    // Then, per-image, we remove the global tag marking it for training.
    // As we loaded all regions above, we dont need to check that none is left.
    ImageInfoToDatabaseFaceHash::iterator it;
    for (it = regionsTrained.begin(); it != regionsTrained.end(); ++it)
    {
        foreach (const DatabaseFace& regionsTrained, it.value())
        {
            // possible optimization: only one pair per tag id
            ImageTagPair pair(it.key().id(), regionsTrained.tagId());
            pair.removeProperty(ImageTagPropertyName::faceToTrain(), TagRegion(regionsTrained.region().toRect()).toXml());
        }
    }
}

void FaceIface::trainFace(const Face& face)
{
    trainFaces(QList<Face>() << face);
}

void FaceIface::trainFaces(const QList<Face>& givenFaceList )
{
    readConfigSettings(); //TODO: do by signal

    QList<Face> faceList = givenFaceList;
    for (int i = 0; i < faceList.size(); i++)
    {
        const Face& face = faceList[i];
        if (face.name().isEmpty() || face.image().isNull())
            faceList.removeAt(i);
    }

    d->database()->updateFaces(faceList);
    kDebug() << "DB file is : " << d->database()->configPath();
    d->database()->saveConfig();
}

void FaceIface::readConfigSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Face Tags Settings");

    d->suggestionsAllowed     = group.readEntry("FaceSuggestion", false);
    d->detectionAccuracy      = group.readEntry("DetectionAccuracy", 3);
    d->recognitionThreshold   = 1 - group.readEntry("SuggestionThreshold", 0.2);

    / *if (d->databaseConst())
        d->database()->setDetectionAccuracy(d->detectionAccuracy);
        * /
}
*/

/* *
 * Returns a list of image ids of all images in the DB which have a specified person within.
 * @param tagId The person's id. Or tag id for the person tag. Same thing.
 * @param repeat If repeat is specified as true, then if person with id tagId is found n times in an image,
 * that image's id will be pushed into the returned list n times. Useful for thumbnailing as I see it.
 */
//QList<qlonglong>    imagesWithPerson(int tagId, bool repeat = false);

/*
What is the use case? This method will be extremely slow.
For listing, we must use a search.
QList< qlonglong > FaceIface::imagesWithPerson(int tagId, bool repeat)
{
    QList<qlonglong> imageIds;
    const AlbumList palbumList = AlbumManager::instance()->allPAlbums();

    // Get all digiKam albums collection pictures path, depending of d->rebuildAll flag.

    QStringList pathList;

    for (AlbumList::ConstIterator it = palbumList.constBegin();
            it != palbumList.constEnd(); ++it)
    {
        pathList += DatabaseAccess().db()->getItemURLsInAlbum((*it)->id());
    }

    for (QStringList::ConstIterator i = pathList.constBegin();
            i != pathList.constEnd(); ++i)
    {
        ImageInfo info(*i);

        if (this->hasBeenScanned(info.id()))
        {
            kDebug() << "Image " << info.filePath() << "has already been scanned.";

            //dddddd
            DImg img;
            QList<Face> faceList = findFacesFromTags(img, info.id());

            QString nameString = TagsCache::instance()->tagName(tagId);
            if(nameString == QString("Unknown"))
                nameString = QString();    // Because we store names for unknown faces this way in the Face objects.

            QListIterator<Face> faceIt(faceList);
            // The number of times the same face was found in this image
            int count = 0;

            while(faceIt.hasNext())
            {
                if(faceIt.next().name() == nameString)
                {
                    count++;
                    // If we have been told to repeat the image n times if the face was found n times, then do so.
                    if(repeat)
                        imageIds += info.id();
                    // If we have not been told to repeat, then only append the id if the count is 1
                    else
                    {
                        if(count == 1)
                            imageIds += info.id();
                    }
                }
            }
        }
    }

    return imageIds;
}
*/


/**
 * Method to mark a face as trained, so that we don't retrain it
 */
//     void                markFaceAsTrained(qlonglong imageid, const QRect& rect, const QString& name);
//     void                markFaceAsTrained(qlonglong imageid, const Face& face);

/**
 * Method to mark a list of faces as trained, so that we don't retrain them
 */
//     void                markFacesAsTrained(qlonglong imageid, QList<Face> faces);

/**
 * Method to mark a face as recognized, so we don't attempt to recognize it again
 */
//     void                markFaceAsRecognized(qlonglong imageid, const QRect& rect, const QString& name);
//     void                markFaceAsRecognized(qlonglong imageid, const Face& face);

/**
 * Method to mark a list of faces as recognized, so that we don't attempt to identify them again
 */
//     void                markFacesAsRecognized(qlonglong imageid, QList<Face> faces);

/**
 * Tells if the face has been marked as Trained
 */
//     bool                isFaceTrained(qlonglong imageid, const QRect& rect, const QString& name);

/**
 * Tells if the face has been marked as recognized
 */
//     bool                isFaceRecognized( qlonglong imageid, const QRect& rect, const QString& name);
/*
void FaceIface::markFaceAsTrained ( qlonglong imageid, const QRect& rect, const QString& name )
{
    if(name.isNull() || name == "Unknown")
        return;

    int nameTagId = tagForPerson ( name );
    ImageTagPair pair ( imageid, nameTagId );
    pair.addProperty("trained", TagRegion(rect).toXml());
    pair.assignTag();

    kDebug() << "Marked face of " << name << " as trained";
}

void FaceIface::markFaceAsTrained ( qlonglong imageid, const KFaceIface::Face& face )
{
    this->markFaceAsTrained(imageid, face.toRect(), face.name());
}

void FaceIface::markFacesAsTrained ( qlonglong imageid, QList< Face > faces )
{
    foreach(Face f, faces)
    {
        this->markFaceAsTrained(imageid, f.toRect(), f.name());
    }
}

void FaceIface::markFaceAsRecognized ( qlonglong imageid, const QRect& rect, const QString& name )
{
    if(name == QString("Unknown") || name == QString())
        return;

    int nameTagId = tagForPerson ( name );
    ImageTagPair pair ( imageid, nameTagId );
    pair.addProperty("recognized", TagRegion(rect).toXml());
    pair.assignTag();
    kDebug() << "Marked face of " << name << " as recognized";
}

void FaceIface::markFaceAsRecognized ( qlonglong imageid, const KFaceIface::Face& face )
{
    this->markFaceAsRecognized(imageid, face.toRect(), face.name());
}

void FaceIface::markFacesAsRecognized ( qlonglong imageid, QList< Face > faces )
{
    foreach(Face f, faces)
    {
        this->markFaceAsRecognized(imageid, f.toRect(), f.name());
    }
}

bool FaceIface::isFaceTrained ( qlonglong imageid, const QRect& rect, const QString& name )
{
    int nameTagId = tagForPerson ( name );
    ImageTagPair pair ( imageid, nameTagId );

    if(pair.values("trained").contains(TagRegion(rect).toXml()))
    {
        if(name == QString("Unknown") || name == QString())
        {
            DatabaseAccess().db()->removeImageTagProperties(imageid, nameTagId, "trained", TagRegion(rect).toXml());
            return false;
        }
        else
        {
            return true;
        }
    }

    return false;
}

bool FaceIface::isFaceRecognized ( qlonglong imageid, const QRect& rect, const QString& name )
{
    int nameTagId = tagForPerson ( name );
    ImageTagPair pair ( imageid, nameTagId );

    if(pair.values("recognized").contains(TagRegion(rect).toXml()))
    {
        if(name == QString("Unknown") || name == QString())
        {
            DatabaseAccess().db()->removeImageTagProperties(imageid, nameTagId, "recognized", TagRegion(rect).toXml());
            return false;
        }
        else
        {
            return true;
        }
    }

    return false;
}
*/

} // Namespace Digikam
