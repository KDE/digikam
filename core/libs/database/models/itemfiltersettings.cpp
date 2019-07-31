/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Filter values for use with ItemFilterModel
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C)      2011 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C)      2014 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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

#include "itemfiltersettings.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QDateTime>

// Local includes

#include "digikam_debug.h"
#include "coredbfields.h"
#include "digikam_globals.h"
#include "iteminfo.h"
#include "tagscache.h"
#include "versionmanagersettings.h"

namespace Digikam
{

ItemFilterSettings::ItemFilterSettings()
{
    m_untaggedFilter       = false;
    m_isUnratedExcluded    = false;
    m_ratingFilter         = 0;
    m_mimeTypeFilter       = MimeFilter::AllFiles;
    m_ratingCond           = GreaterEqualCondition;
    m_matchingCond         = OrCondition;
    m_geolocationCondition = GeolocationNoFilter;
}

DatabaseFields::Set ItemFilterSettings::watchFlags() const
{
    DatabaseFields::Set set;

    if (isFilteringByDay())
    {
        set |= DatabaseFields::CreationDate;
    }

    if (isFilteringByText())
    {
        set |= DatabaseFields::Name;
        set |= DatabaseFields::Comment;
    }

    if (isFilteringByRating())
    {
        set |= DatabaseFields::Rating;
    }

    if (isFilteringByTypeMime())
    {
        set |= DatabaseFields::Category;
        set |= DatabaseFields::Format;
    }

    if (isFilteringByGeolocation())
    {
        set |= DatabaseFields::ItemPositionsAll;
    }

    if (isFilteringByColorLabels())
    {
        set |= DatabaseFields::ColorLabel;
    }

    if (isFilteringByPickLabels())
    {
        set |= DatabaseFields::PickLabel;
    }

    return set;
}

bool ItemFilterSettings::isFilteringByDay() const
{
    if (!m_dayFilter.isEmpty())
    {
        return true;
    }

    return false;
}

bool ItemFilterSettings::isFilteringByTags() const
{
    if (!m_includeTagFilter.isEmpty() || !m_excludeTagFilter.isEmpty() || m_untaggedFilter)
    {
        return true;
    }

    return false;
}

bool ItemFilterSettings::isFilteringByColorLabels() const
{
    if (!m_colorLabelTagFilter.isEmpty())
    {
        return true;
    }

    return false;
}

bool ItemFilterSettings::isFilteringByPickLabels() const
{
    if (!m_pickLabelTagFilter.isEmpty())
    {
        return true;
    }

    return false;
}

bool ItemFilterSettings::isFilteringByText() const
{
    if (!m_textFilterSettings.text.isEmpty())
    {
        return true;
    }

    return false;
}

bool ItemFilterSettings::isFilteringByTypeMime() const
{
    if (m_mimeTypeFilter != MimeFilter::AllFiles)
    {
        return true;
    }

    return false;
}

bool ItemFilterSettings::isFilteringByGeolocation() const
{
    return (m_geolocationCondition != GeolocationNoFilter);
}

bool ItemFilterSettings::isFilteringByRating() const
{
    if (m_ratingFilter != 0 || m_ratingCond != GreaterEqualCondition || m_isUnratedExcluded)
    {
        return true;
    }

    return false;
}

bool ItemFilterSettings::isFilteringInternally() const
{
    return (isFiltering() || !m_urlWhitelists.isEmpty() || !m_idWhitelists.isEmpty());
}

bool ItemFilterSettings::isFiltering() const
{
    return isFilteringByDay()         ||
           isFilteringByTags()        ||
           isFilteringByText()        ||
           isFilteringByRating()      ||
           isFilteringByTypeMime()    ||
           isFilteringByColorLabels() ||
           isFilteringByPickLabels()  ||
           isFilteringByGeolocation();
}

void ItemFilterSettings::setDayFilter(const QList<QDateTime>& days)
{
    m_dayFilter.clear();

    for (QList<QDateTime>::const_iterator it = days.constBegin(); it != days.constEnd(); ++it)
    {
        m_dayFilter.insert(*it, true);
    }
}

void ItemFilterSettings::setTagFilter(const QList<int>& includedTags,
                                       const QList<int>& excludedTags,
                                       MatchingCondition matchingCondition,
                                       bool showUnTagged,
                                       const QList<int>& clTagIds,
                                       const QList<int>& plTagIds)
{
    m_includeTagFilter    = includedTags;
    m_excludeTagFilter    = excludedTags;
    m_matchingCond        = matchingCondition;
    m_untaggedFilter      = showUnTagged;
    m_colorLabelTagFilter = clTagIds;
    m_pickLabelTagFilter  = plTagIds;
}

void ItemFilterSettings::setRatingFilter(int rating, RatingCondition ratingCondition, bool isUnratedExcluded)
{
    m_ratingFilter      = rating;
    m_ratingCond        = ratingCondition;
    m_isUnratedExcluded = isUnratedExcluded;
}

void ItemFilterSettings::setMimeTypeFilter(int mime)
{
    m_mimeTypeFilter = (MimeFilter::TypeMimeFilter)mime;
}

void ItemFilterSettings::setGeolocationFilter(const GeolocationCondition& condition)
{
    m_geolocationCondition = condition;
}

void ItemFilterSettings::setTextFilter(const SearchTextFilterSettings& settings)
{
    m_textFilterSettings = settings;
}

void ItemFilterSettings::setTagNames(const QHash<int, QString>& hash)
{
    m_tagNameHash = hash;
}

void ItemFilterSettings::setAlbumNames(const QHash<int, QString>& hash)
{
    m_albumNameHash = hash;
}

void ItemFilterSettings::setUrlWhitelist(const QList<QUrl>& urlList, const QString& id)
{
    if (urlList.isEmpty())
    {
        m_urlWhitelists.remove(id);
    }
    else
    {
        m_urlWhitelists.insert(id, urlList);
    }
}

void ItemFilterSettings::setIdWhitelist(const QList<qlonglong>& idList, const QString& id)
{
    if (idList.isEmpty())
    {
        m_idWhitelists.remove(id);
    }
    else
    {
        m_idWhitelists.insert(id, idList);
    }
}

template <class ContainerA, class ContainerB>
bool containsAnyOf(const ContainerA& listA, const ContainerB& listB)
{
    foreach (const typename ContainerA::value_type& a, listA)
    {
        if (listB.contains(a))
        {
            return true;
        }
    }
    return false;
}

template <class ContainerA, typename Value, class ContainerB>
bool containsNoneOfExcept(const ContainerA& list, const ContainerB& noneOfList, const Value& exception)
{
    foreach (const typename ContainerB::value_type& n, noneOfList)
    {
        if (n != exception && list.contains(n))
        {
            return false;
        }
    }
    return true;
}

bool ItemFilterSettings::matches(const ItemInfo& info, bool* const foundText) const
{
    if (foundText)
    {
        *foundText = false;
    }

    if (!isFilteringInternally())
    {
        return true;
    }

    bool match = false;

    if (!m_includeTagFilter.isEmpty() || !m_excludeTagFilter.isEmpty())
    {
        QList<int>                 tagIds = info.tagIds();
        QList<int>::const_iterator it;

        match = m_includeTagFilter.isEmpty();

        if (m_matchingCond == OrCondition)
        {
            for (it = m_includeTagFilter.begin() ; it != m_includeTagFilter.end() ; ++it)
            {
                if (tagIds.contains(*it))
                {
                    match = true;
                    break;
                }
            }

            match |= (m_untaggedFilter && tagIds.isEmpty());
        }
        else // AND matching condition...
        {
            // m_untaggedFilter and non-empty tag filter, combined with AND, is logically no match
            if (!m_untaggedFilter)
            {
                for (it = m_includeTagFilter.begin(); it != m_includeTagFilter.end(); ++it)
                {
                    if (!tagIds.contains(*it))
                    {
                        break;
                    }
                }

                if (it == m_includeTagFilter.end())
                {
                    match = true;
                }
            }
        }

        for (it = m_excludeTagFilter.begin() ; it != m_excludeTagFilter.end() ; ++it)
        {
            if (tagIds.contains(*it))
            {
                match = false;
                break;
            }
        }
    }
    else if (m_untaggedFilter)
    {
        match = !TagsCache::instance()->containsPublicTags(info.tagIds());
    }
    else
    {
        match = true;
    }

    //-- Filter by pick labels ------------------------------------------------

    if (!m_pickLabelTagFilter.isEmpty())
    {
        QList<int> tagIds = info.tagIds();
        bool matchPL      = false;

        if (containsAnyOf(m_pickLabelTagFilter, tagIds))
        {
            matchPL = true;
        }
        else
        {
            int noPickLabelTagId = TagsCache::instance()->tagForPickLabel(NoPickLabel);

            if (m_pickLabelTagFilter.contains(noPickLabelTagId))
            {
                // Searching for "has no ColorLabel" requires special handling:
                // Scan that the tag ids contains none of the ColorLabel tags, except maybe the NoColorLabel tag
                matchPL = containsNoneOfExcept(tagIds, TagsCache::instance()->pickLabelTags(), noPickLabelTagId);
            }
        }

        match &= matchPL;
    }

    //-- Filter by color labels ------------------------------------------------

    if (!m_colorLabelTagFilter.isEmpty())
    {
        QList<int> tagIds = info.tagIds();
        bool matchCL      = false;

        if (containsAnyOf(m_colorLabelTagFilter, tagIds))
        {
            matchCL = true;
        }
        else
        {
            int noColorLabelTagId = TagsCache::instance()->tagForColorLabel(NoColorLabel);

            if (m_colorLabelTagFilter.contains(noColorLabelTagId))
            {
                // Searching for "has no ColorLabel" requires special handling:
                // Scan that the tag ids contains none of the ColorLabel tags, except maybe the NoColorLabel tag
                matchCL = containsNoneOfExcept(tagIds, TagsCache::instance()->colorLabelTags(), noColorLabelTagId);
            }
        }

        match &= matchCL;
    }

    //-- Filter by date -----------------------------------------------------------

    if (!m_dayFilter.isEmpty())
    {
        match &= m_dayFilter.contains(QDateTime(info.dateTime().date(), QTime()));
    }

    //-- Filter by rating ---------------------------------------------------------

    if (m_ratingFilter >= 0)
    {
        // for now we treat -1 (no rating) just like a rating of 0.
        int rating = info.rating();

        if (rating == -1)
        {
            rating = 0;
        }

        if (m_isUnratedExcluded && rating == 0)
        {
            match = false;
        }
        else
        {
            if (m_ratingCond == GreaterEqualCondition)
            {
                // If the rating is not >=, i.e it is <, then it does not match.
                if (rating < m_ratingFilter)
                {
                    match = false;
                }
            }
            else if (m_ratingCond == EqualCondition)
            {
                // If the rating is not =, i.e it is !=, then it does not match.
                if (rating != m_ratingFilter)
                {
                    match = false;
                }
            }
            else
            {
                // If the rating is not <=, i.e it is >, then it does not match.
                if (rating > m_ratingFilter)
                {
                    match = false;
                }
            }
        }
    }

    // -- Filter by mime type -----------------------------------------------------

    switch (m_mimeTypeFilter)
    {
        // info.format is a standardized string: Only one possibility per mime type
        case MimeFilter::ImageFiles:
        {
            if (info.category() != DatabaseItem::Image)
            {
                match = false;
            }

            break;
        }
        case MimeFilter::JPGFiles:
        {
            if (info.format() != QLatin1String("JPG"))
            {
                match = false;
            }

            break;
        }
        case MimeFilter::PNGFiles:
        {
            if (info.format() != QLatin1String("PNG"))
            {
                match = false;
            }

            break;
        }
        case MimeFilter::TIFFiles:
        {
            if (info.format() != QLatin1String("TIFF"))
            {
                match = false;
            }

            break;
        }
        case MimeFilter::DNGFiles:
        {
            if (info.format() != QLatin1String("RAW-DNG"))
            {
                match = false;
            }

            break;
        }
        case MimeFilter::NoRAWFiles:
        {
            if (info.format().startsWith(QLatin1String("RAW")))
            {
                match = false;
            }

            break;
        }
        case MimeFilter::RAWFiles:
        {
            if (!info.format().startsWith(QLatin1String("RAW")))
            {
                match = false;
            }

            break;
        }
        case MimeFilter::MoviesFiles:
        {
            if (info.category() != DatabaseItem::Video)
            {
                match = false;
            }

            break;
        }
        case MimeFilter::AudioFiles:
        {
            if (info.category() != DatabaseItem::Audio)
            {
                match = false;
            }

            break;
        }
        case MimeFilter::RasterGraphics:
        {
            if (info.format() != QLatin1String("PSD") &&         // Adobe Photoshop Document
                info.format() != QLatin1String("PSB") &&         // Adobe Photoshop Big
                info.format() != QLatin1String("XCF") &&         // Gimp
                info.format() != QLatin1String("KRA") &&         // Krita
                info.format() != QLatin1String("ORA")            // Open Raster
               )
            {
                match = false;
            }

            break;
        }
        default:
        {
            // All Files: do nothing...
            break;
        }
    }

    //-- Filter by geolocation ----------------------------------------------------

    if (m_geolocationCondition != GeolocationNoFilter)
    {
        if (m_geolocationCondition == GeolocationNoCoordinates)
        {
            if (info.hasCoordinates())
            {
                match = false;
            }
        }
        else if (m_geolocationCondition == GeolocationHasCoordinates)
        {
            if (!info.hasCoordinates())
            {
                match = false;
            }
        }
    }

    //-- Filter by text -----------------------------------------------------------

    if (!m_textFilterSettings.text.isEmpty())
    {
        bool textMatch = false;

        // Image name
        if (m_textFilterSettings.textFields & SearchTextFilterSettings::ImageName &&
            info.name().contains(m_textFilterSettings.text, m_textFilterSettings.caseSensitive))
        {
            textMatch = true;
        }

        // Image title
        if (m_textFilterSettings.textFields & SearchTextFilterSettings::ImageTitle &&
            info.title().contains(m_textFilterSettings.text, m_textFilterSettings.caseSensitive))
        {
            textMatch = true;
        }

        // Image comment
        if (m_textFilterSettings.textFields & SearchTextFilterSettings::ImageComment &&
            info.comment().contains(m_textFilterSettings.text, m_textFilterSettings.caseSensitive))
        {
            textMatch = true;
        }

        // Tag names
        foreach(int id, info.tagIds())
        {
            if (m_textFilterSettings.textFields & SearchTextFilterSettings::TagName &&
                m_tagNameHash.value(id).contains(m_textFilterSettings.text, m_textFilterSettings.caseSensitive))
            {
                textMatch = true;
            }
        }

        // Album names
        if (m_textFilterSettings.textFields & SearchTextFilterSettings::AlbumName &&
            m_albumNameHash.value(info.albumId()).contains(m_textFilterSettings.text, m_textFilterSettings.caseSensitive))
        {
            textMatch = true;
        }

        // Image Aspect Ratio
        if (m_textFilterSettings.textFields & SearchTextFilterSettings::ImageAspectRatio)
        {
            QRegExp expRatio (QLatin1String("^\\d+:\\d+$"));
            QRegExp expFloat (QLatin1String("^\\d+(.\\d+)?$"));

            if (expRatio.indexIn(m_textFilterSettings.text) > -1 && m_textFilterSettings.text.contains(QRegExp(QLatin1String(":\\d+"))))
            {
                QString trimmedTextFilterSettingsText = m_textFilterSettings.text;
                QStringList numberStringList          = trimmedTextFilterSettingsText.split(QLatin1Char(':'), QString::SkipEmptyParts);

                if (numberStringList.length() == 2)
                {
                    QString numString     = (QString)numberStringList.at(0), denomString = (QString)numberStringList.at(1);
                    bool canConverseNum   = false;
                    bool canConverseDenom = false;
                    int num               = numString.toInt(&canConverseNum, 10), denom = denomString.toInt(&canConverseDenom, 10);

                    if (canConverseNum && canConverseDenom)
                    {
                        if (fabs(info.aspectRatio() - (double)num / denom) < 0.1)
                            textMatch = true;
                    }
                }
            }
            else if (expFloat.indexIn(m_textFilterSettings.text) > -1)
            {
                QString trimmedTextFilterSettingsText = m_textFilterSettings.text;
                bool    canConverse                   = false;
                double  ratio                         = trimmedTextFilterSettingsText.toDouble(&canConverse);

                if (canConverse)
                {
                    if (fabs(info.aspectRatio() - ratio) < 0.1)
                        textMatch = true;
                }
            }
        }

        // Image Pixel Size
        // See bug #341053 for details.

        if (m_textFilterSettings.textFields & SearchTextFilterSettings::ImagePixelSize)
        {
            QSize size    = info.dimensions();
            int pixelSize = size.height()*size.width();
            QString text  = m_textFilterSettings.text;

            if (text.contains(QRegExp(QLatin1String("^>\\d{1,15}$"))) && pixelSize > (text.remove(0, 1)).toInt())
            {
                textMatch = true;
            }
            else if (text.contains(QRegExp(QLatin1String("^<\\d{1,15}$"))) && pixelSize < (text.remove(0, 1)).toInt())
            {
                textMatch = true;
            }
            else if (text.contains(QRegExp(QLatin1String("^\\d+$"))) && pixelSize == text.toInt())
            {
                textMatch = true;
            }
        }

        match &= textMatch;

        if (foundText)
        {
            *foundText = textMatch;
        }
    }

    // -- filter by URL-whitelists ------------------------------------------------
    // NOTE: whitelists are always AND for now.

    if (match)
    {
        const QUrl url = info.fileUrl();

        for (QHash<QString, QList<QUrl>>::const_iterator it = m_urlWhitelists.constBegin();
             it != m_urlWhitelists.constEnd() ; ++it)
        {
            match = it->contains(url);

            if (!match)
            {
                break;
            }
        }
    }

    if (match)
    {
        const qlonglong id = info.id();

        for (QHash<QString, QList<qlonglong> >::const_iterator it = m_idWhitelists.constBegin();
             it != m_idWhitelists.constEnd() ; ++it)
        {
            match = it->contains(id);

            if (!match)
            {
                break;
            }
        }
    }

    return match;
}

// -------------------------------------------------------------------------------------------------

VersionItemFilterSettings::VersionItemFilterSettings()
{
    m_includeTagFilter   = 0;
    m_exceptionTagFilter = 0;
}

VersionItemFilterSettings::VersionItemFilterSettings(const VersionManagerSettings& settings)
{
    setVersionManagerSettings(settings);
}

bool VersionItemFilterSettings::operator==(const VersionItemFilterSettings& other) const
{
    return m_excludeTagFilter == other.m_excludeTagFilter &&
           m_exceptionLists   == other.m_exceptionLists;
}

bool VersionItemFilterSettings::matches(const ItemInfo& info) const
{
    if (!isFiltering())
    {
        return true;
    }

    const qlonglong id = info.id();

    for (QHash<QString, QList<qlonglong> >::const_iterator it = m_exceptionLists.constBegin();
         it != m_exceptionLists.constEnd() ; ++it)
    {
        if (it->contains(id))
        {
            return true;
        }
    }

    bool match        = true;
    QList<int> tagIds = info.tagIds();

    if (!tagIds.contains(m_includeTagFilter))
    {
        for (QList<int>::const_iterator it = m_excludeTagFilter.begin();
             it != m_excludeTagFilter.end() ; ++it)
        {
            if (tagIds.contains(*it))
            {
                match = false;
                break;
            }
        }
    }

    if (!match)
    {
        if (tagIds.contains(m_exceptionTagFilter))
        {
            match = true;
        }
    }

    return match;
}

bool VersionItemFilterSettings::isHiddenBySettings(const ItemInfo& info) const
{
    QList<int> tagIds = info.tagIds();

    foreach(int tagId, m_excludeTagFilter)
    {
        if (tagIds.contains(tagId))
        {
            return true;
        }
    }

    return false;
}

bool VersionItemFilterSettings::isExemptedBySettings(const ItemInfo& info) const
{
    return info.tagIds().contains(m_exceptionTagFilter);
}

void VersionItemFilterSettings::setVersionManagerSettings(const VersionManagerSettings& settings)
{
    m_excludeTagFilter.clear();

    if (!settings.enabled)
    {
        return;
    }

    if (!(settings.showInViewFlags & VersionManagerSettings::ShowOriginal))
    {
        m_excludeTagFilter << TagsCache::instance()->getOrCreateInternalTag(InternalTagName::originalVersion());
    }

    if (!(settings.showInViewFlags & VersionManagerSettings::ShowIntermediates))
    {
        m_excludeTagFilter << TagsCache::instance()->getOrCreateInternalTag(InternalTagName::intermediateVersion());
    }

    m_includeTagFilter   = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::currentVersion());
    m_exceptionTagFilter = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::versionAlwaysVisible());
}

