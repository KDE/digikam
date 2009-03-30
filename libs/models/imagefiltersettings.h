/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Filter values for use with ImageFilterModel
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes.

#include <QHash>
#include <QList>
#include <QMap>
#include <QString>

// Local includes.

#include "searchtextbar.h"
#include "mimefilter.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageInfo;

class DIGIKAM_MODEL_EXPORT ImageFilterSettings
{
public:

    ImageFilterSettings();

    /**
     *  Returns true if the given ImageInfo matches the filter criteria.
     *  Optionally, foundText is set to true if it matched by text search.
     */
    bool matches(const ImageInfo &info, bool *foundText = 0) const;

    /// --- Tags filter ---

    /// Possible logical matching condition used to sort tags id.
    enum MatchingCondition
    {
        OrCondition,
        AndCondition
    };

    bool                            untaggedFilter;
    QList<int>                      tagFilter;
    MatchingCondition               matchingCond;

    void setTagFilter(const QList<int>& tags, MatchingCondition matchingCond,
                      bool showUnTagged = false);

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
    void setTagNames(const QHash<int, QString> &tagNameHash);
    void setAlbumNames(const QHash<int, QString> &albumNameHash);

    /// --- Mime filter ---

    MimeFilter::TypeMimeFilter      mimeTypeFilter;
    void setMimeTypeFilter(int mimeTypeFilter);

    /// Returns if the tag is a filter criteria
    bool isFilteringByTags() const;
    /// Returns if the text (including comment) is a filter criteria
    bool isFilteringByText() const;
    /// Returns if images will be filtered by these criteria at all
    bool isFiltering() const;
};

} // namespace Digikam

#endif // IMAGEFILTERSETTINGS_H