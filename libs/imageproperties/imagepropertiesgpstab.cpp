/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a tab widget to display GPS info
 *
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version
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
#include <QStandardItemModel>
#include <QStandardItem>

// KDE includes

#include <kcombobox.h>
#include <kdialog.h>
#include <khbox.h>
#include <klocale.h>
#include <ksqueezedtextlabel.h>
#include <ktoolinvocation.h>
#include <kdebug.h>

// libkgeomap includes

#include <libkgeomap/kgeomap_widget.h>
#include <libkgeomap/itemmarkertiler.h>

// local includes

#include "imagegpsmodelhelper.h"

namespace Digikam
{

class ImagePropertiesGPSTab::Private
{

public:

    Private() :
        altLabel(0),
        latLabel(0),
        lonLabel(0),
        dateLabel(0),
        detailsBtn(0),
        detailsCombo(0),
        altitude(0),
        latitude(0),
        longitude(0),
        date(0),
        map(0),
        itemMarkerTiler(0),
        itemModel(0),
        gpsModelHelper(0),
        gpsImageInfoSorter(0),
        boundariesShouldBeAdjusted(false)
    {
    }

    QLabel*                   altLabel;
    QLabel*                   latLabel;
    QLabel*                   lonLabel;
    QLabel*                   dateLabel;

    QToolButton*              detailsBtn;
    KComboBox*                detailsCombo;

    KSqueezedTextLabel*       altitude;
    KSqueezedTextLabel*       latitude;
    KSqueezedTextLabel*       longitude;
    KSqueezedTextLabel*       date;

    KGeoMap::KGeoMapWidget*   map;
    KGeoMap::ItemMarkerTiler* itemMarkerTiler;
    GPSImageInfo::List        gpsInfoList;

