/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-15
 * Description : widget item delegate for setup collection view
 *
 * Copyright (C) 2015      by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2008 by Rafael Fernández López <ereslibre at kde dot org>
 * Copyright (C) 2008      by Kevin Ottens <ervin at kde dot org>
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

#ifndef DW_ITEM_DELEGATE_POOL_H
#define DW_ITEM_DELEGATE_POOL_H

// Qt includes

#include <QModelIndex>
#include <QHash>
#include <QList>

class QWidget;
class QStyleOptionViewItem;

namespace Digikam
{

class DWItemDelegate;
class DWItemDelegatePoolPrivate;

class DWItemDelegatePool
{
public:

    enum UpdateWidgetsEnum
    {
        UpdateWidgets = 0,
        NotUpdateWidgets
    };

public:

    /**
      * Creates a new ItemDelegatePool.
      *
      * @param delegate the ItemDelegate for this pool.
      */
    DWItemDelegatePool(DWItemDelegate* const delegate);
    ~DWItemDelegatePool();

    /**
      * @brief Returns the widget associated to @p index and @p widget
      * @param index The index to search into.
      * @param option a QStyleOptionViewItem.
      * @return A QList of the pointers to the widgets found.
      * @internal
      */
    QList<QWidget*> findWidgets(const QPersistentModelIndex& index, const QStyleOptionViewItem& option,
                                UpdateWidgetsEnum updateWidgets = UpdateWidgets) const;

    QList<QWidget*> invalidIndexesWidgets() const;

    void fullClear();

private:

    friend class DWItemDelegate;
    friend class DWItemDelegatePrivate;
    DWItemDelegatePoolPrivate* const d;
};

// -----------------------------------------------------------------------------------------------------------

class DWItemDelegateEventListener;

class DWItemDelegatePoolPrivate
{
public:

    DWItemDelegatePoolPrivate(DWItemDelegate* const d);

    DWItemDelegate*                                delegate;
    DWItemDelegateEventListener*                   eventListener;

    QList<QList<QWidget*> >                        allocatedWidgets;
    QHash<QPersistentModelIndex, QList<QWidget*> > usedWidgets;
    QHash<QWidget*, QPersistentModelIndex>         widgetInIndex;

    bool                                           clearing;
};

} // namespace Digikam

#endif // DW_ITEM_DELEGATE_POOL_H
