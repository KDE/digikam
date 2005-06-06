/* ============================================================
 * File  : albumfolderview.cpp
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
 * ============================================================ */
 
#include <qintdict.h>
#include <qpixmap.h>
#include <qguardedptr.h>
#include <qdir.h>
#include <qpopupmenu.h>
#include <qcursor.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kfiledialog.h>

#include "digikamapp.h"
#include "album.h"
#include "albumfolderview.h"
#include "albumpropsedit.h"
#include "album.h"
#include "albummanager.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "thumbnailjob.h"
#include "thumbnailsize.h"
#include "albumpropsedit.h"
#include "folderitem.h"
#include "dio.h"
#include "dragobjects.h"

//-----------------------------------------------------------------------------
// AlbumFolderViewItem
//-----------------------------------------------------------------------------

class AlbumFolderViewItem : public FolderItem
{
public:
    AlbumFolderViewItem(QListView *parent, PAlbum *album);
    AlbumFolderViewItem(QListViewItem *parent, PAlbum *album);    
    
    PAlbum* getAlbum() const;
    
private:
    PAlbum      *m_album;
};

AlbumFolderViewItem::AlbumFolderViewItem(QListView *parent, PAlbum *album)
    : FolderItem(parent, album->title())
{
    setDragEnabled(true);
    m_album = album;
}

AlbumFolderViewItem::AlbumFolderViewItem(QListViewItem *parent, PAlbum *album)
    : FolderItem(parent, album->title())
{
    setDragEnabled(true);
    m_album = album;
}

PAlbum* AlbumFolderViewItem::getAlbum() const
{
    return m_album;
}

//-----------------------------------------------------------------------------
// AlbumFolderViewPriv
//-----------------------------------------------------------------------------

class AlbumFolderViewPriv
{
public:
    AlbumManager                     *albumMan;
    QIntDict<AlbumFolderViewItem>    dict;
    ThumbnailJob                     *iconThumbJob;
};

//-----------------------------------------------------------------------------
// AlbumFolderView
//-----------------------------------------------------------------------------

AlbumFolderView::AlbumFolderView(QWidget *parent)
    : FolderView(parent)
{
    d = new AlbumFolderViewPriv();
    
    d->albumMan = AlbumManager::instance();
    d->iconThumbJob = 0;
    
    addColumn(i18n("My Albums"));
    setResizeMode(QListView::LastColumn);
    setRootIsDecorated(true);
    setAllColumnsShowFocus(true);

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);               
    
    connect(d->albumMan, SIGNAL(signalAlbumAdded(Album*)),
            SLOT(slotAlbumAdded(Album*)));
    connect(d->albumMan, SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));    
    connect(d->albumMan, SIGNAL(signalAlbumIconChanged(Album*)),
            this, SLOT(slotAlbumIconChanged(Album*)));
        
    connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
            SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));    
    
    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

AlbumFolderView::~AlbumFolderView()
{
    if(d->iconThumbJob)
        delete d->iconThumbJob;
    
    delete d;
}

void AlbumFolderView::slotAlbumAdded(Album *album)
{
    if(!album || album->isRoot())
        return;
    
    PAlbum *palbum = dynamic_cast<PAlbum*>(album);
    if(!palbum)
        return;
    
    AlbumFolderViewItem *item;
    if(palbum->parent()->isRoot())
    {
        item = new AlbumFolderViewItem(this, palbum);
        d->dict.insert(palbum->id(), item);
    }
    else
    {
        AlbumFolderViewItem *parent = d->dict.find(palbum->parent()->id());
        if(!parent)
        {
            kdWarning() << k_funcinfo << " Failed to find parent for Album "
                        << palbum->url() << endl;
            return;
        }
        item = new AlbumFolderViewItem(parent, palbum);
        d->dict.insert(palbum->id(), item);
    }
    
    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();    
    item->setPixmap(0, iconLoader->loadIcon("folder", KIcon::NoGroup,
                    32, KIcon::DefaultState, 0, true));
    
    setAlbumThumbnail(palbum);
}

