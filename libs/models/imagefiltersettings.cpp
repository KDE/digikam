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

#include "imagefiltersettings.h"

// Qt includes

#include <QDateTime>

// KDE includes

#include <kdebug.h>

// Local includes

#include "databasefields.h"
#include "imageinfo.h"
#include "tagscache.h"
#include "versionmanagersettings.h"

namespace Digikam
{

ImageFilterSettings::ImageFilterSettings()
{
    untaggedFilter = false;
    ratingFilter   = 0;
    mimeTypeFilter = MimeFilter::AllFiles;
    ratingCond     = ImageFilterSettings::GreaterEqualCondition;
    matchingCond   = ImageFilterSettings::OrCondition;
}

void ImageFilterSettings::setDayFilter(const QList<QDateTime>& days)
{
    dayFilter.clear();

    for (QList<QDateTime>::const_iterator it = days.constBegin(); it != days.constEnd(); ++it)
    {
        dayFilter.insert(*it, true);
    }
}

bool ImageFilterSettings::isFilteringByTags() const
{
    if (!includeTagFilter.isEmpty() || !excludeTagFilter.isEmpty() || untaggedFilter)
    {
        return true;
    }

    return false;
}

bool ImageFilterSettings::isFilteringByColorLabels() const
{
    if (!colorLabelTagFilter.isEmpty())
    {
        return true;
    }

    return false;
}

bool ImageFilterSettings::isFilteringByText() const
{
    if (!textFilterSettings.text.isEmpty())
    {
        return true;
    }

    return false;
}

bool ImageFilterSettings::isFiltering() const
{
    return !dayFilter.isEmpty() || !includeTagFilter.isEmpty() || !excludeTagFilter.isEmpty() ||
           !textFilterSettings.text.isEmpty() || untaggedFilter || ratingFilter >= 0 ||
           mimeTypeFilter != MimeFilter::AllFiles || !colorLabelTagFilter.isEmpty();
}

void ImageFilterSettings::setTagFilter(const QList<int>& includedTags, const QList<int>& excludedTags,
                                       MatchingCondition matchingCondition, bool showUnTagged,
                                       const QList<int>& clTagIds)
{
    includeTagFilter    = includedTags;
    excludeTagFilter    = excludedTags;
    matchingCond        = matchingCondition;
    untaggedFilter      = showUnTagged;
    colorLabelTagFilter = clTagIds;
}

void ImageFilterSettings::setRatingFilter(int rating, RatingCondition ratingCondition)
{
    ratingFilter = rating;
    ratingCond   = ratingCondition;
}

void ImageFilterSettings::setMimeTypeFilter(int mime)
{
    mimeTypeFilter = (MimeFilter::TypeMimeFilter)mime;
}

void ImageFilterSettings::setTextFilter(const SearchTextSettings& settings)
{
    textFilterSettings = settings;
}

void ImageFilterSettings::setTagNames(const QHash<int, QString>& hash)
{
    tagNameHash = hash;
}

void ImageFilterSettings::setAlbumNames(const QHash<int, QString>& hash)
{
    albumNameHash = hash;
}

void ImageFilterSettings::setUrlWhitelist(const KUrl::List& urlList, const QString& id)
{
    if (urlList.isEmpty())
    {
        urlWhitelists.remove(id);
    }
    else
    {
        urlWhitelists.insert(id, urlList);
    }
}

void ImageFilterSettings::setIdWhitelist(const QList<qlonglong>& idList, const QString& id)
{
    if (idList.isEmpty())
    {
        idWhitelists.remove(id);
    }
    else
    {
        idWhitelists.insert(id, idList);
    }
}

bool ImageFilterSettings::matches(const ImageInfo& info, bool* foundText) const
{
    if (foundText)
    {
        *foundText = false;
    }

    if (!isFiltering())
    {
        return true;
    }

    bool match = false;

    if (!includeTagFilter.isEmpty() || !excludeTagFilter.isEmpty() || !colorLabelTagFilter.isEmpty())
    {
        QList<int> tagIds = info.tagIds();
        QList<int>::const_iterator it;

        match = includeTagFilter.isEmpty();

        if (matchingCond == OrCondition)
        {
            for (it = includeTagFilter.begin(); it != includeTagFilter.end(); ++it)
            {
                if (tagIds.contains(*it))
                {
                    match = true;
                    break;
                }
            }

            match |= (untaggedFilter && tagIds.isEmpty());
        }
        else // AND matching condition...
        {
            // untaggedFilter and non-empty tag filter, combined with AND, is logically no match
            if (!untaggedFilter)
            {
                for (it = includeTagFilter.begin(); it != includeTagFilter.end(); ++it)
                {
                    if (!tagIds.contains(*it))
                    {
                        break;
                    }
                }

                if (it == includeTagFilter.end())
                {
                    match = true;
                }
            }
        }

        for (it = excludeTagFilter.begin(); it != excludeTagFilter.end(); ++it)
        {
            if (tagIds.contains(*it))
            {
                match = false;
                break;
            }
        }
    }
    else if (untaggedFilter)
    {
        match = info.tagIds().isEmpty();
    }
    else
    {
        match = true;
    }

    //-- Filter by color labels ------------------------------------------------

    if (!colorLabelTagFilter.isEmpty())
    {
        bool matchCL      = false;
        QList<int> tagIds = info.tagIds();

        for (QList<int>::const_iterator it = colorLabelTagFilter.begin(); it != colorLabelTagFilter.end(); ++it)
        {
            if (tagIds.contains(*it))
            {
                matchCL = true;
                break;
            }
        }

        match &= matchCL;
    }

    //-- Filter by date -----------------------------------------------------------

    if (!dayFilter.isEmpty())
    {
        match &= dayFilter.contains(QDateTime(info.dateTime().date(), QTime()));
    }

    //-- Filter by rating ---------------------------------------------------------

    if (ratingFilter >= 0)
    {
        // for now we treat -1 (no rating) just like a rating of 0.
        int rating = info.rating();

        if (rating == -1)
        {
            rating = 0;
        }

        if (ratingCond == GreaterEqualCondition)
        {
            // If the rating is not >=, i.e it is <, then it does not match.
            if (rating < ratingFilter)
            {
                match = false;
            }
        }
        else if (ratingCond == EqualCondition)
        {
            // If the rating is not =, i.e it is !=, then it does not match.
            if (rating != ratingFilter)
            {
                match = false;
            }
        }
        else
        {
            // If the rating is not <=, i.e it is >, then it does not match.
            if (rating > ratingFilter)
            {
                match = false;
            }
        }
    }

    // -- Filter by mime type -----------------------------------------------------

    switch (mimeTypeFilter)
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
            if (info.format() != "JPG")
            {
                match = false;
            }

            break;
        }
        case MimeFilter::PNGFiles:
        {
            if (info.format() != "PNG")
            {
                match = false;
            }

            break;
        }
        case MimeFilter::TIFFiles:
        {
            if (info.format() != "TIFF")
            {
                match = false;
            }

            break;
        }
        case MimeFilter::DNGFiles:
        {
            if (info.format() != "RAW-DNG")
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
        default:        // All Files: do nothing...
            break;
    }

