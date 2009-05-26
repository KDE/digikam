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

#include "imagefiltersettings.h"

// Qt includes

#include <QDateTime>

// Local includes

#include "imageinfo.h"

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
        dayFilter.insert(*it, true);
}

bool ImageFilterSettings::isFilteringByTags() const
{
    if (!tagFilter.isEmpty() || untaggedFilter)
        return true;

    return false;
}

bool ImageFilterSettings::isFilteringByText() const
{
    if (!textFilterSettings.text.isEmpty())
        return true;

    return false;
}

bool ImageFilterSettings::isFiltering() const
{
    return !dayFilter.isEmpty() || !tagFilter.isEmpty() || !textFilterSettings.text.isEmpty()
            || untaggedFilter || ratingFilter!=-1;
}

void ImageFilterSettings::setTagFilter(const QList<int>& tags, MatchingCondition matchingCond,
                                       bool showUnTagged)
{
    tagFilter      = tags;
    matchingCond   = matchingCond;
    untaggedFilter = showUnTagged;
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

bool ImageFilterSettings::matches(const ImageInfo& info, bool *foundText) const
{
    if (foundText)
        *foundText = false;

    if (!isFiltering())
        return true;

    bool match = false;

    if (!tagFilter.isEmpty())
    {
        QList<int> tagIds = info.tagIds();
        QList<int>::const_iterator it;

        if (matchingCond == OrCondition)
        {
            for (it = tagFilter.begin(); it != tagFilter.end(); ++it)
            {
                if (tagIds.contains(*it))
                {
                    match = true;
                    break;
                }
            }
        }
        else
        {
            // AND matching condition...

            for (it = tagFilter.begin(); it != tagFilter.end(); ++it)
            {
                if (!tagIds.contains(*it))
                    break;
            }

            if (it == tagFilter.end())
                match = true;
        }

        match |= (untaggedFilter && tagIds.isEmpty());
    }
    else if (untaggedFilter)
    {
        match = info.tagIds().isEmpty();
    }
    else
    {
        match = true;
    }

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
            rating = 0;

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

    switch(mimeTypeFilter)
    {
        // info.format is a standardized string: Only one possibility per mime type
        case MimeFilter::ImageFiles:
        {
            if (info.category() != DatabaseItem::Image)
                match = false;
            break;
        }
        case MimeFilter::JPGFiles:
        {
            if (info.format() != "JPG")
                match = false;
            break;
        }
        case MimeFilter::PNGFiles:
        {
            if (info.format() != "PNG")
                match = false;
            break;
        }
        case MimeFilter::TIFFiles:
        {
            if (info.format() != "TIFF")
                match = false;
            break;
        }
        case MimeFilter::DNGFiles:
        {
            if (info.format() != "RAW-DNG")
                match = false;
            break;
        }
        case MimeFilter::NoRAWFiles:
        {
            if (info.format().startsWith("RAW"))
                match = false;
            break;
        }
        case MimeFilter::RAWFiles:
        {
            if (!info.format().startsWith("RAW"))
                match = false;
            break;
        }
        case MimeFilter::MoviesFiles:
        {
            if (info.category() != DatabaseItem::Video)
                match = false;
            break;
        }
        case MimeFilter::AudioFiles:
        {
            if (info.category() != DatabaseItem::Audio)
                match = false;
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
            textMatch = true;

        // Tag names
        foreach (int id, info.tagIds())
        {
            if (tagNameHash.value(id).contains(textFilterSettings.text, textFilterSettings.caseSensitive))
                textMatch = true;
        }

        // Album names
        if (albumNameHash.value(info.albumId()).contains(textFilterSettings.text, textFilterSettings.caseSensitive))
            textMatch = true;

        match &= textMatch;
        if (foundText)
            *foundText = textMatch;
    }

    return match;
}

} // namespace Digikam
