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

#include <QString>
#include <QDataStream>
#include <QFileInfo>
#include <QDir>
#include <QMap>
#include <QPair>
#include <QTimer>

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
        untaggedFilter = false;
        ratingFilter   = 0;
        filterTimer    = 0;
        job            = 0;
        currAlbum      = 0;
        filter         = "*";
        ratingCond     = AlbumLister::GreaterEqualCondition;
        matchingCond   = AlbumLister::OrCondition;
    }

    bool                            untaggedFilter;

    QString                         filter;

    QMap<qlonglong, ImageInfo>      itemMap;
    QSet<int>                       invalidatedItems;
    QSet<int>                       dayFilter;

    QList<int>                      tagFilter;

    int                             ratingFilter; 

    QTimer                         *filterTimer;

    KIO::TransferJob               *job;

    ImageInfoList                   itemList;

    Album                          *currAlbum;

    AlbumLister::MatchingCondition  matchingCond;

    AlbumLister::RatingCondition    ratingCond;
};

AlbumLister* AlbumLister::m_instance = 0;

AlbumLister* AlbumLister::instance()
{
    if (!m_instance)
        new AlbumLister();

    return m_instance;
}

void AlbumLister::cleanUp()
{
    delete m_instance;
}

AlbumLister::AlbumLister()
{
    m_instance = this;

    d = new AlbumListerPriv;
    d->filterTimer = new QTimer(this);

    connect(d->filterTimer, SIGNAL(timeout()),
            this, SLOT(slotFilterItems()));
}

AlbumLister::~AlbumLister()
{
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
    d->job = ImageLister::startListJob(album->kurl());
                                       //d->filter,
                                       //AlbumSettings::instance()->getIconShowResolution());

    connect(d->job, SIGNAL(result(KJob*)),
            this, SLOT(slotResult(KJob*)));

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
    for (ImageInfoList::const_iterator it = d->itemList.begin(); it != d->itemList.end(); ++it)
    {
        d->itemMap.insert(it->id(), *it);
    }

    ImageLister lister;
    d->job = lister.startListJob(d->currAlbum->kurl());//, d->filter, AlbumSettings::instance()->getIconShowResolution());

    connect(d->job, SIGNAL(result(KJob*)),
            this, SLOT(slotResult(KJob*)));

    connect(d->job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotData(KIO::Job*, const QByteArray&)));
}

void AlbumLister::setDayFilter(const QList<int>& days)
{
    d->dayFilter = QSet<int>::fromList(days);

    d->filterTimer->setSingleShot(true);
    d->filterTimer->start(100);
}

void AlbumLister::setTagFilter(const QList<int>& tags, const MatchingCondition& matchingCond,
                               bool showUnTagged)
{
    d->tagFilter      = tags;
    d->matchingCond   = matchingCond;
    d->untaggedFilter = showUnTagged;
    d->filterTimer->setSingleShot(true);
    d->filterTimer->start(100);
}

void AlbumLister::setRatingFilter(int rating, const RatingCondition& ratingCond)
{
    d->ratingFilter = rating;
    d->ratingCond   = ratingCond;
    d->filterTimer->setSingleShot(true);
    d->filterTimer->start(100);
}

bool AlbumLister::matchesFilter(const ImageInfo &info) const
{
    if (d->dayFilter.isEmpty() && d->tagFilter.isEmpty() &&
        !d->untaggedFilter && d->ratingFilter == -1)
        return true;

    bool match = false;

    if (!d->tagFilter.isEmpty())
    {
        QList<int> tagIds = info.tagIds();
        QList<int>::iterator it;

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
        match = info.tagIds().isEmpty();
    }
    else
    {
        match = true;
    }

    if (!d->dayFilter.isEmpty())
    {
        match &= d->dayFilter.contains(info.dateTime().date().day());
    }

    // Filter by rating.
    if (d->ratingFilter >= 0) 
    {
        if (d->ratingCond == GreaterEqualCondition)
        {
            // If the rating is not >=, i.e it is <, then it does not match.
            if (info.rating() < d->ratingFilter)
            {
                match = false;
            }
        }
        else if (d->ratingCond == EqualCondition)
        {
            // If the rating is not =, i.e it is !=, then it does not match.
            if (info.rating() != d->ratingFilter)
            {
                match = false;
            }
        }
        else
        {
            // If the rating is not <=, i.e it is >, then it does not match.
            if (info.rating() > d->ratingFilter)
            {
                match = false;
            }
        }
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

void AlbumLister::invalidateItem(const ImageInfo &item)
{
    d->invalidatedItems << item.id();
}

void AlbumLister::slotClear()
{
    emit signalClear();
}

void AlbumLister::slotFilterItems()
{
    if (d->job)
    {
        d->filterTimer->setSingleShot(true);
        d->filterTimer->start(100);
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
        foreach(ImageInfo info, deleteFilteredItemsList)
            emit signalDeleteFilteredItem(info);
    }
    if (!newFilteredItemsList.isEmpty())
    {
        emit signalNewFilteredItems(newFilteredItemsList);
    }

    if (setCursor)
        kapp->restoreOverrideCursor();
}

void AlbumLister::slotResult(KJob* job)
{
    d->job = 0;

    if (job->error())
    {
        DWarning() << "Failed to list url: " << job->errorString() << endl;
        d->itemMap.clear();
        d->invalidatedItems.clear();
        return;
    }

    for (QMap<qlonglong, ImageInfo>::iterator it = d->itemMap.begin();
         it != d->itemMap.end(); ++it)
    {
        emit signalDeleteItem(it.value());
        emit signalDeleteFilteredItem(it.value());
        d->itemList.removeAll(it.value());
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

    QByteArray tmp(data);
    QDataStream ds(&tmp, QIODevice::ReadOnly);

    while (!ds.atEnd())
    {
        ImageListerRecord record;
        ds >> record;

        if (d->itemMap.contains(record.imageID))
        {
            ImageInfo info = d->itemMap[record.imageID];
            d->itemMap.remove(record.imageID);

            if (d->invalidatedItems.contains(record.imageID))
            {
                emit signalDeleteItem(info);
                emit signalDeleteFilteredItem(info);
                d->itemList.removeAll(info);
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

        ImageInfo info(record);

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



