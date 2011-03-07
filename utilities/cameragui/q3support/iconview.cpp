/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-24
 * Description : icons view.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#define RECT_EXTENSION 300

#include "iconview.moc"

// C++ includes

#include <climits>
#include <cstdlib>

// Qt includes

#include <QCache>
#include <QList>
#include <QSet>
#include <QTimer>
#include <QPainter>
#include <QStyle>
#include <QApplication>
#include <QPaintEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QStyleOption>
#include <QRubberBand>

// KDE includes


#include <kcursor.h>
#include <kiconloader.h>
#include <kglobalsettings.h>

// Local includes

#include "ratingwidget.h"
#include "drubberband.h"
#include "thumbbar.h"
#include "iconitem.h"
#include "icongroupitem.h"
#include "albumsettings.h"

namespace Digikam
{

class IconView::IconViewPriv
{
public:

    IconViewPriv()
    {
        firstGroup               = 0;
        lastGroup                = 0;
        currItem                 = 0;
        highlightedItem          = 0;
        anchorItem               = 0;
        clearing                 = false;
        spacing                  = 10;
        ratingWidget             = 0;
        ratingItem               = 0;

        rubber                   = 0;
        dragging                 = false;
        pressedMoved             = false;

        firstContainer           = 0;
        lastContainer            = 0;

        showTips                 = false;
        toolTipItem              = 0;
        toolTipTimer             = 0;
        rearrangeTimer           = 0;
        rearrangeTimerInterval   = 0;
        storedVisibleItem        = 0;
        needEmitSelectionChanged = false;

        thumbnailBorderCache.setMaxCost(10);

        selectPix   = SmallIcon("list-add");
        deselectPix = SmallIcon("list-remove");
    }

    bool                      clearing;
    bool                      showTips;
    bool                      pressedMoved;
    bool                      dragging;
    bool                      needEmitSelectionChanged; // store for slotRearrange

    int                       rearrangeTimerInterval;
    int                       spacing;

    QSet<IconItem*>           selectedItems;
    QSet<IconItem*>           prevSelectedItems;

    QPixmap                   selectPix;
    QPixmap                   deselectPix;

    QCache<QString, QPixmap>  thumbnailBorderCache;

    DRubberBand*              rubber;

    QPoint                    dragStartPos;

    QTimer*                   rearrangeTimer;
    QTimer*                   toolTipTimer;

    IconItem*                 toolTipItem;
    IconItem*                 currItem;
    IconItem*                 anchorItem;
    IconItem*                 storedVisibleItem; // store position for slotRearrange
    IconItem*                 highlightedItem;
    IconItem*                 ratingItem;

    IconGroupItem*            firstGroup;
    IconGroupItem*            lastGroup;

    RatingWidget*             ratingWidget;

    struct ItemContainer
    {
        ItemContainer(ItemContainer* p, ItemContainer* n, const QRect& r)
            : prev(p), next(n), rect(r)
        {
            if (prev)
            {
                prev->next = this;
            }

            if (next)
            {
                next->prev = this;
            }
        }

        ItemContainer*        prev, *next;
        QRect                 rect;
        QList<IconItem*>      items;
    } *firstContainer, *lastContainer;

    struct SortableItem
    {
        IconGroupItem* group;
    };
};

// -----------------------------------------------------------------------

IconView::IconView(QWidget* parent, const char* name)
    : Q3ScrollView(parent), d(new IconViewPriv)
{
    setObjectName(name);
    setWindowFlags(Qt::WStaticContents|Qt::WNoAutoErase);

    viewport()->setFocusProxy(this);
    viewport()->setFocusPolicy(Qt::WheelFocus);
    viewport()->setMouseTracking(true);

    d->rearrangeTimer = new QTimer(this);
    d->toolTipTimer   = new QTimer(this);
    d->rubber         = new DRubberBand(this);
    d->ratingWidget   = new RatingWidget(viewport());
    d->ratingWidget->setTracking(false);
    d->ratingWidget->hide();

    connect(d->rearrangeTimer, SIGNAL(timeout()),
            this, SLOT(slotRearrange()));

    connect(d->toolTipTimer, SIGNAL(timeout()),
            this, SLOT(slotToolTip()));

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotIconViewFontChanged()));

    connect(d->ratingWidget, SIGNAL(signalRatingChanged(int)),
            this, SLOT(slotEditRatingFromItem(int)));

    slotIconViewFontChanged();
    setEnableToolTips(true);
}

IconView::~IconView()
{
    clear(false);

    delete d->rearrangeTimer;
    delete d->toolTipTimer;
    delete d->rubber;
    delete d;
}

IconGroupItem* IconView::firstGroup() const
{
    return d->firstGroup;
}

IconGroupItem* IconView::lastGroup() const
{
    return d->lastGroup;
}

IconItem* IconView::firstItem() const
{
    if (!d->firstGroup)
    {
        return 0;
    }

    return d->firstGroup->firstItem();
}

IconItem* IconView::lastItem() const
{
    if (!d->lastGroup)
    {
        return 0;
    }

    return d->lastGroup->lastItem();
}

IconItem* IconView::currentItem() const
{
    return d->currItem;
}

void IconView::setCurrentItem(IconItem* item)
{
    d->currItem   = item;
    d->anchorItem = d->currItem;

    if (d->currItem)
    {
        d->currItem->setSelected(true, true);
        ensureItemVisible(d->currItem);
    }
}

IconItem* IconView::ratingItem() const
{
    return d->ratingItem;
}

IconItem* IconView::findItem(const QPoint& pos)
{
    IconViewPriv::ItemContainer* c = d->firstContainer;

    for (; c; c = c->next)
    {
        if ( c->rect.contains(pos) )
        {
            foreach (IconItem* item, c->items)
            {
                if (item->rect().contains(pos))
                {
                    return item;
                }
            }
        }
    }

    return 0;
}

IconGroupItem* IconView::findGroup(const QPoint& pos)
{
    QPoint p = viewportToContents(viewport()->mapFromGlobal(pos));

    for (IconGroupItem* group = d->firstGroup; group; group = group->nextGroup())
    {
        QRect rect = group->rect();
        int bottom;

        if (group == d->lastGroup)
        {
            bottom = contentsHeight();
        }
        else
        {
            bottom = group->nextGroup()->rect().top();
        }

        rect.setBottom(bottom);

        if ( rect.contains(p) )
        {
            return group;
        }
    }

    return 0;
}

