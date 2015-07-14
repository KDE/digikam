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

#include "databaseurl.h"

namespace Digikam
{

class DBJobInfo
{
public:

    enum Type
    {
        AlbumsJob=0,
        TagsJob,
        DatesJob,
        SearchesJob,
        GPSJob
    };

public:

    Type type();

public:

    bool folders;
    bool listAvailableImagesOnly;
    bool recursive;

protected:

    DBJobInfo(Type jType);

private:

    Type jobType;
};

// ---------------------------------------------

class AlbumsDBJobInfo : public DBJobInfo
{
public:

    AlbumsDBJobInfo();

public:

    int     albumRootId;
    QString album;
};

// ---------------------------------------------

class TagsDBJobInfo : public DBJobInfo
{
public:

    TagsDBJobInfo();

public:

    bool       faceFolders;
    QString    specialTag;
    QList<int> tagsIds;
};

// ---------------------------------------------

class GPSDBJobInfo : public DBJobInfo
{
public:

    GPSDBJobInfo();

public:

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

public:
    
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

public:

    QDate startDate;
    QDate endDate;
};

} // namespace Digikam

#endif // DBJOBINFO_H
