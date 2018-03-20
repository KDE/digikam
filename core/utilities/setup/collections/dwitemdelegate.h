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

#ifndef DW_ITEM_DELEGATE_H
#define DW_ITEM_DELEGATE_H

// Qt includes

#include <QEvent>
#include <QList>
#include <QPersistentModelIndex>
#include <QAbstractItemDelegate>

// Local includes

#include "dwitemdelegate_p.h"

class QObject;
class QPainter;
class QStyleOption;
class QStyleOptionViewItem;
class QAbstractItemView;
class QItemSelection;

namespace Digikam
{

class DWItemDelegatePool;

/**
 * This class allows to create item delegates embedding simple widgets to interact
 * with items. For instance you can add push buttons, line edits, etc. to your delegate
 * and use them to modify the state of your model.
 */
class DWItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    /**
     * Creates a new ItemDelegate to be used with a given itemview.
     *
     * @param itemView the item view the new delegate will monitor
     * @param parent the parent of this delegate
     */
    explicit DWItemDelegate(QAbstractItemView* const itemView, QObject* const parent = 0);
    virtual ~DWItemDelegate();

    /**
     * Retrieves the item view this delegate is monitoring.
     *
     * @return the item view this delegate is monitoring
     */
    QAbstractItemView* itemView() const;

    /**
     * Retrieves the currently focused index. An invalid index if none is focused.
     *
     * @return the current focused index, or QPersistentModelIndex() if none is focused.
     */
    QPersistentModelIndex focusedIndex() const;

protected:

    /**
     * Creates the list of widgets needed for an item.
     *
     * @note No initialization of the widgets is supposed to happen here.
     *       The widgets will be initialized based on needs for a given item.
     *
     * @note If you want to connect some widget signals to any slot, you should
     *       do it here.
     *
     * @arg index the index to create widgets for.
     *
     * @note If you want to know the index for which you are creating widgets, it is
     *       available as a QModelIndex Q_PROPERTY called "goya:creatingWidgetsForIndex".
     *       Ensure to add Q_DECLARE_METATYPE(QModelIndex) before your method definition
     *       to tell QVariant about QModelIndex.
     *
     * @return the list of newly created widgets which will be used to interact with an item.
     * @see updateItemWidgets()
     */
    virtual QList<QWidget*> createItemWidgets(const QModelIndex& index) const = 0;

    /**
     * Updates a list of widgets for its use inside of the delegate (painting or
     * event handling).
     *
     * @note All the positioning and sizing should be done in item coordinates.
     *
     * @warning Do not make widget connections in here, since this method will
     * be called very regularly.
     *
     * @param widgets the widgets to update
     * @param option the current set of style options for the view.
     * @param index the model index of the item currently manipulated.
     */
    virtual void updateItemWidgets(const QList<QWidget*> widgets,
                                   const QStyleOptionViewItem& option,
                                   const QPersistentModelIndex& index) const = 0;

    /**
     * Sets the list of event @p types that a @p widget will block.
     *
     * Blocked events are not passed to the view. This way you can prevent an item
     * from being selected when a button is clicked for instance.
     *
     * @param widget the widget which must block events
     * @param types the list of event types the widget must block
     */
    void setBlockedEventTypes(QWidget* const widget, QList<QEvent::Type> types) const;

    /**
     * Retrieves the list of blocked event types for the given widget.
     *
     * @param widget the specified widget.
     *
     * @return the list of blocked event types, can be empty if no events are blocked.
     */
    QList<QEvent::Type> blockedEventTypes(QWidget* const widget) const;

private:

    friend class DWItemDelegatePool;
    friend class DWItemDelegateEventListener;
    DWItemDelegatePrivate* const d;

    Q_PRIVATE_SLOT(d, void slotDWRowsInserted(const QModelIndex&,int,int))
    Q_PRIVATE_SLOT(d, void slotDWRowsAboutToBeRemoved(const QModelIndex&,int,int))
    Q_PRIVATE_SLOT(d, void slotDWRowsRemoved(const QModelIndex&,int,int))
    Q_PRIVATE_SLOT(d, void slotDWDataChanged(const QModelIndex&,const QModelIndex&))
    Q_PRIVATE_SLOT(d, void slotDWLayoutChanged())
    Q_PRIVATE_SLOT(d, void slotDWModelReset())
    Q_PRIVATE_SLOT(d, void slotDWSelectionChanged(const QItemSelection&,const QItemSelection&))
};

} // namespace Digikam

Q_DECLARE_METATYPE(QList<QEvent::Type>)

#endif // Digikam
