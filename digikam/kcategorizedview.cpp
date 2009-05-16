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

#include "kcategorizedview.h"
#include "kcategorizedview_p.h"

#include <math.h> // trunc on C99 compliant systems
#include <kdefakes.h> // trunc for not C99 compliant systems

#include <QPainter>
#include <QScrollBar>
#include <QPaintEvent>

#include <kdebug.h>

#include "kcategorydrawer.h"
#include "kcategorizedsortfilterproxymodel.h"

// By defining DOLPHIN_DRAGANDDROP the custom drag and drop implementation of
// KCategorizedView is bypassed to have a consistent drag and drop look for all
// views. Hopefully transparent pixmaps for drag objects will be supported in
// Qt 4.4, so that this workaround can be skipped.
#define DOLPHIN_DRAGANDDROP

KCategorizedView::Private::Private(KCategorizedView *listView)
    : listView(listView)
    , categoryDrawer(0)
    , biggestItemSize(QSize(0, 0))
    , mouseButtonPressed(false)
    , rightMouseButtonPressed(false)
    , isDragging(false)
    , dragLeftViewport(false)
    , proxyModel(0)
{
}

KCategorizedView::Private::~Private()
{
}

const QModelIndexList &KCategorizedView::Private::intersectionSet(const QRect &rect)
{
    QModelIndex index;
    QRect indexVisualRect;

    intersectedIndexes.clear();

    int itemHeight;

    if (listView->gridSize().isEmpty())
    {
        itemHeight = biggestItemSize.height();
    }
    else
    {
        itemHeight = listView->gridSize().height();
    }

    // Lets find out where we should start
    int top = proxyModel->rowCount() - 1;
    int bottom = 0;
    int middle = (top + bottom) / 2;
    while (bottom <= top)
    {
        middle = (top + bottom) / 2;

        index = proxyModel->index(middle, 0);
        indexVisualRect = visualRect(index);
        // We need the whole height (not only the visualRect). This will help us to update
        // all needed indexes correctly (ereslibre)
        indexVisualRect.setHeight(indexVisualRect.height() + (itemHeight - indexVisualRect.height()));

        if (qMax(indexVisualRect.topLeft().y(),
                 indexVisualRect.bottomRight().y()) < qMin(rect.topLeft().y(),
                                                           rect.bottomRight().y()))
        {
            bottom = middle + 1;
        }
        else
        {
            top = middle - 1;
        }
    }

    for (int i = middle; i < proxyModel->rowCount(); i++)
    {
        index = proxyModel->index(i, 0);
        indexVisualRect = visualRect(index);

        if (rect.intersects(indexVisualRect))
            intersectedIndexes.append(index);

        // If we passed next item, stop searching for hits
        if (qMax(rect.bottomRight().y(), rect.topLeft().y()) <
                                                  qMin(indexVisualRect.topLeft().y(),
                                                       indexVisualRect.bottomRight().y()))
            break;
    }

    return intersectedIndexes;
}

QRect KCategorizedView::Private::visualRectInViewport(const QModelIndex &index) const
{
    if (!index.isValid())
        return QRect();

    QString curCategory = elementsInfo[index.row()].category;

    QRect retRect;
    const bool leftToRightFlow = (listView->flow() == QListView::LeftToRight);
    
    if (leftToRightFlow)
    {
        if (listView->layoutDirection() == Qt::LeftToRight)
        {
            retRect = QRect(listView->spacing(), listView->spacing() * 2 +
                            categoryDrawer->categoryHeight(index, listView->viewOptions()), 0, 0);
        }
        else
        {
            retRect = QRect(listView->viewport()->width() - listView->spacing(), listView->spacing() * 2 +
                            categoryDrawer->categoryHeight(index, listView->viewOptions()), 0, 0);
        }
    }
    else
    {
        retRect = QRect(listView->spacing(), listView->spacing() * 2 +
                        categoryDrawer->categoryHeight(index, listView->viewOptions()), 0, 0);
    }

    int viewportWidth = listView->viewport()->width() - listView->spacing();

    int itemHeight;
    int itemWidth;

    if (listView->gridSize().isEmpty() && leftToRightFlow)
    {
        itemHeight = biggestItemSize.height();
        itemWidth = biggestItemSize.width();
    }
    else if (leftToRightFlow)
    {
        itemHeight = listView->gridSize().height();
        itemWidth = listView->gridSize().width();
    }
    else if (listView->gridSize().isEmpty() && !leftToRightFlow)
    {
        itemHeight = biggestItemSize.height();
        itemWidth = listView->viewport()->width() - listView->spacing() * 2;
    }
    else
    {
        itemHeight = listView->gridSize().height();
        itemWidth = listView->gridSize().width() - listView->spacing() * 2;
    }

    int itemWidthPlusSeparation = listView->spacing() + itemWidth;
    if (!itemWidthPlusSeparation)
        itemWidthPlusSeparation++;
    int elementsPerRow = viewportWidth / itemWidthPlusSeparation;
    if (!elementsPerRow)
        elementsPerRow++;

    int column;
    int row;

    if (leftToRightFlow)
    {
        column = elementsInfo[index.row()].relativeOffsetToCategory % elementsPerRow;
        row = elementsInfo[index.row()].relativeOffsetToCategory / elementsPerRow;

        if (listView->layoutDirection() == Qt::LeftToRight)
        {
            retRect.setLeft(retRect.left() + column * listView->spacing() +
                            column * itemWidth);
        }
        else
        {
            retRect.setLeft(retRect.right() - column * listView->spacing() -
                            column * itemWidth - itemWidth);

            retRect.setRight(retRect.right() - column * listView->spacing() -
                            column * itemWidth);
        }
    }
    else
    {
        elementsPerRow = 1;
        column = elementsInfo[index.row()].relativeOffsetToCategory % elementsPerRow;
        row = elementsInfo[index.row()].relativeOffsetToCategory / elementsPerRow;
    }

    foreach (const QString &category, categories)
    {
        if (category == curCategory)
            break;

        float rows = (float) ((float) categoriesIndexes[category].count() /
                              (float) elementsPerRow);

        int rowsInt = categoriesIndexes[category].count() / elementsPerRow;

        if (rows - trunc(rows)) rowsInt++;

        retRect.setTop(retRect.top() +
                       (rowsInt * itemHeight) +
                       categoryDrawer->categoryHeight(index, listView->viewOptions()) +
                       listView->spacing() * 2);

        if (listView->gridSize().isEmpty())
        {
            retRect.setTop(retRect.top() +
                           (rowsInt * listView->spacing()));
        }
    }

    if (listView->gridSize().isEmpty())
    {
        retRect.setTop(retRect.top() + row * listView->spacing() +
                       (row * itemHeight));
    }
    else
    {
        retRect.setTop(retRect.top() + (row * itemHeight));
    }

    retRect.setWidth(itemWidth);

    QModelIndex heightIndex = proxyModel->index(index.row(), 0);
    if (listView->gridSize().isEmpty())
    {
        retRect.setHeight(listView->sizeHintForIndex(heightIndex).height());
    }
    else
    {
        const QSize sizeHint = listView->sizeHintForIndex(heightIndex);
        if (sizeHint.width() < itemWidth && leftToRightFlow) {
            retRect.setWidth(sizeHint.width());
            retRect.moveLeft(retRect.left() + (itemWidth - sizeHint.width()) / 2);
        }
        retRect.setHeight(qMin(sizeHint.height(), listView->gridSize().height()));
    }

    return retRect;
}

