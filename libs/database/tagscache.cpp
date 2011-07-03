/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-02
 * Description : Cache for Tag information
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "tagscache.moc"

// Qt includes

#include <QMultiHash>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QMap>

// KDE includes

#include <kdebug.h>
#include <kglobal.h>

// Local includes

#include "albumdb.h"
#include "databaseaccess.h"
#include "databasewatch.h"
#include "imagepropertiestab.h"
#include "tagproperties.h"

namespace Digikam
{

static bool lessThanForTagShortInfo(const TagShortInfo& first, const TagShortInfo& second)
{
    return first.id < second.id;
}

static bool lessThanForTagProperty(const TagProperty& first, const TagProperty& second)
{
    return first.tagId < second.tagId;
}
typedef QList<TagProperty>::const_iterator TagPropertiesConstIterator;
typedef QPair<TagPropertiesConstIterator, TagPropertiesConstIterator> TagPropertiesRange;

// ------------------------------------------------------------------------------------------

class TagsCache::TagsCachePriv
{
public:

    TagsCachePriv(TagsCache* q) :
        initialized(false),
        needUpdateInfos(true),
        needUpdateHash(true),
        needUpdateProperties(true),
        needUpdateLabelTags(true),
        changingDB(false),
        q(q)
    {
    }

    bool                        initialized;
    bool                        needUpdateInfos;
    bool                        needUpdateHash;
    bool                        needUpdateProperties;
    bool                        needUpdateLabelTags;
    bool                        changingDB;

    QReadWriteLock              lock;
    QList<TagShortInfo>         infos;
    QMultiHash<QString, int>    nameHash;

    QList<TagProperty>          tagProperties;
    QHash<QString, QList<int> > tagsWithProperty;
    QSet<int>                   internalTags;
    QMap<ColorLabel, int>       colorLabelsTags;              // Map between color Id and tag label Id created in DB.
    QMap<PickLabel, int>        pickLabelsTags;               // Map between pick Id and tag label Id created in DB.

    TagsCache* const            q;

    void checkInfos()
    {
        if (needUpdateInfos && initialized)
        {
            QList<TagShortInfo> newInfos = DatabaseAccess().db()->getTagShortInfos();
            QWriteLocker locker(&lock);
            infos           = newInfos;
            needUpdateInfos = false;
        }
    }

    void checkNameHash()
    {
        checkInfos();

        if (needUpdateHash && initialized)
        {
            QWriteLocker locker(&lock);
            nameHash.clear();
            foreach (const TagShortInfo& info, infos)
            {
                nameHash.insert(info.name, info.id);
            }
            needUpdateHash = false;
        }
    }

    void checkProperties()
    {
        if (needUpdateProperties && initialized)
        {
            QList<TagProperty> props = DatabaseAccess().db()->getTagProperties();

            // Ensure not to lock both locks at the same time
            QWriteLocker locker(&lock);

            needUpdateProperties = false;
            tagProperties = props;
            tagsWithProperty.clear();

            QLatin1String internalProp = TagsCache::propertyNameDigikamInternalTag();
            foreach (const TagProperty& property, tagProperties)
            {
                if (property.property == internalProp)
                {
                    internalTags << property.tagId;
                }
            }
        }
    }

    // remember to call under lock
    QList<TagShortInfo>::const_iterator find(int id) const
    {
        TagShortInfo info;
        info.id = id;

        // we use the fact that d->infos is sorted by id
        return qBinaryFind(infos.constBegin(), infos.constEnd(), info, lessThanForTagShortInfo);
    }

    TagPropertiesRange findProperties(int id) const
    {
        TagProperty prop;
        prop.tagId = id;
        TagPropertiesRange range;
        range.first = qLowerBound(tagProperties.begin(), tagProperties.end(), prop, lessThanForTagProperty);
        range.second = qUpperBound(range.first, tagProperties.end(), prop, lessThanForTagProperty);
        return range;
    }

