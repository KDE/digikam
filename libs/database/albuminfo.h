/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-21
 * Description : Structures for use in AlbumDB
 *
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#ifndef ALBUMINFO_H
#define ALBUMINFO_H

/** @file albuminfo.h */

// Qt includes.

#include <QString>
#include <QList>
#include <QDateTime>

// KDE includes.

#include <kurl.h>

namespace Digikam
{

/**
 * \class AlbumInfo
 * A container class for transporting album information
 * from the database to AlbumManager
 */
class AlbumInfo
{
public:

    AlbumInfo() : id(0) {};

    typedef QList<AlbumInfo> List;

    int      id;
    QString  albumRoot;
    QString  url;
    QString  caption;
    QString  collection;
    QDate    date;
    QString  icon;

    /**
     * needed for sorting
     */
    bool operator<(const AlbumInfo& info)
    {
        return url < info.url;
    }
};

/**
 * \class TagInfo
 * A container class for transporting tag information
 * from the database to AlbumManager
 */
class TagInfo
{
public:

    TagInfo() : id(0), pid(0) {};

    typedef QList<TagInfo> List;

    int      id;
    int      pid;
    QString  name;
    QString  icon;
};

/**
 * \class SearchInfo
 * A container class for transporting search information
 * from the database to AlbumManager
 */
class SearchInfo
{
public:

    SearchInfo() : id(0) {};

    typedef QList<SearchInfo> List;

    int      id;
    QString  name;
    KUrl     url;

    /**
     * needed for sorting
     */
    bool operator<(const SearchInfo& info)
    {
        return id < info.id;
    }
};

class AlbumShortInfo
{
public:

    AlbumShortInfo() : id(0) {};

    int         id;
    QString     url;
    QString     albumRoot;
};

class ItemShortInfo
{
public:

    ItemShortInfo() : id(0), albumID(0) {};

    int         id;
    QString     itemName;
    int         albumID;
    QString     albumRoot;
    QString     album;
};

}  // namespace Digikam

#endif /* ALBUMINFO_H */
