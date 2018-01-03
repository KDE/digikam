/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Filter values for use with ImageFilterModel
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2014 by Mohamed Anwer <m dot anwer at gmx dot com>
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
#include <QRectF>

// Local includes

#include "coredbfields.h"
#include "imageinfo.h"

namespace Digikam
{

ImageSortSettings::ImageSortSettings()
{
    categorizationMode             = NoCategories;
    categorizationSortOrder        = DefaultOrder;
    categorizationCaseSensitivity  = Qt::CaseSensitive;
    sortRole                       = SortByFileName;
    sortOrder                      = DefaultOrder;
    strTypeNatural                 = true;
    sortCaseSensitivity            = Qt::CaseSensitive;
    currentCategorizationSortOrder = Qt::AscendingOrder;
    currentSortOrder               = Qt::AscendingOrder;
}

bool ImageSortSettings::operator==(const ImageSortSettings& other) const
{
    return
        categorizationMode            == other.categorizationMode            &&
        categorizationSortOrder       == other.categorizationSortOrder       &&
        categorizationCaseSensitivity == other.categorizationCaseSensitivity &&
        sortRole                      == other.sortRole                      &&
        sortOrder                     == other.sortOrder                     &&
        sortCaseSensitivity           == other.sortCaseSensitivity;
}

void ImageSortSettings::setCategorizationMode(CategorizationMode mode)
{
    categorizationMode = mode;

    if (categorizationSortOrder == DefaultOrder)
    {
        currentCategorizationSortOrder = defaultSortOrderForCategorizationMode(categorizationMode);
    }
}

void ImageSortSettings::setCategorizationSortOrder(SortOrder order)
{
    categorizationSortOrder = order;

    if (categorizationSortOrder == DefaultOrder)
    {
        currentCategorizationSortOrder = defaultSortOrderForCategorizationMode(categorizationMode);
    }
    else
    {
        currentCategorizationSortOrder = (Qt::SortOrder)categorizationSortOrder;
    }
}

void ImageSortSettings::setSortRole(SortRole role)
{
    sortRole = role;

    if (sortOrder == DefaultOrder)
    {
        currentSortOrder = defaultSortOrderForSortRole(sortRole);
    }
}

void ImageSortSettings::setSortOrder(SortOrder order)
{
    sortOrder = order;

    if (sortOrder == DefaultOrder)
    {
        currentSortOrder = defaultSortOrderForSortRole(sortRole);
    }
    else
    {
        currentSortOrder = (Qt::SortOrder)order;
    }
}

void ImageSortSettings::setStringTypeNatural(bool natural)
{
    strTypeNatural = natural;
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
            return Qt::AscendingOrder;
        case SortByFileSize:
            return Qt::DescendingOrder;
        case SortByModificationDate:
        case SortByCreationDate:
            return Qt::AscendingOrder;
        case SortByRating:
        case SortByImageSize:
            return Qt::DescendingOrder;
        case SortByAspectRatio:
            return Qt::DescendingOrder;
        case SortBySimilarity:
            return Qt::DescendingOrder;
        default:
            return Qt::AscendingOrder;
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
            {
                return 0;
            }
            else if (lessThanByOrder(leftAlbum, rightAlbum, currentCategorizationSortOrder))
            {
                return -1;
            }
            else
            {
                return 1;
            }
        }
        case CategoryByFormat:
        {
            return naturalCompare(left.format(), right.format(),
                                  currentCategorizationSortOrder, categorizationCaseSensitivity, strTypeNatural);
        }
        default:
            return 0;
    }
}

bool ImageSortSettings::lessThan(const ImageInfo& left, const ImageInfo& right) const
{
    int result = compare(left, right, sortRole);

    if (result != 0)
    {
        return result < 0;
    }

    // are they identical?
    if (left == right)
    {
        return false;
    }

    // If left and right equal for first sort order, use a hierarchy of all sort orders
    if ( (result = compare(left, right, SortByFileName)) != 0)
    {
        return result < 0;
    }

    if ( (result = compare(left, right, SortByCreationDate)) != 0)
    {
        return result < 0;
    }

    if ( (result = compare(left, right, SortByModificationDate)) != 0)
    {
        return result < 0;
    }

    if ( (result = compare(left, right, SortByFilePath)) != 0)
    {
        return result < 0;
    }

    if ( (result = compare(left, right, SortByFileSize)) != 0)
    {
        return result < 0;
    }

    if ( (result = compare(left, right, SortBySimilarity)) != 0)
    {
        return result < 0;
    }

    return false;
}

int ImageSortSettings::compare(const ImageInfo& left, const ImageInfo& right) const
{
    return compare(left, right, sortRole);
}

