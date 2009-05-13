/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display GPS info on a world map
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "worldmapwidget.h"
#include "worldmapwidget.moc"

// Qt includes

#include <QVBoxLayout>
#include <QStyle>
#include <QDomDocument>
#include <QTextStream>
#include <QFile>
#include <QLabel>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <ktemporaryfile.h>

#include "config-digikam.h"
#ifdef HAVE_MARBLEWIDGET
#include <marble/MarbleWidget.h>
using namespace Marble;
#endif // HAVE_MARBLEWIDGET

namespace Digikam
{

class WorldMapWidgetPriv
{

public:

    WorldMapWidgetPriv(){};

    GPSInfoList   list;

#ifdef HAVE_MARBLEWIDGET
    MarbleWidget *marbleWidget;
#else
    QLabel       *marbleWidget;
#endif // HAVE_MARBLEWIDGET
};

WorldMapWidget::WorldMapWidget(int w, int h, QWidget *parent)
              : QFrame(parent), d(new WorldMapWidgetPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumWidth(w);
    setMinimumHeight(h);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

#ifdef HAVE_MARBLEWIDGET
    d->marbleWidget = new MarbleWidget(this);
#ifdef MARBLE_VERSION
    d->marbleWidget->setMapThemeId("earth/srtm/srtm.dgml");
#endif // MARBLE_VERSION
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
    return d->list.first().latitude;
}

double WorldMapWidget::getLongitude()
{
    return d->list.first().longitude;
}

void WorldMapWidget::setGPSPositions(const GPSInfoList& list)
{
    d->list = list;

    // To place mark over a map in marble canvas, we will use KML data

    QDomDocument       kmlDocument;
    QDomImplementation impl;
    QDomProcessingInstruction instr = kmlDocument.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    kmlDocument.appendChild(instr);
    QDomElement kmlRoot = kmlDocument.createElementNS( "http://earth.google.com/kml/2.1","kml");
    kmlDocument.appendChild(kmlRoot);

    QDomElement kmlAlbum     = addKmlElement(kmlDocument, kmlRoot, "Document");
    QDomElement kmlName      = addKmlTextElement(kmlDocument, kmlAlbum, "name", "Geolocation");

    double lng = 0;
    double lat = 0;

    if (!d->list.isEmpty())
    {
        for (GPSInfoList::const_iterator it = d->list.constBegin(); it != d->list.constEnd(); ++it)
        {
            QDomElement kmlPlacemark = addKmlElement(kmlDocument, kmlAlbum, "Placemark");
            addKmlTextElement(kmlDocument, kmlPlacemark, "name", (*it).url.fileName());

            QDomElement kmlGeometry  = addKmlElement(kmlDocument, kmlPlacemark, "Point");
            addKmlTextElement(kmlDocument, kmlGeometry, "coordinates", QString("%1,%2")
                            .arg((*it).longitude)
                            .arg((*it).latitude));
            addKmlTextElement(kmlDocument, kmlGeometry, "altitudeMode", "clampToGround");
            addKmlTextElement(kmlDocument, kmlGeometry, "extrude", "1");

            QDomElement kmlTimeStamp = addKmlElement(kmlDocument, kmlPlacemark, "TimeStamp");
            addKmlTextElement(kmlDocument, kmlTimeStamp, "when",
                              (*it).dateTime.toString("yyyy-MM-ddThh:mm:ssZ"));
        }

        lng = d->list.first().longitude;
        lat = d->list.first().latitude;
    }

#ifdef HAVE_MARBLEWIDGET

#ifdef MARBLE_VERSION

    // For Marble > 0.5.1
    if (!d->list.isEmpty())
    {
        d->marbleWidget->setHome(lng, lat);
        d->marbleWidget->centerOn(lng, lat);
#if MARBLE_VERSION >= 0x00800
        d->marbleWidget->addPlacemarkData(kmlDocument.toString());
#else
        d->marbleWidget->addPlaceMarkData(kmlDocument.toString());
#endif
    }
#else // MARBLE_VERSION

    // For Marble 0.5.1, there is no method to place a mark over the map using string.
    // The only way is to use a temp file with all KML information.
    KTemporaryFile KMLFile;
    KMLFile.setSuffix(".kml");
    KMLFile.setAutoRemove(true);
    KMLFile.open();
    QFile file(KMLFile.fileName());
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    stream << kmlDocument.toString();
    file.close();

    if (!d->list.isEmpty())
    {
        d->marbleWidget->setHome(lng, lat);
        d->marbleWidget->centerOn(lng, lat);
    }
    d->marbleWidget->addPlaceMarkFile(KMLFile.fileName());

#endif // MARBLE_VERSION

#endif // HAVE_MARBLEWIDGET
}

QDomElement WorldMapWidget::addKmlElement(QDomDocument& kmlDocument, QDomElement& target, const QString& tag)
{
    QDomElement kmlElement = kmlDocument.createElement(tag);
    target.appendChild(kmlElement);
    return kmlElement;
}

QDomElement WorldMapWidget::addKmlTextElement(QDomDocument& kmlDocument, QDomElement& target,
                                              const QString& tag, const QString& text)
{
    QDomElement kmlElement  = kmlDocument.createElement(tag);
    target.appendChild(kmlElement);
    QDomText kmlTextElement = kmlDocument.createTextNode(text);
    kmlElement.appendChild(kmlTextElement);
    return kmlElement;
}

}  // namespace Digikam
