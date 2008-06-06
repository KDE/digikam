/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-06-06
 * Description : a widget to display GPS info on a world map
 * 
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes.

#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "worldmapwidgetdb.h"
#include "worldmapwidgetdb.moc"

namespace Digikam
{

class WorldMapWidgetDBPriv
{

public:

    WorldMapWidgetDBPriv()
    {
    }

    ImageInfoList imagesList;
};

WorldMapWidgetDB::WorldMapWidgetDB(int w, int h, QWidget *parent)
                : WorldMapWidget(w, h, parent)
{
    d = new WorldMapWidgetDBPriv;
}

WorldMapWidgetDB::~WorldMapWidgetDB()
{
    delete d;
}

void WorldMapWidgetDB::setGPSPositions(const ImageInfoList& /*list*/)
{
/*  TODO

    d->latitude  = lat;
    d->longitude = lng;
    d->altitude  = alt;
    d->dt        = dt;
    d->url       = url;

    // To place mark over a map in marble canvas, we will use KML data

    QDomDocument       kmlDocument;
    QDomImplementation impl;
    QDomProcessingInstruction instr = kmlDocument.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    kmlDocument.appendChild(instr);
    QDomElement kmlRoot = kmlDocument.createElementNS( "http://earth.google.com/kml/2.1","kml");
    kmlDocument.appendChild(kmlRoot);

    QDomElement kmlAlbum     = addKmlElement(kmlDocument, kmlRoot, "Document");
    QDomElement kmlName      = addKmlTextElement(kmlDocument, kmlAlbum, "name", "Geolocation");
    QDomElement kmlPlacemark = addKmlElement(kmlDocument, kmlAlbum, "Placemark");
    addKmlTextElement(kmlDocument, kmlPlacemark, "name", d->url.fileName());

    QDomElement kmlGeometry  = addKmlElement(kmlDocument, kmlPlacemark, "Point");
    addKmlTextElement(kmlDocument, kmlGeometry, "coordinates", QString("%1,%2").arg(lng).arg(lat));
    addKmlTextElement(kmlDocument, kmlGeometry, "altitudeMode", "clampToGround");
    addKmlTextElement(kmlDocument, kmlGeometry, "extrude", "1");

    QDomElement kmlTimeStamp = addKmlElement(kmlDocument, kmlPlacemark, "TimeStamp");
    addKmlTextElement(kmlDocument, kmlTimeStamp, "when", d->dt.toString("yyyy-MM-ddThh:mm:ssZ"));

#ifdef HAVE_MARBLEWIDGET

#ifdef MARBLE_VERSION

    // For Marble > 0.5.1
    d->marbleWidget->setHome(lng, lat);
    d->marbleWidget->centerOn(lng, lat);
    d->marbleWidget->addPlaceMarkData(kmlDocument.toString());

#else // MARBLE_VERSION

    // For Marble 0.5.1, there is no method to place a mark over the map using string.
    // The only way is to use a temp file with all KML informations.
    KTemporaryFile KMLFile;
    KMLFile.setSuffix(".kml");
    KMLFile.setAutoRemove(true);
    KMLFile.open();
    QFile file(KMLFile.fileName());
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file); 
    stream << kmlDocument.toString();
    file.close();

    d->marbleWidget->setHome(lng, lat);
    d->marbleWidget->centerOn(lng, lat);
    d->marbleWidget->addPlaceMarkFile(KMLFile.fileName());

#endif // MARBLE_VERSION

#endif // HAVE_MARBLEWIDGET
*/
}

}  // namespace Digikam
