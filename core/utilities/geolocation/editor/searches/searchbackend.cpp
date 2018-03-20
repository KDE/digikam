/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-01
 * Description : A simple backend to search OSM and Geonames.org.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#include "searchbackend.h"

// Qt includes

#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QUrlQuery>

// KDE includes

#include <klocalizedstring.h>

// local includes

#include "gpscommon.h"

namespace Digikam
{

class SearchBackend::Private
{
public:

    Private()
      : results(),
        netReply(0),
        runningBackend(),
        searchData(),
        errorMessage()
    {
    }

    SearchBackend::SearchResult::List results;
    QNetworkReply*                    netReply;
    QString                           runningBackend;
    QByteArray                        searchData;
    QString                           errorMessage;
};

SearchBackend::SearchBackend(QObject* const parent)
    : QObject(parent),
      d(new Private())
{
}

SearchBackend::~SearchBackend()
{
    delete d;
}

bool SearchBackend::search(const QString& backendName, const QString& searchTerm)
{
    d->searchData.clear();
    d->errorMessage.clear();
    d->results.clear();

    QNetworkAccessManager* const mngr = new QNetworkAccessManager(this);

    connect(mngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    if (backendName == QLatin1String("osm"))
    {
        d->runningBackend = backendName;

        QUrl netUrl(QLatin1String("http://nominatim.openstreetmap.org/search"));

        QUrlQuery q(netUrl);
        q.addQueryItem(QLatin1String("format"), QLatin1String("xml"));
        q.addQueryItem(QLatin1String("q"), searchTerm);
        netUrl.setQuery(q);

        QNetworkRequest netRequest(netUrl);
        netRequest.setRawHeader("User-Agent", getUserAgentName().toLatin1());

        d->netReply = mngr->get(netRequest);

        return true;
    }

    if (backendName == QLatin1String("geonames.org"))
    {
        d->runningBackend = backendName;

        // documentation: http://www.geonames.org/export/geonames-search.html

        QUrl netUrl(QLatin1String("http://api.geonames.org/search"));

        QUrlQuery q(netUrl);
        q.addQueryItem(QLatin1String("type"), QLatin1String("xml"));
        q.addQueryItem(QLatin1String("q"), searchTerm);
        q.addQueryItem(QLatin1String("username"), QLatin1String("digikam"));
        netUrl.setQuery(q);

        QNetworkRequest netRequest(netUrl);
        netRequest.setRawHeader("User-Agent", getUserAgentName().toLatin1());

        d->netReply = mngr->get(netRequest);

        return true;
    }

    return false;
}

void SearchBackend::slotFinished(QNetworkReply* reply)
{
    if (reply != d->netReply)
    {
        return;
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        d->errorMessage = reply->errorString();
        emit(signalSearchCompleted());
        reply->deleteLater();
        return;
    }

    d->searchData.append(reply->readAll());

    const QString resultString = QString::fromUtf8(d->searchData.constData(), d->searchData.count());

    if (d->runningBackend == QLatin1String("osm"))
    {
        QDomDocument doc;
        doc.setContent(resultString); // error-handling
        QDomElement docElement = doc.documentElement(); // error-handling

        for (QDomNode resultNode = docElement.firstChild(); !resultNode.isNull(); resultNode = resultNode.nextSibling())
        {
            QDomElement resultElement = resultNode.toElement();

            if (resultElement.isNull())
            {
                continue;
            }

            if (resultElement.tagName() != QLatin1String("place"))
            {
                continue;
            }

            const QString boundingBoxString = resultElement.attribute(QLatin1String("boundingbox"));
            const QString latString         = resultElement.attribute(QLatin1String("lat"));
            const QString lonString         = resultElement.attribute(QLatin1String("lon"));
            const QString displayName       = resultElement.attribute(QLatin1String("display_name"));
            const QString placeId           = resultElement.attribute(QLatin1String("place_id"));

            if (latString.isEmpty() || lonString.isEmpty() || displayName.isEmpty())
            {
                continue;
            }

            // now parse the strings:
            qreal lat;
            qreal lon;
            bool okay = false;
            lat       = latString.toDouble(&okay);

            if (okay)
            {
                lon = lonString.toDouble(&okay);
            }

            if (!okay)
            {
                continue;
            }

            SearchResult result;
            result.coordinates = GeoCoordinates(lat, lon);
            result.name        = displayName;

            if (!placeId.isEmpty())
            {
                result.internalId = QLatin1String("osm-") + placeId;
            }

            // TODO: parse bounding box

            d->results << result;
        }
    }
    else if (d->runningBackend == QLatin1String("geonames.org"))
    {
        QDomDocument doc;
        doc.setContent(resultString); // error-handling
        QDomElement docElement = doc.documentElement(); // error-handling
        qCDebug(DIGIKAM_GENERAL_LOG)<<docElement.toElement().tagName();

        for (QDomNode resultNode = docElement.firstChild(); !resultNode.isNull(); resultNode = resultNode.nextSibling())
        {
            QDomElement resultElement = resultNode.toElement();
            qCDebug(DIGIKAM_GENERAL_LOG) << resultElement.tagName();

            if (resultElement.isNull())
            {
                continue;
            }

            if (resultElement.tagName() != QLatin1String("geoname"))
            {
                continue;
            }

            QString latString;
            QString lonString;
            QString displayName;
            QString geoNameId;

            for (QDomNode resultSubNode = resultElement.firstChild(); !resultSubNode.isNull(); resultSubNode = resultSubNode.nextSibling())
            {
                QDomElement resultSubElement = resultSubNode.toElement();

                if (resultSubElement.isNull())
                {
                    continue;
                }

                if (resultSubElement.tagName() == QLatin1String("lat"))
                {
                    latString = resultSubElement.text();
                }
                else if (resultSubElement.tagName() == QLatin1String("lng"))
                {
                    lonString = resultSubElement.text();
                }
                else if (resultSubElement.tagName() == QLatin1String("name"))
                {
                    displayName = resultSubElement.text();
                }
                else if (resultSubElement.tagName() == QLatin1String("geonameId"))
                {
                    geoNameId = resultSubElement.text();
                }
            }

            if (latString.isEmpty()||lonString.isEmpty()||displayName.isEmpty())
            {
                continue;
            }

            // now parse the strings:
            qreal lat;
            qreal lon;
            bool okay = false;
            lat       = latString.toDouble(&okay);

            if (okay)
            {
                lon = lonString.toDouble(&okay);
            }

            if (!okay)
            {
                continue;
            }

            SearchResult result;
            result.coordinates = GeoCoordinates(lat, lon);
            result.name        = displayName;

            if (!geoNameId.isEmpty())
            {
                result.internalId = QLatin1String("geonames.org-") + geoNameId;
            }

            d->results << result;
        }
    }

    emit(signalSearchCompleted());

    reply->deleteLater();
}

SearchBackend::SearchResult::List SearchBackend::getResults() const
{
    return d->results;
}

QString SearchBackend::getErrorMessage() const
{
    return d->errorMessage;
}

QList<QPair<QString, QString> > SearchBackend::getBackends() const
{
    QList<QPair<QString, QString> > resultList;
    resultList << QPair<QString, QString>(i18n("GeoNames"), QLatin1String("geonames.org"));
    resultList << QPair<QString, QString>(i18n("OSM"), QLatin1String("osm"));

    return resultList;
}

} // namespace Digikam
