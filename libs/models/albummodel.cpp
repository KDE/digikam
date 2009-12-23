/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-22
 * Description : Qt Model for Albums
 *
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "albummodel.moc"

// KDE includes

#include <kcalendarsystem.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "albummanager.h"
#include "albumthumbnailloader.h"

namespace Digikam
{

AlbumModel::AlbumModel(RootAlbumBehavior rootBehavior, QObject *parent)
          : AbstractCheckableAlbumModel(Album::PHYSICAL,
                                        AlbumManager::instance()->findPAlbum(0),
                                        rootBehavior, parent)
{
    m_columnHeader = i18n("My Albums");
    setupThumbnailLoading();
}

AlbumModel::~AlbumModel()
{

}

PAlbum *AlbumModel::albumForIndex(const QModelIndex& index) const
{
    return static_cast<PAlbum*>(AbstractCheckableAlbumModel::albumForIndex(index));
}

QVariant AlbumModel::decorationRole(Album *album) const
{
    // asynchronous signals are handled by parent class
    return AlbumThumbnailLoader::instance()->getAlbumThumbnailDirectly(static_cast<PAlbum *>(album));
}

Album* AlbumModel::albumForId(int id) const
{
    return AlbumManager::instance()->findPAlbum(id);
}

// ------------------------------------------------------------------

TagModel::TagModel(RootAlbumBehavior rootBehavior, QObject *parent)
        : AbstractCheckableAlbumModel(Album::TAG,
                                      AlbumManager::instance()->findTAlbum(0),
                                      rootBehavior, parent)
{
    m_columnHeader = i18n("My Tags");
    setupThumbnailLoading();
}

TAlbum *TagModel::albumForIndex(const QModelIndex& index) const
{
    return static_cast<TAlbum*>(AbstractCheckableAlbumModel::albumForIndex(index));
}

QVariant TagModel::decorationRole(Album *album) const
{
    return AlbumThumbnailLoader::instance()->getTagThumbnailDirectly(static_cast<TAlbum *>(album), true);
}

Album* TagModel::albumForId(int id) const
{
    return AlbumManager::instance()->findTAlbum(id);
}

// ------------------------------------------------------------------

SearchModel::SearchModel(QObject *parent)
            : AbstractSpecificAlbumModel(Album::SEARCH,
                                         AlbumManager::instance()->findSAlbum(0),
                                         IgnoreRootAlbum, parent)
{
}

SAlbum *SearchModel::albumForIndex(const QModelIndex& index) const
{
    return static_cast<SAlbum*>(AbstractSpecificAlbumModel::albumForIndex(index));
}

void SearchModel::setReplaceNames(QHash<QString, QString> replaceNames)
{
    m_replaceNames = replaceNames;
}

void SearchModel::addReplaceName(const QString& technicalName, const QString& userVisibleName)
{
    m_replaceNames.insert(technicalName, userVisibleName);
}

void SearchModel::setPixmapForNormalSearches(const QPixmap& pix)
{
    m_pixmaps.insert(-1, pix);
}

void SearchModel::setDefaultPixmap(const QPixmap& pix)
{
    m_pixmaps.insert(-2, pix);
}

void SearchModel::setPixmapForTimelineSearches(const QPixmap& pix)
{
    m_pixmaps.insert(DatabaseSearch::TimeLineSearch, pix);
}

void SearchModel::setPixmapForHaarSearches(const QPixmap& pix)
{
    m_pixmaps.insert(DatabaseSearch::HaarSearch, pix);
}

void SearchModel::setPixmapForMapSearches(const QPixmap& pix)
{
    m_pixmaps.insert(DatabaseSearch::MapSearch, pix);
}

void SearchModel::setPixmapForDuplicatesSearches(const QPixmap& pix)
{
    m_pixmaps.insert(DatabaseSearch::DuplicatesSearch, pix);
}

QVariant SearchModel::albumData(Album *a, int role) const
{
    if (role == Qt::DisplayRole || role == AlbumTitleRole)
    {
        QString name = a->title();
        return m_replaceNames.value(name, name);
    }
    else if (role == Qt::DecorationRole)
    {
        SAlbum *salbum = static_cast<SAlbum*>(a);
        QPixmap pixmap = m_pixmaps.value(salbum->searchType());
        if (pixmap.isNull() && salbum->isNormalSearch())
        {
            pixmap = m_pixmaps.value(-1);
        }
        if (pixmap.isNull())
        {
            pixmap = m_pixmaps.value(-2);
        }
        return pixmap;
    }

    return AbstractSpecificAlbumModel::albumData(a, role);
}

// ------------------------------------------------------------------

DateAlbumModel::DateAlbumModel(QObject *parent)
            : AbstractCountingAlbumModel(Album::DATE,
                                         AlbumManager::instance()->findDAlbum(0),
                                         IgnoreRootAlbum, parent)
{
}

DAlbum *DateAlbumModel::albumForIndex(const QModelIndex& index) const
{
    return static_cast<DAlbum*>(AbstractCountingAlbumModel::albumForIndex(index));
}

QModelIndex DateAlbumModel::monthIndexForDate(const QDate &date) const
{

    // iterate over all years
    for (int yearIndex = 0; yearIndex < rowCount(); ++yearIndex)
    {
        QModelIndex year = index(yearIndex, 0);
        DAlbum *yearAlbum = albumForIndex(year);

        // do not search through months if we are sure, that the year already
        // does not match
        if (yearAlbum && (yearAlbum->range() == DAlbum::Year)
                      && (yearAlbum->date().year() != date.year()))
        {
            continue;
        }

        // search the album with the correct month
        for (int monthIndex = 0; monthIndex < rowCount(year); ++monthIndex)
        {
            QModelIndex month = index(monthIndex, 0, year);
            DAlbum *monthAlbum = albumForIndex(month);
            if (monthAlbum && (monthAlbum->range() == DAlbum::Month)
                           && (monthAlbum->date().year() == date.year())
                           && (monthAlbum->date().month() == date.month()))
            {
                return month;
            }
        }

    }

    return QModelIndex();

}

void DateAlbumModel::setPixmaps(const QPixmap& forYearAlbums, const QPixmap& forMonthAlbums)
{
    m_yearPixmap = forYearAlbums;
    m_monthPixmap = forMonthAlbums;
}

QString DateAlbumModel::albumName(Album *album) const
{
    DAlbum *dalbum = static_cast<DAlbum*>(album);
    if (dalbum->range() == DAlbum::Year)
        return QString::number(dalbum->date().year());
    else
        return KGlobal::locale()->calendar()->monthName(dalbum->date(), KCalendarSystem::LongName);
}

QVariant DateAlbumModel::decorationRole(Album *album) const
{
    DAlbum *dalbum = static_cast<DAlbum*>(album);
    if (dalbum->range() == DAlbum::Year)
        return m_yearPixmap;
    else
        return m_monthPixmap;
}

Album* DateAlbumModel::albumForId(int id) const
{
    return AlbumManager::instance()->findDAlbum(id);
}

void DateAlbumModel::setYearMonthMap(const QMap<YearMonth, int>& yearMonthMap)
{
    AlbumIterator it(rootAlbum());

    while (it.current())
    {
        DAlbum *dalbum = static_cast<DAlbum*>(*it);
        QDate date = dalbum->date();

        if (dalbum->range() == DAlbum::Month)
        {
            QMap<YearMonth, int>::const_iterator it2 = yearMonthMap.constFind(YearMonth(date.year(), date.month()));
            if ( it2 != yearMonthMap.constEnd() )
                setCount(*it, it2.value());
        }
        else
        {
            int count = 0;
            for ( QMap<YearMonth, int>::const_iterator it2 = yearMonthMap.constBegin();
                    it2 != yearMonthMap.constEnd(); ++it2 )
            {
                if (it2.key().first == date.year())
                    count += it2.value();
            }
            setCount(*it, count);
        }
        ++it;
    }
}

} // namespace Digikam
