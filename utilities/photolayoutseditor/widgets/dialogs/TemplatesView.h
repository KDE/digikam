/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011-2012 by Lukasz Spas <lukasz dot spas at gmail dot com>
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

#ifndef TEMPLATESVIEW_H
#define TEMPLATESVIEW_H

#include <QTableView>

namespace PhotoLayoutsEditor
{
    class TemplatesView : public QAbstractItemView
    {
            Q_OBJECT

//            int columns;
            mutable int idealWidth;
            mutable int idealHeight;
            mutable bool hashIsDirty;

        public:

            explicit TemplatesView(QWidget * parent = 0);

            void mousePressEvent(QMouseEvent * event);
            void updateGeometries();
            void resizeEvent(QResizeEvent*);
            void paintOutline(QPainter * painter, const QRectF &rectangle);
            void paintEvent(QPaintEvent*);
            QRegion visualRegionForSelection(const QItemSelection &selection) const;
            void setSelection(const QRect &rect, QFlags<QItemSelectionModel::SelectionFlag> flags);
            void scrollContentsBy(int dx, int dy);
            int horizontalOffset() const;
            int verticalOffset() const;
            QModelIndex moveCursor( QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers);
            void rowsInserted(const QModelIndex & parent, int start, int end);
            void rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end);
            void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
            QModelIndex indexAt(const QPoint &point_) const;
            void scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint);
            bool isIndexHidden(const QModelIndex&) const;
            QRectF viewportRectForRow(int row) const;
            QRect visualRect(const QModelIndex &index) const;
            void calculateRectsIfNecessary() const;
            void setModel(QAbstractItemModel * model);

            QString selectedPath() const;

        Q_SIGNALS:

        public Q_SLOTS:

    };
}
#endif // TEMPLATESVIEW_H
