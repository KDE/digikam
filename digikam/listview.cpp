/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-21
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <cstdio>
#include <cstdlib>

#include <qpixmap.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qpalette.h>
#include <qpainter.h>
#include <qevent.h>
#include <qpointarray.h>
#include <qstyle.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qdrawutil.h>

#include "listitem.h"
#include "listview_p.h"
#include "listview.h"

class ListViewToolTip : public QToolTip
{
public:

    ListViewToolTip(QWidget *parent, ListView *lv);

    void maybeTip(const QPoint &pos);

private:
    
    ListView *m_view;
};

ListViewToolTip::ListViewToolTip(QWidget *parent, ListView *lv)
    : QToolTip(parent), m_view(lv)
{
}

void ListViewToolTip::maybeTip(const QPoint &pos)
{
    if ( !parentWidget() || !m_view)
        return;

    ListItem *item = m_view->itemAt(m_view->viewportToContents(pos));
    if (!item || !item->isTruncated())
        return;

    QRect r(m_view->itemRect(item));
    r.setTopLeft(m_view->contentsToViewport(r.topLeft()));
    
    tip(r, item->text());
}


static int listviewCmpItems(const void *n1, const void *n2 )
{
    if ( !n1 || !n2 )
        return 0;

    ListViewPriv::SortableItem *i1 = (ListViewPriv::SortableItem *)n1;
    ListViewPriv::SortableItem *i2 = (ListViewPriv::SortableItem *)n2;

    return i1->item->compare( i2->item );
}

ListView::ListView(QWidget* parent, const char* name,
                   WFlags f)
    : QScrollView(parent, name, f | WStaticContents |
#if QT_VERSION < 0x030200
                  WResizeNoErase | WRepaintNoErase
#else
                  WNoAutoErase
#endif
        )
    
{
    setHScrollBarMode(QScrollView::AlwaysOff);
    viewport()->setBackgroundMode(Qt::NoBackground);
    viewport()->setFocusProxy(this);
    viewport()->setFocusPolicy(QWidget::WheelFocus);

    //new ListViewToolTip(viewport(), this);
    
    // --------------------------------------------------

    d = new ListViewPriv;
    d->timer = new QTimer;

    connect(d->timer, SIGNAL(timeout()),
            SLOT(slotUpdateContents()));
    
    // hidden root item which contains all the items
    d->rootItem             = new ListItem();
    d->rootItem->m_root     = true;
    d->rootItem->m_open     = true;
    d->rootItem->m_offset   =  0;
    d->rootItem->m_depth    = -1;
    d->rootItem->m_listView = this;

    d->selectedItem         = 0;
    d->itemMargin           = 2;
    d->controlSize          = 12;
    d->arrowBoxPos          = d->itemMargin;

    fontChange(font());
}

ListView::~ListView()
{
    delete d->timer;
    d->timer = 0;
    
    delete d->rootItem;
    delete d;
}

void ListView::clear()
{
    d->visibleItems.clear();
    d->rootItem->clear();
}

void ListView::triggerUpdate()
{
    if (!d->timer)
        return;
    
    d->timer->stop();
    d->timer->start(0, true);
}

