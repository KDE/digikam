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
#include <qcstring.h>
#include <qdatastream.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmap.h>
#include <qvaluelist.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kio/job.h>
#include <kurl.h>

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

    enum State
    {
        LIST=0,
        UPDATE
    };
    
    KIO::TransferJob*  job;
    State              state;
    QString            filter;
    ImageInfoList      itemList;

    Album*             currAlbum;
    AlbumDB*           db;
    QByteArray         buffer;

    QMap<int,bool>     dayFilter;
    QValueList<int>    tagFilter;
    QTimer*            filterTimer;
};

AlbumLister* AlbumLister::m_instance = 0;

AlbumLister* AlbumLister::instance()
{
    if (!m_instance)
        new AlbumLister();

    return m_instance;
}

AlbumLister::AlbumLister()
{
    m_instance = this;

    d = new AlbumListerPriv;
    d->job       = 0;
    d->state     = AlbumListerPriv::LIST;
    d->currAlbum = 0;
    d->filter    = "*";
    d->itemList.setAutoDelete(true);
    d->filterTimer = new QTimer(this);

    connect(d->filterTimer, SIGNAL(timeout()),
            SLOT(slotFilterItems()));
}

AlbumLister::~AlbumLister()
{
    delete d->filterTimer;
    delete d;
    m_instance = 0;
}

void AlbumLister::openAlbum(Album *album)
{
    d->currAlbum = album;
    d->filterTimer->stop();
    emit signalClear();
    d->itemList.clear();        

    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }

    if (!album)
        return;

    d->state = AlbumListerPriv::LIST;
    d->buffer.resize(0);
        
    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << AlbumManager::instance()->getLibraryPath();
    ds << album->getKURL().path();
    ds << d->filter;
    ds << AlbumSettings::instance()->getRecurseTags();

    d->job = new KIO::TransferJob(album->getKURL(), KIO::CMD_SPECIAL,
                                  ba, QByteArray(), false);
    connect(d->job, SIGNAL(result(KIO::Job*)),
            SLOT(slotResult(KIO::Job*)));
    connect(d->job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            SLOT(slotData(KIO::Job*, const QByteArray&)));
}

void AlbumLister::updateDirectory()
{
    if (!d->currAlbum)
        return;

    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }

    d->state = AlbumListerPriv::UPDATE;
    d->buffer.resize(0);
        
    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << AlbumManager::instance()->getLibraryPath();
    ds << d->currAlbum->getKURL().path();
    ds << d->filter;
    ds << AlbumSettings::instance()->getRecurseTags();

    d->job = new KIO::TransferJob(d->currAlbum->getKURL(), KIO::CMD_SPECIAL,
                                  ba, QByteArray(), false);
    connect(d->job, SIGNAL(result(KIO::Job*)),
            SLOT(slotResult(KIO::Job*)));
    connect(d->job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            SLOT(slotData(KIO::Job*, const QByteArray&)));
}

void AlbumLister::setDayFilter(const QValueList<int>& days)
{
    d->dayFilter.clear();

    for (QValueList<int>::const_iterator it = days.begin(); it != days.end(); ++it)
        d->dayFilter.insert(*it, true);

    d->filterTimer->start(100, true);    
}

void AlbumLister::setTagFilter(const QValueList<int>& tags)
{
    d->tagFilter = tags;

    d->filterTimer->start(100, true);    
}

bool AlbumLister::matchesFilter(const ImageInfo* info) const
{
    if (d->dayFilter.isEmpty() && d->tagFilter.isEmpty())
        return true;

    bool match = true;

    if (!d->tagFilter.isEmpty())
    {
        QValueList<int> tagIDs = info->tagIDs();
        for (QValueList<int>::iterator it = d->tagFilter.begin();
             it != d->tagFilter.end(); ++it)
        {
            if (!tagIDs.contains(*it))
            {
                match = false;
                break;
            }
        }
    }
    else
    {
        match = true;
    }

    if (!d->dayFilter.isEmpty())
    {
        match &= d->dayFilter.contains(info->dateTime().date().day());
    }
    
    return match;
}

