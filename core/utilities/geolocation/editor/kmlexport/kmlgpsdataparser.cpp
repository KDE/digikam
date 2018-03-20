/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-05-16
 * Description : a tool to export GPS data to KML file.
 *
 * Copyright (C) 2006-2007 by Stephane Pontier <shadow dot walker at free dot fr>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "kmlgpsdataparser.h"

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

KMLGeoDataParser::KMLGeoDataParser()
    : GeoDataParser()
{
    kmlDocument = 0;
}

KMLGeoDataParser::~KMLGeoDataParser()
{
}

QString KMLGeoDataParser::lineString()
{
    QString line = QLatin1String("");
    // cache the end to not recalculate it with large number of points
    GeoDataMap::ConstIterator end (m_GeoDataMap.constEnd());

    for (GeoDataMap::ConstIterator it = m_GeoDataMap.constBegin(); it != end; ++it )
    {
        line += QString::fromUtf8("%1,%2,%3 ").arg(it.value().longitude()).arg(it.value().latitude()).arg(it.value().altitude());
    }

    return line;
}

void KMLGeoDataParser::CreateTrackLine(QDomElement& parent, QDomDocument& root, int altitudeMode)
{
    kmlDocument = &root;

    // add the linetrack
    QDomElement kmlPlacemark = addKmlElement(parent, QLatin1String("Placemark"));
    addKmlTextElement(kmlPlacemark, QLatin1String("name"), i18n("Track"));
    QDomElement kmlLineString = addKmlElement(kmlPlacemark, QLatin1String("LineString"));
    addKmlTextElement(kmlLineString, QLatin1String("coordinates"), lineString());
    addKmlTextElement(kmlPlacemark, QLatin1String("styleUrl"), QLatin1String("#linetrack"));

    if (altitudeMode == 2 )
    {
        addKmlTextElement(kmlLineString, QLatin1String("altitudeMode"), QLatin1String("absolute"));
    }
    else if (altitudeMode == 1 )
    {
        addKmlTextElement(kmlLineString, QLatin1String("altitudeMode"), QLatin1String("relativeToGround"));
    }
    else
    {
        addKmlTextElement(kmlLineString, QLatin1String("altitudeMode"), QLatin1String("clampToGround"));
    }
}

void KMLGeoDataParser::CreateTrackPoints(QDomElement& parent, QDomDocument& root, int timeZone, int altitudeMode)
{
    kmlDocument = &root;

    // create the points
    QDomElement kmlPointsFolder = addKmlElement(parent, QLatin1String("Folder"));
    addKmlTextElement(kmlPointsFolder, QLatin1String("name"),       i18n("Points"));
    addKmlTextElement(kmlPointsFolder, QLatin1String("visibility"), QLatin1String("0"));
    addKmlTextElement(kmlPointsFolder, QLatin1String("open"),       QLatin1String("0"));
    int i = 0;

    // cache the end to not recalculate it with large number of points
    GeoDataMap::ConstIterator end (m_GeoDataMap.constEnd());

    for (GeoDataMap::ConstIterator it = m_GeoDataMap.constBegin(); it != end; ++it, ++i)
    {
        QDomElement kmlPointPlacemark = addKmlElement(kmlPointsFolder, QLatin1String("Placemark"));
        addKmlTextElement(kmlPointPlacemark, QLatin1String("name"), QString::fromUtf8("%1 %2 ").arg(i18n("Point")).arg(i));
        addKmlTextElement(kmlPointPlacemark, QLatin1String("styleUrl"), QLatin1String("#track"));
        QDomElement kmlTimeStamp      = addKmlElement(kmlPointPlacemark, QLatin1String("TimeStamp"));
        // GPS device are sync in time by satellite using GMT time.
        // If the camera time is different than GMT time, we want to
        // convert the GPS time to localtime of the picture to be display
        // in the same timeframe
        QDateTime GPSLocalizedTime = it.key().addSecs(timeZone*3600);

        addKmlTextElement(kmlTimeStamp, QLatin1String("when"), GPSLocalizedTime.toString(QLatin1String("yyyy-MM-ddThh:mm:ssZ")));
        QDomElement kmlGeometry    = addKmlElement(kmlPointPlacemark, QLatin1String("Point"));
        addKmlTextElement(kmlPointPlacemark, QLatin1String("visibility"), QLatin1String("0"));

        if (it.value().latitude())
        {
            addKmlTextElement(kmlGeometry, QLatin1String("coordinates"),
                              QString::fromUtf8("%1,%2,%3 ")
                              .arg(it.value().longitude()).arg(it.value().latitude()).arg(it.value().altitude()));
        }
        else
        {
            addKmlTextElement(kmlGeometry, QLatin1String("coordinates"), QString::fromUtf8("%1,%2 ").arg(it.value().longitude()).arg(it.value().latitude()));
        }
        if (altitudeMode == 2 )
        {
            addKmlTextElement(kmlGeometry, QLatin1String("altitudeMode"), QLatin1String("absolute"));
        }
        else if (altitudeMode == 1 )
        {
            addKmlTextElement(kmlGeometry, QLatin1String("altitudeMode"), QLatin1String("relativeToGround"));
        }
        else
        {
            addKmlTextElement(kmlGeometry, QLatin1String("altitudeMode"), QLatin1String("clampToGround"));
        }
    }
}

} // namespace Digikam
