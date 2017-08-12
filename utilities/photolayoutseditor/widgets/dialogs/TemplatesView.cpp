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

#include "TemplatesView.h"

#include <QScrollBar>
#include <QPainter>
#include <QPen>
#include <QApplication>
#include <QMouseEvent>

#include "TemplatesModel.h"

QHash<int, QRectF> rectForRow;

#define WIDTH 120
#define HEIGHT 120

using namespace PhotoLayoutsEditor;

class TemplateItemDelegate : public QAbstractItemDelegate
{
    public:

        virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
        {
            if (!index.internalPointer())
                return;

            QRectF rf = option.rect;
            rf.setTop(rf.top() + 2);
            rf.setBottom(rf.bottom() - 2);
            rf.setLeft(rf.left() + 2);
            rf.setRight(rf.right() - 2);

            if ( option.state & QStyle::State_Selected )
                painter->fillRect( rf, QApplication::palette().highlight() );
            else
                painter->fillRect( rf, QApplication::palette().base() );

            TemplateItem* const item = static_cast<TemplateItem*>(index.internalPointer());
            QImage i = item->icon();
            QRect ir;
            if (!i.isNull())
            {
                painter->drawImage(rf.left() + (WIDTH - i.width()) / 2,
                                   rf.top() + 5,
                                   i);
                ir = i.rect();
                painter->setPen(QPen(Qt::black, 0));
                painter->drawRect(ir.translated(rf.left() + (WIDTH - ir.width()) / 2,
                                                      rf.top() + 5));
            }

            painter->drawText(QRectF(rf.x(),
                                     rf.y() + HEIGHT + 5,
                                     WIDTH,
                                     999),
                              Qt::AlignHCenter,
                              item->name());
            painter->save();
            painter->setPen(QPen(Qt::gray, 0));
            painter->drawRect(rf.x(), rf.y(), rf.width(), rf.height());
            painter->restore();
        }

        virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
        {
            QSize result;
            if (!index.internalPointer())
                return result;
            return option.rect.size();
        }
};

TemplatesView::TemplatesView(QWidget * parent) :
    QAbstractItemView(parent),
//    columns(0),
    idealWidth(0),
    idealHeight(0),
    hashIsDirty(false)
{
    this->setItemDelegate(new TemplateItemDelegate());
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    setFocusPolicy(Qt::WheelFocus);
    setFont(QApplication::font("QListView"));
    horizontalScrollBar()->setRange(0, 0);
    verticalScrollBar()->setRange(0, 0);
}

void TemplatesView::mousePressEvent(QMouseEvent * event)
{
    QAbstractItemView::mousePressEvent(event);
    setCurrentIndex( indexAt(event->pos()) );
}

void TemplatesView::updateGeometries() //
{
    QFontMetrics fm(font());

    horizontalScrollBar()->setSingleStep(fm.width(QLatin1String("n")));
    horizontalScrollBar()->setPageStep(viewport()->width());
    horizontalScrollBar()->setRange(0, qMax(0, idealWidth - viewport()->width()));

    verticalScrollBar()->setSingleStep(HEIGHT);
    verticalScrollBar()->setPageStep(viewport()->height());
    verticalScrollBar()->setRange(0, qMax(0, idealHeight - viewport()->height()));
}

void TemplatesView::resizeEvent(QResizeEvent *) //
{
    hashIsDirty = true;
    calculateRectsIfNecessary();
    updateGeometries();
}

void TemplatesView::paintOutline(QPainter * painter, const QRectF &rectangle)
{
    const QRectF rect = rectangle.adjusted(0, 0, -1, -1);
    painter->save();
    painter->setPen(QPen(palette().dark().color(), 0.5));
    painter->drawRect(rect);
    painter->setPen(QPen(Qt::black, 0.5));
    painter->drawLine(rect.bottomLeft(), rect.bottomRight());
    painter->drawLine(rect.bottomRight(), rect.topRight());
    painter->restore();
}

void TemplatesView::paintEvent(QPaintEvent *)
{
    if (!model())
        return;

    QPainter painter(viewport());
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    for (int row = 0; row < model()->rowCount(rootIndex()); ++row)
    {
        QModelIndex index = model()->index(row, 0, rootIndex());

        QRectF rect = viewportRectForRow(row);

        if (!rect.isValid() || rect.bottom() < 0 || rect.y() > viewport()->height())
            continue;

        QStyleOptionViewItem option = viewOptions();

        option.rect = rect.toRect();

        if (selectionModel()->isSelected(index))
            option.state |= QStyle::State_Selected;

        if (currentIndex() == index)
            option.state |= QStyle::State_HasFocus;

        itemDelegate()->paint(&painter, option, index);

        //paintOutline(&painter, rect);
    }
}

QRegion TemplatesView::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion region;
    foreach (const QItemSelectionRange & range, selection)
    {
        for (int row = range.top(); row <= range.bottom(); ++row)
        {
            for (int column = range.left(); column < range.right(); ++column)
            {
                QModelIndex index = model()->index(row, column, rootIndex());
                region += visualRect(index);
            }
        }
    }
    return region;
}

void TemplatesView::setSelection(const QRect & rect, QFlags<QItemSelectionModel::SelectionFlag> flags)
{
    QRect rectangle = rect.translated(horizontalScrollBar()->value(), verticalScrollBar()->value()).normalized();

    calculateRectsIfNecessary();

    QHashIterator<int, QRectF> i(rectForRow);
    int firstRow = model()->rowCount();
    int lastRow = -1;

    while (i.hasNext())
    {
        i.next();
        if (i.value().intersects(rectangle))
        {
            firstRow = firstRow < i.key() ? firstRow : i.key();
            lastRow = lastRow > i.key() ? lastRow : i.key();
        }
    }

    if (firstRow != model()->rowCount() && lastRow != -1)
    {
        QItemSelection selection( model()->index(firstRow, 0, rootIndex()),
                                  model()->index(lastRow, 0, rootIndex()) );
        selectionModel()->select(selection, flags);
    }
    else
    {
        QModelIndex invalid;
        QItemSelection selection(invalid, invalid);
        selectionModel()->select(selection, flags);
    }
}

