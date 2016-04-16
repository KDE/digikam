/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2011-04-30
 * @brief  Class for geonames.org based altitude lookup
 *
 * @author Copyright (C) 2010-2011 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010-2016 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
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

#include "lookupaltitudegeonames.h"
#include "geoiface_types.h"

// Qt includes

#include <QPointer>
#include <QUrlQuery>
#include <QNetworkAccessManager>

// KDE includes

#include <klocalizedstring.h>

namespace GeoIface
{

class MergedRequests
{
public:

    QList<QPair<GeoCoordinates, QIntList> > groupedRequestIndices;

    typedef QList<MergedRequests> List;

    bool addRequestIfCoordinatesAreThere(const LookupAltitude::Request& request, const int requestIndex)
    {
        for (int i = 0; i < groupedRequestIndices.size(); ++i)
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

class Q_DECL_HIDDEN LookupAltitudeGeonames::Private
{
public:

    Private()
      : status(StatusSuccess),
        currentMergedRequestIndex(0),
        netReplay(0)

    {
    }

    Request::List        requests;
    MergedRequests::List mergedRequests;
    Status               status;
    QString              errorMessage;

    int                  currentMergedRequestIndex;

    QNetworkReply*       netReplay;
};

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

    for (int i = 0; i < d->requests.size(); ++i)
    {
        const Request& currentRequest = d->requests.at(i);

        // is there another request with the same coordinates?
        bool requestAdded = currentMergedRequest.addRequestIfCoordinatesAreThere(currentRequest, i);

        for (int j = 0; (!requestAdded) && j < d->mergedRequests.size(); ++j)
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

    d->netReplay = mngr->get(QNetworkRequest(netUrl));
}

void LookupAltitudeGeonames::slotFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        d->errorMessage = reply->errorString();
        d->status       = StatusError;

        // after an error, we abort:
        emit(signalDone());
        return;
    }

    QByteArray data                            = reply->readAll();
    const QStringList altitudes                = QString::fromLatin1(data).split(QRegExp(QLatin1String("\\s+")));
    const MergedRequests& currentMergedRequest = d->mergedRequests.at(d->currentMergedRequestIndex);
    QIntList readyRequests;

    for (int i = 0; i < qMin(altitudes.count(), currentMergedRequest.groupedRequestIndices.count()); ++i)
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

    startNextRequest();
}

LookupAltitude::Status LookupAltitudeGeonames::getStatus() const
{
    return d->status;
}

QString LookupAltitudeGeonames::errorMessage() const
{
    return d->errorMessage;
}

void LookupAltitudeGeonames::cancel()
{
    if (d->netReplay && !d->netReplay->isFinished())
    {
        d->netReplay->abort();
    }

    d->status = StatusCanceled;

    emit(signalDone());
}

} /* namespace GeoIface */
