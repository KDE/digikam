/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-06
 * Description : Albums folder view.
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

#include "albumfolderview.h"
#include "albumfolderview.moc"

// Qt includes.

#include <QCursor>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QDropEvent>
#include <QList>
#include <QPixmap>
#include <QPointer>

// KDE includes.

#include <kaction.h>
#include <kapplication.h>
#include <kcalendarsystem.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kstringhandler.h>

// Local includes.

#include "album.h"
#include "albumdb.h"
#include "albumlister.h"
#include "albummanager.h"
#include "albumpropsedit.h"
#include "albumsettings.h"
#include "albumthumbnailloader.h"
#include "cameraui.h"
#include "collectionmanager.h"
#include "contextmenuhelper.h"
#include "ddragobjects.h"
#include "deletedialog.h"
#include "digikamapp.h"
#include "dio.h"
#include "thumbnailsize.h"

namespace Digikam
{

AlbumFolderViewItem::AlbumFolderViewItem(Q3ListView *parent, PAlbum *album)
                   : FolderItem(parent, album->title())
{
    setDragEnabled(true);
    m_album          = album;
    m_groupItem      = false;
    m_count          = 0;
    m_countRecursive = 0;
}

AlbumFolderViewItem::AlbumFolderViewItem(Q3ListViewItem *parent, PAlbum *album)
                   : FolderItem(parent, album->title())
{
    setDragEnabled(true);
    m_album          = album;
    m_groupItem      = false;
    m_count          = 0;
    m_countRecursive = 0;
}

// special group item (collection/dates)
AlbumFolderViewItem::AlbumFolderViewItem(Q3ListViewItem* parent, const QString& name,
                                         int year, int month)
                   : FolderItem(parent, name, true)
{
    setDragEnabled(false);
    m_album          = 0;
    m_year           = year;
    m_month          = month;
    m_groupItem      = true;
    m_count          = 0;
    m_countRecursive = 0;
}

void AlbumFolderViewItem::refresh()
{
    if (!m_album) return;

    if (AlbumSettings::instance()->getShowFolderTreeViewItemsCount() &&
        dynamic_cast<AlbumFolderViewItem*>(parent()))
    {
        m_countRecursive = m_count;
        AlbumIterator it(m_album);
        while ( it.current() )
        {
            AlbumFolderViewItem *item = (AlbumFolderViewItem*)it.current()->extraData(listView());
            if (item)
                m_countRecursive += item->count();
            ++it;
        }

        if (isOpen())
            setText(0, QString("%1 (%2)").arg(m_album->title()).arg(m_count));
        else
            setText(0, QString("%1 (%2)").arg(m_album->title()).arg(m_countRecursive));
    }
    else
    {
        setText(0, m_album->title());
    }
}

void AlbumFolderViewItem::setOpen(bool o)
{
    Q3ListViewItem::setOpen(o);
    refresh();
}

PAlbum* AlbumFolderViewItem::album() const
{
    return m_album;
}

int AlbumFolderViewItem::id() const
{
    if (m_groupItem)
    {
        if (m_year != 0 && m_month != 0)
        {
            return (m_year*(-100) + m_month*(-1));
        }
        else
        {
            return ( - (AlbumSettings::instance()->getAlbumCategoryNames()
                        .indexOf(text(0)) ) );
        }
    }
    else
    {
        return m_album ? m_album->id() : 0;
    }
}

bool AlbumFolderViewItem::isGroupItem() const
{
    return m_groupItem;
}

int AlbumFolderViewItem::compare(Q3ListViewItem *i, int col, bool ascending) const
{
    if (!m_groupItem || m_year == 0 || m_month == 0)
    {
        return KStringHandler::naturalCompare(key(col, ascending), i->key(col, ascending));
    }

    AlbumFolderViewItem* thatItem = dynamic_cast<AlbumFolderViewItem*>(i);
    if (!thatItem)
        return 0;

    int myWeight  = m_year*100 + m_month;
    int hisWeight = thatItem->m_year*100 + thatItem->m_month;

    if (myWeight == hisWeight)
        return 0;
    else if (myWeight > hisWeight)
        return 1;
    else
        return -1;
}

void AlbumFolderViewItem::setCount(int count)
{
    m_count = count;
    refresh();
}

int AlbumFolderViewItem::count()
{
    return m_count;
}

int AlbumFolderViewItem::countRecursive()
{
    return m_countRecursive;
}

// -----------------------------------------------------------------------------

class AlbumFolderViewPriv
{
public:

    AlbumFolderViewPriv()
    {
        albumMan = 0;
    }