void ListView::slotUpdateContents()
{
    // TODO: right now we are sorting at every triggerupdate. change that
    
    viewport()->setUpdatesEnabled(false);
    sort();
    rearrangeItems();
    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

void ListView::rearrangeItems()
{
    d->visibleItems.clear();
    d->visibleItems.resize(100);
    
    uint count = 0;
    uint  pos  = 0;
    
    layoutItem(d->rootItem->m_firstChild, pos, count);
    d->visibleItems.resize(count);

    if (count) {
        ListItem* item = d->visibleItems.at(count-1);
        resizeContents(frameRect().width(),
                       item->m_pos + d->itemHeight + d->itemMargin);
    }
    else {
        resizeContents(0,0);
    }
}

void ListView::layoutItem(ListItem* item, uint& pos, uint& count)
{
    if (!item)
        return;

    ListItem* it   = item;
    ListItem* next = 0;

    while (it) {
        next = it->m_next;

        it->m_pos = pos;
        d->visibleItems.insert(count, it);

        pos += d->itemMargin + d->itemHeight;
        count++;

        if (d->visibleItems.size() <= count) {
            d->visibleItems.resize(count + 100);
        }
        
        if (it->m_open) {
            layoutItem(it->m_firstChild, pos, count);
        }
        
        it = next;
    }
}

void ListView::fontChange(const QFont& oldFont)
{
    QFontMetrics fm(font());
    QRect r = QRect(fm.boundingRect(0, 0, 0xFFFFFFFF,
                                    0xFFFFFFFF, Qt::AlignLeft,
                                    "XXXXXX"));
    d->itemHeight = QMAX(r.height(), 32) + 4;
    
    QScrollView::fontChange(oldFont);

    triggerUpdate();
}

int ListView::itemHeight() const
{
    return d->itemHeight;    
}

void ListView::viewportPaintEvent(QPaintEvent *pe)
{
    QRect r(pe->rect());
    QRegion paintRegion(pe->region());

    QPainter painter(viewport());
    painter.setClipRegion(paintRegion);

    int x, y;
    viewportToContents (r.x(), r.y(), x, y);

    QPixmap pix(frameRect().width(), d->itemHeight);
    QPainter p;

    for (uint i=0; i<d->visibleItems.size(); ++i) {

        ListItem* item = d->visibleItems.at(i);
        if (!item) continue;
        QRect ir(contentsToViewport(QPoint(d->itemMargin, item->m_pos)),
                 QSize(frameRect().width(), d->itemHeight));

        if (r.intersects(ir)) {

            p.begin(&pix);

            paintItemBase(&p, colorGroup(), QRect(0,0,pix.width(), pix.height()),
                          item == d->selectedItem);
            
            p.setPen( item == d->selectedItem ? colorGroup().highlightedText() :
                      colorGroup().text() );

            x = d->arrowBoxPos + item->m_offset;
            y = pix.height()/2 - d->controlSize/2;
            if (item->isExpandable()) 
                drawArrow(&p, QRect(x, y, d->controlSize, d->controlSize),
                          item->isOpen(), item == d->selectedItem);
            
            x += d->controlSize + d->itemMargin;
            item->paint(&p, colorGroup(),
                        QRect(x,0,pix.width()-x,d->itemHeight));

            p.end();
            
            bitBlt(viewport(), ir.x(), ir.y(), &pix);
            
            paintRegion -= ir;
        }
    }
    
    painter.setClipRegion(paintRegion);
    painter.fillRect(r, colorGroup().base());
    
    painter.end();
}

void ListView::paintItemBase(QPainter* p, const QColorGroup& cg,
                             const QRect& r, bool selected)
{
    p->save();
    
    if (selected)
    {
        p->fillRect(r, cg.highlight());
        p->setPen(cg.text());
        p->drawRect(r);
    }
    else
    {
        p->fillRect(r, cg.base());
    }

    p->restore();
}

void ListView::drawArrow(QPainter* p, const QRect& r, bool open,
                         bool selected)
{
    p->save();

    QPointArray a;

    if (open)
        a.setPoints( 9, -4,-3, 4,-3, -3,-2, 3,-2,
                     -2,-1, 2,-1, -1,0, 1,0, 0,1 );
    else
        a.setPoints( 9, -3,-4, -3,4, -2,-3, -2,3,
                     -1,-2, -1,2, 0,-1, 0,1, 1,0 );
    
    a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
    if (selected)
        p->setPen( colorGroup().highlightedText() );
    else
        p->setPen( colorGroup().text() );
    p->drawLineSegments( a, 0, 4 );
    p->drawPoint( a[8] );

    p->restore();
}

void ListView::drawFrameRaised(QPainter *p)
{
    QRect       r      = frameRect();
    int         lwidth = lineWidth();

    const QColorGroup& g = colorGroup();

    qDrawShadeRect( p, r, g, false, lwidth,
                    midLineWidth() );
}

void ListView::drawFrameSunken(QPainter *p)
{
    QRect       r      = frameRect();
    int         lwidth = lineWidth();

    const QColorGroup& g = colorGroup();

    qDrawShadeRect( p, r, g, true, lwidth,
                    midLineWidth() );
}

void ListView::contentsMousePressEvent(QMouseEvent *e)
{
    ListItem* item = itemAt(e->pos());
    if (!item)
        return;

    if (item->isExpandable()) {
        QRect r(d->arrowBoxPos + item->m_offset,
                item->m_pos + d->itemHeight/2 - d->controlSize/2,
                d->controlSize, d->controlSize);
        if (r.contains(e->pos())) {
            item->setOpen(!item->isOpen());
            return;
        }
    }
    
    setSelected(item);

    if (e->button() == Qt::RightButton) {
        emit signalRightButtonPressed(item);
        return;
    }
}

void ListView::contentsMouseReleaseEvent(QMouseEvent *)
{
}

void ListView::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
    if (!e || e->button() != Qt::LeftButton)
        return;

    ListItem* item = itemAt(e->pos());
    if (!item)
        return;

    emit signalDoubleClicked(item);
}

