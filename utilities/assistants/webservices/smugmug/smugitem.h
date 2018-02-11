/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-01
 * Description : a tool to export images to Smugmug web service
 *
 * Copyright (C) 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef SMUG_ITEM_H
#define SMUG_ITEM_H

// Qt includes

#include <QString>

namespace Digikam
{

class SmugUser
{

public:

    explicit SmugUser()
    {
        fileSizeLimit = 0;
    }

    void clear()
    {
        email.clear();
        nickName.clear();
        displayName.clear();
        accountType.clear();
        fileSizeLimit = 0;
    }

public:

    QString email;
    QString nickName;
    QString displayName;
    QString accountType;
    int     fileSizeLimit;
};

// ---------------------------------------------------------------------------------

class SmugAlbum
{

public:

    explicit SmugAlbum()
    {
        id            = -1;
        categoryID    = -1;
        subCategoryID = -1;
        tmplID        = -1;
        isPublic      = true;
        imageCount    = 0;
    }

public:

    qint64  id;

    QString key;
    QString title;
    QString description;
    QString keywords;

    qint64  categoryID;
    QString category;

    qint64  subCategoryID;
    QString subCategory;

    bool    isPublic;
    QString password;
    QString passwordHint;

    int     imageCount;

    // below fields only used by createAlbum (and not by listAlbums)
    qint64  tmplID;
    QString tmpl;

    static bool lessThan(SmugAlbum& a, SmugAlbum& b)
    {
        return a.title.toLower() < b.title.toLower();
    }
};

// ---------------------------------------------------------------------------------

class SmugPhoto
{

public:

    explicit SmugPhoto()
    {
        id = -1;
    }

public:

    qint64  id;

    QString key;
    QString caption;
    QString keywords;

    QString thumbURL;
    QString originalURL;
};

// ---------------------------------------------------------------------------------

class SmugAlbumTmpl
{

public:

    explicit SmugAlbumTmpl()
    {
        id       = -1;
        isPublic = true;
    }

public:

    qint64  id;
    QString name;

    bool    isPublic;
    QString password;
    QString passwordHint;
};

// ---------------------------------------------------------------------------------

class SmugCategory
{

public:

    explicit SmugCategory()
    {
        id = -1;
    }

public:

    qint64  id;
    QString name;
};

} // namespace Digikam

#endif // SMUG_ITEM_H