QRect KCategorizedView::Private::visualCategoryRectInViewport(const QString &category) const
{
    QRect retRect(listView->spacing(),
                  listView->spacing(),
                  listView->viewport()->width() - listView->spacing() * 2,
                  0);

    if (!proxyModel || !categoryDrawer || !proxyModel->isCategorizedModel() || !proxyModel->rowCount() || !categories.contains(category))
        return QRect();

    QModelIndex index = proxyModel->index(0, 0, QModelIndex());

    int viewportWidth = listView->viewport()->width() - listView->spacing();

    int itemHeight;
    int itemWidth;

    if (listView->gridSize().isEmpty())
    {
        itemHeight = biggestItemSize.height();
        itemWidth = biggestItemSize.width();
    }
    else
    {
        itemHeight = listView->gridSize().height();
        itemWidth = listView->gridSize().width();
    }

    int itemWidthPlusSeparation = listView->spacing() + itemWidth;
    int elementsPerRow = viewportWidth / itemWidthPlusSeparation;

    if (!elementsPerRow)
        elementsPerRow++;

    if (listView->flow() == QListView::TopToBottom)
    {
        elementsPerRow = 1;
    }

    foreach (const QString &itCategory, categories)
    {
        if (itCategory == category)
            break;

        float rows = (float) ((float) categoriesIndexes[itCategory].count() /
                              (float) elementsPerRow);
        int rowsInt = categoriesIndexes[itCategory].count() / elementsPerRow;

        if (rows - trunc(rows)) rowsInt++;

        retRect.setTop(retRect.top() +
                       (rowsInt * itemHeight) +
                       categoryDrawer->categoryHeight(index, listView->viewOptions()) +
                       listView->spacing() * 2);

        if (listView->gridSize().isEmpty())
        {
            retRect.setTop(retRect.top() +
                           (rowsInt * listView->spacing()));
        }
    }

    retRect.setHeight(categoryDrawer->categoryHeight(index, listView->viewOptions()));

    return retRect;
}

// We're sure elementsPosition doesn't contain index
const QRect &KCategorizedView::Private::cacheIndex(const QModelIndex &index)
{
    QRect rect = visualRectInViewport(index);
    QHash<int, QRect>::iterator it = elementsPosition.insert(index.row(), rect);

    return *it;
}

// We're sure categoriesPosition doesn't contain category
const QRect &KCategorizedView::Private::cacheCategory(const QString &category)
{
    QRect rect = visualCategoryRectInViewport(category);
    QHash<QString, QRect>::iterator it = categoriesPosition.insert(category, rect);

    return *it;
}

const QRect &KCategorizedView::Private::cachedRectIndex(const QModelIndex &index)
{
    QHash<int, QRect>::const_iterator it = elementsPosition.constFind(index.row());
    if (it != elementsPosition.constEnd()) // If we have it cached
    {                                        // return it
        return *it;
    }
    else                                     // Otherwise, cache it
    {                                        // and return it
        return cacheIndex(index);
    }
}

const QRect &KCategorizedView::Private::cachedRectCategory(const QString &category)
{
    QHash<QString, QRect>::const_iterator it = categoriesPosition.constFind(category);
    if (it != categoriesPosition.constEnd()) // If we have it cached
    {                                                // return it
        return *it;
    }
    else                                            // Otherwise, cache it and
    {                                               // return it
        return cacheCategory(category);
    }
}

QRect KCategorizedView::Private::visualRect(const QModelIndex &index)
{
    QRect retRect = cachedRectIndex(index);
    int dx = -listView->horizontalOffset();
    int dy = -listView->verticalOffset();
    retRect.adjust(dx, dy, dx, dy);

    return retRect;
}

QRect KCategorizedView::Private::categoryVisualRect(const QString &category)
{
    QRect retRect = cachedRectCategory(category);
    int dx = -listView->horizontalOffset();
    int dy = -listView->verticalOffset();
    retRect.adjust(dx, dy, dx, dy);

    return retRect;
}

