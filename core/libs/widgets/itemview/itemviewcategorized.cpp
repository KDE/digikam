/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-16
 * Description : Qt item view for images
 *
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

#include "itemviewcategorized.h"

// Qt includes

#include <QClipboard>
#include <QHelpEvent>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QStyle>
#include <QApplication>

// Local includes

#include "digikam_debug.h"
#include "thememanager.h"
#include "ditemdelegate.h"
#include "abstractitemdragdrophandler.h"
#include "itemviewtooltip.h"

namespace Digikam
{

// -------------------------------------------------------------------------------

class ItemViewCategorized::Private
{
public:

    explicit Private(ItemViewCategorized* const q) :
        delegate(0),
        toolTip(0),
        notificationToolTip(0),
        showToolTip(false),
        usePointingHand(true),
        scrollStepFactor(10),
        currentMouseEvent(0),
        ensureOneSelectedItem(false),
        ensureInitialSelectedItem(false),
        scrollCurrentToCenter(false),
        mouseButtonPressed(Qt::NoButton),
        hintAtSelectionRow(-1),
        q(q)
    {
    }

    QModelIndex scrollPositionHint() const;

public:

    DItemDelegate*             delegate;
    ItemViewToolTip*           toolTip;
    ItemViewToolTip*           notificationToolTip;
    bool                       showToolTip;
    bool                       usePointingHand;
    int                        scrollStepFactor;

    QMouseEvent*               currentMouseEvent;
    bool                       ensureOneSelectedItem;
    bool                       ensureInitialSelectedItem;
    bool                       scrollCurrentToCenter;
    Qt::MouseButton            mouseButtonPressed;
    QPersistentModelIndex      hintAtSelectionIndex;
    int                        hintAtSelectionRow;
    QPersistentModelIndex      hintAtScrollPosition;

    ItemViewCategorized* const q;
};

QModelIndex ItemViewCategorized::Private::scrollPositionHint() const
{
    if (q->verticalScrollBar()->value() == q->verticalScrollBar()->minimum())
    {
        return QModelIndex();
    }

    QModelIndex hint = q->currentIndex();

    // If the user scrolled, do not take current item, but first visible
    if (!hint.isValid() || !q->viewport()->rect().intersects(q->visualRect(hint)))
    {
        QList<QModelIndex> visibleIndexes = q->categorizedIndexesIn(q->viewport()->rect());

        if (!visibleIndexes.isEmpty())
        {
            hint = visibleIndexes.first();
        }
    }

    return hint;
}

// -------------------------------------------------------------------------------

ItemViewCategorized::ItemViewCategorized(QWidget* const parent)
    : DCategorizedView(parent),
      d(new Private(this))
{
    setViewMode(QListView::IconMode);
    setLayoutDirection(Qt::LeftToRight);
    setFlow(QListView::LeftToRight);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setWrapping(true);
    // important optimization for layouting
    setUniformItemSizes(true);
    // disable "feature" from DCategorizedView
    setDrawDraggedItems(false);

    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setDragEnabled(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    viewport()->setAcceptDrops(true);
    setMouseTracking(true);

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotActivated(QModelIndex)));

    connect(this, SIGNAL(clicked(QModelIndex)),
            this, SLOT(slotClicked(QModelIndex)));

    connect(this, SIGNAL(entered(QModelIndex)),
            this, SLOT(slotEntered(QModelIndex)));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

ItemViewCategorized::~ItemViewCategorized()
{
    delete d;
}

void ItemViewCategorized::setToolTip(ItemViewToolTip* tip)
{
    d->toolTip = tip;
}

void ItemViewCategorized::setItemDelegate(DItemDelegate* delegate)
{
    if (d->delegate == delegate)
    {
        return;
    }

    if (d->delegate)
    {
        disconnect(d->delegate, SIGNAL(gridSizeChanged(QSize)),
                   this, SLOT(slotGridSizeChanged(QSize)));
    }

    d->delegate = delegate;
    DCategorizedView::setItemDelegate(d->delegate);

    connect(d->delegate, SIGNAL(gridSizeChanged(QSize)),
            this, SLOT(slotGridSizeChanged(QSize)));
}

void ItemViewCategorized::setSpacing(int spacing)
{
    d->delegate->setSpacing(spacing);
}

void ItemViewCategorized::setUsePointingHandCursor(bool useCursor)
{
    d->usePointingHand = useCursor;
}

void ItemViewCategorized::setScrollStepGranularity(int factor)
{
    d->scrollStepFactor = qMax(1, factor);
}

DItemDelegate* ItemViewCategorized::delegate() const
{
    return d->delegate;
}

int ItemViewCategorized::numberOfSelectedIndexes() const
{
    return selectedIndexes().size();
}

void ItemViewCategorized::toFirstIndex()
{
    QModelIndex index = moveCursor(MoveHome, Qt::NoModifier);
    clearSelection();
    setCurrentIndex(index);
    scrollToTop();
}

void ItemViewCategorized::toLastIndex()
{
    QModelIndex index = moveCursor(MoveEnd, Qt::NoModifier);
    clearSelection();
    setCurrentIndex(index);
    scrollToBottom();
}

void ItemViewCategorized::toNextIndex()
{
    toIndex(moveCursor(MoveNext, Qt::NoModifier));
}

void ItemViewCategorized::toPreviousIndex()
{
    toIndex(moveCursor(MovePrevious, Qt::NoModifier));
}

void ItemViewCategorized::toIndex(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }

