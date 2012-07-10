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

#include "icategorizedview.moc"

// Qt includes

#include <QHelpEvent>
#include <QScrollBar>
#include <QSortFilterProxyModel>

// KDE includes

#include <KApplication>
#include <KGlobalSettings>

// Local includes

#include "thememanager.h"

namespace Digikam
{

class ICategorizedView::Private
{
public:

    Private(ICategorizedView* const q) :
        delegate(0),
        currentMouseEvent(0),
        usePointingHand(true),
        scrollStepFactor(10),
        toolTip(0),
        notificationToolTip(0),
        showToolTip(false),
        hintAtSelectionRow(-1),
        ensureOneSelectedItem(false),
        ensureInitialSelectedItem(false),
        q(q)
    {
    }

    QModelIndex scrollPositionHint() const;

public:

    DItemDelegate*              delegate;
    QMouseEvent*                currentMouseEvent;
    bool                        usePointingHand;
    int                         scrollStepFactor;
    ItemViewToolTip*            toolTip;
    ItemViewToolTip*            notificationToolTip;
    bool                        showToolTip;

    QPersistentModelIndex       hintAtSelectionIndex;
    QPersistentModelIndex       hintAtScrollPosition;
    int                         hintAtSelectionRow;
    bool                        ensureOneSelectedItem;
    bool                        ensureInitialSelectedItem;

    ICategorizedView* const     q;
};

QModelIndex ICategorizedView::Private::scrollPositionHint() const
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

ICategorizedView::ICategorizedView(QWidget* const parent)
    : ImportKCategorizedView(parent), d(new Private(this))
{
    setViewMode(QListView::IconMode);
    setLayoutDirection(Qt::LeftToRight);
    setFlow(QListView::LeftToRight);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setWrapping(true);

    // important optimization for layouting
    setUniformItemSizes(true);
    // disable "feature" from ImportKCategorizedView
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

ICategorizedView::~ICategorizedView()
{
    delete d;
}

int ICategorizedView::numberOfSelectedIndexes() const
{
    return selectedIndexes().size();
}

void ICategorizedView::toFirstIndex()
{
    QModelIndex index = moveCursor(MoveHome, Qt::NoModifier);
    clearSelection();
    setCurrentIndex(index);
    scrollToTop();
}

void ICategorizedView::toLastIndex()
{
    QModelIndex index = moveCursor(MoveEnd, Qt::NoModifier);
    clearSelection();
    setCurrentIndex(index);
    scrollToTop();
}

void ICategorizedView::toNextIndex()
{
    toIndex(moveCursor(MoveNext, Qt::NoModifier));
}

void ICategorizedView::toPreviousIndex()
{
    toIndex(moveCursor(MovePrevious, Qt::NoModifier));
}

void ICategorizedView::toIndex(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }

    clearSelection();
    setCurrentIndex(index);
    scrollTo(index);
}

void ICategorizedView::awayFromSelection()
{
    QItemSelection selection = selectionModel()->selection();

    if (selection.isEmpty())
    {
        return;
    }

    const QModelIndex first = model()->index(0, 0);
    const QModelIndex last  = model()->index(model()->rowCount() -1, 0);

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

void ICategorizedView::scrollToRelaxed(const QModelIndex& index, ScrollHint hint)
{
    if (viewport()->rect().intersects(visualRect(index)))
    {
        return;
    }

    scrollTo(index, hint);
}

void ICategorizedView::setScrollStepGranularity(int factor)
{
    d->scrollStepFactor = qMax(1, factor);
}

void ICategorizedView::setSelectedIndexes(const QList<QModelIndex>& indexes)
{
    if (selectedIndexes() == indexes)
    {
        return;
    }

    QItemSelection mySelection;

    foreach (const QModelIndex& index, indexes)
    {
        mySelection.select(index, index);
    }

    selectionModel()->select(mySelection, QItemSelectionModel::ClearAndSelect);
}

void ICategorizedView::invertSelection()
{
    const QModelIndex topLeft     = model()->index(0, 0);
    const QModelIndex bottomRight = model()->index(model()->rowCount() -1, 0);

    const QItemSelection selection(topLeft, bottomRight);
    selectionModel()->select(selection, QItemSelectionModel::Toggle);
}

void ICategorizedView::setSpacing(int space)
{
    d->delegate->setSpacing(space);
}

void ICategorizedView::setToolTipEnabled(bool enabled)
{
    d->showToolTip;
}

bool ICategorizedView::isToolTipEnabled() const
{
    return d->showToolTip;
}

void ICategorizedView::setUsePointingHandCursor(bool useCursor)
{
    d->usePointingHand = useCursor;
}

void ICategorizedView::showIndexNotification(const QModelIndex& index, const QString& message)
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
    option.state                |= (index == currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);
    d->notificationToolTip->show(option, index);
}

void ICategorizedView::hideIndexNotification()
{
    if (d->notificationToolTip)
    {
        d->notificationToolTip->hide();
    }
}

void ICategorizedView::slotActivated(const QModelIndex& index)
{
    if (d->currentMouseEvent)
    {
        // ignore activation if Ctrl or Shift is pressed (for selection)
        Qt::KeyboardModifiers modifiers = d->currentMouseEvent->modifiers();
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
        if (!d->delegate->acceptsActivation(d->currentMouseEvent->pos(), visualRect(index), index))
        {
            return;
        }
    }

    indexActivated(index);
}

void ICategorizedView::slotClicked(const QModelIndex& index)
{
    if (d->currentMouseEvent)
    {
        emit clicked(d->currentMouseEvent, index);
    }
}

void ICategorizedView::slotEntered(const QModelIndex& index)
{
    if (d->currentMouseEvent)
    {
        emit entered(d->currentMouseEvent, index);
    }
}


void ICategorizedView::layoutAboutToBeChanged()
{
    d->ensureOneSelectedItem = selectionModel()->hasSelection();
    QModelIndex current      = currentIndex();

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
            d->hintAtSelectionRow = indexToAnchor.row();
            d->hintAtSelectionIndex = nextIndexHint(indexToAnchor, QItemSelectionRange(indexToAnchor));
        }
    }

    // some precautions to keep current scroll position
    d->hintAtScrollPosition = d->scrollPositionHint();
}

