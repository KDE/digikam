/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-19
 * Description : Handling of database specific URLs
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "databaseurl.h"

// Qt includes

#include <QStringList>

// Local includes

#include "collectionmanager.h"
#include "collectionlocation.h"

// KDE includes

#include <kdebug.h>

namespace Digikam
{

DatabaseUrl DatabaseUrl::fromFileUrl(const KUrl& fileUrl,
                                     const KUrl& albumRoot,
                                     const DatabaseParameters& parameters)
{
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRoot(albumRoot);
    return fromFileUrl(fileUrl, albumRoot, location.id(), parameters);
}

DatabaseUrl DatabaseUrl::fromFileUrl(const KUrl& fileUrl,
                                     const KUrl& albumRoot,
                                     int   albumRootId,
                                     const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setProtocol("digikamalbums");
    // get album root path without trailing slash
    QString albumRootPath = albumRoot.toLocalFile(KUrl::RemoveTrailingSlash);
    // get the hierarchy below the album root
    QString pathUnderRoot = fileUrl.toLocalFile().remove(albumRootPath);
    url.setPath(pathUnderRoot);
    url.addQueryItem("albumRoot", albumRootPath);
    url.addQueryItem("albumRootId", QString::number(albumRootId));
    url.setParameters(parameters);
    
    kDebug() << "DatabaseUrl::fromFileUrl : " << url.prettyUrl();

    return url;
}

DatabaseUrl DatabaseUrl::fromAlbumAndName(const QString& name,
        const QString& album,
        const KUrl& albumRoot,
        const DatabaseParameters& parameters)
{
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRoot(albumRoot);
    return fromAlbumAndName(name, album, albumRoot, location.id(), parameters);
}

DatabaseUrl DatabaseUrl::fromAlbumAndName(const QString& name,
        const QString& album,
        const KUrl& albumRoot,
        int   albumRootId,
        const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setProtocol("digikamalbums");

    url.setPath("/");
    url.addPath(album + '/');
    url.addPath(name);

    url.addQueryItem("albumRoot", albumRoot.toLocalFile(KUrl::RemoveTrailingSlash));
    url.addQueryItem("albumRootId", QString::number(albumRootId));
    url.setParameters(parameters);
    
    kDebug() << "DatabaseUrl::fromAlbumAndName : " << url.prettyUrl();
    return url;
}

DatabaseUrl DatabaseUrl::albumUrl(const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setProtocol("digikamalbums");
    url.setParameters(parameters);

    kDebug() << "DatabaseUrl::albumUrl : " << url.prettyUrl();
    return url;
}

DatabaseUrl DatabaseUrl::fromTagIds(const QList<int>& tagIds,
                                    const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setProtocol("digikamtags");

    for (QList<int>::const_iterator it = tagIds.constBegin(); it != tagIds.constEnd(); ++it)
    {
        url.addPath(QChar('/') + QString::number(*it));
    }

    url.setParameters(parameters);
    
    kDebug() << "DatabaseUrl::fromTagIds : " << url.prettyUrl();
    return url;
}

DatabaseUrl DatabaseUrl::dateUrl(const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setProtocol("digikamdates");
    url.setParameters(parameters);
    
    kDebug() << "DatabaseUrl::dateUrl : " << url.prettyUrl();
    return url;
}

DatabaseUrl DatabaseUrl::fromDateForMonth(const QDate& date, const DatabaseParameters& parameters)
{
    QDate firstDayOfMonth(date.year(), date.month(), 1);
    QDate firstDayOfNextMonth = firstDayOfMonth.addMonths(1);

    return fromDateRange(firstDayOfMonth, firstDayOfNextMonth, parameters);
}

DatabaseUrl DatabaseUrl::fromDateForYear(const QDate& date, const DatabaseParameters& parameters)
{
    QDate firstDayOfYear(date.year(), 1, 1);
    QDate firstDayOfNextYear = firstDayOfYear.addYears(1);

    return fromDateRange(firstDayOfYear, firstDayOfNextYear, parameters);
}

DatabaseUrl DatabaseUrl::fromDateRange(const QDate& startDate,
                                       const QDate& endDate,
                                       const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setProtocol("digikamdates");
    url.setPath(startDate.toString(Qt::ISODate) + '/' + endDate.toString(Qt::ISODate));
    url.setParameters(parameters);
    
    kDebug() << "DatabaseUrl::fromDateRange : " << url.prettyUrl();
    return url;
}

DatabaseUrl DatabaseUrl::mapImagesUrl(const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setProtocol("digikammapimages");
    url.setParameters(parameters);
    
    kDebug() << "DatabaseUrl::mapImagesUrl : " << url.prettyUrl();
    return url;
}

DatabaseUrl DatabaseUrl::fromAreaRange(const qreal lat1, const qreal lat2,
                                       const qreal lng1, const qreal lng2,
                                       const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setProtocol("digikammapimages");
    url.addQueryItem("lat1", QString::number(lat1));
    url.addQueryItem("lon1", QString::number(lng1));
    url.addQueryItem("lat2", QString::number(lat2));
    url.addQueryItem("lon2", QString::number(lng2));
    url.setParameters(parameters);
    
    kDebug() << "DatabaseUrl::fromAreaRange : " << url.prettyUrl();    
    return url;
}

DatabaseUrl DatabaseUrl::searchUrl(int id,
                                   const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setProtocol("digikamsearch");
    url.addQueryItem("searchId", QString::number(id));
    url.setParameters(parameters);
    
    kDebug() << "DatabaseUrl::searchUrl : " << url.prettyUrl();
    return url;
}

DatabaseUrl::DatabaseUrl(const KUrl& digikamalbumsUrl)
    : KUrl(digikamalbumsUrl)
{
}

DatabaseUrl::DatabaseUrl(const DatabaseUrl& url)
    : KUrl(url)
{
}

DatabaseUrl::DatabaseUrl()
{
}

DatabaseUrl& DatabaseUrl::operator=(const KUrl& digikamalbumsUrl)
{
    KUrl::operator=(digikamalbumsUrl);
    return *this;
}

DatabaseUrl& DatabaseUrl::operator=(const DatabaseUrl& url)
{
    KUrl::operator=(url);
    return *this;
}

bool DatabaseUrl::operator==(const KUrl& digikamalbumsUrl) const
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

void DatabaseUrl::setParameters(const DatabaseParameters& parameters)
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

bool DatabaseUrl::isMapImagesUrl() const
{
    return protocol() == QString("digikammapimages");
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

int DatabaseUrl::albumRootId() const
{
    return queryItem("albumRootId").toInt();
}

QString DatabaseUrl::album() const
{
    // obey trailing slash in the path - albums have a trailing slash
    // get result without trailing slash
    return directory(KUrl::ObeyTrailingSlash);
}

QString DatabaseUrl::name() const
{
    // do not ignore trailing slash in the path - albums have a trailing slash
    return fileName(KUrl::ObeyTrailingSlash);
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
    {
        return -1;
    }

    return fileName().toInt();
}

QList<int> DatabaseUrl::tagIds() const
{
    QList<int> ids;
    QStringList stringIds = path().split('/', QString::SkipEmptyParts);

    for (int i=0; i<stringIds.count(); ++i)
    {
        ids << stringIds.at(i).toInt();
    }

    return ids;
}

// --- Date URL ---

QDate DatabaseUrl::startDate() const
{
    QStringList dates = path().split('/');

    if (dates.size() >= 1)
    {
        return QDate::fromString(dates.at(0), Qt::ISODate);
    }
    else
    {
        return QDate();
    }
}

QDate DatabaseUrl::endDate() const
{
    QStringList dates = path().split('/');

    if (dates.size() >= 2)
    {
        return QDate::fromString(dates.at(1), Qt::ISODate);
    }
    else
    {
        return QDate();
    }
}

// --- MapImages URL ---

bool DatabaseUrl::areaCoordinates(double* lat1, double* lat2, double* lon1, double* lon2) const
{
    bool ok, allOk = true;
    *lat1 = queryItem("lat1").toDouble(&ok);
    allOk = ok && allOk;
    *lat2 = queryItem("lat2").toDouble(&ok);
    allOk = ok && allOk;
    *lon1 = queryItem("lon1").toDouble(&ok);
    allOk = ok && allOk;
    *lon2 = queryItem("lon2").toDouble(&ok);
    allOk = ok && allOk;

    return allOk;
}

// --- Search URL ---

int DatabaseUrl::searchId() const
{
    return queryItem("searchId").toInt();
}

}  // namespace Digikam
