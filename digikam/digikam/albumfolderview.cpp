/* ============================================================
 * File  : albumfolderview.cpp
 * Author: J�n Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-05-06
 * Copyright 2005 by J�n Ahrens
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

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kglobalsettings.h>
#include <kcursor.h>
#include <kmessagebox.h>

#include "albumfolderview.h"
#include "album.h"
#include "albummanager.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "thumbnailjob.h"
#include "thumbnailsize.h"
#include "albumpropsedit.h"

//-----------------------------------------------------------------------------
// AlbumFolderViewItem
//-----------------------------------------------------------------------------

class AlbumFolderViewItem : public QListViewItem
{
public:
    AlbumFolderViewItem(QListView *parent, PAlbum *album);
    AlbumFolderViewItem(QListViewItem *parent, PAlbum *album);    
    
    PAlbum* getAlbum() const;
    
private:
    PAlbum      *m_album;
};

AlbumFolderViewItem::AlbumFolderViewItem(QListView *parent, PAlbum *album)
    : QListViewItem(parent, album->title())
{
    m_album = album;
}

AlbumFolderViewItem::AlbumFolderViewItem(QListViewItem *parent, PAlbum *album)
    : QListViewItem(parent, album->title())
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
    bool                             active;
};

//-----------------------------------------------------------------------------
// AlbumFolderView
//-----------------------------------------------------------------------------

AlbumFolderView::AlbumFolderView(QWidget *parent)
    : QListView(parent)
{
    d = new AlbumFolderViewPriv();
    
    d->albumMan = AlbumManager::instance();
    d->active   = false;
    d->iconThumbJob = 0;
    
    addColumn(i18n("My Albums"));
    setResizeMode(QListView::LastColumn);
    setRootIsDecorated(true);
    setAllColumnsShowFocus(true);
               
    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            SLOT(slotAlbumAdded(Album*)));
    
    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

AlbumFolderView::~AlbumFolderView()
{
    if (d->iconThumbJob)
        delete d->iconThumbJob;
    
    delete d;
}

void AlbumFolderView::setActive(bool val)
{
    d->active = val;

    if (d->active)
        slotSelectionChanged();
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
    if (!d->active)
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
    
    AlbumFolderViewItem *albumitem = dynamic_cast<AlbumFolderViewItem*>(selItem);
    if(!albumitem)
    {
        d->albumMan->setCurrentAlbum(0);
        return;        
    }
    
    d->albumMan->setCurrentAlbum(albumitem->getAlbum());
}

void AlbumFolderView::contentsMousePressEvent(QMouseEvent *e)
{
    if(!e)
        return;

    if(e->button() == RightButton) {
        contextMenu(e->pos());
        return;
    }
    
    QListView::contentsMousePressEvent(e);
}

void AlbumFolderView::contentsMouseMoveEvent(QMouseEvent *e)
{
    if(!e) 
        return;

    AlbumFolderViewItem *item = dynamic_cast<AlbumFolderViewItem*>(itemAt(e->pos()));

    if(e->state() == NoButton)
    {
        if (KGlobalSettings::changeCursorOverIcon())
        {
            if(item)
                setCursor(KCursor::handCursor());
            else
                unsetCursor();
        }
    }
}

void AlbumFolderView::contextMenu(const QPoint &pos)
{
    QPopupMenu popmenu(this);

    AlbumFolderViewItem *item = dynamic_cast<AlbumFolderViewItem*>(itemAt(pos));

    popmenu.insertItem(SmallIcon("album"), i18n("New Album..."), 10);

    if(item)
    {
//        popmenu.insertItem(SmallIcon("pencil"), i18n("Edit Tag Properties..."), 11);
//        popmenu.insertItem(SmallIcon("edittrash"), i18n("Delete Tag"), 12);
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
//            tagEdit(item);
            break;
        }
        case 12:
        {
//            tagDelete(item);
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

void AlbumFolderView::albumEdit(AlbumFolderViewItem *item)
{
}

void AlbumFolderView::albumDelete(AlbumFolderViewItem *item)
{
}

#include "albumfolderview.moc"


