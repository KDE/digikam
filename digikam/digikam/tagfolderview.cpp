/* ============================================================
 * File  : tagfolderview.cpp
 * Author: Joern Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-05-05
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

#include <qintdict.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qlistview.h>
//#include 

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmessagebox.h>

#include "tagfolderview.h"
#include "album.h"
#include "albummanager.h"
#include "syncjob.h"
#include "tagcreatedlg.h"
#include "dragobjects.h"
#include "folderitem.h"

static QPixmap getBlendedIcon(TAlbum* tag)
{
    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();

    QPixmap baseIcon(iconLoader->loadIcon("tag",
                     KIcon::NoGroup,
                     32,
                     KIcon::DefaultState,
                     0, true));

    if(!tag)
        return baseIcon;

    QPixmap pix = SyncJob::getTagThumbnail(tag->icon(), 20);

    if(!pix.isNull())
    {
        QPainter p(&baseIcon);
        p.drawPixmap(6, 9, pix, 0, 0, -1, -1);
        p.end();
    }

    return baseIcon;
}

//-----------------------------------------------------------------------------
// TagFolderViewItem
//-----------------------------------------------------------------------------

class TagFolderViewItem : public FolderItem
{
public:
    TagFolderViewItem(QListView *parent, TAlbum *tag);
    TagFolderViewItem(QListViewItem *parent, TAlbum *tag);    

    TAlbum* getTag() const;

private:
    TAlbum      *m_tag;
};

TagFolderViewItem::TagFolderViewItem(QListView *parent, TAlbum *tag)
    : FolderItem(parent, tag->title())
{
    setDragEnabled(true);
    m_tag = tag;
}

TagFolderViewItem::TagFolderViewItem(QListViewItem *parent, TAlbum *tag)
    : FolderItem(parent, tag->title())
{
    setDragEnabled(true);
    m_tag = tag;
}

TAlbum* TagFolderViewItem::getTag() const
{
    return m_tag;
}

//-----------------------------------------------------------------------------
// TagFolderViewPriv
//-----------------------------------------------------------------------------

class TagFolderViewPriv
{
public:
    AlbumManager                    *albumMan;    
    QIntDict<TagFolderViewItem>     dict;
};

//-----------------------------------------------------------------------------
// TagFolderView
//-----------------------------------------------------------------------------

TagFolderView::TagFolderView(QWidget *parent)
    : FolderView(parent)
{
    d = new TagFolderViewPriv();
    d->albumMan = AlbumManager::instance();
        
    addColumn(i18n("My Tags"));
    setResizeMode(QListView::LastColumn);
    setRootIsDecorated(true);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    
    connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
            SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));
    
    connect(d->albumMan, SIGNAL(signalAlbumAdded(Album*)),
            SLOT(slotAlbumAdded(Album*)));
    connect(d->albumMan, SIGNAL(signalAlbumDeleted(Album*)),
            SLOT(slotAlbumDeleted(Album*)));
    connect(d->albumMan, SIGNAL(signalAlbumsCleared()),
            SLOT(slotAlbumsCleared()));
    connect(d->albumMan, SIGNAL(signalAlbumIconChanged(Album*)),
            this, SLOT(slotAlbumIconChanged(Album*)));
    
    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));    
}

TagFolderView::~TagFolderView()
{
    delete d;    
}

void TagFolderView::slotAlbumAdded(Album *album)
{
    if(!album || album->isRoot())
        return;
    
    TAlbum *tag = dynamic_cast<TAlbum*>(album);
    if(!tag)
        return;
    
    if(tag->parent()->isRoot())
    {
        TagFolderViewItem *item = new TagFolderViewItem(this, tag);
        item->setPixmap(0, getBlendedIcon(tag));
        d->dict.insert(tag->id(), item);
    }
    else
    {
        TagFolderViewItem *parent = d->dict.find(tag->parent()->id());
        if (!parent)
        {
            kdWarning() << k_funcinfo << " Failed to find parent for Tag "
                        << tag->title() << endl;
            return;
        }
        TagFolderViewItem *item = new TagFolderViewItem(parent, tag);
        item->setPixmap(0, getBlendedIcon(tag));
        d->dict.insert(tag->id(), item);        
    }
}

void TagFolderView::slotAlbumDeleted(Album *album)
{
    if(!album)
        return;

    TAlbum *tag = dynamic_cast<TAlbum*>(album);
    if(!tag)
        return;

    TagFolderViewItem *item = d->dict.find(tag->id());
    if(item) {
        d->dict.remove(tag->id());
        delete item;
    }
}

void TagFolderView::slotAlbumsCleared()
{
    d->dict.clear();
    clear();
}

void TagFolderView::slotSelectionChanged()
{
    if (!active())
        return;

    QListViewItem* selItem = 0;
    QListViewItemIterator it(this);
    while (it.current())
    {
        if (it.current()->isSelected())
        {
            selItem = it.current();
            break;
        }
        ++it;
    }

    if (!selItem)
    {
        d->albumMan->setCurrentAlbum(0);
        return;
    }
    
    TagFolderViewItem *tagitem = dynamic_cast<TagFolderViewItem*>(selItem);
    if(!tagitem)
    {
        d->albumMan->setCurrentAlbum(0);
        return;        
    }

    d->albumMan->setCurrentAlbum(tagitem->getTag());
}

void TagFolderView::slotAlbumIconChanged(Album* album)
{
    if(!album || album->type() != Album::TAG)
        return;

    TagFolderViewItem *item = d->dict.find(album->id());

    if(item)
        item->setPixmap(0, getBlendedIcon((TAlbum*)album));
}

void TagFolderView::slotContextMenu(QListViewItem *item, const QPoint &, int)
{
    QPopupMenu popmenu(this);
    
    TagFolderViewItem *tag = dynamic_cast<TagFolderViewItem*>(item);
    popmenu.insertItem(SmallIcon("tag"), i18n("New Tag..."), 10);

    if(tag)
    {
        popmenu.insertItem(SmallIcon("pencil"), i18n("Edit Tag Properties..."), 11);
        popmenu.insertItem(SmallIcon("edittrash"), i18n("Delete Tag"), 12);
    }

    switch(popmenu.exec((QCursor::pos())))
    {
        case 10:
        {
            tagNew(tag);
            break;
        }
        case 11:
        {
            tagEdit(tag);
            break;
        }
        case 12:
        {
            tagDelete(tag);
            break;
        }
        default:
            break;
    }
}

void TagFolderView::tagNew()
{
    TagFolderViewItem *item = dynamic_cast<TagFolderViewItem*>(selectedItem());
    tagNew(item);
}

void TagFolderView::tagNew(TagFolderViewItem *item)
{
    QString title, icon;
    TAlbum *parent;

    if(!item)
        parent = d->albumMan->findTAlbum(0);
    else
        parent = item->getTag();

    if(!TagCreateDlg::tagCreate(parent, title, icon))
        return;

    QString errMsg;
    if(!d->albumMan->createTAlbum(parent, title, icon, errMsg))
        KMessageBox::error(0, errMsg);
}

void TagFolderView::tagEdit()
{
    TagFolderViewItem *item = dynamic_cast<TagFolderViewItem*>(selectedItem());
    tagEdit(item);    
}

void TagFolderView::tagEdit(TagFolderViewItem *item)
{
    if(!item)
        return;
    
    TAlbum *tag = item->getTag();
    if(!tag)
        return;

    QString title, icon;
    if(!TagEditDlg::tagEdit(tag, title, icon))
    {
        return;
    }

    if(tag->title() != title)
    {
        QString errMsg;
        if(!d->albumMan->renameTAlbum(tag, title, errMsg))
            KMessageBox::error(0, errMsg);
        else
            item->setText(0, title);
    }

    if(tag->icon() != icon)
    {
        QString errMsg;
        if (!d->albumMan->updateTAlbumIcon(tag, icon, false, errMsg))
            KMessageBox::error(0, errMsg);
        else
            item->setPixmap(0, getBlendedIcon(tag));
    }

//    emit signalTagsAssigned();
}

void TagFolderView::tagDelete()
{
    TagFolderViewItem *item = dynamic_cast<TagFolderViewItem*>(selectedItem());
    tagDelete(item);
}

void TagFolderView::tagDelete(TagFolderViewItem *item)
{
    if(!item)
        return;

    TAlbum *tag = item->getTag();
    if (!tag || tag->isRoot())
        return;

    // find number of subtags
    int children = 0;
    AlbumIterator iter(tag);
    while(iter.current()) {
        children++;
        ++iter;
    }

    if(children)
    {
        int result =
            KMessageBox::warningYesNo(this, i18n("Tag '%1' has %2 subtag(s). "
                                                 "Deleting this will also delete "
                                                 "the subtag(s). "
                                                 "Are you sure you want to continue?")
                                      .arg(tag->title())
                                      .arg(children));

        if(result == KMessageBox::Yes)
        {
            QString errMsg;
            if (!d->albumMan->deleteTAlbum(tag, errMsg))
                KMessageBox::error(0, errMsg);
        }
    }
    else
    {
        int result =
            KMessageBox::questionYesNo(0, i18n("Delete '%1' tag?")
                                       .arg(tag->title()));

        if(result == KMessageBox::Yes)
        {
            QString errMsg;
            if (!d->albumMan->deleteTAlbum(tag, errMsg))
                KMessageBox::error(0, errMsg);
        }
    }
}

QDragObject* TagFolderView::dragObject()
{
    TagFolderViewItem *item = dynamic_cast<TagFolderViewItem*>(dragItem());
    if(!item)
        return 0;
    
    TagDrag *t = new TagDrag(item->getTag()->id(), this);
    t->setPixmap(*item->pixmap(0));

    return t;
}

bool TagFolderView::acceptDrop(const QDropEvent *e) const
{
    QPoint vp = contentsToViewport(e->pos());
    TagFolderViewItem *itemDrop = dynamic_cast<TagFolderViewItem*>(itemAt(vp));
    TagFolderViewItem *itemDrag = dynamic_cast<TagFolderViewItem*>(dragItem());
 
    if(TagDrag::canDecode(e) || TagListDrag::canDecode(e))
    {
        // Allow dragging at the root, to move the tag at the root
        if(!itemDrop)
            return true;
        
        // Dragging an item on itself makes no sense
        if(itemDrag == itemDrop)
            return false;

        // Dragging a parent on its child makes no sense
        if(itemDrag && itemDrag->getTag()->isAncestorOf(itemDrop->getTag()))
            return false;
        
        return true;
    }

    return false;
}

void TagFolderView::contentsDropEvent(QDropEvent *e)
{
    FolderView::contentsDropEvent(e);

    if(!acceptDrop(e))
        return;

    QPoint vp = contentsToViewport(e->pos());
    TagFolderViewItem *itemDrop = dynamic_cast<TagFolderViewItem*>(itemAt(vp));
    TagFolderViewItem *itemDrag = dynamic_cast<TagFolderViewItem*>(dragItem());
    if(!itemDrag)
        return;

    if(TagDrag::canDecode(e))
    {
        QPopupMenu popMenu(this);
        popMenu.insertItem(SmallIcon("goto"), i18n("&Move Here"), 10);
        popMenu.insertSeparator(-1);
        popMenu.insertItem(SmallIcon("cancel"), i18n("C&ancel"), 20);
        popMenu.setMouseTracking(true);
        int id = popMenu.exec(QCursor::pos());

        if(id == 10)
        {
            TagFolderViewItem *itemDragParent = 
                    dynamic_cast<TagFolderViewItem*>(dragItem()->parent());
            
            if(itemDragParent)
                itemDragParent->takeItem(itemDrag);
            else
                takeItem(itemDrag);
            
            QString errMsg;
            if(!itemDrop)
            {
                // move dragItem to the root
                TAlbum *rootAlbum = d->albumMan->findTAlbum(0);
                TAlbum *tag = itemDrag->getTag();

                d->albumMan->moveTAlbum(tag, rootAlbum, errMsg);
                TagFolderViewItem *item = new TagFolderViewItem(this, tag);
                item->setPixmap(0, getBlendedIcon(tag));
                
                while(QListViewItem *child = itemDrag->firstChild())
                {
                    itemDrag->takeItem(child);
                    item->insertItem(child);
                }
                if(itemDrag->isOpen())
                    item->setOpen(true);
                delete itemDrag;                
                setSelected(item, true);
            }
            else
            {
                // move dragItem below dropItem
                d->albumMan->moveTAlbum(itemDrag->getTag(), itemDrop->getTag(), errMsg);
                itemDrop->insertItem(itemDrag);
                if(!itemDrop->isOpen())
                    itemDrop->setOpen(true);
                setSelected(itemDrag, true);                
            }
        }
    }
}

#include "tagfolderview.moc"

