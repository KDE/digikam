/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-06
 * Description : Albums folder view.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern.ahrens@kdemail.net>
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

#include <Q3ValueList>
#include <QList>
#include <QPixmap>
#include <QPointer>
#include <QDir>
#include <QDropEvent>
#include <QCursor>
#include <QDataStream>
#include <QDateTime>

// KDE includes.

#include <kmenu.h>
#include <klocale.h>
#include <kglobal.h>
#include <kcalendarsystem.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kfiledialog.h>
#include <kdeversion.h>
#include <kinputdialog.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>

// Local includes.

#include "ddebug.h"
#include "digikamapp.h"
#include "album.h"
#include "albumpropsedit.h"
#include "album.h"
#include "albummanager.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "collectionmanager.h"
#include "thumbnailsize.h"
#include "albumpropsedit.h"
#include "folderitem.h"
#include "cameraui.h"
#include "dio.h"
#include "dragobjects.h"
#include "albumthumbnailloader.h"
#include "deletedialog.h"
#include "albumfolderview.h"
#include "albumfolderview.moc"

namespace Digikam
{

//-----------------------------------------------------------------------------
// AlbumFolderViewItem
//-----------------------------------------------------------------------------

class AlbumFolderViewItem : public FolderItem
{
public:

    AlbumFolderViewItem(Q3ListView *parent, PAlbum *album);
    AlbumFolderViewItem(Q3ListViewItem *parent, PAlbum *album);

    // special group item (collection/dates)
    AlbumFolderViewItem(Q3ListViewItem* parent, const QString& name,
                        int year, int month);

    PAlbum* getAlbum() const;
    int id() const;
    bool isGroupItem() const;

    int compare(Q3ListViewItem *i, int col, bool ascending) const;

private:

