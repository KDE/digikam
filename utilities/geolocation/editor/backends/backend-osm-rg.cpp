/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : OSM Nominatim backend for Reverse Geocoding
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
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

#include "backend-osm-rg.h"

// Qt includes

#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QUrlQuery>
#include <QTimer>

// Local includes

#include "digikam_debug.h"
#include "gpscommon.h"

namespace Digikam
{

class OsmInternalJobs
{
public:

    OsmInternalJobs()
      : language(),
        request(),
        data(),
        netReply(0)
    {
    }

    ~OsmInternalJobs()
    {
        if (netReply)
            netReply->deleteLater();
    }

    QString            language;
    QList<RGInfo>      request;
    QByteArray         data;
    QNetworkReply*     netReply;
};

class BackendOsmRG::Private
{
public:

    Private()
      : jobs(),
        errorMessage()
    {
    }

    QList<OsmInternalJobs> jobs;
    QString                errorMessage;
};

/**
 * @class BackendOsmRG
 *
 * @brief This class calls Open Street Map's reverse geocoding service.
 */

/**
 * Constructor
 * @param Parent object.
 */
BackendOsmRG::BackendOsmRG(QObject* const parent)
    : RGBackend(parent),
      d(new Private())
{
}

/**
 * Destructor
 */
BackendOsmRG::~BackendOsmRG()
{
    delete d;
}

/**
 * This slot calls Open Street Map's reverse geocoding service for each image.
 */
void BackendOsmRG::nextPhoto()
{
    if (d->jobs.isEmpty())
        return;

    QUrl netUrl(QLatin1String("http://nominatim.openstreetmap.org/reverse"));

    QUrlQuery q(netUrl);
    q.addQueryItem(QLatin1String("format"), QLatin1String("xml"));
    q.addQueryItem(QLatin1String("lat"), d->jobs.first().request.first().coordinates.latString());
    q.addQueryItem(QLatin1String("lon"), d->jobs.first().request.first().coordinates.lonString());
    q.addQueryItem(QLatin1String("zoom"), QLatin1String("18"));
    q.addQueryItem(QLatin1String("addressdetails"), QLatin1String("1"));
    q.addQueryItem(QLatin1String("accept-language"), d->jobs.first().language);
    netUrl.setQuery(q);

    QNetworkAccessManager* const mngr = new QNetworkAccessManager(this);

    connect(mngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    QNetworkRequest netRequest(netUrl);
    netRequest.setRawHeader("User-Agent", getUserAgentName().toLatin1());

    d->jobs.first().netReply = mngr->get(netRequest);
}

/**
 * Takes the coordinate of each image and then connects to Open Street Map's reverse geocoding service.
 * @param rgList A list containing information needed in reverse geocoding process. At this point, it contains only coordinates.
 * @param language The language in which the data will be returned.
 */
void BackendOsmRG::callRGBackend(const QList<RGInfo>& rgList, const QString& language)
{
    d->errorMessage.clear();

    for ( int i = 0; i < rgList.count(); ++i)
    {
        bool foundIt = false;

        for ( int j = 0; j < d->jobs.count(); ++j)
        {
            if (d->jobs[j].request.first().coordinates.sameLonLatAs(rgList[i].coordinates))
            {
                d->jobs[j].request << rgList[i];
                d->jobs[j].language = language;
                foundIt             = true;
                break;
             }
        }

        if (!foundIt)
        {
            OsmInternalJobs newJob;
            newJob.request << rgList.at(i);
            newJob.language = language;
            d->jobs << newJob;
        }
    }

    if (!d->jobs.empty())
        nextPhoto();
}

/**
 * The data is returned from Open Street Map in a XML. This function translates the XML into a QMap.
 * @param xmlData The returned XML.
 */
QMap<QString,QString> BackendOsmRG::makeQMapFromXML(const QString& xmlData)
{
    QString resultString;
    QMap<QString, QString> mappedData;
    QDomDocument doc;

    doc.setContent(xmlData);

    QDomElement docElem = doc.documentElement();
    QDomNode n          = docElem.lastChild().firstChild();

    while (!n.isNull())
    {
        QDomElement e = n.toElement();

        if (!e.isNull())
        {
            if ((e.tagName() == QLatin1String("country"))         ||
                (e.tagName() == QLatin1String("state"))           ||
                (e.tagName() == QLatin1String("state_district"))  ||
                (e.tagName() == QLatin1String("county"))          ||
                (e.tagName() == QLatin1String("city"))            ||
                (e.tagName() == QLatin1String("city_district"))   ||
                (e.tagName() == QLatin1String("suburb"))          ||
                (e.tagName() == QLatin1String("town"))            ||
                (e.tagName() == QLatin1String("village"))         ||
                (e.tagName() == QLatin1String("hamlet"))          ||
                (e.tagName() == QLatin1String("place"))           ||
                (e.tagName() == QLatin1String("road"))            ||
                (e.tagName() == QLatin1String("house_number")))
            {
                mappedData.insert(e.tagName(), e.text());
                resultString.append(e.tagName() + QLatin1Char(':') + e.text() + QLatin1Char('\n'));
            }
        }

        n = n.nextSibling();
    }

    return mappedData;
}

/**
 * @return Error message, if any.
 */
QString BackendOsmRG::getErrorMessage()
{
    return d->errorMessage;
}

/**
 * @return Backend name.
 */
QString BackendOsmRG::backendName()
{
    return QLatin1String("OSM");
}

void BackendOsmRG::slotFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        d->errorMessage = reply->errorString();
        emit(signalRGReady(d->jobs.first().request));
        d->jobs.clear();

        return;
    }

    for (int i = 0; i < d->jobs.count(); ++i)
    {
        if (d->jobs.at(i).netReply == reply)
        {
            d->jobs[i].data.append(reply->readAll());
            break;
        }
    }

    for (int i = 0; i < d->jobs.count(); ++i)
    {
        if (d->jobs.at(i).netReply == reply)
        {
            QString dataString;
            dataString = QString::fromUtf8(d->jobs[i].data.constData(),qstrlen(d->jobs[i].data.constData()));
            int pos    = dataString.indexOf(QLatin1String("<reversegeocode"));
            dataString.remove(0,pos);

            QMap<QString, QString> resultMap = makeQMapFromXML(dataString);

            for (int j = 0; j < d->jobs[i].request.count(); ++j)
            {
                d->jobs[i].request[j].rgData = resultMap;
            }

            emit(signalRGReady(d->jobs[i].request));

            d->jobs.removeAt(i);

            if (!d->jobs.empty())
            {
                QTimer::singleShot(500, this, SLOT(nextPhoto()));
            }

            break;
        }
    }
}

void BackendOsmRG::cancelRequests()
{
    d->jobs.clear();
    d->errorMessage.clear();
}

} // namespace Digikam