void AlbumFolderView::slotNewAlbumCreated(Album* album)
{
    disconnect(d->albumMan, SIGNAL(signalAlbumAdded(Album*)),
               this, SLOT(slotNewAlbumCreated(Album*)));

    if(!album)
        return;
    
    PAlbum* palbum = dynamic_cast<PAlbum*>(album);
    if(!palbum)
        return;

    AlbumFolderViewItem *item = d->dict.find(album->id());
    if(!item)
        return;

    ensureItemVisible(item);
    setSelected(item, true);
}

void AlbumFolderView::slotAlbumDeleted(Album *album)
{
    if(!album)
        return;

    if(album->type() == Album::PHYSICAL)
    {
        PAlbum* palbum = dynamic_cast<PAlbum*>(album);
        if(!palbum)
            return;
        
        if(!palbum->icon().isEmpty() && !d->iconThumbJob)
            d->iconThumbJob->removeItem(palbum->icon());

        AlbumFolderViewItem* item = d->dict.find(palbum->id());
        if(item)
        {
            AlbumFolderViewItem *itemParent = 
                    dynamic_cast<AlbumFolderViewItem*>(item->parent());
            
            if(itemParent)
                itemParent->takeItem(item);
            else
                takeItem(item);

            delete item;
          //TODO    clearEmptyGroupItems();
        }
    }
}

void AlbumFolderView::setAlbumThumbnail(PAlbum *album)
{
    if(!album)
        return;
    
    AlbumFolderViewItem *item = d->dict.find(album->id());
    
    if(!item)
        return;
    
    if(!album->icon().isEmpty())
    {
        if(!d->iconThumbJob)
        {
            d->iconThumbJob = new ThumbnailJob(album->iconKURL(),
                                               (int)ThumbnailSize::Tiny,
                                               true);
            connect(d->iconThumbJob, 
                    SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
                    this,
                    SLOT(slotGotThumbnailFromIcon(const KURL&, const QPixmap&)));
            connect(d->iconThumbJob,
                    SIGNAL(signalFailed(const KURL&)),
                    SLOT(slotThumbnailLost(const KURL&)));
        }
        else
        {
            d->iconThumbJob->addItem(album->iconKURL());
        }
    }
    else
    {
        KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();
        item->setPixmap(0, iconLoader->loadIcon("folder", KIcon::NoGroup,
                                                32, KIcon::DefaultState,
                                                0, true));
    }
}

void AlbumFolderView::slotGotThumbnailFromIcon(const KURL& url,
                                               const QPixmap& thumbnail)
{
    PAlbum* album = d->albumMan->findPAlbum(url.directory());

    if(!album)
        return;

    AlbumFolderViewItem *item = d->dict.find(album->id());
    
    if(!item)
        return;

    item->setPixmap(0, thumbnail);
}

void AlbumFolderView::slotThumbnailLost(const KURL &url)
{
    PAlbum *album = AlbumManager::instance()->findPAlbum(url.directory());
    if(!album)
        return;
    
    AlbumFolderViewItem *item = d->dict.find(album->id());
    if(item)
    {
        KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();
        item->setPixmap(0, iconLoader->loadIcon("folder", KIcon::NoGroup, 32,
                                                KIcon::DefaultState, 0, true));

        QString errMsg;
        d->albumMan->updatePAlbumIcon(album, "", false, errMsg);
    }
}

void AlbumFolderView::slotAlbumIconChanged(Album* album)
{
    if(!album || album->type() != Album::PHYSICAL)
        return;

    AlbumFolderViewItem *item = d->dict.find(album->id());

    if(item)
        setAlbumThumbnail((PAlbum*)album);
}