int IconView::count() const
{
    int c = 0;

    for (IconGroupItem* group = d->firstGroup; group; group = group->nextGroup())
    {
        c += group->count();
    }

    return c;
}


int IconView::countSelected() const
{
    int c = 0;

    for (IconGroupItem* group = d->firstGroup; group; group = group->nextGroup())
    {
        for (IconItem* it = group->firstItem(); it; it = it->nextItem())
            if (it->isSelected())
            {
                ++c;
            }
    }

    return c;
}

int IconView::groupCount() const
{
    int c = 0;

    for (IconGroupItem* group = d->firstGroup; group; group = group->nextGroup())
    {
        ++c;
    }

    return c;
}

void IconView::clear(bool update)
{
    d->clearing        = true;
    d->highlightedItem = 0;
    d->toolTipItem     = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    if (d->ratingItem)
    {
        d->ratingItem->setEditRating(false);
        d->ratingItem = 0;
        unsetCursor();
        d->ratingWidget->hide();
    }

    deleteContainers();

    d->selectedItems.clear();

    IconGroupItem* group = d->firstGroup;

    while (group)
    {
        IconGroupItem* tmp = group->m_next;
        delete group;
        group = tmp;
    }

    d->firstGroup = 0;
    d->lastGroup  = 0;
    d->currItem   = 0;
    d->anchorItem = 0;

    viewport()->setUpdatesEnabled(false);
    resizeContents(0, 0);
    setContentsPos(0, 0);
    viewport()->setUpdatesEnabled(true);

    if (update)
    {
        updateContents();
    }

    d->clearing = false;

    emit signalSelectionChanged();
}

void IconView::clearSelection()
{
    bool wasBlocked = signalsBlocked();

    if (!wasBlocked)
    {
        blockSignals(true);
    }

    QSet<IconItem*> selItems = d->selectedItems;
    foreach (IconItem* item, selItems)
    {
        item->setSelected(false, false);
    }

    d->selectedItems.clear();

    if (!wasBlocked)
    {
        blockSignals(false);
    }

    emit signalSelectionChanged();
}

void IconView::selectAll()
{
    bool wasBlocked = signalsBlocked();

    if (!wasBlocked)
    {
        blockSignals(true);
    }

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (!item->isSelected())
        {
            item->setSelected(true, false);
        }
    }

    if (!wasBlocked)
    {
        blockSignals(false);
    }

    emit signalSelectionChanged();
}

void IconView::invertSelection()
{
    bool wasBlocked = signalsBlocked();

    if (!wasBlocked)
    {
        blockSignals(true);
    }

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (!item->isSelected())
        {
            item->setSelected(true, false);
        }
        else
        {
            item->setSelected(false, false);
        }
    }

    if (!wasBlocked)
    {
        blockSignals(false);
    }

    emit signalSelectionChanged();
}

void IconView::selectItem(IconItem* item, bool select)
{
    if (!item)
    {
        return;
    }

    if (select)
    {
        d->selectedItems.insert(item);
    }
    else
    {
        d->selectedItems.remove(item);
    }

    emit signalSelectionChanged();
}

void IconView::setStoredVisibleItem(IconItem* item)
{
    d->storedVisibleItem = item;
}

void IconView::insertGroup(IconGroupItem* group)
{
    if (!group)
    {
        return;
    }

    if (!d->firstGroup)
    {
        d->firstGroup = group;
        d->lastGroup  = group;
        group->m_prev = 0;
        group->m_next = 0;
    }
    else
    {
        d->lastGroup->m_next = group;
        group->m_prev        = d->lastGroup;
        group->m_next        = 0;
        d->lastGroup         = group;
    }

    d->storedVisibleItem = findFirstVisibleItem();
    startRearrangeTimer();
}

void IconView::takeGroup(IconGroupItem* group)
{
    if (!group)
    {
        return;
    }

    // this is only to find an alternative visible item if all visible items
    // are removed
    IconGroupItem* alternativeVisibleGroup = 0;
    d->storedVisibleItem = 0;

    if (group == d->firstGroup)
    {
        d->firstGroup = d->firstGroup->m_next;

        if (d->firstGroup)
        {
            d->firstGroup->m_prev = 0;
        }
        else
        {
            d->firstGroup = d->lastGroup = 0;
        }

        alternativeVisibleGroup = d->firstGroup;
    }
    else if (group == d->lastGroup)
    {
        d->lastGroup = d->lastGroup->m_prev;

        if (d->lastGroup)
        {
            d->lastGroup->m_next = 0;
        }
        else
        {
            d->firstGroup = d->lastGroup = 0;
        }

        alternativeVisibleGroup = d->lastGroup->m_prev;
    }
    else
    {
        IconGroupItem* i = group;

        if (i)
        {
            if (i->m_prev)
            {
                i->m_prev->m_next = i->m_next;
            }

            if (i->m_next)
            {
                i->m_next->m_prev = i->m_prev;
            }

            if (i->m_prev)
            {
                alternativeVisibleGroup = i->m_prev;
            }
            else
            {
                alternativeVisibleGroup = i->m_next;
            }
        }
    }

    if (!d->clearing)
    {
        d->storedVisibleItem = findFirstVisibleItem();

        if (!d->storedVisibleItem && alternativeVisibleGroup)
        {
            // find an alternative visible item
            d->storedVisibleItem = alternativeVisibleGroup->lastItem();
        }

        startRearrangeTimer();
    }
}

void IconView::insertItem(IconItem* item)
{
    if (!item)
    {
        return;
    }

    d->storedVisibleItem = findFirstVisibleItem();
    startRearrangeTimer();
}

