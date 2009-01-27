/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-26
 * Description : Albums lister.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007 by Arnd Baecker <arnd dot baecker at web dot de>
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
#include <qcstring.h>
#include <qdatastream.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmap.h>
#include <qpair.h>
#include <qvaluelist.h>
#include <qtimer.h>

// KDE includes.

#include <kapplication.h>
#include <kcursor.h>
#include <kio/job.h>
#include <kurl.h>

// Local includes.

#include "ddebug.h"
#include "mimefilter.h"
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
        namesFilter    = "*";
        mimeTypeFilter = MimeFilter::AllFiles;
        ratingCond     = AlbumLister::GreaterEqualCondition;
        matchingCond   = AlbumLister::OrCondition;
        recurseAlbums  = false;
        recurseTags    = false;
    }

    bool                            untaggedFilter;

    int                             ratingFilter;
    int                             recurseAlbums;
    int                             recurseTags;

    QString                         namesFilter;
    QString                         textFilter;

    QMap<Q_LLONG, ImageInfo*>       itemMap;
    QMap<int, int>                  invalidatedItems;
    QMap<QDateTime, bool>           dayFilter;

    QValueList<int>                 tagFilter;

    QTimer                         *filterTimer;

    KIO::TransferJob               *job;

    ImageInfoList                   itemList;

    Album                          *currAlbum;

    MimeFilter::TypeMimeFilter      mimeTypeFilter;

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

AlbumLister::AlbumLister()
{
    m_instance = this;

    d = new AlbumListerPriv;
    d->itemList.setAutoDelete(true);
    d->filterTimer = new QTimer(this);

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

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << AlbumManager::instance()->getLibraryPath();
    ds << album->kurl();
    ds << d->namesFilter;
    ds << AlbumSettings::instance()->getIconShowResolution();
    ds << d->recurseAlbums;
    ds << d->recurseTags;

    // Protocol = digikamalbums -> kio_digikamalbums
    d->job = new KIO::TransferJob(album->kurl(), KIO::CMD_SPECIAL,
                                  ba, QByteArray(), false);

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
    ImageInfo* item;
    for (ImageInfoListIterator it(d->itemList); (item = it.current()); ++it)
    {
        d->itemMap.insert(item->id(), item);
    }

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << AlbumManager::instance()->getLibraryPath();
    ds << d->currAlbum->kurl();
    ds << d->namesFilter;
    ds << AlbumSettings::instance()->getIconShowResolution();
    ds << d->recurseAlbums;
    ds << d->recurseTags;

    d->job = new KIO::TransferJob(d->currAlbum->kurl(), KIO::CMD_SPECIAL,
                                  ba, QByteArray(), false);

    connect(d->job, SIGNAL(result(KIO::Job*)),
            this, SLOT(slotResult(KIO::Job*)));

    connect(d->job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotData(KIO::Job*, const QByteArray&)));
}

void AlbumLister::setDayFilter(const QValueList<QDateTime>& days)
{
    d->dayFilter.clear();

    for (QValueList<QDateTime>::const_iterator it = days.begin(); it != days.end(); ++it)
        d->dayFilter.insert(*it, true);

    d->filterTimer->start(100, true);
}

bool AlbumLister::tagFiltersIsActive()
{
    if (!d->tagFilter.isEmpty() || d->untaggedFilter)
        return true;

    return false;
}

void AlbumLister::setTagFilter(const QValueList<int>& tags, const MatchingCondition& matchingCond,
                               bool showUnTagged)
{
    d->tagFilter      = tags;
    d->matchingCond   = matchingCond;
    d->untaggedFilter = showUnTagged;
    d->filterTimer->start(100, true);
}

void AlbumLister::setRatingFilter(int rating, const RatingCondition& ratingCond)
{
    d->ratingFilter = rating;
    d->ratingCond   = ratingCond;
    d->filterTimer->start(100, true);
}

void AlbumLister::setMimeTypeFilter(int mimeTypeFilter)
{
    d->mimeTypeFilter = (MimeFilter::TypeMimeFilter)mimeTypeFilter;
    d->filterTimer->start(100, true);
}

void AlbumLister::setTextFilter(const QString& text)
{
    d->textFilter = text;
    d->filterTimer->start(100, true);
}

void AlbumLister::setRecurseAlbums(bool recursive)
{
    d->recurseAlbums = recursive;
    refresh();
}

void AlbumLister::setRecurseTags(bool recursive)
{
    d->recurseTags = recursive;
    refresh();
}

