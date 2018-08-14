/* ============================================================
 * 
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-26
 * Description : common items needed for web services
 *
 * Copyright (C) 2018 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#ifndef DIGIKAM_WS_ITEM_H
#define DIGIKAM_WS_ITEM_H

// Qt includes

#include <QString>
#include <QStringList>

namespace Digikam
{

class WSAlbum
{

public:

    explicit WSAlbum()
      : parentID(QLatin1String("")),
        isRoot(true),
        description(QLatin1String("")),
        url(QLatin1String("")),
        uploadable(true)
    {
    }

    /*
     * This method is used by derived class of WSAblum,
     * to set the attributes inherited from WSAlbum, knowing
     * a WSAlbum.
     */
    void setBaseAlbum(const WSAlbum& album)
    {
        id          = album.id;
        parentID    = album.parentID;
        isRoot      = album.isRoot;
        title       = album.title;
        description = album.description;
        location    = album.location;
        url         = album.url;
        uploadable  = album.uploadable;
    }

    QString   id;
    QString   parentID;
    bool      isRoot;

    QString   title;
    QString   description;
    QString   location;
    QString   url;
    bool      uploadable;
};

/*
 * This class is used when parsing response of listAlbums().
 * It contains only the most important attributes of an album,
 * which is needed for further usage (e.g upload photos, create new album).
 */
class AlbumSimplified 
{

public:

    explicit AlbumSimplified()
      : uploadable(true)
    {
    }

    explicit AlbumSimplified(const QString& title)
      : title(title),
        uploadable(true)
    {
    }

    explicit AlbumSimplified(const QString& title, bool uploadable)
      : title(title),
        uploadable(uploadable)
    {
    }

public:

    QString     title;
    QStringList childrenIDs;
    bool        uploadable;
};

} // namespace Digikam

#endif // DIGIKAM_WS_ITEM_H
