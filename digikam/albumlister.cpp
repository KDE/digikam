/* ============================================================
 * File  : albumlister.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-26
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
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
#include <qdict.h>
#include <qmap.h>

#include <kdirlister.h>
#include <kdirwatch.h>

#include "album.h"
#include "albummanager.h"
#include "albumdb.h"

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
    KDirWatch*  dirWatch;

    Album*      currAlbum;
    AlbumDB*    db;

    KFileItemList itemList;
    QMap<KFileItem*,QString> comments;
};
    
AlbumLister::AlbumLister()
{
    d = new AlbumListerPriv;
    d->dirLister = new KDirLister;
    d->dirWatch  = 0;
    d->currAlbum = 0;
    d->db = AlbumManager::instance()->albumDB();
    d->itemList.setAutoDelete(true);
    
    connect(d->dirLister, SIGNAL(newItems(const KFileItemList&)),
            SLOT(slotNewItems(const KFileItemList&)));
    connect(d->dirLister, SIGNAL(deleteItem(KFileItem*)),
            SLOT(slotDeleteItem(KFileItem*)) );
    connect(d->dirLister, SIGNAL(clear()),
            SLOT(slotClear()));
    connect(d->dirLister, SIGNAL(completed()),
            SIGNAL(signalCompleted()));
    connect(d->dirLister, SIGNAL(refreshItems(const KFileItemList&)),
            SIGNAL(signalRefreshItems(const KFileItemList&)));
}

AlbumLister::~AlbumLister()
{
    delete d->dirLister;
    if (d->dirWatch)
        delete d->dirWatch;
    delete d;
}

void AlbumLister::openAlbum(Album *album)
{
    d->dirLister->stop();
    if (d->dirWatch) {
        delete d->dirWatch;
        d->dirWatch = 0;
    }

    d->dirLister->disconnect(this);
    
    d->currAlbum = album;
    d->comments.clear();
    
    if (album->type() == Album::PHYSICAL)
    {
        connect(d->dirLister, SIGNAL(newItems(const KFileItemList&)),
                SLOT(slotNewItems(const KFileItemList&)));
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

        struct timeval t1, t2;

        gettimeofday(&t1, 0);
        
        emit signalClear();
        d->itemList.clear();

        d->dirWatch = new KDirWatch;

        QString basePath(AlbumManager::instance()->getLibraryPath() + 
                         QString("/"));
        
        TAlbum* t = static_cast<TAlbum*>(album);
        if (!t->isRoot())
        {

            d->db->beginTransaction();
            
            QStringList     urls;
            QValueList<int> dirIDs;

            d->db->getItemsInTAlbum(t, urls, dirIDs);

            AlbumManager* man = AlbumManager::instance();
            
            QStringList::iterator     itU = urls.begin();
            QValueList<int>::iterator itD = dirIDs.begin();
            while (itU != urls.end())
            {
                KFileItem *fileItem =
                    new KFileItem(KFileItem::Unknown,
                                  KFileItem::Unknown,
                                  KURL(basePath + *itU), true);
                fileItem->setExtraData(this, man->findPAlbum(*itD));
                d->itemList.append(fileItem);
                itU++;
                itD++;
            }
            
            d->db->commitTransaction();

            emit signalNewItems(d->itemList);

            gettimeofday(&t2, 0);
            printf("Scan time = %ld ms\n", (t2.tv_sec-t1.tv_sec)*1000 +
                   (t2.tv_usec-t1.tv_usec)/1000);

        }
    }
    else {
        emit signalClear();
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

void AlbumLister::slotNewItems(const KFileItemList& items)
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

