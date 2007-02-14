/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2005-04-24
 * Description : 
 *
 * Copyright 2005 by Renchi Raju
 * Copyright 2006-2007 by Gilles Caulier
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

// C Ansi includes.

extern "C"
{
#include <stdlib.h>
}

// Qt includes.

#include <qtimer.h>
#include <qpainter.h>
#include <qvaluelist.h>
#include <qptrdict.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qdrawutil.h>

// KDE includes.

#include <kcursor.h>
#include <kglobalsettings.h>

// Local includes.

#include "ddebug.h"
#include "iconitem.h"
#include "icongroupitem.h"
#include "iconview.h"
#include "iconview.moc"

namespace Digikam
{

class IconViewPriv
{
public:

    IconViewPriv()
    {
        firstGroup          = 0;
        lastGroup           = 0;
        currItem            = 0;
        anchorItem          = 0;
        firstVisibleItem    = 0;
        clearing            = false;
        spacing             = 10;

        rubber              = 0;
        dragging            = false;
        pressedMoved        = false;
        
        firstContainer      = 0;
        lastContainer       = 0;

        showTips            = false;
        toolTipItem         = 0;
        toolTipTimer        = 0;
        updateTimer         = 0;
        updateTimerInterval = 0;
    }
    
    bool               clearing;
    bool               showTips;
    bool               pressedMoved;
    bool               dragging;
    
    int                spacing;

    QPtrDict<IconItem> selectedItems;
    QPtrDict<IconItem> prevSelectedItems;

    QRect*             rubber;
    
    QPoint             dragStartPos;
    
    QTimer*            updateTimer;
    QTimer*            toolTipTimer;

    IconItem*          toolTipItem;
    IconItem*          currItem;
    IconItem*          anchorItem;
    IconItem*          firstVisibleItem; // store position for slotUpdate
    
    IconGroupItem*     firstGroup;
    IconGroupItem*     lastGroup;

    int                updateTimerInterval;
    
    struct ItemContainer 
    {
        ItemContainer(ItemContainer *p, ItemContainer *n, const QRect &r) 
            : prev(p), next(n), rect(r) 
        {
            if (prev)
                prev->next = this;
            if (next)
                next->prev = this;
        }

        ItemContainer        *prev, *next;
        QRect                 rect;
        QValueList<IconItem*> items;
    } *firstContainer, *lastContainer;

