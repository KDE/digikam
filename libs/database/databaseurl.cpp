/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-19
 * Description : Handling of database specific URLs
 *
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <qstringlist.h>
//Added by qt3to4:
#include <Q3ValueList>

// Local includes

#include "databaseurl.h"

namespace Digikam
{

DatabaseUrl DatabaseUrl::fromFileUrl(const KUrl &fileUrl,
                                     const KUrl &albumRoot,
                                     const DatabaseParameters &parameters)
{
    DatabaseUrl url;
    url.setProtocol("digikamalbums");
    // get album root path without trailing slash
    QString albumRootPath = albumRoot.path(-1);
    // get the hierarchy below the album root
    QString pathUnderRoot = fileUrl.path().remove(albumRootPath);
    url.setPath(pathUnderRoot);
    url.addQueryItem("albumRoot", albumRootPath);
    url.setParameters(parameters);
    return url;
}

DatabaseUrl DatabaseUrl::fromAlbumAndName(const QString &name,
                                          const QString &album,
                                          const KUrl &albumRoot,
                                          const DatabaseParameters &parameters)
{
    DatabaseUrl url;
    url.setProtocol("digikamalbums");

    url.setPath("/");
    url.addPath(album + '/');
    url.addPath(name);

    url.addQueryItem("albumRoot", albumRoot.path(-1));
    url.setParameters(parameters);
    return url;
}

DatabaseUrl DatabaseUrl::fromTagIds(const Q3ValueList<int> tagIds,
                                   const DatabaseParameters &parameters)
{
    DatabaseUrl url;
    url.setProtocol("digikamtags");

    for (Q3ValueList<int>::const_iterator it = tagIds.begin(); it != tagIds.end(); ++it)
    {
        url.addPath('/' + QString::number(*it));
    }

    url.setParameters(parameters);
    return url;
}

DatabaseUrl DatabaseUrl::fromDate(const QDate &date,
                                  const DatabaseParameters &parameters)
{
    DatabaseUrl url;
    url.setProtocol("digikamdates");

    url.setPath(date.toString(Qt::ISODate));

    url.setParameters(parameters);
    return url;
}

DatabaseUrl DatabaseUrl::fromSearchUrl(const KUrl &searchURL,
                          const DatabaseParameters &parameters)
{
    DatabaseUrl url(searchURL);
    url.setParameters(parameters);
    return url;
}



DatabaseUrl::DatabaseUrl(const KUrl &digikamalbumsUrl)
    : KUrl(digikamalbumsUrl)
{
}

DatabaseUrl::DatabaseUrl(const DatabaseUrl &url)
    : KUrl(url)
{
}

DatabaseUrl::DatabaseUrl()
{
}

DatabaseUrl &DatabaseUrl::operator=(const KUrl &digikamalbumsUrl)
{
    KUrl::operator=(digikamalbumsUrl);
    return *this;
}

DatabaseUrl &DatabaseUrl::operator=(const DatabaseUrl &url)
{
    KUrl::operator=(url);
    return *this;
}

bool DatabaseUrl::operator==(const KUrl &digikamalbumsUrl)
{
    return KUrl::operator==(digikamalbumsUrl);
}

/*
DatabaseUrl::operator DatabaseParameters() const
{
    return parameters();
}*/



// --- Database parameters ---

DatabaseParameters DatabaseUrl::parameters() const
{
    return DatabaseParameters(*this);
}

void DatabaseUrl::setParameters(const DatabaseParameters &parameters)
{
    parameters.insertInUrl(*this);
}


// --- Protocol ---

bool DatabaseUrl::isAlbumUrl() const
{
    return protocol() == QString("digikamalbums");
}

bool DatabaseUrl::isTagUrl() const
{
    return protocol() == QString("digikamtags");
}

bool DatabaseUrl::isDateUrl() const
{
    return protocol() == QString("digikamdates");
}

bool DatabaseUrl::isSearchUrl() const
{
    return protocol() == QString("digikamsearch");
}


// --- Album URL ---

KUrl DatabaseUrl::albumRoot() const
{
    QString albumRoot = queryItem("albumRoot");
    if (!albumRoot.isNull())
    {
        KUrl albumRootUrl;
        albumRootUrl.setPath(albumRoot);
        return albumRootUrl;
    }
    return KUrl();
}

QString DatabaseUrl::albumRootPath() const
{
    return queryItem("albumRoot");
}

QString DatabaseUrl::album() const
{
    // strip the trailing slash from result
    // do not ignore trailing slash in the path - albums have a trailing slash
    return directory(true, false);
}

QString DatabaseUrl::name() const
{
    // do not ignore trailing slash in the path - albums have a trailing slash
    return fileName(false);
}

KUrl DatabaseUrl::fileUrl() const
{
    KUrl fileUrl(albumRoot());
    fileUrl.addPath(path());
    return fileUrl;
}


// --- Tag URL ---

int DatabaseUrl::tagId() const
{
    if (path() == "/")
        return -1;
    return fileName().toInt();
}

Q3ValueList<int> DatabaseUrl::tagIds() const
{
    Q3ValueList<int> ids;
    QStringList stringIds = QStringList::split('/', path());
    for (QStringList::iterator it = stringIds.begin(); it != stringIds.end(); ++it)
    {
        ids << (*it).toInt();
    }
    return ids;
}

// --- Date URL ---

QDate DatabaseUrl::date() const
{
    return QDate::fromString(path(), Qt::ISODate);
}

// --- Search URL ---

KUrl DatabaseUrl::searchUrl() const
{
    KUrl url(*this);
    DatabaseParameters::removeFromUrl(url);
    return url;
}



}

