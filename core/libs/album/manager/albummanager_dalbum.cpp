/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - Date Album helpers.
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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

#include "albummanager_p.h"

namespace Digikam
{

void AlbumManager::scanDAlbums()
{
    d->scanDAlbumsTimer->stop();

    if (d->dateListJob)
    {
        d->dateListJob->cancel();
        d->dateListJob = nullptr;
    }

    DatesDBJobInfo jInfo;
    jInfo.setFoldersJob();
    d->dateListJob = DBJobsManager::instance()->startDatesJobThread(jInfo);

    connect(d->dateListJob, SIGNAL(finished()),
            this, SLOT(slotDatesJobResult()));

    connect(d->dateListJob, SIGNAL(foldersData(QMap<QDateTime,int>)),
            this, SLOT(slotDatesJobData(QMap<QDateTime,int>)));
}

AlbumList AlbumManager::allDAlbums() const
{
    AlbumList list;

    if (d->rootDAlbum)
    {
        list.append(d->rootDAlbum);
    }

    AlbumIterator it(d->rootDAlbum);

    while (it.current())
    {
        list.append(*it);
        ++it;
    }

    return list;
}

DAlbum* AlbumManager::findDAlbum(int id) const
{
    if (!d->rootDAlbum)
    {
        return nullptr;
    }

    int gid = d->rootDAlbum->globalID() + id;

    return static_cast<DAlbum*>((d->allAlbumsIdHash.value(gid)));
}

QMap<YearMonth, int> AlbumManager::getDAlbumsCount() const
{
    return d->dAlbumsCount;
}

void AlbumManager::slotDatesJobResult()
{
    if (!d->dateListJob)
    {
        return;
    }

    if (d->dateListJob->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list dates";

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), d->dateListJob->errorsList().first(),
                             nullptr, i18n("digiKam"));
    }

    d->dateListJob = nullptr;

    emit signalAllDAlbumsLoaded();
}

void AlbumManager::slotDatesJobData(const QMap<QDateTime, int>& datesStatMap)
{
    if (datesStatMap.isEmpty() || !d->rootDAlbum)
    {
        return;
    }

    // insert all the DAlbums into a qmap for quick access
    QMap<QDate, DAlbum*> mAlbumMap;
    QMap<int, DAlbum*>   yAlbumMap;

    AlbumIterator it(d->rootDAlbum);

    while (it.current())
    {
        DAlbum* const a = (DAlbum*)(*it);

        if (a->range() == DAlbum::Month)
        {
            mAlbumMap.insert(a->date(), a);
        }
        else
        {
            yAlbumMap.insert(a->date().year(), a);
        }

        ++it;
    }

    QMap<YearMonth, int> yearMonthMap;

    for (QMap<QDateTime, int>::const_iterator it = datesStatMap.constBegin() ;
         it != datesStatMap.constEnd() ; ++it)
    {
        YearMonth yearMonth = YearMonth(it.key().date().year(), it.key().date().month());

        QMap<YearMonth, int>::iterator it2 = yearMonthMap.find(yearMonth);

        if (it2 == yearMonthMap.end())
        {
            yearMonthMap.insert(yearMonth, *it);
        }
        else
        {
            *it2 += *it;
        }
    }

    int year, month;

    for (QMap<YearMonth, int>::const_iterator iter = yearMonthMap.constBegin() ;
         iter != yearMonthMap.constEnd() ; ++iter)
    {
        year  = iter.key().first;
        month = iter.key().second;

        QDate md(year, month, 1);

        // Do we already have this Month album
        if (mAlbumMap.contains(md))
        {
            // already there. remove Month album from map
            mAlbumMap.remove(md);

            if (yAlbumMap.contains(year))
            {
                // already there. remove from map
                yAlbumMap.remove(year);
            }

            continue;
        }

        // Check if Year Album already exist.
        DAlbum* yAlbum = nullptr;
        AlbumIterator it(d->rootDAlbum);

        while (it.current())
        {
            DAlbum* const a = (DAlbum*)(*it);

            if (a->date() == QDate(year, 1, 1) && a->range() == DAlbum::Year)
            {
                yAlbum = a;
                break;
            }

            ++it;
        }

        // If no, create Year album.
        if (!yAlbum)
        {
            yAlbum = new DAlbum(QDate(year, 1, 1), false, DAlbum::Year);
            emit signalAlbumAboutToBeAdded(yAlbum, d->rootDAlbum, d->rootDAlbum->lastChild());
            yAlbum->setParent(d->rootDAlbum);
            d->allAlbumsIdHash.insert(yAlbum->globalID(), yAlbum);
            emit signalAlbumAdded(yAlbum);
        }

        // Create Month album
        DAlbum* const mAlbum = new DAlbum(md);

        emit signalAlbumAboutToBeAdded(mAlbum, yAlbum, yAlbum->lastChild());

        mAlbum->setParent(yAlbum);
        d->allAlbumsIdHash.insert(mAlbum->globalID(), mAlbum);

        emit signalAlbumAdded(mAlbum);
    }

    // Now the items contained in the maps are the ones which
    // have been deleted.
    for (QMap<QDate, DAlbum*>::const_iterator it = mAlbumMap.constBegin() ;
         it != mAlbumMap.constEnd() ; ++it)
    {
        DAlbum* const album = it.value();
        emit signalAlbumAboutToBeDeleted(album);
        d->allAlbumsIdHash.remove(album->globalID());

        emit signalAlbumDeleted(album);

        quintptr deletedAlbum = reinterpret_cast<quintptr>(album);
        delete album;

        emit signalAlbumHasBeenDeleted(deletedAlbum);
    }

    for (QMap<int, DAlbum*>::const_iterator it = yAlbumMap.constBegin() ;
         it != yAlbumMap.constEnd() ; ++it)
    {
        DAlbum* const album = it.value();
        emit signalAlbumAboutToBeDeleted(album);
        d->allAlbumsIdHash.remove(album->globalID());

        emit signalAlbumDeleted(album);

        quintptr deletedAlbum = reinterpret_cast<quintptr>(album);
        delete album;

        emit signalAlbumHasBeenDeleted(deletedAlbum);
    }

    d->dAlbumsCount = yearMonthMap;

    emit signalDAlbumsDirty(yearMonthMap);
    emit signalDatesMapDirty(datesStatMap);
}

void AlbumManager::scanDAlbumsScheduled()
{
    // Avoid a cycle of killing a job which takes longer than the timer interval
    if (d->dateListJob)
    {
        d->scanDAlbumsTimer->start();
        return;
    }

    scanDAlbums();
}

} // namespace Digikam
