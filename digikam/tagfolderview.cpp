/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-22
 * Descritpion : tags folder view.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern.ahrens@kdemail.net>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <qcursor.h>
#include <qlistview.h>

// KDE includes.

#include <kpopupmenu.h>
#include <klocale.h>
#include <kabc/stdaddressbook.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmessagebox.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "albumlister.h"
#include "syncjob.h"
#include "tageditdlg.h"
#include "dragobjects.h"
#include "folderitem.h"
#include "dio.h"
#include "imageattributeswatch.h"
#include "imageinfo.h"
#include "metadatahub.h"
#include "albumthumbnailloader.h"
#include "statusprogressbar.h"
#include "tagfolderview.h"
#include "tagfolderview.moc"

// X11 includes.

extern "C"
{
#include <X11/Xlib.h>
}

namespace Digikam
{

class TagFolderViewItem : public FolderItem
{

public:

    TagFolderViewItem(QListView *parent, TAlbum *album);
    TagFolderViewItem(QListViewItem *parent, TAlbum *album);

    TAlbum* album() const;
    int     id() const;
    void    refresh();
    void    setOpen(bool o);
    void    setCount(int count);
    int     count();

private:

    int     m_count;

    TAlbum *m_album;
};

TagFolderViewItem::TagFolderViewItem(QListView *parent, TAlbum *album)
                 : FolderItem(parent, album->title())
{
    setDragEnabled(true);
    m_album = album;
    m_count = 0;
}

TagFolderViewItem::TagFolderViewItem(QListViewItem *parent, TAlbum *album)
                 : FolderItem(parent, album->title())
{
    setDragEnabled(true);
    m_album = album;
    m_count = 0;
}

void TagFolderViewItem::refresh()
{
    if (!m_album) return;

    if (AlbumSettings::instance()->getShowFolderTreeViewItemsCount() &&
        dynamic_cast<TagFolderViewItem*>(parent()))
    {
        if (isOpen())
            setText(0, QString("%1 (%2)").arg(m_album->title()).arg(m_count));
        else
        {
            int countRecursive = m_count;
            AlbumIterator it(m_album);
            while ( it.current() )
            {
                TagFolderViewItem *item = (TagFolderViewItem*)it.current()->extraData(listView());
                if (item)
                    countRecursive += item->count();
                ++it;
            }
            setText(0, QString("%1 (%2)").arg(m_album->title()).arg(countRecursive));
        }
    }
    else
    {
        setText(0, m_album->title());
    }
}

void TagFolderViewItem::setOpen(bool o)
{
    QListViewItem::setOpen(o);
    refresh();
}

TAlbum* TagFolderViewItem::album() const
{
    return m_album;
}

int TagFolderViewItem::id() const
{
    return m_album ? m_album->id() : 0;
}

void TagFolderViewItem::setCount(int count)
{
    m_count = count;
    refresh();
}

int TagFolderViewItem::count()
{
    return m_count;
}

//-----------------------------------------------------------------------------

class TagFolderViewPriv
{

public:

    TagFolderViewPriv()
    {
        ABCMenu  = 0;
        albumMan = 0;
    }

    QPopupMenu   *ABCMenu;

    AlbumManager *albumMan;
};

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

    // ------------------------------------------------------------------------

    connect(d->albumMan, SIGNAL(signalTAlbumsDirty(const QMap<int, int>&)),
            this, SLOT(slotRefresh(const QMap<int, int>&)));

