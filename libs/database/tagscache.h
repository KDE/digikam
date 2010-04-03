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

class DIGIKAM_EXPORT TagsCache : public QObject
{
    Q_OBJECT

public:

    static TagsCache *instance();

    enum LeadingSlashPolicy
    {
        NoLeadingSlash,      /// "Places/Cities/Paris"
        IncludeLeadingSlash ///  "/Places/Cities/Paris"
    };

    /**
     * Returns the name of the tag with the given id.
     * For the tag Places/Cities/Paris, this is Paris.
     * If there is no tag for the given id a null string is returned.
     */
    QString tagName(int id);
    QStringList tagNames(const QList<int>& ids);

    /**
     * Returns the path of the tag with the given id.
     * For the tag Places/Cities/Paris, this is Places/Cities/Paris.
     * If there is no tag for the given id a null string is returned.
     */
    QString tagPath(int id, LeadingSlashPolicy slashPolicy = IncludeLeadingSlash);
    QStringList tagPaths(const QList<int>& ids, LeadingSlashPolicy slashPolicy = IncludeLeadingSlash);

    /**
     * Returns true if the tag for the given id exists.
     */
    bool hasTag(int id);

    /**
     * Finds all tags with the given name.
     * For "Paris", this may give "Places/Cities/Paris" and "Places/USA/Texas/Paris".
     * If there is no tag with the given name at all, returns an empty list.
     */
    QList<int> tagsForName(const QString& tagName);

    /**
     * Returns the tag matched exactly by the given path.
     * The path can be given with or without leading slash.
     * Returns 0 if there is no such tag, or if tagPath is empty.
     */
    int tagForPath(const QString& tagPath);

    /**
     * Add the tag described by the given tag path,
     * and all missing parent tags, to the database.
     * Returns the tag id.
     */
    int createTag(const QString& tagPathToCreate);

    /** A combination of tagForPath and createTag:
     *  Finds ids for the given tagPaths.
     *  If a tag does not exist yet and create is true, it will be created.
     *  Otherwise the id 0 is returned for this path.
     */
    QList<int> getTagsFromTagPaths(const QStringList& tagPaths, bool create);

private Q_SLOTS:

    void slotTagChange(const TagChangeset& changeset);

private:

    friend class DatabaseAccess;
    friend class TagsCacheCreator;
    TagsCache();
    ~TagsCache();
    void initialize();

    TagsCachePriv* const d;
};

}

#endif // TAGSCACHE_H