void AlbumFolderView::slotSelectionChanged()
{
    if(!active())
        return;
    
    QListViewItem* selItem = 0;
    QListViewItemIterator it(this);
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

void AlbumFolderView::slotContextMenu(QListViewItem *listitem, const QPoint &, int)
{
    QPopupMenu popmenu(this);
    KActionMenu menuImport(i18n("Import"));
    KActionMenu menuKIPIBatch(i18n("Batch Processes"));        
        
    popmenu.insertItem(SmallIcon("album"), i18n("New Album..."), 10);
    
    AlbumFolderViewItem *item = dynamic_cast<AlbumFolderViewItem*>(listitem);
    if(item)
    {
        popmenu.insertItem(SmallIcon("pencil"), i18n("Edit Album Properties..."), 11);
        
        popmenu.insertSeparator();
        
        KAction *action;
        // Add KIPI Albums plugins Actions
        const QPtrList<KAction>& albumActions =
                DigikamApp::getinstance()->menuAlbumActions();
        if(!albumActions.isEmpty())
        {
            QPtrListIterator<KAction> it(albumActions);
            while((action = it.current()))
            {
                action->plug(&popmenu);
                ++it;
            }
        }
        
        // Add All Import Actions
        const QPtrList<KAction> importActions =
                DigikamApp::getinstance()->menuImportActions();
        if(!importActions.isEmpty())
        {
            QPtrListIterator<KAction> it3(importActions);
            while((action = it3.current()))
            {
                menuImport.insert(action);
                ++it3;
            }
            menuImport.plug(&popmenu);
        }
        
        // Add KIPI Batch processes plugins Actions
        const QPtrList<KAction>& batchActions =
                DigikamApp::getinstance()->menuBatchActions();
        if(!batchActions.isEmpty())
        {
            QPtrListIterator<KAction> it2(batchActions);
            while((action = it2.current()))
            {
                menuKIPIBatch.insert(action);
                ++it2;
            }
            menuKIPIBatch.plug(&popmenu);
        }
        
        if(!albumActions.isEmpty() || !batchActions.isEmpty() ||
           !importActions.isEmpty())
        {
            popmenu.insertSeparator();
        }
        
        if(AlbumSettings::instance()->getUseTrash())
        {
            popmenu.insertItem(SmallIcon("edittrash"),
                               i18n("Move Album to Trash"), 12);
        }
        else
        {
            popmenu.insertItem(SmallIcon("editdelete"),
                               i18n("Delete Album"), 12);
        }
    }

    switch(popmenu.exec((QCursor::pos())))
    {
        case 10:
        {
            albumNew(item);
            break;
        }
        case 11:
        {
            albumEdit(item);
            break;
        }
        case 12:
        {
            albumDelete(item);
            break;
        }
        default:
            break;
    }
}

void AlbumFolderView::albumNew(AlbumFolderViewItem *item)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if(!settings)
    {
        kdWarning() << "AlbumFolderView_Deprecated: Couldn't get Album Settings" << endl;
        return;
    }

    QDir libraryDir(settings->getAlbumLibraryPath());
    if(!libraryDir.exists())
    {
        KMessageBox::error(0,
                           i18n("The Albums Library has not been set correctly.\n"
                                "Select \"Configure Digikam\" from the Settings "
                                "menu and choose a folder to use for the Albums "
                                "Library."));
        return;
    }

    PAlbum *parent;

    if(!item)
        parent = d->albumMan->findPAlbum(0);
    else
        parent = item->getAlbum();

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
// TODO:       resort();
    }
    
    QString errMsg;
    if(d->albumMan->createPAlbum(parent, title, comments, date, collection, errMsg))
    {
        connect(d->albumMan, SIGNAL(signalAlbumAdded(Album*)),
                this, SLOT(slotNewAlbumCreated(Album*)));
    }
    else
    {
        KMessageBox::error(0, errMsg);
    }
}

void AlbumFolderView::albumDelete(AlbumFolderViewItem *item)
{
    PAlbum *album = item->getAlbum();
    
    if(!album || album->isRoot())
        return;

    // find number of subalbums
    int children = 0;
    AlbumIterator it(album);
    while(it.current())
    {
        children++;
        ++it;
    }

    int result = KMessageBox::No;
    AlbumSettings* settings = AlbumSettings::instance();
    if (children)
    {
        if(settings->getUseTrash())
        {
            result = KMessageBox::warningYesNo(this,
                        i18n("Album '%1' has %2 subalbums. "
                             "Moving this to trash will also move the "
                             "subalbums to trash. "
                             "Are you sure you want to continue?")
                             .arg(album->title())
                             .arg(children));
        }
        else
        {
            result = KMessageBox::warningYesNo(this,
                        i18n("Album '%1' has %2 subalbums. "
                             "Deleting this will also delete "
                             "the subalbums. "
                             "Are you sure you want to continue?")
                             .arg(album->title())
                             .arg(children));
        }
    }
    else
    {
        result = KMessageBox::questionYesNo(this, settings->getUseTrash() ?
                i18n("Move album '%1' to trash?").arg(album->title()) :
                i18n("Delete album '%1' from disk?").arg(album->title()));
    }

    if(result == KMessageBox::Yes)
    {
        // TODO: currently trash kioslave can handle only full paths.
        // pass full folder path to the trashing job
        KURL u;
        u.setProtocol("file");
        u.setPath(album->folderPath());
        KIO::Job* job = DIO::del(u);
        connect(job, SIGNAL(result(KIO::Job *)),
                this, SLOT(slotDIOResult(KIO::Job *)));
    }
}