    PAlbum *m_album;
    int     m_year;
    int     m_month;
    bool    m_groupItem;
};

AlbumFolderViewItem::AlbumFolderViewItem(Q3ListView *parent, PAlbum *album)
                   : FolderItem(parent, album->title())
{
    setDragEnabled(true);
    m_album     = album;
    m_groupItem = false;
}

AlbumFolderViewItem::AlbumFolderViewItem(Q3ListViewItem *parent, PAlbum *album)
                   : FolderItem(parent, album->title())
{
    setDragEnabled(true);
    m_album     = album;
    m_groupItem = false;
}

// special group item (collection/dates)
AlbumFolderViewItem::AlbumFolderViewItem(Q3ListViewItem* parent, const QString& name,
                                         int year, int month)
                   : FolderItem(parent, name, true),
                     m_album(0), m_year(year), m_month(month), m_groupItem(true)
{
    setDragEnabled(false);
}

PAlbum* AlbumFolderViewItem::getAlbum() const
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
            return ( - (AlbumSettings::instance()->getAlbumCollectionNames()
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
        return Q3ListViewItem::compare(i, col, ascending);

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


//-----------------------------------------------------------------------------
// AlbumFolderViewPriv
//-----------------------------------------------------------------------------

class AlbumFolderViewPriv
{
public:

    AlbumFolderViewPriv()
    {
        albumMan     = 0;
    }

    AlbumManager                     *albumMan;
    Q3ValueList<AlbumFolderViewItem*>  groupItems;
};

//-----------------------------------------------------------------------------
// AlbumFolderView
//-----------------------------------------------------------------------------

AlbumFolderView::AlbumFolderView(QWidget *parent)
               : FolderView(parent, "AlbumFolderView")
{
    d = new AlbumFolderViewPriv();
    d->albumMan     = AlbumManager::instance();

    addColumn(i18n("My Albums"));
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
    delete d;
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
        DWarning() << " Failed to find Album parent "
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
    {
        item->setText(0, palbum->title());
    }
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
    AlbumList tList = AlbumManager::instance()->allPAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
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

    d->albumMan->setCurrentAlbum(albumitem->getAlbum());
}

void AlbumFolderView::slotContextMenu(Q3ListViewItem *listitem, const QPoint &, int)
{
    QMenu menuImport(i18n("Import"));
    QMenu menuExport(i18n("Export"));
    QMenu menuKIPIBatch(i18n("Batch Process"));

    KMenu popmenu(this);
    popmenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
    QAction *newAction = popmenu.addAction(SmallIcon("albumfolder-new"), i18n("New Album..."));

    AlbumFolderViewItem *item = dynamic_cast<AlbumFolderViewItem*>(listitem);
    if (item && !item->getAlbum())
    {
        // if collection/date return
        return;
    }

    // Root folder only shows "New Album..."
    QAction *renameAction = 0, *propertiesAction = 0, *resetIconAction = 0, *deleteAction = 0;
    if(item && item->parent())
    {
        renameAction     = popmenu.addAction(SmallIcon("pencil"), i18n("Rename..."));
        propertiesAction = popmenu.addAction(SmallIcon("albumfolder-properties"), i18n("Album Properties..."));
        resetIconAction  = popmenu.addAction(SmallIcon("view-refresh"), i18n("Reset Album Icon"));
        popmenu.addSeparator();

        // Add KIPI Albums plugins Actions
        const QList<QAction*>& albumActions = DigikamApp::getinstance()->menuAlbumActions();
        if(!albumActions.isEmpty())
        {
            foreach(QAction *action, albumActions)
            {
                popmenu.addAction(action);
            }
        }

        // Add All Export Actions
        const QList<QAction*> exportActions = DigikamApp::getinstance()->menuExportActions();
        if(!exportActions.isEmpty())
        {
            foreach(QAction *action, exportActions)
            {
                menuExport.addAction(action);
            }
            popmenu.addMenu(&menuExport);
        }

        // Add KIPI Batch processes plugins Actions
        const QList<QAction*>& batchActions = DigikamApp::getinstance()->menuBatchActions();
        if(!batchActions.isEmpty())
        {
            foreach(QAction *action, batchActions)
            {
                menuKIPIBatch.addAction(action);
            }
            popmenu.addMenu(&menuKIPIBatch);
        }

        if(!albumActions.isEmpty() || !batchActions.isEmpty())
        {
            popmenu.addSeparator();
        }

        if(AlbumSettings::instance()->getUseTrash())
        {
            deleteAction = popmenu.addAction(SmallIcon("user-trash"), i18n("Move Album to Trash"));
        }
        else
        {
            deleteAction = popmenu.addAction(SmallIcon("editshred"), i18n("Delete Album"));
        }
    }

    QAction *choice = popmenu.exec(QCursor::pos());
    if (choice)
    {
        if (choice == newAction)
        {
            albumNew(item);
        }
        else if (choice == propertiesAction)
        {
            albumEdit(item);
        }
        else if (choice == resetIconAction)
        {
            QString err;
            AlbumManager::instance()->updatePAlbumIcon(item->getAlbum(), 0, err);
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
        DWarning() << "AlbumFolderView: Couldn't get Album Settings" << endl;
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
        parent = item->getAlbum();

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
    QString     collection;
    QDate       date;
    QStringList albumCollections;

    if(!AlbumPropsEdit::createNew(parent, title, comments, date, collection,
                                  albumCollections))
        return;

    QStringList oldAlbumCollections(AlbumSettings::instance()->getAlbumCollectionNames());
    if(albumCollections != oldAlbumCollections)
    {
        AlbumSettings::instance()->setAlbumCollectionNames(albumCollections);
        resort();
    }

    QString errMsg;
    PAlbum* album = d->albumMan->createPAlbum(parent, albumRootPath, title, comments,
                                              date, collection, errMsg);
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
    PAlbum *album = item->getAlbum();

    if(!album || album->isRoot())
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
        list.append(album->kurl());
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
    PAlbum *album = item->getAlbum();

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
    PAlbum *album = item->getAlbum();

    if (!album)
        return;

    QString     oldTitle(album->title());
    QString     oldComments(album->caption());
    QString     oldCollection(album->collection());
    QDate       oldDate(album->date());
    QStringList oldAlbumCollections(AlbumSettings::instance()->getAlbumCollectionNames());

    QString     title, comments, collection;
    QDate       date;
    QStringList albumCollections;

    if(AlbumPropsEdit::editProps(album, title, comments, date,
                                 collection, albumCollections))
    {
        if(comments != oldComments)
            album->setCaption(comments);

        if(date != oldDate && date.isValid())
            album->setDate(date);

        if(collection != oldCollection)
            album->setCollection(collection);

        AlbumSettings::instance()->setAlbumCollectionNames(albumCollections);
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

Q3DragObject* AlbumFolderView::dragObject()
{
    AlbumFolderViewItem *item = dynamic_cast<AlbumFolderViewItem*>(dragItem());
    if(!item)
        return 0;

    PAlbum *album = item->getAlbum();
    if(album->isRoot())
        return 0;

    AlbumDrag *a = new AlbumDrag(album->kurl(), album->id(), this);
    if(!a)
        return 0;
    a->setPixmap(*item->pixmap(0));

    return a;
}

bool AlbumFolderView::acceptDrop(const QDropEvent *e) const
{
    QPoint vp = contentsToViewport(e->pos());
    AlbumFolderViewItem *itemDrop = dynamic_cast<AlbumFolderViewItem*>(itemAt(vp));
    AlbumFolderViewItem *itemDrag = dynamic_cast<AlbumFolderViewItem*>(dragItem());

    if(AlbumDrag::canDecode(e))
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
                if(itemDrag && itemDrag->getAlbum()->isAncestorOf(itemDrop->getAlbum()))
                    return false;
    
                return true;
            }
            case (AlbumSettings::ByCollection):
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

    if (itemDrop && itemDrop->isGroupItem())
    {
        // do not allow drop on a group item
        return false;
    }

    if(ItemDrag::canDecode(e))
    {
        return true;
    }

    if (CameraItemListDrag::canDecode(e))
    {
        return true;
    }

    if(Q3UriDrag::canDecode(e))
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

    if(AlbumDrag::canDecode(e))
    {
        AlbumFolderViewItem *itemDrag = dynamic_cast<AlbumFolderViewItem*>(dragItem());
        if(!itemDrag)
            return;

        if (AlbumSettings::instance()->getAlbumSortOrder()
            == AlbumSettings::ByFolder)
        {
            // TODO: Copy?
            KMenu popMenu(this);
            popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
            QAction *moveAction = popMenu.addAction(SmallIcon("goto"), i18n("&Move Here"));
            popMenu.addSeparator();
            popMenu.addAction(SmallIcon("cancel"), i18n("C&ancel"));
            popMenu.setMouseTracking(true);
            QAction *choice = popMenu.exec(QCursor::pos());

            if(choice == moveAction)
            {
                PAlbum *album = itemDrag->getAlbum();
                PAlbum *destAlbum;
                if(!itemDrop)
                {
                    // move dragItem to the root
                    destAlbum = d->albumMan->findPAlbum(0);
                }
                else
                {
                    // move dragItem below dropItem
                    destAlbum = itemDrop->getAlbum();
                }
                KIO::Job* job = DIO::move(album->kurl(), destAlbum->kurl());
                connect(job, SIGNAL(result(KJob*)),
                        this, SLOT(slotDIOResult(KJob*)));
            }
        }
        else if (AlbumSettings::instance()->getAlbumSortOrder()
                 == AlbumSettings::ByCollection)
        {
            if (!itemDrop)
                return;

            if (itemDrop->isGroupItem())
            {
                PAlbum *album = itemDrag->getAlbum();
                if (!album)
                    return;

                album->setCollection(itemDrop->text(0));
                resort();
            }
        }

        return;
    }

    if (ItemDrag::canDecode(e))
    {
        if (!itemDrop)
            return;

        PAlbum *destAlbum = itemDrop->getAlbum();
        PAlbum *srcAlbum;

        KUrl::List       urls;
        KUrl::List       kioURLs;
        Q3ValueList<int> albumIDs;
        Q3ValueList<int> imageIDs;

        if (!ItemDrag::decode(e, urls, kioURLs, albumIDs, imageIDs))
            return;

        if (urls.isEmpty() || kioURLs.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
            return;

        // all the albumids will be the same
        int albumID = albumIDs.first();
        srcAlbum    = d->albumMan->findPAlbum(albumID);
        if (!srcAlbum)
        {
            DWarning() << "Could not find drag source album"
                    << endl;
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
                popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
                QAction *setAction = popMenu.addAction(i18n("Set as Album Thumbnail"));
                popMenu.addSeparator();
                popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
                popMenu.setMouseTracking(true);
                QAction *choice = popMenu.exec(QCursor::pos());
                set = (setAction == choice);
            }

            if(set)
            {
                QString errMsg;
                AlbumManager::instance()->updatePAlbumIcon(destAlbum, imageIDs.first(), errMsg);
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
            QAction *moveAction = popMenu.addAction(SmallIcon("footprint"), i18n("&Move Here"));
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
            KIO::Job* job = DIO::move(kioURLs, destAlbum->kurl());
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(slotDIOResult(KJob*)));
        }
        else if (copy)
        {
            KIO::Job* job = DIO::copy(kioURLs, destAlbum->kurl());
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(slotDIOResult(KJob*)));
        }
        else if (setThumbnail)
        {
            QString errMsg;
            AlbumManager::instance()->updatePAlbumIcon(destAlbum, imageIDs.first(), errMsg);
        }

        return;
    }

    // -- DnD from Camera GUI ------------------------------------------------

    if(CameraItemListDrag::canDecode(e))
    {
        Album *album = dynamic_cast<Album*>(itemDrop->getAlbum());
        if (!album) return;
        
        CameraUI *ui = dynamic_cast<CameraUI*>(e->source());
        if (ui)
        {
            KMenu popMenu(this);
            popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
            QAction *downAction    = popMenu.addAction(SmallIcon("file-export"), 
                                                       i18n("Download from camera"));
            QAction *downDelAction = popMenu.addAction(SmallIcon("file-export"), 
                                                       i18n("Download && Delete from camera"));
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

    if(Q3UriDrag::canDecode(e))               
    {
        PAlbum* destAlbum = 0;

        if (itemDrop)
            destAlbum = itemDrop->getAlbum();
        else
            destAlbum = d->albumMan->findPAlbum(0);

        // B.K.O #119205: do not handle root album.
        if (destAlbum->isRoot())
            return;

        KUrl destURL(destAlbum->kurl());

        KUrl::List srcURLs = KUrl::List::fromMimeData( e->mimeData() );

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
            QAction *moveAction = popMenu.addAction( SmallIcon("footprint"), i18n("&Move Here"));
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
            KIO::Job* job = DIO::move(srcURLs, destAlbum->kurl());
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(slotDIOResult(KJob*)));
        }
        else if (copy)
        {
            KIO::Job* job = DIO::copy(srcURLs, destAlbum->kurl());
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
        case(AlbumSettings::ByCollection):
        {
            return findParentByCollection(album, failed);
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
    AlbumFolderViewItem* parent =
        (AlbumFolderViewItem*) album->parent()->extraData(this);

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
            for (Q3ValueList<AlbumFolderViewItem*>::iterator it=d->groupItems.begin();
                 it != d->groupItems.end(); ++it)
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

AlbumFolderViewItem* AlbumFolderView::findParentByCollection(PAlbum* album, bool& failed)
{
    QStringList collectionList = AlbumSettings::instance()->getAlbumCollectionNames();
    QString collection = album->collection();

    if (collection.isEmpty() || !collectionList.contains(collection))
        collection = i18n("Uncategorized Albums");

    AlbumFolderViewItem* parent = 0;

    for (Q3ValueList<AlbumFolderViewItem*>::iterator it=d->groupItems.begin();
         it != d->groupItems.end(); ++it)
    {
        AlbumFolderViewItem* groupItem = *it;
        if (groupItem->text(0) == collection)
        {
            parent = groupItem;
            break;
        }
    }

    // Need to create a new parent item
    if (!parent)
    {
        parent = new AlbumFolderViewItem(firstChild(), collection, 0, 0);
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

    for (Q3ValueList<AlbumFolderViewItem*>::iterator it=d->groupItems.begin();
         it != d->groupItems.end(); ++it)
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

    AlbumList pList(AlbumManager::instance()->allPAlbums());
    for (AlbumList::iterator it = pList.begin(); it != pList.end(); ++it)
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

    PAlbum* album = folderItem->getAlbum();
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
    Q3ValueList<AlbumFolderViewItem*> deleteItems;

    for (Q3ValueList<AlbumFolderViewItem*>::iterator it=d->groupItems.begin();
         it != d->groupItems.end(); ++it)
    {
        AlbumFolderViewItem* groupItem = *it;

        if (!groupItem->firstChild())
        {
            deleteItems.append(groupItem);
        }
    }

    for (Q3ValueList<AlbumFolderViewItem*>::iterator it=deleteItems.begin();
         it != deleteItems.end(); ++it)
    {
        d->groupItems.remove(*it);
        delete *it;
    }
}

}  // namespace Digikam
