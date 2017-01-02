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

#ifndef CAMITEMSORTSETTINGS_H
#define CAMITEMSORTSETTINGS_H

// Qt includes

#include <QCollator>

// Local includes

#include "camiteminfo.h"

namespace Digikam
{

class CamItemSortSettings
{
public:

    CamItemSortSettings();
    ~CamItemSortSettings();

    bool operator==(const CamItemSortSettings& other) const;

    /** Compares the categories of left and right camItemInfos.
     *  It returns -1 if the left camItemInfo is less than right, and 0 if both fall
     *  in the same category, and 1 if the left camItemInfo is greater than right.
     *  Adheres to set categorization mode and current category sort order.
     */
    int compareCategories(const CamItemInfo& left, const CamItemInfo& right) const;


    /** Returns true if left is less than right.
     *  Adheres to current sort role and sort order.
     */
    bool lessThan(const CamItemInfo& left, const CamItemInfo& right) const;

    /** Returns true if left QVariant is less than right.
     *  Adheres to current sort role and sort order.
     */
    bool lessThan(const QVariant& left, const QVariant& right) const;

    /** Compares the camItemInfos left and right.
     *  Return -1 if left is less than right, 1 if left is greater than right,
     *  and 0 if left equals right comparing the current sort role's value.
     *  Adheres to set sort role and sort order.
     */
    int compare(const CamItemInfo& left, const CamItemInfo& right) const;

    enum SortOrder
    {
        AscendingOrder  = Qt::AscendingOrder,
        DescendingOrder = Qt::DescendingOrder,
        DefaultOrder /// sort order depends on the chosen sort role
    };

    /// --- Categories ---------------

    enum CategorizationMode
    {
        NoCategories,
        CategoryByFolder,
        CategoryByFormat,
        CategoryByDate
    };

    CategorizationMode  categorizationMode;
    SortOrder           categorizationSortOrder;

    void setCategorizationMode(CategorizationMode mode);
    void setCategorizationSortOrder(SortOrder order);

    /// Only Ascending or Descending, never be DefaultOrder
    Qt::SortOrder        currentCategorizationSortOrder;
    Qt::CaseSensitivity  categorizationCaseSensitivity;

    bool isCategorized() const { return categorizationMode >= CategoryByFolder; }

    /// --- Camera Items Sorting ---------------

    enum SortRole
    {
        SortByFileName,
        SortByFilePath,
        SortByCreationDate,
        SortByFileSize,
        SortByDownloadState,
        SortByRating
    };

    SortOrder   sortOrder;
    SortRole    sortRole;
    bool        strTypeNatural;

    void setSortRole(SortRole role);
    void setSortOrder(SortOrder order);
    void setStringTypeNatural(bool natural);

    Qt::SortOrder       currentSortOrder;
    Qt::CaseSensitivity sortCaseSensitivity;

    int compare(const CamItemInfo& left, const CamItemInfo& right, SortRole sortRole) const;

    static Qt::SortOrder defaultSortOrderForCategorizationMode(CategorizationMode mode);
    static Qt::SortOrder defaultSortOrderForSortRole(SortRole role);

    /** Returns a < b if sortOrder is Ascending, or b < a if order is descending
     */
    template <typename T>
    static inline bool lessThanByOrder(const T& a, const T& b, Qt::SortOrder sortOrder)
    {
        if (sortOrder == Qt::AscendingOrder)
            return a < b;
        else
            return b < a;
    }

    /** Returns the usual compare result of -1, 0, or 1 for lessThan, equals and greaterThan.
     */
    template <typename T>
    static inline int compareValue(const T& a, const T &b)
    {
        if (a == b)
            return 0;

        if (a < b)
            return -1;
        else
            return 1;
    }

    /** Takes a typical result from a compare method (0 is equal, -1 is less than, 1 is greater than)
     *  and applies the given sort order to it.
     */
    static inline int compareByOrder(int compareResult,  Qt::SortOrder sortOrder)
    {
        if (sortOrder == Qt::AscendingOrder)
            return compareResult;
        else
            return - compareResult;
    }

    template <typename T>
    static inline int compareByOrder(const T& a, const T& b, Qt::SortOrder sortOrder)
    {
        return compareByOrder(compareValue(a, b), sortOrder);
    }

    /** Compares the two string by natural comparison and adheres to given sort order
     */
    static inline int naturalCompare(const QString& a, const QString& b, Qt::SortOrder sortOrder,
                                     Qt::CaseSensitivity caseSensitive = Qt::CaseSensitive,
                                     bool natural = true, bool versioning = false)
    {
        QCollator collator;
        collator.setNumericMode(natural);
        collator.setIgnorePunctuation(versioning);
        collator.setCaseSensitivity(caseSensitive);
        return (compareByOrder(collator.compare(a, b), sortOrder));
    }
};

} // namespace Digikam

#endif // CAMITEMSORTSETTINGS_H
