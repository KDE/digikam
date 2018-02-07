/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export items to Google web services
 *
 * Copyright (C) 2013      by Pankaj Kumar <me at panks dot me>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#ifndef GS_ITEM_H
#define GS_ITEM_H

// Qt includes

#include <QUrl>
#include <QString>

namespace Digikam
{

enum GoogleService
{
    GDrive = 1,
    GPhotoExport,
    GPhotoImport
};

// -----------------------------------------------------------

class GSPhoto
{
public:

    GSPhoto()
    {
        id         = QLatin1String("-1");
        canComment = true;
        gpsLon     = QLatin1String("");
        gpsLat     = QLatin1String("");
    }

public:

    QString     id;
    QString     title;
    QString     timestamp;
    QString     description;
    QString     location;
    QString     access;
    bool        canComment;
    QStringList tags;
    QString     mimeType;
    QString     gpsLon;
    QString     gpsLat;
    QUrl        originalURL;
    QUrl        thumbURL;
    QUrl        editUrl;
};

// -----------------------------------------------------------

class GSFolder
{
public:

    GSFolder()
    {
        id         = QLatin1String("-1");
        canComment = true;
    }

public:

    QString     id;
    QString     title;
    QString     timestamp;
    QString     description;
    QString     location;
    QString     access;
    bool        canComment;
    QStringList tags;
};

} // namespace Digikam

#endif // GS_ITEM_H