int ImageSortSettings::compare(const ImageInfo& left, const ImageInfo& right, SortRole role) const
{
    switch (role)
    {
        case SortByFileName:
        {
            bool versioning = (left.name().contains(QLatin1String("_v"), Qt::CaseInsensitive) ||
                               right.name().contains(QLatin1String("_v"), Qt::CaseInsensitive));
            return naturalCompare(left.name(), right.name(), currentSortOrder, sortCaseSensitivity, strTypeNatural, versioning);
        }
        case SortByFilePath:
            return naturalCompare(left.filePath(), right.filePath(), currentSortOrder, sortCaseSensitivity, strTypeNatural);
        case SortByFileSize:
            return compareByOrder(left.fileSize(), right.fileSize(), currentSortOrder);
        case SortByModificationDate:
            return compareByOrder(left.modDateTime(), right.modDateTime(), currentSortOrder);
        case SortByCreationDate:
            return compareByOrder(left.dateTime(), right.dateTime(), currentSortOrder);
        case SortByRating:
            // I have the feeling that inverting the sort order for rating is the natural order
            return - compareByOrder(left.rating(), right.rating(), currentSortOrder);
        case SortByImageSize:
        {
            QSize leftSize  = left.dimensions();
            QSize rightSize = right.dimensions();
            int leftPixels  = leftSize.width() * leftSize.height();
            int rightPixels = rightSize.width() * rightSize.height();
            return compareByOrder(leftPixels, rightPixels, currentSortOrder);
        }
        case SortByAspectRatio:
        {
            QSize leftSize = left.dimensions();
            QSize rightSize = right.dimensions();
            int leftAR = (double(leftSize.width()) / double(leftSize.height())) * 1000000;
            int rightAR = (double(rightSize.width()) / double(rightSize.height())) * 1000000;
            return compareByOrder(leftAR, rightAR, currentSortOrder);
        }
        case SortBySimilarity:
        {
            qlonglong leftReferenceImageId  = left.currentReferenceImage();
            qlonglong rightReferenceImageId = right.currentReferenceImage();
            // make sure that the original image has always the highest similarity.
            double leftSimilarity  = left.id() == leftReferenceImageId ? 1.1 : left.currentSimilarity();
            double rightSimilarity = right.id() == rightReferenceImageId ? 1.1 : right.currentSimilarity();
            return compareByOrder(leftSimilarity, rightSimilarity, currentSortOrder);
        }
        default:
            return 1;
    }
}

bool ImageSortSettings::lessThan(const QVariant& left, const QVariant& right) const
{
    if (left.type() != right.type())
    {
        return false;
    }

    switch (left.type())
    {
        case QVariant::Int:
            return compareByOrder(left.toInt(), right.toInt(), currentSortOrder);
        case QVariant::UInt:
            return compareByOrder(left.toUInt(), right.toUInt(), currentSortOrder);
        case QVariant::LongLong:
            return compareByOrder(left.toLongLong(), right.toLongLong(), currentSortOrder);
        case QVariant::ULongLong:
            return compareByOrder(left.toULongLong(), right.toULongLong(), currentSortOrder);
        case QVariant::Double:
            return compareByOrder(left.toDouble(), right.toDouble(), currentSortOrder);
        case QVariant::Date:
            return compareByOrder(left.toDate(), right.toDate(), currentSortOrder);
        case QVariant::DateTime:
            return compareByOrder(left.toDateTime(), right.toDateTime(), currentSortOrder);
        case QVariant::Time:
            return compareByOrder(left.toTime(), right.toTime(), currentSortOrder);
        case QVariant::Rect:
        case QVariant::RectF:
        {
            QRectF rectLeft  = left.toRectF();
            QRectF rectRight = right.toRectF();
            int result;

            if ((result = compareByOrder(rectLeft.top(), rectRight.top(), currentSortOrder)) != 0)
            {
                return result < 0;
            }

            if ((result = compareByOrder(rectLeft.left(), rectRight.left(), currentSortOrder)) != 0)
            {
                return result < 0;
            }

            QSizeF sizeLeft  = rectLeft.size();
            QSizeF sizeRight = rectRight.size();

            if ((result = compareByOrder(sizeLeft.width()*sizeLeft.height(), sizeRight.width()*sizeRight.height(), currentSortOrder)) != 0)
            {
                return result < 0;
            }

            [[fallthrough]];
        }
        default:
            return naturalCompare(left.toString(), right.toString(), currentSortOrder, sortCaseSensitivity, strTypeNatural);
    }
}

DatabaseFields::Set ImageSortSettings::watchFlags() const
{
    DatabaseFields::Set set;

    switch (sortRole)
    {
        case SortByFileName:
            set |= DatabaseFields::Name;
            break;
        case SortByFilePath:
            set |= DatabaseFields::Name;
            break;
        case SortByFileSize:
            set |= DatabaseFields::FileSize;
            break;
        case SortByModificationDate:
            set |= DatabaseFields::ModificationDate;
            break;
        case SortByCreationDate:
            set |= DatabaseFields::CreationDate;
            break;
        case SortByRating:
            set |= DatabaseFields::Rating;
            break;
        case SortByImageSize:
            set |= DatabaseFields::Width | DatabaseFields::Height;
            break;
        case SortByAspectRatio:
            set |= DatabaseFields::Width | DatabaseFields::Height;
            break;
        case SortBySimilarity:
            // TODO: Not sure what to do here....
            set |= DatabaseFields::Name;
            break;
    }

    switch (categorizationMode)
    {
        case NoCategories:
        case OneCategory:
        case CategoryByAlbum:
            break;
        case CategoryByFormat:
            set |= DatabaseFields::Format;
            break;
    }

    return set;
}

} // namespace Digikam
