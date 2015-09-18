/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-06-01
 * @brief  GPSSync common functions and structures
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
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

#ifndef GPS_COMMON_H
#define GPS_COMMON_H

// Qt includes

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QString>
#include <QUrl>

// Libkgeomap includes

#include <KGeoMap/GeoCoordinates>

namespace Digikam
{

enum MapLayout
{
    MapLayoutOne        = 0,
    MapLayoutHorizontal = 1,
    MapLayoutVertical   = 2
};

inline QString getUserAgentName()
{
    return QStringLiteral("KIPI-Plugins GPSSync - kde-imaging@kde.org");
}

inline void CoordinatesToClipboard(const KGeoMap::GeoCoordinates& coordinates, const QUrl& url, const QString& title)
{
    const QString lat       = coordinates.latString();
    const QString lon       = coordinates.lonString();
    const bool haveAltitude = coordinates.hasAltitude();
    const QString altitude  = coordinates.altString();
    const QString nameToUse = title.isEmpty() ? url.toLocalFile() : title;

    // importing this representation into Marble does not show anything,
    // but Merkaartor shows the point
    const QString kmlCoordinatesString = haveAltitude ? QString::fromLatin1("%1,%2,%3").arg(lon).arg(lat).arg(altitude)
                                                      : QString::fromLatin1("%1,%2").arg(lon).arg(lat);

    const QString kmlRepresentation = QString::fromLatin1(
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"
      "<Document>\n"
      " <Placemark>\n"
      "   <name>%1</name>\n"
      "   <Point>\n"
      "     <coordinates>%2</coordinates>\n"
      "   </Point>\n"
      " </Placemark>\n"
      "</Document>\n"
      "</kml>\n"
      ).arg(nameToUse).arg(kmlCoordinatesString);

    // importing this data into Marble and Merkaartor works
    const QString gpxElevationString = haveAltitude ? QString::fromLatin1("   <ele>%1</ele>\n").arg(altitude)
                                                    : QString();

    const QString gpxRepresentation  = QString::fromLatin1(
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
      "<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" creator=\"trippy\" version=\"0.1\"\n"
      " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
      " xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n"
      "  <wpt lat=\"%1\" lon=\"%2\">\n"
      "%3"
//      "   <time></time>\n"
      "   <name>%4</name>\n"
      "  </wpt>\n"
      "</gpx>\n"
      ).arg(lat).arg(lon).arg(gpxElevationString).arg(nameToUse);

    QMimeData* const myMimeData = new QMimeData();
    myMimeData->setText(coordinates.geoUrl());
    myMimeData->setData(QLatin1String("application/vnd.google-earth.kml+xml"), kmlRepresentation.toUtf8());
    myMimeData->setData(QLatin1String("application/gpx+xml"),                  gpxRepresentation.toUtf8());

    QClipboard* const clipboard = QApplication::clipboard();
    clipboard->setMimeData(myMimeData);
}

} /* Digikam */

Q_DECLARE_METATYPE(Digikam::MapLayout)

#endif /* GPS_COMMON_H */
