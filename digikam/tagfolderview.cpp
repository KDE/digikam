/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : tags folder view.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern.ahrens@kdemail.net>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "tagfolderview.h"
#include "tagfolderview.moc"

// Qt includes

#include <QCursor>
#include <QList>
#include <QPainter>

// KDE includes

#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kstringhandler.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albumlister.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "albumthumbnailloader.h"
#include "contextmenuhelper.h"
#include "databasetransaction.h"
#include "ddragobjects.h"
#include "digikamapp.h"
#include "dio.h"
#include "folderitem.h"
#include "imageattributeswatch.h"
#include "imageinfo.h"
#include "metadatahub.h"
#include "scancontroller.h"
#include "statusprogressbar.h"
#include "syncjob.h"
#include "tageditdlg.h"

namespace Digikam
{

class TagFolderViewItem : public FolderItem
{
public:

    TagFolderViewItem(Q3ListView *parent, TAlbum *album);
    TagFolderViewItem(Q3ListViewItem *parent, TAlbum *album);

    TAlbum* album() const;
    int     id() const;
    void    refresh();
    void    setOpen(bool o);
    void    setCount(int count);
    int     count();
    int     compare(Q3ListViewItem *i, int col, bool ascending) const;


private:

    int     m_count;

    TAlbum *m_album;
};

int TagFolderViewItem::compare(Q3ListViewItem *i, int col, bool ascending) const
{
    return KStringHandler::naturalCompare(key(col, ascending), i->key(col, ascending));
}

TagFolderViewItem::TagFolderViewItem(Q3ListView *parent, TAlbum *album)
                 : FolderItem(parent, album->title())
{
    setDragEnabled(true);
    m_album = album;
    m_count = 0;
}

TagFolderViewItem::TagFolderViewItem(Q3ListViewItem *parent, TAlbum *album)
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
    Q3ListViewItem::setOpen(o);
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
        albumMan          = 0;
    }

    AlbumManager      *albumMan;
};

TagFolderView::TagFolderView(QWidget *parent)
             : FolderView(parent, "TagFolderView"),
               d(new TagFolderViewPriv)
{
    d->albumMan = AlbumManager::instance();

    addColumn(i18n("Tags"));
    setResizeMode(Q3ListView::LastColumn);
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

    connect(this, SIGNAL(contextMenuRequested(Q3ListViewItem*, const QPoint&, int)),
            SLOT(slotContextMenu(Q3ListViewItem*, const QPoint&, int)));

    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));

    // ------------------------------------------------------------------------

    connect(this, SIGNAL(assignTags(int, const QList<int> &)),
            this, SLOT(slotAssignTags(int, const QList<int> &)),
            Qt::QueuedConnection);

}

TagFolderView::~TagFolderView()
{
    saveViewState();
    delete d;
}