void KCategorizedView::Private::drawNewCategory(const QModelIndex &index,
                                                int sortRole,
                                                const QStyleOption &option,
                                                QPainter *painter)
{
    if (!index.isValid())
    {
        return;
    }

    QStyleOption optionCopy = option;
    const QString category = proxyModel->data(index, KCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();

    optionCopy.state &= ~QStyle::State_Selected;

    if ((listView->selectionMode() != SingleSelection) && (listView->selectionMode() != NoSelection)) {
        if ((category == hoveredCategory) && !mouseButtonPressed)
        {
            optionCopy.state |= QStyle::State_MouseOver;
        }
        else if ((category == hoveredCategory) && mouseButtonPressed)
        {
            QPoint initialPressPosition = listView->viewport()->mapFromGlobal(QCursor::pos());
            initialPressPosition.setY(initialPressPosition.y() + listView->verticalOffset());
            initialPressPosition.setX(initialPressPosition.x() + listView->horizontalOffset());

            if (initialPressPosition == this->initialPressPosition)
            {
                optionCopy.state |= QStyle::State_Selected;
            }
        }
    }

    categoryDrawer->drawCategory(index,
                                 sortRole,
                                 optionCopy,
                                 painter);
}


void KCategorizedView::Private::updateScrollbars()
{
    // find the last index in the last category
    QModelIndex lastIndex = categoriesIndexes.isEmpty() ? QModelIndex() : categoriesIndexes[categories.last()].last();

    int lastItemBottom = cachedRectIndex(lastIndex).top() +
                         listView->spacing() + (listView->gridSize().isEmpty() ? biggestItemSize.height() : listView->gridSize().height()) - listView->viewport()->height();

    listView->horizontalScrollBar()->setRange(0, 0);

    if (listView->verticalScrollMode() == QAbstractItemView::ScrollPerItem)
    {
        listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    }

    if (listView->horizontalScrollMode() == QAbstractItemView::ScrollPerItem)
    {
        listView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    }

    listView->verticalScrollBar()->setSingleStep(listView->viewport()->height() / 10);
    listView->verticalScrollBar()->setPageStep(listView->viewport()->height());
    listView->verticalScrollBar()->setRange(0, lastItemBottom);
}

void KCategorizedView::Private::drawDraggedItems(QPainter *painter)
{
    QStyleOptionViewItemV4 option = listView->viewOptions();
    option.state &= ~QStyle::State_MouseOver;
    foreach (const QModelIndex &index, listView->selectionModel()->selectedIndexes())
    {
        const int dx = mousePosition.x() - initialPressPosition.x() + listView->horizontalOffset();
        const int dy = mousePosition.y() - initialPressPosition.y() + listView->verticalOffset();

        option.rect = visualRect(index);
        option.rect.adjust(dx, dy, dx, dy);

        if (option.rect.intersects(listView->viewport()->rect()))
        {
            listView->itemDelegate(index)->paint(painter, option, index);
        }
    }
}

void KCategorizedView::Private::layoutChanged(bool forceItemReload)
{
    if (proxyModel && categoryDrawer && proxyModel->isCategorizedModel() &&
        ((forceItemReload ||
          (modelSortRole != proxyModel->sortRole()) ||
          (modelSortColumn != proxyModel->sortColumn()) ||
          (modelSortOrder != proxyModel->sortOrder()) ||
          (modelLastRowCount != proxyModel->rowCount()) ||
          (modelCategorized != proxyModel->isCategorizedModel()))))
    {
        // Force the view to update all elements
        listView->rowsInsertedArtifficial(QModelIndex(), 0, proxyModel->rowCount() - 1);

        if (!forceItemReload)
        {
            modelSortRole = proxyModel->sortRole();
            modelSortColumn = proxyModel->sortColumn();
            modelSortOrder = proxyModel->sortOrder();
            modelLastRowCount = proxyModel->rowCount();
            modelCategorized = proxyModel->isCategorizedModel();
        }
    }

    if (proxyModel && categoryDrawer && proxyModel->isCategorizedModel())
    {
        updateScrollbars();
    }
}

void KCategorizedView::Private::drawDraggedItems()
{
    QRect rectToUpdate;
    QRect currentRect;
    foreach (const QModelIndex &index, listView->selectionModel()->selectedIndexes())
    {
        int dx = mousePosition.x() - initialPressPosition.x() + listView->horizontalOffset();
        int dy = mousePosition.y() - initialPressPosition.y() + listView->verticalOffset();

        currentRect = visualRect(index);
        currentRect.adjust(dx, dy, dx, dy);

        if (currentRect.intersects(listView->viewport()->rect()))
        {
            rectToUpdate = rectToUpdate.united(currentRect);
        }
    }

    listView->viewport()->update(lastDraggedItemsRect.united(rectToUpdate));

    lastDraggedItemsRect = rectToUpdate;
}


//==============================================================================


KCategorizedView::KCategorizedView(QWidget *parent)
    : QListView(parent)
    , d(new Private(this))
{
}

KCategorizedView::~KCategorizedView()
{
    delete d;
}

void KCategorizedView::setGridSize(const QSize &size)
{
    QListView::setGridSize(size);

    d->layoutChanged(true);
}

void KCategorizedView::setModel(QAbstractItemModel *model)
{
    d->lastSelection = QItemSelection();
    d->forcedSelectionPosition = 0;
    d->elementsInfo.clear();
    d->elementsPosition.clear();
    d->categoriesIndexes.clear();
    d->categoriesPosition.clear();
    d->categories.clear();
    d->intersectedIndexes.clear();
    d->modelIndexList.clear();
    d->hovered = QModelIndex();
    d->mouseButtonPressed = false;
    d->rightMouseButtonPressed = false;

    if (d->proxyModel)
    {
        QObject::disconnect(d->proxyModel,
                            SIGNAL(layoutChanged()),
                            this, SLOT(slotLayoutChanged()));

        QObject::disconnect(d->proxyModel,
                            SIGNAL(rowsRemoved(QModelIndex,int,int)),
                            this, SLOT(rowsRemoved(QModelIndex,int,int)));
    }

    QListView::setModel(model);

    d->proxyModel = dynamic_cast<KCategorizedSortFilterProxyModel*>(model);

    if (d->proxyModel)
    {
        d->modelSortRole = d->proxyModel->sortRole();
        d->modelSortColumn = d->proxyModel->sortColumn();
        d->modelSortOrder = d->proxyModel->sortOrder();
        d->modelLastRowCount = d->proxyModel->rowCount();
        d->modelCategorized = d->proxyModel->isCategorizedModel();

        QObject::connect(d->proxyModel,
                         SIGNAL(layoutChanged()),
                         this, SLOT(slotLayoutChanged()));

        QObject::connect(d->proxyModel,
                         SIGNAL(rowsRemoved(QModelIndex,int,int)),
                         this, SLOT(rowsRemoved(QModelIndex,int,int)));

        if (d->proxyModel->rowCount())
        {
            d->layoutChanged(true);
        }
    }
    else
    {
        d->modelCategorized = false;
    }
}

QRect KCategorizedView::visualRect(const QModelIndex &index) const
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return QListView::visualRect(index);
    }

    if (!qobject_cast<const QSortFilterProxyModel*>(index.model()))
    {
        return d->visualRect(d->proxyModel->mapFromSource(index));
    }

    return d->visualRect(index);
}

