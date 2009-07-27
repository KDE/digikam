/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a tab widget to display GPS info
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

/*
Good explanations about GPS (in French) can be found at this url :
http://www.gpspassion.com/forumsen/topic.asp?TOPIC_ID=16593
*/

#include "imagepropertiesgpstab.h"
#include "imagepropertiesgpstab.moc"

// Qt includes

#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QToolButton>

// KDE includes

#include <kdebug.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <khbox.h>
#include <klocale.h>
#include <ksqueezedtextlabel.h>
#include <ktoolinvocation.h>

namespace Digikam
{

class ImagePropertiesGPSTabPriv
{

public:

    ImagePropertiesGPSTabPriv()
    {
        detailsButton = 0;
        detailsCombo  = 0;
        map           = 0;
        altLabel      = 0;
        latLabel      = 0;
        lonLabel      = 0;
        dateLabel     = 0;
        altitude      = 0;
        latitude      = 0;
        longitude     = 0;
        date          = 0;
        zoomInBtn     = 0;
        zoomOutBtn    = 0;
    }

    QLabel             *altLabel;
    QLabel             *latLabel;
    QLabel             *lonLabel;
    QLabel             *dateLabel;

    QPushButton        *detailsButton;

    QToolButton        *zoomInBtn;
    QToolButton        *zoomOutBtn;

    QComboBox          *detailsCombo;

    KSqueezedTextLabel *altitude;
    KSqueezedTextLabel *latitude;
    KSqueezedTextLabel *longitude;
    KSqueezedTextLabel *date;

