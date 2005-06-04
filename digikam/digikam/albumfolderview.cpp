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

#include "albumfolderview.h"
#include "album.h"
#include "albummanager.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "thumbnailjob.h"
#include "thumbnailsize.h"
#include "albumpropsedit.h"
#include "folderitem.h"

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
    m_album = album;
}

AlbumFolderViewItem::AlbumFolderViewItem(QListViewItem *parent, PAlbum *album)
    : FolderItem(parent, album->title())
{
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
               
    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            SLOT(slotAlbumAdded(Album*)));
    
    connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
            SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));    
    
    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

AlbumFolderView::~AlbumFolderView()
{
    if (d->iconThumbJob)
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
        if (!parent)
        {
            kdWarning() << k_funcinfo << " Failed to find parent for Tag "
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

    if (!album)
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
                    SIGNAL(signalThumbnail(const KURL&,
                                           const QPixmap&)),
                    this,
                    SLOT(slotGotThumbnailFromIcon(const KURL&,
                                                  const QPixmap&)));
            /*connect(d->iconThumbJob,
                    SIGNAL(signalFailed(const KURL&)),
                    SLOT(slotThumbnailLost(const KURL&)));*/
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

    if (!album)
        return;

    AlbumFolderViewItem *item = d->dict.find(album->id());
    
    if(!item)
        return;

    item->setPixmap(0, thumbnail);
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

void AlbumFolderView::slotContextMenu(QListViewItem *item, const QPoint &, int)
{
    QPopupMenu popmenu(this);

    AlbumFolderViewItem *album = dynamic_cast<AlbumFolderViewItem*>(item);

    popmenu.insertItem(SmallIcon("album"), i18n("New Album..."), 10);

    if(album)
    {
//        popmenu.insertItem(SmallIcon("pencil"), i18n("Edit Tag Properties..."), 11);
//        popmenu.insertItem(SmallIcon("edittrash"), i18n("Delete Tag"), 12);
    }

    switch(popmenu.exec((QCursor::pos())))
    {
        case 10:
        {
            albumNew(album);
            break;
        }
        case 11:
        {
//            tagEdit(album);
            break;
        }
        case 12:
        {
//            tagDelete(album);
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

void AlbumFolderView::albumEdit(AlbumFolderViewItem* /*item*/)
{
}

void AlbumFolderView::albumDelete(AlbumFolderViewItem* /*item*/)
{
}

#include "albumfolderview.moc"


