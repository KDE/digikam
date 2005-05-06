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

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kapplication.h>

#include "albumfolderview.h"
#include "album.h"
#include "albummanager.h"

//-----------------------------------------------------------------------------
// AlbumFolderViewItem
//-----------------------------------------------------------------------------

class AlbumFolderViewItem : public QListViewItem
{
public:
    AlbumFolderViewItem(QListView *parent, PAlbum *album);
    AlbumFolderViewItem(QListViewItem *parent, PAlbum *album);    

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

//-----------------------------------------------------------------------------
// AlbumFolderViewPriv
//-----------------------------------------------------------------------------

class AlbumFolderViewPriv
{
public:
    QIntDict<AlbumFolderViewItem>    dict;
};

//-----------------------------------------------------------------------------
// AlbumFolderView
//-----------------------------------------------------------------------------

AlbumFolderView::AlbumFolderView(QWidget *parent)
    : QListView(parent)
{
    d = new AlbumFolderViewPriv();

    addColumn(i18n("My Albums"));
    setResizeMode(QListView::LastColumn);
    setRootIsDecorated(true);
    
    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            SLOT(slotAlbumAdded(Album*)));
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
}

#include "albumfolderview.moc"
