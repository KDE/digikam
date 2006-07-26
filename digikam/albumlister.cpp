/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-26
 * Description :
 *
 * Copyright 2004 by Renchi Raju
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

// C Ansi includes.

extern "C"
{
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
}

// Qt includes.

#include <qstring.h>
#include <qcstring.h>
#include <qdatastream.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmap.h>
#include <qpair.h>
#include <qvaluelist.h>
#include <qtimer.h>

// KDE includes.

#include <kdebug.h>
#include <kio/job.h>
#include <kurl.h>

// Local includes.

#include "album.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "albumlister.h"

namespace Digikam
{

class AlbumListerPriv
{
public:

    KIO::TransferJob*          job;
    QString                    filter;

    Album*                     currAlbum;

    QMap<int,bool>             dayFilter;
    QValueList<int>            tagFilter;
    bool                       untaggedFilter;
    QTimer*                    filterTimer;

    ImageInfoList              itemList;
    QMap<Q_LLONG, ImageInfo*>  itemMap;
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
    d->currAlbum = 0;
    d->filter    = "*";
    d->itemList.setAutoDelete(true);
    d->untaggedFilter = false;
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
    d->itemMap.clear();

    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }

    if (!album)
        return;

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << AlbumManager::instance()->getLibraryPath();
    ds << album->kurl();
    ds << d->filter;
    ds << AlbumSettings::instance()->getIconShowResolution();

    // Protocol = digikamalbums -> kio_digikamalbums
    d->job = new KIO::TransferJob(album->kurl(), KIO::CMD_SPECIAL,
                                  ba, QByteArray(), false);
    connect(d->job, SIGNAL(result(KIO::Job*)),
            SLOT(slotResult(KIO::Job*)));
    connect(d->job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            SLOT(slotData(KIO::Job*, const QByteArray&)));
}

void AlbumLister::refresh()
{
    if (!d->currAlbum)
        return;

    d->filterTimer->stop();

    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }

    d->itemMap.clear();
    ImageInfo* item;
    for (ImageInfoListIterator it(d->itemList); (item = it.current()); ++it)
    {
        d->itemMap.insert(item->id(), item);
    }

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << AlbumManager::instance()->getLibraryPath();
    ds << d->currAlbum->kurl();
    ds << d->filter;
    ds << AlbumSettings::instance()->getIconShowResolution();

    d->job = new KIO::TransferJob(d->currAlbum->kurl(), KIO::CMD_SPECIAL,
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

void AlbumLister::setTagFilter(const QValueList<int>& tags, bool showUnTagged)
{
    d->tagFilter = tags;
    d->untaggedFilter = showUnTagged;

    d->filterTimer->start(100, true);
}

bool AlbumLister::matchesFilter(const ImageInfo* info) const
{
    if (d->dayFilter.isEmpty() && d->tagFilter.isEmpty() &&
        !d->untaggedFilter)
        return true;

    bool match = false;

    if (!d->tagFilter.isEmpty())
    {
        QValueList<int> tagIDs = info->tagIDs();
        for (QValueList<int>::iterator it = d->tagFilter.begin();
             it != d->tagFilter.end(); ++it)
        {
            if (tagIDs.contains(*it))
            {
                match = true;
                break;
            }
        }

        match |= (d->untaggedFilter && tagIDs.isEmpty());
    }
    else if (d->untaggedFilter)
    {
        match = info->tagIDs().isEmpty();
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
    d->currAlbum = 0;
    d->filterTimer->stop();
    emit signalClear();
    d->itemList.clear();
    d->itemMap.clear();

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

    QPtrList<ImageInfo> newFilteredItemsList;

    ImageInfo* item;
    for (ImageInfoListIterator it(d->itemList);
         (item = it.current()); ++it)
    {
        if (matchesFilter(item))
        {
            if (!item->getViewItem())
                newFilteredItemsList.append(item);
        }
        else
        {
            if (item->getViewItem())
                emit signalDeleteFilteredItem(item);
        }
    }

    if (!newFilteredItemsList.isEmpty())
        emit signalNewFilteredItems(newFilteredItemsList);
}

void AlbumLister::slotResult(KIO::Job* job)
{
    d->job = 0;

    if (job->error())
    {
        kdWarning() << "Failed to list url: " << job->errorString() << endl;
        d->itemMap.clear();
        return;
    }


    typedef QMap<Q_LLONG, ImageInfo*> ImMap;

    for (ImMap::iterator it = d->itemMap.begin();
         it != d->itemMap.end(); ++it)
    {
        emit signalDeleteItem(it.data());
        emit signalDeleteFilteredItem(it.data());
        d->itemList.remove(it.data());
    }

    d->itemMap.clear();

    emit signalCompleted();
}

void AlbumLister::slotData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
        return;

    Q_LLONG imageID;
    int     albumID;
    QString name;
    QString date;
    size_t  size;
    QSize   dims;

    ImageInfoList newItemsList;
    ImageInfoList newFilteredItemsList;

    QDataStream ds(data, IO_ReadOnly);
    while (!ds.atEnd())
    {
        ds >> imageID;
        ds >> albumID;
        ds >> name;
        ds >> date;
        ds >> size;
        ds >> dims;

        if (d->itemMap.contains(imageID))
        {
            ImageInfo* info = d->itemMap[imageID];
            if (!matchesFilter(info))
            {
                emit signalDeleteFilteredItem(info);
            }
            
            d->itemMap.remove(imageID);
            continue;
        }

        ImageInfo* info = new ImageInfo(imageID, albumID, name,
                                        QDateTime::fromString(date,
                                                              Qt::ISODate),
                                        size, dims);

        if (matchesFilter(info))
            newFilteredItemsList.append(info);
        newItemsList.append(info);
        d->itemList.append(info);
    }

    if (!newFilteredItemsList.isEmpty())
        emit signalNewFilteredItems(newFilteredItemsList);

    if (!newItemsList.isEmpty())
        emit signalNewItems(newItemsList);
}

}  // namespace Digikam

#include "albumlister.moc"