    WorldMapWidget     *map;
};

ImagePropertiesGPSTab::ImagePropertiesGPSTab(QWidget* parent)
                     : QWidget(parent), d(new ImagePropertiesGPSTabPriv)
{
    QGridLayout *layout = new QGridLayout(this);
    d->map              = new WorldMapWidget(256, 256, this);
    d->altLabel         = new QLabel(i18n("<b>Altitude</b>:"),  this);
    d->latLabel         = new QLabel(i18n("<b>Latitude</b>:"),  this);
    d->lonLabel         = new QLabel(i18n("<b>Longitude</b>:"), this);
    d->dateLabel        = new QLabel(i18n("<b>Date</b>:"),      this);
    d->altitude         = new KSqueezedTextLabel(0, this);
    d->latitude         = new KSqueezedTextLabel(0, this);
    d->longitude        = new KSqueezedTextLabel(0, this);
    d->date             = new KSqueezedTextLabel(0, this);
    d->altitude->setAlignment(Qt::AlignRight);
    d->latitude->setAlignment(Qt::AlignRight);
    d->longitude->setAlignment(Qt::AlignRight);
    d->date->setAlignment(Qt::AlignRight);

    // --------------------------------------------------------

    QWidget* box2           = new QWidget(this);
    QHBoxLayout* box2Layout = new QHBoxLayout(box2);

    d->zoomOutBtn = new QToolButton(box2);
    d->zoomInBtn  = new QToolButton(box2);
    d->zoomOutBtn->setIcon(SmallIcon("zoom-out"));
    d->zoomInBtn->setIcon(SmallIcon("zoom-in"));

    d->detailsCombo  = new KComboBox(box2);
    d->detailsButton = new QPushButton(i18n("More Info..."), box2);
    d->detailsButton->setMaximumHeight( fontMetrics().height()+4 );
    d->detailsCombo->setMaximumHeight( fontMetrics().height()+4 );
    d->detailsCombo->insertItem(MapQuest,      QString("MapQuest"));
    d->detailsCombo->insertItem(GoogleMaps,    QString("Google Maps"));
    d->detailsCombo->insertItem(MsnMaps,       QString("MSN Maps"));
    d->detailsCombo->insertItem(MultiMap,      QString("MultiMap"));
    d->detailsCombo->insertItem(OpenStreetMap, QString("OpenStreetMap"));

    box2Layout->addWidget(d->zoomOutBtn);
    box2Layout->addWidget(d->zoomInBtn);
    box2Layout->addStretch(10);
    box2Layout->addWidget(d->detailsCombo);
    box2Layout->addWidget(d->detailsButton);
    box2Layout->setSpacing(0);
    box2Layout->setMargin(0);

    // --------------------------------------------------------

    layout->addWidget(d->map,       0, 0, 1, 2);
    layout->addWidget(d->altLabel,  1, 0, 1, 1);
    layout->addWidget(d->altitude,  1, 1, 1, 1);
    layout->addWidget(d->latLabel,  2, 0, 1, 1);
    layout->addWidget(d->latitude,  2, 1, 1, 1);
    layout->addWidget(d->lonLabel,  3, 0, 1, 1);
    layout->addWidget(d->longitude, 3, 1, 1, 1);
    layout->addWidget(d->dateLabel, 4, 0, 1, 1);
    layout->addWidget(d->date,      4, 1, 1, 1);
    layout->addWidget(box2,         5, 0, 1, 2);
    layout->setRowStretch(0, 10);
    layout->setColumnStretch(1, 10);
    layout->setSpacing(0);
    layout->setMargin(0);

    readConfig();

    // --------------------------------------------------------

    connect(d->detailsButton, SIGNAL(clicked()),
            this, SLOT(slotGPSDetails()));

    connect(d->zoomInBtn, SIGNAL(released()),
            d->map, SLOT(slotZoomIn()));

    connect(d->zoomOutBtn, SIGNAL(released()),
            d->map, SLOT(slotZoomOut()));
}

ImagePropertiesGPSTab::~ImagePropertiesGPSTab()
{
    writeConfig();
    delete d;
}

int ImagePropertiesGPSTab::getWebGPSLocator()
{
    return ( d->detailsCombo->currentIndex() );
}

void ImagePropertiesGPSTab::setWebGPSLocator(int locator)
{
    d->detailsCombo->setCurrentIndex(locator);
}

void ImagePropertiesGPSTab::slotGPSDetails()
{
    QString val, url;

    switch( getWebGPSLocator() )
    {
        case MapQuest:
        {
            url.append("http://www.mapquest.com/maps/map.adp?searchtype=address"
                        "&formtype=address&latlongtype=decimal");
            url.append("&latitude=");
            url.append(val.setNum(d->map->getLatitude(), 'g', 12));
            url.append("&longitude=");
            url.append(val.setNum(d->map->getLongitude(), 'g', 12));
            break;
        }

        case GoogleMaps:
        {
            url.append("http://maps.google.com/?q=");
            url.append(val.setNum(d->map->getLatitude(), 'g', 12));
            url.append(",");
            url.append(val.setNum(d->map->getLongitude(), 'g', 12));
            url.append("&spn=0.05,0.05&t=h&om=1&hl=en");
            break;
        }

        case MsnMaps:
        {
            url.append("http://maps.msn.com/map.aspx?");
            url.append("&lats1=");
            url.append(val.setNum(d->map->getLatitude(), 'g', 12));
            url.append("&lons1=");
            url.append(val.setNum(d->map->getLongitude(), 'g', 12));
            url.append("&name=HERE");
            url.append("&alts1=7");
            break;
        }

        case MultiMap:
        {
            url.append("http://www.multimap.com/map/browse.cgi?");
            url.append("lat=");
            url.append(val.setNum(d->map->getLatitude(), 'g', 12));
            url.append("&lon=");
            url.append(val.setNum(d->map->getLongitude(), 'g', 12));
            url.append("&scale=10000");
            url.append("&icon=x");
            break;
        }

        case OpenStreetMap:
        {
            // lat and lon would also work, but wouldn't show a marker
            url.append("http://www.openstreetmap.org/?");
            url.append("mlat=");
            url.append(val.setNum(d->map->getLatitude(), 'g', 12));
            url.append("&mlon=");
            url.append(val.setNum(d->map->getLongitude(), 'g', 12));
            url.append("&zoom=15");
            break;
        }

    }

    kDebug(50003) << url;
    KToolInvocation::self()->invokeBrowser(url);
}

void ImagePropertiesGPSTab::setCurrentURL(const KUrl& url)
{
    if (url.isEmpty())
    {
        setGPSInfo();
        return;
    }

    DMetadata meta(url.path());
    setMetadata(meta, url);
}

void ImagePropertiesGPSTab::setMetadata(const DMetadata& meta, const KUrl& url)
{
    double alt, lat, lon;
    QDateTime dt = meta.getImageDateTime();
    if (meta.getGPSInfo(alt, lat, lon))
    {
        GPSInfo gpsInfo;
        gpsInfo.longitude = lon;
        gpsInfo.latitude  = lat;
        gpsInfo.altitude  = alt;
        gpsInfo.dateTime  = dt;
        gpsInfo.url       = url;
        setGPSInfoList(GPSInfoList() << gpsInfo);
    }
    else
        setGPSInfo();
}

void ImagePropertiesGPSTab::setGPSInfo()
{
    d->altitude->setText(QString());
    d->latitude->setText(QString());
    d->longitude->setText(QString());
    d->date->setText(QString());
    d->map->setGPSPositions(GPSInfoList());
    setEnabled(false);
}

void ImagePropertiesGPSTab::setGPSInfoList(const GPSInfoList& list)
{
    // Clear info label
    d->altitude->setText(QString());
    d->latitude->setText(QString());
    d->longitude->setText(QString());
    d->date->setText(QString());

    if (list.count() == 0)
    {
        setEnabled(false);
    }
    else if (list.count() == 1)
    {
        d->altitude->setText(QString("%1 m").arg(QString::number(list.first().altitude)));
        d->latitude->setText(QString::number(list.first().latitude));
        d->longitude->setText(QString::number(list.first().longitude));
        d->date->setText(KGlobal::locale()->formatDateTime(list.first().dateTime,
                                                           KLocale::ShortDate, true));
        setEnabled(true);
    }
    else if (list.count() > 1)
    {
        setEnabled(true);
    }

    d->map->setGPSPositions(list);
}

void ImagePropertiesGPSTab::readConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Image Properties SideBar"));
    d->map->readConfig(group);
}

void ImagePropertiesGPSTab::writeConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Image Properties SideBar"));
    d->map->writeConfig(group);
    group.sync();
}

}  // namespace Digikam