void IconView::takeItem(IconItem* item)
{
    if (!item)
    {
        return;
    }

    // First remove item from any containers holding it
    IconViewPriv::ItemContainer* tmp = d->firstContainer;

    while (tmp)
    {
        tmp->items.removeAll(item);
        tmp = tmp->next;
    }

    // Remove from selected item list
    d->selectedItems.remove(item);

    // See bug 161084
    if (d->selectedItems.count() || item->isSelected())
    {
        d->needEmitSelectionChanged = true;
    }

    if (d->toolTipItem == item)
    {
        d->toolTipItem = 0;
        d->toolTipTimer->stop();
        slotToolTip();
    }

    if (d->highlightedItem == item)
    {
        d->highlightedItem = 0;
    }

    if (d->ratingItem == item)
    {
        d->ratingItem->setEditRating(false);
        d->ratingItem = 0;
        unsetCursor();
        d->ratingWidget->hide();
    }

    // if it is current item, change the current item
    if (d->currItem == item)
    {
        d->currItem = item->nextItem();

        if (!d->currItem)
        {
            d->currItem = item->prevItem();
            // defer calling d->currItem->setSelected (and emitting the signals) to slotRearrange
        }
    }

    d->anchorItem = d->currItem;

    if (!d->clearing)
    {
        d->storedVisibleItem = findFirstVisibleItem();

        if (d->storedVisibleItem == item)
        {
            d->storedVisibleItem = d->currItem;
        }

        startRearrangeTimer();
    }
}

void IconView::triggerRearrangement()
{
    d->storedVisibleItem = findFirstVisibleItem();
    startRearrangeTimer();
}

void IconView::setDelayedRearrangement(bool delayed)
{
    // if it is known that e.g. several items will be added or deleted in the next time,
    // but not from the same event queue thread stack location, it may be desirable to delay
    // the rearrangeTimer a bit
    if (delayed)
    {
        d->rearrangeTimerInterval = 50;
    }
    else
    {
        d->rearrangeTimerInterval = 0;
    }
}

void IconView::startRearrangeTimer()
{
    // We want to reduce the number of updates, but not remove all updates
    if (!d->rearrangeTimer->isActive())
    {
        d->rearrangeTimer->setSingleShot(true);
        d->rearrangeTimer->start(d->rearrangeTimerInterval);
    }
}

void IconView::sort()
{
    // first sort the groups
    for (IconGroupItem* group = d->firstGroup; group;
         group = group->nextGroup())
    {
        group->sort();
    }

    int gcount = groupCount();

    // then sort the groups themselves
    IconViewPriv::SortableItem* groups = new IconViewPriv::SortableItem[ gcount ];

    IconGroupItem* group = d->firstGroup;
    int i = 0;

    for (; group; group = group->m_next)
    {
        groups[i++].group = group;
    }

    qsort( groups, gcount, sizeof( IconViewPriv::SortableItem ), cmpItems );

    IconGroupItem* prev = 0;
    group = 0;

    for (i = 0; i < (int) gcount; ++i)
    {
        group = groups[i].group;

        if (group)
        {
            group->m_prev = prev;

            if (group->m_prev)
            {
                group->m_prev->m_next = group;
            }

            group->m_next = 0;
        }

        if (i == 0)
        {
            d->firstGroup = group;
        }

        if (i == (int) gcount - 1)
        {
            d->lastGroup = group;
        }

        prev = group;
    }

    delete [] groups;
}

void IconView::slotRearrange()
{
    if (d->highlightedItem)
    {
        d->highlightedItem->setHighlighted(false);
        d->highlightedItem = 0;
    }

    if (d->ratingItem)
    {
        d->ratingItem->setEditRating(false);
        d->ratingItem = 0;
        unsetCursor();
        d->ratingWidget->hide();
    }

    // hide tooltip
    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    sort();
    arrangeItems();

    // ensure there is a current item
    if (!d->currItem)
    {
        // set the currItem to first item
        if (d->firstGroup)
        {
            d->currItem = d->firstGroup->firstItem();
        }
    }

    d->anchorItem = d->currItem;

    // ensure there is a selection
    if (d->selectedItems.isEmpty() && d->currItem)
    {
        d->currItem->setSelected(true, true);
    }
    else if (d->needEmitSelectionChanged)
    {
        emit signalSelectionChanged();
    }

    d->needEmitSelectionChanged = false;

    // set first visible item if they where stored before update was triggered
    if (d->storedVisibleItem)
    {
        ensureItemVisible(d->storedVisibleItem);
        // reset to 0
        d->storedVisibleItem = 0;
    }
    else
    {
        ensureItemVisible(d->currItem);
    }

    viewport()->update();
    emit signalItemsRearranged();
}

bool IconView::arrangeItems()
{
    int  y     = 0;
    int  itemW = itemRect().width();
    int  itemH = itemRect().height();
    int  maxW  = 0;

    int numItemsPerRow = visibleWidth()/(itemW + d->spacing);

    bool changed = false;

    IconGroupItem* group = d->firstGroup;
    IconItem*      item  = 0;

    while (group)
    {
        changed = group->move(y) || changed;
        y      += group->rect().height() + d->spacing;

        item = group->firstItem();

        int col = 0;
        int x   = d->spacing;

        while (item)
        {
            changed = item->move(x, y) || changed;
            x       += itemW + d->spacing;
            ++col;

            if (col >= numItemsPerRow)
            {
                x  = d->spacing;
                y += itemH + d->spacing;
                col = 0;
            }

            maxW = qMax(maxW, x + itemW);
            item = item->m_next;
        }

        if (col != 0)
        {
            y += itemH + d->spacing;
        }

        y     += d->spacing;
        group = group->m_next;
    }

    viewport()->setUpdatesEnabled(false);
    resizeContents( maxW, y );
    viewport()->setUpdatesEnabled(true);

    rebuildContainers();

    return changed;
}

QRect IconView::itemRect() const
{
    return QRect(0, 0, 100, 100);
}

QRect IconView::bannerRect() const
{
    return QRect(0, 0, visibleWidth(), 0);
}

