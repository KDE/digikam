/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-01
 * Description : GPSSync common functions and structures
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Michael G. Hansen <mike at mghansen dot de>
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

#include <gpscommon.h>

// Qt includes

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

// Local includes

#include "digikam_debug.h"
#include "dmetadata.h"
#include "metadatasettings.h"
#include "metadatasettingscontainer.h"
#include "dmessagebox.h"

namespace Digikam
{

QString getUserAgentName()
{
    return QString::fromLatin1("KIPI-Plugins GPSSync - kde-imaging@kde.org");
}

void coordinatesToClipboard(const GeoCoordinates& coordinates,
                            const QUrl& url,
                            const QString& title)
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

bool checkSidecarSettings()
{
    if ( (MetadataSettings::instance()->settings().metadataWritingMode != DMetadata::WRITETOIMAGEONLY) &&
         (!MetadataSettings::instance()->settings().useXMPSidecar4Reading) )
    {
        const int result = DMessageBox::showContinueCancel(QMessageBox::Warning,
                                                           QApplication::activeWindow(),
                                                           i18n("Warning: Sidecar settings"),
                                                           i18n("You have enabled writing to sidecar files for metadata storage in the host application,"
                                                                " but not for reading."
                                                                " This means that any metadata stored in the sidecar files will be overwritten here.\n"
                                                                "Please enable reading of sidecar files in the host application or continue at your own risk.")
                                                          );

        if (result != QMessageBox::Yes)
        {
            return false;
        }
    }

    return true;
}

} // namespace Digikam