void AlbumFolderView::slotDIOResult(KIO::Job* job)
{
    if (job->error())
        job->showErrorDialog(this);
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
//TODO        resort();

    // Do this last : so that if anything else changed we can
    // successfully save to the db with the old name

        if(title != oldTitle)
        {
            QString errMsg;
            if (!d->albumMan->renamePAlbum(album, title, errMsg))
                KMessageBox::error(0, errMsg);
        }

        emit signalAlbumModified();
    }
    
}

QDragObject* AlbumFolderView::dragObject()
{
    AlbumFolderViewItem *item = dynamic_cast<AlbumFolderViewItem*>(dragItem());
    if(!item)
        return 0;
    
    PAlbum *album = item->getAlbum();
    AlbumDrag *a = new AlbumDrag(album->kurl(), album->id(), this);
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

    return false;
}

void AlbumFolderView::contentsDropEvent(QDropEvent *e)
{
    FolderView::contentsDropEvent(e);

    if(!acceptDrop(e))
        return;

    QPoint vp = contentsToViewport(e->pos());
    AlbumFolderViewItem *itemDrop = dynamic_cast<AlbumFolderViewItem*>(itemAt(vp));
    AlbumFolderViewItem *itemDrag = dynamic_cast<AlbumFolderViewItem*>(dragItem());
    if(!itemDrag)
        return;

    if(AlbumDrag::canDecode(e))
    {
        QPopupMenu popMenu(this);
        popMenu.insertItem(SmallIcon("goto"), i18n("&Move Here"), 10);
        popMenu.insertSeparator(-1);
        popMenu.insertItem(SmallIcon("cancel"), i18n("C&ancel"), 20);
        popMenu.setMouseTracking(true);
        int id = popMenu.exec(QCursor::pos());

        if(id == 10)
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
            connect(job, SIGNAL(result(KIO::Job*)),
                    SLOT(slotDIOResult(KIO::Job*)));
        }
    }
}

void AlbumFolderView::albumImportFolder()
{
    AlbumSettings* settings = AlbumSettings::instance();
    QDir libraryDir(settings->getAlbumLibraryPath());
    if(!libraryDir.exists())
    {
        KMessageBox::error(0,
                           i18n("The Albums Library has not been set correctly.\n"
                                "Select \"Configure Digikam\" from the Settings "
                                "menu and choose a folder to use for the Albums "
                                "Library."));
        return;
    }

    PAlbum* parent = 0;
    if(selectedItem())
    {
        AlbumFolderViewItem *folderItem =
                dynamic_cast<AlbumFolderViewItem*>(selectedItem());
        Album *album = folderItem->getAlbum();
        if (album && album->type() == Album::PHYSICAL)
        {
            parent = dynamic_cast<PAlbum*>(album);
        }
    }
    if(!parent)
        parent = dynamic_cast<PAlbum*>(d->albumMan->findPAlbum(0));

    QString libraryPath = parent->folderPath();

    KFileDialog dlg(QString::null, "inode/directory", this, "importFolder", true);
    dlg.setMode(KFile::Directory |  KFile::Files);
    if(dlg.exec() != QDialog::Accepted)
        return;

    KURL::List urls = dlg.selectedURLs();
    if(urls.empty())
        return;

    KIO::Job* job = DIO::copy(urls, parent->kurl());
    connect(job, SIGNAL(result(KIO::Job *)),
            this, SLOT(slotDIOResult(KIO::Job *)));
}


#include "albumfolderview.moc"


