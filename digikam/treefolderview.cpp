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

    QPoint           dragStartPos;

    TreeFolderItem  *dragItem;
    TreeFolderItem  *oldHighlightItem;
};

//-----------------------------------------------------------------------------

TreeFolderView::TreeFolderView(QWidget *parent, const char *name)
              : QTreeWidget(parent)
{
    d = new TreeFolderViewPriv;
    setObjectName(name);
    setColumnCount(1);
    setRootIsDecorated(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setDragEnabled(true);
    setDropIndicatorShown(true);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(AlbumManager::instance(), SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(slotAllAlbumsLoaded()));

    connect(AlbumThumbnailLoader::instance(), SIGNAL(signalReloadThumbnails()),
            this, SLOT(slotIconSizeChanged()));

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

void TreeFolderView::fontChange(const QFont& oldFont)
{
    QTreeWidget::fontChange(oldFont);
    slotThemeChanged();
}

void TreeFolderView::slotIconSizeChanged()
{
    slotThemeChanged();
}

void TreeFolderView::mouseMoveEvent(QMouseEvent *e)
{
    QTreeWidget::mouseMoveEvent(e);

    if(e->buttons() == Qt::NoButton)
    {
        if(KGlobalSettings::changeCursorOverIcon())
        {
            QPoint vp = viewport()->mapFrom(this, e->pos());
            QTreeWidgetItem *item = itemAt(vp);
            if (item)
                setCursor(Qt::PointingHandCursor);
            else
                unsetCursor();
        }
        return;
    }

    if(d->dragItem && 
       (d->dragStartPos - e->pos()).manhattanLength() > QApplication::startDragDistance())
    {
        QPoint vp = viewport()->mapFrom(this, e->pos());
        TreeFolderItem *item = dynamic_cast<TreeFolderItem*>(itemAt(vp));
        if(!item)
        {
            d->dragItem = 0;
            return;
        }
    }
}

void TreeFolderView::mousePressEvent(QMouseEvent *e)
{
    QTreeWidget::mousePressEvent(e);

    QPoint vp = viewport()->mapFrom(this, e->pos());
    TreeFolderItem *item = dynamic_cast<TreeFolderItem*>(itemAt(vp));

    if(item && e->button() == Qt::LeftButton) 
    {
        d->dragStartPos = e->pos();
        d->dragItem     = item;
        return;
    }
}

void TreeFolderView::mouseReleaseEvent(QMouseEvent *e)
{
    QTreeWidget::mouseReleaseEvent(e);

    d->dragItem = 0;
}

TreeFolderItem* TreeFolderView::dragItem() const
{
    return d->dragItem;
}

void TreeFolderView::dragEnterEvent(QDragEnterEvent *e)
{
    QTreeWidget::dragEnterEvent(e);

    e->setAccepted(acceptDrop(e));
}

void TreeFolderView::dragLeaveEvent(QDragLeaveEvent * e)
{
    QTreeWidget::dragLeaveEvent(e);

    if(d->oldHighlightItem)
    {
        d->oldHighlightItem->setFocus(false);
//        d->oldHighlightItem->repaint();
        d->oldHighlightItem = 0;
    }
}

void TreeFolderView::dragMoveEvent(QDragMoveEvent *e)
{
    QTreeWidget::dragMoveEvent(e);

    QPoint vp = viewport()->mapFrom(this, e->pos());
    TreeFolderItem *item = dynamic_cast<TreeFolderItem*>(itemAt(vp));
    if(item)
    {
        if(d->oldHighlightItem)
        {
            d->oldHighlightItem->setFocus(false);
//            d->oldHighlightItem->repaint();
        }
        item->setFocus(true);
        d->oldHighlightItem = item;
//        item->repaint();
    }
    e->setAccepted(acceptDrop(e));
}

void TreeFolderView::dropEvent(QDropEvent *e)
{
    QTreeWidget::dropEvent(e);

    if(d->oldHighlightItem)
    {
        d->oldHighlightItem->setFocus(false);
//        d->oldHighlightItem->repaint();
        d->oldHighlightItem = 0;
    }
}

bool TreeFolderView::acceptDrop(const QDropEvent *) const
{
    return false;
}

/*bool TreeFolderView::mouseInItemRect(QTreeWidgetItem* item, int x) const
{
    if (!item)
        return false;

    x += contentsX();

    int offset = treeStepSize()*(item->depth() + (rootIsDecorated() ? 1 : 0));
    offset    += itemMargin();
    int width  = item->width(fontMetrics(), this, 0);

    return (x > offset && x < (offset + width));
}*/

void TreeFolderView::slotThemeChanged()
{
/* TODO
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
*/
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

    QList<int> openFolders;
    if(group.hasKey("OpenFolders"))
        openFolders = group.readEntry("OpenFolders", QList<int>());

    TreeFolderItem *item      = 0;
    TreeFolderItem *foundItem = 0;
    QTreeWidgetItemIterator it(this);
    for( ; *it; ++it)
    {
        item = dynamic_cast<TreeFolderItem*>(*it);
        if(!item)
            continue;
        // Start the album root always open
        if(openFolders.contains(item->id()) || item->id() == 0)
            expandItem(item);
        else
            collapseItem(item);

        if(item->id() == selectedItem)
        {
            // Save the found selected item so that it can be made visible.
            foundItem = item;
        }
    }

    if (foundItem)
    {
        setCurrentItem(foundItem);
        scrollToItem(foundItem);
    }
}

void TreeFolderView::saveViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(objectName());

    TreeFolderItem *item = dynamic_cast<TreeFolderItem*>(currentItem());
    if(item)
        group.writeEntry("LastSelectedItem", item->id());
    else
        group.writeEntry("LastSelectedItem", 0);

    QList<int> openFolders;
    QTreeWidgetItemIterator it(this);
    for( ; *it; ++it)
    {
        item = dynamic_cast<TreeFolderItem*>(*it);
        if(item && item->isExpanded())
            openFolders.push_back(item->id());
    }
    group.writeEntry("OpenFolders", openFolders);
}

void TreeFolderView::slotSelectionChanged()
{
    QTreeWidget::itemSelectionChanged();
}

void TreeFolderView::selectItem(int)
{
}

}  // namespace Digikam
