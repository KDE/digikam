/* ============================================================
 * File  : tagfolderview.cpp
 * Author: Joern Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-05-05
 * Copyright 2005-2006 by Joern Ahrens
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

#include <qpainter.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qlistview.h>

// KDE includes.

#include <klocale.h>
#include <kdebug.h>
#include <kabc/stdaddressbook.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmessagebox.h>

// Local includes.

#include "tagfolderview.h"
#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "syncjob.h"
#include "tagcreatedlg.h"
#include "dragobjects.h"
#include "folderitem.h"
#include "dio.h"
#include "imageattributeswatch.h"
#include "albumthumbnailloader.h"

// X11 Includes.

extern "C"
{
#include <X11/Xlib.h>
}

namespace Digikam
{


//-----------------------------------------------------------------------------
// TagFolderViewItem
//-----------------------------------------------------------------------------

class TagFolderViewItem : public FolderItem
{
public:
    TagFolderViewItem(QListView *parent, TAlbum *tag);
    TagFolderViewItem(QListViewItem *parent, TAlbum *tag);

    TAlbum* getTag() const;
    int id() const;

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

int TagFolderViewItem::id() const
{
    return m_tag ? m_tag->id() : 0;
}

//-----------------------------------------------------------------------------
// TagFolderViewPriv
//-----------------------------------------------------------------------------

class TagFolderViewPriv
{
public:
    AlbumManager                    *albumMan;
    QPopupMenu                      *ABCMenu;
};

//-----------------------------------------------------------------------------
// TagFolderView
//-----------------------------------------------------------------------------

TagFolderView::TagFolderView(QWidget *parent)
    : FolderView(parent, "TagFolderView")
{
    d = new TagFolderViewPriv();
    d->albumMan = AlbumManager::instance();

    addColumn(i18n("My Tags"));
    setResizeMode(QListView::LastColumn);
    setRootIsDecorated(false);

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    connect(d->albumMan, SIGNAL(signalAlbumAdded(Album*)),
            SLOT(slotAlbumAdded(Album*)));
    connect(d->albumMan, SIGNAL(signalAlbumDeleted(Album*)),
            SLOT(slotAlbumDeleted(Album*)));
    connect(d->albumMan, SIGNAL(signalAlbumRenamed(Album*)),
            SLOT(slotAlbumRenamed(Album*)));
    connect(d->albumMan, SIGNAL(signalAlbumsCleared()),
            SLOT(slotAlbumsCleared()));
    connect(d->albumMan, SIGNAL(signalAlbumIconChanged(Album*)),
            SLOT(slotAlbumIconChanged(Album*)));
    connect(d->albumMan, SIGNAL(signalTAlbumMoved(TAlbum*, TAlbum*)),
            SLOT(slotAlbumMoved(TAlbum*, TAlbum*)));

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();
    connect(loader, SIGNAL(signalThumbnail(Album *, const QPixmap&)),
            SLOT(slotGotThumbnailFromIcon(Album *, const QPixmap&)));
    connect(loader, SIGNAL(signalFailed(Album *)),
            SLOT(slotThumbnailLost(Album *)));

    connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
            SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));

    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

TagFolderView::~TagFolderView()
{
    delete d;
}

void TagFolderView::slotAlbumAdded(Album *album)
{
    if(!album)
        return;

    TAlbum *tag = dynamic_cast<TAlbum*>(album);
    if(!tag)
        return;

    TagFolderViewItem *item;

    if(tag->isRoot())
    {
        item = new TagFolderViewItem(this, tag);
        tag->setExtraData(this, item);
    }
    else
    {
        TagFolderViewItem *parent =
                (TagFolderViewItem*)tag->parent()->extraData(this);

        if (!parent)
        {
            kdWarning() << k_funcinfo << " Failed to find parent for Tag "
                        << tag->title() << endl;
            return;
        }

        item = new TagFolderViewItem(parent, tag);
        tag->setExtraData(this, item);
    }

    setTagThumbnail(tag);
}

void TagFolderView::slotAlbumDeleted(Album *album)
{
    if(!album)
        return;

    TAlbum *tag = dynamic_cast<TAlbum*>(album);
    if(!tag)
        return;

    TagFolderViewItem *item = (TagFolderViewItem*)album->extraData(this);
    if(item)
    {
        TagFolderViewItem *itemParent =
                dynamic_cast<TagFolderViewItem*>(item->parent());

        if(itemParent)
            itemParent->takeItem(item);
        else
            takeItem(item);

        delete item;
    }
}

void TagFolderView::slotAlbumsCleared()
{
    clear();
}

void TagFolderView::slotAlbumMoved(TAlbum* tag, TAlbum* newParent)
{
    if (!tag || !newParent)
        return;

    TagFolderViewItem* item = (TagFolderViewItem*)tag->extraData(this);
    if (!item)
        return;

    if (item->parent())
    {
        QListViewItem* oldPItem = item->parent();
        oldPItem->takeItem(item);
    }
    else
    {
        takeItem(item);
    }

    TagFolderViewItem* newPItem = (TagFolderViewItem*)newParent->extraData(this);
    if (newPItem)
        newPItem->insertItem(item);
    else
        insertItem(item);
}

void TagFolderView::slotAlbumRenamed(Album* album)
{
    if (!album)
        return;

    TAlbum* tag = dynamic_cast<TAlbum*>(album);
    if (!tag)
        return;

    TagFolderViewItem* item = (TagFolderViewItem*)(tag->extraData(this));
    if (item)
    {
        item->setText(0, tag->title());
    }
}

void TagFolderView::setTagThumbnail(TAlbum *album)
{
    if(!album)
        return;

    TagFolderViewItem* item = (TagFolderViewItem*) album->extraData(this);

    if(!item)
        return;

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();
    QPixmap icon;
    if (!loader->getTagThumbnail(album, icon))
    {
        if (icon.isNull())
        {
            item->setPixmap(0, loader->getStandardTagIcon(album));
        }
        else
        {
            QPixmap blendedIcon = loader->blendIcons(loader->getStandardTagIcon(), icon);
            item->setPixmap(0, blendedIcon);
        }
    }
}

void TagFolderView::slotGotThumbnailFromIcon(Album *album,
                                             const QPixmap& thumbnail)
{
    if(!album || album->type() != Album::TAG)
        return;

    TagFolderViewItem* item = (TagFolderViewItem*)album->extraData(this);

    if(!item)
        return;

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();
    QPixmap blendedIcon = loader->blendIcons(loader->getStandardTagIcon(), thumbnail);
    item->setPixmap(0, blendedIcon);
}

void TagFolderView::slotThumbnailLost(Album *)
{
    // we already set the standard icon before loading
}

void TagFolderView::slotAlbumIconChanged(Album* album)
{
    if(!album || album->type() != Album::TAG)
        return;

    setTagThumbnail((TAlbum *)album);
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

void TagFolderView::slotContextMenu(QListViewItem *item, const QPoint &, int)
{
    QPopupMenu popmenu(this);
    d->ABCMenu = new QPopupMenu;
    connect( d->ABCMenu, SIGNAL( aboutToShow() ),
             SLOT( slotABCContextMenu() ) );

    TagFolderViewItem *tag = dynamic_cast<TagFolderViewItem*>(item);
    popmenu.insertItem(SmallIcon("tag"), i18n("New Tag..."), 10);
    popmenu.insertItem(SmallIcon("tag"), i18n("Create Tag From AddressBook"),
                       d->ABCMenu);

    if(tag && tag->parent())
    {
        popmenu.insertItem(SmallIcon("pencil"), i18n("Edit Tag Properties..."), 11);
        popmenu.insertItem(SmallIcon("edittrash"), i18n("Delete Tag"), 12);
    }

    int choice = popmenu.exec((QCursor::pos()));
    switch( choice )
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

    if ( choice > 100 )
    {
        tagNew( tag, d->ABCMenu->text( choice ), "tag-people" );
    }

    delete d->ABCMenu;
    d->ABCMenu = 0;
}

void TagFolderView::slotABCContextMenu()
{
    d->ABCMenu->clear();

    int counter = 100;
    KABC::AddressBook* ab = KABC::StdAddressBook::self();
    QStringList names;
    for ( KABC::AddressBook::Iterator it = ab->begin(); it != ab->end(); ++it )
    {
        names.push_back(it->formattedName());
    }

    qHeapSort(names);

    for ( QStringList::Iterator it = names.begin(); it != names.end(); ++it )
    {
        QString name = *it;
        if ( !name.isNull() )
            d->ABCMenu->insertItem( name, ++counter );
    }

    if (counter == 100)
    {
        d->ABCMenu->insertItem( i18n("No AddressBook Entries Found"), ++counter );
        d->ABCMenu->setItemEnabled( counter, false );
    }
}

void TagFolderView::tagNew()
{
    TagFolderViewItem *item = dynamic_cast<TagFolderViewItem*>(selectedItem());
    tagNew(item);
}

void TagFolderView::tagNew( TagFolderViewItem *item,
                            const QString& _title,
                            const QString& _icon )
{
    QString title = _title;
    QString icon  = _icon;
    TAlbum *parent;

    if(!item)
        parent = d->albumMan->findTAlbum(0);
    else
        parent = item->getTag();

    if (title.isNull())
    {
        if(!TagCreateDlg::tagCreate(parent, title, icon))
            return;
    }

    QString errMsg;
    TAlbum* newAlbum = d->albumMan->createTAlbum(parent, title, icon, errMsg);

    if( !newAlbum )
        KMessageBox::error(0, errMsg);
    else
    {
        TagFolderViewItem *item = (TagFolderViewItem*)newAlbum->extraData(this);
        if ( item )
            ensureItemVisible( item );
    }
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
        if (!d->albumMan->updateTAlbumIcon(tag, icon, 0, errMsg))
            KMessageBox::error(0, errMsg);
        else
            setTagThumbnail(tag);
    }
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
            KMessageBox::warningContinueCancel(this,
                    i18n("Tag '%1' has one subtag. "
                         "Deleting this will also delete "
                         "the subtag. "
                         "Are you sure you want to continue?",
                         "Tag '%1' has %n subtags. "
                         "Deleting this will also delete "
                         "the subtags. "
                         "Are you sure you want to continue?",
                         children).arg(tag->title()),
                      i18n("Delete Tag"),KGuiItem(i18n("Delete"),"editdelete"));

        if(result == KMessageBox::Continue)
        {
            QString errMsg;
            if (!d->albumMan->deleteTAlbum(tag, errMsg))
                KMessageBox::error(0, errMsg);
        }
    }
    else
    {
        int result =
            KMessageBox::warningContinueCancel(0, i18n("Delete '%1' tag?")
                                       .arg(tag->title()),i18n("Delete Tag"),KGuiItem(i18n("Delete"),"editdelete"));

        if(result == KMessageBox::Continue)
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

    if(!item->parent())
        return 0;

    TagDrag *t = new TagDrag(item->getTag()->globalID(), this);
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
        // Allow dragging at the root, to move the tag to the root
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

    if (ItemDrag::canDecode(e) && itemDrop && itemDrop->parent())
    {
        // Only other possibility is image items being dropped
        // And allow this only if there is a Tag to be dropped
        // on and also the Tag is not root.
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

    if(TagDrag::canDecode(e))
    {
        TagFolderViewItem *itemDrag = dynamic_cast<TagFolderViewItem*>(dragItem());
        if(!itemDrag)
            return;

        QPopupMenu popMenu(this);
        popMenu.insertItem(SmallIcon("goto"), i18n("&Move Here"), 10);
        popMenu.insertSeparator(-1);
        popMenu.insertItem(SmallIcon("cancel"), i18n("C&ancel"), 20);
        popMenu.setMouseTracking(true);
        int id = popMenu.exec(QCursor::pos());

        if(id == 10)
        {
            TAlbum *tag = itemDrag->getTag();
            TAlbum *newParentTag = 0;

            if (!itemDrop)
            {
                // move dragItem to the root
                newParentTag = d->albumMan->findTAlbum(0);
            }
            else
            {
                // move dragItem as child of dropItem
                newParentTag = itemDrop->getTag();
            }

            QString errMsg;
            if (!d->albumMan->moveTAlbum(tag, newParentTag, errMsg))
            {
                KMessageBox::error(this, errMsg);
            }

            if(itemDrop && !itemDrop->isOpen())
                itemDrop->setOpen(true);
        }

        return;
    }

    if (ItemDrag::canDecode(e))
    {
        TAlbum *destAlbum = itemDrop->getTag();
        TAlbum *srcAlbum;

        KURL::List      urls;
        KURL::List      kioURLs;        
        QValueList<int> albumIDs;
        QValueList<int> imageIDs;

        if (!ItemDrag::decode(e, urls, kioURLs, albumIDs, imageIDs))
            return;

        if (urls.isEmpty() || kioURLs.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
            return;

        // all the albumids will be the same
        int albumID = albumIDs.first();
        srcAlbum = d->albumMan->findTAlbum(albumID);
        if (!srcAlbum)
        {
            kdWarning() << "Could not find source album of drag"
                    << endl;
            return;
        }

        int id = 0;
        char keys_return[32];
        XQueryKeymap(x11Display(), keys_return);
        int key_1 = XKeysymToKeycode(x11Display(), 0xFFE3);
        int key_2 = XKeysymToKeycode(x11Display(), 0xFFE4);

        if(srcAlbum == destAlbum)
        {
            // Setting the dropped image as the album thumbnail
            // If the ctrl key is pressed, when dropping the image, the
            // thumbnail is set without a popup menu
            if (((keys_return[key_1 / 8]) && (1 << (key_1 % 8))) ||
                ((keys_return[key_2 / 8]) && (1 << (key_2 % 8))))
            {
                id = 12;
            }
            else
            {
                QPopupMenu popMenu(this);
                popMenu.insertItem(i18n("Set as Tag Thumbnail"), 12);
                popMenu.insertSeparator(-1);
                popMenu.insertItem( SmallIcon("cancel"), i18n("C&ancel") );

                popMenu.setMouseTracking(true);
                id = popMenu.exec(QCursor::pos());
            }

            if(id == 12)
            {
                QString errMsg;
                AlbumManager::instance()->updateTAlbumIcon(destAlbum, QString(),
                                                           imageIDs.first(), errMsg);
            }
            return;
        }

        // If a ctrl key is pressed while dropping the drag object,
        // the tag is assigned to the images without showing a
        // popup menu.
        if (((keys_return[key_1 / 8]) && (1 << (key_1 % 8))) ||
            ((keys_return[key_2 / 8]) && (1 << (key_2 % 8))))
        {
            id = 10;
        }
        else
        {
            QPopupMenu popMenu(this);
            popMenu.insertItem( SmallIcon("tag"),
                                i18n("Assign Tag '%1' to Dropped Items")
                                .arg(destAlbum->prettyURL()), 10) ;
            popMenu.insertSeparator(-1);
            popMenu.insertItem( SmallIcon("cancel"), i18n("C&ancel") );

            popMenu.setMouseTracking(true);
            id = popMenu.exec(QCursor::pos());
        }

        if (id == 10)
        {
            AlbumDB* db = AlbumManager::instance()->albumDB();

            db->beginTransaction();
            for (QValueList<int>::const_iterator it = imageIDs.begin();
                 it != imageIDs.end(); ++it)
            {
                db->addItemTag(*it, destAlbum->id());
            }
            db->commitTransaction();

            ImageAttributesWatch::instance()->imagesChanged(destAlbum->id());
        }
    }
}

void TagFolderView::selectItem(int id)
{
    TAlbum* tag = d->albumMan->findTAlbum(id);
    if(!tag)
        return;

    TagFolderViewItem *item =
            (TagFolderViewItem*)tag->extraData(this);
    if(item)
    {
        setSelected(item, true);
        ensureItemVisible(item);
    }
}

}  // namespace Digikam

#include "tagfolderview.moc"

