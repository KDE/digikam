/* ============================================================
 * File  : folderview.cpp
 * Author: Joern Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-05-06
 * Copyright 2005 by Joern Ahrens
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
 * ============================================================ */

#include <kglobalsettings.h>
#include <kcursor.h>
#include <kdebug.h>

#include <qpixmap.h>
#include <qapp.h>

#include "themeengine.h"
#include "folderview.h"
#include "folderitem.h"
#include "dragobjects.h"

//-----------------------------------------------------------------------------
// FolderViewPriv
//-----------------------------------------------------------------------------

class FolderViewPriv
{
public:

    bool        active;
    QPixmap     itemRegPix;
    QPixmap     itemSelPix;
    int         itemHeight;
    QPoint      dragStartPos;    
    FolderItem  *dragItem;
};

//-----------------------------------------------------------------------------
// FolderViewPriv
//-----------------------------------------------------------------------------

FolderView::FolderView(QWidget *parent)
    : QListView(parent)
{
    d = new FolderViewPriv;
    
    d->active = false;
    d->dragItem = 0;

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            SLOT(slotThemeChanged()));

    setColumnAlignment(0, Qt::AlignLeft|Qt::AlignVCenter);
    fontChange(font());
}

FolderView::~FolderView()
{
    delete d;
}

void FolderView::setActive(bool val)
{
    d->active = val;

    if (d->active)
        slotSelectionChanged();
}

bool FolderView::active() const
{
    return d->active;
}

int FolderView::itemHeight() const
{
    return d->itemHeight;
}

QPixmap FolderView::itemBasePixmapRegular() const
{
    return d->itemRegPix;    
}

QPixmap FolderView::itemBasePixmapSelected() const
{
    return d->itemSelPix;    
}

void FolderView::resizeEvent(QResizeEvent* e)
{
    QListView::resizeEvent(e);

    int w = frameRect().width();
    int h = itemHeight();
    if (d->itemRegPix.width() != w ||
        d->itemRegPix.height() != h)
    {
        slotThemeChanged();
    }
}

void FolderView::fontChange(const QFont& oldFont)
{
    d->itemHeight = QMAX(32 + 2*itemMargin(), fontMetrics().height());
    QListView::fontChange(oldFont);
    slotThemeChanged();
}

void FolderView::contentsMouseMoveEvent(QMouseEvent *e)
{
    QListView::contentsMouseMoveEvent(e);

    if(e->state() == NoButton)
    {
        if(KGlobalSettings::changeCursorOverIcon())
        {
            QPoint vp = contentsToViewport(e->pos());
            QListViewItem *item = itemAt(vp);
            if (mouseInItemRect(item, vp.x()))
                setCursor(KCursor::handCursor());
            else
                unsetCursor();
        }
        return;
    }
    
    if(d->dragItem && 
       (d->dragStartPos - e->pos()).manhattanLength() > QApplication::startDragDistance())
    {
        FolderItem *item = dynamic_cast<FolderItem*>(itemAt(e->pos()));
        if(!item)
        {
            d->dragItem = 0;
            return;
        }
    }    
}

void FolderView::contentsMousePressEvent(QMouseEvent *e)
{
    QListView::contentsMousePressEvent(e);

    FolderItem *item = dynamic_cast<FolderItem*>(itemAt(e->pos()));
    if(item && e->button() == LeftButton) {
        d->dragStartPos = e->pos();
        d->dragItem = item;
        return;
    }
}

void FolderView::contentsMouseReleaseEvent(QMouseEvent *e)
{
    QListView::contentsMouseReleaseEvent(e);

    d->dragItem = 0;
}

void FolderView::startDrag()
{
    dragObject()->drag();        
}

FolderItem* FolderView::dragItem() const
{
    return d->dragItem;
}

bool FolderView::mouseInItemRect(QListViewItem* item, int x) const
{
    if (!item)
        return false;
    
    x += contentsX();

    int offset = treeStepSize()*(item->depth() + (rootIsDecorated() ? 1 : 0));
    offset    += itemMargin();
    int width = item->width(fontMetrics(), this, 0);
    
    return (x > offset && x < (offset + width));
}

void FolderView::slotThemeChanged()
{
    int w = frameRect().width();
    int h = itemHeight();

    d->itemRegPix = ThemeEngine::instance()->listRegPixmap(w, h);
    d->itemSelPix = ThemeEngine::instance()->listSelPixmap(w, h);

    QPalette plt(palette());
    QColorGroup cg(plt.active());
    cg.setColor(QColorGroup::Base, ThemeEngine::instance()->baseColor());
    cg.setColor(QColorGroup::Text, ThemeEngine::instance()->textRegColor());
    cg.setColor(QColorGroup::HighlightedText, ThemeEngine::instance()->textSelColor());
    cg.setColor(QColorGroup::Link, ThemeEngine::instance()->textSpecialRegColor());
    cg.setColor(QColorGroup::LinkVisited, ThemeEngine::instance()->textSpecialSelColor());
    plt.setActive(cg);
    plt.setInactive(cg);
    setPalette(plt);

    viewport()->update();
}

void FolderView::slotSelectionChanged()
{
    QListView::selectionChanged();    
}

#include "folderview.moc"
