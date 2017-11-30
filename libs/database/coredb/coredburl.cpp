/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-19
 * Description : Core database Url interface.
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

#include "coredburl.h"

// Qt includes

#include <QUrlQuery>
#include <QStringList>

// Local includes

#include "collectionmanager.h"
#include "collectionlocation.h"
#include "digikam_debug.h"

namespace Digikam
{

CoreDbUrl CoreDbUrl::fromFileUrl(const QUrl& fileUrl,
                                 const QUrl& albumRoot,
                                 const DbEngineParameters& parameters)
{
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRoot(albumRoot);

    return fromFileUrl(fileUrl, albumRoot, location.id(), parameters);
}

CoreDbUrl CoreDbUrl::fromFileUrl(const QUrl& fileUrl,
                                 const QUrl& albumRoot,
                                 int   albumRootId,
                                 const DbEngineParameters& parameters)
{
    CoreDbUrl url;
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

//    qCDebug(DIGIKAM_COREDB_LOG) << "CoreDbUrl::fromFileUrl : " << url.toDisplayString();

    return url;
}

CoreDbUrl CoreDbUrl::fromAlbumAndName(const QString& name,
                                      const QString& album,
                                      const QUrl& albumRoot,
                                      const DbEngineParameters& parameters)
{
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRoot(albumRoot);

    return fromAlbumAndName(name, album, albumRoot, location.id(), parameters);
}

CoreDbUrl CoreDbUrl::fromAlbumAndName(const QString& name,
                                      const QString& album,
                                      const QUrl& albumRoot,
                                      int   albumRootId,
                                      const DbEngineParameters& parameters)
{
    CoreDbUrl url;
    QString path(album);
    url.setScheme(QLatin1String("digikamalbums"));

    if (path != QLatin1String("/"))
        path += QLatin1Char('/');

    url.setPath(path + name);

    QUrlQuery q(url);
    q.addQueryItem(QLatin1String("albumRoot"),   albumRoot.adjusted(QUrl::StripTrailingSlash).toLocalFile());
    q.addQueryItem(QLatin1String("albumRootId"), QString::number(albumRootId));
    url.setQuery(q);
    url.setParameters(parameters);

//    qCDebug(DIGIKAM_COREDB_LOG) << "CoreDbUrl::fromAlbumAndName : " << url.toDisplayString();

    return url;
}

CoreDbUrl CoreDbUrl::albumUrl(const DbEngineParameters& parameters)
{
    CoreDbUrl url;
    url.setScheme(QLatin1String("digikamalbums"));
    url.setParameters(parameters);

    qCDebug(DIGIKAM_COREDB_LOG) << "CoreDbUrl::albumUrl : " << url.toDisplayString();
    return url;
}

CoreDbUrl CoreDbUrl::fromTagIds(const QList<int>& tagIds,
                                const DbEngineParameters& parameters)
{
    CoreDbUrl url;
    url.setScheme(QLatin1String("digikamtags"));

    for (QList<int>::const_iterator it = tagIds.constBegin(); it != tagIds.constEnd(); ++it)
    {
        url.setPath(url.path() + QLatin1Char('/') + QString::number(*it));
    }

    url.setParameters(parameters);

//    qCDebug(DIGIKAM_COREDB_LOG) << "CoreDbUrl::fromTagIds : " << url.toDisplayString();

    return url;
}

CoreDbUrl CoreDbUrl::dateUrl(const DbEngineParameters& parameters)
{
    CoreDbUrl url;
    url.setScheme(QLatin1String("digikamdates"));
    url.setParameters(parameters);

//    qCDebug(DIGIKAM_COREDB_LOG) << "CoreDbUrl::dateUrl : " << url.toDisplayString();

    return url;
}

CoreDbUrl CoreDbUrl::fromDateForMonth(const QDate& date, const DbEngineParameters& parameters)
{
    QDate firstDayOfMonth(date.year(), date.month(), 1);
    QDate firstDayOfNextMonth = firstDayOfMonth.addMonths(1);

    return ( fromDateRange(firstDayOfMonth, firstDayOfNextMonth, parameters) );
}

CoreDbUrl CoreDbUrl::fromDateForYear(const QDate& date, const DbEngineParameters& parameters)
{
    QDate firstDayOfYear(date.year(), 1, 1);
    QDate firstDayOfNextYear = firstDayOfYear.addYears(1);

    return ( fromDateRange(firstDayOfYear, firstDayOfNextYear, parameters) );
}

CoreDbUrl CoreDbUrl::fromDateRange(const QDate& startDate,
                                   const QDate& endDate,
                                   const DbEngineParameters& parameters)
{
    CoreDbUrl url;
    url.setScheme(QLatin1String("digikamdates"));
    url.setPath(startDate.toString(Qt::ISODate) + QLatin1Char('/') + endDate.toString(Qt::ISODate));
    url.setParameters(parameters);

//    qCDebug(DIGIKAM_COREDB_LOG) << "CoreDbUrl::fromDateRange : " << url.toDisplayString();

    return url;
}

CoreDbUrl CoreDbUrl::mapImagesUrl(const DbEngineParameters& parameters)
{
    CoreDbUrl url;
    url.setScheme(QLatin1String("digikammapimages"));
    url.setParameters(parameters);

//    qCDebug(DIGIKAM_COREDB_LOG) << "CoreDbUrl::mapImagesUrl : " << url.toDisplayString();

    return url;
}

CoreDbUrl CoreDbUrl::fromAreaRange(const qreal lat1, const qreal lat2,
                                   const qreal lng1, const qreal lng2,
                                   const DbEngineParameters& parameters)
{
    CoreDbUrl url;
    url.setScheme(QLatin1String("digikammapimages"));

    QUrlQuery q(url);
    q.addQueryItem(QLatin1String("lat1"), QString::number(lat1));
    q.addQueryItem(QLatin1String("lon1"), QString::number(lng1));
    q.addQueryItem(QLatin1String("lat2"), QString::number(lat2));
    q.addQueryItem(QLatin1String("lon2"), QString::number(lng2));
    url.setQuery(q);

    url.setParameters(parameters);

//    qCDebug(DIGIKAM_COREDB_LOG) << "CoreDbUrl::fromAreaRange : " << url.toDisplayString();

    return url;
}

CoreDbUrl CoreDbUrl::searchUrl(int id, const DbEngineParameters& parameters)
{
    CoreDbUrl url;
    url.setScheme(QLatin1String("digikamsearch"));

    QUrlQuery q(url);
    q.addQueryItem(QLatin1String("searchId"), QString::number(id));
    url.setQuery(q);

    url.setParameters(parameters);

//    qCDebug(DIGIKAM_COREDB_LOG) << "CoreDbUrl::searchUrl : " << url.toDisplayString();

    return url;
}

CoreDbUrl::CoreDbUrl(const QUrl& digikamalbumsUrl)
    : QUrl(digikamalbumsUrl)
{
}

CoreDbUrl::CoreDbUrl(const CoreDbUrl& url)
    : QUrl(url)
{
}

CoreDbUrl::CoreDbUrl()
{
}

CoreDbUrl& CoreDbUrl::operator=(const QUrl& digikamalbumsUrl)
{
    QUrl::operator=(digikamalbumsUrl);

    return *this;
}

CoreDbUrl& CoreDbUrl::operator=(const CoreDbUrl& url)
{
    QUrl::operator=(url);

    return *this;
}

bool CoreDbUrl::operator==(const QUrl& digikamalbumsUrl) const
{
    return ( QUrl::operator==(digikamalbumsUrl) );
}

/*
CoreDbUrl::operator DbEngineParameters() const
{
    return parameters();
}
*/

// --- Database parameters ---------------------------------------------------------------------

DbEngineParameters CoreDbUrl::parameters() const
{
    return DbEngineParameters(*this);
}

void CoreDbUrl::setParameters(const DbEngineParameters& parameters)
{
    parameters.insertInUrl(*this);
}

// --- Protocol --------------------------------------------------------------------------------

bool CoreDbUrl::isAlbumUrl() const
{
    return ( scheme() == QLatin1String("digikamalbums") );
}

bool CoreDbUrl::isTagUrl() const
{
    return ( scheme() == QLatin1String("digikamtags") );
}

bool CoreDbUrl::isDateUrl() const
{
    return ( scheme() == QLatin1String("digikamdates") );
}

bool CoreDbUrl::isMapImagesUrl() const
{
    return ( scheme() == QLatin1String("digikammapimages") );
}

bool CoreDbUrl::isSearchUrl() const
{
    return ( scheme() == QLatin1String("digikamsearch") );
}

// --- Album URL ----------------------------------------------------------------------------

QUrl CoreDbUrl::albumRoot() const
{
    QString albumRoot = QUrlQuery(*this).queryItemValue(QLatin1String("albumRoot"));

    if (!albumRoot.isNull())
    {
        return QUrl::fromLocalFile(albumRoot);
    }

    return QUrl();
}

QString CoreDbUrl::albumRootPath() const
{
    return ( QUrlQuery(*this).queryItemValue(QLatin1String("albumRoot")));
}

int CoreDbUrl::albumRootId() const
{
    return ( QUrlQuery(*this).queryItemValue(QLatin1String("albumRootId")).toInt() );
}

QString CoreDbUrl::album() const
{
    // obey trailing slash in the path - albums have a trailing slash
    // get result without trailing slash
    QUrl url = adjusted(QUrl::RemoveFilename);

    return ( url.adjusted(QUrl::StripTrailingSlash).path() );
}

QString CoreDbUrl::name() const
{
    // do not ignore trailing slash in the path - albums have a trailing slash
    return fileName();
}

QUrl CoreDbUrl::fileUrl() const
{
    return QUrl::fromLocalFile(albumRoot().toLocalFile() + path());
}

// --- Tag URL ------------------------------------------------------------------------

int CoreDbUrl::tagId() const
{
    if (path() == QLatin1String("/"))
    {
        return (-1);
    }

    return ( fileName().toInt() );
}

QList<int> CoreDbUrl::tagIds() const
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

QDate CoreDbUrl::startDate() const
{
    QStringList dates = path().split(QLatin1Char('/'));

    if (dates.size() >= 1)
    {
        return QDate::fromString(dates.at(0), Qt::ISODate);
    }

    return QDate();
}

QDate CoreDbUrl::endDate() const
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

bool CoreDbUrl::areaCoordinates(double* lat1, double* lat2, double* lon1, double* lon2) const
{
    bool ok;
    bool allOk = true;
    *lat1      = QUrlQuery(*this).queryItemValue(QLatin1String("lat1")).toDouble(&ok);
    allOk      = ok && allOk;
    *lat2      = QUrlQuery(*this).queryItemValue(QLatin1String("lat2")).toDouble(&ok);
    allOk      = ok && allOk;
    *lon1      = QUrlQuery(*this).queryItemValue(QLatin1String("lon1")).toDouble(&ok);
    allOk      = ok && allOk;
    *lon2      = QUrlQuery(*this).queryItemValue(QLatin1String("lon2")).toDouble(&ok);
    allOk      = ok && allOk;

    return allOk;
}

// --- Search URL --------------------------------------------------------------------

int CoreDbUrl::searchId() const
{
    return QUrlQuery(*this).queryItemValue(QLatin1String("searchId")).toInt();
}

}  // namespace Digikam