KCategoryDrawer *KCategorizedView::categoryDrawer() const
{
    return d->categoryDrawer;
}

void KCategorizedView::setCategoryDrawer(KCategoryDrawer *categoryDrawer)
{
    d->lastSelection = QItemSelection();
    d->forcedSelectionPosition = 0;
    d->elementsInfo.clear();
    d->elementsPosition.clear();
    d->categoriesIndexes.clear();
    d->categoriesPosition.clear();
    d->categories.clear();
    d->intersectedIndexes.clear();
    d->modelIndexList.clear();
    d->hovered = QModelIndex();
    d->mouseButtonPressed = false;
    d->rightMouseButtonPressed = false;

    if (!categoryDrawer && d->proxyModel)
    {
        QObject::disconnect(d->proxyModel,
                            SIGNAL(layoutChanged()),
                            this, SLOT(slotLayoutChanged()));

        QObject::disconnect(d->proxyModel,
                            SIGNAL(rowsRemoved(QModelIndex,int,int)),
                            this, SLOT(rowsRemoved(QModelIndex,int,int)));
    }
    else if (categoryDrawer && d->proxyModel)
    {
        QObject::connect(d->proxyModel,
                         SIGNAL(layoutChanged()),
                         this, SLOT(slotLayoutChanged()));

        QObject::connect(d->proxyModel,
                         SIGNAL(rowsRemoved(QModelIndex,int,int)),
                         this, SLOT(rowsRemoved(QModelIndex,int,int)));
    }

    d->categoryDrawer = categoryDrawer;

    if (categoryDrawer)
    {
        if (d->proxyModel)
        {
            if (d->proxyModel->rowCount())
            {
                d->layoutChanged(true);
            }
        }
    }
    else
    {
        updateGeometries();
    }
}

QModelIndex KCategorizedView::indexAt(const QPoint &point) const
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return QListView::indexAt(point);
    }

    QModelIndex index;

    const QModelIndexList item = d->intersectionSet(QRect(point, point));

    if (item.count() == 1)
    {
        index = item[0];
    }

    return index;
}

void KCategorizedView::reset()
{
    QListView::reset();

    d->lastSelection = QItemSelection();
    d->forcedSelectionPosition = 0;
    d->elementsInfo.clear();
    d->elementsPosition.clear();
    d->categoriesIndexes.clear();
    d->categoriesPosition.clear();
    d->categories.clear();
    d->intersectedIndexes.clear();
    d->modelIndexList.clear();
    d->hovered = QModelIndex();
    d->biggestItemSize = QSize(0, 0);
    d->mouseButtonPressed = false;
    d->rightMouseButtonPressed = false;
}

void KCategorizedView::paintEvent(QPaintEvent *event)
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        QListView::paintEvent(event);
        return;
    }

    bool alternatingRows = alternatingRowColors();

    QStyleOptionViewItemV4 option = viewOptions();
    option.widget = this;
    if (wordWrap())
    {
        option.features |= QStyleOptionViewItemV4::WrapText;
    }

    QPainter painter(viewport());
    QRect area = event->rect();
    const bool focus = (hasFocus() || viewport()->hasFocus()) &&
                        currentIndex().isValid();
    const QStyle::State state = option.state;
    const bool enabled = (state & QStyle::State_Enabled) != 0;

    painter.save();

    QModelIndexList dirtyIndexes = d->intersectionSet(area);
    bool alternate = false;
    if (dirtyIndexes.count())
    {
        alternate = dirtyIndexes[0].row() % 2;
    }
    foreach (const QModelIndex &index, dirtyIndexes)
    {
        if (alternatingRows && alternate)
        {
            option.features |= QStyleOptionViewItemV4::Alternate;
            alternate = false;
        }
        else if (alternatingRows)
        {
            option.features &= ~QStyleOptionViewItemV4::Alternate;
            alternate = true;
        }
        option.state = state;
        option.rect = visualRect(index);

        if (selectionModel() && selectionModel()->isSelected(index))
        {
            option.state |= QStyle::State_Selected;
        }

        if (enabled)
        {
            QPalette::ColorGroup cg;
            if ((d->proxyModel->flags(index) & Qt::ItemIsEnabled) == 0)
            {
                option.state &= ~QStyle::State_Enabled;
                cg = QPalette::Disabled;
            }
            else
            {
                cg = QPalette::Normal;
            }
            option.palette.setCurrentColorGroup(cg);
        }

        if (focus && currentIndex() == index)
        {
            option.state |= QStyle::State_HasFocus;
            if (this->state() == EditingState)
                option.state |= QStyle::State_Editing;
        }

        if (index == d->hovered)
            option.state |= QStyle::State_MouseOver;
        else
            option.state &= ~QStyle::State_MouseOver;

        itemDelegate(index)->paint(&painter, option, index);
    }

    // Redraw categories
    QStyleOptionViewItemV4 otherOption;
    bool intersectedInThePast = false;
    foreach (const QString &category, d->categories)
    {
        otherOption = option;
        otherOption.rect = d->categoryVisualRect(category);
        otherOption.state &= ~QStyle::State_MouseOver;

        if (otherOption.rect.intersects(area))
        {
            intersectedInThePast = true;

            QModelIndex indexToDraw = d->proxyModel->index(d->categoriesIndexes[category][0].row(), d->proxyModel->sortColumn());

            d->drawNewCategory(indexToDraw,
                               d->proxyModel->sortRole(), otherOption, &painter);
        }
        else if (intersectedInThePast)
        {
            break; // the visible area has been finished, we don't need to keep asking, the rest won't intersect
                // this is doable because we know that categories are correctly ordered on the list
        }
    }

    if ((selectionMode() != SingleSelection) && (selectionMode() != NoSelection))
    {
        if (d->mouseButtonPressed && !d->isDragging)
        {
            QPoint start, end, initialPressPosition;

            initialPressPosition = d->initialPressPosition;

            initialPressPosition.setY(initialPressPosition.y() - verticalOffset());
            initialPressPosition.setX(initialPressPosition.x() - horizontalOffset());

            if (d->initialPressPosition.x() > d->mousePosition.x() ||
                d->initialPressPosition.y() > d->mousePosition.y())
            {
                start = d->mousePosition;
                end = initialPressPosition;
            }
            else
            {
                start = initialPressPosition;
                end = d->mousePosition;
            }

            QStyleOptionRubberBand yetAnotherOption;
            yetAnotherOption.initFrom(this);
            yetAnotherOption.shape = QRubberBand::Rectangle;
            yetAnotherOption.opaque = false;
            yetAnotherOption.rect = QRect(start, end).intersected(viewport()->rect().adjusted(-16, -16, 16, 16));
            painter.save();
            style()->drawControl(QStyle::CE_RubberBand, &yetAnotherOption, &painter);
            painter.restore();
        }
    }

    if (d->isDragging && !d->dragLeftViewport)
    {
        painter.setOpacity(0.5);
        d->drawDraggedItems(&painter);
    }

    painter.restore();
}