void IconView::viewportPaintEvent(QPaintEvent* pe)
{
    QRect contentsPaintRect(viewportToContents(pe->rect().topLeft()), viewportToContents(pe->rect().bottomRight()));
    QRegion unpaintedRegion(pe->region());

    QPainter painter(viewport());

    // paint any group banners which intersect this paintevent rect
    for (IconGroupItem* group = d->firstGroup; group; group = group->nextGroup())
    {
        if (contentsPaintRect.intersects(group->rect()))
        {
            QRect viewportRect = contentsRectToViewport(group->rect());
            //painter.save();
            painter.translate(viewportRect.x(), viewportRect.y());
            group->paintBanner(&painter);
            painter.translate( - viewportRect.x(), - viewportRect.y());
            //painter.restore();
            unpaintedRegion -= QRegion(viewportRect);
        }
    }

    // now paint any items which intersect
    QList<IconItem*> itemsToRepaint;

    for (IconViewPriv::ItemContainer* c = d->firstContainer; c;
         c = c->next)
    {
        if (contentsPaintRect.intersects(c->rect))
        {
            foreach (IconItem* item, c->items)
            {
                if (contentsPaintRect.intersects(item->rect()))
                {
                    itemsToRepaint << item;
                }
            }
        }
    }

    prepareRepaint(itemsToRepaint);

    foreach (IconItem* item, itemsToRepaint)
    {
        QRect viewportRect = contentsRectToViewport(item->rect());
        //painter.save();
        painter.translate(viewportRect.x(), viewportRect.y());
        item->paintItem(&painter);
        painter.translate( - viewportRect.x(), - viewportRect.y());
        //painter.restore();
        unpaintedRegion -= QRegion(viewportRect);
    }

    painter.setClipRegion(unpaintedRegion);
    painter.fillRect(pe->rect(), palette().color(QPalette::Base));
}

void IconView::prepareRepaint(const QList<IconItem*> &)
{
}

QRect IconView::contentsRectToViewport(const QRect& r) const
{
    QRect vr = QRect(contentsToViewport(QPoint(r.x(), r.y())), r.size());
    return vr;
}

void IconView::resizeEvent(QResizeEvent* e)
{
    Q3ScrollView::resizeEvent(e);
    triggerRearrangement();
}

void IconView::rebuildContainers()
{
    deleteContainers();

    IconItem* item = 0;
    appendContainer();

    if (d->firstGroup)
    {
        item = d->firstGroup->firstItem();
    }

    IconViewPriv::ItemContainer* c = d->lastContainer;

    while (item)
    {
        if (c->rect.contains(item->rect()))
        {
            c->items << item;
            item = item->nextItem();
        }
        else if (c->rect.intersects(item->rect()))
        {
            c->items << item;
            c = c->next;

            if (!c)
            {
                appendContainer();
                c = d->lastContainer;
            }

            c->items << item;
            item = item->nextItem();
            c = c->prev;
        }
        else
        {
            if (item->y() < c->rect.y() && c->prev)
            {
                c = c->prev;
                continue;
            }

            c = c->next;

            if (!c)
            {
                appendContainer();
                c = d->lastContainer;
            }
        }
    }
}

void IconView::appendContainer()
{
    QSize s( INT_MAX - 1, RECT_EXTENSION );

    if (!d->firstContainer)
    {
        d->firstContainer = new IconViewPriv::ItemContainer(0, 0, QRect(QPoint(0, 0), s));
        d->lastContainer = d->firstContainer;
    }
    else
    {
        d->lastContainer = new IconViewPriv::ItemContainer(
            d->lastContainer, 0, QRect(d->lastContainer->rect.bottomLeft(), s));
    }
}

void IconView::deleteContainers()
{
    IconViewPriv::ItemContainer* c = d->firstContainer;
    IconViewPriv::ItemContainer* tmp;

    while (c)
    {
        tmp = c->next;
        delete c;
        c = tmp;
    }

    d->firstContainer = d->lastContainer = 0;
}

void IconView::leaveEvent(QEvent* e)
{

    // if the mouse leaves the widget we are not dragging
    // anymore
    d->dragging = false;

    Q3ScrollView::leaveEvent(e);
}

void IconView::focusOutEvent(QFocusEvent* e)
{
    if (d->highlightedItem)
    {
        d->highlightedItem->setHighlighted(false);
        d->highlightedItem = 0;
    }

    if (d->ratingItem)
    {
        d->ratingItem->setEditRating(false);
        d->ratingItem = 0;
        unsetCursor();
        d->ratingWidget->hide();
    }

    // hide tooltip
    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    Q3ScrollView::focusOutEvent(e);
}

bool IconView::acceptToolTip(IconItem*, const QPoint&)
{
    return true;
}

void IconView::contentsMousePressEvent(QMouseEvent* e)
{
    d->pressedMoved = false;

    // hide tooltip
    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    // Clear any existing rubber -------------------------------
    d->rubber->setActive(false);

    if (e->button() == Qt::RightButton)
    {
        IconItem* item = findItem(e->pos());

        if (item)
        {
            IconItem* prevCurrItem = d->currItem;
            d->currItem            = item;
            d->anchorItem          = item;

            if (prevCurrItem)
            {
                prevCurrItem->repaint();
            }

            if (!item->isSelected())
            {
                item->setSelected(true, true);
            }

            item->repaint();

            emit signalRightButtonClicked(item, e->globalPos());
        }
        else
        {
            clearSelection();
            emit signalRightButtonClicked(e->globalPos());
        }

        return;
    }

    IconItem* item = findItem(e->pos());

    if (item)
    {
        if (e->modifiers() == Qt::ControlModifier)
        {
            item->setSelected(!item->isSelected(), false);
        }
        else if (e->modifiers() == Qt::ShiftModifier
                 || e->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier) )
        {
            blockSignals(true);

            if (d->currItem)
            {
                if ( !(e->modifiers() & Qt::ControlModifier) )
                {
                    clearSelection();
                }

                // select all items from/upto the current item
                bool bwdSelect = false;

                // find if the current item is before the clicked item
                for (IconItem* it = item->prevItem(); it; it = it->prevItem())
                {
                    if (it == d->currItem)
                    {
                        bwdSelect = true;
                        break;
                    }
                }

                if (bwdSelect)
                {
                    for (IconItem* it = item; it; it = it->prevItem())
                    {
                        it->setSelected(true, false);

                        if (it == d->currItem)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    for (IconItem* it = item; it; it = it->nextItem())
                    {
                        it->setSelected(true, false);

                        if (it == d->currItem)
                        {
                            break;
                        }
                    }
                }
            }
            else
            {
                item->setSelected(true, false);
            }

            blockSignals(false);

            emit signalSelectionChanged();
        }
        else if (e->modifiers() == Qt::NoModifier)
        {
            if (item->clickToToggleSelectRect().contains(e->pos()))
            {
                item->setSelected(!item->isSelected(), false);
            }
            else if (!item->isSelected())
            {
                item->setSelected(true, true);
            }
        }

        IconItem* prevCurrItem = d->currItem;
        d->currItem            = item;
        d->anchorItem          = item;

        if (prevCurrItem)
        {
            prevCurrItem->repaint();
        }

        d->currItem->repaint();

        d->dragging     = true;
        d->dragStartPos = e->pos();

        return;
    }

    // Press outside any item.
    if (!(e->modifiers() & Qt::ControlModifier))
    {
        // unselect all if the ctrl button is not pressed
        clearSelection();
    }
    else
    {
        // ctrl is pressed. make sure our current selection is not lost
        d->prevSelectedItems = d->selectedItems;
    }

    d->rubber->setFirstPointOnViewport(e->pos());
}

