/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-26
 * Description : Albums lister.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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
}

// C++ includes.

#include <cstdio>
#include <ctime>

// Qt includes.

#include <qstring.h>
#include <q3cstring.h>
#include <qdatastream.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmap.h>
#include <qpair.h>
#include <q3valuelist.h>
#include <qtimer.h>

// KDE includes.

#include <kapplication.h>
#include <kcursor.h>
#include <kio/job.h>
#include <kurl.h>

// Local includes.

#include "ddebug.h"
#include "imagelister.h"
#include "album.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "albumlister.h"
#include "albumlister.moc"

namespace Digikam
{

class AlbumListerPriv
{
public:

    AlbumListerPriv()
    {
        filterTimer  = 0;
        job          = 0;
        currAlbum    = 0;
        filter       = "*";
        matchingCond = AlbumLister::OrCondition;
    }

    bool                            untaggedFilter;

    QString                         filter;

    QMap<qlonglong, ImageInfo*>       itemMap;
    QMap<int,int>                   invalidatedItems;
    QMap<int,bool>                  dayFilter;

    Q3ValueList<int>                 tagFilter;

    QTimer                         *filterTimer;

    KIO::TransferJob               *job;

    ImageInfoList                   itemList;

    Album                          *currAlbum;

    AlbumLister::MatchingCondition matchingCond;
};

AlbumLister* AlbumLister::m_instance = 0;

AlbumLister* AlbumLister::componentData()
{
    if (!m_instance)
        new AlbumLister();

    return m_instance;
}

AlbumLister::AlbumLister()
{
    m_instance = this;

    d = new AlbumListerPriv;
    d->itemList.setAutoDelete(true);
    d->untaggedFilter = false;
    d->filterTimer    = new QTimer(this);

    connect(d->filterTimer, SIGNAL(timeout()),
            this, SLOT(slotFilterItems()));
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

    // Protocol = digikamalbums -> kio_digikamalbums
    d->job = ImageLister::startListJob(album->kurl(),
                                       d->filter,
                                       AlbumSettings::componentData().getIconShowResolution());

    connect(d->job, SIGNAL(result(KIO::Job*)),
            this, SLOT(slotResult(KIO::Job*)));

    connect(d->job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotData(KIO::Job*, const QByteArray&)));
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
    for (ImageInfoListIterator it = d->itemList.begin(); it != d->itemList.end(); ++it)
    {
        d->itemMap.insert((*it)->id(), *it);
    }

    ImageLister lister;
    d->job = lister.startListJob(d->currAlbum->kurl(), d->filter, AlbumSettings::componentData().getIconShowResolution());

    connect(d->job, SIGNAL(result(KIO::Job*)),
            this, SLOT(slotResult(KIO::Job*)));

    connect(d->job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotData(KIO::Job*, const QByteArray&)));
}

void AlbumLister::setDayFilter(const Q3ValueList<int>& days)
{
    d->dayFilter.clear();

    for (Q3ValueList<int>::const_iterator it = days.begin(); it != days.end(); ++it)
        d->dayFilter.insert(*it, true);

    d->filterTimer->start(100, true);
}

void AlbumLister::setTagFilter(const Q3ValueList<int>& tags, const MatchingCondition& matchingCond, 
                               bool showUnTagged)
{
    d->tagFilter      = tags;
    d->matchingCond   = matchingCond;
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
        Q3ValueList<int> tagIds = info->tagIds();
        Q3ValueList<int>::iterator it;

        if (d->matchingCond == OrCondition)        
        {
            for (it = d->tagFilter.begin(); it != d->tagFilter.end(); ++it)
            {
                if (tagIds.contains(*it))
                {
                    match = true;
                    break;
                }
            }
        }
        else
        {
            // AND matching condition...

            for (it = d->tagFilter.begin(); it != d->tagFilter.end(); ++it)
            {
                if (!tagIds.contains(*it))
                    break;
            }
    
            if (it == d->tagFilter.end())
                match = true;
        }

        match |= (d->untaggedFilter && tagIds.isEmpty());
    }
    else if (d->untaggedFilter)
    {
        match = info->tagIds().isEmpty();
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

void AlbumLister::invalidateItem(const ImageInfo *item)
{
    d->invalidatedItems.insert(item->id(), item->id());
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

    ImageInfoList newFilteredItemsList;
    ImageInfoList deleteFilteredItemsList;

    for (ImageInfoListIterator it = d->itemList.begin();
         it != d->itemList.end(); ++it)
    {
        if (matchesFilter(*it))
        {
            newFilteredItemsList.append(*it);
        }
        else
        {
            deleteFilteredItemsList.append(*it);
        }
    }

    // This takes linear time - and deleting seems to take longer. Set wait cursor for large numbers.
    bool setCursor = (3*deleteFilteredItemsList.count() + newFilteredItemsList.count()) > 1500;
    if (setCursor)
        kapp->setOverrideCursor(Qt::WaitCursor);

    if (!deleteFilteredItemsList.isEmpty())
    {
        for (ImageInfo *info=deleteFilteredItemsList.first(); info; info = deleteFilteredItemsList.next())
            emit signalDeleteFilteredItem(info);
    }
    if (!newFilteredItemsList.isEmpty())
    {
        emit signalNewFilteredItems(newFilteredItemsList);
    }

    if (setCursor)
        kapp->restoreOverrideCursor();
}

void AlbumLister::slotResult(KIO::Job* job)
{
    d->job = 0;

    if (job->error())
    {
        DWarning() << "Failed to list url: " << job->errorString() << endl;
        d->itemMap.clear();
        d->invalidatedItems.clear();
        return;
    }

    typedef QMap<qlonglong, ImageInfo*> ImMap;

    for (ImMap::iterator it = d->itemMap.begin();
         it != d->itemMap.end(); ++it)
    {
        emit signalDeleteItem(it.data());
        emit signalDeleteFilteredItem(it.data());
        d->itemList.remove(it.data());
    }

    d->itemMap.clear();
    d->invalidatedItems.clear();

    emit signalCompleted();
}

void AlbumLister::slotData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
        return;

    ImageInfoList newItemsList;
    ImageInfoList newFilteredItemsList;

    QDataStream ds(data, QIODevice::ReadOnly);

    while (!ds.atEnd())
    {
        ImageListerRecord record;
        ds >> record;

        if (d->itemMap.contains(record.imageID))
        {
            ImageInfo* info = d->itemMap[record.imageID];
            d->itemMap.remove(record.imageID);

            if (d->invalidatedItems.contains(record.imageID))
            {
                emit signalDeleteItem(info);
                emit signalDeleteFilteredItem(info);
                d->itemList.remove(info);
            }
            else
            {
                if (!matchesFilter(info))
                {
                    emit signalDeleteFilteredItem(info);
                }
                continue;
            }
        }

        ImageInfo* info = new ImageInfo(record);

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