void TemplatesView::scrollContentsBy(int dx, int dy)
{
    scrollDirtyRegion(dx, dy);
    viewport()->scroll(dx, dy);
}

int TemplatesView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int TemplatesView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

QModelIndex TemplatesView::moveCursor( QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers)
{
    QModelIndex index = currentIndex();
    if (index.isValid())
    {
        if ((cursorAction == MoveLeft && index.row() > 0) ||
                (cursorAction == MoveRight &&
                 index.row() + 1 < model()->rowCount()))
        {
            const int offset = (cursorAction == MoveLeft ? -1 : 1);
            index = model()->index(index.row() + offset, index.column(), index.parent());
        }
        else if ((cursorAction == MoveUp && index.row() > 0) ||
                 (cursorAction == MoveDown &&
                  index.row() + 1 < model()->rowCount()))
        {
            QFontMetrics fm(font());
            const int RowHeight = (fm.height() + HEIGHT) * (cursorAction == MoveUp ? -1 : 1);
            QRect rect = viewportRectForRow(index.row()).toRect();
            QPoint point(rect.center().x(), rect.center().y() + RowHeight);
            while (point.x() >= 0)
            {
                index = indexAt(point);
                if (index.isValid())
                    break;
                point.rx() -= fm.width(QLatin1String("n"));
            }
        }
    }
    return index;
}

void TemplatesView::rowsInserted(const QModelIndex & parent, int start, int end)
{
    hashIsDirty = true;
    QAbstractItemView::rowsInserted(parent, start, end);
}

void TemplatesView::rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{
    hashIsDirty = true;
    QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}

void TemplatesView::dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
    hashIsDirty = true;
    QAbstractItemView::dataChanged(topLeft, bottomRight);
}

QModelIndex TemplatesView::indexAt(const QPoint & point_) const
{
    QPoint point(point_);
    point.rx() += horizontalScrollBar()->value();
    point.ry() += verticalScrollBar()->value();
    calculateRectsIfNecessary();
    QHashIterator<int, QRectF> i(rectForRow);
    while (i.hasNext())
    {
        i.next();
        if (i.value().contains(point))
            return model()->index(i.key(), 0, rootIndex());
    }
    return QModelIndex();
}

void TemplatesView::scrollTo(const QModelIndex & index, QAbstractItemView::ScrollHint)
{
    QRect viewRect = viewport()->rect();
    QRect itemRect = visualRect(index);

    if (itemRect.left() < viewRect.left())
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + itemRect.left() - viewRect.left());
    else if (itemRect.right() > viewRect.right())
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + qMin(itemRect.right() - viewRect.right(), itemRect.left() - viewRect.left()));

    if (itemRect.top() < viewRect.top())
        verticalScrollBar()->setValue(verticalScrollBar()->value() + itemRect.top() - viewRect.top());
    else if (itemRect.bottom() > viewRect.bottom())
        verticalScrollBar()->setValue(verticalScrollBar()->value() + qMin(itemRect.bottom() - viewRect.bottom(), itemRect.top() - viewRect.top()));

    viewport()->update();
}

bool TemplatesView::isIndexHidden(const QModelIndex &) const
{
    return false;
}

QRectF TemplatesView::viewportRectForRow(int row) const //
{
    calculateRectsIfNecessary();

    QRectF rect = rectForRow.value(row).toRect();

    if (!rect.isValid())
        return rect;

    return QRectF(rect.x() - horizontalScrollBar()->value(),
                  rect.y() - verticalScrollBar()->value(),
                  rect.width(),
                  rect.height());
}

QRect TemplatesView::visualRect(const QModelIndex & index) const //
{
    QRect rect;
    if (index.isValid())
        rect = viewportRectForRow(index.row()).toRect();
    return rect;
}

void TemplatesView::calculateRectsIfNecessary() const //1
{
    if (!hashIsDirty)
        return;

    if (!model())
        return;

    QFontMetrics fm(font());
    const int MaxWidth = viewport()->width();

    int minimumWidth = 0;
    int x = 0;
    int y = 0;

    for (int row = 0; row < model()->rowCount(rootIndex()); ++row)
    {
        QModelIndex index = model()->index(row, 0, rootIndex());
        QString text = model()->data(index).toString();
        QRect nameRect = fm.boundingRect(0, 0, WIDTH, 999, 0, text);
        int RowHeight = nameRect.height() + HEIGHT + 10;

        if (!(x == 0 || x + WIDTH < MaxWidth))
        {
            y += RowHeight;
            x = 0;
        }
        else if (x != 0)
        {
            x += 0;
        }

        rectForRow[row] = QRectF(x, y, WIDTH, RowHeight);

        if (WIDTH > minimumWidth)
            minimumWidth = WIDTH;

        x += WIDTH;
    }

    idealWidth = minimumWidth;
    idealHeight = y + HEIGHT * 1.5;
    hashIsDirty = false;

    viewport()->update();
}

void TemplatesView::setModel(QAbstractItemModel * model) //
{
    QAbstractItemView::setModel(model);
    hashIsDirty = true;
}

QString TemplatesView::selectedPath() const
{
    if (!model())
        return QString();
    TemplateItem * item = static_cast<TemplateItem*>(this->currentIndex().internalPointer());
    if (!item)
        return QString();
    return item->path();
}