void IconView::contentsMouseMoveEvent(QMouseEvent* e)
{
    if (e->buttons() == Qt::NoButton)
    {
        IconItem* item = findItem(e->pos());

        if (d->showTips)
        {
            if (!isActiveWindow())
            {
                d->toolTipItem = 0;
                d->toolTipTimer->stop();
                slotToolTip();
                return;
            }

            if (item != d->toolTipItem)
            {
                d->toolTipItem = 0;
                d->toolTipTimer->stop();
                slotToolTip();

                if (acceptToolTip(item, e->pos()))
                {
                    d->toolTipItem = item;
                    d->toolTipTimer->setSingleShot(true);
                    d->toolTipTimer->start(500);
                }
            }

            if (item == d->toolTipItem && !acceptToolTip(item, e->pos()))
            {
                d->toolTipItem = 0;
                d->toolTipTimer->stop();
                slotToolTip();
            }
        }

        if (item && KGlobalSettings::changeCursorOverIcon() && item->clickToOpenRect().contains(e->pos()))
        {
            setCursor(Qt::PointingHandCursor);
            d->ratingWidget->hide();

            if (d->ratingItem)
            {
                d->ratingItem->setEditRating(false);
            }

            d->ratingItem = 0;
        }
        else if (item && item->clickToRateRect().contains(e->pos()))
        {
            setCursor(Qt::CrossCursor);
            d->ratingItem = item;

            if (d->ratingItem)
            {
                d->ratingItem->setEditRating(true);
            }

            QRect rect = item->clickToRateRect();
            rect.moveTopLeft(contentsToViewport(rect.topLeft()));
            d->ratingWidget->setFixedSize(rect.width()+1, rect.height()+1);
            d->ratingWidget->move(rect.topLeft().x(), rect.topLeft().y());
            d->ratingWidget->setRating(item->rating());
            d->ratingWidget->show();
        }
        else
        {
            unsetCursor();
            d->ratingWidget->hide();

            if (d->ratingItem)
            {
                d->ratingItem->setEditRating(false);
            }

            d->ratingItem = 0;
        }

        // Draw item highlightment when mouse is over.

        if (item != d->highlightedItem)
        {
            if (d->highlightedItem)
            {
                d->highlightedItem->setHighlighted(false);
            }

            d->highlightedItem = item;

            if (d->highlightedItem)
            {
                d->highlightedItem->setHighlighted(true);
            }
        }

        return;
    }

    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    if (d->dragging && (e->buttons() & Qt::LeftButton))
    {
        if ( (d->dragStartPos - e->pos()).manhattanLength()
             > QApplication::startDragDistance() )
        {
            startDrag();
        }

        return;
    }

    if (!d->rubber->isActive())
    {
        return;
    }

    QRect oldArea = d->rubber->rubberBandAreaOnContents();

    d->rubber->setSecondPointOnViewport(e->pos());

    QRect newArea     = d->rubber->rubberBandAreaOnContents();
    QRect rubberUnion = oldArea.unite(newArea);
    bool changed      = false;

    QRegion paintRegion;
    viewport()->setUpdatesEnabled(false);
    blockSignals(true);

    IconViewPriv::ItemContainer* c = d->firstContainer;

    for (; c; c = c->next)
    {
        if ( rubberUnion.intersects(c->rect) )
        {
            foreach (IconItem* item, c->items)
            {
                if (newArea.intersects(item->rect()))
                {
                    if (!item->isSelected())
                    {
                        item->setSelected(true, false);
                        changed = true;
                        paintRegion += QRect(item->rect());
                    }
                }
                else
                {
                    if (item->isSelected() &&  !d->prevSelectedItems.contains(item))
                    {
                        item->setSelected(false, false);
                        changed = true;
                        paintRegion += QRect(item->rect());
                    }
                }
            }
        }
    }

    blockSignals(false);
    viewport()->setUpdatesEnabled(true);

    if (changed)
    {
        paintRegion.translate(-contentsX(), -contentsY());
        viewport()->repaint(paintRegion);
    }

    ensureVisible(e->pos().x(), e->pos().y());

    d->pressedMoved = true;

    if (changed)
    {
        emit signalSelectionChanged();
    }
}

void IconView::contentsMouseReleaseEvent(QMouseEvent* e)
{
    d->dragging = false;
    d->prevSelectedItems.clear();

    if (d->rubber->isActive())
    {
        d->rubber->setActive(false);
    }

    if (e->button() == Qt::LeftButton
        && e->buttons() == Qt::NoButton
        && e->modifiers() == Qt::NoModifier)
    {
        if (d->pressedMoved)
        {
            emit signalSelectionChanged();
            d->pressedMoved = false;
            return;
        }

        // click on item
        IconItem* item = findItem(e->pos());

        if (item && !item->clickToToggleSelectRect().contains(e->pos()))
        {
            IconItem* prevCurrItem = d->currItem;
            item->setSelected(true, true);
            d->currItem   = item;
            d->anchorItem = item;

            if (prevCurrItem)
            {
                prevCurrItem->repaint();
            }

            if (KGlobalSettings::singleClick())
            {
                if (item->clickToOpenRect().contains(e->pos()))
                {
                    itemClickedToOpen(item);
                }
            }
        }
    }
}