    clearSelection();
    setCurrentIndex(index);
    scrollTo(index);
}

void ItemViewCategorized::awayFromSelection()
{
    QItemSelection selection = selectionModel()->selection();

    if (selection.isEmpty())
    {
        return;
    }

    const QModelIndex first = model()->index(0, 0);
    const QModelIndex last  = model()->index(model()->rowCount() - 1, 0);

    if (selection.contains(first) && selection.contains(last))
    {
        QItemSelection remaining(first, last);
        remaining.merge(selection, QItemSelectionModel::Toggle);
        QList<QModelIndex> indexes = remaining.indexes();

        if (indexes.isEmpty())
        {
            clearSelection();
            setCurrentIndex(QModelIndex());
        }
        else
        {
            toIndex(remaining.indexes().first());
        }
    }
    else if (selection.contains(last))
    {
        setCurrentIndex(selection.indexes().first());
        toPreviousIndex();
    }
    else
    {
        setCurrentIndex(selection.indexes().last());
        toNextIndex();
    }
}

void ItemViewCategorized::scrollToRelaxed(const QModelIndex& index, QAbstractItemView::ScrollHint hint)
{
    if (viewport()->rect().intersects(visualRect(index)))
    {
        return;
    }

    scrollTo(index, hint);
}

void ItemViewCategorized::invertSelection()
{
    const QModelIndex topLeft     = model()->index(0, 0);
    const QModelIndex bottomRight = model()->index(model()->rowCount() - 1, 0);

    const QItemSelection selection(topLeft, bottomRight);
    selectionModel()->select(selection, QItemSelectionModel::Toggle);
}

void ItemViewCategorized::setSelectedIndexes(const QList<QModelIndex>& indexes)
{
    if (selectedIndexes() == indexes)
    {
        return;
    }

    QItemSelection mySelection;

    foreach(const QModelIndex& index, indexes)
    {
        mySelection.select(index, index);
    }

    selectionModel()->select(mySelection, QItemSelectionModel::ClearAndSelect);
}

void ItemViewCategorized::setToolTipEnabled(bool enable)
{
    d->showToolTip = enable;
}

bool ItemViewCategorized::isToolTipEnabled() const
{
    return d->showToolTip;
}

void ItemViewCategorized::slotThemeChanged()
{
    viewport()->update();
}

void ItemViewCategorized::slotSetupChanged()
{
    viewport()->update();
}

void ItemViewCategorized::slotGridSizeChanged(const QSize& gridSize)
{
    setGridSize(gridSize);

    if (!gridSize.isNull())
    {
        horizontalScrollBar()->setSingleStep(gridSize.width() / d->scrollStepFactor);
        verticalScrollBar()->setSingleStep(gridSize.height()  / d->scrollStepFactor);
    }
}

void ItemViewCategorized::updateDelegateSizes()
{
    QStyleOptionViewItem option = viewOptions();
/*
    int frameAroundContents = 0;

    if (style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents))
    {
        frameAroundContents = style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2;
    }

    const int contentWidth  = viewport()->width() - 1
                              - frameAroundContents
                              - style()->pixelMetric(QStyle::PM_ScrollBarExtent, 0, verticalScrollBar());
    const int contentHeight = viewport()->height() - 1
                              - frameAroundContents
                              - style()->pixelMetric(QStyle::PM_ScrollBarExtent, 0, horizontalScrollBar());
    option.rect             = QRect(0, 0, contentWidth, contentHeight);
*/
    option.rect = QRect(QPoint(0, 0), viewport()->size());
    d->delegate->setDefaultViewOptions(option);
}

