/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-30
 * Description : Class for geonames.org based altitude lookup
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

#include "lookupaltitudegeonames.h"

// Qt includes

#include <QNetworkAccessManager>
#include <QUrlQuery>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "geoifacetypes.h"

namespace Digikam
{

class MergedRequests
{
public:

    QList<QPair<GeoCoordinates, QIntList> > groupedRequestIndices;

    typedef QList<MergedRequests> List;

    bool addRequestIfCoordinatesAreThere(const LookupAltitude::Request& request, const int requestIndex)
    {
        for (int i = 0 ; i < groupedRequestIndices.size() ; ++i)
        {
            if (groupedRequestIndices.at(i).first.sameLonLatAs(request.coordinates))
            {
                groupedRequestIndices[i].second << requestIndex;
                return true;
            }
        }

        return false;
    }
};

// ------------------------------------------------------------

class Q_DECL_HIDDEN LookupAltitudeGeonames::Private
{
public:

    Private()
      : status(StatusSuccess),
        currentMergedRequestIndex(0),
        netReply(0)

    {
    }

    Request::List        requests;
    MergedRequests::List mergedRequests;
    StatusAltitude       status;
    QString              errorMessage;

    int                  currentMergedRequestIndex;

    QNetworkReply*       netReply;
};

// ------------------------------------------------------------

LookupAltitudeGeonames::LookupAltitudeGeonames(QObject* const parent)
    : LookupAltitude(parent),
      d(new Private)
{
}

LookupAltitudeGeonames::~LookupAltitudeGeonames()
{
}

QString LookupAltitudeGeonames::backendName() const
{
    return QLatin1String("geonames");
}

QString LookupAltitudeGeonames::backendHumanName() const
{
    return i18n("geonames.org");
}

void LookupAltitudeGeonames::addRequests(const Request::List& requests)
{
    d->requests << requests;
}

LookupAltitude::Request::List LookupAltitudeGeonames::getRequests() const
{
    return d->requests;
}

LookupAltitude::Request LookupAltitudeGeonames::getRequest(const int index) const
{
    return d->requests.at(index);
}

void LookupAltitudeGeonames::startLookup()
{
    MergedRequests currentMergedRequest;

    for (int i = 0 ; i < d->requests.size() ; ++i)
    {
        const Request& currentRequest = d->requests.at(i);

        // is there another request with the same coordinates?
        bool requestAdded = currentMergedRequest.addRequestIfCoordinatesAreThere(currentRequest, i);

        for (int j = 0 ; (!requestAdded) && j < d->mergedRequests.size() ; ++j)
        {
            requestAdded = d->mergedRequests[j].addRequestIfCoordinatesAreThere(currentRequest, i);
        }

        if (!requestAdded)
        {
            currentMergedRequest.groupedRequestIndices.append(QPair<GeoCoordinates, QIntList>(currentRequest.coordinates, QIntList()<<i));

            if (currentMergedRequest.groupedRequestIndices.size() >= (20-1))
            {
                d->mergedRequests << currentMergedRequest;
                currentMergedRequest = MergedRequests();
            }
        }
    }

    if (!currentMergedRequest.groupedRequestIndices.isEmpty())
    {
        d->mergedRequests << currentMergedRequest;
    }

    // all requests have been grouped into batches of 20, now start the first one
    d->currentMergedRequestIndex = -1;
    startNextRequest();
}

void LookupAltitudeGeonames::startNextRequest()
{
    ++(d->currentMergedRequestIndex);

    if (d->currentMergedRequestIndex >= d->mergedRequests.count())
    {
        d->status = StatusSuccess;
        emit(signalDone());
        return;
    }

    const MergedRequests& currentMergedRequest = d->mergedRequests.at(d->currentMergedRequestIndex);

    QString latString;
    QString lonString;

    for (int i = 0; i < currentMergedRequest.groupedRequestIndices.count(); ++i)
    {
        const QPair<GeoCoordinates, QIntList>& currentPair = currentMergedRequest.groupedRequestIndices.at(i);
        const GeoCoordinates requestCoordinates            = currentPair.first;

        if (!latString.isEmpty())
        {
            latString += QLatin1Char(',');
            lonString += QLatin1Char(',');
        }

        latString += requestCoordinates.latString();
        lonString += requestCoordinates.lonString();
    }

    QUrl netUrl(QLatin1String("http://api.geonames.org/srtm3"));

    QUrlQuery q(netUrl);
    q.addQueryItem(QLatin1String("lats"), latString);
    q.addQueryItem(QLatin1String("lngs"), lonString);
    q.addQueryItem(QLatin1String("username"), QLatin1String("digikam"));
    netUrl.setQuery(q);

    QNetworkAccessManager* const mngr = new QNetworkAccessManager(this);

    connect(mngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    d->netReply = mngr->get(QNetworkRequest(netUrl));
}

void LookupAltitudeGeonames::slotFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        d->errorMessage = reply->errorString();
        d->status       = StatusError;

        // after an error, we abort:
        reply->deleteLater();
        emit(signalDone());
        return;
    }

    QByteArray data                            = reply->readAll();
    const QStringList altitudes                = QString::fromLatin1(data).split(QRegExp(QLatin1String("\\s+")));
    const MergedRequests& currentMergedRequest = d->mergedRequests.at(d->currentMergedRequestIndex);
    QIntList readyRequests;

    for (int i = 0 ; i < qMin(altitudes.count(), currentMergedRequest.groupedRequestIndices.count()) ; ++i)
    {
        const QString& altitudeString = altitudes.at(i);
        bool haveAltitude             = false;
        const qreal altitude          = altitudeString.toFloat(&haveAltitude);

        // -32786 means that geonames.org has no data for these coordinates
        if (altitude == -32768)
        {
            haveAltitude = false;
        }

        const QIntList& currentRequestIndexes = currentMergedRequest.groupedRequestIndices.at(i).second;

        foreach(const int requestIndex, currentRequestIndexes)
        {
            if (haveAltitude)
            {
                d->requests[requestIndex].coordinates.setAlt(altitude);
            }
            else
            {
                d->requests[requestIndex].coordinates.clearAlt();
            }

            // The request has been carried out. Even if no altitude was
            // found, we return success.
            d->requests[requestIndex].success = true;
        }

        readyRequests << currentRequestIndexes;
    }

    emit(signalRequestsReady(readyRequests));

    reply->deleteLater();

    startNextRequest();
}

LookupAltitude::StatusAltitude LookupAltitudeGeonames::getStatus() const
{
    return d->status;
}

QString LookupAltitudeGeonames::errorMessage() const
{
    return d->errorMessage;
}

void LookupAltitudeGeonames::cancel()
{
    if (d->netReply && !d->netReply->isFinished())
    {
        d->netReply->abort();
    }

    d->status = StatusCanceled;

    emit(signalDone());
}

} // namespace Digikam