void KCategorizedView::resizeEvent(QResizeEvent *event)
{
    QListView::resizeEvent(event);

    // Clear the items positions cache
    d->elementsPosition.clear();
    d->categoriesPosition.clear();
    d->forcedSelectionPosition = 0;

    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return;
    }

    d->updateScrollbars();
}

void KCategorizedView::setSelection(const QRect &rect,
                                    QItemSelectionModel::SelectionFlags flags)
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        QListView::setSelection(rect, flags);
        return;
    }

    if (!flags)
        return;

    if (flags & QItemSelectionModel::Clear)
    {
        selectionModel()->clear();
        d->lastSelection.clear();
    }

    QModelIndexList dirtyIndexes = d->intersectionSet(rect);

    // no items affected, just leave
    if (!dirtyIndexes.count())
    {
        selectionModel()->select(d->lastSelection, QItemSelectionModel::SelectCurrent);

        return;
    }

    QModelIndex topLeft;
    QModelIndex bottomRight;

    if (d->mouseButtonPressed || d->rightMouseButtonPressed) // selection with click + drag
    {
        QItemSelection selection;

        QModelIndex prev = dirtyIndexes[0];
        QModelIndex first = prev;
        foreach (const QModelIndex &index, dirtyIndexes)
        {
            // we have a different interval. non-contiguous items
            if ((index.row() - prev.row()) > 1) {
                selection << QItemSelectionRange(first, prev);

                first = index;
            }

            prev = index;
        }

        selection << QItemSelectionRange(first, prev);

        if (flags & QItemSelectionModel::Current)
        {
            if (rect.topLeft() == rect.bottomRight())
            {
                selectionModel()->setCurrentIndex(indexAt(rect.topLeft()), QItemSelectionModel::NoUpdate);
            }

            selection.merge(d->lastSelection, flags);
        }
        else
        {
            selection.merge(selectionModel()->selection(), flags);

            selectionModel()->select(selection, QItemSelectionModel::SelectCurrent);

            return;
        }

        selectionModel()->select(selection, flags);
    }
    else // selection with click + keyboard keys
    {
        QModelIndex topLeftIndex = indexAt(QPoint(rect.topLeft().x(),
                                                  rect.topLeft().y()));
        QModelIndex bottomRightIndex = indexAt(QPoint(rect.bottomRight().x(),
                                                      rect.bottomRight().y()));

        // keyboard selection comes "upside down". Let's normalize it
        if (topLeftIndex.row() > bottomRightIndex.row())
        {
            QModelIndex auxIndex = topLeftIndex;
            topLeftIndex = bottomRightIndex;
            bottomRightIndex = auxIndex;
        }

        int viewportWidth = viewport()->width() - spacing();
        int itemWidth;

        if (gridSize().isEmpty())
        {
            itemWidth = d->biggestItemSize.width();
        }
        else
        {
            itemWidth = gridSize().width();
        }

        int itemWidthPlusSeparation = spacing() + itemWidth;
        if (!itemWidthPlusSeparation)
            itemWidthPlusSeparation++;
        int elementsPerRow = viewportWidth / itemWidthPlusSeparation;
        if (!elementsPerRow)
            elementsPerRow++;

        QModelIndexList theoricDirty(dirtyIndexes);
        dirtyIndexes.clear();
        int first = model()->rowCount();
        int last = 0;

        foreach (const QModelIndex &index, theoricDirty)
        {
            if ((index.row() < first) &&
                ((((topLeftIndex.row() / elementsPerRow) == (index.row() / elementsPerRow)) &&
                  ((topLeftIndex.row() % elementsPerRow) <= (index.row() % elementsPerRow))) ||
                 (topLeftIndex.row() / elementsPerRow) != (index.row() / elementsPerRow)))
            {
                first = index.row();
                topLeft = index;
            }

            if ((index.row() > last) &&
                ((((bottomRightIndex.row() / elementsPerRow) == (index.row() / elementsPerRow)) &&
                  ((bottomRightIndex.row() % elementsPerRow) >= (index.row() % elementsPerRow))) ||
                 (bottomRightIndex.row() / elementsPerRow) != (index.row() / elementsPerRow)))
            {
                last = index.row();
                bottomRight = index;
            }
        }

        for (int i = first; i <= last; i++)
        {
            dirtyIndexes << model()->index(i, theoricDirty[0].column(), theoricDirty[0].parent());
        }

        QItemSelection selection(topLeft, bottomRight);

        selectionModel()->select(selection, flags);
    }
}

