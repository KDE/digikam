/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a tab widget to display GPS info
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

/*
Good explainations about GPS (in French) can be found at this url :
http://www.gpspassion.com/forumsen/topic.asp?TOPIC_ID=16593
*/

// Qt includes.

#include <QPushButton>
#include <QMap>
#include <QFile>
#include <QComboBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>

// KDE includes.

#include <khbox.h>
#include <kdialog.h>
#include <klocale.h>
#include <ktoolinvocation.h>
#include <ksqueezedtextlabel.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "worldmapwidget.h"
#include "gpstab.h"
#include "gpstab.moc"

namespace Digikam
{

class GPSTabPriv
{

public:

    GPSTabPriv()
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
    }

    QLabel             *altLabel;
    QLabel             *latLabel;
    QLabel             *lonLabel;
    QLabel             *dateLabel;

    QPushButton        *detailsButton;

    QComboBox          *detailsCombo;

    KSqueezedTextLabel *altitude;
    KSqueezedTextLabel *latitude;
    KSqueezedTextLabel *longitude;
    KSqueezedTextLabel *date;

    WorldMapWidget     *map;
};

GPSTab::GPSTab(QWidget* parent, bool navBar)
      : NavigateBarTab(parent)
{
    d = new GPSTabPriv;
    setupNavigateBar(navBar);

    // --------------------------------------------------------

    QWidget *gpsInfo    = new QWidget(this);
    m_navigateBarLayout->addWidget(gpsInfo);

    QGridLayout *layout = new QGridLayout(gpsInfo);
    d->map              = new WorldMapWidget(256, 256, gpsInfo);
    d->altLabel         = new QLabel(i18n("<b>Altitude</b>:"),  gpsInfo);
    d->latLabel         = new QLabel(i18n("<b>Latitude</b>:"),  gpsInfo);
    d->lonLabel         = new QLabel(i18n("<b>Longitude</b>:"), gpsInfo);
    d->dateLabel        = new QLabel(i18n("<b>Date</b>:"),      gpsInfo);
    d->altitude         = new KSqueezedTextLabel(0, gpsInfo);
    d->latitude         = new KSqueezedTextLabel(0, gpsInfo);
    d->longitude        = new KSqueezedTextLabel(0, gpsInfo);
    d->date             = new KSqueezedTextLabel(0, gpsInfo);
    d->altitude->setAlignment(Qt::AlignRight);
    d->latitude->setAlignment(Qt::AlignRight);
    d->longitude->setAlignment(Qt::AlignRight);
    d->date->setAlignment(Qt::AlignRight);

    // --------------------------------------------------------

    QWidget* box2           = new QWidget(gpsInfo);
    QHBoxLayout* box2Layout = new QHBoxLayout(box2);

    d->detailsCombo  = new QComboBox(box2);
    d->detailsButton = new QPushButton(i18n("More Info..."), box2);
    d->detailsButton->setMaximumHeight( fontMetrics().height()+4 );
    d->detailsCombo->setMaximumHeight( fontMetrics().height()+4 );
    d->detailsCombo->insertItem(MapQuest,   QString("MapQuest"));
    d->detailsCombo->insertItem(GoogleMaps, QString("Google Maps"));
    d->detailsCombo->insertItem(MsnMaps,    QString("MSN Maps"));
    d->detailsCombo->insertItem(MultiMap,   QString("MultiMap"));

    box2Layout->addWidget(d->detailsCombo);
    box2Layout->addStretch(1);
    box2Layout->addWidget(d->detailsButton);
    box2Layout->addStretch(100);
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

    // --------------------------------------------------------

    connect(d->detailsButton, SIGNAL(clicked()),
            this, SLOT(slotGPSDetails()));
}

GPSTab::~GPSTab()
{
    delete d;
}

int GPSTab::getWebGPSLocator()
{
    return ( d->detailsCombo->currentIndex() );
}

void GPSTab::setWebGPSLocator(int locator)
{
    d->detailsCombo->setCurrentIndex(locator);
}

void GPSTab::slotGPSDetails()
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
    }

    DDebug() << url << endl;
    KToolInvocation::self()->invokeBrowser(url);
}

void GPSTab::setCurrentURL(const KUrl& url)
{
    if (url.isEmpty())
    {
        setGPSInfo();
        return;
    }

    double alt, lat, lon;
    DMetadata meta(url.path());
    QDateTime dt = meta.getImageDateTime();
    if (meta.getGPSInfo(alt, lat, lon))
        setGPSInfo(lat, lon, alt, dt);
    else
        setGPSInfo();
}

void GPSTab::setGPSInfo()
{
    d->altitude->setText(QString());
    d->latitude->setText(QString());
    d->longitude->setText(QString());
    d->date->setText(QString());

    setNavigateBarFileName();
    setEnabled(false);
}

void GPSTab::setGPSInfo(double lat, double lon, long alt, const QDateTime dt)
{
    d->altitude->setText(QString("%1 m").arg(QString::number(alt)));
    d->latitude->setText(QString::number(lat));
    d->longitude->setText(QString::number(lon));
    d->date->setText(KGlobal::locale()->formatDateTime(dt, KLocale::ShortDate, true));

    setEnabled(true);
    d->map->setGPSPosition(lat, lon);
}

}  // namespace Digikam
