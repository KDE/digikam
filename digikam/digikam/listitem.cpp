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

#include <qpainter.h>
#include <qpixmap.h>
#include <qpalette.h>

#include "listview.h"
#include "listview_p.h"
#include "listitem.h"

ListItem::ListItem()
{
    init();
}

ListItem::ListItem(ListView* parent, const QString& text)
{
    init();
    m_text     = text;
    m_listView = parent;
    
    if (parent) {
        parent->d->rootItem->insertChild(this);
    }
}

ListItem::ListItem(ListItem* parent, const QString& text)
{
    init();    
    m_text = text;

    if (parent)
        parent->insertChild(this);

    const ListItem* c = this;
    while ( c && !c->m_root ) {
        c = c->m_parent;
    }
    
    if ( c ) 
        m_listView = c->m_listView;
}

ListItem::~ListItem()
{
    clear();
    if (m_parent) 
        m_parent->removeChild(this);

    if (m_pixmap)
        delete m_pixmap;
}

void ListItem::init()
{
    m_root       = false;
    m_open       = false;
    m_selected   = false;
    m_expandable = false;
    m_clearing   = false;
    m_truncated  = false;
    m_depth      = 0;
    m_offset     = 0;
    m_pos        = 0;
    m_childCount = 0;

    m_pixmap     = 0;
    m_listView   = 0;

    m_parent     = 0;
    m_firstChild = 0;
    m_lastChild  = 0;
    m_next       = 0;
    m_prev       = 0;
}

void ListItem::insertChild(ListItem* child)
{
    if (!child)
        return;

    if (!m_firstChild) {
        m_firstChild = child;
        m_lastChild  = child;
        child->m_next = 0;
        child->m_prev = 0;
    }
    else {
        m_lastChild->m_next = child;
        child->m_prev  = m_lastChild;
        child->m_next  = 0;
        m_lastChild    = child;
    }

    if (m_root) {
        child->m_offset = 0;
        child->m_depth  = 0;
    }
    else {
        child->m_offset = m_offset + 20;
        child->m_depth  = m_depth  + 1;
        adjustChildOffsets();
    }
    
    m_expandable = true;
    m_childCount++;
    child->m_parent = this;

    if (m_listView)
        m_listView->triggerUpdate();
}

void ListItem::adjustChildOffsets()
{
    ListItem *child = m_firstChild;
    while(child) {
        child->m_offset = m_offset + 20;
        child->m_depth  = m_depth + 1;
        child->adjustChildOffsets();
        child = child->m_next;
    }
}

void ListItem::removeChild(ListItem* child)
{
    if (!child || m_clearing)
        return;

    if (child == m_firstChild) {
        m_firstChild = m_firstChild->m_next;
        if (m_firstChild)
            m_firstChild->m_prev = 0;
        else
            m_firstChild = m_lastChild = 0;
    }
    else if (child == m_lastChild) {
        m_lastChild = m_lastChild->m_prev;
        if (m_lastChild)
            m_lastChild->m_next = 0;
        else
            m_firstChild = m_lastChild = 0;
    }
    else {
        if (child->m_prev)
            child->m_prev->m_next = child->m_next;
        if (child->m_next)
            child->m_next->m_prev = child->m_prev;
    }

    m_expandable = (m_firstChild != 0);
    m_childCount--;
    child->m_parent = 0;
    
    if (m_listView) {
        m_listView->takeItem(child);

        if (m_listView->d->selectedItem == this)
            m_listView->d->selectedItem = 0;
        
        m_listView->triggerUpdate();
    }
}

void ListItem::clear()
{
    m_clearing = true;
    
    m_firstChild = 0;
    m_lastChild  = 0;
    m_childCount = 0;
    
    ListItem* child = m_firstChild;
    ListItem* nextChild;
    while (child) {
        nextChild = child->m_next;
        delete child;
        child = nextChild;
    }
    
    m_clearing = false;

    if (m_listView) {

        if (m_listView->d->selectedItem == this)
            m_listView->d->selectedItem = 0;
        
        m_listView->triggerUpdate();
    }
}

ListView* ListItem::listView() const
{
    return m_listView;
}
bool ListItem::isExpandable() const
{
    return m_expandable;
}

bool ListItem::isSelected() const
{
    return m_listView ? m_listView->d->selectedItem == this : false;
}

bool ListItem::isTruncated() const
{
    return m_truncated;
}

void ListItem::setOpen(bool val)
{
    if (val == m_open)
        return;

    m_open = val;

    if (m_listView) {

        ListItem* item = m_listView->getSelected();
        if (item && item != this) {

            bool selIsMyChild = false;
            item = item->m_parent;
            while (item && !item->m_root) {
                if (item == this) {
                    selIsMyChild = true;
                    break;
                }
                item = item->m_parent;
            }

            if (selIsMyChild)
                m_listView->setSelected(this);
        }

        //m_listView->triggerUpdate();
        m_listView->slotUpdateContents();
    }
}

bool ListItem::isOpen() const
{
    return m_open;
}

bool ListItem::isRoot() const
{
    return m_root;
}

void ListItem::repaint()
{
    if (m_listView)
        m_listView->repaintItem(this);
}

void ListItem::setText(const QString& text)
{
    m_text = text;
    repaint();
}

QString ListItem::text() const
{
    return m_text;    
}

void ListItem::setPixmap(const QPixmap& pixmap)
{
    if (m_pixmap) 
        delete m_pixmap;
    
    m_pixmap = new QPixmap(pixmap);
    repaint();
}

QPixmap* ListItem::pixmap() const
{
    return m_pixmap;    
}

void ListItem::paint(QPainter *p, const QColorGroup&, const QRect& r)
{
    if (!m_listView)
        return;

    QRect tr(r);

    if (m_pixmap) {
        p->drawPixmap(r.x(), (r.height()-m_pixmap->height())/2, *m_pixmap);
        tr.setLeft(tr.x() + m_pixmap->width() + m_listView->d->itemMargin);
    }

    p->drawText(tr, Qt::AlignLeft|Qt::AlignVCenter, m_text, -1);

    // TODO : find a way to see if item text is truncated. 
    // so that we can show a tooltip 
}

int ListItem::compare(ListItem* item) const
{
    return m_text.localeAwareCompare(item->m_text);
}

int ListItem::childCount() const
{
    return m_childCount;    
}

ListItem* ListItem::parent() const
{
    return m_parent;
}

ListItem* ListItem::firstChild()
{
    return m_firstChild;
}

ListItem* ListItem::lastChild()
{
    return m_lastChild;    
}

ListItem* ListItem::next()
{
    return m_next;    
}

ListItem* ListItem::prev()
{
    return m_prev;
}
