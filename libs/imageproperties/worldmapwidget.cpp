/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display GPS info on a world map
 * 
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QVBoxLayout>
#include <QStyle>
#include <QDomDocument>
#include <QTextStream>
#include <QFile>
#include <QLabel>

// KDE includes.

#include <klocale.h>
#include <ktemporaryfile.h>

#include "config-digikam.h"
#ifdef HAVE_MARBLEWIDGET
#include <marble/MarbleWidget.h>
#endif // HAVE_MARBLEWIDGET

// Local includes.

#include "ddebug.h"
#include "worldmapwidget.h"
#include "worldmapwidget.moc"

namespace Digikam
{

class WorldMapWidgetPriv
{

public:

    WorldMapWidgetPriv()
    {
        latitude     = 0;
        longitude    = 0;
        altitude     = 0;
        marbleWidget = 0;
    }

    double        latitude;
    double        longitude;
    double        altitude;

    QDateTime     dt;

    KUrl          url;

#ifdef HAVE_MARBLEWIDGET
    MarbleWidget *marbleWidget;
#else
    QLabel       *marbleWidget;
#endif // HAVE_MARBLEWIDGET
};

WorldMapWidget::WorldMapWidget(int w, int h, QWidget *parent)
              : QFrame(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumWidth(w);
    setMinimumHeight(h);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

    d = new WorldMapWidgetPriv;
#ifdef HAVE_MARBLEWIDGET
    d->marbleWidget = new MarbleWidget(this);
    d->marbleWidget->setMapThemeId("earth/srtm/srtm.dgml");
#else
    d->marbleWidget = new QLabel(this);
    d->marbleWidget->setText(i18n("Geolocation using Marble not available"));
    d->marbleWidget->setWordWrap(true);
#endif // HAVE_MARBLEWIDGET

    QVBoxLayout *vlay = new QVBoxLayout(this);    
    vlay->addWidget(d->marbleWidget);
    vlay->setMargin(0);
    vlay->setSpacing(0);
}

WorldMapWidget::~WorldMapWidget()
{
    delete d;
}

double WorldMapWidget::getLatitude()
{
    return d->latitude;
}

double WorldMapWidget::getLongitude()
{
    return d->longitude;
}

void WorldMapWidget::setGPSPosition(double lat, double lng, double alt, const QDateTime& dt, const KUrl& url)
{
    d->latitude  = lat;
    d->longitude = lng;
    d->altitude  = alt;
    d->dt        = dt;
    d->url       = url;

    // NOTE: There is no method currently to place a mark over the map in Marble 0.5.1.
    // The only way is to use a temporary KML file with all informations that 
    // we need.

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

    KTemporaryFile KMLFile;
    KMLFile.setSuffix(".kml");
    KMLFile.setAutoRemove(true);
    KMLFile.open();
    QFile file(KMLFile.fileName());
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file); 
    stream << kmlDocument.toString();
    file.close();

#ifdef HAVE_MARBLEWIDGET
    d->marbleWidget->setHome(lng, lat);
    d->marbleWidget->centerOn(lng, lat);
    d->marbleWidget->addPlaceMarkFile(KMLFile.fileName());
#endif // HAVE_MARBLEWIDGET
}

QDomElement WorldMapWidget::addKmlElement(QDomDocument &kmlDocument, QDomElement &target, const QString& tag)
{
    QDomElement kmlElement = kmlDocument.createElement(tag);
    target.appendChild(kmlElement);
    return kmlElement;
}

QDomElement WorldMapWidget::addKmlTextElement(QDomDocument &kmlDocument, QDomElement &target, const QString& tag, const QString& text)
{
    QDomElement kmlElement  = kmlDocument.createElement(tag);
    target.appendChild(kmlElement);
    QDomText kmlTextElement = kmlDocument.createTextNode(text);
    kmlElement.appendChild(kmlTextElement);
    return kmlElement;
}

}  // namespace Digikam
