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

#include "imagesortsettings.h"

// Qt includes

#include <QDateTime>

// KDE includes

#include <kdebug.h>

// Local includes

#include "imageinfo.h"

namespace Digikam
{

ImageSortSettings::ImageSortSettings()
{
    categorizationMode = NoCategories;
    categorizationSortOrder = DefaultOrder;
    categorizationCaseSensitivity = Qt::CaseSensitive;
    sortRole = SortByFileName;
    sortOrder = DefaultOrder;
    sortCaseSensitivity = Qt::CaseSensitive;
}

bool ImageSortSettings::operator==(const ImageSortSettings &other) const
{
    return
        categorizationMode == other.categorizationMode &&
        categorizationSortOrder == other.categorizationSortOrder &&
        categorizationCaseSensitivity == other.categorizationCaseSensitivity &&
        sortRole == other.sortRole &&
        sortOrder == other.sortOrder &&
        sortCaseSensitivity == other.sortCaseSensitivity;
}

void ImageSortSettings::setCategorizationMode(CategorizationMode mode)
{
    categorizationMode = mode;
    if (categorizationSortOrder == DefaultOrder)
        currentCategorizationSortOrder = defaultSortOrderForCategorizationMode(categorizationMode);
}

void ImageSortSettings::setCategorizationSortOrder(SortOrder order)
{
    categorizationSortOrder = order;
    if (categorizationSortOrder == DefaultOrder)
        currentCategorizationSortOrder = defaultSortOrderForCategorizationMode(categorizationMode);
    else
        currentCategorizationSortOrder = (Qt::SortOrder)categorizationSortOrder;
}

void ImageSortSettings::setSortRole(SortRole role)
{
    sortRole = role;
    if (sortOrder == DefaultOrder)
        currentSortOrder = defaultSortOrderForSortRole(sortRole);
}

void ImageSortSettings::setSortOrder(SortOrder order)
{
    sortOrder = order;
    if (sortOrder == DefaultOrder)
        currentSortOrder = defaultSortOrderForSortRole(sortRole);
    else
        currentSortOrder = (Qt::SortOrder)order;
}

Qt::SortOrder ImageSortSettings::defaultSortOrderForCategorizationMode(CategorizationMode mode)
{
    switch (mode)
    {
        case NoCategories:
        case OneCategory:
        case CategoryByAlbum:
        case CategoryByFormat:
        default:
            return Qt::AscendingOrder;
    }
}

Qt::SortOrder ImageSortSettings::defaultSortOrderForSortRole(SortRole role)
{
    switch (role)
    {
        case SortByFileName:
        case SortByFilePath:
            return Qt::Ascending;
        case SortByFileSize:
            return Qt::Descending;
        case SortByModificationDate:
        case SortByCreationDate:
            return Qt::Ascending;
        case SortByRating:
        case SortByImageSize:
            return Qt::Descending;
        default:
            return Qt::Ascending;
    }
}

int ImageSortSettings::compareCategories(const ImageInfo& left, const ImageInfo& right) const
{
    switch (categorizationMode)
    {
        case NoCategories:
        case OneCategory:
            return 0;
        case CategoryByAlbum:
        {
            int leftAlbum = left.albumId();
            int rightAlbum = right.albumId();

            // return comparation result
            if (leftAlbum == rightAlbum)
                return 0;
            else if (lessThan(leftAlbum, rightAlbum, currentCategorizationSortOrder))
                return -1;
            else
                return 1;
        }
        case CategoryByFormat:
        {
            return naturalCompare(left.format(), right.format(),
                                  currentCategorizationSortOrder, categorizationCaseSensitivity);
        }
        default:
            return 0;
    }
}

bool ImageSortSettings::lessThan(const ImageInfo& left, const ImageInfo& right) const
{
    switch (sortRole)
    {
        case SortByFileName:
            return naturalCompare(left.name(), right.name(), currentSortOrder, sortCaseSensitivity) < 0;
        case SortByFilePath:
            return naturalCompare(left.filePath(), right.filePath(), currentSortOrder, sortCaseSensitivity) < 0;
        case SortByFileSize:
            return lessThan(left.fileSize(), right.fileSize(), currentSortOrder);
        case SortByModificationDate:
            return lessThan(left.modDateTime(), right.modDateTime(), currentSortOrder);
        case SortByCreationDate:
            return lessThan(left.dateTime(), right.dateTime(), currentSortOrder);
        case SortByRating:
            return lessThan(left.rating(), right.rating(), currentSortOrder);
        case SortByImageSize:
        {
            QSize leftSize = left.dimensions();
            QSize rightSize = right.dimensions();
            return lessThan(leftSize.width() * leftSize.height(), rightSize.width() * rightSize.height(), currentSortOrder);
        }
        default:
            return false;
    }
}



} // namespace Digikam
