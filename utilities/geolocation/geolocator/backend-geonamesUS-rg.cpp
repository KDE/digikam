/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-05-12
 * @brief  Backend for reverse geocoding using geonames.org (US-only)
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gabriel Voicu
 *         <a href="mailto:ping dot gabi at gmail dot com">ping dot gabi at gmail dot com</a>
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

#include "backend-geonamesUS-rg.h"

// Qt includes

#include <QDomDocument>
#include <QMap>
#include <QPointer>
#include <QTimer>
#include <QUrlQuery>
#include <QUrl>

// KDE includes

#include <kio/job.h>

// Local includes

#include "digikam_debug.h"
#include "gpssync_common.h"

namespace Digikam
{

/**
 * @class BackendGeonamesUSRG
 *
 * @brief This class calls Geonames' get address service available only for USA locations.
 */
class GeonamesUSInternalJobs
{

public:

    GeonamesUSInternalJobs()
      : language(),
        request(),
        data(),
        kioJob(0)
    {
    }

    ~GeonamesUSInternalJobs()
    {
        if (kioJob)
            kioJob->deleteLater();
    }

    QString            language;
    QList<RGInfo>      request;
    QByteArray         data;
    QPointer<KIO::Job> kioJob;
};

class BackendGeonamesUSRG::Private
{

public:

    Private()
      : itemCounter(0),
        itemCount(0),
        jobs(),
        errorMessage()
    {
    }

    int                           itemCounter;
    int                           itemCount;
    QList<GeonamesUSInternalJobs> jobs;
    QString                       errorMessage;
};

/**
 * Constructor
 * @param Parent object.
 */
BackendGeonamesUSRG::BackendGeonamesUSRG(QObject* const parent)
    : RGBackend(parent), d(new Private())
{
}

/**
 * Destructor
 */
BackendGeonamesUSRG::~BackendGeonamesUSRG()
{
    delete d;
}

/**
 * This slot calls Geonames's get address service for each image.
 */
void BackendGeonamesUSRG::nextPhoto()
{
    if (d->jobs.isEmpty())
        return;

    QUrl jobUrl(QStringLiteral("http://ws.geonames.org/findNearestAddress"));
    
    QUrlQuery q(jobUrl);
    q.addQueryItem(QStringLiteral("lat"), d->jobs.first().request.first().coordinates.latString());
    q.addQueryItem(QStringLiteral("lng"), d->jobs.first().request.first().coordinates.lonString());
//  q.addQueryItem(QStringLiteral("lang"), d->jobs.first().language);
    jobUrl.setQuery(q);

    d->jobs.first().kioJob = KIO::get(jobUrl, KIO::NoReload, KIO::HideProgressInfo);
    d->jobs.first().kioJob->addMetaData(QStringLiteral("User-Agent"), getKipiUserAgentName());

    connect(d->jobs.first().kioJob, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(dataIsHere(KIO::Job*,QByteArray)));

    connect(d->jobs.first().kioJob, SIGNAL(result(KJob*)),
            this, SLOT(slotResult(KJob*)));
}

/**
 * Takes the coordinate of each image and then connects to Open Street Map's reverse geocoding service.
 * @param rgList A list containing information needed in reverse geocoding process. At this point, it contains only coordinates.
 * @param language The language in which the data will be returned.
 */
void BackendGeonamesUSRG::callRGBackend(const QList<RGInfo>& rgList, const QString& language)
{
    d->errorMessage.clear();

    for (int i = 0; i < rgList.count(); ++i)
    {
        bool foundIt = false;

        for (int j=0; j < d->jobs.count(); ++j)
        {
            if (d->jobs[j].request.first().coordinates.sameLonLatAs(rgList[i].coordinates))
            {

                d->jobs[j].request << rgList[i];
                d->jobs[j].language = language;
                foundIt = true;
                break;

            }
        }

        if (!foundIt)
        {
            GeonamesUSInternalJobs newJob;
            newJob.request << rgList.at(i);
            newJob.language = language;
            d->jobs << newJob;
        }
    }

    nextPhoto();
}

void BackendGeonamesUSRG::dataIsHere(KIO::Job* job, const QByteArray & data)
{
    for (int i = 0; i < d->jobs.count(); ++i)
    {
        if (d->jobs.at(i).kioJob == job)
        {
            d->jobs[i].data.append(data);
            break;
        }
    }
}

/**
 * The data is returned from Open Street Map in a XML. This function translates the XML into a QMap.
 * @param xmlData The returned XML.
 */
QMap<QString,QString> BackendGeonamesUSRG::makeQMapFromXML(const QString& xmlData)
{
    QMap<QString, QString> mappedData;
    QString resultString;
    QDomDocument doc;
    doc.setContent(xmlData);

    QDomElement docElem =  doc.documentElement();
    QDomNode n          = docElem.firstChild().firstChild();

    while (!n.isNull())
    {
        const QDomElement e = n.toElement();

        if (!e.isNull())
        {

            if ((e.tagName().compare(QStringLiteral("adminName2")) == 0) ||
               (e.tagName().compare(QStringLiteral("adminName1"))  == 0) ||
               (e.tagName().compare(QStringLiteral("placeName"))   == 0))
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
QString BackendGeonamesUSRG::getErrorMessage()
{
    return d->errorMessage;
}

/**
 * @return Backend name.
 */
QString BackendGeonamesUSRG::backendName()
{
    return QStringLiteral("GeonamesUS");
}

void BackendGeonamesUSRG::slotResult(KJob* kJob)
{
    KIO::Job* const kioJob = qobject_cast<KIO::Job*>(kJob);

    if (kioJob->error())
    {
        d->errorMessage = kioJob->errorString();
        emit(signalRGReady(d->jobs.first().request));
        d->jobs.clear();
        return;
    }

    for (int i = 0;i < d->jobs.count(); ++i)
    {
        if (d->jobs.at(i).kioJob == kioJob)
        {
            QString dataString;
            dataString = QString::fromUtf8(d->jobs[i].data.constData(),qstrlen(d->jobs[i].data.constData()));
            int pos    = dataString.indexOf(QStringLiteral("<geonames"));
            dataString.remove(0,pos);
            dataString.chop(1);
            QMap<QString,QString> resultMap = makeQMapFromXML(dataString);

            for (int j = 0; j < d->jobs[i].request.count(); ++j)
            {

                d->jobs[i].request[j].rgData =  resultMap;

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

void BackendGeonamesUSRG::cancelRequests()
{
    d->jobs.clear();
    d->errorMessage.clear();
}

} // Digikam
