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

#ifndef KCATEGORIZEDVIEW_P_H
#define KCATEGORIZEDVIEW_P_H

// Qt includes

#include <QVector>

class KCategorizedSortFilterProxyModel;
class KCategoryDrawer;

namespace Digikam
{

class SparseModelIndexVector : public QVector<QModelIndex>
{
public:

    SparseModelIndexVector(int rowCount, QAbstractItemModel* const model, int column)
        : QVector<QModelIndex>(rowCount),
          model(model),
          column(column)
    {
    }

    inline QModelIndex& operator[](int i)
    {
        QModelIndex& index = QVector<QModelIndex>::operator[](i);

        if (!index.isValid())
        {
            index = model->index(i, column);
        }

        return index;
    }

private:
    // not to be used
    const QModelIndex& operator[](int i) const
    {
        return QVector<QModelIndex>::operator[](i);
    }

private:

    QAbstractItemModel* model;
    int                 column;
};

/**
  * @internal
  */
class DigikamKCategorizedView::Private
{
public:

    explicit Private(DigikamKCategorizedView* const listView);
    ~Private();

    // Methods

    /**
      * Returns the list of items that intersects with @p rect
      */
    const QModelIndexList& intersectionSet(const QRect& rect);

    /**
      * Gets the item rect in the viewport for @p index
      */
    QRect visualRectInViewport(const QModelIndex& index) const;

    /**
      * Returns the category rect in the viewport for @p category
      */
    QRect visualCategoryRectInViewport(const QString& category) const;

    /**
      * Caches and returns the rect that corresponds to @p index
      */
    const QRect& cacheIndex(const QModelIndex& index);

    /**
      * Caches and returns the rect that corresponds to @p category
      */
    const QRect& cacheCategory(const QString& category);

    /**
      * Returns the rect that corresponds to @p index
      * @note If the rect is not cached, it becomes cached
      */
    const QRect& cachedRectIndex(const QModelIndex& index);

    /**
      * Returns the rect that corresponds to @p category
      * @note If the rect is not cached, it becomes cached
      */
    const QRect& cachedRectCategory(const QString& category);

    /**
      * Returns the visual rect (taking in count x and y offsets) for @p index
      * @note If the rect is not cached, it becomes cached
      */
    QRect visualRect(const QModelIndex& index);

    /**
      * Returns the visual rect (taking in count x and y offsets) for @p category
      * @note If the rect is not cached, it becomes cached
      */
    QRect categoryVisualRect(const QString& category);

    /**
      * Returns the contents size of this view (topmost category to bottommost index + spacing)
      */
    QSize contentsSize();

    /**
      * This method will draw a new category represented by index
      * @p index on the rect specified by @p option.rect, with
      * painter @p painter
      */
    void drawNewCategory(const QModelIndex& index,
                         int sortRole,
                         const QStyleOption& option,
                         QPainter* painter);

    /**
      * This method will update scrollbars ranges. Called when our model changes
      * or when the view is resized
      */
    void updateScrollbars();

    /**
      * This method will draw dragged items in the painting operation
      */
    void drawDraggedItems(QPainter* painter);

    /**
      * This method will determine which rect needs to be updated because of a
      * dragging operation
      */
    void drawDraggedItems();

    /**
      * This method will, starting from the index at begin in the given (sorted) modelIndex List,
      * find the last index having the same category as the index to begin with.
      */
    int categoryUpperBound(SparseModelIndexVector& modelIndexList, int begin, int averageSize = 0);

    /**
      * Returns a QItemSelection for all items intersection rect.
      */
    QItemSelection selectionForRect(const QRect& rect);

public:

    // Attributes

    struct ElementInfo
    {
        QString category;
        int     relativeOffsetToCategory;
    };

    // Basic data
    DigikamKCategorizedView*          listView;
    KCategoryDrawer*                  categoryDrawer;
    QSize                             biggestItemSize;

    // Behavior data
    bool                              mouseButtonPressed;
    bool                              rightMouseButtonPressed;
    bool                              dragLeftViewport;
    bool                              drawItemsWhileDragging;
    QModelIndex                       hovered;
    QString                           hoveredCategory;
    QPoint                            initialPressPosition;
    QPoint                            mousePosition;
    int                               forcedSelectionPosition;

    // Cache data
    // We cannot merge some of them into structs because it would affect
    // performance
    QVector<struct ElementInfo>       elementsInfo;
    QHash<int, QRect>                 elementsPosition;
    QHash<QString, QVector<int> >     categoriesIndexes;
    QHash<QString, QRect>             categoriesPosition;
    QStringList                       categories;
    QModelIndexList                   intersectedIndexes;
    QRect                             lastDraggedItemsRect;
    QItemSelection                    lastSelection;

    // Attributes for speed reasons
    KCategorizedSortFilterProxyModel* proxyModel;
};

}  // namespace Digikam

#endif // KCATEGORIZEDVIEW_P_H
