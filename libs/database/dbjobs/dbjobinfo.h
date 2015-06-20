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

#include <QString>
#include "databaseurl.h"

namespace Digikam
{

class DBJobInfo
{
public:
    enum Type {
        AlbumsJob=0,
        TagsJob,
        DatesJob,
        SearchesJob,
        GPSJob
    };

    DBJobInfo(Type jType);
    DBJobInfo();

    Type        type();

    bool folders;
    bool listAvailableImagesOnly;
    bool recursive;

private:
    Type jobType;
};

// ---------------------------------------------

class AlbumsDBJobInfo : public DBJobInfo
{
public:
    AlbumsDBJobInfo();

    int     albumRootId;
    QString album;
};

// ---------------------------------------------

class TagsDBJobInfo : public DBJobInfo
{
public:
    TagsDBJobInfo();

    bool       faceFolders;
    QString    specialTag;
    QList<int> tagsIds;
};

// ---------------------------------------------

class GPSDBJobInfo : public DBJobInfo
{
public:
    GPSDBJobInfo();

    bool  wantDirectQuery;
    qreal lat1;
    qreal lng1;
    qreal lat2;
    qreal lng2;
};

// ---------------------------------------------

class SearchesDBJobInfo : public DBJobInfo
{
public:
    SearchesDBJobInfo();

    bool       duplicates;
    int        searchId;
    double     threshold;
    QList<int> albumIds;
    QList<int> tagIds;
};

// ---------------------------------------------

class DatesDBJobInfo : public DBJobInfo
{
public:
    DatesDBJobInfo();

    QDate startDate;
    QDate endDate;
};

}
#endif // DBJOBINFO_H
