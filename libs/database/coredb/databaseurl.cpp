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

#include <QUrlQuery>
#include <QStringList>

// Local includes

#include "collectionmanager.h"
#include "collectionlocation.h"
#include "digikam_debug.h"

namespace Digikam
{

DatabaseUrl DatabaseUrl::fromFileUrl(const QUrl& fileUrl,
                                     const QUrl& albumRoot,
                                     const DatabaseParameters& parameters)
{
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRoot(albumRoot);

    return fromFileUrl(fileUrl, albumRoot, location.id(), parameters);
}

DatabaseUrl DatabaseUrl::fromFileUrl(const QUrl& fileUrl,
                                     const QUrl& albumRoot,
                                     int   albumRootId,
                                     const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setScheme(QLatin1String("digikamalbums"));
    // get album root path without trailing slash
    QString albumRootPath = albumRoot.adjusted(QUrl::StripTrailingSlash).toLocalFile();
    // get the hierarchy below the album root
    QString pathUnderRoot = fileUrl.toLocalFile().remove(albumRootPath);
    url.setPath(pathUnderRoot);

    QUrlQuery q(url);
    q.addQueryItem(QLatin1String("albumRoot"),   albumRootPath);
    q.addQueryItem(QLatin1String("albumRootId"), QString::number(albumRootId));
    url.setQuery(q);
    
    url.setParameters(parameters);

    qCDebug(DIGIKAM_DATABASE_LOG) << "DatabaseUrl::fromFileUrl : " << url.toDisplayString();

    return url;
}

DatabaseUrl DatabaseUrl::fromAlbumAndName(const QString& name,
        const QString& album,
        const QUrl& albumRoot,
        const DatabaseParameters& parameters)
{
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRoot(albumRoot);

    return fromAlbumAndName(name, album, albumRoot, location.id(), parameters);
}

DatabaseUrl DatabaseUrl::fromAlbumAndName(const QString& name,
        const QString& album,
        const QUrl& albumRoot,
        int   albumRootId,
        const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setScheme(QLatin1String("digikamalbums"));
    url.setPath(QLatin1String("/"));

    url.setPath(url.path() + QLatin1Char('/') + album + QLatin1Char('/'));
    url.setPath(url.path() + QLatin1Char('/') + name);

    QUrlQuery q(url);
    q.addQueryItem(QLatin1String("albumRoot"),   albumRoot.adjusted(QUrl::StripTrailingSlash).toLocalFile());
    q.addQueryItem(QLatin1String("albumRootId"), QString::number(albumRootId));
    url.setQuery(q);
    url.setParameters(parameters);

    //qCDebug(DIGIKAM_DATABASE_LOG) << "DatabaseUrl::fromAlbumAndName : " << url.toDisplayString();

    return url;
}

DatabaseUrl DatabaseUrl::albumUrl(const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setScheme(QLatin1String("digikamalbums"));
    url.setParameters(parameters);

    qCDebug(DIGIKAM_DATABASE_LOG) << "DatabaseUrl::albumUrl : " << url.toDisplayString();
    return url;
}

DatabaseUrl DatabaseUrl::fromTagIds(const QList<int>& tagIds,
                                    const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setScheme(QLatin1String("digikamtags"));

    for (QList<int>::const_iterator it = tagIds.constBegin(); it != tagIds.constEnd(); ++it)
    {
        url.setPath(url.path() + QLatin1Char('/') + QString::number(*it));
    }

    url.setParameters(parameters);

    qCDebug(DIGIKAM_DATABASE_LOG) << "DatabaseUrl::fromTagIds : " << url.toDisplayString();
    return url;
}

DatabaseUrl DatabaseUrl::dateUrl(const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setScheme(QLatin1String("digikamdates"));
    url.setParameters(parameters);

    qCDebug(DIGIKAM_DATABASE_LOG) << "DatabaseUrl::dateUrl : " << url.toDisplayString();
    return url;
}

DatabaseUrl DatabaseUrl::fromDateForMonth(const QDate& date, const DatabaseParameters& parameters)
{
    QDate firstDayOfMonth(date.year(), date.month(), 1);
    QDate firstDayOfNextMonth = firstDayOfMonth.addMonths(1);

    return ( fromDateRange(firstDayOfMonth, firstDayOfNextMonth, parameters) );
}

DatabaseUrl DatabaseUrl::fromDateForYear(const QDate& date, const DatabaseParameters& parameters)
{
    QDate firstDayOfYear(date.year(), 1, 1);
    QDate firstDayOfNextYear = firstDayOfYear.addYears(1);

    return ( fromDateRange(firstDayOfYear, firstDayOfNextYear, parameters) );
}

DatabaseUrl DatabaseUrl::fromDateRange(const QDate& startDate,
                                       const QDate& endDate,
                                       const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setScheme(QLatin1String("digikamdates"));
    url.setPath(startDate.toString(Qt::ISODate) + QLatin1Char('/') + endDate.toString(Qt::ISODate));
    url.setParameters(parameters);

    qCDebug(DIGIKAM_DATABASE_LOG) << "DatabaseUrl::fromDateRange : " << url.toDisplayString();
    return url;
}

DatabaseUrl DatabaseUrl::mapImagesUrl(const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setScheme(QLatin1String("digikammapimages"));
    url.setParameters(parameters);

    qCDebug(DIGIKAM_DATABASE_LOG) << "DatabaseUrl::mapImagesUrl : " << url.toDisplayString();
    return url;
}

DatabaseUrl DatabaseUrl::fromAreaRange(const qreal lat1, const qreal lat2,
                                       const qreal lng1, const qreal lng2,
                                       const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setScheme(QLatin1String("digikammapimages"));

    QUrlQuery q(url);
    q.addQueryItem(QLatin1String("lat1"), QString::number(lat1));
    q.addQueryItem(QLatin1String("lon1"), QString::number(lng1));
    q.addQueryItem(QLatin1String("lat2"), QString::number(lat2));
    q.addQueryItem(QLatin1String("lon2"), QString::number(lng2));
    url.setQuery(q);

    url.setParameters(parameters);

    qCDebug(DIGIKAM_DATABASE_LOG) << "DatabaseUrl::fromAreaRange : " << url.toDisplayString();
    return url;
}

DatabaseUrl DatabaseUrl::searchUrl(int id, const DatabaseParameters& parameters)
{
    DatabaseUrl url;
    url.setScheme(QLatin1String("digikamsearch"));
    
    QUrlQuery q(url);
    q.addQueryItem(QLatin1String("searchId"), QString::number(id));
    url.setQuery(q);
    
    url.setParameters(parameters);

    qCDebug(DIGIKAM_DATABASE_LOG) << "DatabaseUrl::searchUrl : " << url.toDisplayString();
    return url;
}

DatabaseUrl::DatabaseUrl(const QUrl& digikamalbumsUrl)
    : QUrl(digikamalbumsUrl)
{
}

DatabaseUrl::DatabaseUrl(const DatabaseUrl& url)
    : QUrl(url)
{
}

DatabaseUrl::DatabaseUrl()
{
}

DatabaseUrl& DatabaseUrl::operator=(const QUrl& digikamalbumsUrl)
{
    QUrl::operator=(digikamalbumsUrl);

    return *this;
}

DatabaseUrl& DatabaseUrl::operator=(const DatabaseUrl& url)
{
    QUrl::operator=(url);

    return *this;
}

bool DatabaseUrl::operator==(const QUrl& digikamalbumsUrl) const
{
    return ( QUrl::operator==(digikamalbumsUrl) );
}

/*
DatabaseUrl::operator DatabaseParameters() const
{
    return parameters();
}
*/

// --- Database parameters ---------------------------------------------------------------------

DatabaseParameters DatabaseUrl::parameters() const
{
    return DatabaseParameters(*this);
}

void DatabaseUrl::setParameters(const DatabaseParameters& parameters)
{
    parameters.insertInUrl(*this);
}

// --- Protocol --------------------------------------------------------------------------------

bool DatabaseUrl::isAlbumUrl() const
{
    return ( scheme() == QLatin1String("digikamalbums") );
}

bool DatabaseUrl::isTagUrl() const
{
    return ( scheme() == QLatin1String("digikamtags") );
}

bool DatabaseUrl::isDateUrl() const
{
    return ( scheme() == QLatin1String("digikamdates") );
}

bool DatabaseUrl::isMapImagesUrl() const
{
    return ( scheme() == QLatin1String("digikammapimages") );
}

bool DatabaseUrl::isSearchUrl() const
{
    return ( scheme() == QLatin1String("digikamsearch") );
}

// --- Album URL ----------------------------------------------------------------------------

QUrl DatabaseUrl::albumRoot() const
{
    QString albumRoot = QUrlQuery(*this).queryItemValue(QLatin1String("albumRoot"));

    if (!albumRoot.isNull())
    {
        QUrl albumRootUrl;
        albumRootUrl.setPath(albumRoot);
        return albumRootUrl;
    }

    return QUrl();
}

QString DatabaseUrl::albumRootPath() const
{
    return ( QUrlQuery(*this).queryItemValue(QLatin1String("albumRoot")));
}

int DatabaseUrl::albumRootId() const
{
    return ( QUrlQuery(*this).queryItemValue(QLatin1String("albumRootId")).toInt() );
}

QString DatabaseUrl::album() const
{
    // obey trailing slash in the path - albums have a trailing slash
    // get result without trailing slash
    QUrl url = adjusted(QUrl::RemoveFilename);
    
    return ( url.adjusted(QUrl::StripTrailingSlash).path() );
}

QString DatabaseUrl::name() const
{
    // do not ignore trailing slash in the path - albums have a trailing slash
    return fileName();
}

QUrl DatabaseUrl::fileUrl() const
{
    QUrl fileUrl = QUrl::fromLocalFile(albumRoot().toString() + path());

    return fileUrl;
}

// --- Tag URL ------------------------------------------------------------------------

int DatabaseUrl::tagId() const
{
    if (path() == QLatin1String("/"))
    {
        return (-1);
    }

    return ( fileName().toInt() );
}

QList<int> DatabaseUrl::tagIds() const
{
    QList<int>  ids;
    QStringList stringIds = path().split(QLatin1Char('/'), QString::SkipEmptyParts);

    for (int i=0; i<stringIds.count(); ++i)
    {
        ids << stringIds.at(i).toInt();
    }

    return ids;
}

// --- Date URL -----------------------------------------------------------------------

QDate DatabaseUrl::startDate() const
{
    QStringList dates = path().split(QLatin1Char('/'));

    if (dates.size() >= 1)
    {
        return QDate::fromString(dates.at(0), Qt::ISODate);
    }

    return QDate();
}

QDate DatabaseUrl::endDate() const
{
    QStringList dates = path().split(QLatin1Char('/'));

    if (dates.size() >= 2)
    {
        return QDate::fromString(dates.at(1), Qt::ISODate);
    }
    else
    {
        return QDate();
    }
}

// --- MapImages URL --------------------------------------------------------------------------------

bool DatabaseUrl::areaCoordinates(double* lat1, double* lat2, double* lon1, double* lon2) const
{
    bool ok, allOk = true;
    *lat1 = QUrlQuery(*this).queryItemValue(QLatin1String("lat1")).toDouble(&ok);
    allOk = ok && allOk;
    *lat2 = QUrlQuery(*this).queryItemValue(QLatin1String("lat2")).toDouble(&ok);
    allOk = ok && allOk;
    *lon1 = QUrlQuery(*this).queryItemValue(QLatin1String("lon1")).toDouble(&ok);
    allOk = ok && allOk;
    *lon2 = QUrlQuery(*this).queryItemValue(QLatin1String("lon2")).toDouble(&ok);
    allOk = ok && allOk;

    return allOk;
}

// --- Search URL --------------------------------------------------------------------

int DatabaseUrl::searchId() const
{
    return QUrlQuery(*this).queryItemValue(QLatin1String("searchId")).toInt();
}

}  // namespace Digikam
