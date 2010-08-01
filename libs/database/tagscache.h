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

#ifndef TAGSCACHE_H
#define TAGSCACHE_H

// Local includes

#include "databasechangesets.h"
#include "digikam_export.h"

namespace Digikam
{

class TagsCachePriv;

class DIGIKAM_DATABASE_EXPORT TagsCache : public QObject
{
    Q_OBJECT

public:

    static TagsCache* instance();

    enum LeadingSlashPolicy
    {
        NoLeadingSlash,      /// "Places/Cities/Paris"
        IncludeLeadingSlash  ///  "/Places/Cities/Paris"
    };

    /**
     * Returns the name of the tag with the given id.
     * For the tag Places/Cities/Paris, this is Paris.
     * If there is no tag for the given id a null string is returned.
     */
    QString     tagName(int id);
    QStringList tagNames(const QList<int>& ids);

    /**
     * Returns the path of the tag with the given id.
     * For the tag Places/Cities/Paris, this is Places/Cities/Paris.
     * If there is no tag for the given id a null string is returned.
     */
    QString     tagPath(int id, LeadingSlashPolicy slashPolicy = IncludeLeadingSlash);
    QStringList tagPaths(const QList<int>& ids, LeadingSlashPolicy slashPolicy = IncludeLeadingSlash);

    /**
     * Returns true if the tag for the given id exists.
     */
    bool hasTag(int id);

    /**
     * Returns the parent tag id, or 0 if a toplevel tag or tag does not exist.
     */
    int parentTag(int id);

    /**
     * Finds all tags with the given name.
     * For "Paris", this may give "Places/Cities/Paris" and "Places/USA/Texas/Paris".
     * If there is no tag with the given name at all, returns an empty list.
     */
    QList<int> tagsForName(const QString& tagName);

    /**
     * Returns the id of the tag with the given name and parent tag.
     * If parentId is 0, the tag is a toplevel tag.
     * Returns 0 if there is no such tag.
     */
    int tagForName(const QString& tagName, int parentId = 0);

    /**
     * Returns the tag matched exactly by the given path.
     * The path can be given with or without leading slash.
     * Returns 0 if there is no such tag, or if tagPath is empty.
     * If you want to create the tag if it does not yet exist,
     * use getOrCreateTag.
     */
    int tagForPath(const QString& tagPath);
    QList<int> tagsForPaths(const QStringList& tagPaths);

    /**
     * Add the tag described by the given tag path,
     * and all missing parent tags, to the database.
     * Returns the tag id.
     * Use this if you know that tag path does not exist.
     * If you are unsure, use getOrCreateTag.
     */
    int createTag(const QString& tagPathToCreate);
    QList<int> createTags(const QStringList& tagPaths);

    /** A combination of tagForPath and createTag:
     *  Finds ids for the given tagPaths.
     *  If a tag does not exist yet and create is true, it will be created.
     *  Otherwise the id 0 is returned for this path.
     */
    int getOrCreateTag(const QString& tagPath);
    QList<int> getOrCreateTags(const QStringList& tagPaths);
    
    QList<int> getAllSubtags(int tagId);

signals:

    /** These signals are provided for convenience; for finer grained information
        use DatabaseWatch. Use a queued connection if you carry out
        longer operations from slots connected to these signals.
    */
    void tagAdded(int tagId);
    void tagDeleted(int tagId);

private Q_SLOTS:

    void slotTagChanged(const TagChangeset& changeset);

private:

    friend class DatabaseAccess;
    friend class TagsCacheCreator;

    TagsCache();
    ~TagsCache();
    void initialize();

    TagsCachePriv* const d;
};

} // namespace Digikam

#endif // TAGSCACHE_H
