/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-26
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qstring.h>

#include <kdirlister.h>
#include <kdebug.h>

#include "album.h"
#include "albummanager.h"
#include "albumdb.h"
#include "albumsettings.h"

#include "albumlister.h"

extern "C"
{
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
}

class AlbumListerPriv
{
public:

    KDirLister* dirLister;

    Album*      currAlbum;
    AlbumDB*    db;
};
    
AlbumLister::AlbumLister()
{
    d = new AlbumListerPriv;
    d->dirLister = new KDirLister;
    d->currAlbum = 0;
}

AlbumLister::~AlbumLister()
{
    delete d->dirLister;
    delete d;
}

void AlbumLister::openAlbum(Album *album)
{
    d->dirLister->stop();

    d->dirLister->disconnect(this);
    
    d->currAlbum = album;
    if (!album)
        return;

    if (album->type() == Album::PHYSICAL)
    {
        connect(d->dirLister, SIGNAL(newItems(const KFileItemList&)),
                SLOT(slotNewPhyItems(const KFileItemList&)));
        connect(d->dirLister, SIGNAL(deleteItem(KFileItem*)),
                SLOT(slotDeleteItem(KFileItem*)) );
        connect(d->dirLister, SIGNAL(clear()),
                SLOT(slotClear()));
        connect(d->dirLister, SIGNAL(completed()),
                SIGNAL(signalCompleted()));
        connect(d->dirLister, SIGNAL(refreshItems(const KFileItemList&)),
                SIGNAL(signalRefreshItems(const KFileItemList&)));

        PAlbum *a = static_cast<PAlbum*>(album);
        d->dirLister->openURL(a->getKURL(), false, true);
    }
    else if (album->type() == Album::TAG)
    {
        //TODO: if the album library path has changed: kill the tags kioslave
        // by pumping a metadata to it, causing it to reload.
        
        connect(d->dirLister, SIGNAL(newItems(const KFileItemList&)),
                SLOT(slotNewTagItems(const KFileItemList&)));
        connect(d->dirLister, SIGNAL(deleteItem(KFileItem*)),
                SLOT(slotDeleteItem(KFileItem*)) );
        connect(d->dirLister, SIGNAL(clear()),
                SLOT(slotClear()));
        connect(d->dirLister, SIGNAL(completed()),
                SIGNAL(signalCompleted()));
        connect(d->dirLister, SIGNAL(refreshItems(const KFileItemList&)),
                SIGNAL(signalRefreshItems(const KFileItemList&)));

        TAlbum *a = static_cast<TAlbum*>(album);
        KURL url(a->getKURL());
        if (AlbumSettings::instance()->getRecurseTags())
            url.setQuery("?recurse=yes");
        d->dirLister->openURL(url, false, true);
    }
    else
    {
        emit signalClear();
    }
}

void AlbumLister::updateDirectory()
{
    if (!d->currAlbum)
        return;

    if (d->currAlbum->type() == Album::PHYSICAL)
    {
        PAlbum *a = static_cast<PAlbum*>(d->currAlbum);
        d->dirLister->updateDirectory(a->getKURL());
    }
    else if (d->currAlbum->type() == Album::TAG)
    {
        TAlbum *a = static_cast<TAlbum*>(d->currAlbum);
        KURL url(a->getKURL());
        if (AlbumSettings::instance()->getRecurseTags())
            url.setQuery("?recurse=yes");
        d->dirLister->updateDirectory(url);
    }        
}

PAlbum* AlbumLister::findParentAlbum(const KFileItem *item) const
{
    if (!item)
        return 0;
    
    return (PAlbum*)item->extraData(this);
}

void AlbumLister::stop()
{
    d->dirLister->stop();
}

void AlbumLister::setNameFilter(const QString& nameFilter)
{
    d->dirLister->setNameFilter(nameFilter);    
}

void AlbumLister::slotNewPhyItems(const KFileItemList& items)
{
    if (d->currAlbum && d->currAlbum->type() == Album::PHYSICAL)
    {
        PAlbum* album = dynamic_cast<PAlbum*>(d->currAlbum);
        KFileItem* item;
        for (KFileItemListIterator it(items); (item = it.current()); ++it)
        {
            item->setExtraData(this, album);
        }
    }

    emit signalNewItems(items);
}

void AlbumLister::slotNewTagItems(const KFileItemList& items)
{
    KFileItemList filteredItems;
    
    KIO::UDSEntry entry;
    PAlbum*       album;
    int           id;
    KFileItem*    item;

    AlbumManager* man = AlbumManager::instance();
    
    for (KFileItemListIterator it(items); (item = it.current()); ++it)
    {
        if (item->isDir())
            continue;
        
        album = 0;
        
        entry = item->entry();
        for( KIO::UDSEntry::ConstIterator it = entry.begin();
             it != entry.end(); it++ )
        {
            if ((*it).m_uds == KIO::UDS_XML_PROPERTIES)
            {
                id = (*it).m_str.toInt();
                album = man->findPAlbum(id);
                break;
            }
        }

        if (!album)
        {
            kdWarning() << k_funcinfo << "Failed to retrieve dirid from kioslave for "
                        << item->url().prettyURL() << endl;
            continue;
        }

        filteredItems.append(item);
        item->setExtraData(this, album);
    }

    emit signalNewItems(filteredItems);
}

void AlbumLister::slotDeleteItem(KFileItem *item)
{
    emit signalDeleteItem(item);
    item->removeExtraData(this);
}

void AlbumLister::slotClear()
{
    emit signalClear();
}

#include "albumlister.moc"

