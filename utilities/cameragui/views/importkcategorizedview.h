/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-07
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

#ifndef ImportKCategorizedView_H
#define ImportKCategorizedView_H

// Qt includes

#include <QtGui/QListView>

class KCategoryDrawer;

namespace Digikam
{

class ImportKCategorizedView : public QListView
{
    Q_OBJECT

public:

    ImportKCategorizedView(QWidget* parent = 0);
    ~ImportKCategorizedView();

    void setGridSize(const QSize& size);

    void setCategoryDrawer(KCategoryDrawer* categoryDrawer);
    KCategoryDrawer* categoryDrawer() const;

    /**
     * Switch on drawing of dragged items. Default: on.
     * While dragging over the view, dragged items will be drawn transparently
     * following the mouse cursor.
     */
    void setDrawDraggedItems(bool drawDraggedItems);

    virtual void setModel(QAbstractItemModel* model);
    virtual QRect visualRect(const QModelIndex& index) const;
    virtual QModelIndex indexAt(const QPoint& p) const;

    /**
     * This method will return all indexes whose visual rect intersects @p rect.
     * @param rect rectangle to test intersection with
     * @note Returns an empty list if the view is not categorized.
     */
    virtual QModelIndexList categorizedIndexesIn(const QRect& rect) const;

    /**
     * This method will return the visual rect of the header of the category
     * in which @p index is sorted.
     * @note Returns QRect() if the view is not categorized.
     */
    virtual QRect categoryVisualRect(const QModelIndex& index) const;

    /**
     * This method will return the first index of the category
     * in the region of which @p point is found.
     * @note Returns QModelIndex() if the view is not categorized.
     */
    virtual QModelIndex categoryAt(const QPoint& point) const;

    /**
     * This method returns the range of indexes contained
     * in the category in which @p index is sorted.
     * @note Returns an empty range if the view is no categorized.
     */
    virtual QItemSelectionRange categoryRange(const QModelIndex& index) const;

public Q_SLOTS:

    virtual void reset();

protected:

    virtual void paintEvent(QPaintEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
    virtual void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags flags);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void leaveEvent(QEvent* event);
    virtual void startDrag(Qt::DropActions supportedActions);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dragLeaveEvent(QDragLeaveEvent* event);
    virtual void dropEvent(QDropEvent* event);
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

protected Q_SLOTS:

    virtual void rowsInserted(const QModelIndex& parent, int start, int end);
    virtual void rowsInsertedArtifficial(const QModelIndex& parent, int start, int end);
    virtual void rowsRemoved(const QModelIndex& parent, int start, int end);
    virtual void updateGeometries();
    virtual void slotLayoutChanged();
    virtual void currentChanged(const QModelIndex& current, const QModelIndex& previous);

private:

    class ImportKCategorizedViewPriv;
    ImportKCategorizedViewPriv* const d;
};

} //namespace Digikam

#endif // ImportKCategorizedView_H
