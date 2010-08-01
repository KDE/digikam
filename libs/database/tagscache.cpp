/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-02
 * Description : Cache for Tag information
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// KDE includes

#include <kdebug.h>
#include <kglobal.h>

// Local includes

#include "albumdb.h"
#include "databaseaccess.h"
#include "databasewatch.h"


namespace Digikam
{

static bool lessThanForTagShortInfo(const TagShortInfo& first, const TagShortInfo& second)
{
    return first.id < second.id;
}

class TagsCachePriv
{
public:

    TagsCachePriv()
    {
        initialized     = false;
        needUpdateInfos = true;
        needUpdateHash  = true;
        changingDB      = false;
    }

    bool                     initialized;
    bool                     needUpdateInfos;
    bool                     needUpdateHash;
    bool                     changingDB;
    QReadWriteLock           lock;
    QList<TagShortInfo>      infos;
    QMultiHash<QString, int> nameHash;

    void checkInfos()
    {
        if (needUpdateInfos && initialized)
        {
            QList<TagShortInfo> newInfos = DatabaseAccess().db()->getTagShortInfos();
            QWriteLocker locker(&lock);
            infos = newInfos;
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

    // remember to call under lock
    QList<TagShortInfo>::const_iterator find(int id) const
    {
        TagShortInfo info;
        info.id = id;

        // we use the fact that d->infos is sorted by id
        return qBinaryFind(infos.constBegin(), infos.constEnd(), info, lessThanForTagShortInfo);
    }
};

class ChangingDB
{
public:

    ChangingDB(TagsCachePriv* d)
        : d(d)
    {
        d->changingDB = true;
    }
    ~ChangingDB()
    {
        d->changingDB = false;
    }
    TagsCachePriv* const d;
};

class TagsCacheCreator { public: TagsCache object; };
K_GLOBAL_STATIC(TagsCacheCreator, creator)

TagsCache* TagsCache::instance()
{
    return &creator->object;
}

TagsCache::TagsCache()
         : d(new TagsCachePriv)
{
}

TagsCache::~TagsCache()
{
    delete d;
}

void TagsCache::initialize()
{
    if (d->initialized)
        return;

    connect(DatabaseAccess::databaseWatch(), SIGNAL(tagChange(const TagChangeset &)),
            this, SLOT(slotTagChanged(const TagChangeset &)),
            Qt::DirectConnection);

    d->initialized = true;
}

QString TagsCache::tagName(int id)
{
    d->checkInfos();

    QReadLocker locker(&d->lock);
    QList<TagShortInfo>::const_iterator it = d->find(id);
    if (it != d->infos.constEnd())
        return it->name;
    return QString();
}

QStringList TagsCache::tagNames(const QList<int>& ids)
{
    QStringList names;
    foreach (int id, ids)
        names << tagName(id);
    return names;
}

QString TagsCache::tagPath(int id, LeadingSlashPolicy slashPolicy)
{
    d->checkInfos();

    QString path;
    QReadLocker locker(&d->lock);
    QList<TagShortInfo>::const_iterator it;
    for (it = d->find(id); it != d->infos.constEnd(); it = d->find(it->pid))
    {
        if (path.isNull())
            path = it->name;
        else
            path = it->name + "/" + path;
    }

    if (slashPolicy == IncludeLeadingSlash)
        path.prepend("/");

    return path;
}

QStringList TagsCache::tagPaths(const QList<int>& ids, LeadingSlashPolicy slashPolicy)
{
    QStringList paths;
    foreach (int id, ids)
        paths << tagPath(id, slashPolicy);
    return paths;
}

QList<int> TagsCache::tagsForName(const QString& tagName)
{
    d->checkNameHash();
    return d->nameHash.values(tagName);
}

int TagsCache::tagForName(const QString& tagName, int parentId)
{
    d->checkNameHash();
    QReadLocker locker(&d->lock);

    QList<TagShortInfo>::const_iterator tag;
    foreach (int id, d->nameHash.values(tagName))
    {
        tag = d->find(id);
        if (tag == d->infos.constEnd())
            continue; // error

        if (tag->pid == parentId)
            return tag->id;
    }
    return 0;
}

bool TagsCache::hasTag(int id)
{
    d->checkInfos();
    QReadLocker locker(&d->lock);
    return d->find(id) != d->infos.constEnd();
}

int TagsCache::parentTag(int id)
{
    d->checkInfos();
    QReadLocker locker(&d->lock);
    QList<TagShortInfo>::const_iterator tag = d->find(id);
    if (tag != d->infos.constEnd())
        return tag->pid;
    return 0;
}

int TagsCache::tagForPath(const QString& tagPath)
{
    // split full tag "url" into list of single tag names
    QStringList tagHierarchy = tagPath.split('/', QString::SkipEmptyParts);
    if (tagHierarchy.isEmpty())
        return 0;

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
            continue; // error

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

QList<int> TagsCache::tagsForPaths(const QStringList& tagPaths)
{
    QList<int> ids;
    foreach (const QString& tagPath, tagPaths)
    {
        ids << tagForPath(tagPath);
    }
    return ids;
}

int TagsCache::createTag(const QString& tagPathToCreate)
{
    // split full tag "url" into list of single tag names
    QStringList tagHierarchy = tagPathToCreate.split('/', QString::SkipEmptyParts);

    if (tagHierarchy.isEmpty())
        return 0;

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
                    parentTagIDForCreation = parentTagID;

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
    foreach (const QString& tagPath, tagPaths)
    {
        ids << createTag(tagPath);
    }
    return ids;
}

QList<int> TagsCache::getOrCreateTags(const QStringList& tagPaths)
{
    QList<int> ids;
    foreach (const QString& tagPath, tagPaths)
    {
        ids << getOrCreateTag(tagPath);
    }
    return ids;
}

int TagsCache::getOrCreateTag(const QString& tagPath)
{
    int id = tagForPath(tagPath);
    if (!id)
        id = createTag(tagPath);
    return id;
}

void TagsCache::slotTagChanged(const TagChangeset& changeset)
{
    if (!d->changingDB && changeset.operation() != TagChangeset::IconChanged)
    {
        d->needUpdateInfos = true;
        d->needUpdateHash  = true;
    }
    if (changeset.operation() == TagChangeset::Added)
        emit tagAdded(changeset.tagId());
    else if (changeset.operation() == TagChangeset::Deleted)
        emit tagDeleted(changeset.tagId());
}


} // namespace Digikam
