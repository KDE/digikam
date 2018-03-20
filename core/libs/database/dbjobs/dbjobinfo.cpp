/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-05
 * Description : DB Jobs Info
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "dbjobinfo.h"

namespace Digikam
{

void DBJobInfo::setFoldersJob()
{
    m_folders = true;
}

bool DBJobInfo::isFoldersJob() const
{
    return m_folders;
}

void DBJobInfo::setListAvailableImagesOnly()
{
    m_listAvailableImagesOnly = true;
}

bool DBJobInfo::isListAvailableImagesOnly() const
{
    return m_listAvailableImagesOnly;
}

void DBJobInfo::setRecursive()
{
    m_recursive = true;
}

bool DBJobInfo::isRecursive() const
{
    return m_recursive;
}

DBJobInfo::DBJobInfo()
{
    m_folders                 = false;
    m_listAvailableImagesOnly = false;
    m_recursive               = false;
}

// ---------------------------------------------

AlbumsDBJobInfo::AlbumsDBJobInfo()
    : DBJobInfo()
{
    m_albumRootId = -1;
}

void AlbumsDBJobInfo::setAlbumRootId(int id)
{
    m_albumRootId = id;
}

int AlbumsDBJobInfo::albumRootId()
{
    return m_albumRootId;
}

void AlbumsDBJobInfo::setAlbum(const QString& album)
{
    m_album = album;
}

QString AlbumsDBJobInfo::album()
{
    return m_album;
}

// ---------------------------------------------

TagsDBJobInfo::TagsDBJobInfo()
    : DBJobInfo()
{
    m_faceFolders = false;
}

void TagsDBJobInfo::setFaceFoldersJob()
{
    m_faceFolders = true;
}

bool TagsDBJobInfo::isFaceFoldersJob() const
{
    return m_faceFolders;
}

void TagsDBJobInfo::setSpecialTag(const QString& tag)
{
    m_specialTag = tag;
}

QString TagsDBJobInfo::specialTag() const
{
    return m_specialTag;
}

void TagsDBJobInfo::setTagsIds(const QList<int>& tagsIds)
{
    m_tagsIds = tagsIds;
}

QList<int> TagsDBJobInfo::tagsIds() const
{
    return m_tagsIds;
}

// ---------------------------------------------

GPSDBJobInfo::GPSDBJobInfo()
    : DBJobInfo()
{
    m_directQuery = false;
    m_lat1        = 0;
    m_lng1        = 0;
    m_lat2        = 0;
    m_lng2        = 0;
}

void GPSDBJobInfo::setDirectQuery()
{
    m_directQuery = true;
}

bool GPSDBJobInfo::isDirectQuery() const
{
    return m_directQuery;
}

void GPSDBJobInfo::setLat1(qreal lat)
{
    m_lat1 = lat;
}

qreal GPSDBJobInfo::lat1() const
{
    return m_lat1;
}

void GPSDBJobInfo::setLng1(qreal lng)
{
    m_lng1 = lng;
}

qreal GPSDBJobInfo::lng1() const
{
    return m_lng1;
}

void GPSDBJobInfo::setLat2(qreal lat)
{
    m_lat2 = lat;
}

qreal GPSDBJobInfo::lat2() const
{
    return m_lat2;
}

void GPSDBJobInfo::setLng2(qreal lng)
{
    m_lng2 = lng;
}

qreal GPSDBJobInfo::lng2() const
{
    return m_lng2;
}

// ---------------------------------------------

SearchesDBJobInfo::SearchesDBJobInfo()
    : DBJobInfo()
{
    m_duplicates              = false;
    m_albumUpdate             = false;
    m_minThreshold            = 0;
    m_maxThreshold            = 1;
    m_albumTagRelation        = 0;
    m_searchResultRestriction = 0;
    m_searchIds               = QList<int>();
}

void SearchesDBJobInfo::setDuplicatesJob()
{
    m_duplicates = true;
}

bool SearchesDBJobInfo::isDuplicatesJob() const
{
    return m_duplicates;
}

void SearchesDBJobInfo::setAlbumUpdate()
{
    m_albumUpdate = true;
}

bool SearchesDBJobInfo::isAlbumUpdate() const
{
    return m_albumUpdate;
}

void SearchesDBJobInfo::setAlbumTagRelation(int type)
{
    m_albumTagRelation = type;
}

int SearchesDBJobInfo::albumTagRelation() const
{
    return m_albumTagRelation;
}

void SearchesDBJobInfo::setSearchResultRestriction(int type)
{
    m_searchResultRestriction = type;
}

int SearchesDBJobInfo::searchResultRestriction() const
{
    return m_searchResultRestriction;
}

void SearchesDBJobInfo::setSearchId(int id)
{
    m_searchIds = QList<int>() << id;
}

void SearchesDBJobInfo::setSearchIds(QList<int> ids)
{
    m_searchIds = ids;
}

QList<int> SearchesDBJobInfo::searchIds() const
{
    return m_searchIds;
}

void SearchesDBJobInfo::setMinThreshold(double t)
{
    m_minThreshold = t;
}

double SearchesDBJobInfo::minThreshold() const
{
    return m_minThreshold;
}

void SearchesDBJobInfo::setMaxThreshold(double t)
{
    m_maxThreshold = t;
}

double SearchesDBJobInfo::maxThreshold() const
{
    return m_maxThreshold;
}

void SearchesDBJobInfo::setAlbumsIds(const QList<int>& albumsIds)
{
    m_albumsIds = albumsIds;
}

QList<int> SearchesDBJobInfo::albumsIds() const
{
    return m_albumsIds;
}

void SearchesDBJobInfo::setImageIds(const QList<qlonglong>& imageIds)
{
    m_imageIds = imageIds;
}

QList<qlonglong> SearchesDBJobInfo::imageIds() const
{
    return m_imageIds;
}

void SearchesDBJobInfo::setTagsIds(const QList<int>& tagsIds)
{
    m_tagsIds = tagsIds;
}

QList<int> SearchesDBJobInfo::tagsIds() const
{
    return m_tagsIds;
}

// ---------------------------------------------

DatesDBJobInfo::DatesDBJobInfo()
    : DBJobInfo()
{
}

void DatesDBJobInfo::setStartDate(const QDate& date)
{
    m_startDate = date;
}

QDate DatesDBJobInfo::startDate() const
{
    return m_startDate;
}

void DatesDBJobInfo::setEndDate(const QDate& date)
{
    m_endDate = date;
}

QDate DatesDBJobInfo::endDate() const
{
    return m_endDate;
}

} // namespace Digikam
