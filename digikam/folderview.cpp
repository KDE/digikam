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
 *
 * ============================================================ */

// Qt includes.

#include <qpixmap.h>
#include <qvaluelist.h>

// KDE includes.

#include <kglobalsettings.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>

// Local includes.

#include "albummanager.h"
#include "themeengine.h"
#include "folderview.h"
#include "folderitem.h"
#include "dragobjects.h"

namespace Digikam
{

//-----------------------------------------------------------------------------
// FolderViewPriv class
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
    
    FolderItem  *oldHighlightItem;
};

//-----------------------------------------------------------------------------
// FolderView class
//-----------------------------------------------------------------------------

FolderView::FolderView(QWidget *parent, const char *name)
    : QListView(parent, name)
{
    d = new FolderViewPriv;
    
    d->active = false;
    d->dragItem = 0;
    d->oldHighlightItem = 0;

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            SLOT(slotThemeChanged()));

    connect(AlbumManager::instance(), SIGNAL(signalAllAlbumsLoaded()),
            SLOT(slotAllAlbumsLoaded()));
    
    setColumnAlignment(0, Qt::AlignLeft|Qt::AlignVCenter);
    fontChange(font());
}

FolderView::~FolderView()
{
    saveViewState();
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

QRect FolderView::itemRect(QListViewItem *item) const
{
    if(!item)
        return QRect();
    
    QRect r = QListView::itemRect(item);
    r.setLeft(r.left()+(item->depth()+(rootIsDecorated() ? 1 : 0))*treeStepSize());
    return r;    
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
        QPoint vp = contentsToViewport(e->pos());
        FolderItem *item = dynamic_cast<FolderItem*>(itemAt(vp));
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

    QPoint vp = contentsToViewport(e->pos());
    FolderItem *item = dynamic_cast<FolderItem*>(itemAt(vp));
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
    QDragObject *o = dragObject();
    if(o)
        o->drag();        
}

FolderItem* FolderView::dragItem() const
{
    return d->dragItem;
}

void FolderView::contentsDragEnterEvent(QDragEnterEvent *e)
{
    QListView::contentsDragEnterEvent(e);
    
    e->accept(acceptDrop(e));
}

void FolderView::contentsDragLeaveEvent(QDragLeaveEvent * e)
{
    QListView::contentsDragLeaveEvent(e);
    
    if(d->oldHighlightItem)
    {
        d->oldHighlightItem->setFocus(false);
        d->oldHighlightItem->repaint();
        d->oldHighlightItem = 0;        
    }
}

void FolderView::contentsDragMoveEvent(QDragMoveEvent *e)
{
    QListView::contentsDragMoveEvent(e);
    
    QPoint vp = contentsToViewport(e->pos());
    FolderItem *item = dynamic_cast<FolderItem*>(itemAt(vp));
    if(item)
    {
        if(d->oldHighlightItem)
        {
            d->oldHighlightItem->setFocus(false);
            d->oldHighlightItem->repaint();
        }
        item->setFocus(true);
        d->oldHighlightItem = item;
        item->repaint();
    }
    e->accept(acceptDrop(e));
}

void FolderView::contentsDropEvent(QDropEvent *e)
{
    QListView::contentsDropEvent(e);
    
    if(d->oldHighlightItem)
    {
        d->oldHighlightItem->setFocus(false);
        d->oldHighlightItem->repaint();
        d->oldHighlightItem = 0;
    }
}

bool FolderView::acceptDrop(const QDropEvent *) const
{
    return false;
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

void FolderView::slotAllAlbumsLoaded()
{
    disconnect(AlbumManager::instance(), SIGNAL(signalAllAlbumsLoaded()),
               this, SLOT(slotAllAlbumsLoaded()));    
    loadViewState();
}

void FolderView::loadViewState()
{
    KConfig *config = kapp->config();
    config->setGroup(name());
    
    int selectedItem = config->readNumEntry("LastSelectedItem", 0);
    
    QValueList<int> openFolders;
    if(config->hasKey("OpenFolders"))
    {
        openFolders = config->readIntListEntry("OpenFolders");
    }
    
    FolderItem *item;    
    QListViewItemIterator it(this->lastItem());
    for( ; it.current(); --it)
    {
        item = dynamic_cast<FolderItem*>(it.current());
        if(!item)
            continue;
        // Start the album root always open
        if(openFolders.contains(item->id()) || item->id() == 0)
            setOpen(item, true);
        else
            setOpen(item, false);
        if(item->id() == selectedItem)
        {
            setSelected(item, true);
            ensureItemVisible(item);
        }
    }
}

void FolderView::saveViewState()
{
    KConfig *config = kapp->config();
    config->setGroup(name());
   
    FolderItem *item = dynamic_cast<FolderItem*>(selectedItem());
    if(item)
        config->writeEntry("LastSelectedItem", item->id());
    else
        config->writeEntry("LastSelectedItem", 0);
    
    QValueList<int> openFolders;
    QListViewItemIterator it(this);
    for( ; it.current(); ++it)
    {
        item = dynamic_cast<FolderItem*>(it.current());
        if(item && isOpen(item))
            openFolders.push_back(item->id());
    }
    config->writeEntry("OpenFolders", openFolders);
}

void FolderView::slotSelectionChanged()
{
    QListView::selectionChanged();    
}

void FolderView::selectItem(int)
{
}

}  // namespace Digikam
    
#include "folderview.moc"
