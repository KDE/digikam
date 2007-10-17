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
 * \class AlbumRootInfo
 */
class AlbumRootInfo
{
public:

    AlbumRootInfo() : id(0), status(-1), type(-1) {};

    int id;
    int status;
    QString absolutePath;
    int type;
    QString uuid;
    QString specificPath;
};

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
    bool operator<(const AlbumInfo& info) const
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

    bool operator<(const TagInfo& info) const
    {
        return name < info.name;
    }
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
    bool operator<(const SearchInfo& info) const
    {
        return id < info.id;
    }
};

class AlbumShortInfo
{
public:

    AlbumShortInfo() : id(0) {};

    int         id;
    QString     relativePath;
    QString     albumRoot;
};

namespace DatabaseItem
{
    enum Status
    {
        // Keep values constant
        UndefinedStatus = 0,
        Visible         = 1,
        Hidden          = 2,
        Removed         = 3
    };
}

class ItemShortInfo
{
public:

    ItemShortInfo() : id(0), albumID(0) {};

    qlonglong   id;
    QString     itemName;
    int         albumID;
    QString     albumRoot;
    QString     album;
};

class ItemScanInfo
{
public:

    ItemScanInfo()
        : id(0), albumID(0), status(DatabaseItem::UndefinedStatus)
    {};

    qlonglong            id;
    int                  albumID;
    QString              itemName;
    DatabaseItem::Status status;
    QDateTime            modificationDate;
    QString              uniqueHash;
};

namespace DatabaseComment
{
    enum Source
    {
        UndefinedSource = 0
    };
}

class CommentInfo
{
public:

    CommentInfo() : imageId(-1), source(DatabaseComment::UndefinedSource) {};

    int                     id;
    qlonglong               imageId;
    DatabaseComment::Source source;
    QString                 author;
    QString                 language;
    QDateTime               date;
    QString                 comment;
};

}  // namespace Digikam

#endif /* ALBUMINFO_H */
