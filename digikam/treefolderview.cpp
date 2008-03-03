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

#include <QHeaderView>
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
        dragStarted      = false;
        active           = false;
        dragItem         = 0;
        oldHighlightItem = 0;
    }

    bool             active;
    bool             dragStarted;

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
    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);
    header()->setSortIndicatorShown(false);
    setRootIsDecorated(true);
    setUniformRowHeights(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setDragEnabled(true);
    setDropIndicatorShown(false);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setItemsExpandable(true);
    setAutoExpandDelay(500);

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(AlbumManager::instance(), SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(slotAllAlbumsLoaded()));

    connect(AlbumThumbnailLoader::instance(), SIGNAL(signalReloadThumbnails()),
            this, SLOT(slotIconSizeChanged()));

    slotIconSizeChanged();
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

void TreeFolderView::slotIconSizeChanged()
{
    int size = AlbumThumbnailLoader::instance()->thumbnailSize();
    setIconSize(QSize(size, size));
    slotThemeChanged();
}

void TreeFolderView::mouseMoveEvent(QMouseEvent *e)
{
    QPoint vp = viewport()->mapFrom(this, e->pos());
    if (!header()->isHidden())
        vp.setY(vp.y()+header()->height());

    //QTreeWidget::mouseMoveEvent(e);

    if(e->buttons() == Qt::NoButton)
    {
        if(KGlobalSettings::changeCursorOverIcon())
        {
            QTreeWidgetItem *item = itemAt(vp);
            if (item)
                setCursor(Qt::PointingHandCursor);
            else
                unsetCursor();
        }
        return;
    }

    if(d->dragItem && (d->dragStartPos - e->pos()).manhattanLength() > QApplication::startDragDistance())
    {
        TreeFolderItem *item = dynamic_cast<TreeFolderItem*>(itemAt(vp));
        if(!item)
        {
            d->dragItem    = 0;
            d->dragStarted = false;
            return;
        }

        if (!d->dragStarted)
        {
            makeDragObject();
            d->dragStarted = true;
        }
    }

    e->accept();
}

void TreeFolderView::mousePressEvent(QMouseEvent *e)
{
    QPoint vp = viewport()->mapFrom(this, e->pos());
    if (!header()->isHidden())
        vp.setY(vp.y()+header()->height());

    TreeFolderItem *item = dynamic_cast<TreeFolderItem*>(itemAt(vp));
    if (!item) return;

    // If item is a checkbox, restore the status with middle mouse button.
    if((item->flags() & Qt::ItemIsUserCheckable) && 
       e->button() == Qt::MidButton) 
    {
        Qt::CheckState state = item->checkState(0);
        QTreeWidget::mousePressEvent(e);
        item->setCheckState(0, state);
        return;
    }

    QTreeWidget::mousePressEvent(e);

    if(e->button() == Qt::LeftButton) 
    {
        d->dragStartPos = e->pos();
        d->dragItem     = item;
    }

    e->accept();
}

void TreeFolderView::mouseReleaseEvent(QMouseEvent *e)
{
    QTreeWidget::mouseReleaseEvent(e);
    d->dragItem    = 0;
    d->dragStarted = false;
    e->accept();
}

TreeFolderItem* TreeFolderView::dragItem() const
{
    return d->dragItem;
}

QStringList TreeFolderView::mimeTypes() const
{
    QStringList list;
    list.append("text/uri-list");
    list.append("digikam/item-ids");
    list.append("digikam/image-ids");
    list.append("digikam/album-ids");
    list.append("digikam/album-id");
    list.append("digikam/tag-id");
    list.append("digikam/taglist");
    list.append("digikam/digikamalbums");
    list.append("digikam/cameraItemlist");
    return list;
}

void TreeFolderView::dragEnterEvent(QDragEnterEvent *e)
{
    QTreeWidget::dragEnterEvent(e);
    e->acceptProposedAction();
}

void TreeFolderView::dragLeaveEvent(QDragLeaveEvent *e)
{
    QTreeWidget::dragLeaveEvent(e);

    if(d->oldHighlightItem)
    {
        d->oldHighlightItem->setFocus(false);
        d->oldHighlightItem = 0;
    }
}

void TreeFolderView::dragMoveEvent(QDragMoveEvent *e)
{
    QTreeWidget::dragMoveEvent(e);
    if (acceptDrop(e)) 
        e->acceptProposedAction();

    QPoint vp = viewport()->mapFrom(this, e->pos());
    if (!header()->isHidden())
        vp.setY(vp.y()+header()->height());

    TreeFolderItem *item = dynamic_cast<TreeFolderItem*>(itemAt(vp));
    if(item)
    {
        if(d->oldHighlightItem)
            d->oldHighlightItem->setFocus(false);

        item->setFocus(true);
        d->oldHighlightItem = item;
    }
}

void TreeFolderView::dropEvent(QDropEvent*)
{
    if(d->oldHighlightItem)
    {
        d->oldHighlightItem->setFocus(false);
        d->oldHighlightItem = 0;
    }
}

bool TreeFolderView::acceptDrop(const QDropEvent*) const
{
    return false;
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

    TreeAlbumItem *item      = 0;
    TreeAlbumItem *foundItem = 0;
    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        item = dynamic_cast<TreeAlbumItem*>(*it);
        if(item)
        {
            int id = item->id();
            if (id != 0)
            {
                if (openFolders.contains(id))
                {
                    DDebug() << objectName() << ": expanded item ID: " << id << endl;
                    item->setExpanded(true);
                }
                else
                    item->setExpanded(false);
            }

            if(item->id() == selectedItem)
            {
                // Save the found selected item so that it can be made visible.
                foundItem = item;
            }
        }
        ++it;
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
    while (*it)
    {
        item = dynamic_cast<TreeFolderItem*>(*it);
        if(item && item->isExpanded())
            openFolders.push_back(item->id());
        ++it;
    }
    group.writeEntry("OpenFolders", openFolders);
}

void TreeFolderView::slotThemeChanged()
{
/* TODO : Port to Qt4::StyleSheet !

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

}  // namespace Digikam
