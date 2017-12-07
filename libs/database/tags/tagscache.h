/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-02
 * Description : Cache for Tag information
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QObject>

// Local includes

#include "coredbchangesets.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT TagsCache : public QObject
{
    Q_OBJECT

public:

    enum LeadingSlashPolicy
    {
        NoLeadingSlash,      /// Ex: "Places/Cities/Paris"
        IncludeLeadingSlash  /// Ex: "/Places/Cities/Paris"
    };

    enum HiddenTagsPolicy
    {
        NoHiddenTags,
        IncludeHiddenTags
    };

public:

    /**
     * Returns the name of the tag with the given id.
     * For the tag Places/Cities/Paris, this is Paris.
     * If there is no tag for the given id a null string is returned.
     */
    QString     tagName(int id) const;
    QStringList tagNames(const QList<int>& ids, HiddenTagsPolicy hiddenTagsPolicy = IncludeHiddenTags) const;

    /**
     * Returns the path of the tag with the given id.
     * For the tag Places/Cities/Paris, this is Places/Cities/Paris.
     * If there is no tag for the given id a null string is returned.
     */
    QString     tagPath(int id, LeadingSlashPolicy slashPolicy = IncludeLeadingSlash) const;
    QStringList tagPaths(const QList<int>& ids,
                         LeadingSlashPolicy slashPolicy = IncludeLeadingSlash,
                         HiddenTagsPolicy hiddenTagsPolicy = IncludeHiddenTags) const;

    /**
     * Returns true if the tag for the given id exists.
     */
    bool hasTag(int id) const;

    /**
     * Returns the parent tag id, or 0 if a toplevel tag or tag does not exist.
     */
    int parentTag(int id) const;

    /**
     * Returns the parent tag ids of the given tag, starting with the toplevel tag,
     * ending with the direct parent tag.
     * If the tag is a toplevel tag or does not exist, an empty list is returned.
     */
    QList<int> parentTags(int id) const;

    /**
     * Finds all tags with the given name.
     * For "Paris", this may give "Places/Cities/Paris" and "Places/USA/Texas/Paris".
     * If there is no tag with the given name at all, returns an empty list.
     */
    QList<int> tagsForName(const QString& tagName, HiddenTagsPolicy hiddenTagsPolicy = NoHiddenTags) const;

    /**
     * Returns the id of the tag with the given name and parent tag.
     * If parentId is 0, the tag is a toplevel tag.
     * Returns 0 if there is no such tag.
     */
    int tagForName(const QString& tagName, int parentId = 0) const;

    /**
     * Returns the tag matched exactly by the given path.
     * The path can be given with or without leading slash.
     * Returns 0 if there is no such tag, or if tagPath is empty.
     * If you want to create the tag if it does not yet exist,
     * use getOrCreateTag.
     */
    int tagForPath(const QString& tagPath) const;
    QList<int> tagsForPaths(const QStringList& tagPaths) const;

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

    /**
     * Calls getOrCreateTag for the given path,
     * and ensures that the tag has assigned the given property.
     * If you pass a null string as value, then the value is not checked and not changed.
     */
    int getOrCreateTagWithProperty(const QString& tagPath, const QString& property, const QString& value = QString());

    /**
     * Tests if the tag has the given property
     *  a) just has the property
     *  b) has the property with the given value (value not null)
     */
    bool hasProperty(int tagId, const QString& property, const QString& value = QString()) const;

    /**
     * Returns the value of the property.
     * Returning a null string cannot distinguish between the property set
     * with a null value, or the property not set.
     * The first method returns any property, if multiple are set with the same key.
     */
    QString     propertyValue(int tagId, const QString& property) const;
    QStringList propertyValues(int tagId, const QString& property) const;

    /**
     * Returns a list or a map of the properties of the tag.
     * Note: The list and map may be constructed for each call. Prefer hasProperty() and property().
     */
    QMap<QString, QString> properties(int tagId) const;

    /**
     * Finds all tags with the given property. The tag
     *  a)just has the property
     *  b) has the property with the given value (value not null)
     * Note: The returned list is sorted.
     */
    QList<int> tagsWithProperty(const QString& property, const QString& value = QString()) const;

    /**
     * This method is equivalent to calling tagsWithProperty(property), but the immediate result
     * will be cached for subsequent calls.
     * Use it for queries for which you know that they will be issued very often,
     * so that it's worth caching the result of the already pretty fast tagsWithProperty().
     */
    QList<int> tagsWithPropertyCached(const QString& property) const;

    /**
     * Returns if a tag is to be regarded program-internal, that is,
     * a technical implementation detail not visible to the user at any time.
     */
    bool isInternalTag(int tagId) const;

    /**
     * From the given list of tag ids, filter out any internal tags
     * and return only public tags.
     */
    QList<int> publicTags(const QList<int>& tagIds) const;

    /**
     * Returns true if the given list of tag ids contains at
     * least one non-internal tag
     */
    bool  containsPublicTags(const QList<int>& tagIds) const;

    /**
     * Returns if a tag shall be written to the metadata of a file.
     * Always returns false if the tag is a program-internal tag.
     */
    bool canBeWrittenToMetadata(int tagId) const;

    /**
     * For the given tag name (not path!), find the existing tag or
     * creates a new internal tags under the usual tag path used for
     * internal tags.
     */
    int getOrCreateInternalTag(const QString& tagName);

    /**
     * Return internal tags ID corresponding of color label id. see ColorLabel values from globals.h.
     * Return 0 if not it's found.
     */
    int tagForColorLabel(int label);

    /**
     * Returns all color label tags, where index is the label id and value the tag id.
     */
    QVector<int> colorLabelTags();

    /**
     * Return color label id corresponding of internal tags ID. see ColorLabel values from globals.h.
     * Return -1 if not it's found.
     */
    int colorLabelForTag(int tagId);

    /**
     * From the given list of tags, returns the color label corresponding to the first encountered
     * tag which is a color label tag.
     * Returns -1 if no tag in the list is a color label tag.
     */
    int colorLabelFromTags(QList<int> tagIds);

    /**
     * Return internal tags ID corresponding of pick label id. see PickLabel values from globals.h.
     * Return 0 if not it's found.
     */
    int tagForPickLabel(int label);

    /**
     * Returns all pick label tags, where index is the label id and value the tag id.
     */
    QVector<int> pickLabelTags();

    /**
     * Return pick label id corresponding of internal tags ID. see PickLabel values from globals.h.
     * Return -1 if not it's found.
     */
    int pickLabelForTag(int tagId);

    /**
     * From the given list of tags, returns the pick label corresponding to the first encountered
     * tag which is a pick label tag.
     * Returns -1 if no tag in the list is a pick label tag.
     */
    int pickLabelFromTags(QList<int> tagIds);

    /**
     * Returns a list of tag ids whose tag name (not path) starts with /  contains the given fragment
     */
    QList<int> tagsContaining(const QString& fragment,
                               Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive,
                               HiddenTagsPolicy hiddenTagsPolicy = NoHiddenTags);
    QList<int> tagsStartingWith(const QString& begin,
                                Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive,
                                HiddenTagsPolicy hiddenTagsPolicy = NoHiddenTags);

    /**
     * Utility method.
     * Orders the given tag paths. If tags begin with the same path (parent tags),
     * the relevant part is cut off in the second line.
     * The second variant allows you to pass a list as return parameter.
     * This list will contain, upon return, the tag id corresponding to each
     * tag in the returned, sorted list of shortened paths.
     */
    QStringList shortenedTagPaths(const QList<int>& ids,
                                  LeadingSlashPolicy slashPolicy = IncludeLeadingSlash,
                                  HiddenTagsPolicy hiddenTagsPolicy = IncludeHiddenTags) const;
    QStringList shortenedTagPaths(const QList<int>& ids, QList<int>* sortedIds,
                                  LeadingSlashPolicy slashPolicy = IncludeLeadingSlash,
                                  HiddenTagsPolicy hiddenTagsPolicy = IncludeHiddenTags) const;

public:

    static TagsCache* instance();

    static QLatin1String tagPathOfDigikamInternalTags(LeadingSlashPolicy slashPolicy = IncludeLeadingSlash);
    static QLatin1String propertyNameDigikamInternalTag();
    static QLatin1String propertyNameExcludedFromWriting();

Q_SIGNALS:

    /**
     * These signals are provided for convenience; for finer grained information
     * use CoreDbWatch. Use a queued connection if you carry out
     * longer operations from slots connected to these signals.
     */
    void tagAdded(int tagId);
    void tagDeleted(int tagId);
    void tagAboutToBeDeleted(QString name);

private Q_SLOTS:

    void slotTagChanged(const TagChangeset& changeset);

private:

    TagsCache();
    ~TagsCache();
    void initialize();
    void invalidate();

private:

    friend class CoreDbAccess;
    friend class TagsCacheCreator;
    friend class ChangingDB;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // TAGSCACHE_H