void KCategorizedView::mouseMoveEvent(QMouseEvent *event)
{
    QListView::mouseMoveEvent(event);

    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return;
    }

    const QModelIndexList item = d->intersectionSet(QRect(event->pos(), event->pos()));

    if (item.count() == 1)
    {
        d->hovered = item[0];
    }
    else
    {
        d->hovered = QModelIndex();
    }

    const QString previousHoveredCategory = d->hoveredCategory;

    d->mousePosition = event->pos();
    d->hoveredCategory.clear();

    // Redraw categories
    foreach (const QString &category, d->categories)
    {
        if (d->categoryVisualRect(category).intersects(QRect(event->pos(), event->pos())))
        {
            d->hoveredCategory = category;
            viewport()->update(d->categoryVisualRect(category));
        }
        else if ((category == previousHoveredCategory) &&
                 (!d->categoryVisualRect(previousHoveredCategory).intersects(QRect(event->pos(), event->pos()))))
        {
            viewport()->update(d->categoryVisualRect(category));
        }
    }

    QRect rect;
    if (d->mouseButtonPressed && !d->isDragging)
    {
        QPoint start, end, initialPressPosition;

        initialPressPosition = d->initialPressPosition;

        initialPressPosition.setY(initialPressPosition.y() - verticalOffset());
        initialPressPosition.setX(initialPressPosition.x() - horizontalOffset());

        if (d->initialPressPosition.x() > d->mousePosition.x() ||
            d->initialPressPosition.y() > d->mousePosition.y())
        {
            start = d->mousePosition;
            end = initialPressPosition;
        }
        else
        {
            start = initialPressPosition;
            end = d->mousePosition;
        }

        rect = QRect(start, end).adjusted(-16, -16, 16, 16);
        rect = rect.united(QRect(start, end).adjusted(16, 16, -16, -16)).intersected(viewport()->rect());

        viewport()->update(rect);
    }
}

void KCategorizedView::mousePressEvent(QMouseEvent *event)
{
    d->dragLeftViewport = false;

    if (event->button() == Qt::LeftButton)
    {
        d->mouseButtonPressed = true;

        d->initialPressPosition = event->pos();
        d->initialPressPosition.setY(d->initialPressPosition.y() +
                                                              verticalOffset());
        d->initialPressPosition.setX(d->initialPressPosition.x() +
                                                            horizontalOffset());
    }
    else if (event->button() == Qt::RightButton)
    {
        d->rightMouseButtonPressed = true;
    }

    QListView::mousePressEvent(event);

    if (selectionModel())
    {
        d->lastSelection = selectionModel()->selection();
    }

    viewport()->update(d->categoryVisualRect(d->hoveredCategory));
}

void KCategorizedView::mouseReleaseEvent(QMouseEvent *event)
{
    d->mouseButtonPressed = false;
    d->rightMouseButtonPressed = false;

    QListView::mouseReleaseEvent(event);

    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return;
    }

    QPoint initialPressPosition = viewport()->mapFromGlobal(QCursor::pos());
    initialPressPosition.setY(initialPressPosition.y() + verticalOffset());
    initialPressPosition.setX(initialPressPosition.x() + horizontalOffset());

    if ((selectionMode() != SingleSelection) && (selectionMode() != NoSelection) &&
        (initialPressPosition == d->initialPressPosition))
    {
        foreach(const QString &category, d->categories)
        {
            if (d->categoryVisualRect(category).contains(event->pos()) &&
                selectionModel())
            {
                QItemSelection selection = selectionModel()->selection();
                QModelIndexList indexList = d->categoriesIndexes[category];

                foreach (const QModelIndex &index, indexList)
                {
                    QModelIndex selectIndex = index.model()->index(index.row(), 0);

                    selection << QItemSelectionRange(selectIndex);
                }

                selectionModel()->select(selection, QItemSelectionModel::SelectCurrent);

                break;
            }
        }
    }

    QRect rect;
    if (!d->isDragging)
    {
        QPoint start, end, initialPressPosition;

        initialPressPosition = d->initialPressPosition;

        initialPressPosition.setY(initialPressPosition.y() - verticalOffset());
        initialPressPosition.setX(initialPressPosition.x() - horizontalOffset());

        if (d->initialPressPosition.x() > d->mousePosition.x() ||
            d->initialPressPosition.y() > d->mousePosition.y())
        {
            start = d->mousePosition;
            end = initialPressPosition;
        }
        else
        {
            start = initialPressPosition;
            end = d->mousePosition;
        }

        rect = QRect(start, end).adjusted(-16, -16, 16, 16);
        rect = rect.united(QRect(start, end).adjusted(16, 16, -16, -16)).intersected(viewport()->rect());

        viewport()->update(rect);
    }

    if (d->hovered.isValid())
        viewport()->update(visualRect(d->hovered));
    else if (!d->hoveredCategory.isEmpty())
        viewport()->update(d->categoryVisualRect(d->hoveredCategory));
}

void KCategorizedView::leaveEvent(QEvent *event)
{
    d->hovered = QModelIndex();
    d->hoveredCategory.clear();

    QListView::leaveEvent(event);
}

void KCategorizedView::startDrag(Qt::DropActions supportedActions)
{
    // FIXME: QAbstractItemView does far better here since it sets the
    //        pixmap of selected icons to the dragging cursor, but it sets a non
    //        ARGB window so it is no transparent. Use QAbstractItemView when
    //        this is fixed on Qt.
    // QAbstractItemView::startDrag(supportedActions);
#if defined(DOLPHIN_DRAGANDDROP)
    Q_UNUSED(supportedActions);
#else
    QListView::startDrag(supportedActions);
#endif

    d->isDragging = false;
    d->mouseButtonPressed = false;
    d->rightMouseButtonPressed = false;

    viewport()->update(d->lastDraggedItemsRect);
}

void KCategorizedView::dragMoveEvent(QDragMoveEvent *event)
{
    d->mousePosition = event->pos();

    if (d->mouseButtonPressed)
    {
        d->isDragging = true;
    }
    else
    {
        d->isDragging = false;
    }

    d->dragLeftViewport = false;

#if defined(DOLPHIN_DRAGANDDROP)
    QAbstractItemView::dragMoveEvent(event);
#else
    QListView::dragMoveEvent(event);
#endif

    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return;
    }
    
    d->hovered = indexAt(event->pos());

#if !defined(DOLPHIN_DRAGANDDROP)
    d->drawDraggedItems();
#endif
}

void KCategorizedView::dragLeaveEvent(QDragLeaveEvent *event)
{
    d->dragLeftViewport = true;

#if defined(DOLPHIN_DRAGANDDROP)
    QAbstractItemView::dragLeaveEvent(event);
#else
    QListView::dragLeaveEvent(event);
#endif
}

void KCategorizedView::dropEvent(QDropEvent *event)
{
#if defined(DOLPHIN_DRAGANDDROP)
    QAbstractItemView::dropEvent(event);
#else
    QListView::dropEvent(event);
#endif
}