    QStandardItemModel*       itemModel;
    ImageGPSModelHelper*      gpsModelHelper;
    GPSImageInfoSorter*       gpsImageInfoSorter;
    bool                      boundariesShouldBeAdjusted;
};

ImagePropertiesGPSTab::ImagePropertiesGPSTab(QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    QGridLayout* const layout = new QGridLayout(this);

    // --------------------------------------------------------

    QFrame* const mapPanel    = new QFrame(this);
    mapPanel->setMinimumWidth(200);
    mapPanel->setMinimumHeight(200);
    mapPanel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mapPanel->setLineWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

    QVBoxLayout* const vlay2  = new QVBoxLayout(mapPanel);
    d->map                    = new KGeoMap::KGeoMapWidget(mapPanel);
    d->map->setAvailableMouseModes(KGeoMap::MouseModePan|KGeoMap::MouseModeZoomIntoGroup);
    d->map->setVisibleMouseModes(KGeoMap::MouseModePan|KGeoMap::MouseModeZoomIntoGroup);
    d->map->setEnabledExtraActions(KGeoMap::ExtraActionSticky);
    d->map->setVisibleExtraActions(KGeoMap::ExtraActionSticky);
    d->gpsImageInfoSorter = new GPSImageInfoSorter(this);
    d->gpsImageInfoSorter->addToKGeoMapWidget(d->map);
    vlay2->addWidget(d->map);
    vlay2->setMargin(0);
    vlay2->setSpacing(0);

    // --------------------------------------------------------

    d->itemModel        = new QStandardItemModel(this);
    d->gpsModelHelper   = new ImageGPSModelHelper(d->itemModel, this);
    d->itemMarkerTiler  = new KGeoMap::ItemMarkerTiler(d->gpsModelHelper, this);
    d->map->setGroupedModel(d->itemMarkerTiler);

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

    QWidget* const box            = new KHBox(this);
    QHBoxLayout* const hBoxLayout = reinterpret_cast<QHBoxLayout*>(box->layout());

    if (hBoxLayout)
    {
        hBoxLayout->addStretch();
    }

    d->detailsCombo = new KComboBox(box);
    d->detailsBtn   = new QToolButton(box);
    d->detailsBtn->setIcon(SmallIcon("internet-web-browser"));
    d->detailsBtn->setToolTip(i18n("See more information on the Internet"));
    d->detailsCombo->insertItem(MapQuest,      QString("MapQuest"));
    d->detailsCombo->insertItem(GoogleMaps,    QString("Google Maps"));
    d->detailsCombo->insertItem(MsnMaps,       QString("MSN Maps"));
    d->detailsCombo->insertItem(MultiMap,      QString("MultiMap"));
    d->detailsCombo->insertItem(OpenStreetMap, QString("OpenStreetMap"));
    d->detailsCombo->insertItem(LocAlizeMaps,  QString("loc.alize.us Maps"));

    // --------------------------------------------------------

    layout->addWidget(mapPanel,                   0, 0, 1, 2);
    layout->addWidget(d->altLabel,                1, 0, 1, 1);
    layout->addWidget(d->altitude,                1, 1, 1, 1);
    layout->addWidget(d->latLabel,                2, 0, 1, 1);
    layout->addWidget(d->latitude,                2, 1, 1, 1);
    layout->addWidget(d->lonLabel,                3, 0, 1, 1);
    layout->addWidget(d->longitude,               3, 1, 1, 1);
    layout->addWidget(d->dateLabel,               4, 0, 1, 1);
    layout->addWidget(d->date,                    4, 1, 1, 1);
    layout->addWidget(d->map->getControlWidget(), 5, 0, 1, 2);
    layout->addWidget(box,                        6, 0, 1, 2);
    layout->setRowStretch(0, 10);
    layout->setColumnStretch(1, 10);
    layout->setSpacing(0);
    layout->setMargin(0);

    // --------------------------------------------------------

    connect(d->detailsBtn, SIGNAL(clicked()),
            this, SLOT(slotGPSDetails()));
}

ImagePropertiesGPSTab::~ImagePropertiesGPSTab()
{
    delete d;
}

void ImagePropertiesGPSTab::readSettings(const KConfigGroup& group)
{
    d->gpsImageInfoSorter->setSortOptions(
            GPSImageInfoSorter::SortOptions(group.readEntry("Sort Order", int(d->gpsImageInfoSorter->getSortOptions())))
        );
    setWebGPSLocator(group.readEntry("Web GPS Locator", getWebGPSLocator()));

    KConfigGroup groupMapWidget = KConfigGroup(&group, "Map Widget");
    d->map->readSettingsFromGroup(&groupMapWidget);

}
void ImagePropertiesGPSTab::writeSettings(KConfigGroup& group)
{
    group.writeEntry("Sort Order", int(d->gpsImageInfoSorter->getSortOptions()));
    group.writeEntry("Web GPS Locator", getWebGPSLocator());

    KConfigGroup groupMapWidget = KConfigGroup(&group, "Map Widget");
    d->map->saveSettingsToGroup(&groupMapWidget);
}

int ImagePropertiesGPSTab::getWebGPSLocator() const
{
    return d->detailsCombo->currentIndex();
}

void ImagePropertiesGPSTab::setWebGPSLocator(int locator)
{
    d->detailsCombo->setCurrentIndex(locator);
}

void ImagePropertiesGPSTab::slotGPSDetails()
{
    QString val, url;

    if (d->gpsInfoList.isEmpty())
    {
        return;
    }

    const GPSImageInfo info = d->gpsInfoList.first();

    switch ( getWebGPSLocator() )
    {
        case MapQuest:
        {
            url.append("http://www.mapquest.com/maps/map.adp?searchtype=address"
                       "&formtype=address&latlongtype=decimal");
            url.append("&latitude=");
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append("&longitude=");
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            break;
        }

        case GoogleMaps:
        {
            url.append("http://maps.google.com/?q=");
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append(",");
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            url.append("&spn=0.05,0.05&t=h&om=1");
            break;
        }

        case LocAlizeMaps:
        {
            url.append("http://loc.alize.us/#/geo:");
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append(",");
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            url.append(",4,k/");
            break;
        }

        case MsnMaps:
        {
            url.append("http://maps.msn.com/map.aspx?");
            url.append("&lats1=");
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append("&lons1=");
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            url.append("&name=HERE");
            url.append("&alts1=7");
            break;
        }

        case MultiMap:
        {
            url.append("http://www.multimap.com/map/browse.cgi?");
            url.append("lat=");
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append("&lon=");
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            url.append("&scale=10000");
            url.append("&icon=x");
            break;
        }

        case OpenStreetMap:
        {
            // lat and lon would also work, but wouldn't show a marker
            url.append("http://www.openstreetmap.org/?");
            url.append("mlat=");
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append("&mlon=");
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            url.append("&zoom=15");
            break;
        }
    }

    kDebug() << url;
    KToolInvocation::self()->invokeBrowser(url);
}

void ImagePropertiesGPSTab::setCurrentURL(const KUrl& url)
{
    if (url.isEmpty())
    {
        clearGPSInfo();
        return;
    }

    const DMetadata meta(url.toLocalFile());

    setMetadata(meta, url);
}

void ImagePropertiesGPSTab::setMetadata(const DMetadata& meta, const KUrl& url)
{
    double lat, lng;
    const bool haveCoordinates = meta.getGPSLatitudeNumber(&lat) && meta.getGPSLongitudeNumber(&lng);

    if (haveCoordinates)
    {
        double alt;
        const bool haveAlt = meta.getGPSAltitude(&alt);

        KGeoMap::GeoCoordinates coordinates(lat, lng);

        if (haveAlt)
        {
            coordinates.setAlt(alt);
        }

        GPSImageInfo gpsInfo;
        gpsInfo.coordinates = coordinates;
        gpsInfo.dateTime    = meta.getImageDateTime();
        gpsInfo.rating      = meta.getImageRating();
        gpsInfo.url         = url;
        setGPSInfoList(GPSImageInfo::List() << gpsInfo);
    }
    else
    {
        clearGPSInfo();
    }
}

void ImagePropertiesGPSTab::clearGPSInfo()
{
    d->altitude->clear();
    d->latitude->clear();
    d->longitude->clear();
    d->date->clear();
    d->itemModel->clear();
    setEnabled(false);
}

void ImagePropertiesGPSTab::setGPSInfoList(const GPSImageInfo::List& list)
{
    // Clear info label
    d->altitude->clear();
    d->latitude->clear();
    d->longitude->clear();
    d->date->clear();
    d->gpsInfoList.clear();
    d->itemModel->clear();
    d->gpsInfoList = list;

    setEnabled(!list.isEmpty());

    if (list.isEmpty())
    {
        return;
    }

    if (list.count() == 1)
    {
        const GPSImageInfo info = list.first();
        const KGeoMap::GeoCoordinates& coordinates = info.coordinates;

        if (!coordinates.hasAltitude())
        {
            d->altitude->setText(i18n("Undefined"));
        }
        else
        {
            d->altitude->setText(QString("%1 m").arg(QString::number(coordinates.alt())));
        }

        d->latitude->setText(QString::number(coordinates.lat()));
        d->longitude->setText(QString::number(coordinates.lon()));
        d->date->setText(KGlobal::locale()->formatDateTime(info.dateTime,
                         KLocale::ShortDate, true));
    }

    for (int i=0; i<d->gpsInfoList.count(); ++i)
    {
        QStandardItem* const currentImageGPSItem = new QStandardItem();
        currentImageGPSItem->setData(QVariant::fromValue(d->gpsInfoList.at(i)), RoleGPSImageInfo);
        d->itemModel->appendRow(currentImageGPSItem);
    }

    if (!d->map->getStickyModeState())
    {
        if (!d->map->getActiveState())
        {
            d->boundariesShouldBeAdjusted = true;
        }
        else
        {
            d->boundariesShouldBeAdjusted = false;
            d->map->adjustBoundariesToGroupedMarkers();
        }
    }
}

void ImagePropertiesGPSTab::setActive(const bool state)
{
    d->map->setActive(state);

    if (state && d->boundariesShouldBeAdjusted)
    {
        d->boundariesShouldBeAdjusted = false;
        d->map->adjustBoundariesToGroupedMarkers();
    }
}

}  // namespace Digikam
