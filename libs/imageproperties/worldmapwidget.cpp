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
#include <kmenu.h>
#include <kiconloader.h>

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

    WorldMapWidgetPriv()
    {
        mapTheme     = WorldMapWidget::DefaultMap;
        marbleWidget = 0;
    };

    GPSInfoList              list;

    WorldMapWidget::MapTheme mapTheme;

#ifdef HAVE_MARBLEWIDGET
    MarbleWidget*            marbleWidget;
#else
    QLabel*                  marbleWidget;
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
#if MARBLE_VERSION < 0x000800
    d->marbleWidget->setDownloadUrl("http://download.kde.org/apps/marble/");
#endif
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

QWidget* WorldMapWidget::marbleWidget() const
{
    return d->marbleWidget;
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

    QDomElement kmlAlbum = addKmlElement(kmlDocument, kmlRoot, "Document");
    QDomElement kmlName  = addKmlTextElement(kmlDocument, kmlAlbum, "name", "Geolocation");

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

void WorldMapWidget::slotZoomIn()
{
#ifdef HAVE_MARBLEWIDGET
    d->marbleWidget->zoomIn();
    d->marbleWidget->repaint();
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::slotZoomOut()
{
#ifdef HAVE_MARBLEWIDGET
    d->marbleWidget->zoomOut();
    d->marbleWidget->repaint();
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::getCenterPosition(double& lat, double& lng)
{
#ifdef HAVE_MARBLEWIDGET
    lat = d->marbleWidget->centerLatitude();
    lng = d->marbleWidget->centerLongitude();
#else // HAVE_MARBLEWIDGET
    // GPS location : Paris
    lat = 48.850258199721495;
    lng = 2.3455810546875;
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::setCenterPosition(double lat, double lng)
{
#ifdef HAVE_MARBLEWIDGET
    d->marbleWidget->setCenterLatitude(lat);
    d->marbleWidget->setCenterLongitude(lng);
#endif // HAVE_MARBLEWIDGET
}

int WorldMapWidget::getZoomLevel()
{
#ifdef HAVE_MARBLEWIDGET
    return d->marbleWidget->zoom();
#else // HAVE_MARBLEWIDGET
    return 5;
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::setZoomLevel(int l)
{
#ifdef HAVE_MARBLEWIDGET
    d->marbleWidget->zoomView(l);
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::readConfig(KConfigGroup& group)
{
    setMapTheme((MapTheme)group.readEntry("Map Theme", (int)DefaultMap));
    setZoomLevel(group.readEntry("Zoom Level", 5));
    // Default GPS location : Paris
    setCenterPosition(group.readEntry("Latitude",  48.850258199721495),
                      group.readEntry("Longitude", 2.3455810546875));

    emit signalSettingsChanged();
}

void WorldMapWidget::writeConfig(KConfigGroup& group)
{
    group.writeEntry("Map Theme",  (int)getMapTheme());
    group.writeEntry("Zoom Level", getZoomLevel());
    double lat, lng;
    getCenterPosition(lat, lng);
    group.writeEntry("Latitude",  lat);
    group.writeEntry("Longitude", lng);
}

void WorldMapWidget::setMapTheme(MapTheme theme)
{
    d->mapTheme = theme;

#ifdef HAVE_MARBLEWIDGET

#ifdef MARBLE_VERSION
    switch(theme)
    {
        case OpenStreetMap:
            d->marbleWidget->setMapThemeId("earth/openstreetmap/openstreetmap.dgml");
            break;
        default: // DefaultMap
            d->marbleWidget->setMapThemeId("earth/srtm/srtm.dgml");
            break;
    }
#endif // MARBLE_VERSION

#endif // HAVE_MARBLEWIDGET
}

// ------------------------------------------------------------------------

WorldMapWidget::MapTheme WorldMapWidget::getMapTheme()
{
    return d->mapTheme;
}

class WorldMapThemeBtnPriv
{

public:

    WorldMapThemeBtnPriv()
    {
        defaultMapAction    = 0;
        openStreetMapAction = 0;
        mapThemeMenu        = 0;
        map                 = 0;
    };

    QAction*        defaultMapAction;
    QAction*        openStreetMapAction;

    KMenu*          mapThemeMenu;

    WorldMapWidget* map;
};

WorldMapThemeBtn::WorldMapThemeBtn(WorldMapWidget *map, QWidget *parent)
                : QToolButton(parent), d(new WorldMapThemeBtnPriv)
{
    d->map          = map;
    d->mapThemeMenu = new KMenu(this);
    setToolTip(i18n("Map Theme"));
    setIcon(SmallIcon("applications-internet"));
    setMenu(d->mapThemeMenu);
    setPopupMode(QToolButton::DelayedPopup);
    d->defaultMapAction    = d->mapThemeMenu->addAction(i18n("Default"));
    d->defaultMapAction->setCheckable(true);
    d->openStreetMapAction = d->mapThemeMenu->addAction(i18n("OpenStreetMap"));
    d->openStreetMapAction->setCheckable(true);

    connect(d->mapThemeMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotMapThemeChanged(QAction*)));

    connect(d->map, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotUpdateMenu()));
}

WorldMapThemeBtn::~WorldMapThemeBtn()
{
    delete d;
}

void WorldMapThemeBtn::slotMapThemeChanged(QAction *action)
{
    if (action == d->defaultMapAction)
    {
        d->map->setMapTheme(WorldMapWidget::DefaultMap);
    }
    else if (action == d->openStreetMapAction)
    {
        d->map->setMapTheme(WorldMapWidget::OpenStreetMap);
    }
    slotUpdateMenu();
}

void WorldMapThemeBtn::slotUpdateMenu()
{
    switch(d->map->getMapTheme())
    {
        case WorldMapWidget::OpenStreetMap:
        {
            d->defaultMapAction->setChecked(false);
            d->openStreetMapAction->setChecked(true);
            break;
        }
        default:
        {
            d->defaultMapAction->setChecked(true);
            d->openStreetMapAction->setChecked(false);
            break;
        }
    }
}

}  // namespace Digikam
