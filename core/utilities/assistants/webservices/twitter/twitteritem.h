/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter social network
 *
 * Copyright (C) 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
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

#ifndef DIGIKAM_TW_ITEM_H
#define DIGIKAM_TW_ITEM_H

 // Qt includes

#include <QString>

namespace Digikam
{

class TwUser
{
public:

    TwUser()
    {
        id         = 0;
        uploadPerm = false;
    }

    void clear()
    {
        id         = 0;
        name.clear();
        profileURL = QStringLiteral("https://www.facebook.com");
        uploadPerm = true;
    }

    long long id;

    QString   name;
    QString   profileURL;
    bool      uploadPerm;
};

// ---------------------------------------------------------------

/*enum TWPrivacy
{
    FB_ME = 0,
    FB_FRIENDS = 1,
    FB_FRIENDS_OF_FRIENDS,
    FB_NETWORKS,
    FB_EVERYONE,
    FB_CUSTOM
};*/

// ---------------------------------------------------------------

class TwAlbum
{
public:

    TwAlbum()
    {
        //privacy = FB_FRIENDS;
    }

    QString   id;

    QString   title;
    QString   description;
    QString   location;
    //FbPrivacy privacy;
    QString   url;
};

// ---------------------------------------------------------------

class TwPhoto
{
public:

    TwPhoto()
    {
    }

    QString id;

    QString caption;
    QString thumbURL;
    QString originalURL;
};

} // namespace Digikam

#endif // DIGIKAM_TW_ITEM_H
