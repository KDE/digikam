/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-21
 * Description : Structures for use in AlbumDB
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

typedef QPair<int, int> YearMonth;

namespace AlbumRoot
{
    enum Type
    {
        // Keep values constant
        UndefinedType   = 0,
        VolumeHardWired = 1,
        VolumeRemovable = 2,
        Network         = 3
    };
}

/**
 * \class AlbumRootInfo
 */
class AlbumRootInfo
{
public:

    AlbumRootInfo() : id(0), type(AlbumRoot::UndefinedType) {};

    int             id;
    QString         label;
    AlbumRoot::Type type;
    int             status;
    QString         identifier;
    QString         specificPath;
};

/**
 * \class AlbumInfo
 * A container class for transporting album information
 * from the database to AlbumManager
 */
class AlbumInfo
{
public:

    AlbumInfo() : id(0), iconAlbumRootId(0) {};

    typedef QList<AlbumInfo> List;

    int     id;
    int     albumRootId;
    QString relativePath;
    QString caption;
    QString category;
    QDate   date;
    int     iconAlbumRootId;
    QString iconRelativePath;

    /**
     * needed for sorting
     */
    bool operator<(const AlbumInfo& info) const
    {
        // include album root id?
        return relativePath < info.relativePath;
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

    TagInfo() : id(0), pid(0), iconAlbumRootId(0) {};

    typedef QList<TagInfo> List;

    int     id;
    int     pid;
    QString name;
    QString icon;
    int     iconAlbumRootId;
    QString iconRelativePath;

    bool operator<(const TagInfo& info) const
    {
        return name < info.name;
    }
};

namespace DatabaseSearch
{
    enum Type
    {
        UndefinedType,
        KeywordSearch,
        AdvancedSearch,
        LegacyUrlSearch,
        TimeLineSearch,
        HaarSearch,
        MapSearch,
        DuplicatesSearch
    };
}

/**
 * \class SearchInfo
 * A container class for transporting search information
 * from the database to AlbumManager
 */
class SearchInfo
{
public:

    SearchInfo() : id(0), type(DatabaseSearch::UndefinedType) {};

    typedef QList<SearchInfo> List;

    int                  id;
    QString              name;
    DatabaseSearch::Type type;
    QString              query;

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
    int         albumRootId;
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

    enum Category
    {
        // Keep values constant
        UndefinedCategory = 0,
        Image             = 1,
        Video             = 2,
        Audio             = 3,
        Other             = 4
    };
}

class ItemShortInfo
{
public:

    ItemShortInfo() : id(0), albumID(0) {};

    qlonglong id;
    QString   itemName;
    int       albumID;
    int       albumRootID;
    QString   album;
};

class ItemScanInfo
{
public:

    ItemScanInfo()
        : id(0), albumID(0), status(DatabaseItem::UndefinedStatus),
          category(DatabaseItem::UndefinedCategory)
    {};

    qlonglong              id;
    int                    albumID;
    QString                itemName;
    DatabaseItem::Status   status;
    DatabaseItem::Category category;
    QDateTime              modificationDate;
    QString                uniqueHash;
};

namespace DatabaseComment
{
    enum Type
    {
        // Keep values constant
        /// UndefinedType: Shall never appear in the database
        UndefinedType   = 0,
        /**
         *  Comment: The default - a normal comment
         *  This is what the user in digikam edits as the comment.
         *  It is mapped to and from the JFIF comment,
         *  the EXIF user comment, the IPTC Caption,
         *  Dublin Core and Photoshop Description.
         */
        Comment         = 1,
        /// Headline: as with IPTC or Photoshop
        Headline        = 2,
        /// Title: as with Dublin Core Title, Photoshop Title, IPTC Object Name
        Title           = 3
        // Feel free to add here any more types that you need!
    };
}

class CommentInfo
{
public:

    CommentInfo() : imageId(-1), type(DatabaseComment::UndefinedType) {};

    int                   id;
    qlonglong             imageId;
    DatabaseComment::Type type;
    QString               author;
    QString               language;
    QDateTime             date;
    QString               comment;
};

class CopyrightInfo
{
public:

    CopyrightInfo() : id(-1) {};

    qlonglong id;
    QString   property;
    QString   value;
    QString   extraValue;
};

}  // namespace Digikam

#endif /* ALBUMINFO_H */
