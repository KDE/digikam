/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Filter values for use with ImageFilterModel
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C)      2011 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C)      2014 by Mohamed Anwer <m dot anwer at gmx dot com>
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
#include <QSet>
#include <QUrl>

// Local includes

#include "searchtextbar.h"
#include "mimefilter.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageInfo;
class VersionManagerSettings;

namespace DatabaseFields
{
    class Set;
}

// ---------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT SearchTextFilterSettings : public SearchTextSettings
{

public:

    enum TextFilterFields
    {
        None             = 0x00,
        ImageName        = 0x01,
        ImageTitle       = 0x02,
        ImageComment     = 0x04,
        TagName          = 0x08,
        AlbumName        = 0x10,
        ImageAspectRatio = 0x20,
        ImagePixelSize   = 0x40,
        All              = ImageName | ImageTitle | ImageComment | TagName | AlbumName | ImageAspectRatio | ImagePixelSize
    };

public:

    SearchTextFilterSettings()
    {
        textFields = None;
    }

    explicit SearchTextFilterSettings(const SearchTextSettings& settings)
    {
        caseSensitive = settings.caseSensitive;
        text          = settings.text;
        textFields    = None;
    }

    TextFilterFields textFields;
};

// ---------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ImageFilterSettings
{
public:

    ImageFilterSettings();

    /**
     *  Returns true if the given ImageInfo matches the filter criteria.
     *  Optionally, foundText is set to true if it matched by text search.
     */
    bool matches(const ImageInfo& info, bool* const foundText = 0) const;

public:

    /// --- Tags filter ---

    /// Possible logical matching condition used to sort tags id.
    enum MatchingCondition
    {
        OrCondition,
        AndCondition
    };

    void setTagFilter(const QList<int>& includedTags,
                      const QList<int>& excludedTags,
                      MatchingCondition matchingCond,
                      bool              showUnTagged,
                      const QList<int>& clTagIds,
                      const QList<int>& plTagIds);

public:

    /// --- Rating filter ---

    /// Possible conditions used to filter rating: >=, =, <=
    enum RatingCondition
    {
        GreaterEqualCondition,
        EqualCondition,
        LessEqualCondition
    };

    void setRatingFilter(int rating, RatingCondition ratingCond, bool isUnratedExcluded);

public:

    /// --- Date filter ---
    void setDayFilter(const QList<QDateTime>& days);

public:

    /// --- Text filter ---
    void setTextFilter(const SearchTextFilterSettings& settings);
    void setTagNames(const QHash<int, QString>& tagNameHash);
    void setAlbumNames(const QHash<int, QString>& albumNameHash);

public:

    /// --- Mime filter ---
    void setMimeTypeFilter(int mimeTypeFilter);

public:

    /// --- Geolocation filter
    enum GeolocationCondition
    {
        GeolocationNoFilter       = 0,
        GeolocationNoCoordinates  = 1 << 1,
        GeolocationHasCoordinates = 1 << 2
    };

    void setGeolocationFilter(const GeolocationCondition& condition);

public:

    /// Returns if the day is a filter criteria
    bool isFilteringByDay()         const;

    /// Returns if the type mime is a filter criteria
    bool isFilteringByTypeMime()    const;

    /// Returns whether geolocation is a filter criteria
    bool isFilteringByGeolocation() const;

    /// Returns if the rating is a filter criteria
    bool isFilteringByRating()      const;

    /// Returns if the pick labels is a filter criteria
    bool isFilteringByPickLabels()  const;

    /// Returns if the color labels is a filter criteria
    bool isFilteringByColorLabels() const;

    /// Returns if the tag is a filter criteria
    bool isFilteringByTags()        const;

    /// Returns if the text (including comment) is a filter criteria
    bool isFilteringByText()        const;

    /// Returns if images will be filtered by these criteria at all
    bool isFiltering()              const;

public:

    /// --- URL whitelist filter
    void setUrlWhitelist(const QList<QUrl>& urlList, const QString& id);

public:

    /// --- ID whitelist filter
    void setIdWhitelist(const QList<qlonglong>& idList, const QString& id);

public:

    /// --- Change notification ---

    /** Returns database fields a change in which would affect the current filtering.
     *  To find out if an image tag change affects filtering, test isFilteringByTags().
     *  The text filter will also be affected by changes in tags and album names.
     */
    DatabaseFields::Set watchFlags() const;

private:

    /**
     * @brief Returns whether some internal filtering (whitelist by id or URL) or normal filtering is going on
     */
    bool isFilteringInternally() const;

private:

    /// --- Tags filter ---
    bool                             m_untaggedFilter;
    QList<int>                       m_includeTagFilter;
    QList<int>                       m_excludeTagFilter;
    MatchingCondition                m_matchingCond;
    QList<int>                       m_colorLabelTagFilter;
    QList<int>                       m_pickLabelTagFilter;

    /// --- Rating filter ---
    int                              m_ratingFilter;
    RatingCondition                  m_ratingCond;
    bool                             m_isUnratedExcluded;

    /// --- Date filter ---
    QMap<QDateTime, bool>            m_dayFilter;

    /// --- Text filter ---
    SearchTextFilterSettings         m_textFilterSettings;

    /// Helpers for text search: Set these if you want to search album or tag names with text search
    QHash<int, QString>              m_tagNameHash;
    QHash<int, QString>              m_albumNameHash;

    /// --- Mime filter ---
    MimeFilter::TypeMimeFilter       m_mimeTypeFilter;

    /// --- Geolocation filter
    GeolocationCondition             m_geolocationCondition;

    /// --- URL whitelist filter
    QHash<QString,QList<QUrl>>        m_urlWhitelists;

    /// --- ID whitelist filter
    QHash<QString,QList<qlonglong> > m_idWhitelists;
};

// ---------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT VersionImageFilterSettings
{
public:

    VersionImageFilterSettings();
    explicit VersionImageFilterSettings(const VersionManagerSettings& settings);

    bool operator==(const VersionImageFilterSettings& other) const;

    /**
     *  Returns true if the given ImageInfo matches the filter criteria.
     */
    bool matches(const ImageInfo& info) const;

    bool isHiddenBySettings(const ImageInfo& info)   const;
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

    QList<int>                       m_excludeTagFilter;
    int                              m_includeTagFilter;
    int                              m_exceptionTagFilter;
    QHash<QString,QList<qlonglong> > m_exceptionLists;
};

// ---------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT GroupImageFilterSettings
{
public:

    GroupImageFilterSettings();

    bool operator==(const GroupImageFilterSettings& other) const;

    /**
     *  Returns true if the given ImageInfo matches the filter criteria.
     */
    bool matches(const ImageInfo& info) const;

    /**
     * Open or close a group.
     */
    void setOpen(qlonglong group, bool open);
    bool isOpen(qlonglong group) const;

    /**
     * Open all groups
     */
    void setAllOpen(bool open);
    bool isAllOpen() const;

    /// Returns if images will be filtered by these criteria at all
    bool isFiltering() const;

    DatabaseFields::Set watchFlags() const;

protected:

    bool            m_allOpen;
    QSet<qlonglong> m_openGroups;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ImageFilterSettings::GeolocationCondition)

#endif // IMAGEFILTERSETTINGS_H
