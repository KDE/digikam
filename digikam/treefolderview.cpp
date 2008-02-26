/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : Tree folder view.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QList>
#include <QPixmap>

// KDE includes.

#include <kglobalsettings.h>
#include <kcursor.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kconfiggroup.h>

// Local includes.

#include "ddebug.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "albumthumbnailloader.h"
#include "themeengine.h"
#include "dragobjects.h"
#include "treefolderitem.h"
#include "treefolderview.h"
#include "treefolderview.moc"

namespace Digikam
{

class TreeFolderViewPriv
{
public:

    TreeFolderViewPriv()
    {
        active           = false;
        dragItem         = 0;
        oldHighlightItem = 0;
    }

    bool             active;

    int              itemHeight;

    QPixmap          itemRegPix;
    QPixmap          itemSelPix;

    QPoint           dragStartPos;

    TreeFolderItem  *dragItem;
    TreeFolderItem  *oldHighlightItem;
};

//-----------------------------------------------------------------------------

TreeFolderView::TreeFolderView(QWidget *parent, const char *name)
              : Q3ListView(parent)
{
    d = new TreeFolderViewPriv;
    setObjectName(name);

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(AlbumManager::instance(), SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(slotAllAlbumsLoaded()));

    connect(AlbumThumbnailLoader::instance(), SIGNAL(signalReloadThumbnails()),
            this, SLOT(slotIconSizeChanged()));

    setColumnAlignment(0, Qt::AlignLeft|Qt::AlignVCenter);
    fontChange(font());
}

TreeFolderView::~TreeFolderView()
{
    saveViewState();
    delete d;
}

void TreeFolderView::setActive(bool val)
{
    d->active = val;

    if (d->active)
        slotSelectionChanged();
}

bool TreeFolderView::active() const
{
    return d->active;
}

int TreeFolderView::itemHeight() const
{
    return d->itemHeight;
}

QRect TreeFolderView::itemRect(Q3ListViewItem *item) const
{
    if(!item)
        return QRect();

    QRect r = Q3ListView::itemRect(item);
    r.setLeft(r.left()+(item->depth()+(rootIsDecorated() ? 1 : 0))*treeStepSize());
    return r;
}

QPixmap TreeFolderView::itemBasePixmapRegular() const
{
    return d->itemRegPix;
}

QPixmap TreeFolderView::itemBasePixmapSelected() const
{
    return d->itemSelPix;    
}

void TreeFolderView::resizeEvent(QResizeEvent* e)
{
    Q3ListView::resizeEvent(e);

    int w = frameRect().width();
    int h = itemHeight();
    if (d->itemRegPix.width() != w ||
        d->itemRegPix.height() != h)
    {
        slotThemeChanged();
    }
}

void TreeFolderView::fontChange(const QFont& oldFont)
{
    // this is bad, since the settings value might not always be the _real_ height of the thumbnail.
    // (e.g. when it is blended, as for the tags)
    d->itemHeight = qMax(AlbumThumbnailLoader::instance()->thumbnailSize() + 2*itemMargin(), fontMetrics().height());
    Q3ListView::fontChange(oldFont);
    slotThemeChanged();
}

void TreeFolderView::slotIconSizeChanged()
{
    d->itemHeight = qMax(AlbumThumbnailLoader::instance()->thumbnailSize() + 2*itemMargin(), fontMetrics().height());
    slotThemeChanged();
}

void TreeFolderView::contentsMouseMoveEvent(QMouseEvent *e)
{
    Q3ListView::contentsMouseMoveEvent(e);

    if(e->buttons() == Qt::NoButton)
    {
        if(KGlobalSettings::changeCursorOverIcon())
        {
            QPoint vp = contentsToViewport(e->pos());
            Q3ListViewItem *item = itemAt(vp);
            if (mouseInItemRect(item, vp.x()))
                setCursor(Qt::PointingHandCursor);
            else
                unsetCursor();
        }
        return;
    }

    if(d->dragItem && 
       (d->dragStartPos - e->pos()).manhattanLength() > QApplication::startDragDistance())
    {
        QPoint vp = contentsToViewport(e->pos());
        TreeFolderItem *item = dynamic_cast<TreeFolderItem*>(itemAt(vp));
        if(!item)
        {
            d->dragItem = 0;
            return;
        }
    }
}

void TreeFolderView::contentsMousePressEvent(QMouseEvent *e)
{
    Q3ListView::contentsMousePressEvent(e);

    QPoint vp            = contentsToViewport(e->pos());
    TreeFolderItem *item = dynamic_cast<TreeFolderItem*>(itemAt(vp));

    if(item && e->button() == Qt::LeftButton) 
    {
        d->dragStartPos = e->pos();
        d->dragItem     = item;
        return;
    }
}

void TreeFolderView::contentsMouseReleaseEvent(QMouseEvent *e)
{
    Q3ListView::contentsMouseReleaseEvent(e);

    d->dragItem = 0;
}

void TreeFolderView::startDrag()
{
    Q3DragObject *o = dragObject();
    if(o)
        o->drag();
}

TreeFolderItem* TreeFolderView::dragItem() const
{
    return d->dragItem;
}

void TreeFolderView::contentsDragEnterEvent(QDragEnterEvent *e)
{
    Q3ListView::contentsDragEnterEvent(e);

    e->setAccepted(acceptDrop(e));
}

void TreeFolderView::contentsDragLeaveEvent(QDragLeaveEvent * e)
{
    Q3ListView::contentsDragLeaveEvent(e);

    if(d->oldHighlightItem)
    {
        d->oldHighlightItem->setFocus(false);
        d->oldHighlightItem->repaint();
        d->oldHighlightItem = 0;
    }
}

void TreeFolderView::contentsDragMoveEvent(QDragMoveEvent *e)
{
    Q3ListView::contentsDragMoveEvent(e);

    QPoint vp            = contentsToViewport(e->pos());
    TreeFolderItem *item = dynamic_cast<TreeFolderItem*>(itemAt(vp));
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
    e->setAccepted(acceptDrop(e));
}

void TreeFolderView::contentsDropEvent(QDropEvent *e)
{
    Q3ListView::contentsDropEvent(e);

    if(d->oldHighlightItem)
    {
        d->oldHighlightItem->setFocus(false);
        d->oldHighlightItem->repaint();
        d->oldHighlightItem = 0;
    }
}

bool TreeFolderView::acceptDrop(const QDropEvent *) const
{
    return false;
}

bool TreeFolderView::mouseInItemRect(Q3ListViewItem* item, int x) const
{
    if (!item)
        return false;

    x += contentsX();

    int offset = treeStepSize()*(item->depth() + (rootIsDecorated() ? 1 : 0));
    offset    += itemMargin();
    int width  = item->width(fontMetrics(), this, 0);

    return (x > offset && x < (offset + width));
}

void TreeFolderView::slotThemeChanged()
{
    int w = frameRect().width();
    int h = itemHeight();

    d->itemRegPix = ThemeEngine::instance()->listRegPixmap(w, h);
    d->itemSelPix = ThemeEngine::instance()->listSelPixmap(w, h);

    QPalette plt(palette());
    plt.setColor(QPalette::Active, QPalette::Base, 
                 ThemeEngine::instance()->baseColor());
    plt.setColor(QPalette::Active, QPalette::Text, 
                 ThemeEngine::instance()->textRegColor());
    plt.setColor(QPalette::Active, QPalette::HighlightedText, 
                 ThemeEngine::instance()->textSelColor());
    plt.setColor(QPalette::Active, QPalette::Link, 
                 ThemeEngine::instance()->textSpecialRegColor());
    plt.setColor(QPalette::Active, QPalette::LinkVisited,
                 ThemeEngine::instance()->textSpecialSelColor());
    plt.setColor(QPalette::Inactive, QPalette::Base, 
                 ThemeEngine::instance()->baseColor());
    plt.setColor(QPalette::Inactive, QPalette::Text, 
                 ThemeEngine::instance()->textRegColor());
    plt.setColor(QPalette::Inactive, QPalette::HighlightedText, 
                 ThemeEngine::instance()->textSelColor());
    plt.setColor(QPalette::Inactive, QPalette::Link, 
                 ThemeEngine::instance()->textSpecialRegColor());
    plt.setColor(QPalette::Inactive, QPalette::LinkVisited,
                 ThemeEngine::instance()->textSpecialSelColor());
    setPalette(plt);

    viewport()->update();
}

void TreeFolderView::slotAllAlbumsLoaded()
{
    disconnect(AlbumManager::instance(), SIGNAL(signalAllAlbumsLoaded()),
               this, SLOT(slotAllAlbumsLoaded()));    
    loadViewState();
}

void TreeFolderView::loadViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(objectName());

