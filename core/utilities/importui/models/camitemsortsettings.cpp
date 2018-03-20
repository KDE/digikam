/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-23-06
 * Description : Sort settings for camera item infos
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

#include "camitemsortsettings.h"

// Qt includes

#include <QRectF>

namespace Digikam
{

CamItemSortSettings::CamItemSortSettings()
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

CamItemSortSettings::~CamItemSortSettings()
{
}

bool CamItemSortSettings::operator ==(const CamItemSortSettings& other) const
{
    return (categorizationMode            == other.categorizationMode            &&
            categorizationSortOrder       == other.categorizationSortOrder       &&
            categorizationCaseSensitivity == other.categorizationCaseSensitivity &&
            sortRole                      == other.sortRole                      &&
            sortOrder                     == other.sortOrder                     &&
            sortCaseSensitivity           == other.sortCaseSensitivity);
}

void CamItemSortSettings::setCategorizationMode(CategorizationMode mode)
{
    categorizationMode = mode;

    if (categorizationSortOrder == DefaultOrder)
    {
        currentCategorizationSortOrder = defaultSortOrderForCategorizationMode(categorizationMode);
    }
}

void CamItemSortSettings::setCategorizationSortOrder(SortOrder order)
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

void CamItemSortSettings::setSortRole(SortRole role)
{
    sortRole = role;

    if (sortOrder == DefaultOrder)
    {
        currentSortOrder = defaultSortOrderForSortRole(sortRole);
    }
}

void CamItemSortSettings::setSortOrder(SortOrder order)
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

void CamItemSortSettings::setStringTypeNatural(bool natural)
{
    strTypeNatural = natural;
}

Qt::SortOrder CamItemSortSettings::defaultSortOrderForCategorizationMode(CategorizationMode mode)
{
    switch (mode)
    {
        case NoCategories:
        case CategoryByFolder:
        case CategoryByFormat:
        case CategoryByDate:
        default:
            return Qt::AscendingOrder;
    }
}

Qt::SortOrder CamItemSortSettings::defaultSortOrderForSortRole(SortRole role)
{
    switch (role)
    {
        case SortByFileName:
        case SortByFilePath:
            return Qt::AscendingOrder;
        case SortByFileSize:
            return Qt::DescendingOrder;
        case SortByCreationDate:
            return Qt::AscendingOrder;
        case SortByDownloadState:
            return Qt::AscendingOrder;
        case SortByRating:
            return Qt::DescendingOrder;
        default:
            return Qt::AscendingOrder;
    }
}

int CamItemSortSettings::compareCategories(const CamItemInfo& left, const CamItemInfo& right) const
{
    switch (categorizationMode)
    {
        case NoCategories:
        case CategoryByFolder:
            return naturalCompare(left.folder, right.folder, currentCategorizationSortOrder, categorizationCaseSensitivity, strTypeNatural);
        case CategoryByFormat:
            return naturalCompare(left.mime, right.mime, currentCategorizationSortOrder, categorizationCaseSensitivity, strTypeNatural);
        case CategoryByDate:
            return compareByOrder(left.ctime.date(), right.ctime.date(), currentCategorizationSortOrder);
        default:
            return 0;
    }
}

bool CamItemSortSettings::lessThan(const CamItemInfo& left, const CamItemInfo& right) const
{
    int result = compare(left, right, sortRole);

    if (result != 0)
    {
        return result < 0;
    }

    if (left == right)
    {
        return false;
    }

    if ((result = compare(left, right, SortByFileName)) != 0)
    {
        return result < 0;
    }

    if ((result = compare(left, right, SortByCreationDate)) != 0)
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

    if ( (result = compare(left, right, SortByRating)) != 0)
    {
        return result < 0;
    }

    if ( (result = compare(left, right, SortByDownloadState)) != 0)
    {
        return result < 0;
    }

    return false;
}

int CamItemSortSettings::compare(const CamItemInfo& left, const CamItemInfo& right) const
{
    return compare(left, right, sortRole);
}

int CamItemSortSettings::compare(const CamItemInfo& left, const CamItemInfo& right, SortRole role) const
{
    switch (role)
    {
        case SortByFileName:
        {
            bool versioning = (left.name.contains(QLatin1String("_v"), Qt::CaseInsensitive) ||
                               right.name.contains(QLatin1String("_v"), Qt::CaseInsensitive));
            return naturalCompare(left.name, right.name, currentSortOrder, sortCaseSensitivity, strTypeNatural, versioning);
        }
        case SortByFilePath:
            return naturalCompare(left.url().toLocalFile(), right.url().toLocalFile(), currentSortOrder, sortCaseSensitivity, strTypeNatural);
        case SortByFileSize:
            return compareByOrder(left.size, right.size, currentSortOrder);
            //FIXME: Change it to creation date instead of modification date.
        case SortByCreationDate:
            return compareByOrder(left.ctime, right.ctime, currentSortOrder);
        case SortByRating:
            return compareByOrder(left.rating, right.rating, currentSortOrder);
        case SortByDownloadState:
            return compareByOrder(left.downloaded, right.downloaded, currentSortOrder);
        default:
            return 1;
    }
}

bool CamItemSortSettings::lessThan(const QVariant& left, const QVariant& right) const
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

} // namespace Digikam