    struct SortableItem 
    {
        IconGroupItem *group;
    };
};

IconView::IconView(QWidget* parent, const char* name)
        : QScrollView(parent, name, Qt::WStaticContents|Qt::WNoAutoErase)
{
    viewport()->setBackgroundMode(Qt::NoBackground);
    viewport()->setFocusProxy(this);
    viewport()->setFocusPolicy(QWidget::WheelFocus);
    viewport()->setMouseTracking(true);

    d = new IconViewPriv;
    d->updateTimer  = new QTimer(this);
    d->toolTipTimer = new QTimer(this);
    
    connect(d->updateTimer, SIGNAL(timeout()),
            this, SLOT(slotUpdate()));
            
    connect(d->toolTipTimer, SIGNAL(timeout()),
            this, SLOT(slotToolTip()));

    setEnableToolTips(true);
}

IconView::~IconView()
{
    clear(false);

    delete d->updateTimer;
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
        return 0;

    return d->firstGroup->firstItem();
}

IconItem* IconView::lastItem() const
{
    if (!d->lastGroup)
        return 0;

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

IconItem* IconView::findItem(const QPoint& pos)
{
    IconViewPriv::ItemContainer *c = d->firstContainer;
    for (; c; c = c->next)
    {
        if ( c->rect.contains(pos) ) 
        {
            for (QValueList<IconItem*>::iterator it = c->items.begin();
                 it != c->items.end(); ++it)
            {
                IconItem* item = *it;
                if (item->rect().contains(pos))
                    return item;
            }
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
        for (IconItem *it = group->firstItem(); it; it = it->nextItem())
            if (it->isSelected())
                c++;
    }

    return c;
}

int IconView::groupCount() const
{
    int c = 0;
    for (IconGroupItem* group = d->firstGroup; group; group = group->nextGroup())
    {
        c++;
    }

    return c;
}

void IconView::clear(bool update)
{
    d->clearing = true;

    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();
    
    deleteContainers();

    d->selectedItems.clear();
    
    IconGroupItem *group = d->firstGroup;
    while (group)
    {
        IconGroupItem *tmp = group->m_next;
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
        updateContents();

    d->clearing = false;

    emit signalSelectionChanged();
}

void IconView::clearSelection()
{
    bool wasBlocked = signalsBlocked();;

    if (!wasBlocked)
        blockSignals(true);

    QPtrDict<IconItem> selItems = d->selectedItems;
    QPtrDictIterator<IconItem> it( selItems );
    for ( ; it.current(); ++it )
        it.current()->setSelected(false, false);

    d->selectedItems.clear();

    if (!wasBlocked)
        blockSignals(false);

    emit signalSelectionChanged();
}

void IconView::selectAll()
{
    bool wasBlocked = signalsBlocked();

    if (!wasBlocked)
        blockSignals(true);
    
    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (!item->isSelected())
        {
            item->setSelected(true, false);
        }
    }
    
    if (!wasBlocked)
        blockSignals(false);

    emit signalSelectionChanged();
}

void IconView::invertSelection()
{
    bool wasBlocked = signalsBlocked();

    if (!wasBlocked)
        blockSignals(true);
    
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
        blockSignals(false);

    emit signalSelectionChanged();
}

void IconView::selectItem(IconItem* item, bool select)
{
    if (!item)
        return;

    if (select)
    {
        d->selectedItems.replace(item, item);
    }
    else
    {
        d->selectedItems.remove(item);
    }

    emit signalSelectionChanged();
}

void IconView::insertGroup(IconGroupItem* group)
{
    if (!group)
        return;

    if (!d->firstGroup)
    {
        d->firstGroup = group;
        d->lastGroup  = group;
        group->m_prev   = 0;
        group->m_next   = 0;
    }
    else
    {
        d->lastGroup->m_next = group;
        group->m_prev        = d->lastGroup;
        group->m_next        = 0;
        d->lastGroup         = group;
    }

    d->firstVisibleItem = findFirstVisibleItem();
    startUpdateTimer();
}

void IconView::takeGroup(IconGroupItem* group)
{
    if (!group)
        return;

    // this is only to find an alternative visible item if all visible items
    // are removed
    IconGroupItem *alternativeVisibleGroup = 0;
    d->firstVisibleItem = 0;

    if (group == d->firstGroup)
    {
        d->firstGroup = d->firstGroup->m_next;
        if (d->firstGroup)
            d->firstGroup->m_prev = 0;
        else
            d->firstGroup = d->lastGroup = 0;
        alternativeVisibleGroup = d->firstGroup;
    }
    else if (group == d->lastGroup) 
    {
        d->lastGroup = d->lastGroup->m_prev;
        if ( d->lastGroup )
            d->lastGroup->m_next = 0;
        else
            d->firstGroup = d->lastGroup = 0;
        alternativeVisibleGroup = d->lastGroup->m_prev;
    }
    else 
    {
        IconGroupItem *i = group;
        if (i) 
        {
            if (i->m_prev )
                i->m_prev->m_next = i->m_next;
            if ( i->m_next )
                i->m_next->m_prev = i->m_prev;

            if (i->m_prev)
                alternativeVisibleGroup = i->m_prev;
            else
                alternativeVisibleGroup = i->m_next;
        }
    }

    if (!d->clearing)
    {
        d->firstVisibleItem = findFirstVisibleItem();
        if (!d->firstVisibleItem && alternativeVisibleGroup)
        {
            // find an alternative visible item
            d->firstVisibleItem = alternativeVisibleGroup->lastItem();
        }
        startUpdateTimer();
    }
}

void IconView::insertItem(IconItem* item)
{
    if (!item)
        return;

    d->firstVisibleItem = findFirstVisibleItem();
    startUpdateTimer();
}

void IconView::takeItem(IconItem* item)
{
    if (!item)
        return;

    // First remove item from any containers holding it
    IconViewPriv::ItemContainer *tmp = d->firstContainer;
    while (tmp) 
    {
        tmp->items.remove(item);
        tmp = tmp->next;
    }

    // Remove from selected item list
    d->selectedItems.remove(item);

    if (d->toolTipItem == item)
    {
        d->toolTipItem = 0;
        d->toolTipTimer->stop();
        slotToolTip();
    }
    
    // if it is current item, change the current item
    if (d->currItem == item)
    {
        d->currItem = item->nextItem();
        if (!d->currItem)
            d->currItem = item->prevItem();
        // defer calling d->currItem->setSelected (and emitting the signals) to slotUpdate
    }

    d->anchorItem = d->currItem;
    
    if (!d->clearing)
    {
        d->firstVisibleItem = findFirstVisibleItem();
        if (d->firstVisibleItem == item)
            d->firstVisibleItem = d->currItem;
        startUpdateTimer();
    }
}

void IconView::triggerUpdate()
{
    d->firstVisibleItem = findFirstVisibleItem();
    startUpdateTimer();
}

void IconView::setDelayedUpdate(bool delayed)
{
    // if it is known that e.g. several items will be added or deleted in the next time,
    // but not from the same event queue thread stack location, it may be desirable to delay
    // the updateTimer a bit
    if (delayed)
        d->updateTimerInterval = 50;
    else
        d->updateTimerInterval = 0;
}

void IconView::startUpdateTimer()
{
    // We want to reduce the number of updates, but not remove all updates
    if (!d->updateTimer->isActive())
        d->updateTimer->start(d->updateTimerInterval, true);
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
    IconViewPriv::SortableItem *groups = new IconViewPriv::SortableItem[ gcount ];

    IconGroupItem *group = d->firstGroup;
    int i = 0;
    
    for ( ; group; group = group->m_next )
        groups[ i++ ].group = group;

    qsort( groups, gcount, sizeof( IconViewPriv::SortableItem ), cmpItems );

    IconGroupItem *prev = 0;
    group = 0;
    
    for ( i = 0; i < (int)gcount; ++i ) 
    {
        group = groups[ i ].group;
        if ( group ) 
        {
            group->m_prev = prev;
            
            if ( group->m_prev )
                group->m_prev->m_next = group;
            
            group->m_next = 0;
        }
        
        if ( i == 0 )
            d->firstGroup = group;
            
        if ( i == (int)gcount - 1 )
            d->lastGroup = group;
        prev = group;
    }
    
    delete [] groups;
}

void IconView::slotUpdate()
{
    sort();
    rearrangeItems();

    // ensure there is a current item
    if (!d->currItem)
    {
        // set the currItem to first item
        if (d->firstGroup)
            d->currItem = d->firstGroup->firstItem();
    }
    d->anchorItem = d->currItem;

    if (d->currItem)
    {
        d->currItem->setSelected(true, true);
    }
    else
    {
        // no selection
        emit signalSelectionChanged();
    }

    // set first visible item if they where stored before update was triggered
    if (d->firstVisibleItem)
    {
        ensureItemVisible(d->firstVisibleItem);
        // reset to 0
        d->firstVisibleItem = 0;
    }
    else
    {
        ensureItemVisible(d->currItem);
    }

    viewport()->update();
}

void IconView::rearrangeItems(bool update)
{
    int  y   = 0;
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
            x += itemW + d->spacing;
            col++;

            if (col >= numItemsPerRow)
            {
                x  = d->spacing;
                y += itemH + d->spacing;
                col = 0;
            }

            maxW = QMAX(maxW, x + itemW);
            
            item = item->m_next;
        }

        if (col != 0)
        {
            y += itemH + d->spacing;
        }

        y += d->spacing;

        group = group->m_next;
    }

    viewport()->setUpdatesEnabled(false);  
    resizeContents( maxW, y );
    viewport()->setUpdatesEnabled(true);

    rebuildContainers();

    if (changed && update)
        viewport()->update();
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
    QRect r(pe->rect());
    QRegion paintRegion(pe->region());

    QPainter painter(viewport());
    painter.setClipRegion(paintRegion);

    // paint any group banners which intersect this paintevent rect
    for (IconGroupItem* group = d->firstGroup; group; group = group->nextGroup())
    {
        QRect br(contentsRectToViewport(group->rect()));
        if (r.intersects(br)) 
        {
            group->paintBanner();
            paintRegion -= QRegion(br);
        }
    }

    // now paint any items which intersect
    for (IconViewPriv::ItemContainer* c = d->firstContainer; c;
         c = c->next) 
    {
        QRect cr(contentsRectToViewport(c->rect));

        if (r.intersects(cr)) 
        {
            
            for (QValueList<IconItem*>::iterator it = c->items.begin();
                 it != c->items.end(); ++it)
            {
                IconItem* item = *it;
                QRect ir(contentsRectToViewport(item->rect()));
                if (r.intersects(ir)) 
                {
                    item->paintItem();
                    paintRegion -= QRegion(ir);
                }
            }
        }
    }

    painter.setClipRegion(paintRegion);
    painter.fillRect(r, colorGroup().base());
    painter.end();
}

QRect IconView::contentsRectToViewport(const QRect& r) const
{
    QRect vr = QRect(contentsToViewport(QPoint(r.x(), r.y())), r.size());
    return vr;
}

void IconView::resizeEvent(QResizeEvent* e)
{
    QScrollView::resizeEvent(e);
    rearrangeItems();

    if (d->currItem)
        ensureItemVisible(d->currItem);
}

void IconView::rebuildContainers()
{
    deleteContainers();

    IconItem *item = 0;
    appendContainer();

    if (d->firstGroup)
        item = d->firstGroup->firstItem();
    
    IconViewPriv::ItemContainer* c = d->lastContainer;
    while (item) 
    {
        if (c->rect.contains(item->rect())) 
        {
            c->items.append(item);
            item = item->nextItem();
        }
        else if (c->rect.intersects(item->rect())) 
        {
            c->items.append( item );
            c = c->next;
            
            if (!c) 
            {
                appendContainer();
                c = d->lastContainer;
            }
            
            c->items.append(item);
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
        d->firstContainer =
            new IconViewPriv::ItemContainer(0, 0, QRect(QPoint(0, 0), s));
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
    IconViewPriv::ItemContainer *c = d->firstContainer;
    IconViewPriv::ItemContainer *tmp;

    while (c) 
    {
        tmp = c->next;
        delete c;
        c = tmp;
    }

    d->firstContainer = d->lastContainer = 0;
}

void IconView::leaveEvent(QEvent *e)
{
    // hide tooltip
    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    // if the mouse leaves the widget we are not dragging
    // anymore
    d->dragging = false;
    
    QScrollView::leaveEvent(e);
}

void IconView::focusOutEvent(QFocusEvent* e)
{
    // hide tooltip
    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    QScrollView::focusOutEvent(e);
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
    
    // Delete any existing rubber -------------------------------
    if ( d->rubber )
    {
        QPainter p;
        p.begin(viewport());
        p.setRasterOp(NotROP);
        p.setPen(QPen(color0, 1));
        p.setBrush(NoBrush);

        drawRubber(&p);
        p.end();
        delete d->rubber;
        d->rubber = 0;
    }

    if (e->button() == Qt::RightButton)
    {
        IconItem* item = findItem(e->pos());
        if (item)
        {
            IconItem* prevCurrItem = d->currItem;
            d->currItem   = item;
            d->anchorItem = item;
            if (prevCurrItem)
                prevCurrItem->repaint();

            if (!item->isSelected())
                item->setSelected(true, true);
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
    
    IconItem *item = findItem(e->pos());
    if (item) 
    {
        if (e->state() & Qt::ControlButton)
        {
            item->setSelected(!item->isSelected(), false);
        }
        else if (e->state() & Qt::ShiftButton)
        {
            blockSignals(true);

            if (d->currItem)
            {
                clearSelection();

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
                            break;
                    }
                }
                else
                {
                    for (IconItem* it = item; it; it = it->nextItem())
                    {
                        it->setSelected(true, false);
                        if (it == d->currItem)
                            break;
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
        else
        {
            if (!item->isSelected())
                item->setSelected(true, true);
        }

        IconItem* prevCurrItem = d->currItem;
        d->currItem   = item;
        d->anchorItem = item;
        
        if (prevCurrItem)
            prevCurrItem->repaint();
        
        d->currItem->repaint();

        d->dragging = true;
        d->dragStartPos = e->pos();
        
        return;
    }

    // Press outside any item. 
    if (!(e->state() & Qt::ControlButton))
    {
        // unselect all if the ctrl button is not pressed        
        clearSelection();
    }
    else
    {
        // ctrl is pressed. make sure our current selection is not lost
        d->prevSelectedItems.clear();
        QPtrDictIterator<IconItem> it( d->selectedItems );
        
        for ( ; it.current(); ++it )
        {
            d->prevSelectedItems.insert(it.current(), it.current());
        }
    }
    
    d->rubber = new QRect( e->x(), e->y(), 0, 0 );

    QPainter p;
    p.begin( viewport() );
    p.setRasterOp( NotROP );
    p.setPen( QPen( color0, 1 ) );
    p.setBrush( NoBrush );
    drawRubber( &p );
    p.end();
}

void IconView::drawRubber(QPainter* p)
{
    if ( !p || !d->rubber )
        return;

    QRect r(d->rubber->normalize());

    r = contentsRectToViewport(r);

    QPoint pnt(r.x(), r.y());

    style().drawPrimitive(QStyle::PE_FocusRect, p,
                          QRect( pnt.x(), pnt.y(),
                                 r.width(), r.height() ),
                          colorGroup(), QStyle::Style_Default,
                          QStyleOption(colorGroup().base()));
}

void IconView::contentsMouseMoveEvent(QMouseEvent* e)
{
    if (e->state() == NoButton)
    {
        IconItem* item = findItem(e->pos());

        if(d->showTips)
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

                if(acceptToolTip(item, e->pos()))
                {
                    d->toolTipItem = item;
                    d->toolTipTimer->start(500, true);
                }
            }

            if(item == d->toolTipItem && !acceptToolTip(item, e->pos()))
            {
                d->toolTipItem = 0;
                d->toolTipTimer->stop();
                slotToolTip();
            }
        }

        if (KGlobalSettings::changeCursorOverIcon())
        {
            if (item && item->clickToOpenRect().contains(e->pos()))
                setCursor(KCursor::handCursor());
            else
                unsetCursor();
        }
        return;
    }

    d->toolTipItem  = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    if (d->dragging && (e->state() & Qt::LeftButton))
    {
        if ( (d->dragStartPos - e->pos()).manhattanLength()
             > QApplication::startDragDistance() )
        {
            startDrag();
        }
        return;
    }

    if (!d->rubber)
        return;

    QRect oldRubber = QRect(*d->rubber);

    d->rubber->setRight( e->pos().x() );
    d->rubber->setBottom( e->pos().y() );

    QRect nr = d->rubber->normalize();
    QRect rubberUnion = nr.unite(oldRubber.normalize());

    bool changed = false;

    QRegion paintRegion;
    viewport()->setUpdatesEnabled(false);
    blockSignals(true);

    IconViewPriv::ItemContainer *c = d->firstContainer;
    for (; c; c = c->next) 
    {
        if ( rubberUnion.intersects(c->rect) ) 
        {
            for (QValueList<IconItem*>::iterator it = c->items.begin();
                 it != c->items.end(); ++it)
            {
                IconItem* item = *it;
                if (nr.intersects(item->rect())) 
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
                    if (item->isSelected() &&  !d->prevSelectedItems.find(item))
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

    QRect r = *d->rubber;
    *d->rubber = oldRubber;

    QPainter p;
    p.begin( viewport() );
    p.setRasterOp( NotROP );
    p.setPen( QPen( color0, 1 ) );
    p.setBrush( NoBrush );
    drawRubber( &p );
    p.end();

    if (changed)
    {
        paintRegion.translate(-contentsX(), -contentsY());
        viewport()->repaint(paintRegion);
    }

    ensureVisible(e->pos().x(), e->pos().y());

    *d->rubber = r;

    p.begin(viewport());
    p.setRasterOp(NotROP);
    p.setPen(QPen(color0, 1));
    p.setBrush(NoBrush);
    drawRubber(&p);
    p.end();

    d->pressedMoved = true;
}

void IconView::contentsMouseReleaseEvent(QMouseEvent* e)
{
    d->dragging = false;
    d->prevSelectedItems.clear();

    if (d->rubber)
    {
        QPainter p;
        p.begin( viewport() );
        p.setRasterOp( NotROP );
        p.setPen( QPen( color0, 1 ) );
        p.setBrush( NoBrush );

        drawRubber( &p );
        p.end();

        delete d->rubber;
        d->rubber = 0;
    }

    if (e->state() == Qt::LeftButton)
    {
        if (d->pressedMoved)
        {
            emit signalSelectionChanged();
            d->pressedMoved = false;
            return;
        }

        // click on item
        IconItem *item = findItem(e->pos());
        if (item)
        {
            IconItem* prevCurrItem = d->currItem;
            item->setSelected(true, true);
            d->currItem   = item;
            d->anchorItem = item;
            if (prevCurrItem)
                prevCurrItem->repaint();
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
    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();
    viewport()->update();

    QScrollView::contentsWheelEvent(e);
}

void IconView::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
    if (KGlobalSettings::singleClick())
        return;

    IconItem *item = findItem(e->pos());
    if (item) 
    {
        itemClickedToOpen(item);
    }
}

void IconView::keyPressEvent(QKeyEvent* e)
{
    bool handled = false;

    if (!firstItem())
        return;

    switch ( e->key() )
    {
        case Key_Home: 
        {
            IconItem* tmp = d->currItem;
            d->currItem = firstItem();
            d->anchorItem = d->currItem;
            if (tmp)
                tmp->repaint();
    
            firstItem()->setSelected(true, true);
            ensureItemVisible(firstItem());
            handled = true;
            break;
        }
    
        case Key_End: 
        {
            IconItem* tmp = d->currItem;
            d->currItem   = lastItem();
            d->anchorItem = d->currItem;
            if (tmp)
                tmp->repaint();
    
            lastItem()->setSelected(true, true);
            ensureItemVisible(lastItem());
            handled = true;
            break;
        }
    
        case Key_Enter:
        case Key_Return: 
        {
            if (d->currItem)
            {
                emit signalReturnPressed(d->currItem);
                handled = true;
            }
            break;
        }

        case Key_Right: 
        {
            IconItem *item = 0;
            
            if (d->currItem)
            {
                if (d->currItem->nextItem())
                {
                    if (e->state() & Qt::ControlButton)
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = d->currItem->nextItem();
                        d->anchorItem = d->currItem;
                        tmp->repaint();
                        d->currItem->repaint();
    
                        item = d->currItem;
                    }
                    else if (e->state() & Qt::ShiftButton)
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = d->currItem->nextItem();
                        tmp->repaint();
    
                        // if the anchor is behind us, move forward preserving
                        // the previously selected item. otherwise unselect the
                        // previously selected item
                        if (!anchorIsBehind())
                            tmp->setSelected(false, false);
    
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
    
        case Key_Left: 
        {
            IconItem *item = 0;
            
            if (d->currItem)
            {
                if (d->currItem->prevItem())
                {
                    if (e->state() & Qt::ControlButton)
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = d->currItem->prevItem();
                        d->anchorItem = d->currItem;
                        tmp->repaint();
                        d->currItem->repaint();
    
                        item = d->currItem;
                    }
                    else if (e->state() & Qt::ShiftButton)
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = d->currItem->prevItem();
                        tmp->repaint();
    
                        // if the anchor is ahead of us, move forward preserving
                        // the previously selected item. otherwise unselect the
                        // previously selected item
                        if (anchorIsBehind())
                            tmp->setSelected(false, false);
    
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
    
        case Key_Up:
        {
            IconItem *item = 0;
            
            if (d->currItem)
            {
                int x = d->currItem->x() + itemRect().width()/2;
                int y = d->currItem->y() - d->spacing*2;
    
                IconItem *it = 0;
    
                while (!it && y > 0)
                {
                    it  = findItem(QPoint(x,y));
                    y  -= d->spacing * 2;
                }
    
                if (it)            
                {
                    if (e->state() & Qt::ControlButton)
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = it;
                        d->anchorItem = it;
                        tmp->repaint();
                        d->currItem->repaint();
    
                        item = d->currItem;
                    }
                    else if (e->state() & Qt::ShiftButton)
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
                                    break;
                            }
                        }
                        else
                        {
                            for (IconItem* i = d->currItem; i; i = i->nextItem())
                            {
                                i->setSelected(true, false);
                                if (i == d->anchorItem)
                                    break;
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
    
        case Key_Down:
        {
            IconItem *item = 0;
            
            if (d->currItem)
            {
                int x = d->currItem->x() + itemRect().width()/2;
                int y = d->currItem->y() + itemRect().height() + d->spacing*2;
    
                IconItem *it = 0;
    
                while (!it && y < contentsHeight())
                {
                    it  = findItem(QPoint(x,y));
                    y  += d->spacing * 2;
                }
    
                if (it)            
                {
                    if (e->state() & Qt::ControlButton)
                    {
                        IconItem* tmp = d->currItem;
                        d->currItem   = it;
                        d->anchorItem = it;
                        tmp->repaint();
                        d->currItem->repaint();
    
                        item = d->currItem;
                    }
                    else if (e->state() & Qt::ShiftButton)
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
                                    break;
                            }
                        }
                        else
                        {
                            for (IconItem* i = d->currItem; i; i = i->nextItem())
                            {
                                i->setSelected(true, false);
                                if (i == d->anchorItem)
                                    break;
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
    
        case Key_Next: 
        {
            IconItem *item = 0;
    
            if (d->currItem)
            {
                QRect r( 0, d->currItem->y() + visibleHeight(),
                        contentsWidth(), visibleHeight() );
                IconItem *ni = findFirstVisibleItem(r, false);
    
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
    
        case Key_Prior: 
        {
            IconItem *item = 0;
    
            if (d->currItem)
            {
                QRect r(0, d->currItem->y() - visibleHeight(),
                        contentsWidth(), visibleHeight() );
    
                IconItem *ni = findFirstVisibleItem(r, false);
    
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

        // Key_Space is used as a global shortcut in DigikamApp.
        // Ctrl+Space comes through, Shift+Space is filtered out.
        case Key_Space:
        {
            if (d->currItem)
            {
                if ( (e->state() & Qt::ControlButton) || (e->state() & Qt::ShiftButton) )
                {
                    d->currItem->setSelected(!d->currItem->isSelected(), false);
                }
                else
                {
                    if (!d->currItem->isSelected())
                        d->currItem->setSelected(true, true);
                }
                handled = true;
            }
            break;
        }

        case Key_Menu:
        {
            if (d->currItem)
            {
                if (!d->currItem->isSelected())
                    d->currItem->setSelected(true, false);

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
        return false;

    for (IconItem* it = d->anchorItem; it; it = it->nextItem())
    {
        if (it == d->currItem)
            return true;
    }

    return false;
}


void IconView::startDrag()
{
}

void IconView::ensureItemVisible(IconItem *item)
{
    if ( !item )
        return;

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
    IconViewPriv::ItemContainer *c = d->firstContainer;
    bool alreadyIntersected = false;
    IconItem* i = 0;
    for ( ; c; c = c->next )
    {
        if ( c->rect.intersects( r ) )
        {
            alreadyIntersected = true;
            for (QValueList<IconItem*>::iterator it = c->items.begin();
                 it != c->items.end(); ++it)
            {
                IconItem *item = *it;

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
                            i = item;
                        else if ( r2.y() == r3.y() &&
                                  r2.x() < r3.x() )
                            i = item;
                    }
                }
            }
        }
        else
        {
            if ( alreadyIntersected )
                break;
        }
    }

    return i;
}

IconItem* IconView::findLastVisibleItem(const QRect& r, bool useThumbnailRect) const
{
    IconViewPriv::ItemContainer *c = d->firstContainer;
    IconItem *i = 0;
    bool alreadyIntersected = false;
    for ( ; c; c = c->next ) 
    {
        if ( c->rect.intersects( r ) )
        {
            alreadyIntersected = true;
            for (QValueList<IconItem*>::iterator it = c->items.begin();
                 it != c->items.end(); ++it)
            {
                IconItem *item = *it;

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
                            i = item;
                        else if ( r2.y() == r3.y() &&
                                  r2.x() > r3.x() )
                            i = item;
                    }
                }
            }
        }
        else {
            if ( alreadyIntersected )
                break;
        }
    }

    return i;
}

void IconView::drawFrameRaised(QPainter* p)
{
    QRect       r      = frameRect();
    int         lwidth = lineWidth();

    const QColorGroup & g = colorGroup();

    qDrawShadeRect( p, r, g, false, lwidth,
                    midLineWidth() );
}

void IconView::drawFrameSunken(QPainter* p)
{
    QRect       r      = frameRect();
    int         lwidth = lineWidth();

    const QColorGroup & g = colorGroup();

    qDrawShadeRect( p, r, g, true, lwidth,
                    midLineWidth() );
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
        return;

    IconItem* prevCurrItem = d->currItem;
    d->currItem   = item;
    d->anchorItem = item;
    
    if (prevCurrItem)
        prevCurrItem->repaint();

    item->setSelected(true);
    emit signalDoubleClicked(item);
}

int IconView::cmpItems(const void *n1, const void *n2)
{
    if ( !n1 || !n2 )
        return 0;

    IconViewPriv::SortableItem *i1 = (IconViewPriv::SortableItem *)n1;
    IconViewPriv::SortableItem *i2 = (IconViewPriv::SortableItem *)n2;

    return i1->group->compare( i2->group );
}

}  // namespace Digikam