void VersionItemFilterSettings::setExceptionList(const QList<qlonglong>& idList, const QString& id)
{
    if (idList.isEmpty())
    {
        m_exceptionLists.remove(id);
    }
    else
    {
        m_exceptionLists.insert(id, idList);
    }
}

bool VersionItemFilterSettings::isFiltering() const
{
    return !m_excludeTagFilter.isEmpty();
}

bool VersionItemFilterSettings::isFilteringByTags() const
{
    return isFiltering();
}

// -------------------------------------------------------------------------------------------------

GroupItemFilterSettings::GroupItemFilterSettings()
    : m_allOpen(false)
{
}

bool GroupItemFilterSettings::operator==(const GroupItemFilterSettings& other) const
{
    return (m_allOpen    == other.m_allOpen &&
            m_openGroups == other.m_openGroups);
}

bool GroupItemFilterSettings::matches(const ItemInfo& info) const
{
    if (m_allOpen)
    {
        return true;
    }

    if (info.isGrouped())
    {
        return m_openGroups.contains(info.groupImage().id());
    }

    return true;
}

void GroupItemFilterSettings::setOpen(qlonglong group, bool open)
{
    if (open)
    {
        m_openGroups << group;
    }
    else
    {
        m_openGroups.remove(group);
    }
}

bool GroupItemFilterSettings::isOpen(qlonglong group) const
{
    return m_openGroups.contains(group);
}

void GroupItemFilterSettings::setAllOpen(bool open)
{
    m_allOpen = open;
}

bool GroupItemFilterSettings::isAllOpen() const
{
    return m_allOpen;
}

bool GroupItemFilterSettings::isFiltering() const
{
    return !m_allOpen;
}

DatabaseFields::Set GroupItemFilterSettings::watchFlags() const
{
    return DatabaseFields::Set(DatabaseFields::ImageRelations);
}

} // namespace Digikam
