/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Filter values for use with ImageFilterModel
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C)      2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef IMAGEFILTERSETTINGS_H
#define IMAGEFILTERSETTINGS_H

// Qt includes

#include <QHash>
#include <QList>
#include <QMap>
#include <QString>

// KDE includes

#include <kurl.h>

// Local includes

#include "searchtextbar.h"
#include "mimefilter.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageInfo;

namespace DatabaseFields
{
class Set;
}

class DIGIKAM_DATABASE_EXPORT ImageFilterSettings
{
public:

    ImageFilterSettings();

    /**
     *  Returns true if the given ImageInfo matches the filter criteria.
     *  Optionally, foundText is set to true if it matched by text search.
     */
    bool matches(const ImageInfo& info, bool* foundText = 0) const;

    /// --- Tags filter ---

    /// Possible logical matching condition used to sort tags id.
    enum MatchingCondition
    {
        OrCondition,
        AndCondition
    };

    bool              untaggedFilter;
    QList<int>        includeTagFilter;
    QList<int>        excludeTagFilter;
    MatchingCondition matchingCond;
    QList<int>        colorLabelTagFilter;

    void setTagFilter(const QList<int>& includedTags, const QList<int>& excludedTags, MatchingCondition matchingCond,
                      bool showUnTagged, const QList<int>& clTagIds);

    /// --- Rating filter ---

    /// Possible conditions used to filter rating: >=, =, <=
    enum RatingCondition
    {
        GreaterEqualCondition,
        EqualCondition,
        LessEqualCondition
    };

    int                             ratingFilter;
    RatingCondition                 ratingCond;

    void setRatingFilter(int rating, RatingCondition ratingCond);

    /// --- Date filter ---

    QMap<QDateTime, bool>           dayFilter;
    void setDayFilter(const QList<QDateTime>& days);

    /// --- Text filter ---

    SearchTextSettings              textFilterSettings;
    void setTextFilter(const SearchTextSettings& settings);

    /// Helpers for text search: Set these if you want to search album or tag names with text search
    QHash<int, QString> tagNameHash;
    QHash<int, QString> albumNameHash;
    void setTagNames(const QHash<int, QString>& tagNameHash);
    void setAlbumNames(const QHash<int, QString>& albumNameHash);

    /// --- Mime filter ---

    MimeFilter::TypeMimeFilter      mimeTypeFilter;
    void setMimeTypeFilter(int mimeTypeFilter);

    /// Returns if the color labales is a filter criteria
    bool isFilteringByColorLabels() const;
    /// Returns if the tag is a filter criteria
    bool isFilteringByTags() const;
    /// Returns if the text (including comment) is a filter criteria
    bool isFilteringByText() const;
    /// Returns if images will be filtered by these criteria at all
    bool isFiltering() const;

    /// --- URL whitelist filter
    QHash<QString,KUrl::List> urlWhitelists;
    void setUrlWhitelist(const KUrl::List& urlList, const QString& id);

    /// --- ID whitelist filter
    QHash<QString,QList<qlonglong> > idWhitelists;
    void setIdWhitelist(const QList<qlonglong>& idList, const QString& id);

    /// --- Change notification ---

    /** Returns database fields a change in which would affect the current filtering.
     *  To find out if an image tag change affects filtering, test isFilteringByTags().
     *  The text filter will also be affected by changes in tags and album names.
     */
    DatabaseFields::Set watchFlags() const;
};

// ---------------------------------------------------------------------------------------
class VersionManagerSettings;

class DIGIKAM_DATABASE_EXPORT VersionImageFilterSettings
{
public:

    VersionImageFilterSettings();
    VersionImageFilterSettings(const VersionManagerSettings& settings);

    bool operator==(const VersionImageFilterSettings& other) const;

    /**
     *  Returns true if the given ImageInfo matches the filter criteria.
     */
    bool matches(const ImageInfo& info) const;

    bool isHiddenBySettings(const ImageInfo& info) const;
    bool isExemptedBySettings(const ImageInfo& info) const;

    /// --- Tags filter ---

    void setVersionManagerSettings(const VersionManagerSettings& settings);

    /**
     * Add list with exceptions: These images will be exempted from filtering by this filter
     */
    void setExceptionList(const QList<qlonglong>& idlist, const QString& id);

    /// Returns if images will be filtered by these criteria at all
    bool isFiltering() const;
    /// Returns if the tag is a filter criteria
    bool isFilteringByTags() const;

    /// DatabaseFields::Set watchFlags() const: Would return 0

protected:

    QList<int>                       excludeTagFilter;
    int                              includeTagFilter;
    int                              exceptionTagFilter;
    QHash<QString,QList<qlonglong> > exceptionLists;
};

} // namespace Digikam

#endif // IMAGEFILTERSETTINGS_H