    connect(d->albumMan, SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(d->albumMan, SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(d->albumMan, SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    connect(d->albumMan, SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(d->albumMan, SIGNAL(signalAlbumIconChanged(Album*)),
            this, SLOT(slotAlbumIconChanged(Album*)));

    connect(d->albumMan, SIGNAL(signalTAlbumMoved(TAlbum*, TAlbum*)),
            this, SLOT(slotAlbumMoved(TAlbum*, TAlbum*)));

    // ------------------------------------------------------------------------

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();

    connect(loader, SIGNAL(signalThumbnail(Album *, const QPixmap&)),
            this, SLOT(slotGotThumbnailFromIcon(Album *, const QPixmap&)));

    connect(loader, SIGNAL(signalFailed(Album *)),
            this, SLOT(slotThumbnailLost(Album *)));

    connect(loader, SIGNAL(signalReloadThumbnails()),
            this, SLOT(slotReloadThumbnails()));

    connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
            SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));

    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

TagFolderView::~TagFolderView()
{
    delete d;
}

void TagFolderView::slotTextTagFilterChanged(const QString& filter)
{
    if (filter.isEmpty())
    {
        collapseView();
        return;
    }

    QString search = filter.lower();

    bool atleastOneMatch = false;

    AlbumList tList = d->albumMan->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbum* talbum  = (TAlbum*)(*it);

        // don't touch the root Album
        if (talbum->isRoot())
            continue;

        bool match = talbum->title().lower().contains(search);
        bool doesExpand = false;
        if (!match)
        {
            // check if any of the parents match the search
            Album* parent = talbum->parent();
            while (parent && !parent->isRoot())
            {
                if (parent->title().lower().contains(search))
                {
                    match = true;
                    break;
                }

                parent = parent->parent();
            }
        }

        if (!match)
        {
            // check if any of the children match the search
            AlbumIterator it(talbum);
            while (it.current())
            {
                if ((*it)->title().lower().contains(search))
                {
                    match = true;
                    doesExpand = true;
                    break;
                }
                ++it;
            }
        }

        TagFolderViewItem* viewItem = (TagFolderViewItem*) talbum->extraData(this);

        if (match)
        {
            atleastOneMatch = true;

            if (viewItem)
            {
                viewItem->setVisible(true);
                viewItem->setOpen(doesExpand);
        }
        }
        else
        {
            if (viewItem)
            {
                viewItem->setVisible(false);
                viewItem->setOpen(false);
            }
        }
    }

    emit signalTextTagFilterMatch(atleastOneMatch);
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
        // Toplevel tags are all children of root, and should always be visible - set root to open
        item->setOpen(true);
    }
    else
    {
        TagFolderViewItem *parent =
                (TagFolderViewItem*)tag->parent()->extraData(this);

        if (!parent)
        {
            DWarning() << k_funcinfo << " Failed to find parent for Tag "
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
        TagFolderViewItem *itemParent = dynamic_cast<TagFolderViewItem*>(item->parent());

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
        item->refresh();
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
    else
    {
        // for the time being, set standard icon
        item->setPixmap(0, loader->getStandardTagIcon(album));
    }
}

void TagFolderView::slotGotThumbnailFromIcon(Album *album, const QPixmap& thumbnail)
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

void TagFolderView::slotReloadThumbnails()
{
    AlbumList tList = d->albumMan->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbum* tag  = (TAlbum*)(*it);
        setTagThumbnail(tag);
    }
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

    d->albumMan->setCurrentAlbum(tagitem->album());
}

void TagFolderView::slotContextMenu(QListViewItem *item, const QPoint &, int)
{
    d->ABCMenu = new QPopupMenu;

    connect( d->ABCMenu, SIGNAL( aboutToShow() ),
             this, SLOT( slotABCContextMenu() ) );

    TagFolderViewItem *tag = dynamic_cast<TagFolderViewItem*>(item);

    KPopupMenu popmenu(this);
    popmenu.insertTitle(SmallIcon("digikam"), i18n("My Tags"));
    popmenu.insertItem(SmallIcon("tag-new"), i18n("New Tag..."), 10);
    popmenu.insertItem(SmallIcon("tag-addressbook"), i18n("Create Tag From AddressBook"), d->ABCMenu);

    if(tag && tag->parent())
    {
        popmenu.insertItem(SmallIcon("tag-properties"), i18n("Edit Tag Properties..."), 11);
        popmenu.insertItem(SmallIcon("tag-reset"),      i18n("Reset Tag Icon"),         13);
        popmenu.insertSeparator(-1);
        popmenu.insertItem(SmallIcon("tag-delete"),     i18n("Delete Tag"),             12);
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
        case 13:
        {
            QString errMsg;
            d->albumMan->updateTAlbumIcon(tag->album(), QString("tag"), 0, errMsg);
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
        d->ABCMenu->insertItem( i18n("No AddressBook entries found"), ++counter );
        d->ABCMenu->setItemEnabled( counter, false );
    }
}

void TagFolderView::tagNew()
{
    TagFolderViewItem *item = dynamic_cast<TagFolderViewItem*>(selectedItem());
    tagNew(item);
}

void TagFolderView::tagNew( TagFolderViewItem *item, const QString& _title, const QString& _icon )
{
    QString title = _title;
    QString icon  = _icon;
    TAlbum *parent;

    if(!item)
        parent = d->albumMan->findTAlbum(0);
    else
        parent = item->album();

    if (title.isNull())
    {
        if(!TagEditDlg::tagCreate(kapp->activeWindow(), parent, title, icon))
            return;
    }

    QMap<QString, QString> errMap;
    AlbumList tList = TagEditDlg::createTAlbum(parent, title, icon, errMap);
    TagEditDlg::showtagsListCreationError(kapp->activeWindow(), errMap);

    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TagFolderViewItem* item = (TagFolderViewItem*)(*it)->extraData(this);
        if (item)
            ensureItemVisible(item);
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

    TAlbum *tag = item->album();
    if(!tag)
        return;

    QString title, icon;
    if(!TagEditDlg::tagEdit(kapp->activeWindow(), tag, title, icon))
        return;

    if(tag->title() != title)
    {
        QString errMsg;
        if(!d->albumMan->renameTAlbum(tag, title, errMsg))
            KMessageBox::error(0, errMsg);
        else
            item->refresh();
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

    TAlbum *tag = item->album();
    if (!tag || tag->isRoot())
        return;

    // find number of subtags
    int children = 0;
    AlbumIterator iter(tag);
    while(iter.current())
    {
        children++;
        ++iter;
    }

    if(children)
    {
        int result = KMessageBox::warningContinueCancel(this,
                       i18n("Tag '%1' has one subtag. "
                            "Deleting this will also delete "
                            "the subtag. "
                            "Do you want to continue?",
                            "Tag '%1' has %n subtags. "
                            "Deleting this will also delete "
                            "the subtags. "
                            "Do you want to continue?",
                            children).arg(tag->title()));

        if(result != KMessageBox::Continue)
            return;
    }

    QString message;
    LLongList assignedItems = d->albumMan->albumDB()->getItemIDsInTag(tag->id());
    if (!assignedItems.isEmpty())
    {
        message = i18n("Tag '%1' is assigned to one item. "
                        "Do you want to continue?",
                        "Tag '%1' is assigned to %n items. "
                        "Do you want to continue?",
                        assignedItems.count()).arg(tag->title());
    }
    else
    {
        message = i18n("Delete '%1' tag?").arg(tag->title());
    }

    int result = KMessageBox::warningContinueCancel(0, message,
                                                    i18n("Delete Tag"),
                                                    KGuiItem(i18n("Delete"),
                                                    "editdelete"));

    if(result == KMessageBox::Continue)
    {
        QString errMsg;
        if (!d->albumMan->deleteTAlbum(tag, errMsg))
            KMessageBox::error(0, errMsg);
    }
}

QDragObject* TagFolderView::dragObject()
{
    TagFolderViewItem *item = dynamic_cast<TagFolderViewItem*>(dragItem());
    if(!item)
        return 0;

    if(!item->parent())
        return 0;

    TagDrag *t = new TagDrag(item->album()->id(), this);
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
        if(itemDrag && itemDrag->album()->isAncestorOf(itemDrop->album()))
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

    if (!itemDrop)
        return;

    if(TagDrag::canDecode(e))
    {
        QByteArray ba = e->encodedData("digikam/tag-id");
        QDataStream ds(ba, IO_ReadOnly);
        int tagID;
        ds >> tagID;

        TAlbum* talbum = d->albumMan->findTAlbum(tagID);

        if(!talbum)
            return;

        if (talbum == itemDrop->album())
            return;

        KPopupMenu popMenu(this);
        popMenu.insertTitle(SmallIcon("digikam"), i18n("My Tags"));
        popMenu.insertItem(SmallIcon("goto"), i18n("&Move Here"), 10);
        popMenu.insertSeparator(-1);
        popMenu.insertItem(SmallIcon("cancel"), i18n("C&ancel"), 20);
        popMenu.setMouseTracking(true);
        int id = popMenu.exec(QCursor::pos());

        if(id == 10)
        {
            TAlbum *newParentTag = 0;

            if (!itemDrop)
            {
                // move dragItem to the root
                newParentTag = d->albumMan->findTAlbum(0);
            }
            else
            {
                // move dragItem as child of dropItem
                newParentTag = itemDrop->album();
            }

            QString errMsg;
            if (!d->albumMan->moveTAlbum(talbum, newParentTag, errMsg))
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
        TAlbum *destAlbum = itemDrop->album();
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
            DWarning() << "Could not find source album of drag"
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
                KPopupMenu popMenu(this);
                popMenu.insertTitle(SmallIcon("digikam"), i18n("My Tags"));
                popMenu.insertItem(i18n("Set as Tag Thumbnail"), 12);
                popMenu.insertSeparator(-1);
                popMenu.insertItem( SmallIcon("cancel"), i18n("C&ancel") );

                popMenu.setMouseTracking(true);
                id = popMenu.exec(QCursor::pos());
            }

            if(id == 12)
            {
                QString errMsg;
                d->albumMan->updateTAlbumIcon(destAlbum, QString(), imageIDs.first(), errMsg);
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
            KPopupMenu popMenu(this);
            popMenu.insertTitle(SmallIcon("digikam"), i18n("My Tags"));
            popMenu.insertItem( SmallIcon("tag"), i18n("Assign Tag '%1' to Items")
                                .arg(destAlbum->prettyURL()), 10) ;
            popMenu.insertSeparator(-1);
            popMenu.insertItem( SmallIcon("cancel"), i18n("C&ancel") );

            popMenu.setMouseTracking(true);
            id = popMenu.exec(QCursor::pos());
        }

        if (id == 10)
        {
            emit signalProgressBarMode(StatusProgressBar::ProgressBarMode,
                                       i18n("Assigning image tags. Please wait..."));

            AlbumLister::instance()->blockSignals(true);
            d->albumMan->albumDB()->beginTransaction();
            int i=0;
            for (QValueList<int>::const_iterator it = imageIDs.begin();
                 it != imageIDs.end(); ++it)
            {
                // create temporary ImageInfo object
                ImageInfo info(*it);

                MetadataHub hub;
                hub.load(&info);
                hub.setTag(destAlbum, true);
                hub.write(&info, MetadataHub::PartialWrite);
                hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);

                emit signalProgressValue((int)((i++/(float)imageIDs.count())*100.0));
                kapp->processEvents();
            }
            AlbumLister::instance()->blockSignals(false);
            d->albumMan->albumDB()->commitTransaction();

            ImageAttributesWatch::instance()->imagesChanged(destAlbum->id());

            emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
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

void TagFolderView::refresh()
{
    QListViewItemIterator it(this);

    while (it.current())
    {
        TagFolderViewItem* item = dynamic_cast<TagFolderViewItem*>(*it);
        if (item)
            item->refresh();
        ++it;
    }
}

void TagFolderView::slotRefresh(const QMap<int, int>& tagsStatMap)
{
    QListViewItemIterator it(this);

    while (it.current())
    {
        TagFolderViewItem* item = dynamic_cast<TagFolderViewItem*>(*it);
        if (item)
        {
            if (item->album())
            {
                int id = item->id();
                QMap<int, int>::const_iterator it2 = tagsStatMap.find(id);
                if ( it2 != tagsStatMap.end() )
                    item->setCount(it2.data());
            }
        }
        ++it;
    }

    refresh();
}

}  // namespace Digikam