QModelIndex KCategorizedView::moveCursor(CursorAction cursorAction,
                                         Qt::KeyboardModifiers modifiers)
{
    if ((viewMode() != KCategorizedView::IconMode) ||
        !d->proxyModel ||
        !d->categoryDrawer ||
         d->categories.isEmpty() ||
        !d->proxyModel->isCategorizedModel())
    {
        return QListView::moveCursor(cursorAction, modifiers);
    }

    int viewportWidth = viewport()->width() - spacing();
    int itemWidth;

    if (gridSize().isEmpty())
    {
        itemWidth = d->biggestItemSize.width();
    }
    else
    {
        itemWidth = gridSize().width();
    }

    int itemWidthPlusSeparation = spacing() + itemWidth;
    if (!itemWidthPlusSeparation)
        itemWidthPlusSeparation++;
    int elementsPerRow = viewportWidth / itemWidthPlusSeparation;
    if (!elementsPerRow)
        elementsPerRow++;

    QModelIndex current = selectionModel() ? selectionModel()->currentIndex()
                                           : QModelIndex();

    if (!current.isValid())
    {
        if (cursorAction == MoveEnd)
        {
            current = model()->index(model()->rowCount() - 1, 0, QModelIndex());
            d->forcedSelectionPosition = d->elementsInfo[current.row()].relativeOffsetToCategory % elementsPerRow;
        }
        else
        {
            current = model()->index(0, 0, QModelIndex());
            d->forcedSelectionPosition = 0;
        }

        return current;
    }

    QString lastCategory = d->categories.first();
    QString theCategory = d->categories.first();
    QString afterCategory = d->categories.first();

    bool hasToBreak = false;
    foreach (const QString &category, d->categories)
    {
        if (hasToBreak)
        {
            afterCategory = category;

            break;
        }

        if (category == d->elementsInfo[current.row()].category)
        {
            theCategory = category;

            hasToBreak = true;
        }

        if (!hasToBreak)
        {
            lastCategory = category;
        }
    }

    switch (cursorAction)
    {
        case QAbstractItemView::MoveUp: {
            if (d->elementsInfo[current.row()].relativeOffsetToCategory >= elementsPerRow)
            {
                int indexToMove = current.row();
                indexToMove -= qMin(((d->elementsInfo[current.row()].relativeOffsetToCategory) + d->forcedSelectionPosition), elementsPerRow - d->forcedSelectionPosition + (d->elementsInfo[current.row()].relativeOffsetToCategory % elementsPerRow));

                return d->proxyModel->index(indexToMove, 0);
            }
            else
            {
                int lastCategoryLastRow = (d->categoriesIndexes[lastCategory].count() - 1) % elementsPerRow;
                int indexToMove = current.row() - d->elementsInfo[current.row()].relativeOffsetToCategory;

                if (d->forcedSelectionPosition >= lastCategoryLastRow)
                {
                    indexToMove -= 1;
                }
                else
                {
                    indexToMove -= qMin((lastCategoryLastRow - d->forcedSelectionPosition + 1), d->forcedSelectionPosition + elementsPerRow + 1);
                }

                return d->proxyModel->index(indexToMove, 0);
            }
        }

        case QAbstractItemView::MoveDown: {
            if (d->elementsInfo[current.row()].relativeOffsetToCategory < (d->categoriesIndexes[theCategory].count() - 1 - ((d->categoriesIndexes[theCategory].count() - 1) % elementsPerRow)))
            {
                int indexToMove = current.row();
                indexToMove += qMin(elementsPerRow, d->categoriesIndexes[theCategory].count() - 1 - d->elementsInfo[current.row()].relativeOffsetToCategory);

                return d->proxyModel->index(indexToMove, 0);
            }
            else
            {
                int afterCategoryLastRow = qMin(elementsPerRow, d->categoriesIndexes[afterCategory].count());
                int indexToMove = current.row() + (d->categoriesIndexes[theCategory].count() - d->elementsInfo[current.row()].relativeOffsetToCategory);

                if (d->forcedSelectionPosition >= afterCategoryLastRow)
                {
                    indexToMove += afterCategoryLastRow - 1;
                }
                else
                {
                    indexToMove += qMin(d->forcedSelectionPosition, elementsPerRow);
                }

                return d->proxyModel->index(indexToMove, 0);
            }
        }

        case QAbstractItemView::MoveLeft:
            if (layoutDirection() == Qt::RightToLeft)
            {
                if (!(d->elementsInfo[current.row() + 1].relativeOffsetToCategory % elementsPerRow))
                    return current;

                d->forcedSelectionPosition = d->elementsInfo[current.row() + 1].relativeOffsetToCategory % elementsPerRow;

#if 0 //follow qt view behavior. lateral movements won't change visual row
                if (d->forcedSelectionPosition < 0)
                    d->forcedSelectionPosition = (d->categoriesIndexes[theCategory].count() - 1) % elementsPerRow;
#endif

                return d->proxyModel->index(current.row() + 1, 0);
            }

            if (!(d->elementsInfo[current.row()].relativeOffsetToCategory % elementsPerRow))
                return current;

            d->forcedSelectionPosition = d->elementsInfo[current.row() - 1].relativeOffsetToCategory % elementsPerRow;

#if 0 //follow qt view behavior. lateral movements won't change visual row
            if (d->forcedSelectionPosition < 0)
                d->forcedSelectionPosition = (d->categoriesIndexes[theCategory].count() - 1) % elementsPerRow;
#endif

            return d->proxyModel->index(current.row() - 1, 0);

        case QAbstractItemView::MoveRight:
            if (layoutDirection() == Qt::RightToLeft)
            {
                if (!(d->elementsInfo[current.row()].relativeOffsetToCategory % elementsPerRow))
                    return current;

                d->forcedSelectionPosition = d->elementsInfo[current.row() - 1].relativeOffsetToCategory % elementsPerRow;

#if 0 //follow qt view behavior. lateral movements won't change visual row
                if (d->forcedSelectionPosition < 0)
                    d->forcedSelectionPosition = (d->categoriesIndexes[theCategory].count() - 1) % elementsPerRow;
#endif

                return d->proxyModel->index(current.row() - 1, 0);
            }

            if (!(d->elementsInfo[current.row() + 1].relativeOffsetToCategory % elementsPerRow))
                return current;

            d->forcedSelectionPosition = d->elementsInfo[current.row() + 1].relativeOffsetToCategory % elementsPerRow;

#if 0 //follow qt view behavior. lateral movements won't change visual row
            if (d->forcedSelectionPosition < 0)
                d->forcedSelectionPosition = (d->categoriesIndexes[theCategory].count() - 1) % elementsPerRow;
#endif

            return d->proxyModel->index(current.row() + 1, 0);

        default:
            break;
    }

    return QListView::moveCursor(cursorAction, modifiers);
}

