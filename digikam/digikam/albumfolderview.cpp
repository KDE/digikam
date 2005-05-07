/* ============================================================
 * File  : albumfolderview.cpp
 * Author: Jörn Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-05-06
 * Copyright 2005 by Jörn Ahrens
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

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kapplication.h>

#include "albumfolderview.h"
#include "album.h"
#include "albummanager.h"
#include "albummanager.h"
#include "thumbnailjob.h"
#include "thumbnailsize.h"

//-----------------------------------------------------------------------------
// AlbumFolderViewItem
//-----------------------------------------------------------------------------

class AlbumFolderViewItem : public QListViewItem
{
public:
    AlbumFolderViewItem(QListView *parent, PAlbum *album);
    AlbumFolderViewItem(QListViewItem *parent, PAlbum *album);    
    
    PAlbum* getAlbum();
    
private:
    PAlbum      *m_album;
};

AlbumFolderViewItem::AlbumFolderViewItem(QListView *parent, PAlbum *album)
    : QListViewItem(parent, album->getTitle())
{
    m_album = album;
}

AlbumFolderViewItem::AlbumFolderViewItem(QListViewItem *parent, PAlbum *album)
    : QListViewItem(parent, album->getTitle())
{
    m_album = album;
}

PAlbum* AlbumFolderViewItem::getAlbum()
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
    : QListView(parent)
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
    
    connect(this, SIGNAL(selectionChanged(QListViewItem *)),
            this, SLOT(slotSelectionChanged(QListViewItem *)));
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
    if(palbum->getParent()->isRoot())
    {
        item = new AlbumFolderViewItem(this, palbum);
        d->dict.insert(palbum->getID(), item);
    }
    else
    {
        AlbumFolderViewItem *parent = d->dict.find(palbum->getParent()->getID());
        if (!parent)
        {
            kdWarning() << k_funcinfo << " Failed to find parent for Tag "
                        << palbum->getURL() << endl;
            return;
        }
        item = new AlbumFolderViewItem(parent, palbum);
        d->dict.insert(palbum->getID(), item);
    }
    
    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();    
    item->setPixmap(0, iconLoader->loadIcon("folder", KIcon::NoGroup,
                    32, KIcon::DefaultState, 0, true));
    
    setAlbumThumbnail(palbum);
}

void AlbumFolderView::setAlbumThumbnail(PAlbum *album)
{
    if(!album)
        return;
    
    AlbumFolderViewItem *item = d->dict.find(album->getID());
    
    if(!item)
        return;
    
    if(!album->getIcon().isEmpty())
    {
        if(!d->iconThumbJob)
        {
            d->iconThumbJob = new ThumbnailJob(album->getIconKURL(),
                                               (int)ThumbnailSize::Tiny,
                                               true);
            connect(d->iconThumbJob,
                    SIGNAL(signalThumbnailMetaInfo(const KURL&,
                           const QPixmap&,
                           const KFileMetaInfo*)),
                    this,
                    SLOT(slotGotThumbnailFromIcon(const KURL&,
                         const QPixmap&,
                         const KFileMetaInfo*)));
            /*connect(d->iconThumbJob,
                    SIGNAL(signalFailed(const KURL&)),
                    SLOT(slotThumbnailLost(const KURL&)));*/
        }
        else
        {
            d->iconThumbJob->addItem(album->getIconKURL());
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
        const QPixmap& thumbnail,
        const KFileMetaInfo*)
{
    PAlbum* album = d->albumMan->findPAlbum(url.directory());

    if (!album)
        return;

    AlbumFolderViewItem *item = d->dict.find(album->getID());
    
    if(!item)
        return;

    item->setPixmap(0, thumbnail);
}

void AlbumFolderView::slotSelectionChanged(QListViewItem *item)
{
    if(!item)
    {
        d->albumMan->setCurrentAlbum(0);
        return;
    }
    
    AlbumFolderViewItem *albumitem = dynamic_cast<AlbumFolderViewItem*>(item);
    if(!albumitem)
    {
        d->albumMan->setCurrentAlbum(0);
        return;        
    }
    
    d->albumMan->setCurrentAlbum(albumitem->getAlbum());
}

#include "albumfolderview.moc"


