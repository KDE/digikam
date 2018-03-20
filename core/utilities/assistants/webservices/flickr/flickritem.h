/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-07
 * Description : a tool to export images to Flickr web service
 *
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FLICKR_ITEM_H
#define FLICKR_ITEM_H

// Qt includes

#include <QStringList>
#include <QString>

// Local includes

#include "flickrlist.h"

namespace Digikam
{

class GPhoto
{

public:

    GPhoto()
    {
        is_public  = false;
        is_private = false;
        is_family  = false;
        ref_num    = -1;
    }

    bool        is_public;
    bool        is_private;
    bool        is_family;

    int         ref_num;

    QStringList tags;
    QString     title;
    QString     description;

/*
    int         album_num;
    QString     caption;
    QString     thumbName;
    QString     albumURL;
*/
};

// -------------------------------------------------------------

class FPhotoInfo
{

public:

    FPhotoInfo()
    {
        is_public    = false;
        is_family    = false;
        is_friend    = false;
        safety_level = FlickrList::SAFE;
        content_type = FlickrList::PHOTO;
        size         = 0;
    }

    bool                    is_public;
    bool                    is_friend;
    bool                    is_family;

    QString                 title;
    QString                 description;
    qlonglong               size;
    QStringList             tags;

    FlickrList::SafetyLevel safety_level;
    FlickrList::ContentType content_type;
};

// -------------------------------------------------------------

class GAlbum
{

public:

    GAlbum()
    {
        ref_num        = -1;
        parent_ref_num = -1;

        add            = false;
        write          = false;
        del_item       = false;
        del_alb        = false;
        create_sub     = false;
    }

    bool    add;
    bool    write;
    bool    del_item;
    bool    del_alb;
    bool    create_sub;

    int     ref_num;
    int     parent_ref_num;

    QString name;
    QString parentName;
    QString title;
    QString summary;
    QString baseurl;
};

// -------------------------------------------------------------

class FPhotoSet
{

public:

    FPhotoSet()
    {
        id = QStringLiteral("-1");
    }

    QString id;
    QString primary;    //="2483"
    QString secret;     //="abcdef"
    QString server;
    QString photos;
    QString title;
    QString description;
};

} // namespace Digikam

#endif // FLICKR_ITEM_H