void ICategorizedView::layoutWasChanged()
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

void ICategorizedView::slotThemeChanged()
{
    viewport()->update();
}

void ICategorizedView::slotSetupChanged()
{
    viewport()->update();
}

bool ICategorizedView::showToolTip(const QModelIndex& index, QStyleOptionViewItem& option, QHelpEvent* he)
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

    if (d->delegate->acceptsToolTip(he->pos(), option.rect, index,& innerRect))
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

void ICategorizedView::setItemDelegate(DItemDelegate* delegate)
{
    if (d->delegate == delegate)
    {
        return;
    }

    if (d->delegate)
    {
        disconnect(d->delegate, SIGNAL(gridSizeChanged(QSize)), this, SLOT(slotGridSizeChanged(QSize)));
    }

    d->delegate = delegate;
    ImportKCategorizedView::setItemDelegate(d->delegate);

    connect(d->delegate, SIGNAL(gridSizeChanged(QSize)), this, SLOT(slotGridSizeChanged(QSize)));
}

void ICategorizedView::contextMenuEvent(QContextMenuEvent* event)
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

void ICategorizedView::keyPressEvent(QKeyEvent* event)
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

    ImportKCategorizedView::keyPressEvent(event);

    emit keyPressed(event);
}

void ICategorizedView::mousePressEvent(QMouseEvent* event)
{
    userInteraction();
    const QModelIndex index         = indexAt(event->pos());

    // Clear selection on click on empty area. Standard behavior, but not done by QAbstractItemView for some reason.
    Qt::KeyboardModifiers modifiers = event->modifiers();
    const Qt::MouseButton button    = event->button();
    const bool rightButtonPressed   = button & Qt::RightButton;
    const bool shiftKeyPressed      = modifiers & Qt::ShiftModifier;
    const bool controlKeyPressed    = modifiers & Qt::ControlModifier;

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

    ImportKCategorizedView::mousePressEvent(event);
    if (!index.isValid())
    {
        emit viewportClicked(event);
    }
    d->currentMouseEvent = 0;
}

void ICategorizedView::mouseReleaseEvent(QMouseEvent* event)
{
    userInteraction();
    d->currentMouseEvent = event;
    ImportKCategorizedView::mouseReleaseEvent(event);
    d->currentMouseEvent = 0;
}