    inline TagPropertiesConstIterator toNextTag(TagPropertiesConstIterator it) const
    {
        // increment iterator until the next tagid is reached
        int currentId = it->tagId;

        for (++it; it != tagProperties.end(); ++it)
            if (it->tagId != currentId)
            {
                break;
            }

        return it;
    }

    inline bool compareProperty(const TagPropertiesConstIterator& it,  const QString& property, const QString& value)
    {
        if (value.isNull())
        {
            return it->property == property;
        }
        else
        {
            return it->property == property && it->value == value;
        }
    }

    template <typename T>
    inline bool sortedListContains(const QList<T>& list, const T& value)
    {
        return qBinaryFind(list, value) != list.end();
    }

    void checkLabelTags()
    {
        if (needUpdateLabelTags && initialized)
        {
            QMap<ColorLabel, int> map;
            map.insert(NoColorLabel, q->getOrCreateInternalTag(InternalTagName::colorLabelNone()));
            map.insert(RedLabel,     q->getOrCreateInternalTag(InternalTagName::colorLabelRed()));
            map.insert(OrangeLabel,  q->getOrCreateInternalTag(InternalTagName::colorLabelOrange()));
            map.insert(YellowLabel,  q->getOrCreateInternalTag(InternalTagName::colorLabelYellow()));
            map.insert(GreenLabel,   q->getOrCreateInternalTag(InternalTagName::colorLabelGreen()));
            map.insert(BlueLabel,    q->getOrCreateInternalTag(InternalTagName::colorLabelBlue()));
            map.insert(MagentaLabel, q->getOrCreateInternalTag(InternalTagName::colorLabelMagenta()));
            map.insert(GrayLabel,    q->getOrCreateInternalTag(InternalTagName::colorLabelGray()));
            map.insert(BlackLabel,   q->getOrCreateInternalTag(InternalTagName::colorLabelBlack()));
            map.insert(WhiteLabel,   q->getOrCreateInternalTag(InternalTagName::colorLabelWhite()));

            QMap<PickLabel, int> map2;
            map2.insert(NoPickLabel,   q->getOrCreateInternalTag(InternalTagName::pickLabelNone()));
            map2.insert(RejectedLabel, q->getOrCreateInternalTag(InternalTagName::pickLabelRejected()));
            map2.insert(PendingLabel,  q->getOrCreateInternalTag(InternalTagName::pickLabelPending()));
            map2.insert(AcceptedLabel, q->getOrCreateInternalTag(InternalTagName::pickLabelAccepted()));

            QWriteLocker locker(&lock);
            needUpdateLabelTags = false;
            colorLabelsTags     = map;
            pickLabelsTags      = map2;
        }
    }
};

// ------------------------------------------------------------------------------------------

class ChangingDB
{
public:

    ChangingDB(TagsCache::TagsCachePriv* d)
        : d(d)
    {
        d->changingDB = true;
    }
    ~ChangingDB()
    {
        d->changingDB = false;
    }