bool AlbumLister::matchesFilter(const ImageInfo* info, bool &foundText)
{
    if (d->dayFilter.isEmpty() && d->tagFilter.isEmpty() && d->textFilter.isEmpty() &&
        !d->untaggedFilter && d->ratingFilter==-1)
        return true;

    bool match = false;

    if (!d->tagFilter.isEmpty())
    {
        QValueList<int> tagIDs = info->tagIDs();
        QValueList<int>::iterator it;

        if (d->matchingCond == OrCondition)
        {
            for (it = d->tagFilter.begin(); it != d->tagFilter.end(); ++it)
            {
                if (tagIDs.contains(*it))
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
                if (!tagIDs.contains(*it))
                    break;
            }

            if (it == d->tagFilter.end())
                match = true;
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
        match &= d->dayFilter.contains(QDateTime(info->dateTime().date(), QTime()));
    }

    //-- Filter by rating ---------------------------------------------------------

    if (d->ratingFilter >= 0)
    {
        if (d->ratingCond == GreaterEqualCondition)
        {
            // If the rating is not >=, i.e it is <, then it does not match.
            if (info->rating() < d->ratingFilter)
            {
                match = false;
            }
        }
        else if (d->ratingCond == EqualCondition)
        {
            // If the rating is not =, i.e it is !=, then it does not match.
            if (info->rating() != d->ratingFilter)
            {
                match = false;
            }
        }
        else
        {
            // If the rating is not <=, i.e it is >, then it does not match.
            if (info->rating() > d->ratingFilter)
            {
                match = false;
            }
        }
    }

    // -- Filter by mime type -----------------------------------------------------

    QFileInfo fi(info->filePath());
    QString mimeType = fi.extension(false).upper();

    switch(d->mimeTypeFilter)
    {
        case MimeFilter::ImageFiles:
        {
            QString imageFilesExt(AlbumSettings::instance()->getImageFileFilter());
            imageFilesExt.append(AlbumSettings::instance()->getRawFileFilter());
            if (!imageFilesExt.upper().contains(mimeType))
                match = false;
            break;
        }
        case MimeFilter::JPGFiles:
        {
            if (mimeType != QString("JPG") && mimeType != QString("JPE") &&
                mimeType != QString("JPEG"))
                match = false;
            break;
        }
        case MimeFilter::PNGFiles:
        {
            if (mimeType != QString("PNG"))
                match = false;
            break;
        }
        case MimeFilter::TIFFiles:
        {
            if (mimeType != QString("TIF") && mimeType != QString("TIFF"))
                match = false;
            break;
        }
        case MimeFilter::NoRAWFiles:
        {
            QString rawFilesExt(AlbumSettings::instance()->getRawFileFilter());
            if (rawFilesExt.upper().contains(mimeType))
                match = false;
            break;
        }
        case MimeFilter::RAWFiles:
        {
            QString rawFilesExt(AlbumSettings::instance()->getRawFileFilter());
            if (!rawFilesExt.upper().contains(mimeType))
                match = false;
            break;
        }
        case MimeFilter::MoviesFiles:
        {
            QString moviesFilesExt(AlbumSettings::instance()->getMovieFileFilter());
            if (!moviesFilesExt.upper().contains(mimeType))
                match = false;
            break;
        }
        case MimeFilter::AudioFiles:
        {
            QString audioFilesExt(AlbumSettings::instance()->getAudioFileFilter());
            if (!audioFilesExt.upper().contains(mimeType))
                match = false;
            break;
        }
        default:        // All Files: do nothing...
            break;
    }

    //-- Filter by text -----------------------------------------------------------

    if (!d->textFilter.isEmpty())
    {
        foundText = false;
        if (info->name().lower().contains(d->textFilter.lower()))
        {
            foundText = true;
        }
        if (info->caption().lower().contains(d->textFilter.lower()))
            foundText = true;
        QStringList tags = info->tagNames();
        for (QStringList::const_iterator it = tags.constBegin() ; it != tags.constEnd() ; ++it)
        {
            if ((*it).lower().contains(d->textFilter.lower()))
                foundText = true;
        }
        // check for folder names
        PAlbum* palbum = AlbumManager::instance()->findPAlbum(info->albumID());
        if ((palbum && palbum->title().lower().contains(d->textFilter.lower())))
        {
            foundText = true;
        }
        match &= foundText;
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

void AlbumLister::setNamesFilter(const QString& namesFilter)
{
    d->namesFilter = namesFilter;
}

void AlbumLister::invalidateItem(const ImageInfo *item)
{
    d->invalidatedItems.insert(item->id(), item->id());
}

void AlbumLister::slotFilterItems()
{
    if (d->job)
    {
        d->filterTimer->start(100, true);
        return;
    }

    QPtrList<ImageInfo> newFilteredItemsList;
    QPtrList<ImageInfo> deleteFilteredItemsList;
    ImageInfo *item   = 0;
    bool matchForText = false;
    bool match        = false;

    for (ImageInfoListIterator it(d->itemList);
         (item = it.current()); ++it)
    {
        bool foundText = false;
        if (matchesFilter(item, foundText))
        {
            match = true;
            if (!item->getViewItem())
                newFilteredItemsList.append(item);
        }
        else
        {
            if (item->getViewItem())
                deleteFilteredItemsList.append(item);
        }

        if (foundText)
            matchForText = true;
    }

    // This takes linear time - and deleting seems to take longer. Set wait cursor for large numbers.
    bool setCursor = (3*deleteFilteredItemsList.count() + newFilteredItemsList.count()) > 1500;
    if (setCursor)
        kapp->setOverrideCursor(KCursor::waitCursor());

    emit signalItemsTextFilterMatch(matchForText);
    emit signalItemsFilterMatch(match);

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

    typedef QMap<Q_LLONG, ImageInfo*> ImMap;

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
        bool foundText = false;

        ds >> imageID;
        ds >> albumID;
        ds >> name;
        ds >> date;
        ds >> size;
        ds >> dims;

        if (d->itemMap.contains(imageID))
        {
            ImageInfo* info = d->itemMap[imageID];
            d->itemMap.remove(imageID);

            if (d->invalidatedItems.contains(imageID))
            {
                emit signalDeleteItem(info);
                emit signalDeleteFilteredItem(info);
                d->itemList.remove(info);
            }
            else
            {
                if (!matchesFilter(info, foundText))
                {
                    emit signalDeleteFilteredItem(info);
                }
                continue;
            }
        }

        ImageInfo* info = new ImageInfo(imageID, albumID, name,
                                        QDateTime::fromString(date, Qt::ISODate),
                                        size, dims);

        if (matchesFilter(info, foundText))
            newFilteredItemsList.append(info);

        newItemsList.append(info);
        d->itemList.append(info);
    }

    if (!newFilteredItemsList.isEmpty())
        emit signalNewFilteredItems(newFilteredItemsList);

    if (!newItemsList.isEmpty())
        emit signalNewItems(newItemsList);

    slotFilterItems();
}

}  // namespace Digikam