void TagFolderView::slotTextTagFilterChanged(const SearchTextSettings& settings)
{
    if (settings.text.isEmpty())
    {
        collapseView();
        return;
    }

    QString search       = settings.text;
    bool atleastOneMatch = false;

    AlbumList tList = d->albumMan->allTAlbums();
    for (AlbumList::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
    {
        TAlbum* talbum  = (TAlbum*)(*it);

        // don't touch the root Album
        if (talbum->isRoot())
            continue;

        bool match = talbum->title().contains(search, settings.caseSensitive);
        bool doesExpand = false;
        if (!match)
        {
            // check if any of the parents match the search
            Album* parent = talbum->parent();
            while (parent && !parent->isRoot())
            {
                if (parent->title().contains(search, settings.caseSensitive))
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
                if ((*it)->title().contains(search, settings.caseSensitive))
                {
                    match      = true;
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
        TagFolderViewItem *parent = 0;
        if (tag->parent())
            parent = static_cast<TagFolderViewItem*>(tag->parent()->extraData(this));

        if (!parent)
        {
            kWarning(50003) << " Failed to find parent for Tag "
                            << tag->title();
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
        Q3ListViewItem* oldPItem = item->parent();
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
    for (AlbumList::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
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

    Q3ListViewItem* selItem = 0;
    Q3ListViewItemIterator it(this);
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

void TagFolderView::slotContextMenu(Q3ListViewItem *item, const QPoint &, int)
{
    TagFolderViewItem *tag = dynamic_cast<TagFolderViewItem*>(item);

    // temporary actions --------------------------------------

    QAction *resetIconAction = new QAction(SmallIcon("view-refresh"), i18n("Reset Tag Icon"), this);
    QAction *findDuplAction  = new QAction(SmallIcon("tools-wizard"), i18n("Find Duplicates..."), this);

    if (!tag || !tag->parent())
        resetIconAction->setEnabled(false);
    // --------------------------------------------------------

    KMenu popmenu(this);
    popmenu.addTitle(SmallIcon("digikam"), i18n("My Tags"));
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction("tag_new");
    cmhelper.addCreateTagFromAddressbookMenu();
    cmhelper.addAction(resetIconAction);
    cmhelper.addAction(findDuplAction);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("tag_delete");
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("tag_edit");

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAddNewTagFromABCMenu(const QString&)),
            this, SLOT(slotTagNewFromABCMenu(const QString&)));

    QAction* choice = cmhelper.exec(QCursor::pos());
    if (choice)
    {
        if (choice == resetIconAction)
        {
            QString errMsg;
            d->albumMan->updateTAlbumIcon(tag->album(), QString("tag"), 0, errMsg);
        }
        else if (choice == findDuplAction)
        {
            TAlbum* album = tag->album();
            emit signalFindDuplicatesInTag(album);
        }
    }
}

void TagFolderView::slotTagNewFromABCMenu(const QString& name)
{
    TagFolderViewItem *item = dynamic_cast<TagFolderViewItem*>(selectedItem());
    tagNew(item, name);
}

void TagFolderView::tagNew()
{
    TagFolderViewItem *item = dynamic_cast<TagFolderViewItem*>(selectedItem());
    tagNew(item);
}

void TagFolderView::tagNew(TagFolderViewItem *item, const QString& _title, const QString& _icon)
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

    for (AlbumList::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
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
    {
        return;
    }

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
        ++children;
        ++iter;
    }

    if(children)
    {
        int result = KMessageBox::warningContinueCancel(this,
                       i18np("Tag '%2' has one subtag. "
                             "Deleting this will also delete "
                             "the subtag. "
                             "Do you want to continue?",
                             "Tag '%2' has %1 subtags. "
                             "Deleting this will also delete "
                             "the subtags. "
                             "Do you want to continue?",
                             children,
                             tag->title()));

        if(result != KMessageBox::Continue)
            return;
    }

    QString message;
    QList<qlonglong> assignedItems = DatabaseAccess().db()->getItemIDsInTag(tag->id());
    if (!assignedItems.isEmpty())
    {
        message = i18np("Tag '%2' is assigned to one item. "
                        "Do you want to continue?",
                        "Tag '%2' is assigned to %1 items. "
                        "Do you want to continue?",
                        assignedItems.count(), tag->title());
    }
    else
    {
        message = i18n("Delete '%1' tag?", tag->title());
    }

    int result = KMessageBox::warningContinueCancel(0, message,
                                                    i18n("Delete Tag"),
                                                    KGuiItem(i18n("Delete"),
                                                    "edit-delete"));

    if(result == KMessageBox::Continue)
    {
        QString errMsg;
        if (!d->albumMan->deleteTAlbum(tag, errMsg))
            KMessageBox::error(0, errMsg);
    }
}

QDrag* TagFolderView::makeDragObject()
{
    TagFolderViewItem *item = dynamic_cast<TagFolderViewItem*>(dragItem());
    if(!item)
        return 0;

    if(!item->parent())
        return 0;

    QDrag *drag = new QDrag(this);
    drag->setMimeData(new DTagDrag(item->album()->id()));
    drag->setPixmap(*item->pixmap(0));

    return drag;
}

bool TagFolderView::acceptDrop(const QDropEvent *e) const
{
    QPoint vp = contentsToViewport(e->pos());
    TagFolderViewItem *itemDrop = dynamic_cast<TagFolderViewItem*>(itemAt(vp));
    TagFolderViewItem *itemDrag = dynamic_cast<TagFolderViewItem*>(dragItem());

    if(DTagDrag::canDecode(e->mimeData()) || DTagListDrag::canDecode(e->mimeData()))
    {
        // Allow dragging on empty space when the itemDrag isn't already at root level
        if (!itemDrop && itemDrag->album()->parent()->isRoot())
            return false;

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

    if (DItemDrag::canDecode(e->mimeData()) && itemDrop && itemDrop->parent())
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

    if(DTagDrag::canDecode(e->mimeData()))
    {
        int tagID;
        if (!DTagDrag::decode(e->mimeData(), tagID))
            return;

        TAlbum* talbum = d->albumMan->findTAlbum(tagID);

        if(!talbum)
            return;

        if (itemDrop && talbum == itemDrop->album())
            return;

        KMenu popMenu(this);
        popMenu.addTitle(SmallIcon("digikam"), i18n("My Tags"));

        QAction *gotoAction = popMenu.addAction(SmallIcon("go-jump"), i18n("&Move Here"));
        popMenu.addSeparator();
        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
        popMenu.setMouseTracking(true);
        QAction *choice = popMenu.exec(QCursor::pos());

        if(choice == gotoAction)
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

    if (DItemDrag::canDecode(e->mimeData()))
    {
        TAlbum *destAlbum = itemDrop->album();
        TAlbum *srcAlbum;

        KUrl::List urls;
        KUrl::List kioURLs;
        QList<int> albumIDs;
        QList<int> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
            return;

        if (urls.isEmpty() || kioURLs.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
            return;

        // all the albumids will be the same
        int albumID = albumIDs.first();
        srcAlbum = d->albumMan->findTAlbum(albumID);
        if (!srcAlbum)
        {
            kWarning(50003) << "Could not find source album of drag";
            return;
        }

        if(srcAlbum == destAlbum)
        {
            // Setting the dropped image as the album thumbnail
            // If the ctrl key is pressed, when dropping the image, the
            // thumbnail is set without a popup menu
            bool set = false;
            if (e->keyboardModifiers() == Qt::ControlModifier)
            {
                set = true;
            }
            else
            {
                KMenu popMenu(this);
                popMenu.addTitle(SmallIcon("digikam"), i18n("My Tags"));
                QAction *setAction = 0;
                if (imageIDs.count() == 1)
                    setAction = popMenu.addAction(i18n("Set as Tag Thumbnail"));
                popMenu.addSeparator();
                popMenu.addAction( SmallIcon("dialog-cancel"), i18n("C&ancel") );

                popMenu.setMouseTracking(true);
                QAction *choice = popMenu.exec(QCursor::pos());
                set = (choice == setAction);
            }

            if(set)
            {
                QString errMsg;
                d->albumMan->updateTAlbumIcon(destAlbum, QString(), imageIDs.first(), errMsg);
            }
            return;
        }

        // If a ctrl key is pressed while dropping the drag object,
        // the tag is assigned to the images without showing a
        // popup menu.
        bool assign = false;
        if (e->keyboardModifiers() == Qt::ControlModifier)
        {
            assign = true;
        }
        else
        {
            KMenu popMenu(this);
            popMenu.addTitle(SmallIcon("digikam"), i18n("My Tags"));
            QAction * assignAction = popMenu.addAction(SmallIcon("tag"),
                                             i18n("Assign Tag '%1' to Items", destAlbum->prettyUrl()));
            popMenu.addSeparator();
            popMenu.addAction( SmallIcon("dialog-cancel"), i18n("C&ancel") );

            popMenu.setMouseTracking(true);
            QAction *choice = popMenu.exec(QCursor::pos());
            assign = (choice == assignAction);
        }

        if (assign)
        {
            emit assignTags(destAlbum->id(), imageIDs);
        }
    }
}

void TagFolderView::slotAssignTags(int tagId, const QList<int>& imageIDs)
{
    TAlbum *destAlbum = AlbumManager::instance()->findTAlbum(tagId);
    if (!destAlbum)
        return;

    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode,
                               i18n("Assigning image tags. Please wait..."));

    AlbumLister::instance()->blockSignals(true);
    ScanController::instance()->suspendCollectionScan();
    DatabaseTransaction transaction;
    MetadataHub         hub;
    int i=0;

    for (QList<int>::const_iterator it = imageIDs.constBegin(); it != imageIDs.constEnd(); ++it)
    {
                // create temporary ImageInfo object
        ImageInfo info(*it);

        hub.load(info);
        hub.setTag(destAlbum, true);

        QString filePath = info.filePath();
        hub.write(info, MetadataHub::PartialWrite);
        bool fileChanged = hub.write(filePath, MetadataHub::FullWriteIfChanged);
        if (fileChanged)
            ScanController::instance()->scanFileDirectly(filePath);

        emit signalProgressValue((int)((i++/(float)imageIDs.count())*100.0));
        kapp->processEvents();
    }
    ScanController::instance()->resumeCollectionScan();
    AlbumLister::instance()->blockSignals(false);

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
}

void TagFolderView::selectItem(int id)
{
    TAlbum* tag = d->albumMan->findTAlbum(id);
    if(!tag)
        return;

    TagFolderViewItem *item = (TagFolderViewItem*)tag->extraData(this);
    if(item)
    {
        setSelected(item, true);
        ensureItemVisible(item);
    }
}

void TagFolderView::refresh()
{
    Q3ListViewItemIterator it(this);

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
    Q3ListViewItemIterator it(this);

    while (it.current())
    {
        TagFolderViewItem* item = dynamic_cast<TagFolderViewItem*>(*it);
        if (item)
        {
            if (item->album())
            {
                int id = item->id();
                QMap<int, int>::const_iterator it2 = tagsStatMap.constFind(id);
                if ( it2 != tagsStatMap.constEnd() )
                    item->setCount(it2.value());
            }
        }
        ++it;
    }

    refresh();
}

}  // namespace Digikam