    TagsCache::TagsCachePriv* const d;
};

// ------------------------------------------------------------------------------------------

class TagsCacheCreator
{
public:
    TagsCache object;
};
K_GLOBAL_STATIC(TagsCacheCreator, creator)

// ------------------------------------------------------------------------------------------

TagsCache* TagsCache::instance()
{
    return &creator->object;
}

TagsCache::TagsCache()
    : d(new TagsCachePriv(this))
{
}

TagsCache::~TagsCache()
{
    delete d;
}

void TagsCache::initialize()
{
    if (d->initialized)
    {
        return;
    }

    connect(DatabaseAccess::databaseWatch(), SIGNAL(tagChange(const TagChangeset&)),
            this, SLOT(slotTagChanged(const TagChangeset&)),
            Qt::DirectConnection);

    d->initialized = true;
}

void TagsCache::invalidate()
{
    d->needUpdateInfos          = true;
    d->needUpdateHash           = true;
    d->needUpdateProperties     = true;
    d->needUpdateLabelTags = true;
}

QLatin1String TagsCache::tagPathOfDigikamInternalTags(LeadingSlashPolicy slashPolicy)
{
    if (slashPolicy == IncludeLeadingSlash)
    {
        return QLatin1String("/_Digikam_Internal_Tags_");
    }
    else
    {
        return QLatin1String("_Digikam_Internal_Tags_");
    }
}

QLatin1String TagsCache::propertyNameDigikamInternalTag()
{
    // Do not change, is written to users' databases
    return QLatin1String("internalTag");
}

QLatin1String TagsCache::propertyNameExcludedFromWriting()
{
    // Do not change, is written to users' databases
    return QLatin1String("noMetadataTag");
}

QString TagsCache::tagName(int id) const
{
    d->checkInfos();

    QReadLocker locker(&d->lock);
    QList<TagShortInfo>::const_iterator it = d->find(id);

    if (it != d->infos.constEnd())
    {
        return it->name;
    }

    return QString();
}

QStringList TagsCache::tagNames(const QList<int>& ids, HiddenTagsPolicy hiddenTagsPolicy) const
{
    QStringList names;

    if (!ids.isEmpty())
    {
        foreach (int id, ids)
        {
            if (hiddenTagsPolicy == IncludeHiddenTags || !isInternalTag(id))
            {
                names << tagName(id);
            }
        }
    }

    return names;
}

QString TagsCache::tagPath(int id, LeadingSlashPolicy slashPolicy) const
{
    d->checkInfos();

    QString path;
    QReadLocker locker(&d->lock);
    QList<TagShortInfo>::const_iterator it;

    for (it = d->find(id); it != d->infos.constEnd(); it = d->find(it->pid))
    {
        if (path.isNull())
        {
            path = it->name;
        }
        else
        {
            path = it->name + "/" + path;
        }
    }

    if (slashPolicy == IncludeLeadingSlash)
    {
        path.prepend("/");
    }

    return path;
}

QStringList TagsCache::tagPaths(const QList<int>& ids, LeadingSlashPolicy slashPolicy,
                                HiddenTagsPolicy hiddenTagsPolicy) const
{
    QStringList paths;

    if (!ids.isEmpty())
    {
        foreach (int id, ids)
        {
            if (hiddenTagsPolicy == IncludeHiddenTags || !isInternalTag(id))
            {
                paths << tagPath(id, slashPolicy);
            }
        }
    }

    return paths;
}

QList<int> TagsCache::tagsForName(const QString& tagName, HiddenTagsPolicy hiddenTagsPolicy) const
{
    d->checkNameHash();

    if (hiddenTagsPolicy == NoHiddenTags)
    {
        d->checkProperties();
        QList<int> ids;
        QMultiHash<QString, int>::const_iterator it;

        for (it = d->nameHash.find(tagName); it != d->nameHash.end() && it.key() == tagName; ++it)
        {
            if (!d->internalTags.contains(it.value()))
            {
                ids << it.value();
            }
        }

        return ids;
    }
    else
    {
        return d->nameHash.values(tagName);
    }
}

int TagsCache::tagForName(const QString& tagName, int parentId) const
{
    d->checkNameHash();
    QReadLocker locker(&d->lock);

    QList<TagShortInfo>::const_iterator tag;
    foreach (int id, d->nameHash.values(tagName))
    {
        tag = d->find(id);

        if (tag == d->infos.constEnd())
        {
            continue;    // error
        }

        if (tag->pid == parentId)
        {
            return tag->id;
        }
    }
    return 0;
}

bool TagsCache::hasTag(int id) const
{
    d->checkInfos();
    QReadLocker locker(&d->lock);
    return d->find(id) != d->infos.constEnd();
}

int TagsCache::parentTag(int id) const
{
    d->checkInfos();
    QReadLocker locker(&d->lock);
    QList<TagShortInfo>::const_iterator tag = d->find(id);

    if (tag != d->infos.constEnd())
    {
        return tag->pid;
    }

    return 0;
}

int TagsCache::tagForPath(const QString& tagPath) const
{
    // split full tag "url" into list of single tag names
    QStringList tagHierarchy = tagPath.split('/', QString::SkipEmptyParts);

    if (tagHierarchy.isEmpty())
    {
        return 0;
    }

    d->checkNameHash();

    // last entry in list is the actual tag name
    int tagID = 0;
    QString tagName = tagHierarchy.back();
    tagHierarchy.pop_back();
    QList<TagShortInfo>::const_iterator tag, parentTag;

    QReadLocker locker(&d->lock);
    // There might be multiple tags with the same name, but in different
    // hierarchies. We must check them all until we find the correct hierarchy
    foreach (int id, d->nameHash.values(tagName))
    {
        tag = d->find(id);

        if (tag == d->infos.constEnd())
        {
            continue;    // error
        }

        int parentID = tag->pid;

        // Check hierarchy, from bottom to top
        bool foundParentTag                 = true;
        QStringList::iterator parentTagName = tagHierarchy.end();

        while (foundParentTag && parentTagName != tagHierarchy.begin())
        {
            --parentTagName;

            foundParentTag = false;

            parentTag = d->find(parentID);

            // check if the parent is found and has the name we need
            if ( parentTag != d->infos.constEnd() &&
                 parentTag->name == (*parentTagName) )
            {
                parentID       = parentTag->pid;
                foundParentTag = true;
            }

            // If the parent matches, we continue with the grandparent.
            // If the candidate's parent do not match,
            // foundParentTag will be false, the while loop breaks.
        }

        // If we managed to traverse the full hierarchy,
        // we have our tag.
        if (foundParentTag)
        {
            tagID = tag->id;
            break;
        }
    }

    return tagID;
}

QList<int> TagsCache::tagsForPaths(const QStringList& tagPaths) const
{
    QList<int> ids;

    if (!tagPaths.isEmpty())
    {
        foreach (const QString& tagPath, tagPaths)
        {
            ids << tagForPath(tagPath);
        }
    }

    return ids;
}

int TagsCache::createTag(const QString& tagPathToCreate)
{
    // split full tag "url" into list of single tag names
    QStringList tagHierarchy = tagPathToCreate.split('/', QString::SkipEmptyParts);

    if (tagHierarchy.isEmpty())
    {
        return 0;
    }

    d->checkNameHash();

    int  parentTagID      = 0;
    int  tagID            = 0;
    bool parentTagExisted = true;

    QStringList tagsToCreate;
    int parentTagIDForCreation = 0;

    {
        QReadLocker locker(&d->lock);

        // Traverse hierarchy from top to bottom
        foreach (const QString& tagName, tagHierarchy)
        {
            tagID = 0;

            // if the parent tag did not exist, we need not check if the child exists
            if (parentTagExisted)
            {
                QList<TagShortInfo>::const_iterator tag;
                // find the tag with tag name according to tagHierarchy,
                // and parent ID identical to the ID of the tag we found in
                // the previous run.
                foreach (int id, d->nameHash.values(tagName))
                {
                    tag = d->find(id);

                    if (tag != d->infos.constEnd() && tag->pid == parentTagID)
                    {
                        tagID = tag->id;
                        break;
                    }
                }
            }

            if (tagID)
            {
                // tag already found in DB
                parentTagID      = tagID;
                parentTagExisted = true;
                continue;
            }
            else
            {
                tagsToCreate << tagName;

                if (parentTagExisted)
                {
                    parentTagIDForCreation = parentTagID;
                }

                parentTagID      = 0;
                parentTagExisted = false;
            }
        }
    }

    {
        DatabaseAccess access;
        foreach (const QString& tagName, tagsToCreate)
        {
            tagID = access.db()->addTag(parentTagIDForCreation, tagName, QString(), 0);

            if (tagID == -1)
            {
                break; // something wrong with DB
            }
            else
            {
                // change signals may be queued within a transaction. We know it changed.
                d->needUpdateInfos = true;
                d->needUpdateHash  = true;
            }

            parentTagIDForCreation = tagID;
        }
    }

    return tagID;
}

QList<int> TagsCache::createTags(const QStringList& tagPaths)
{
    QList<int> ids;

    if (!tagPaths.isEmpty())
    {
        foreach (const QString& tagPath, tagPaths)
        {
            ids << createTag(tagPath);
        }
    }

    return ids;
}

QList<int> TagsCache::getOrCreateTags(const QStringList& tagPaths)
{
    QList<int> ids;

    if (!tagPaths.isEmpty())
    {
        foreach (const QString& tagPath, tagPaths)
        {
            ids << getOrCreateTag(tagPath);
        }
    }

    return ids;
}

int TagsCache::getOrCreateTag(const QString& tagPath)
{
    int id = tagForPath(tagPath);

    if (!id)
    {
        id = createTag(tagPath);
    }

    return id;
}

int TagsCache::getOrCreateTagWithProperty(const QString& tagPath, const QString& property, const QString& value)
{
    int tagId = getOrCreateTag(tagPath);

    if (!hasProperty(tagId, property, value))
    {
        TagProperties props(tagId);
        props.setProperty(property, value);
    }

    return tagId;
}

bool TagsCache::hasProperty(int tagId, const QString& property, const QString& value) const
{
    d->checkProperties();
    QReadLocker locker(&d->lock);
    TagPropertiesRange range = d->findProperties(tagId);

    for (TagPropertiesConstIterator it = range.first; it != range.second; ++it)
        if (d->compareProperty(it, property, value))
        {
            return true;
        }

    return false;
}

QString TagsCache::propertyValue(int tagId, const QString& property) const
{
    d->checkProperties();
    QReadLocker locker(&d->lock);
    TagPropertiesRange range = d->findProperties(tagId);

    for (TagPropertiesConstIterator it = range.first; it != range.second; ++it)
        if (it->property == property)
        {
            return it->value;
        }

    return QString();
}

QStringList TagsCache::propertyValues(int tagId, const QString& property) const
{
    d->checkProperties();
    QReadLocker locker(&d->lock);
    TagPropertiesRange range = d->findProperties(tagId);
    QStringList values;

    for (TagPropertiesConstIterator it = range.first; it != range.second; ++it)
    {
        if (it->property == property)
        {
            // the list is ordered by property, after id
            for (; it != range.second && it->property == property; ++it)
            {
                values << it->value;
            }

            return values;
        }
    }

    return values;
}

QMap<QString, QString> TagsCache::properties(int tagId) const
{
    d->checkProperties();
    QReadLocker locker(&d->lock);
    QMap<QString, QString> map;
    TagPropertiesRange range = d->findProperties(tagId);
    QStringList values;

    for (TagPropertiesConstIterator it = range.first; it != range.second; ++it)
    {
        map[it->property] = it->value;
    }

    return map;
}

QList<int> TagsCache::tagsWithProperty(const QString& property, const QString& value) const
{
    d->checkProperties();
    QReadLocker locker(&d->lock);
    QList<int>  ids;

    for (TagPropertiesConstIterator it = d->tagProperties.begin(); it != d->tagProperties.end(); )
    {
        if (d->compareProperty(it, property, value))
        {
            ids << it->tagId;
            it = d->toNextTag(it);
        }
        else
        {
            ++it;
        }
    }

    return ids;
}

QList<int> TagsCache::tagsWithPropertyCached(const QString& property) const
{
    d->checkProperties();
    {
        QReadLocker locker(&d->lock);
        QHash<QString, QList<int> >::iterator it;
        it = d->tagsWithProperty.find(property);

        if (it != d->tagsWithProperty.end())
        {
            return it.value();
        }
    }

    QList<int> tags = tagsWithProperty(property);

    {
        QWriteLocker locker(&d->lock);
        d->tagsWithProperty[property] = tags;
    }

    return tags;
}

bool TagsCache::isInternalTag(int tagId) const
{
    d->checkProperties();
    QReadLocker locker(&d->lock);
    return d->internalTags.contains(tagId);
}

bool TagsCache::canBeWrittenToMetadata(int tagId) const
{
    // as long as we always call isInternalTag first, no need to call checkProperties() again
    //d->checkProperties();
    if (isInternalTag(tagId))
    {
        return false;
    }

    if (d->sortedListContains(tagsWithPropertyCached(propertyNameExcludedFromWriting()), tagId))
    {
        return false;
    }

    return true;
}

int TagsCache::getOrCreateInternalTag(const QString& tagName)
{
    // ensure the parent tag exists, including the internal property
    getOrCreateTagWithProperty(tagPathOfDigikamInternalTags(IncludeLeadingSlash), propertyNameDigikamInternalTag());

    QString tagPath = tagPathOfDigikamInternalTags(IncludeLeadingSlash) + '/' + tagName;
    return getOrCreateTagWithProperty(tagPath, propertyNameDigikamInternalTag());
}

void TagsCache::slotTagChanged(const TagChangeset& changeset)
{
    if (!d->changingDB && changeset.operation() != TagChangeset::IconChanged)
    {
        invalidate();
    }

    if (changeset.operation() == TagChangeset::Added)
    {
        emit tagAdded(changeset.tagId());
    }
    else if (changeset.operation() == TagChangeset::Deleted)
    {
        emit tagDeleted(changeset.tagId());
    }
}

int TagsCache::getTagForColorLabel(int label)
{
    if (label < NoColorLabel || label > WhiteLabel)
        return 0;

    d->checkLabelTags();
    QReadLocker locker(&d->lock);
    return d->colorLabelsTags[(ColorLabel)label];
}

int TagsCache::getColorLabelForTag(int tagId)
{
    d->checkLabelTags();
    QReadLocker locker(&d->lock);
    return d->colorLabelsTags.key(tagId, (ColorLabel)(-1));
}

int TagsCache::getTagForPickLabel(int label)
{
    if (label < NoPickLabel || label > AcceptedLabel)
        return 0;

    d->checkLabelTags();
    QReadLocker locker(&d->lock);
    return d->pickLabelsTags[(PickLabel)label];
}

int TagsCache::getPickLabelForTag(int tagId)
{
    d->checkLabelTags();
    QReadLocker locker(&d->lock);
    return d->pickLabelsTags.key(tagId, (PickLabel)(-1));
}

QStringList TagsCache::shortenedTagPaths(const QList<int>& ids, QList<int>* sortedIds,
                              LeadingSlashPolicy slashPolicy, HiddenTagsPolicy hiddenTagsPolicy) const
{
    QStringList paths;
    QList<QVariant> variantIds;

    // duplicates tagPath(), but we need the additional list of tag ids
    foreach (int id, ids)
    {
        if (hiddenTagsPolicy == IncludeHiddenTags || !isInternalTag(id))
        {
            paths      << tagPath(id, slashPolicy);
            variantIds << id;
        }
    }

    // The code is needed in libdigikamcore, so it cannot be moved here. TODO: Find a good place
    QStringList shortenedPaths = ImagePropertiesTab::shortenedTagPaths(paths, &variantIds);

    foreach (const QVariant& var, variantIds)
    {
        (*sortedIds) << var.toInt();
    }

    return shortenedPaths;
}

QStringList TagsCache::shortenedTagPaths(const QList<int>& ids,
                                         LeadingSlashPolicy slashPolicy, HiddenTagsPolicy hiddenTagsPolicy) const
{
    return ImagePropertiesTab::shortenedTagPaths(tagPaths(ids, slashPolicy, hiddenTagsPolicy));
}


} // namespace Digikam