void ListView::keyPressEvent(QKeyEvent *e)
{
    if (!e)
        return;

    if(!e->state())
    {
        switch(e->key()) {
        case (Qt::Key_Down): {
            if (!d->selectedItem)
                setSelected(d->rootItem->m_firstChild);
            else {
                int pos = d->visibleItems.findRef(d->selectedItem);
                if (pos != -1 && pos < (int)d->visibleItems.size()-1) {
                    pos++;
                    ListItem* item = d->visibleItems.at(pos);
                    if (item) {
                        setSelected(item);
                        ensureVisible(0, item->m_pos+d->itemHeight/2);
                    }
                }
            }
            break;
        }
    
        case (Qt::Key_Up): {
            if (!d->selectedItem)
                setSelected(d->rootItem->m_firstChild);
            else {
                int pos = d->visibleItems.findRef(d->selectedItem);
                if (pos != -1 && pos > 0) {
                    pos--;
                    ListItem* item = d->visibleItems.at(pos);
                    if (item) {
                        setSelected(item);
                        ensureVisible(0, item->m_pos+d->itemHeight/2);
                    }
                }
            }
            break;
        }
    
        case (Qt::Key_Right): {
            if (d->selectedItem)
            {
                if (!d->selectedItem->isOpen())
                {
                    d->selectedItem->setOpen(true);
                }
                else
                {
                    ListItem *child = d->selectedItem->firstChild();
                    if (child)
                        setSelected(child);
                }
            }       
            break;
        }
    
        case (Qt::Key_Left): {
            if (d->selectedItem)
            {
                if (d->selectedItem->isOpen())
                {
                    d->selectedItem->setOpen(false);
                }
                else
                {
                    ListItem* p = d->selectedItem->parent();
                    if (p && p != d->rootItem)
                    {
                        setSelected(p);
                    }
                }
            }
            break;
        }
            
        default:
            e->ignore();
        }
    }
    else
        e->ignore();
}

ListItem* ListView::itemAt(const QPoint& pt)
{
    int x = pt.x();
    if (x < 0 || x > frameRect().width())
        return 0;
    
    int y = pt.y();
    int pos = y/(d->itemHeight + d->itemMargin);

    if (pos >= (int)d->visibleItems.size())
        return 0;

    return d->visibleItems.at(pos);
}

QRect ListView::itemRect(ListItem* item) const
{
    QRect r(0, item->m_pos, frameRect().width(), d->itemHeight);
    return r;
}

void ListView::setSelected(ListItem* item)
{
    if (item == d->selectedItem)
        return;

    ensureItemVisible(item);        
    d->selectedItem = item;
    viewport()->update();
    
    emit signalSelectionChanged(item);
}

ListItem* ListView::getSelected() const
{
    return d->selectedItem;    
}

void ListView::repaintItem(ListItem* item)
{
    if (!item)
        return;
    viewport()->update();    
}

void ListView::sort()
{
    sortChildItems(d->rootItem);    
}

void ListView::ensureItemVisible(ListItem *item)
{
    if (!item)
        return;

    ListItem *parent = item->parent();
    while ( parent ) {
        if ( !parent->isOpen() )
            parent->setOpen( true );
        parent = parent->parent();
    }

    ensureVisible(0, item->m_pos+d->itemHeight/2);
}

void ListView::sortChildItems(ListItem* parent)
{
    if (!parent || !parent->m_firstChild)
        return;

    int count = parent->childCount();
    
    ListViewPriv::SortableItem *items
        = new ListViewPriv::SortableItem[ count ];

    ListItem *item = parent->m_firstChild;
    int i = 0;
    for ( ; item; item = item->m_next )
        items[ i++ ].item = item;

    qsort( items, count, sizeof( ListViewPriv::SortableItem ),
           listviewCmpItems );

    ListItem *prev = 0;
    item = 0;
    for ( i = 0; i < count; ++i ) {
        item = items[ i ].item;
        if ( item ) {
            item->m_prev = prev;
            if ( item->m_prev )
                item->m_prev->m_next = item;
            item->m_next = 0;
        }
        if ( i == 0 )
            parent->m_firstChild = item;
        if ( i == count - 1 )
            parent->m_lastChild = item;
        prev = item;
    }

    delete [] items;

    for ( item = parent->m_firstChild; item; item = item->m_next )
        sortChildItems(item);
}

void ListView::takeItem(ListItem* item)
{
    int pos = d->visibleItems.find(item);
    if (pos != -1)
        d->visibleItems.remove(pos);    
}

QSize ListView::sizeHint() const
{
    if (cachedSizeHint().isValid())
        return cachedSizeHint();

    constPolish();

    QSize s(0,0);
    if (verticalScrollBar()->isVisible())
        s.setWidth(s.width() + style().pixelMetric(QStyle::PM_ScrollBarExtent));
    s += QSize(frameWidth()*2,frameWidth()*2);

    s.setHeight(s.height() + 10 * d->itemHeight);
    
    if ( s.width() > s.height() * 3 )
	s.setHeight( s.width() / 3 );
    else if ( s.width() *3 < s.height() )
	s.setHeight( s.width() * 3 );

    setCachedSizeHint( s );

    return s;
}

#include "listview.moc"