void ICategorizedView::mouseMoveEvent(QMouseEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    QRect indexVisualRect;

    if (index.isValid())
    {
        indexVisualRect = visualRect(index);

        if (d->usePointingHand &&
            KGlobalSettings::changeCursorOverIcon() &&
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

    d->currentMouseEvent = event;
    ImportKCategorizedView::mouseMoveEvent(event);
    d->currentMouseEvent = 0;

    d->delegate->mouseMoved(event, indexVisualRect, index);
}

void ICategorizedView::resizeEvent(QResizeEvent* e)
{
    QModelIndex oldPosition = d->scrollPositionHint();
    ImportKCategorizedView::resizeEvent(e);
    updateDelegateSizes();
    scrollToRelaxed(oldPosition, QAbstractItemView::PositionAtTop);
}

void ICategorizedView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    ImportKCategorizedView::rowsAboutToBeRemoved(parent, start, end);

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

void ICategorizedView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    ImportKCategorizedView::rowsInserted(parent, start, end);

    if (start == 0)
    {
        ensureSelectionAfterChanges();
    }
}

void ICategorizedView::reset()
{
    ImportKCategorizedView::reset();

    emit selectionChanged();
    emit selectionCleared();

    d->ensureInitialSelectedItem = true;
    d->hintAtScrollPosition      = QModelIndex();
    d->hintAtSelectionIndex      = QModelIndex();
    d->hintAtSelectionRow        = -1;
    verticalScrollBar()->setValue(verticalScrollBar()->minimum());
    horizontalScrollBar()->setValue(horizontalScrollBar()->minimum());
}

void ICategorizedView::selectionChanged(const QItemSelection& selectedItems, const QItemSelection& deselectedItems)
{
    ImportKCategorizedView::selectionChanged(selectedItems, deselectedItems);

    emit selectionChanged();

    if (!selectionModel()->hasSelection())
    {
        emit selectionCleared();
    }

    userInteraction();
}

void ICategorizedView::wheelEvent(QWheelEvent* event)
{
    // ImportKCategorizedView updates the single step at some occasions in a private method
    horizontalScrollBar()->setSingleStep(d->delegate->gridSize().height() / d->scrollStepFactor);
    verticalScrollBar()->setSingleStep(d->delegate->gridSize().width() / d->scrollStepFactor);

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
        ImportKCategorizedView::wheelEvent(event);
    }
}

bool ICategorizedView::viewportEvent(QEvent* event)
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
            option.state                |= (index == currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);
            showToolTip(index, option, he);
            return true;
        }
        default:
            break;
    }

    return ImportKCategorizedView::viewportEvent(event);
}

QModelIndex ICategorizedView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    QModelIndex current = currentIndex();

    if (!current.isValid())
    {
        return ImportKCategorizedView::moveCursor(cursorAction, modifiers);
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

    return ImportKCategorizedView::moveCursor(cursorAction, modifiers);
}


void ICategorizedView::showContextMenuOnIndex(QContextMenuEvent*, const QModelIndex&)
{
    // implemented in subclass
}

void ICategorizedView::showContextMenu(QContextMenuEvent*)
{
    // implemented in subclass
}

void ICategorizedView::indexActivated(const QModelIndex&)
{
}

QModelIndex ICategorizedView::mapIndexForDragDrop(const QModelIndex& index) const
{
    return filterModel()->mapToSource(index);
}

QPixmap ICategorizedView::pixmapForDrag(const QList<QModelIndex>& indexes) const
{
    QStyleOptionViewItem option = viewOptions();
    option.rect                 = viewport()->rect();
    return d->delegate->pixmapForDrag(option, indexes);
}

QModelIndex ICategorizedView::nextIndexHint(const QModelIndex& indexToAnchor, const QItemSelectionRange& removed) const
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

void ICategorizedView::setToolTip(ItemViewToolTip* tip)
{
    d->toolTip = tip;
}

void ICategorizedView::updateDelegateSizes()
{
    QStyleOptionViewItem option = viewOptions();
    option.rect = QRect(QPoint(0,0), viewport()->size());
    d->delegate->setDefaultViewOptions(option);
}

void ICategorizedView::userInteraction()
{
    // as soon as the user did anything affecting selection, we don't interfere anymore
    d->ensureInitialSelectedItem = false;
    d->hintAtSelectionIndex      = QModelIndex();
}


QModelIndex ICategorizedView::indexForCategoryAt(const QPoint& pos) const
{
    return categoryAt(pos);
}

void ICategorizedView::slotGridSizeChanged(const QSize& size)
{
    setGridSize(size);

    if (!size.isNull())
    {
        horizontalScrollBar()->setSingleStep(size.width() / d->scrollStepFactor);
        verticalScrollBar()->setSingleStep(size.height() / d->scrollStepFactor);
    }
}

void ICategorizedView::ensureSelectionAfterChanges()
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

            d->hintAtSelectionRow = -1;
            d->hintAtSelectionIndex = QModelIndex();

            if (index.isValid())
            {
                setCurrentIndex(index);
                selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
            }
        }
    }
}

} // namespace Digikam