    AlbumManager                *albumMan;
    QList<AlbumFolderViewItem*>  groupItems;
};

AlbumFolderView::AlbumFolderView(QWidget *parent)
               : FolderView(parent, "AlbumFolderView"),
                 d(new AlbumFolderViewPriv)
{
    d->albumMan = AlbumManager::instance();

    addColumn(i18n("Albums"));
    setResizeMode(Q3ListView::LastColumn);
    setRootIsDecorated(false);
    setAllColumnsShowFocus(true);

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    connect(d->albumMan, SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(d->albumMan, SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(d->albumMan, SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(d->albumMan, SIGNAL(signalAlbumIconChanged(Album*)),
            this, SLOT(slotAlbumIconChanged(Album*)));

    connect(d->albumMan, SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    connect(d->albumMan, SIGNAL(signalPAlbumsDirty(const QMap<int, int>&)),
            this, SLOT(slotRefresh(const QMap<int, int>&)));

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();

    connect(loader, SIGNAL(signalThumbnail(Album *, const QPixmap&)),
            this, SLOT(slotGotThumbnailFromIcon(Album *, const QPixmap&)));

    connect(loader, SIGNAL(signalFailed(Album *)),
            this, SLOT(slotThumbnailLost(Album *)));

    connect(loader, SIGNAL(signalReloadThumbnails()),
            this, SLOT(slotReloadThumbnails()));

    connect(this, SIGNAL(contextMenuRequested(Q3ListViewItem*, const QPoint&, int)),
            this, SLOT(slotContextMenu(Q3ListViewItem*, const QPoint&, int)));

    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

AlbumFolderView::~AlbumFolderView()
{
    saveViewState();
    delete d;
}

void AlbumFolderView::slotTextFolderFilterChanged(const SearchTextSettings& settings)
{
    if (settings.text.isEmpty())
    {
        collapseView();
        return;
    }

    QString search       = settings.text;
    bool atleastOneMatch = false;

    AlbumList pList = d->albumMan->allPAlbums();
    for (AlbumList::const_iterator it = pList.constBegin(); it != pList.constEnd(); ++it)
    {
        PAlbum* palbum  = (PAlbum*)(*it);
        AlbumFolderViewItem* viewItem = (AlbumFolderViewItem*) palbum->extraData(this);

        if (!viewItem)
            continue;

        // don't touch the root Album
        if (palbum->isRoot() || palbum->isAlbumRoot())
        {
            viewItem->setOpen(true);
            continue;
        }

        bool doesExpand = false;
        bool match      = palbum->title().contains(search, settings.caseSensitive);

        if (!match)
        {
            // check if any of the parents match the search
            PAlbum* parent = dynamic_cast<PAlbum*>(palbum->parent());

            while (parent && !(parent->isRoot() || parent->isAlbumRoot()) )
            {
                if (parent->title().contains(search, settings.caseSensitive))
                {
                    match = true;
                    break;
                }

                parent = dynamic_cast<PAlbum*>(parent->parent());
            }
        }

        if (!match)
        {
            // check if any of the children match the search
            AlbumIterator it(palbum);
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

        if (match)
        {
            atleastOneMatch = true;

            viewItem->setVisible(true);
            viewItem->setOpen(doesExpand);
        }
        else
        {
            viewItem->setVisible(false);
            viewItem->setOpen(false);
        }
    }

    emit signalTextFolderFilterMatch(atleastOneMatch);
}

void AlbumFolderView::slotAlbumAdded(Album *album)
{
    if(!album)
        return;

    PAlbum *palbum = dynamic_cast<PAlbum*>(album);
    if(!palbum)
        return;

    bool failed;
    AlbumFolderViewItem* parent = findParent(palbum, failed);
    if (failed)
    {
        kWarning(50003) << " Failed to find Album parent "
                        << palbum->albumPath() << endl;
        return;
    }

    AlbumFolderViewItem *item;
    if (!parent)
    {
        // root album
        item = new AlbumFolderViewItem(this, palbum);
        palbum->setExtraData(this, item);
        item->setOpen(true);
    }
    else
    {
        item = new AlbumFolderViewItem(parent, palbum);
        palbum->setExtraData(this, item);
    }

    setAlbumThumbnail(palbum);
}

void AlbumFolderView::slotAlbumDeleted(Album *album)
{
    if(!album)
        return;

    PAlbum* palbum = dynamic_cast<PAlbum*>(album);
    if(!palbum)
        return;

    AlbumFolderViewItem* item = (AlbumFolderViewItem*) palbum->extraData(this);
    if(item)
    {
        AlbumFolderViewItem *itemParent = dynamic_cast<AlbumFolderViewItem*>(item->parent());

        if(itemParent)
            itemParent->takeItem(item);
        else
            takeItem(item);

        delete item;
        clearEmptyGroupItems();
    }
}

void AlbumFolderView::slotAlbumRenamed(Album *album)
{
    PAlbum* palbum = dynamic_cast<PAlbum*>(album);
    if(!palbum)
        return;

    AlbumFolderViewItem* item = (AlbumFolderViewItem*) palbum->extraData(this);
    if(item)
        item->refresh();
    if (item->parent())
        item->parent()->sort();
}

void AlbumFolderView::slotAlbumsCleared()
{
    d->groupItems.clear();
    clear();
}

void AlbumFolderView::setAlbumThumbnail(PAlbum *album)
{
    if(!album)
        return;

    AlbumFolderViewItem* item = (AlbumFolderViewItem*) album->extraData(this);

    if(!item)
        return;

    // Either, getThumbnail returns true and loads an icon asynchronously.
    // Then, for the time being, we set the standard icon.
    // Or, no icon is associated with the album, then we set the standard icon anyway.
    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();
    item->setPixmap(0, loader->getStandardAlbumIcon(album));
    loader->getAlbumThumbnail(album);
}

void AlbumFolderView::setCurrentAlbum(Album *album)
{
    if(!album) return;

    AlbumFolderViewItem* item = (AlbumFolderViewItem*) album->extraData(this);
    if(!item) return;

    setCurrentItem(item);
    ensureItemVisible(item);
}

void AlbumFolderView::slotGotThumbnailFromIcon(Album *album,
                                               const QPixmap& thumbnail)
{
    if(!album || album->type() != Album::PHYSICAL)
        return;

    AlbumFolderViewItem* item = (AlbumFolderViewItem*)album->extraData(this);

    if(!item)
        return;

    item->setPixmap(0, thumbnail);
}

void AlbumFolderView::slotThumbnailLost(Album *)
{
    // we already set the standard icon before loading
}

void AlbumFolderView::slotReloadThumbnails()
{
    AlbumList tList = d->albumMan->allPAlbums();
    for (AlbumList::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
    {
        PAlbum* album  = (PAlbum*)(*it);
        setAlbumThumbnail(album);
    }
}

void AlbumFolderView::slotAlbumIconChanged(Album* album)
{
    if(!album || album->type() != Album::PHYSICAL)
        return;

    setAlbumThumbnail((PAlbum*)album);
}

void AlbumFolderView::slotSelectionChanged()
{
    if(!active())
        return;

    Q3ListViewItem* selItem = 0;
    Q3ListViewItemIterator it(this);
    while(it.current())
    {
        if(it.current()->isSelected())
        {
            selItem = it.current();
            break;
        }
        ++it;
    }

    if(!selItem)
    {
        d->albumMan->setCurrentAlbum(0);
        return;
    }

    AlbumFolderViewItem *albumitem = dynamic_cast<AlbumFolderViewItem*>(selItem);
    if(!albumitem)
    {
        d->albumMan->setCurrentAlbum(0);
        return;
    }

    d->albumMan->setCurrentAlbum(albumitem->album());
}

void AlbumFolderView::slotContextMenu(Q3ListViewItem *listitem, const QPoint &, int)
{
    AlbumFolderViewItem *item = dynamic_cast<AlbumFolderViewItem*>(listitem);
    if (!item)
        return;

    PAlbum *album = item->album();
    if (item && (!album || album->isRoot()))
    {
        // if collection/date return
        return;
    }

    // temporary actions  -------------------------------------

    QAction *renameAction, *resetIconAction, *deleteAction = 0;

    // FIXME: use the same icon for both actions? If not, maybe we need to have a second one that is
    // showing a trashcan or something like that? Right now the icon looks more like a permanent delete,
    // not like a trash action.
    if(AlbumSettings::instance()->getUseTrash())
        deleteAction = new QAction(SmallIcon("albumfolder-user-trash"), i18n("Move Album to Trash"), this);
    else
        deleteAction = new QAction(SmallIcon("edit-delete-shred"), i18n("Delete Album"), this);

    renameAction    = new QAction(SmallIcon("edit-rename"), i18n("Rename..."), this);
    resetIconAction = new QAction(SmallIcon("view-refresh"), i18n("Reset Album Icon"), this);

    if (album->isAlbumRoot())
    {
        renameAction->setEnabled(false);
        deleteAction->setEnabled(false);
    }

    // --------------------------------------------------------

    KMenu popmenu(this);
    popmenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction("album_new");
    cmhelper.addAction(renameAction);
    cmhelper.addAction(resetIconAction);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addImportMenu();
    cmhelper.addExportMenu();
    cmhelper.addBatchMenu();
    cmhelper.addAlbumActions();
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction(deleteAction);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("album_propsEdit");

    // special action handling --------------------------------

    QAction* choice = cmhelper.exec(QCursor::pos());
    if (choice)
    {
        if (choice == resetIconAction)
        {
            QString err;
            d->albumMan->updatePAlbumIcon(item->album(), 0, err);
        }
        else if (choice == renameAction)
        {
            albumRename(item);
        }
        else if (choice == deleteAction)
        {
            albumDelete(item);
        }
    }

    // cleanup -----------------------

    popmenu.deleteLater();
    delete deleteAction;
    delete renameAction;
    delete resetIconAction;
}

void AlbumFolderView::albumNew()
{
    AlbumFolderViewItem *item = dynamic_cast<AlbumFolderViewItem*>(selectedItem());
    if (!item)
    {
        item = dynamic_cast<AlbumFolderViewItem*>(firstChild());
    }

    if (!item)
        return;

    albumNew(item);
}

void AlbumFolderView::albumNew(AlbumFolderViewItem *item)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if(!settings)
    {
        kWarning(50003) << "AlbumFolderView: Could not get Album Settings" << endl;
        return;
    }

    /*
    QDir libraryDir(settings->getAlbumLibraryPath());
    if(!libraryDir.exists())
    {
        KMessageBox::error(0,
                           i18n("The album library has not been set correctly.\n"
                                "Select \"Configure Digikam\" from the Settings "
                                "menu and choose a folder to use for the album "
                                "library."));
        return;
    }
    */

    PAlbum *parent;

    if(!item)
        parent = d->albumMan->findPAlbum(0);
    else
        parent = item->album();

    if (!parent)
        return;

    // if we create an album under root, need to supply the album root path.
    QString albumRootPath;
    if (parent->isRoot())
    {
        //TODO: Let user choose an album root
        albumRootPath = CollectionManager::instance()->oneAlbumRootPath();
    }

    QString     title;
    QString     comments;
    QString     category;
    QDate       date;
    QStringList albumCategories;

    if(!AlbumPropsEdit::createNew(parent, title, comments, date, category,
                                  albumCategories))
        return;

    QStringList oldAlbumCategories(AlbumSettings::instance()->getAlbumCategoryNames());
    if(albumCategories != oldAlbumCategories)
    {
        AlbumSettings::instance()->setAlbumCategoryNames(albumCategories);
        resort();
    }

    QString errMsg;
    PAlbum* album;
    if (parent->isRoot())
        album = d->albumMan->createPAlbum(albumRootPath, title, comments,
                                          date, category, errMsg);
    else
        album = d->albumMan->createPAlbum(parent, title, comments,
                                          date, category, errMsg);

    if (!album)
    {
        KMessageBox::error(0, errMsg);
        return;
    }

    // by this time the signalAlbumAdded has been fired and the appropriate
    // AlbumFolderViewItem has been created. Now make this folderviewitem visible
    AlbumFolderViewItem* newItem = (AlbumFolderViewItem*)album->extraData(this);
    if (newItem)
    {
        if(item)
            item->setOpen(true);

        ensureItemVisible(newItem);
        setSelected(newItem, true);
    }
}

void AlbumFolderView::albumDelete()
{
    AlbumFolderViewItem *item = dynamic_cast<AlbumFolderViewItem*>(selectedItem());
    if(!item)
        return;

    albumDelete(item);
}

void AlbumFolderView::albumDelete(AlbumFolderViewItem *item)
{
    PAlbum *album = item->album();

    if(!album || album->isRoot() || album->isAlbumRoot())
        return;

    // find subalbums
    KUrl::List childrenList;
    addAlbumChildrenToList(childrenList, album);

    DeleteDialog dialog(this);

    // All subalbums will be presented in the list as well
    if (!dialog.confirmDeleteList(childrenList,
                                  childrenList.size() == 1 ?
                                  DeleteDialogMode::Albums : DeleteDialogMode::Subalbums,
                                  DeleteDialogMode::UserPreference))
        return;

    bool useTrash = !dialog.shouldDelete();

    // Currently trash kioslave can handle only full paths.
    // pass full folder path to the trashing job
    //TODO: Use digikamalbums:// url?
    KUrl u;
    u.setProtocol("file");
    u.setPath(album->folderPath());
    KIO::Job* job = DIO::del(u, useTrash);
    connect(job, SIGNAL(result(KJob *)),
            this, SLOT(slotDIOResult(KJob *)));
}

void AlbumFolderView::addAlbumChildrenToList(KUrl::List &list, Album *album)
{
    // simple recursive helper function
    if (album)
    {
        list.append(album->databaseUrl());
        AlbumIterator it(album);
        while(it.current())
        {
            addAlbumChildrenToList(list, *it);
            ++it;
        }
    }
}

void AlbumFolderView::slotDIOResult(KJob* kjob)
{
    KIO::Job *job = static_cast<KIO::Job*>(kjob);
    if (job->error())
    {
        job->ui()->setWindow(this);
        job->ui()->showErrorMessage();
    }
}

void AlbumFolderView::albumRename()
{
    AlbumFolderViewItem *item = dynamic_cast<AlbumFolderViewItem*>(selectedItem());
    if(!item)
        return;

    albumRename(item);
}

void AlbumFolderView::albumRename(AlbumFolderViewItem* item)
{
    PAlbum *album = item->album();

    if (!album)
        return;

    QString oldTitle(album->title());
    bool    ok;

    QString title = KInputDialog::getText(i18n("Rename Album (%1)",oldTitle),
                                          i18n("Enter new album name:"),
                                          oldTitle, &ok, this);
    if (!ok)
        return;

    if(title != oldTitle)
    {
        QString errMsg;
        if (!d->albumMan->renamePAlbum(album, title, errMsg))
            KMessageBox::error(0, errMsg);
    }

    emit signalAlbumModified();
}

void AlbumFolderView::albumEdit()
{
    AlbumFolderViewItem *item = dynamic_cast<AlbumFolderViewItem*>(selectedItem());
    if(!item)
        return;

    albumEdit(item);
}

void AlbumFolderView::albumEdit(AlbumFolderViewItem* item)
{
    PAlbum *album = item->album();

    if (!album || album->isRoot() || album->isAlbumRoot())
        return;

    QString     oldTitle(album->title());
    QString     oldComments(album->caption());
    QString     oldCategory(album->category());
    QDate       oldDate(album->date());
    QStringList oldAlbumCategories(AlbumSettings::instance()->getAlbumCategoryNames());

    QString     title, comments, category;
    QDate       date;
    QStringList albumCategories;

    if(AlbumPropsEdit::editProps(album, title, comments, date,
                                 category, albumCategories))
    {
        if(comments != oldComments)
            album->setCaption(comments);

        if(date != oldDate && date.isValid())
            album->setDate(date);

        if(category != oldCategory)
            album->setCategory(category);

        AlbumSettings::instance()->setAlbumCategoryNames(albumCategories);
        resort();

        // Do this last : so that if anything else changed we can
        // successfuly save to the db with the old name

        if(title != oldTitle)
        {
            QString errMsg;
            if (!d->albumMan->renamePAlbum(album, title, errMsg))
                KMessageBox::error(0, errMsg);
        }

        emit signalAlbumModified();
    }
}

QDrag* AlbumFolderView::makeDragObject()
{
    AlbumFolderViewItem *item = dynamic_cast<AlbumFolderViewItem*>(dragItem());
    if(!item)
        return 0;

    PAlbum *album = item->album();
    if(album->isRoot())
        return 0;

    QDrag *drag = new QDrag(this);
    drag->setMimeData(new DAlbumDrag(album->databaseUrl(), album->id()));
    drag->setPixmap(*item->pixmap(0));

    return drag;
}

bool AlbumFolderView::acceptDrop(const QDropEvent *e) const
{
    QPoint vp = contentsToViewport(e->pos());
    AlbumFolderViewItem *itemDrop = dynamic_cast<AlbumFolderViewItem*>(itemAt(vp));
    AlbumFolderViewItem *itemDrag = dynamic_cast<AlbumFolderViewItem*>(dragItem());

    if(DAlbumDrag::canDecode(e->mimeData()))
    {
        switch(AlbumSettings::instance()->getAlbumSortOrder())
        {
            case(AlbumSettings::ByFolder):
            {
                // Allow dragging at the root, to move the album at the root
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
            case (AlbumSettings::ByCategory):
            {
                if (!itemDrop)
                    return false;

                // Only allow dragging onto Collection
                if (itemDrop->isGroupItem())
                    return true;

                return false;
            }
            default:
            {
                return false;
            }
        }
    }

    if(itemDrop  && !itemDrop->parent())
    {
        // Do not allow drop images on album root
        return false;
    }

    if(itemDrop && itemDrop->isGroupItem())
    {
        // do not allow drop on a group item
        return false;
    }

    if(DItemDrag::canDecode(e->mimeData()))
    {
        return true;
    }

    if(DCameraItemListDrag::canDecode(e->mimeData()))
    {
        return true;
    }

    if(KUrl::List::canDecode(e->mimeData()))
    {
        return true;
    }

    return false;
}

void AlbumFolderView::contentsDropEvent(QDropEvent *e)
{
    FolderView::contentsDropEvent(e);

    if(!acceptDrop(e))
        return;

    QPoint vp                     = contentsToViewport(e->pos());
    AlbumFolderViewItem *itemDrop = dynamic_cast<AlbumFolderViewItem*>(itemAt(vp));

    if(DAlbumDrag::canDecode(e->mimeData()))
    {
        AlbumFolderViewItem *itemDrag = dynamic_cast<AlbumFolderViewItem*>(dragItem());
        if(!itemDrag)
            return;

        if (AlbumSettings::instance()->getAlbumSortOrder() == AlbumSettings::ByFolder)
        {
            // TODO: Copy?
            KMenu popMenu(this);
            popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
            QAction *moveAction = popMenu.addAction(SmallIcon("go-jump"), i18n("&Move Here"));
            popMenu.addSeparator();
            popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
            popMenu.setMouseTracking(true);
            QAction *choice = popMenu.exec(QCursor::pos());

            if(choice == moveAction)
            {
                PAlbum *album = itemDrag->album();
                PAlbum *destAlbum;
                if(!itemDrop)
                {
                    // move dragItem to the root
                    destAlbum = d->albumMan->findPAlbum(0);
                }
                else
                {
                    // move dragItem below dropItem
                    destAlbum = itemDrop->album();
                }
                KIO::Job* job = DIO::move(album, destAlbum);
                connect(job, SIGNAL(result(KJob*)),
                        this, SLOT(slotDIOResult(KJob*)));
            }
        }
        else if (AlbumSettings::instance()->getAlbumSortOrder() == AlbumSettings::ByCategory)
        {
            if (!itemDrop)
                return;

            if (itemDrop->isGroupItem())
            {
                PAlbum *album = itemDrag->album();
                if (!album)
                    return;

                album->setCategory(itemDrop->text(0));
                resort();
            }
        }

        return;
    }

    if (DItemDrag::canDecode(e->mimeData()))
    {
        if (!itemDrop)
            return;

        PAlbum *destAlbum = itemDrop->album();

        KUrl::List urls;
        KUrl::List kioURLs;
        QList<int> albumIDs;
        QList<int> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
            return;

        if (urls.isEmpty() || kioURLs.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
            return;

        // Check if items dropped come from outside current album.
        // This can be the case with reccursive content album mode.
        KUrl::List extUrls;
        ImageInfoList extImgInfList;
        QList<qlonglong> extImageIDs;
        for (QList<int>::const_iterator it = imageIDs.constBegin(); it != imageIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);
            if (info.albumId() != destAlbum->id())
            {
                extUrls.append(info.databaseUrl());
                extImgInfList.append(info);
                extImageIDs << *it;
            }
        }

        if(extUrls.isEmpty())
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
                popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
                QAction *setAction = 0;
                if (imageIDs.count() == 1)
                    setAction = popMenu.addAction(i18n("Set as Album Thumbnail"));
                popMenu.addSeparator();
                popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
                popMenu.setMouseTracking(true);
                QAction *choice = popMenu.exec(QCursor::pos());
                set = (setAction == choice);
            }

            if(set)
            {
                QString errMsg;
                d->albumMan->updatePAlbumIcon(destAlbum, imageIDs.first(), errMsg);
            }
            return;
        }

        // If shift key is pressed while dragging, move the drag object without
        // displaying popup menu -> move
        bool move = false, copy = false, setThumbnail = false;
        if (e->keyboardModifiers() == Qt::ShiftModifier)
        {
            move = true;
        }
        // If ctrl key is pressed while dragging, copy the drag object without
        // displaying popup menu -> copy
        else if (e->keyboardModifiers() == Qt::ControlModifier)
        {
            copy = true;
        }
        else
        {
            KMenu popMenu(this);
            popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
            QAction *moveAction = popMenu.addAction(SmallIcon("go-jump"), i18n("&Move Here"));
            QAction *copyAction = popMenu.addAction(SmallIcon("edit-copy"), i18n("&Copy Here"));
            QAction *thumbnailAction = 0;
            if (imageIDs.count() == 1)
                thumbnailAction = popMenu.addAction(i18n("Set as Album Thumbnail"));
            popMenu.addSeparator();
            popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
            popMenu.setMouseTracking(true);
            QAction *choice = popMenu.exec(QCursor::pos());
            if (choice)
            {
                if (choice == moveAction)
                    move = true;
                else if (choice == copyAction)
                    copy = true;
                else if (choice == thumbnailAction)
                    setThumbnail = true;
            }
        }

        if (move)
        {
            KIO::Job* job = DIO::move(extUrls, extImageIDs, destAlbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(slotDIOResult(KJob*)));

            // In recurssive album contents mode, we need to force AlbumLister to take a care about
            // moved items. This will have no incidence in normal mode.
            for (ImageInfoListIterator it = extImgInfList.begin(); it != extImgInfList.end(); ++it)
            {
                AlbumLister::instance()->invalidateItem(*it);
            }
        }
        else if (copy)
        {
            KIO::Job* job = DIO::copy(extUrls, extImageIDs, destAlbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(slotDIOResult(KJob*)));
        }
        else if (setThumbnail)
        {
            QString errMsg;
            d->albumMan->updatePAlbumIcon(destAlbum, imageIDs.first(), errMsg);
        }

        return;
    }

    // -- DnD from Camera GUI ------------------------------------------------

    if(DCameraItemListDrag::canDecode(e->mimeData()))
    {
        Album *album = dynamic_cast<Album*>(itemDrop->album());
        if (!album) return;

        CameraUI *ui = dynamic_cast<CameraUI*>(e->source());
        if (ui)
        {
            KMenu popMenu(this);
            popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
            QAction *downAction    = popMenu.addAction(SmallIcon("file-export"),
                                                       i18n("Download From Camera"));
            QAction *downDelAction = popMenu.addAction(SmallIcon("file-export"),
                                                       i18n("Download && Delete From Camera"));
            popMenu.addSeparator();
            popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
            popMenu.setMouseTracking(true);
            QAction *choice = popMenu.exec(QCursor::pos());
            if (choice)
            {
                if (choice == downAction)
                    ui->slotDownload(true, false, album);
                else if (choice == downDelAction)
                    ui->slotDownload(true, true, album);
            }
        }
    }

    // -- DnD from an external source ----------------------------------------

    if(KUrl::List::canDecode(e->mimeData()))
    {
        PAlbum* destAlbum = 0;

        if (itemDrop)
            destAlbum = itemDrop->album();
        else
            destAlbum = d->albumMan->findPAlbum(0);

        // B.K.O #119205: do not handle root album.
        if (destAlbum->isRoot())
            return;

        KUrl destURL(destAlbum->databaseUrl());

        KUrl::List srcURLs = KUrl::List::fromMimeData(e->mimeData());

        bool move = false, copy = false;
        // If shift key is pressed while dropping, move the drag object without
        // displaying popup menu -> move
        if (e->keyboardModifiers() == Qt::ShiftModifier)
        {
            move = true;
        }
        // If ctrl key is pressed while dropping, copy the drag object without
        // displaying popup menu -> copy
        else if (e->keyboardModifiers() == Qt::ControlModifier)
        {
            copy = true;
        }
        else
        {
            KMenu popMenu(this);
            popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
            QAction *moveAction = popMenu.addAction( SmallIcon("go-jump"), i18n("&Move Here"));
            QAction *copyAction = popMenu.addAction( SmallIcon("edit-copy"), i18n("&Copy Here"));
            popMenu.addSeparator();
            popMenu.addAction( SmallIcon("dialog-cancel"), i18n("C&ancel") );
            popMenu.setMouseTracking(true);
            QAction *choice = popMenu.exec(QCursor::pos());
            if (choice == copyAction)
                copy = true;
            else if (choice == moveAction)
                move = true;
        }

        if (move)
        {
            KIO::Job* job = DIO::move(srcURLs, destAlbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(slotDIOResult(KJob*)));
        }
        else if (copy)
        {
            KIO::Job* job = DIO::copy(srcURLs, destAlbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(slotDIOResult(KJob*)));
        }

        return;
    }
}

void AlbumFolderView::selectItem(int id)
{
    PAlbum* album = d->albumMan->findPAlbum(id);
    if(!album)
        return;

    AlbumFolderViewItem *item = (AlbumFolderViewItem*)album->extraData(this);
    if(item)
    {
        setSelected(item, true);
        ensureItemVisible(item);
    }
}

AlbumFolderViewItem* AlbumFolderView::findParent(PAlbum* album, bool& failed)
{
    if (album->isRoot())
    {
        failed = false;
        return 0;
    }

    switch(AlbumSettings::instance()->getAlbumSortOrder())
    {
        case(AlbumSettings::ByFolder):
        {
            return findParentByFolder(album, failed);
        }
        case(AlbumSettings::ByCategory):
        {
            return findParentByCategory(album, failed);
        }
        case(AlbumSettings::ByDate):
        {
            return findParentByDate(album, failed);
        }
    }

    failed = true;
    return 0;
}

AlbumFolderViewItem* AlbumFolderView::findParentByFolder(PAlbum* album, bool& failed)
{
    AlbumFolderViewItem* parent = 0;
    if (album->parent())
        parent = static_cast<AlbumFolderViewItem*>(album->parent()->extraData(this));

    if (!parent)
    {
        failed = true;
        return 0;
    }

    if (album->parent()->isRoot())
    {
        QStringList albumRoots = CollectionManager::instance()->allAvailableAlbumRootPaths();
        if (albumRoots.count() > 1)
        {
            for (QList<AlbumFolderViewItem*>::const_iterator it = d->groupItems.constBegin();
                 it != d->groupItems.constEnd(); ++it)
            {
                AlbumFolderViewItem* groupItem = *it;
                if (groupItem->text(0) == album->albumRootPath())
                {
                    parent = groupItem;
                    break;
                }
            }

            // Need to create a new parent item
            if (!parent)
            {
                parent = new AlbumFolderViewItem(firstChild(), album->albumRootPath(), 0, 0);
                d->groupItems.append(parent);
            }
        }
    }

    failed = false;
    return parent;
}

AlbumFolderViewItem* AlbumFolderView::findParentByCategory(PAlbum* album, bool& failed)
{
    QStringList categoryList = AlbumSettings::instance()->getAlbumCategoryNames();
    QString category         = album->category();

    if (category.isEmpty() || !categoryList.contains(category))
        category = i18n("Uncategorized Albums");

    AlbumFolderViewItem* parent = 0;

    for (QList<AlbumFolderViewItem*>::const_iterator it = d->groupItems.constBegin();
         it != d->groupItems.constEnd(); ++it)
    {
        AlbumFolderViewItem* groupItem = *it;
        if (groupItem->text(0) == category)
        {
            parent = groupItem;
            break;
        }
    }

    // Need to create a new parent item
    if (!parent)
    {
        parent = new AlbumFolderViewItem(firstChild(), category, 0, 0);
        d->groupItems.append(parent);
    }

    failed = false;
    return parent;
}

AlbumFolderViewItem* AlbumFolderView::findParentByDate(PAlbum* album, bool& failed)
{
    QDate date = album->date();

    QString timeString = QString::number(date.year()) + ", " +
                         KGlobal::locale()->calendar()->monthName(date, KCalendarSystem::LongName);

    AlbumFolderViewItem* parent = 0;

    for (QList<AlbumFolderViewItem*>::const_iterator it = d->groupItems.constBegin();
         it != d->groupItems.constEnd(); ++it)
    {
        AlbumFolderViewItem* groupItem = *it;
        if (groupItem->text(0) == timeString)
        {
            parent = groupItem;
            break;
        }
    }

    // Need to create a new parent item
    if (!parent)
    {
        parent = new AlbumFolderViewItem(firstChild(), timeString,
                                         date.year(), date.month());
        d->groupItems.append(parent);
    }

    failed = false;
    return parent;
}

void AlbumFolderView::resort()
{
    AlbumFolderViewItem* prevSelectedItem = dynamic_cast<AlbumFolderViewItem*>(selectedItem());
    if (prevSelectedItem && prevSelectedItem->isGroupItem())
        prevSelectedItem = 0;

    AlbumList pList(d->albumMan->allPAlbums());
    for (AlbumList::const_iterator it = pList.constBegin(); it != pList.constEnd(); ++it)
    {
        PAlbum *album = (PAlbum*)(*it);
        if (!album->isRoot() && album->extraData(this))
        {
            reparentItem(static_cast<AlbumFolderViewItem*>(album->extraData(this)));
        }
    }

    // Clear any groupitems which have been left empty
    clearEmptyGroupItems();

    if (prevSelectedItem)
    {
        ensureItemVisible(prevSelectedItem);
        setSelected(prevSelectedItem, true);
    }
}

void AlbumFolderView::reparentItem(AlbumFolderViewItem* folderItem)
{
    if (!folderItem)
        return;

    PAlbum* album = folderItem->album();
    if (!album || album->isRoot())
        return;

    AlbumFolderViewItem* oldParent = dynamic_cast<AlbumFolderViewItem*>(folderItem->parent());

    bool failed;
    AlbumFolderViewItem* newParent = findParent(album, failed);
    if (failed)
        return;

    if (oldParent == newParent)
        return;

    if (oldParent)
        oldParent->removeItem(folderItem);
    else
        removeItem(folderItem);

    // insert into new parent
    if (newParent)
        newParent->insertItem(folderItem);
    else
        insertItem(folderItem);
}

void AlbumFolderView::clearEmptyGroupItems()
{
    QList<AlbumFolderViewItem*> deleteItems;

    for (QList<AlbumFolderViewItem*>::iterator it=d->groupItems.begin();
         it != d->groupItems.end(); )
    {
        AlbumFolderViewItem* groupItem = *it;

        if (!groupItem->firstChild())
        {
            it = d->groupItems.erase(it);
            delete groupItem;
        }
        else
            ++it;
    }
}

void AlbumFolderView::refresh()
{
    Q3ListViewItemIterator it(this);

    while (it.current())
    {
        AlbumFolderViewItem* item = dynamic_cast<AlbumFolderViewItem*>(*it);
        if (item)
            item->refresh();
        ++it;
    }
}

void AlbumFolderView::slotRefresh(const QMap<int, int>& albumsStatMap)
{
    Q3ListViewItemIterator it(this);

    while (it.current())
    {
        AlbumFolderViewItem* item = dynamic_cast<AlbumFolderViewItem*>(*it);
        if (item)
        {
            if (item->album())
            {
                int id = item->id();
                QMap<int, int>::const_iterator it2 = albumsStatMap.constFind(id);
                if ( it2 != albumsStatMap.constEnd() )
                    item->setCount(it2.value());
            }
        }
        ++it;
    }

    refresh();
}

}  // namespace Digikam
