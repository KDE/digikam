/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-21
 * Description : Structures to use in AlbumDB
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2005 by Renchi Raju <renchi dot raju at gmail dot com>
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

// Qt includes

#include <QString>
#include <QList>
#include <QDateTime>

// KDE includes

#include <kurl.h>

// Local includes

#include "databaseconstants.h"

namespace Digikam
{

typedef QPair<int, int> YearMonth;

/**
 * \class AlbumRootInfo
 */
class AlbumRootInfo
{
public:

    AlbumRootInfo() : id(0), type(AlbumRoot::UndefinedType), status(0) {};

    int             id;
    QString         label;
    AlbumRoot::Type type;
    int             status;
    QString         identifier;
    QString         specificPath;
};

// --------------------------------------------------------------------------

/**
 * \class AlbumInfo
 * A container class for transporting album information
 * from the database to AlbumManager
 */
class AlbumInfo
{
public:

    AlbumInfo() : id(0), albumRootId(0), iconId(0) {};

    typedef QList<AlbumInfo> List;

    bool isNull() const
    {
        return id == 0;
    }

    int     id;
    int     albumRootId;
    QString relativePath;
    QString caption;
    QString category;
    QDate   date;
    qlonglong iconId;

    /**
     * needed for sorting
     */
    bool operator<(const AlbumInfo& info) const
    {
        // include album root id?
        return relativePath < info.relativePath;
    }
};

// --------------------------------------------------------------------------

/**
 * \class TagInfo
 * A container class for transporting tag information
 * from the database to AlbumManager
 */
class TagInfo
{
public:

    TagInfo() : id(0), pid(0), iconId(0) {};

    typedef QList<TagInfo> List;

    bool isNull() const
    {
        return id == 0;
    }

    int     id;
    int     pid;
    QString name;
    QString icon;
    qlonglong iconId;

    bool operator<(const TagInfo& info) const
    {
        return name < info.name;
    }
};

// --------------------------------------------------------------------------

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

    bool isNull() const
    {
        return id == 0;
    }

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

// --------------------------------------------------------------------------

class AlbumShortInfo
{
public:

    AlbumShortInfo() : id(0), albumRootId(0) {};

    bool isNull() const
    {
        return id == 0;
    }

    int     id;
    QString relativePath;
    int     albumRootId;
};

// --------------------------------------------------------------------------

class TagShortInfo
{
public:

    TagShortInfo() : id(0), pid(0) {};

    bool isNull() const
    {
        return id == 0;
    }

    int     id;
    int     pid;
    QString name;
};

// --------------------------------------------------------------------------

class ItemShortInfo
{
public:

    ItemShortInfo() : id(0), albumID(0), albumRootID(0) {};

    bool isNull() const
    {
        return id == 0;
    }

    qlonglong id;
    QString   itemName;
    int       albumID;
    int       albumRootID;
    QString   album;
};

// --------------------------------------------------------------------------

class ItemScanInfo
{
public:

    ItemScanInfo()
        : id(0), albumID(0), status(DatabaseItem::UndefinedStatus),
          category(DatabaseItem::UndefinedCategory), fileSize(0)
    {};

    bool isNull() const
    {
        return id == 0;
    }

    qlonglong              id;
    int                    albumID;
    QString                itemName;
    DatabaseItem::Status   status;
    DatabaseItem::Category category;
    QDateTime              modificationDate;
    qlonglong              fileSize;
    QString                uniqueHash;
};

// --------------------------------------------------------------------------

class CommentInfo
{
public:

    CommentInfo() : id(-1), imageId(-1), type(DatabaseComment::UndefinedType) {};

    bool isNull() const
    {
        return id == -1;
    }

    int                   id;
    qlonglong             imageId;
    DatabaseComment::Type type;
    QString               author;
    QString               language;
    QDateTime             date;
    QString               comment;
};

// --------------------------------------------------------------------------

class CopyrightInfo
{
public:

    CopyrightInfo() : id(-1) {};

    bool isNull() const
    {
        return id == -1;
    }

    qlonglong id;
    QString   property;
    QString   value;
    QString   extraValue;
};

// --------------------------------------------------------------------------

class ImageHistoryEntry
{
public:

    ImageHistoryEntry() : imageId(0) {};

    bool isNull() const
    {
        return imageId == 0;
    }

    qlonglong imageId;
    QString   uuid;
    QString   history;
};

// --------------------------------------------------------------------------

class ImageRelation
{
public:

    ImageRelation() : subjectId(0), objectId(0), type(DatabaseRelation::UndefinedType) {}

    qlonglong              subjectId;
    qlonglong              objectId;
    DatabaseRelation::Type type;
};

// --------------------------------------------------------------------------

class TagProperty
{
public:

    TagProperty() : tagId(-1) {};

    bool isNull() const
    {
        return tagId == -1;
    }

    int tagId;
    QString property;
    QString value;
};

// --------------------------------------------------------------------------

class ImageTagProperty
{
public:

    ImageTagProperty() : imageId(-1), tagId(-1) {};

    bool isNull() const
    {
        return imageId == -1;
    }

    qlonglong imageId;
    int tagId;
    QString property;
    QString value;
};

}  // namespace Digikam

#endif /* ALBUMINFO_H */
