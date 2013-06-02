/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-08-08
 * Description : Accessing face tags
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

#include "facetags.h"

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// Local includes

#include "databaseconstants.h"
#include "tagscache.h"
#include "tagregion.h"
#include "tagproperties.h"

namespace Digikam
{

// --- FaceIfacePriv ----------------------------------------------------------------------------------------

class FaceTagsHelper
{
public:

    static QString tagPath(const QString& name, int parentId);
    static void    makeFaceTag(int tagId, const QString& fullName);
    static int     findFirstTagWithProperty(const QString& property, const QString& value = QString());
    static int     tagForName(const QString& name, int tagId, int parentId,
                              const QString& givenFullName, bool convert, bool create);
};

// --- Private methods ---

int FaceTagsHelper::findFirstTagWithProperty(const QString& property, const QString& value)
{
    QList<int> candidates = TagsCache::instance()->tagsWithProperty(property, value);

    if (!candidates.isEmpty())
    {
        return candidates.first();
    }

    return 0;
}

QString FaceTagsHelper::tagPath(const QString& name, int parentId)
{
    QString faceParentTagName = TagsCache::instance()->tagName(parentId);
    return faceParentTagName + '/' + name;
}

void FaceTagsHelper::makeFaceTag(int tagId, const QString& fullName)
{
    QString kfaceName  = fullName;
    /*
     *    // find a unique kfaceId
     *    for (int i=0; d->findFirstTagWithProperty(TagPropertyName::kfaceId(), kfaceId); ++i)
     *        kfaceId = fullName + QString(" (%1)").arg(i);
     */
    TagProperties props(tagId);
    props.setProperty(TagPropertyName::person(), fullName);
    props.setProperty(TagPropertyName::kfaceName(), kfaceName);
}

int FaceTagsHelper::tagForName(const QString& name, int tagId, int parentId, const QString& givenFullName,
                               bool convert, bool create)
{
    if (name.isEmpty() && givenFullName.isEmpty() && !tagId)
    {
        return FaceTags::unknownPersonTagId();
    }

    QString fullName = givenFullName.isNull() ? name : givenFullName;

    if (tagId)
    {
        if (FaceTags::isPerson(tagId))
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

    foreach(int id, candidates)
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

    foreach(int id, candidates)
    {
        // Is this tag already a person tag?
        if (FaceTags::isPerson(id))
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
            parentId = FaceTags::personParentTag();
        }

        tagId = TagsCache::instance()->getOrCreateTag(tagPath(name, parentId));
        makeFaceTag(tagId, fullName);
        return tagId;
    }

    return 0;
}

// --- public methods ---

QList<QString> FaceTags::allPersonNames()
{
    return TagsCache::instance()->tagNames(allPersonTags());
}

QList<QString> FaceTags::allPersonPaths()
{
    return TagsCache::instance()->tagPaths(allPersonTags());
}

int FaceTags::tagForPerson(const QString& name, int parentId, const QString& fullName)
{
    return FaceTagsHelper::tagForName(name, 0, parentId, fullName, false, false);
}

int FaceTags::getOrCreateTagForPerson(const QString& name, int parentId, const QString& fullName)
{
    return FaceTagsHelper::tagForName(name, 0, parentId, fullName, true, true);
}

void FaceTags::ensureIsPerson(int tagId, const QString& fullName)
{
    FaceTagsHelper::tagForName(QString(), tagId, 0, fullName, true, false);
}

bool FaceTags::isPerson(int tagId)
{
    return TagsCache::instance()->hasProperty(tagId, TagPropertyName::person());
}

bool FaceTags::isTheUnknownPerson(int tagId)
{
    return TagsCache::instance()->hasProperty(tagId, TagPropertyName::unknownPerson());
}

QList<int> FaceTags::allPersonTags()
{
    return TagsCache::instance()->tagsWithProperty(TagPropertyName::person());
}

int FaceTags::scannedForFacesTagId()
{
    return TagsCache::instance()->getOrCreateInternalTag(InternalTagName::scannedForFaces()); // no i18n
}

QMap<QString, QString> FaceTags::identityAttributes(int tagId)
{
    QMap<QString, QString> attributes;
    QString uuid = TagsCache::instance()->propertyValue(tagId, TagPropertyName::kfaceUuid());
    if (!uuid.isEmpty())
    {
        attributes["uuid"] = uuid;
    }

    QString fullName = TagsCache::instance()->propertyValue(tagId, TagPropertyName::person());
    if (!fullName.isEmpty())
    {
        attributes["fullName"] = fullName;
    }

    QString kfaceName = TagsCache::instance()->propertyValue(tagId, TagPropertyName::person());
    QString tagName = TagsCache::instance()->tagName(tagId);
    if (tagName != kfaceName)
    {
        attributes.insertMulti("name", kfaceName);
        attributes.insertMulti("name", tagName);
    }
    else
    {
        attributes["name"] = tagName;
    }

    return attributes;
}

void FaceTags::applyTagIdentityMapping(int tagId, const QMap<QString, QString>& attributes)
{
    TagProperties props(tagId);

    if (attributes.contains("fullName"))
    {
        props.setProperty(TagPropertyName::person(), attributes.value("fullName"));
    }

    // we do not change the digikam tag name at this point, but we have this extra tag property
    if (attributes.contains("name"))
    {
        props.setProperty(TagPropertyName::kfaceName(), attributes.value("name"));
    }

    props.setProperty(TagPropertyName::kfaceUuid(), attributes.value("uuid"));
}

int FaceTags::getOrCreateTagForIdentity(const QMap<QString, QString>& attributes)
{
    // Attributes from libkface's Identity object.
    // The text constants are defines in libkface's API docs
    if (attributes.isEmpty())
    {
        return FaceTags::unknownPersonTagId();
    }

    int tagId;

    // First, look for UUID
    if (!attributes.value("uuid").isEmpty())
    {
        if ( (tagId = FaceTagsHelper::findFirstTagWithProperty(TagPropertyName::kfaceUuid(), attributes.value("uuid"))) )
        {
            return tagId;
        }
    }

    // Second, look for full name
    if (!attributes.value("fullName").isEmpty())
    {
        if ( (tagId = FaceTagsHelper::findFirstTagWithProperty(TagPropertyName::person(), attributes.value("fullName"))) )
        {
            return tagId;
        }
    }

    // Third, look for either name or full name
    // TODO: better support for "fullName"
    QString name = attributes.value("name");
    if (name.isEmpty())
    {
        name = attributes.value("fullName");
    }
    if (name.isEmpty())
    {
        return FaceTags::unknownPersonTagId();
    }

    if ( (tagId = FaceTagsHelper::findFirstTagWithProperty(TagPropertyName::kfaceName(), name)) )
    {
        return tagId;
    }
    if ( (tagId = FaceTagsHelper::findFirstTagWithProperty(TagPropertyName::person(), name)) )
    {
        return tagId;
    }

    // identity is in libkface's database, but not in ours, so create.
    tagId = FaceTagsHelper::tagForName(name, 0, -1, attributes.value("fullName"), true, true);
    applyTagIdentityMapping(tagId, attributes);
    return tagId;
}

QString FaceTags::faceNameForTag(int tagId)
{
    if (!TagsCache::instance()->hasTag(tagId))
    {
        return QString();
    }

    QString id = TagsCache::instance()->propertyValue(tagId, TagPropertyName::person());

    if (id.isNull())
    {
        id = TagsCache::instance()->tagName(tagId);
    }

    return id;
}

int FaceTags::personParentTag()
{
    // check default
    QString i18nName = i18nc("People on your photos", "People");
    int tagId        = TagsCache::instance()->tagForPath(i18nName);

    if (tagId)
    {
        return tagId;
    }

    // employ a heuristic
    QList<int> personTags = allPersonTags();

    if (!personTags.isEmpty())
    {
        // we find the most toplevel parent tag of a person tag
        QMultiMap<int,int> tiers;
        foreach(int tagId, personTags)
        {
            tiers.insert(TagsCache::instance()->parentTags(tagId).size(), tagId);
        }

        QList<int> mosttoplevelTags = tiers.values(tiers.begin().key());

        // as a pretty weak criterion, take the largest id which usually corresponds to the latest tag creation.
        qSort(mosttoplevelTags);
        return TagsCache::instance()->parentTag(mosttoplevelTags.last());
    }

    // create default
    return TagsCache::instance()->getOrCreateTag(i18nName);
}

int FaceTags::unknownPersonTagId()
{
    QList<int> ids = TagsCache::instance()->tagsWithPropertyCached(TagPropertyName::unknownPerson());

    if (!ids.isEmpty())
    {
        return ids.first();
    }

    int unknownPersonTagId = TagsCache::instance()->getOrCreateTag(
                                        FaceTagsHelper::tagPath(
                                        i18nc("The list of detected faces from the collections but not recognized", "Unknown"),
                                        personParentTag()));
    TagProperties props(unknownPersonTagId);
    props.setProperty(TagPropertyName::person(), QString());        // no name associated
    props.setProperty(TagPropertyName::unknownPerson(), QString()); // special property

    return unknownPersonTagId;
}

} // Namespace Digikam
