/**
  * This file is part of the KDE project
  * Copyright (C) 2007 Rafael Fernández López <ereslibre@kde.org>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License as published by the Free Software Foundation; either
  * version 2 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#ifndef KCATEGORIZEDVIEW_H
#define KCATEGORIZEDVIEW_H

#include <QtGui/QListView>

#include <kdeui_export.h>

class KCategoryDrawer;

/**
  * @short Item view for listing items
  *
  * KCategorizedView allows you to use it as it were a QListView. 
  * Subclass KCategorizedSortFilterProxyModel to provide category information for items.
  *
  * @see KCategorizedSortFilterProxyModel
  *
  * @author Rafael Fernández López <ereslibre@kde.org>
  */
class KDEUI_EXPORT KCategorizedView
    : public QListView
{
    Q_OBJECT

public:
    KCategorizedView(QWidget *parent = 0);

    ~KCategorizedView();

    virtual void setModel(QAbstractItemModel *model);

    void setGridSize(const QSize &size);

    virtual QRect visualRect(const QModelIndex &index) const;

    KCategoryDrawer *categoryDrawer() const;

    void setCategoryDrawer(KCategoryDrawer *categoryDrawer);

    virtual QModelIndex indexAt(const QPoint &point) const;

public Q_SLOTS:
    virtual void reset();

protected:
    virtual void paintEvent(QPaintEvent *event);

    virtual void resizeEvent(QResizeEvent *event);

    virtual void setSelection(const QRect &rect,
                              QItemSelectionModel::SelectionFlags flags);

    virtual void mouseMoveEvent(QMouseEvent *event);

    virtual void mousePressEvent(QMouseEvent *event);

    virtual void mouseReleaseEvent(QMouseEvent *event);

    virtual void leaveEvent(QEvent *event);

    virtual void startDrag(Qt::DropActions supportedActions);

    virtual void dragMoveEvent(QDragMoveEvent *event);

    virtual void dragLeaveEvent(QDragLeaveEvent *event);

    virtual void dropEvent(QDropEvent *event);

    virtual QModelIndex moveCursor(CursorAction cursorAction,
                                   Qt::KeyboardModifiers modifiers);

protected Q_SLOTS:
    virtual void rowsInserted(const QModelIndex &parent,
                              int start,
                              int end);

    virtual void rowsInsertedArtifficial(const QModelIndex &parent,
                                         int start,
                                         int end);

    virtual void rowsRemoved(const QModelIndex &parent,
                             int start,
                             int end);

    virtual void updateGeometries();

    virtual void slotLayoutChanged();

    virtual void currentChanged(const QModelIndex &current,
                                const QModelIndex &previous);

    virtual void dataChanged(const QModelIndex &topLeft,
                             const QModelIndex &bottomRight);

private:
    class Private;
    Private *const d;
};

#endif // KCATEGORIZEDVIEW_H
