/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-08
 * Description : Item view to list import interface items.
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

#ifndef ICATEGORIZEDVIEW_H
#define ICATEGORIZEDVIEW_H

// Local includes

#include "importkcategorizedview.h"
#include "dragdropimplementations.h"
#include "ditemdelegate.h"
#include "itemviewtooltip.h"

class QSortFilterProxyModel;

namespace Digikam
{

class ICategorizedView : public ImportKCategorizedView, DragDropViewImplementation
{
    Q_OBJECT

public:

    ICategorizedView(QWidget* const parent = 0);
    ~ICategorizedView();

    DItemDelegate* delegate() const;

    int numberOfSelectedIndexes() const;

    /** Selects the index as current and scrolls to it */
    void toFirstIndex();
    void toLastIndex();
    void toNextIndex();
    void toPreviousIndex();
    void toIndex(const QModelIndex& index);
    void awayFromSelection();

    /** Like scrollTo, but only scrolls if the index is not visible, regardless of hint. */
    void scrollToRelaxed(const QModelIndex& index, ScrollHint hint = EnsureVisible);

    /** Determine a step size for scrolling. The larget this number, the smaller and more
     *  precise is the scrolling. Default is 10. */
    void setScrollStepGranularity(int factor);

    void setSelectedIndexes(const QList<QModelIndex>& indexes);
    void invertSelection();

    /** Sets the spacing. Does not use setSpacing()/spacing() from QListView */
    void setSpacing(int space);

    void setToolTipEnabled(bool enabled);
    bool isToolTipEnabled() const;

    /** Set if the PointingHand Cursor should be shown over the activation area */
    void setUsePointingHandCursor(bool useCursor);

    virtual QSortFilterProxyModel* filterModel() const = 0;

public Q_SLOTS:

    virtual void copy()  { DragDropViewImplementation::copy();  }
    virtual void cut()   { DragDropViewImplementation::cut();   }
    virtual void paste() { DragDropViewImplementation::paste(); }

    void showIndexNotification(const QModelIndex& index, const QString& message);
    void hideIndexNotification();

Q_SIGNALS:

    /** For overlays: Like the respective parent class signals, but with additional info.
     *  Do not change the mouse events.
     */
    void clicked(const QMouseEvent* e, const QModelIndex& index);
    void entered(const QMouseEvent* e, const QModelIndex& index);

    /** While clicked() is emitted with a valid index, this corresponds to clicking on empty space.
     */
    void viewportClicked(const QMouseEvent* e);

    /**  Remember you may want to check if the event is accepted or ignored.
     *   This signal is emitted after being handled by this widget.
     *   You can accept it if ignored.
     */
    void keyPressed(QKeyEvent* e);

    void zoomInStep();
    void zoomOutStep();

    /** Emitted when any selection change occurs. Any of the signals below will be emitted before.
     */
    void selectionChanged();

    /** Emitted when the selection is completely cleared.
     */
    void selectionCleared();

protected Q_SLOTS:

    void slotClicked(const QModelIndex& index);
    void slotActivated(const QModelIndex& index);
    void slotEntered(const QModelIndex& index);
    void layoutAboutToBeChanged();
    void layoutWasChanged();

    virtual void slotThemeChanged();
    virtual void slotSetupChanged();

protected:

    /** Provides default behavior, can reimplement in a subclass.
     *  Returns true if a tooltip was shown.
     *  The help event is optional.
     */
    virtual bool showToolTip(const QModelIndex& index, QStyleOptionViewItem& option, QHelpEvent* e = 0);

    // reimplemented from parent class
    void contextMenuEvent(QContextMenuEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void resizeEvent(QResizeEvent* event);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void reset();
    void selectionChanged(const QItemSelection& selectedItems, const QItemSelection& deselectedItems);
    void wheelEvent(QWheelEvent* event);
    bool viewportEvent(QEvent* event);
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

    /** Reimplement thses in a subclass.
     */
    virtual void showContextMenuOnIndex(QContextMenuEvent* event, const QModelIndex& index);
    virtual void showContextMenu(QContextMenuEvent* event);
    virtual void indexActivated(const QModelIndex& index);

    DECLARE_VIEW_DRAG_DROP_METHODS(ImportKCategorizedView)

    /** Note: pure virtual dragDropHandler() still open from DragDropViewImplementation.
     */
    virtual QModelIndex mapIndexForDragDrop(const QModelIndex& index) const;
    virtual QPixmap     pixmapForDrag(const QList<QModelIndex>& indexes) const;

    /**
     * Assuming the given indexes would be removed (hypothetically!),
     * return the index to be selected instead, starting from anchor.
     * The default implementation returns the next remaining sibling.
     */
    virtual QModelIndex nextIndexHint(const QModelIndex& indexToAnchor, const QItemSelectionRange& removed) const;

    void setToolTip(ItemViewToolTip* tip);
    void setItemDelegate(DItemDelegate* delegate);

    void updateDelegateSizes();
    void userInteraction();

    /** Returns an index that is representative for the category at position pos.
     */
    QModelIndex indexForCategoryAt(const QPoint& pos) const;

private Q_SLOTS:

    void slotGridSizeChanged(const QSize&);

private:

    void ensureSelectionAfterChanges();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ICATEGORIZEDVIEW_H
