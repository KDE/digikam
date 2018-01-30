/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a kipi plugin to export images to Google-Drive web service
 *
 * Copyright (C) 2013 by Pankaj Kumar <me at panks dot me>
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

#ifndef GSITEM_H
#define GSITEM_H

// Qt includes

#include <QUrl>
#include <QString>

namespace KIPIGoogleServicesPlugin
{

enum PluginName
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

} // namespace KIPIGoogleServicesPlugin

#endif // GSITEM_H
