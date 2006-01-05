/* ============================================================
 * File  : iconitem.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-24
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju
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

// Qt includes.

#include <qpixmap.h>
#include <qpainter.h>

// Local includes.

#include "icongroupitem.h"
#include "iconview.h"
#include "iconitem.h"

namespace Digikam
{

IconItem::IconItem(IconGroupItem* parent)
    : m_group(parent)
{
    m_next = 0;
    m_prev = 0;
    m_x    = 0;
    m_y    = 0;
    m_selected = false;

    m_group->insertItem(this);
}

IconItem::~IconItem()
{
    m_group->takeItem(this);
}

IconItem* IconItem::nextItem() const
{
    if (m_next)
        return m_next;

    if (m_group->nextGroup())
        return m_group->nextGroup()->firstItem();

    return 0;    
}

IconItem* IconItem::prevItem() const
{
    if (m_prev)
        return m_prev;

    if (m_group->prevGroup())
        return m_group->prevGroup()->lastItem();

    return 0;
}

int IconItem::x() const
{
    return m_x;
}

int IconItem::y() const
{
    return m_y;
}

QRect IconItem::rect() const
{
    IconView* view = m_group->iconView();
    QRect r(view->itemRect());
    r.moveTopLeft(QPoint(m_x, m_y));
    return r;
}

QRect IconItem::clickToOpenRect()
{
    return rect();
}

bool IconItem::move(int x, int y)
{
    if (m_x == x && m_y == y)
        return false;

    m_x = x; m_y = y;
    return true;
}

void IconItem::setSelected(bool val, bool cb)
{
    IconView* view = m_group->iconView();

    if (cb) {
        view->blockSignals(true);
        view->clearSelection();
        view->blockSignals(false);
    }
    
    m_selected = val;
    view->selectItem(this, val);
    view->updateContents(rect());
}

bool IconItem::isSelected() const
{
    return m_selected;
}

void IconItem::repaint(bool force)
{
    if (force)
        m_group->iconView()->repaintContents(rect());
    else
        m_group->iconView()->updateContents(rect());
}

IconView* IconItem::iconView() const
{
    return m_group->iconView();
}

int IconItem::compare(IconItem* /*item*/)
{
    return 0;
}

void IconItem::paintItem()
{
    IconView* view = m_group->iconView();

    QRect r(rect());
    QPixmap pix(r.width(), r.height());
    pix.fill(m_selected ? Qt::blue : Qt::gray);

    if (this == iconView()->currentItem())
    {
        QPainter p(&pix);
        p.setPen(QPen(m_selected ? Qt::white : Qt::black, 1, Qt::DotLine));
        p.drawRect(2, 2, r.width()-4, r.width()-4);
        p.end();
    }
        
    r = QRect(view->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));
    
    bitBlt(view->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width(), r.height());
}

}  // namespace Digikam
