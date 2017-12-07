/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-16
 * Description : categorize item view based on DCategorizedView
 *
 * Copyright (C) 2007      by Rafael Fernández López <ereslibre at kde dot org>
 * Copyright (C) 2007      by John Tapsell <tapsell at kde dot org>
 * Copyright (C) 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DCATEGORIZED_SORT_FILTER_PROXY_MODEL_H
#define DCATEGORIZED_SORT_FILTER_PROXY_MODEL_H

// Qt includes

#include <QSortFilterProxyModel>

// Local includes

#include "digikam_export.h"

class QItemSelection;

namespace Digikam
{

/**
  * This class lets you categorize a view. It is meant to be used along with
  * DCategorizedView class.
  *
  * In general terms all you need to do is to reimplement subSortLessThan() and
  * compareCategories() methods. In order to make categorization work, you need
  * to also call setCategorizedModel() class to enable it, since the categorization
  * is disabled by default.
  */
class DIGIKAM_EXPORT DCategorizedSortFilterProxyModel : public QSortFilterProxyModel
{

public:

    enum AdditionalRoles
    {
        // Note: use printf "0x%08X\n" $(($RANDOM*$RANDOM))
        // to define additional roles.
        CategoryDisplayRole = 0x17CE990A,  ///< This role is used for asking the category to a given index

        CategorySortRole    = 0x27857E60   ///< This role is used for sorting categories. You can return a
                                           ///< string or a long long value. Strings will be sorted alphabetically
                                           ///< while long long will be sorted by their value. Please note that this
                                           ///< value won't be shown on the view, is only for sorting purposes. What will
                                           ///< be shown as "Category" on the view will be asked with the role
                                           ///< CategoryDisplayRole.
    };

    DCategorizedSortFilterProxyModel(QObject* const parent = 0);
    virtual ~DCategorizedSortFilterProxyModel();

    /**
      * Overridden from QSortFilterProxyModel. Sorts the source model using
      * @p column for the given @p order.
      */
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    /**
      * @return whether the model is categorized or not. Disabled by default.
      */
    bool isCategorizedModel() const;

    /**
      * Enables or disables the categorization feature.
      *
      * @param categorizedModel whether to enable or disable the categorization feature.
      */
    void setCategorizedModel(bool categorizedModel);

    /**
      * @return the column being used for sorting.
      */
    int sortColumn() const;

    /**
      * @return the sort order being used for sorting.
      */
    Qt::SortOrder sortOrder() const;

    /**
      * Set if the sorting using CategorySortRole will use a natural comparison
      * in the case that strings were returned. If enabled, QCollator
      * will be used for sorting.
      *
      * @param sortCategoriesByNaturalComparison whether to sort using a natural comparison or not.
      */
    void setSortCategoriesByNaturalComparison(bool sortCategoriesByNaturalComparison);

    /**
      * @return whether it is being used a natural comparison for sorting. Enabled by default.
      */
    bool sortCategoriesByNaturalComparison() const;

protected:
    /**
      * Overridden from QSortFilterProxyModel. If you are subclassing
      * DCategorizedSortFilterProxyModel, you will probably not need to reimplement this
      * method.
      *
      * It calls compareCategories() to sort by category.  If the both items are in the
      * same category (i.e. compareCategories returns 0), then subSortLessThan is called.
      *
      * @return Returns true if the item @p left is less than the item @p right when sorting.
      *
      * @warning You usually won't need to reimplement this method when subclassing
      *          from DCategorizedSortFilterProxyModel.
      */
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    /**
      * This method has a similar purpose as lessThan() has on QSortFilterProxyModel.
      * It is used for sorting items that are in the same category.
      *
      * @return Returns true if the item @p left is less than the item @p right when sorting.
      */
    virtual bool subSortLessThan(const QModelIndex &left, const QModelIndex &right) const;

    /**
      * This method compares the category of the @p left index with the category
      * of the @p right index.
      *
      * Internally and if not reimplemented, this method will ask for @p left and
      * @p right models for role CategorySortRole. In order to correctly sort
      * categories, the data() metod of the model should return a qlonglong (or numeric) value, or
      * a QString object. QString objects will be sorted with QString::localeAwareCompare if
      * sortCategoriesByNaturalComparison() is true.
      *
      * @note Please have present that:
      *       QString(QChar(QChar::ObjectReplacementCharacter)) >
      *       QString(QChar(QChar::ReplacementCharacter)) >
      *       [ all possible strings ] >
      *       QString();
      *
      *       This means that QString() will be sorted the first one, while
      *       QString(QChar(QChar::ObjectReplacementCharacter)) and
      *       QString(QChar(QChar::ReplacementCharacter)) will be sorted in last
      *       position.
      *
      * @warning Please note that data() method of the model should return always
      *          information of the same type. If you return a QString for an index,
      *          you should return always QStrings for all indexes for role CategorySortRole
      *          in order to correctly sort categories. You can't mix by returning
      *          a QString for one index, and a qlonglong for other.
      *
      * @note If you need a more complex layout, you will have to reimplement this
      *       method.
      *
      * @return A negative value if the category of @p left should be placed before the
      *         category of @p right. 0 if @p left and @p right are on the same category, and
      *         a positive value if the category of @p left should be placed after the
      *         category of @p right.
      */
    virtual int compareCategories(const QModelIndex &left, const QModelIndex &right) const;

private:

    class Private;
    Private *const d;
};

} // namespace Digikam

#endif // DCATEGORIZED_SORT_FILTER_PROXY_MODEL_H
