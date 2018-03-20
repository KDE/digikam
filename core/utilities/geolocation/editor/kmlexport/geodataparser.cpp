/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : GPS data file parser.
 *               (GPX format http://www.topografix.com/gpx.asp).
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "geodataparser.h"
#include "geodataparser_time.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QDomDocument>
#include <QFile>
#include <QString>
#include <QIODevice>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

GeoDataParser::GeoDataParser()
{
    clear();
}

void GeoDataParser::clear()
{
    m_GeoDataMap.clear();
}

int GeoDataParser::numPoints() const
{
    return m_GeoDataMap.count();
}

bool GeoDataParser::matchDate(const QDateTime& photoDateTime, int maxGapTime, int secondsOffset,
                              bool offsetContainsTimeZone,
                              bool interpolate, int interpolationDstTime,
                              GeoDataContainer* const gpsData)
{
    // GPS device are sync in time by satelite using GMT time.
    QDateTime cameraGMTDateTime = photoDateTime.addSecs(secondsOffset*(-1));

    if (offsetContainsTimeZone)
    {
        cameraGMTDateTime.setTimeSpec(Qt::UTC);
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "    photoDateTime: " << photoDateTime << photoDateTime.timeSpec();
    qCDebug(DIGIKAM_GENERAL_LOG) << "cameraGMTDateTime: " << cameraGMTDateTime << cameraGMTDateTime.timeSpec();

    // We are trying to find the right date in the GPS points list.
    bool findItem = false;
    int nbSecItem = maxGapTime;
    int nbSecs;

    for (GeoDataMap::ConstIterator it = m_GeoDataMap.constBegin(); it != m_GeoDataMap.constEnd(); ++it )
    {
        // Here we check a possible accuracy in seconds between the
        // Camera GMT time and the GPS device GMT time.

        nbSecs = qAbs(cameraGMTDateTime.secsTo( it.key() ));
//         qCDebug(DIGIKAM_GENERAL_LOG) << it.key() << cameraGMTDateTime << nbSecs;
//         qCDebug(DIGIKAM_GENERAL_LOG) << it.key().timeSpec() << cameraGMTDateTime.timeSpec() << nbSecs;

        // We tring to find the minimal accuracy.
        if (nbSecs < maxGapTime && nbSecs < nbSecItem)
        {
            if (gpsData)
            {
                *gpsData = m_GeoDataMap[it.key()];
            }

            findItem  = true;
            nbSecItem = nbSecs;
        }
    }

    if (findItem) return true;

    // If we can't find it, we will trying to interpolate the GPS point.

    if (interpolate)
    {
        // The interpolate GPS point will be separate by at the maximum of 'interpolationDstTime'
        // seconds before and after the next and previous real GPS point found.

        QDateTime prevDateTime = findPrevDate(cameraGMTDateTime, interpolationDstTime);
        QDateTime nextDateTime = findNextDate(cameraGMTDateTime, interpolationDstTime);

        if (!nextDateTime.isNull() && !prevDateTime.isNull())
        {
            GeoDataContainer prevGPSPoint = m_GeoDataMap[prevDateTime];
            GeoDataContainer nextGPSPoint = m_GeoDataMap[nextDateTime];

            double alt1 = prevGPSPoint.altitude();
            double lon1 = prevGPSPoint.longitude();
            double lat1 = prevGPSPoint.latitude();
            uint   t1   = prevDateTime.toTime_t();
            double alt2 = nextGPSPoint.altitude();
            double lon2 = nextGPSPoint.longitude();
            double lat2 = nextGPSPoint.latitude();
            uint   t2   = nextDateTime.toTime_t();
            uint   tCor = cameraGMTDateTime.toTime_t();

            if (tCor-t1 != 0)
            {
                if (gpsData)
                {
                    gpsData->setAltitude(alt1  + (alt2-alt1) * (tCor-t1)/(t2-t1));
                    gpsData->setLatitude(lat1  + (lat2-lat1) * (tCor-t1)/(t2-t1));
                    gpsData->setLongitude(lon1 + (lon2-lon1) * (tCor-t1)/(t2-t1));
                    gpsData->setInterpolated(true);
                }
                return true;
            }
        }
    }

    return false;
}

QDateTime GeoDataParser::findNextDate(const QDateTime& dateTime, int secs)
{
    // We will find the item in GPS data list where the time is
    // at the maximum bigger than 'secs' mn of the value to match.
    QDateTime itemFound = dateTime.addSecs(secs);
    bool found          = false;

    for (GeoDataMap::ConstIterator it = m_GeoDataMap.constBegin(); it != m_GeoDataMap.constEnd(); ++it )
    {
        if (it.key() > dateTime)
        {
            if (it.key() < itemFound)
            {
                itemFound = it.key();
                found = true;
            }
        }
    }

    if (found)
        return itemFound;

    return QDateTime();
}

QDateTime GeoDataParser::findPrevDate(const QDateTime& dateTime, int secs)
{
    // We will find the item in GPS data list where the time is
    // at the maximum smaller than 'secs' mn of the value to match.
    QDateTime itemFound = dateTime.addSecs((-1)*secs);
    bool found          = false;

    for (GeoDataMap::ConstIterator it = m_GeoDataMap.constBegin(); it != m_GeoDataMap.constEnd(); ++it )
    {
        if (it.key() < dateTime)
        {
            if (it.key() > itemFound)
            {
                itemFound = it.key();
                found = true;
            }
        }
    }

    if (found)
        return itemFound;

    return QDateTime();
}

bool GeoDataParser::loadGPXFile(const QUrl& url)
{
    QFile gpxfile(url.toLocalFile());

    if (!gpxfile.open(QIODevice::ReadOnly))
        return false;

    QDomDocument gpxDoc(QLatin1String("gpx"));

    if (!gpxDoc.setContent(&gpxfile))
        return false;

    QDomElement gpxDocElem = gpxDoc.documentElement();

    if (gpxDocElem.tagName() != QLatin1String("gpx"))
        return false;

    for (QDomNode nTrk = gpxDocElem.firstChild(); !nTrk.isNull(); nTrk = nTrk.nextSibling())
    {
        QDomElement trkElem = nTrk.toElement();

        if (trkElem.isNull()) continue;

        if (trkElem.tagName() != QLatin1String("trk")) continue;

        for (QDomNode nTrkseg = trkElem.firstChild(); !nTrkseg.isNull(); nTrkseg = nTrkseg.nextSibling())
        {
            QDomElement trksegElem = nTrkseg.toElement();

            if (trksegElem.isNull()) continue;

            if (trksegElem.tagName() != QLatin1String("trkseg")) continue;

            for (QDomNode nTrkpt = trksegElem.firstChild(); !nTrkpt.isNull(); nTrkpt = nTrkpt.nextSibling())
            {
                QDomElement trkptElem = nTrkpt.toElement();

                if (trkptElem.isNull()) continue;

                if (trkptElem.tagName() != QLatin1String("trkpt")) continue;

                QDateTime ptDateTime;
                double    ptAltitude  = 0.0;
                double    ptLatitude  = 0.0;
                double    ptLongitude = 0.0;

                // Get GPS position. If not available continue to next point.
                QString lat = trkptElem.attribute(QLatin1String("lat"));
                QString lon = trkptElem.attribute(QLatin1String("lon"));

                if (lat.isEmpty() || lon.isEmpty()) continue;

                ptLatitude  = lat.toDouble();
                ptLongitude = lon.toDouble();

                // Get metadata of track point (altitude and time stamp)
                for (QDomNode nTrkptMeta = trkptElem.firstChild(); !nTrkptMeta.isNull(); nTrkptMeta = nTrkptMeta.nextSibling())
                {
                    QDomElement trkptMetaElem = nTrkptMeta.toElement();

                    if (trkptMetaElem.isNull()) continue;

                    if (trkptMetaElem.tagName() == QLatin1String("time"))
                    {
                        // Get GPS point time stamp. If not available continue to next point.
                        const QString time = trkptMetaElem.text();

                        if (time.isEmpty()) continue;

                        ptDateTime = GeoDataParserParseTime(time);
                    }

                    if (trkptMetaElem.tagName() == QLatin1String("ele"))
                    {
                        // Get GPS point altitude. If not available continue to next point.
                        QString ele = trkptMetaElem.text();

                        if (!ele.isEmpty())
                            ptAltitude  = ele.toDouble();
                    }
                }

                if (ptDateTime.isNull())
                    continue;

                GeoDataContainer gpsData(ptAltitude, ptLatitude, ptLongitude, false);
                m_GeoDataMap.insert( ptDateTime, gpsData );
            }
        }
    }

    //qCDebug(DIGIKAM_GENERAL_LOG) << "GPX File " << url.fileName()
    //                         << " parsed with " << numPoints()
    //                         << " points extracted" ;
    return true;
}

} // namespace Digikam