void IconView::contentsWheelEvent(QWheelEvent* e)
{
    e->accept();

    if (d->highlightedItem)
    {
        d->highlightedItem->setHighlighted(false);
        d->highlightedItem = 0;
    }

    if (d->ratingItem)
    {
        d->ratingItem->setEditRating(false);
        d->ratingItem = 0;
        unsetCursor();
        d->ratingWidget->hide();
    }

    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();
    viewport()->update();

    if (e->modifiers() & Qt::ControlModifier)
    {
        if (e->delta() < 0)
        {
            emit signalZoomOut();
        }
        else if (e->delta() > 0)
        {
            emit signalZoomIn();
        }

        // We don't want to scroll contents.
        return;
    }

    Q3ScrollView::contentsWheelEvent(e);
}

void IconView::contentsMouseDoubleClickEvent(QMouseEvent* e)
{
    if (KGlobalSettings::singleClick())
    {
        return;
    }

    IconItem* item = findItem(e->pos());

    if (item)
    {
        itemClickedToOpen(item);
    }
}

void IconView::keyPressEvent(QKeyEvent* e)
{
    bool handled = false;

    if (!firstItem())
    {
        return;
    }

    switch ( e->key() )
    {
        case Qt::Key_Home:
        {
            if (e->modifiers() & Qt::ShiftModifier)
            {
                IconItem* const tmp = d->currItem;
                d->currItem = firstItem();

                if (tmp)
                {
                    tmp->repaint();
                }

                // select items: anchor until before firstItem
                // block signals while selecting all except the firstItem
                blockSignals(true);

                for (IconItem* i = d->anchorItem; i && ( i != firstItem() ); i = i->prevItem())
                {
                    i->setSelected(true, false);
                }

                blockSignals(false);

                // select the firstItem with signals enabled to ensure updates
                firstItem()->setSelected(true, false);
            }
            else
            {
                IconItem* const tmp = d->currItem;
                d->currItem = firstItem();
                d->anchorItem = d->currItem;

                if (tmp)
                {
                    tmp->repaint();
                }

                // select only the first item
                firstItem()->setSelected(true, true);
            }

            ensureItemVisible(firstItem());
            handled = true;
            break;
        }

        case Qt::Key_End:
        {
            if (e->modifiers() & Qt::ShiftModifier)
            {
                IconItem* const tmp = d->currItem;
                d->currItem = lastItem();

                if (tmp)
                {
                    tmp->repaint();
                }

                // select items: current until lastItem
                // block signals while selecting all except the lastItem
                blockSignals(true);

                for (IconItem* i = d->anchorItem; i && ( i != lastItem() ); i = i->nextItem())
                {
                    i->setSelected(true, false);
                }

                blockSignals(false);

                // select the lastItem with signals enabled to ensure updates
                lastItem()->setSelected(true, false);
            }
            else
            {
                IconItem* const tmp = d->currItem;
                d->currItem = lastItem();
                d->anchorItem = d->currItem;

                if (tmp)
                {
                    tmp->repaint();
                }

                lastItem()->setSelected(true, true);
            }

            ensureItemVisible(lastItem());
            handled = true;
            break;
        }

        case Qt::Key_Enter:
        case Qt::Key_Return:
        {
            if (d->currItem)
            {
                emit signalReturnPressed(d->currItem);
                handled = true;
            }

            break;
        }

        case Qt::Key_Right:
        {
            IconItem* item = 0;

            if (d->currItem)
            {
                if (d->currItem->nextItem())
                {
                    if (e->modifiers() & Qt::ControlModifier)
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = d->currItem->nextItem();
                        d->anchorItem = d->currItem;
                        tmp->repaint();
                        d->currItem->repaint();

                        item = d->currItem;
                    }
                    else if (e->modifiers() & Qt::ShiftModifier)
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = d->currItem->nextItem();
                        tmp->repaint();

                        // if the anchor is behind us, move forward preserving
                        // the previously selected item. otherwise unselect the
                        // previously selected item
                        if (!anchorIsBehind())
                        {
                            tmp->setSelected(false, false);
                        }

                        d->currItem->setSelected(true, false);

                        item = d->currItem;
                    }
                    else
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = d->currItem->nextItem();
                        d->anchorItem = d->currItem;
                        d->currItem->setSelected(true, true);
                        tmp->repaint();

                        item = d->currItem;
                    }
                }
            }
            else
            {
                d->currItem   = firstItem();
                d->anchorItem = d->currItem;
                d->currItem->setSelected(true, true);
                item = d->currItem;
            }

            ensureItemVisible(item);
            handled = true;
            break;
        }

        case Qt::Key_Left:
        {
            IconItem* item = 0;

            if (d->currItem)
            {
                if (d->currItem->prevItem())
                {
                    if (e->modifiers() & Qt::ControlModifier)
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = d->currItem->prevItem();
                        d->anchorItem = d->currItem;
                        tmp->repaint();
                        d->currItem->repaint();

                        item = d->currItem;
                    }
                    else if (e->modifiers() & Qt::ShiftModifier)
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = d->currItem->prevItem();
                        tmp->repaint();

                        // if the anchor is ahead of us, move forward preserving
                        // the previously selected item. otherwise unselect the
                        // previously selected item
                        if (anchorIsBehind())
                        {
                            tmp->setSelected(false, false);
                        }

                        d->currItem->setSelected(true, false);

                        item = d->currItem;
                    }
                    else
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = d->currItem->prevItem();
                        d->anchorItem = d->currItem;
                        d->currItem->setSelected(true, true);
                        tmp->repaint();

                        item = d->currItem;
                    }
                }
            }
            else
            {
                d->currItem   = firstItem();
                d->anchorItem = d->currItem;
                d->currItem->setSelected(true, true);
                item = d->currItem;
            }

            ensureItemVisible(item);
            handled = true;
            break;
        }

        case Qt::Key_Up:
        {
            IconItem* item = 0;

            if (d->currItem)
            {
                int x = d->currItem->x() + itemRect().width()/2;
                int y = d->currItem->y() - d->spacing*2;

                IconItem* it = 0;

                while (!it && y > 0)
                {
                    it  = findItem(QPoint(x,y));
                    y  -= d->spacing * 2;
                }

                if (it)
                {
                    if (e->modifiers() & Qt::ControlModifier)
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = it;
                        d->anchorItem = it;
                        tmp->repaint();
                        d->currItem->repaint();

                        item = d->currItem;
                    }
                    else if (e->modifiers() & Qt::ShiftModifier)
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = it;
                        tmp->repaint();

                        clearSelection();

                        if (anchorIsBehind())
                        {
                            for (IconItem* i = d->currItem; i; i = i->prevItem())
                            {
                                i->setSelected(true, false);

                                if (i == d->anchorItem)
                                {
                                    break;
                                }
                            }
                        }
                        else
                        {
                            for (IconItem* i = d->currItem; i; i = i->nextItem())
                            {
                                i->setSelected(true, false);

                                if (i == d->anchorItem)
                                {
                                    break;
                                }
                            }
                        }

                        item = d->currItem;
                    }
                    else
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = it;
                        d->anchorItem = it;
                        d->currItem->setSelected(true, true);
                        tmp->repaint();

                        item = d->currItem;
                    }
                }
            }
            else
            {
                d->currItem   = firstItem();
                d->anchorItem = d->currItem;
                d->currItem->setSelected(true, true);
                item = d->currItem;
            }

            ensureItemVisible(item);
            handled = true;
            break;
        }

        case Qt::Key_Down:
        {
            IconItem* item = 0;

            if (d->currItem)
            {
                int x = d->currItem->x() + itemRect().width()/2;
                int y = d->currItem->y() + itemRect().height() + d->spacing*2;

                IconItem* it = 0;

                while (!it && y < contentsHeight())
                {
                    it  = findItem(QPoint(x,y));
                    y  += d->spacing * 2;
                }

                if (it)
                {
                    if (e->modifiers() & Qt::ControlModifier)
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = it;
                        d->anchorItem = it;
                        tmp->repaint();
                        d->currItem->repaint();

                        item = d->currItem;
                    }
                    else if (e->modifiers() & Qt::ShiftModifier)
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = it;
                        tmp->repaint();

                        clearSelection();

                        if (anchorIsBehind())
                        {
                            for (IconItem* i = d->currItem; i; i = i->prevItem())
                            {
                                i->setSelected(true, false);

                                if (i == d->anchorItem)
                                {
                                    break;
                                }
                            }
                        }
                        else
                        {
                            for (IconItem* i = d->currItem; i; i = i->nextItem())
                            {
                                i->setSelected(true, false);

                                if (i == d->anchorItem)
                                {
                                    break;
                                }
                            }
                        }

                        item = d->currItem;
                    }
                    else
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = it;
                        d->anchorItem = it;
                        d->currItem->setSelected(true, true);
                        tmp->repaint();

                        item = d->currItem;
                    }
                }
            }
            else
            {
                d->currItem   = firstItem();
                d->anchorItem = d->currItem;
                d->currItem->setSelected(true, true);
                item = d->currItem;
            }

            ensureItemVisible(item);
            handled = true;
            break;
        }

        case Qt::Key_PageDown:
        {
            IconItem* item = 0;

            if (d->currItem)
            {
                QRect r( 0, d->currItem->y() + visibleHeight(),
                         contentsWidth(), visibleHeight() );
                IconItem* ni = findFirstVisibleItem(r, false);

                if (!ni)
                {
                    r = QRect( 0, d->currItem->y() + itemRect().height(),
                               contentsWidth(), contentsHeight() );
                    ni = findLastVisibleItem(r, false);
                }

                if (ni)
                {
                    IconItem* tmp = d->currItem;
                    d->currItem   = ni;
                    d->anchorItem = ni;
                    item          = ni;
                    tmp->repaint();
                    d->currItem->setSelected(true, true);
                }
            }
            else
            {
                d->currItem   = firstItem();
                d->anchorItem = d->currItem;
                d->currItem->setSelected(true, true);
                item = d->currItem;
            }

            ensureItemVisible(item);
            handled = true;
            break;
        }

        case Qt::Key_PageUp:
        {
            IconItem* item = 0;

            if (d->currItem)
            {
                QRect r(0, d->currItem->y() - visibleHeight(),
                        contentsWidth(), visibleHeight() );

                IconItem* ni = findFirstVisibleItem(r, false);

                if (!ni)
                {
                    r = QRect( 0, 0, contentsWidth(), d->currItem->y() );
                    ni = findFirstVisibleItem(r, false);
                }

                if (ni)
                {
                    IconItem* tmp = d->currItem;
                    d->currItem   = ni;
                    d->anchorItem = ni;
                    item          = ni;
                    tmp->repaint();
                    d->currItem->setSelected(true, true);
                }
            }
            else
            {
                d->currItem   = firstItem();
                d->anchorItem = d->currItem;
                d->currItem->setSelected(true, true);
                item = d->currItem;
            }

            ensureItemVisible(item);
            handled = true;
            break;
        }

        // Qt::Key_Space is used as a global shortcut in DigikamApp.
        // Ctrl+Space comes through, Shift+Space is filtered out.
        case Qt::Key_Space:
        {
            if (d->currItem)
            {
                if ( (e->modifiers() & Qt::ControlModifier) || (e->modifiers() & Qt::ShiftModifier) )
                {
                    d->currItem->setSelected(!d->currItem->isSelected(), false);
                }
                else
                {
                    if (!d->currItem->isSelected())
                    {
                        d->currItem->setSelected(true, true);
                    }
                }

                handled = true;
            }

            break;
        }

        case Qt::Key_Menu:
        {
            if (d->currItem)
            {
                if (!d->currItem->isSelected())
                {
                    d->currItem->setSelected(true, false);
                }

                ensureItemVisible(d->currItem);

                QRect r(itemRect());
                int w = r.width();
                int h = r.height();
                QPoint p(d->currItem->x() + w / 2, d->currItem->y() + h / 2);

                emit signalRightButtonClicked(d->currItem, mapToGlobal(contentsToViewport(p)));
            }

            break;
        }

        default:
            break;
    }

    if (!handled)
    {
        e->ignore();
    }
    else
    {
        if (d->highlightedItem)
        {
            d->highlightedItem->setHighlighted(false);
            d->highlightedItem = 0;
        }

        if (d->ratingItem)
        {
            d->ratingItem->setEditRating(false);
            d->ratingItem = 0;
            unsetCursor();
            d->ratingWidget->hide();
        }

        emit signalSelectionChanged();
        viewport()->update();
        d->toolTipItem = 0;
        d->toolTipTimer->stop();
        slotToolTip();
    }
}

