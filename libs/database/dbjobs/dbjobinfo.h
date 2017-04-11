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

#ifndef DBJOBINFO_H
#define DBJOBINFO_H

// Qt includes

#include <QString>

// Local includes

#include "coredburl.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT DBJobInfo
{
public:

    void setFoldersJob();
    bool isFoldersJob() const;

    void setListAvailableImagesOnly();
    bool isListAvailableImagesOnly() const;

    void setRecursive();
    bool isRecursive() const;

protected:

    DBJobInfo();

private:

    bool m_folders;
    bool m_listAvailableImagesOnly;
    bool m_recursive;
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT AlbumsDBJobInfo : public DBJobInfo
{
public:

    AlbumsDBJobInfo();

    void setAlbumRootId(int id);
    int  albumRootId();

    void setAlbum(const QString& album);
    QString album();

private:

    int     m_albumRootId;
    QString m_album;
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT TagsDBJobInfo : public DBJobInfo
{
public:

    TagsDBJobInfo();

    void setFaceFoldersJob();
    bool isFaceFoldersJob() const;

    void setSpecialTag(const QString& tag);
    QString specialTag() const;

    void setTagsIds(const QList<int>& tagsIds);
    QList<int> tagsIds() const;

private:

    bool       m_faceFolders;
    QString    m_specialTag;
    QList<int> m_tagsIds;
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT GPSDBJobInfo : public DBJobInfo
{
public:

    GPSDBJobInfo();

    void setDirectQuery();
    bool isDirectQuery() const;

    void setLat1(qreal lat);
    qreal lat1() const;

    void setLng1(qreal lng);
    qreal lng1() const;

    void setLat2(qreal lat);
    qreal lat2() const;

    void setLng2(qreal lng);
    qreal lng2() const;

private:

    bool  m_directQuery;
    qreal m_lat1;
    qreal m_lng1;
    qreal m_lat2;
    qreal m_lng2;
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT SearchesDBJobInfo : public DBJobInfo
{
public:

    SearchesDBJobInfo();

    void setDuplicatesJob();
    bool isDuplicatesJob() const;

    void setAlbumUpdate();
    bool isAlbumUpdate() const;

    void setSearchIds(QList<int> ids);
    void setSearchId(int id);
    QList<int> searchIds() const;

    void setMinThreshold(double t);
    double minThreshold() const;

    void setMaxThreshold(double t);
    double maxThreshold() const;

    void setAlbumTagRelation(int type);
    int albumTagRelation() const;

    void setSearchResultRestriction(int type);
    int searchResultRestriction() const;

    void setAlbumsIds(const QList<int>& albumsIds);
    QList<int> albumsIds() const;

    void setImageIds(const QList<qlonglong>& imageIds);
    QList<qlonglong> imageIds() const;

    void setTagsIds(const QList<int>& tagsIds);
    QList<int> tagsIds() const;

public:

    bool             m_duplicates;
    bool             m_albumUpdate;
    int              m_albumTagRelation;
    int              m_searchResultRestriction;
    QList<int>       m_searchIds;
    double           m_minThreshold;
    double           m_maxThreshold;
    QList<int>       m_albumsIds;
    QList<qlonglong> m_imageIds;
    QList<int>       m_tagsIds;
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT DatesDBJobInfo : public DBJobInfo
{
public:

    DatesDBJobInfo();

    void setStartDate(const QDate& date);
    QDate startDate() const;

    void setEndDate(const QDate& date);
    QDate endDate() const;

private:

    QDate m_startDate;
    QDate m_endDate;
};

} // namespace Digikam

#endif // DBJOBINFO_H