    //-- Filter by text -----------------------------------------------------------

    if (!textFilterSettings.text.isEmpty())
    {
        bool textMatch = false;

        // Image name
        if (info.name().contains(textFilterSettings.text, textFilterSettings.caseSensitive))
        {
            textMatch = true;
        }

        // Image comment
        if (info.comment().contains(textFilterSettings.text, textFilterSettings.caseSensitive))
        {
            textMatch = true;
        }

        // Tag names
        foreach (int id, info.tagIds())
        {
            if (tagNameHash.value(id).contains(textFilterSettings.text, textFilterSettings.caseSensitive))
            {
                textMatch = true;
            }
        }

        // Album names
        if (albumNameHash.value(info.albumId()).contains(textFilterSettings.text, textFilterSettings.caseSensitive))
        {
            textMatch = true;
        }

        match &= textMatch;

        if (foundText)
        {
            *foundText = textMatch;
        }
    }

    // filter by URL-whitelists:
    // whitelists are always AND for now:
    if (match)
    {
        const KUrl url = info.fileUrl();

        for (QHash<QString, KUrl::List>::const_iterator it = urlWhitelists.constBegin(); it!=urlWhitelists.constEnd(); ++it)
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

        for (QHash<QString, QList<qlonglong> >::const_iterator it = idWhitelists.constBegin(); it!=idWhitelists.constEnd(); ++it)
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

DatabaseFields::Set ImageFilterSettings::watchFlags() const
{
    DatabaseFields::Set set;

    if (!dayFilter.isEmpty())
    {
        set |= DatabaseFields::CreationDate;
    }

    if (!textFilterSettings.text.isEmpty())
    {
        set |= DatabaseFields::Name;
        set |= DatabaseFields::Comment;
    }

    if (ratingFilter >= 0)
    {
        set |= DatabaseFields::Rating;
    }

    if (mimeTypeFilter != MimeFilter::AllFiles)
    {
        set |= DatabaseFields::Category;
        set |= DatabaseFields::Format;
    }

    return set;
}

// -----------------

VersionImageFilterSettings::VersionImageFilterSettings()
{
}

VersionImageFilterSettings::VersionImageFilterSettings(const VersionManagerSettings& settings)
{
    setVersionManagerSettings(settings);
}

bool VersionImageFilterSettings::operator==(const VersionImageFilterSettings& other) const
{
    return excludeTagFilter == other.excludeTagFilter
        && exceptionLists == other.exceptionLists;
}

bool VersionImageFilterSettings::matches(const ImageInfo& info) const
{
    if (!isFiltering())
    {
        return true;
    }

    const qlonglong id = info.id();
    for (QHash<QString, QList<qlonglong> >::const_iterator it = exceptionLists.constBegin();
         it != exceptionLists.constEnd(); ++it)
    {
        if (it->contains(id))
        {
            return true;
        }
    }

    bool match = true;

    QList<int> tagIds = info.tagIds();

    if (!tagIds.contains(includeTagFilter))
    {
        for (QList<int>::const_iterator it = excludeTagFilter.begin();
            it != excludeTagFilter.end(); ++it)
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
        if (tagIds.contains(exceptionTagFilter))
        {
            match = true;
        }
    }

    return match;
}

bool VersionImageFilterSettings::isHiddenBySettings(const ImageInfo& info) const
{
    QList<int> tagIds = info.tagIds();
    foreach (int tagId, excludeTagFilter)
    {
        if (tagIds.contains(tagId))
        {
            return true;
        }
    }
    return false;
}

bool VersionImageFilterSettings::isExemptedBySettings(const ImageInfo& info) const
{
    return info.tagIds().contains(exceptionTagFilter);
}

void VersionImageFilterSettings::setVersionManagerSettings(const VersionManagerSettings& settings)
{
    excludeTagFilter.clear();

    if (!(settings.showInViewFlags & VersionManagerSettings::ShowOriginal))
    {
        excludeTagFilter << TagsCache::instance()->getOrCreateInternalTag(InternalTagName::originalVersion());
    }

    if (!(settings.showInViewFlags & VersionManagerSettings::ShowIntermediates))
    {
        excludeTagFilter << TagsCache::instance()->getOrCreateInternalTag(InternalTagName::intermediateVersion());
    }

    includeTagFilter = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::currentVersion());
    exceptionTagFilter = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::versionAlwaysVisible());
}

void VersionImageFilterSettings::setExceptionList(const QList<qlonglong>& idList, const QString& id)
{
    if (idList.isEmpty())
    {
        exceptionLists.remove(id);
    }
    else
    {
        exceptionLists.insert(id, idList);
    }
}

bool VersionImageFilterSettings::isFiltering() const
{
    return !excludeTagFilter.isEmpty();
}

bool VersionImageFilterSettings::isFilteringByTags() const
{
    return isFiltering();
}



} // namespace Digikam