    int selectedItem = group.readEntry("LastSelectedItem", 0);

    Q3ValueList<int> openFolders;
    if(group.hasKey("OpenFolders"))
    {
        openFolders = group.readEntry("OpenFolders",QList<int>());
    }

    TreeFolderItem *item      = 0;
    TreeFolderItem *foundItem = 0;
    Q3ListViewItemIterator it(this->lastItem());

    for( ; it.current(); --it)
    {
        item = dynamic_cast<TreeFolderItem*>(it.current());
        if(!item)
            continue;
        // Start the album root always open
        if(openFolders.contains(item->id()) || item->id() == 0)
            setOpen(item, true);
        else
            setOpen(item, false);

        if(item->id() == selectedItem)
        {
            // Save the found selected item so that it can be made visible.
            foundItem = item;
        }
    }

    // Important note: this cannot be done inside the previous loop
    // because opening folders prevents the visibility.
    // Fixes bug #144815.
    // (Looks a bit like a bug in Qt to me ...)
    if (foundItem)
    {
        setSelected(foundItem, true);
        ensureItemVisible(foundItem);
    }
}

void TreeFolderView::saveViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(objectName());

    TreeFolderItem *item = dynamic_cast<TreeFolderItem*>(selectedItem());
    if(item)
        group.writeEntry("LastSelectedItem", item->id());
    else
        group.writeEntry("LastSelectedItem", 0);

    QList<int> openFolders;
    Q3ListViewItemIterator it(this);
    for( ; it.current(); ++it)
    {
        item = dynamic_cast<TreeFolderItem*>(it.current());
        if(item && isOpen(item))
            openFolders.push_back(item->id());
    }
    group.writeEntry("OpenFolders", openFolders);
}

void TreeFolderView::slotSelectionChanged()
{
    Q3ListView::selectionChanged();
}

void TreeFolderView::selectItem(int)
{
}

}  // namespace Digikam