void ItemViewCategorized::slotActivated(const QModelIndex& index)
{
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;

    if (d->currentMouseEvent)
    {
        // Ignore activation if Ctrl or Shift is pressed (for selection)
        modifiers                       = d->currentMouseEvent->modifiers();
        const bool shiftKeyPressed      = modifiers & Qt::ShiftModifier;
        const bool controlKeyPressed    = modifiers & Qt::ControlModifier;

        if (shiftKeyPressed || controlKeyPressed)
        {
            return;
        }

        const bool rightClick = d->currentMouseEvent->button() & Qt::RightButton;

        if (rightClick)
        {
            return;
        }

        // if the activation is caused by mouse click (not keyboard)
        // we need to check the hot area
        if (d->currentMouseEvent->isAccepted() &&
            !d->delegate->acceptsActivation(d->currentMouseEvent->pos(), visualRect(index), index))
        {
            return;
        }
    }

    d->currentMouseEvent = 0;
    indexActivated(index, modifiers);
}

void ItemViewCategorized::slotClicked(const QModelIndex& index)
{
    if (d->currentMouseEvent)
    {
        emit clicked(d->currentMouseEvent, index);
    }
}

void ItemViewCategorized::slotEntered(const QModelIndex& index)
{
    if (d->currentMouseEvent)
    {
        emit entered(d->currentMouseEvent, index);
    }
}

void ItemViewCategorized::reset()
{
    DCategorizedView::reset();

    // FIXME : Emiting this causes a crash importstackedview, because the model is not yet set.
    //         atm there's a check agaisnt null models though.
    emit selectionChanged();
    emit selectionCleared();

    d->ensureInitialSelectedItem = true;
    d->hintAtScrollPosition      = QModelIndex();
    d->hintAtSelectionIndex      = QModelIndex();
    d->hintAtSelectionRow        = -1;
    verticalScrollBar()->setValue(verticalScrollBar()->minimum());
    horizontalScrollBar()->setValue(horizontalScrollBar()->minimum());
}

void ItemViewCategorized::selectionChanged(const QItemSelection& selectedItems, const QItemSelection& deselectedItems)
{
    DCategorizedView::selectionChanged(selectedItems, deselectedItems);

    emit selectionChanged();

    if (!selectionModel()->hasSelection())
    {
        emit selectionCleared();
    }

    userInteraction();
}

void ItemViewCategorized::rowsInserted(const QModelIndex& parent, int start, int end)
{
    DCategorizedView::rowsInserted(parent, start, end);

    if (start == 0)
    {
        ensureSelectionAfterChanges();
    }
}

void ItemViewCategorized::rowsRemoved(const QModelIndex& parent, int start, int end)
{
    DCategorizedView::rowsRemoved(parent, start, end);

    if (d->scrollCurrentToCenter)
    {
        scrollTo(currentIndex(), QAbstractItemView::PositionAtCenter);
    }
}

void ItemViewCategorized::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    DCategorizedView::rowsAboutToBeRemoved(parent, start, end);

    // Ensure one selected item
    int totalToRemove  = end - start + 1;
    bool remainingRows = model()->rowCount(parent) > totalToRemove;

    if (!remainingRows)
    {
        return;
    }

    QItemSelection removed(model()->index(start, 0), model()->index(end, 0));

    if (selectionModel()->hasSelection())
    {
        // find out which selected indexes are left after rows are removed
        QItemSelection selected = selectionModel()->selection();
        QModelIndex current     = currentIndex();
        QModelIndex indexToAnchor;

        if (selected.contains(current))
        {
            indexToAnchor = current;
        }
        else if (!selected.isEmpty())
        {
            indexToAnchor = selected.first().topLeft();
        }

        selected.merge(removed, QItemSelectionModel::Deselect);

        if (selected.isEmpty())
        {
            QModelIndex newCurrent = nextIndexHint(indexToAnchor, removed.first() /*a range*/);
            setCurrentIndex(newCurrent);
        }
    }

    QModelIndex hint = d->scrollPositionHint();

    if (removed.contains(hint))
    {
        d->hintAtScrollPosition = nextIndexHint(hint, removed.first() /*a range*/);
    }
}