void AlbumLister::stop()
{
    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }
}

void AlbumLister::setNameFilter(const QString& nameFilter)
{
    d->filter = nameFilter;
}

void AlbumLister::slotClear()
{
    emit signalClear();
}

void AlbumLister::slotFilterItems()
{
    if (d->job)
    {
        d->filterTimer->start(100, true);
        return;
    }

    QPtrList<ImageInfo> newItemsList;

    ImageInfo* item;
    for (ImageInfoListIterator it(d->itemList);
         (item = it.current()); ++it)
    {
        if (matchesFilter(item))
        {
            if (!item->getViewItem())
                newItemsList.append(item);
        }
        else
        {
            if (item->getViewItem())
                emit signalDeleteItem(item);
        }
    }

    if (!newItemsList.isEmpty())
        emit signalNewItems(newItemsList);
}

void AlbumLister::slotResult(KIO::Job* job)
{
    d->job = 0;

    if (job->error())
    {
        kdWarning() << "Failed to list url" << endl;
        return;
    }

    if (d->state == AlbumListerPriv::UPDATE)
    {
        // insert the currently listed items into a map for quick searches
        typedef QMap<QString, ImageInfo*> ImMap;
        ImMap currItems;

        ImageInfo* item;
        for (ImageInfoListIterator it(d->itemList);
             (item = it.current()); ++it)
        {
            currItems.insert(QDir::cleanDirPath(item->kurl().path()),
                                item);
        }

        QValueList<ImageInfo*> newItems;
        
        // generate a list of what the kioslave has sent us.
        int     pid;    
        QString path;
        QString date;
        size_t  size;
    
        QDataStream ds(d->buffer, IO_ReadOnly);
        while (!ds.atEnd())
        {
            ds >> pid;
            ds >> path;
            ds >> date;
            ds >> size;

            if (currItems.contains(path))
            {
                // already present. remove it from the map
                currItems.remove(path);
            }
            else
            {
                // new item. add it to the list of newItems
                newItems.append(new ImageInfo(pid, path.section('/', -1),
                                              QDateTime::fromString(date, Qt::ISODate),
                                              size));
            }
        }

        // now the items contained in the map are the ones which
        // have been deleted
        for (ImMap::iterator it = currItems.begin();
             it != currItems.end(); ++it)
        {
            emit signalDeleteItem(it.data());
            d->itemList.remove(it.data());
        }

        // now deal with the new items which have been listed
        for (QValueList<ImageInfo*>::iterator it = newItems.begin();
             it != newItems.end(); ++it)
        {
            d->itemList.append(*it);
        }

        // also emit the signal that new items have arrived
        QPtrList<ImageInfo> newItemsList;
        for (QValueList<ImageInfo*>::iterator it = newItems.begin();
             it != newItems.end(); ++it)
        {
            if (matchesFilter(*it))
                newItemsList.append(*it);
        }

        if (!newItemsList.isEmpty())
            emit signalNewItems(newItemsList);
    }


    emit signalCompleted();
}

void AlbumLister::slotData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
        return;

    if (d->state == AlbumListerPriv::LIST)
    {    
        int     pid;    
        QString path;
        QString date;
        size_t  size;

        ImageInfoList itemList;
        
        QDataStream ds(data, IO_ReadOnly);
        while (!ds.atEnd())
        {
            ds >> pid;
            ds >> path;
            ds >> date;
            ds >> size;

            ImageInfo* info = new ImageInfo(pid, path.section('/', -1),
                                            QDateTime::fromString(date, Qt::ISODate),
                                            size);

            if (matchesFilter(info))
                itemList.append(info);
            d->itemList.append(info);
        }

        if (!itemList.isEmpty())
            emit signalNewItems(itemList);

        return;
    }

    int oldSize = d->buffer.size();
    d->buffer.resize(d->buffer.size() + data.size());
    memcpy(d->buffer.data()+oldSize, data.data(), data.size());
}

#include "albumlister.moc"