void KCategorizedView::rowsInserted(const QModelIndex &parent,
                                    int start,
                                    int end)
{
    QListView::rowsInserted(parent, start, end);

    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        d->forcedSelectionPosition = 0;
        d->elementsInfo.clear();
        d->elementsPosition.clear();
        d->categoriesIndexes.clear();
        d->categoriesPosition.clear();
        d->categories.clear();
        d->intersectedIndexes.clear();
        d->modelIndexList.clear();
        d->hovered = QModelIndex();
        d->biggestItemSize = QSize(0, 0);
        d->mouseButtonPressed = false;
        d->rightMouseButtonPressed = false;

        return;
    }

    rowsInsertedArtifficial(parent, start, end);
}

void KCategorizedView::rowsInsertedArtifficial(const QModelIndex &parent,
                                               int start,
                                               int end)
{
    Q_UNUSED(parent);

    d->forcedSelectionPosition = 0;
    d->elementsInfo.clear();
    d->elementsPosition.clear();
    d->categoriesIndexes.clear();
    d->categoriesPosition.clear();
    d->categories.clear();
    d->intersectedIndexes.clear();
    d->modelIndexList.clear();
    d->hovered = QModelIndex();
    d->biggestItemSize = QSize(0, 0);
    d->mouseButtonPressed = false;
    d->rightMouseButtonPressed = false;

    if (start > end || end < 0 || start < 0 || !d->proxyModel->rowCount())
    {
        return;
    }

    // Add all elements mapped to the source model and explore categories
    QString prevCategory = d->proxyModel->data(d->proxyModel->index(0, d->proxyModel->sortColumn()), KCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();
    QString lastCategory = prevCategory;
    QModelIndexList modelIndexList;
    struct Private::ElementInfo elementInfo;
    int offset = -1;
    for (int k = 0; k < d->proxyModel->rowCount(); ++k)
    {
        QModelIndex index = d->proxyModel->index(k, d->proxyModel->sortColumn());
        QModelIndex indexSize = d->proxyModel->index(k, 0);

        d->biggestItemSize = QSize(qMax(sizeHintForIndex(indexSize).width(),
                                        d->biggestItemSize.width()),
                                   qMax(sizeHintForIndex(indexSize).height(),
                                        d->biggestItemSize.height()));

        d->modelIndexList << index;

        lastCategory = d->proxyModel->data(index, KCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();

        elementInfo.category = lastCategory;

        if (prevCategory != lastCategory)
        {
            offset = 0;
            d->categoriesIndexes.insert(prevCategory, modelIndexList);
            d->categories << prevCategory;
            modelIndexList.clear();
        }
        else
        {
            offset++;
        }

        elementInfo.relativeOffsetToCategory = offset;

        modelIndexList << index;
        prevCategory = lastCategory;

        d->elementsInfo.insert(index.row(), elementInfo);
    }

    d->categoriesIndexes.insert(prevCategory, modelIndexList);
    d->categories << prevCategory;

    d->updateScrollbars();

    // FIXME: We need to safely save the last selection. This is on my TODO
    // list (ereslibre).
    selectionModel()->clear();
}

void KCategorizedView::rowsRemoved(const QModelIndex &parent,
                                   int start,
                                   int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    if (d->proxyModel && d->categoryDrawer && d->proxyModel->isCategorizedModel())
    {
        // Force the view to update all elements
        rowsInsertedArtifficial(QModelIndex(), 0, d->proxyModel->rowCount() - 1);
    }
}

void KCategorizedView::updateGeometries()
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        QListView::updateGeometries();
        return;
    }

    // Avoid QListView::updateGeometries(), since it will try to set another
    // range to our scroll bars, what we don't want (ereslibre)
    QAbstractItemView::updateGeometries();
}

void KCategorizedView::slotLayoutChanged()
{
    d->layoutChanged();
}

void KCategorizedView::currentChanged(const QModelIndex &current,
                                      const QModelIndex &previous)
{
    // We need to update the forcedSelectionPosition property in order to correctly
    // navigate after with keyboard using up & down keys

    int viewportWidth = viewport()->width() - spacing();

    int itemHeight;
    int itemWidth;

    if (gridSize().isEmpty())
    {
        itemHeight = d->biggestItemSize.height();
        itemWidth = d->biggestItemSize.width();
    }
    else
    {
        itemHeight = gridSize().height();
        itemWidth = gridSize().width();
    }

    int itemWidthPlusSeparation = spacing() + itemWidth;
    if (!itemWidthPlusSeparation)
        itemWidthPlusSeparation++;
    int elementsPerRow = viewportWidth / itemWidthPlusSeparation;
    if (!elementsPerRow)
        elementsPerRow++;

    if (d->mouseButtonPressed || d->rightMouseButtonPressed)
        d->forcedSelectionPosition = d->elementsInfo[current.row()].relativeOffsetToCategory % elementsPerRow;

    QListView::currentChanged(current, previous);
}

void KCategorizedView::dataChanged(const QModelIndex &topLeft,
                                   const QModelIndex &bottomRight)
{
    if (topLeft == bottomRight)
    {
        d->cacheIndex(topLeft);
    }
    else
    {
        const int columnStart = topLeft.column();
        const int columnEnd = bottomRight.column();
        const int rowStart = topLeft.row();
        const int rowEnd = bottomRight.row();

        for (int row = rowStart; row <= rowEnd; ++row)
        {
            for (int column = columnStart; column <= columnEnd; ++column)
            {
                d->cacheIndex(d->proxyModel->index(row, column));
            }
        }
    }

    QListView::dataChanged(topLeft, bottomRight);
    slotLayoutChanged();
}

#include "kcategorizedview.moc"