void ItemViewCategorized::layoutAboutToBeChanged()
{
    if(selectionModel())
    {
        d->ensureOneSelectedItem = selectionModel()->hasSelection();
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Called without selection model, check whether the models are ok..";
    }

    QModelIndex current = currentIndex();

    // store some hints so that if all selected items were removed do not need to default to 0,0.
    if (d->ensureOneSelectedItem)
    {
        QItemSelection currentSelection = selectionModel()->selection();
        QModelIndex indexToAnchor;

        if (currentSelection.contains(current))
        {
            indexToAnchor = current;
        }
        else if (!currentSelection.isEmpty())
        {
            indexToAnchor = currentSelection.first().topLeft();
        }

        if (indexToAnchor.isValid())
        {
            d->hintAtSelectionRow   = indexToAnchor.row();
            d->hintAtSelectionIndex = nextIndexHint(indexToAnchor, QItemSelectionRange(indexToAnchor));
        }
    }

    // some precautions to keep current scroll position
    d->hintAtScrollPosition = d->scrollPositionHint();
}

QModelIndex ItemViewCategorized::nextIndexHint(const QModelIndex& indexToAnchor, const QItemSelectionRange& removed) const
{
    Q_UNUSED(indexToAnchor);

    if (removed.bottomRight().row() == model()->rowCount() - 1)
    {
        if (removed.topLeft().row() == 0)
        {
            return QModelIndex();
        }

        return model()->index(removed.topLeft().row() - 1, 0);    // last remaining, no next one left
    }
    else
    {
        return model()->index(removed.bottomRight().row() + 1, 0);    // next remaining
    }
}

void ItemViewCategorized::layoutWasChanged()
{
    // connected queued to layoutChanged()
    ensureSelectionAfterChanges();
    if (d->hintAtScrollPosition.isValid())
    {
        scrollToRelaxed(d->hintAtScrollPosition);
        d->hintAtScrollPosition = QModelIndex();
    }
    else
    {
        scrollToRelaxed(currentIndex());
    }
}

void ItemViewCategorized::userInteraction()
{
    // as soon as the user did anything affecting selection, we don't interfere anymore
    d->ensureInitialSelectedItem = false;
    d->hintAtSelectionIndex      = QModelIndex();
}

void ItemViewCategorized::ensureSelectionAfterChanges()
{
    if (d->ensureInitialSelectedItem && model()->rowCount())
    {
        // Ensure the item (0,0) is selected, if the model was reset previously
        // and the user did not change the selection since reset.
        // Caveat: Item at (0,0) may have changed.
        bool hadInitial              = d->ensureInitialSelectedItem;
        d->ensureInitialSelectedItem = false;
        d->ensureOneSelectedItem     = false;
        QModelIndex index            = model()->index(0,0);

        if (index.isValid())
        {
            selectionModel()->select(index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Clear);
            setCurrentIndex(index);

            // we want ensureInitial set to false if and only if the selection
            // is done from any other place than the previous line (i.e., by user action)
            // Effect: we select whatever is the current index(0,0)
            if (hadInitial)
            {
                d->ensureInitialSelectedItem = true;
            }
        }
    }
    else if (d->ensureOneSelectedItem)
    {
        // ensure we have a selection if there was one before
        d->ensureOneSelectedItem = false;

        if (model()->rowCount() && selectionModel()->selection().isEmpty())
        {
            QModelIndex index;

            if (d->hintAtSelectionIndex.isValid())
            {
                index = d->hintAtSelectionIndex;
            }
            else if (d->hintAtSelectionRow != -1)
            {
                index = model()->index(qMin(model()->rowCount(), d->hintAtSelectionRow), 0);
            }
            else
            {
                index = currentIndex();
            }

            if (!index.isValid())
            {
                index = model()->index(0,0);
            }

            d->hintAtSelectionRow   = -1;
            d->hintAtSelectionIndex = QModelIndex();

            if (index.isValid())
            {
                setCurrentIndex(index);
                selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
            }
        }
    }
}

QModelIndex ItemViewCategorized::indexForCategoryAt(const QPoint& pos) const
{
    return categoryAt(pos);
}

QModelIndex ItemViewCategorized::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    QModelIndex current = currentIndex();

    if (!current.isValid())
    {
        return DCategorizedView::moveCursor(cursorAction, modifiers);
    }

    // We want a simple wrapping navigation.
    // Default behavior we do not want: right/left does never change row; Next/Previous is equivalent to Down/Up
    switch (cursorAction)
    {
        case MoveNext:
        case MoveRight:
        {
            QModelIndex next = model()->index(current.row() + 1, 0);

            if (next.isValid())
            {
                return next;
            }
            else
            {
                return current;
            }

            break;
        }
        case MovePrevious:
        case MoveLeft:
        {
            QModelIndex previous = model()->index(current.row() - 1, 0);

            if (previous.isValid())
            {
                return previous;
            }
            else
            {
                return current;
            }

            break;
        }
        default:
            break;
    }

    return DCategorizedView::moveCursor(cursorAction, modifiers);
}