bool IconView::anchorIsBehind() const
{
    if (!d->anchorItem || !d->currItem)
    {
        return false;
    }

    for (IconItem* it = d->anchorItem; it; it = it->nextItem())
    {
        if (it == d->currItem)
        {
            return true;
        }
    }

    return false;
}


void IconView::startDrag()
{
}

void IconView::ensureItemVisible(IconItem* item)
{
    if ( !item )
    {
        return;
    }

    if ( item->y() == firstItem()->y() )
    {
        QRect r(itemRect());
        int w = r.width();
        ensureVisible( item->x() + w / 2, 0, w/2+1, 0 );
    }
    else
    {
        QRect r(itemRect());
        int w = r.width();
        int h = r.height();
        ensureVisible( item->x() + w / 2, item->y() + h / 2,
                       w / 2 + 1, h / 2 + 1 );
    }
}

IconItem* IconView::findFirstVisibleItem(bool useThumbnailRect) const
{
    QRect r(contentsX(), contentsY(), visibleWidth(), visibleHeight());
    return findFirstVisibleItem(r, useThumbnailRect);
}

IconItem* IconView::findLastVisibleItem(bool useThumbnailRect) const
{
    QRect r(contentsX(), contentsY(), visibleWidth(), visibleHeight());
    return findLastVisibleItem(r, useThumbnailRect);
}

