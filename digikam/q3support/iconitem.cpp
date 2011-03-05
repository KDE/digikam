/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-24
 * Description : icon item.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "iconitem.moc"

// Qt includes

#include <QPixmap>
#include <QPainter>

// KDE includes

#include <kiconeffect.h>
#include <kiconloader.h>

// Local includes

#include "icongroupitem.h"
#include "iconview.h"

namespace Digikam
{

class IconItemPriv
{
public:

    IconItemPriv()
    {
        xpos        = 0;
        ypos        = 0;
        group       = 0;
        selected    = false;
        highlighted = false;
        editRating  = false;
    }

    bool           editRating;
    bool           selected;
    bool           highlighted;

    int            xpos;
    int            ypos;

    IconGroupItem* group;
};

IconItem::IconItem(IconGroupItem* parent)
    : QObject(), d(new IconItemPriv)
{
    m_next   = 0;
    m_prev   = 0;
    d->group = parent;
    d->group->insertItem(this);
}

IconItem::~IconItem()
{
    d->group->takeItem(this);
    delete d;
}

IconItem* IconItem::nextItem() const
{
    if (m_next)
    {
        return m_next;
    }

    if (d->group->nextGroup())
    {
        return d->group->nextGroup()->firstItem();
    }

    return 0;
}

IconItem* IconItem::prevItem() const
{
    if (m_prev)
    {
        return m_prev;
    }

    if (d->group->prevGroup())
    {
        return d->group->prevGroup()->lastItem();
    }

    return 0;
}

int IconItem::x() const
{
    return d->xpos;
}

int IconItem::y() const
{
    return d->ypos;
}

QRect IconItem::rect() const
{
    IconView* view = d->group->iconView();
    QRect r(view->itemRect());
    r.moveTopLeft(QPoint(d->xpos, d->ypos));
    return r;
}

QRect IconItem::toggleSelectRect() const
{
    QRect r       = d->group->iconView()->itemRect();
    QSize selSize = d->group->iconView()->selectPixmap().size();
    QRect selRect(QPoint(0, 0), selSize);

    selRect.translate(r.x()+5, r.y()+5);
    return selRect;
}

QRect IconItem::clickToOpenRect()
{
    return rect();
}

QRect IconItem::clickToRateRect()
{
    return QRect();
}

QRect IconItem::clickToToggleSelectRect() const
{
    QRect r       = rect();
    QSize selSize = d->group->iconView()->selectPixmap().size();
    QRect selRect(QPoint(0, 0), selSize);

    selRect.translate(r.x()+5, r.y()+5);
    return selRect;
}

bool IconItem::move(int x, int y)
{
    if (d->xpos == x && d->ypos == y)
    {
        return false;
    }

    d->xpos = x;
    d->ypos = y;
    return true;
}

void IconItem::setSelected(bool val, bool cb)
{
    IconView* view = d->group->iconView();

    if (cb)
    {
        view->blockSignals(true);
        view->clearSelection();
        view->blockSignals(false);
    }

    d->selected = val;
    view->selectItem(this, val);
    view->updateContents(rect());
}

bool IconItem::isSelected() const
{
    return d->selected;
}

void IconItem::setHighlighted(bool val)
{
    d->highlighted = val;
    d->group->iconView()->updateContents(rect());
}

bool IconItem::isHighlighted() const
{
    return d->highlighted;
}

void IconItem::setEditRating(bool val)
{
    d->editRating = val;
    d->group->iconView()->updateContents(rect());
}

bool IconItem::editRating() const
{
    return d->editRating;
}

void IconItem::setRating(int /*rating*/)
{
}

int IconItem::rating() const
{
    return -1;
}

void IconItem::repaint()
{
    d->group->iconView()->repaintContents(rect());
}

void IconItem::update()
{
    d->group->iconView()->updateContents(rect());
}

IconView* IconItem::iconView() const
{
    return d->group->iconView();
}

int IconItem::compare(IconItem* /*item*/)
{
    return 0;
}

void IconItem::paintItem(QPainter* p)
{
    IconView* view = d->group->iconView();

    QRect r(rect());
    QPixmap pix(r.width(), r.height());
    QColor color(d->selected ? Qt::blue : Qt::gray);
    color.setAlpha(d->highlighted ? 128 : 0);
    pix.fill(color);

    if (this == iconView()->currentItem())
    {
        QPainter p(&pix);
        p.setPen(QPen(d->selected ? Qt::white : Qt::black, 1, Qt::DotLine));
        p.drawRect(2, 2, r.width()-4, r.width()-4);
        p.end();
    }

    r = QRect(view->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));

    p->drawPixmap(r.x(), r.y(), pix, 0, 0, r.width(), r.height());
}

void IconItem::paintToggleSelectButton(QPainter* p)
{
    IconView*   view      = d->group->iconView();
    const int   fadingVal = 128;
    KIconEffect iconEffect;
    QPixmap     selPix;

    p->save();
    p->setClipRect(toggleSelectRect());
    p->setRenderHint(QPainter::Antialiasing);

    // draw an alpha blended circle as background
    const QPalette& pal   = view->palette();
    const QBrush& bgBrush = pal.brush(QPalette::Normal, QPalette::Window);
    QColor bg             = bgBrush.color();
    bg.setAlpha(fadingVal / 2);
    p->setBrush(bg);

    const QBrush& fgBrush = pal.brush(QPalette::Normal, QPalette::WindowText);
    QColor fg             = fgBrush.color();
    fg.setAlpha(fadingVal / 4);
    p->setPen(fg);

    p->drawEllipse(toggleSelectRect());

    if (isSelected())
    {
        selPix = view->deselectPixmap();
    }
    else
    {
        selPix = view->selectPixmap();
    }

    p->drawPixmap(toggleSelectRect(), iconEffect.apply(selPix, KIconLoader::Desktop, KIconLoader::ActiveState));
    p->restore();
}

}  // namespace Digikam