void ItemViewCategorized::showContextMenuOnIndex(QContextMenuEvent*, const QModelIndex&)
{
    // implemented in subclass
}

void ItemViewCategorized::showContextMenu(QContextMenuEvent*)
{
    // implemented in subclass
}

void ItemViewCategorized::indexActivated(const QModelIndex&, Qt::KeyboardModifiers)
{
}

bool ItemViewCategorized::showToolTip(const QModelIndex& index, QStyleOptionViewItem& option, QHelpEvent* he)
{
    QRect  innerRect;
    QPoint pos;

    if (he)
    {
        pos = he->pos();
    }
    else
    {
        pos = option.rect.center();
    }

    if (d->delegate->acceptsToolTip(he->pos(), option.rect, index, &innerRect))
    {
        if (!innerRect.isNull())
        {
            option.rect = innerRect;
        }

        d->toolTip->show(option, index);
        return true;
    }

    return false;
}

void ItemViewCategorized::contextMenuEvent(QContextMenuEvent* event)
{
    userInteraction();
    QModelIndex index = indexAt(event->pos());

    if (index.isValid())
    {
        showContextMenuOnIndex(event, index);
    }
    else
    {
        showContextMenu(event);
    }
}

void ItemViewCategorized::leaveEvent(QEvent*)
{
    hideIndexNotification();

    if (d->mouseButtonPressed != Qt::RightButton)
    {
        d->mouseButtonPressed = Qt::NoButton;
    }
}

void ItemViewCategorized::mousePressEvent(QMouseEvent* event)
{
    userInteraction();

    const QModelIndex index         = indexAt(event->pos());

    // Clear selection on click on empty area. Standard behavior, but not done by QAbstractItemView for some reason.
    Qt::KeyboardModifiers modifiers = event->modifiers();
    const Qt::MouseButton button    = event->button();
    const bool rightButtonPressed   = button & Qt::RightButton;
    const bool shiftKeyPressed      = modifiers & Qt::ShiftModifier;
    const bool controlKeyPressed    = modifiers & Qt::ControlModifier;
    d->mouseButtonPressed           = button;

    if (!index.isValid() && !rightButtonPressed && !shiftKeyPressed && !controlKeyPressed)
    {
        clearSelection();
    }

    // store event for entered(), clicked(), activated() signal handlers
    if (!rightButtonPressed)
    {
        d->currentMouseEvent = event;
    }
    else
    {
        d->currentMouseEvent = 0;
    }

    DCategorizedView::mousePressEvent(event);

    if (!index.isValid())
    {
        emit viewportClicked(event);
    }
}

void ItemViewCategorized::mouseReleaseEvent(QMouseEvent* event)
{
    userInteraction();

    if (d->scrollCurrentToCenter)
    {
        scrollTo(currentIndex(), QAbstractItemView::PositionAtCenter);
    }

    DCategorizedView::mouseReleaseEvent(event);
}

void ItemViewCategorized::mouseMoveEvent(QMouseEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    QRect indexVisualRect;

    if (index.isValid())
    {
        indexVisualRect = visualRect(index);

        if (d->usePointingHand &&
            d->delegate->acceptsActivation(event->pos(), indexVisualRect, index))
        {
            setCursor(Qt::PointingHandCursor);
        }
        else
        {
            unsetCursor();
        }
    }
    else
    {
        unsetCursor();
    }

    if (d->notificationToolTip && d->notificationToolTip->isVisible())
    {
        if (!d->notificationToolTip->rect().adjusted(-50, -50, 50, 50).contains(event->pos()))
        {
            hideIndexNotification();
        }
    }

    DCategorizedView::mouseMoveEvent(event);

    d->delegate->mouseMoved(event, indexVisualRect, index);
}