IconItem* IconView::findFirstVisibleItem(const QRect& r, bool useThumbnailRect) const
{
    IconViewPriv::ItemContainer* c = d->firstContainer;
    bool alreadyIntersected = false;
    IconItem* i = 0;

    for ( ; c; c = c->next )
    {
        if ( c->rect.intersects( r ) )
        {
            alreadyIntersected = true;
            foreach (IconItem* item, c->items)
            {
                // if useThumbnailRect, we only check for the clickToOpenRect, which is the thumbnail,
                // otherwise, we take the whole item rect
                if ( r.intersects( useThumbnailRect ? item->clickToOpenRect() : item->rect() ) )
                {
                    if ( !i )
                    {
                        i = item;
                    }
                    else
                    {
                        QRect r2 = item->rect();
                        QRect r3 = i->rect();

                        if ( r2.y() < r3.y() )
                        {
                            i = item;
                        }
                        else if ( r2.y() == r3.y() &&
                                  r2.x() < r3.x() )
                        {
                            i = item;
                        }
                    }
                }
            }
        }
        else
        {
            if ( alreadyIntersected )
            {
                break;
            }
        }
    }

    return i;
}

IconItem* IconView::findLastVisibleItem(const QRect& r, bool useThumbnailRect) const
{
    IconViewPriv::ItemContainer* c = d->firstContainer;
    IconItem* i = 0;
    bool alreadyIntersected = false;

    for ( ; c; c = c->next )
    {
        if ( c->rect.intersects( r ) )
        {
            alreadyIntersected = true;
            foreach (IconItem* item, c->items)
            {
                if ( r.intersects( useThumbnailRect ? item->clickToOpenRect() : item->rect() ) )
                {
                    if ( !i )
                    {
                        i = item;
                    }
                    else
                    {
                        QRect r2 = item->rect();
                        QRect r3 = i->rect();

                        if ( r2.y() > r3.y() )
                        {
                            i = item;
                        }
                        else if ( r2.y() == r3.y() &&
                                  r2.x() > r3.x() )
                        {
                            i = item;
                        }
                    }
                }
            }
        }
        else
        {
            if ( alreadyIntersected )
            {
                break;
            }
        }
    }

    return i;
}

void IconView::drawFrameRaised(QPainter* p)
{
    QRect r               = frameRect();
    int lwidth            = lineWidth();
    const QColorGroup& g = QColorGroup(palette());
    qDrawShadeRect( p, r, g, false, lwidth, midLineWidth() );
}

void IconView::drawFrameSunken(QPainter* p)
{
    QRect r               = frameRect();
    int lwidth            = lineWidth();
    const QColorGroup& g = QColorGroup(palette());
    qDrawShadeRect( p, r, g, true, lwidth, midLineWidth() );
}

void IconView::setEnableToolTips(bool val)
{
    d->showTips = val;

    if (!val)
    {
        d->toolTipItem = 0;
        d->toolTipTimer->stop();
        slotToolTip();
    }
}

void IconView::slotToolTip()
{
    emit signalShowToolTip(d->toolTipItem);
}

void IconView::itemClickedToOpen(IconItem* item)
{
    if (!item)
    {
        return;
    }

    IconItem* prevCurrItem = d->currItem;
    d->currItem            = item;
    d->anchorItem          = item;

    if (prevCurrItem)
    {
        prevCurrItem->repaint();
    }

    item->setSelected(true);
    emit signalDoubleClicked(item);
}

int IconView::cmpItems(const void* n1, const void* n2)
{
    if ( !n1 || !n2 )
    {
        return 0;
    }

    IconViewPriv::SortableItem* i1 = (IconViewPriv::SortableItem*)n1;
    IconViewPriv::SortableItem* i2 = (IconViewPriv::SortableItem*)n2;

    return i1->group->compare( i2->group );
}

QPixmap IconView::thumbnailBorderPixmap(const QSize& pixSize)
{
    const int radius         = 3;
    const QColor borderColor = QColor(0, 0, 0, 128);

    QString cacheKey  = QString::number(pixSize.width()) + '-' + QString::number(pixSize.height());
    QPixmap* cachePix = d->thumbnailBorderCache.object(cacheKey);

    if (!cachePix)
    {
        QPixmap pix = ThumbBarView::generateFuzzyRect(QSize(pixSize.width()  + 2*radius,
                      pixSize.height() + 2*radius),
                      borderColor, radius);
        d->thumbnailBorderCache.insert(cacheKey, new QPixmap(pix));
        return pix;
    }

    return *cachePix;
}

void IconView::clearThumbnailBorderCache()
{
    d->thumbnailBorderCache.clear();
}

QPixmap IconView::selectPixmap() const
{
    return d->selectPix;
}

QPixmap IconView::deselectPixmap() const
{
    return d->deselectPix;
}

void IconView::slotIconViewFontChanged()
{
    setFont(AlbumSettings::instance()->getIconViewFont());
}

}  // namespace Digikam