void ItemViewCategorized::wheelEvent(QWheelEvent* event)
{
    // DCategorizedView updates the single step at some occasions in a private methody
    horizontalScrollBar()->setSingleStep(d->delegate->gridSize().height() / d->scrollStepFactor);
    verticalScrollBar()->setSingleStep(d->delegate->gridSize().width()    / d->scrollStepFactor);

    if (event->modifiers() & Qt::ControlModifier)
    {
        const int delta = event->delta();

        if (delta > 0)
        {
            emit zoomInStep();
        }
        else if (delta < 0)
        {
            emit zoomOutStep();
        }

        event->accept();
        return;
    }

    if (verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff && event->orientation() == Qt::Vertical)
    {
        QWheelEvent n(event->pos(), event->globalPos(), event->delta(),
                      event->buttons(), event->modifiers(), Qt::Horizontal);
        QApplication::sendEvent(horizontalScrollBar(), &n);
        event->setAccepted(n.isAccepted());
    }
    else
    {
        DCategorizedView::wheelEvent(event);
    }
}

void ItemViewCategorized::keyPressEvent(QKeyEvent* event)
{
    userInteraction();

    if (event == QKeySequence::Copy)
    {
        copy();
        event->accept();
        return;
    }
    else if (event == QKeySequence::Paste)
    {
        paste();
        event->accept();
        return;
    }

/*
    // from dolphincontroller.cpp
    const QItemSelectionModel* selModel = m_itemView->selectionModel();
    const QModelIndex currentIndex      = selModel->currentIndex();
    const bool trigger                  = currentIndex.isValid() &&
                                          ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)) &&
                                          (selModel->selectedIndexes().count() > 0);
    if (trigger)
    {
        const QModelIndexList indexList = selModel->selectedIndexes();
        foreach(const QModelIndex& index, indexList)
        {
            emit itemTriggered(itemForIndex(index));
        }
    }
*/
    DCategorizedView::keyPressEvent(event);

    emit keyPressed(event);
}

void ItemViewCategorized::resizeEvent(QResizeEvent* e)
{
    QModelIndex oldPosition = d->scrollPositionHint();
    DCategorizedView::resizeEvent(e);
    updateDelegateSizes();
    scrollToRelaxed(oldPosition, QAbstractItemView::PositionAtTop);
}

bool ItemViewCategorized::viewportEvent(QEvent* event)
{
    switch (event->type())
    {
        case QEvent::FontChange:
        {
            updateDelegateSizes();
            break;
        }
        case QEvent::ToolTip:
        {
            if (!d->showToolTip)
            {
                return true;
            }

            QHelpEvent* he          = static_cast<QHelpEvent*>(event);
            const QModelIndex index = indexAt(he->pos());

            if (!index.isValid())
            {
                break;
            }

            QStyleOptionViewItem option =  viewOptions();
            option.rect                 =  visualRect(index);
            option.state               |= (index == currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);
            showToolTip(index, option, he);
            return true;
        }
        default:
            break;
    }

    return DCategorizedView::viewportEvent(event);
}

void ItemViewCategorized::showIndexNotification(const QModelIndex& index, const QString& message)
{
    hideIndexNotification();

    if (!index.isValid())
    {
        return;
    }

    if (!d->notificationToolTip)
    {
        d->notificationToolTip = new ItemViewToolTip(this);
    }

    d->notificationToolTip->setTipContents(message);

    QStyleOptionViewItem option =  viewOptions();
    option.rect                 =  visualRect(index);
    option.state               |= (index == currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);
    d->notificationToolTip->show(option, index);
}

void ItemViewCategorized::hideIndexNotification()
{
    if (d->notificationToolTip)
    {
        d->notificationToolTip->hide();
    }
}

/**
 * cut(), copy(), paste(), dragEnterEvent(), dragMoveEvent(), dropEvent(), startDrag()
 * are implemented by DragDropViewImplementation
 */
QModelIndex ItemViewCategorized::mapIndexForDragDrop(const QModelIndex& index) const
{
    return filterModel()->mapToSource(index);
}

QPixmap ItemViewCategorized::pixmapForDrag(const QList<QModelIndex>& indexes) const
{
    QStyleOptionViewItem option = viewOptions();
    option.rect                 = viewport()->rect();
    return d->delegate->pixmapForDrag(option, indexes);
}

void ItemViewCategorized::setScrollCurrentToCenter(bool enabled)
{
    d->scrollCurrentToCenter = enabled;
}

void ItemViewCategorized::scrollTo(const QModelIndex& index, ScrollHint hint)
{
    if (d->scrollCurrentToCenter && d->mouseButtonPressed == Qt::NoButton)
    {
        hint = QAbstractItemView::PositionAtCenter;
    }

    d->mouseButtonPressed = Qt::NoButton;
    DCategorizedView::scrollTo(index, hint);
}

} // namespace Digikam
