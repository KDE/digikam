/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a tab widget to display GPS info
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagepropertiesgpstab.h"

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
#include <QComboBox>
#include <QDesktopServices>
#include <QIcon>
#include <QLocale>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "mapwidget.h"
#include "itemmarkertiler.h"
#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "imagegpsmodelhelper.h"
#include "dexpanderbox.h"
//#include "webbrowserdlg.h"

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

    QLabel*                    altLabel;
    QLabel*                    latLabel;
    QLabel*                    lonLabel;
    QLabel*                    dateLabel;

    QToolButton*               detailsBtn;
    QComboBox*                 detailsCombo;

    DAdjustableLabel*          altitude;
    DAdjustableLabel*          latitude;
    DAdjustableLabel*          longitude;
    DAdjustableLabel*          date;

    MapWidget*                 map;
    ItemMarkerTiler*           itemMarkerTiler;
    GPSImageInfo::List         gpsInfoList;

    QStandardItemModel*        itemModel;
    ImageGPSModelHelper*       gpsModelHelper;
    GPSImageInfoSorter*        gpsImageInfoSorter;
    bool                       boundariesShouldBeAdjusted;
};

ImagePropertiesGPSTab::ImagePropertiesGPSTab(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* const layout = new QGridLayout(this);

    // --------------------------------------------------------

    QFrame* const mapPanel    = new QFrame(this);
    mapPanel->setMinimumWidth(200);
    mapPanel->setMinimumHeight(200);
    mapPanel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mapPanel->setLineWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

    QVBoxLayout* const vlay2  = new QVBoxLayout(mapPanel);
    d->map                    = new MapWidget(mapPanel);
    d->map->setAvailableMouseModes(MouseModePan | MouseModeZoomIntoGroup);
    d->map->setVisibleMouseModes(MouseModePan | MouseModeZoomIntoGroup);
    d->map->setEnabledExtraActions(ExtraActionSticky);
    d->map->setVisibleExtraActions(ExtraActionSticky);
    d->map->setBackend(QLatin1String("marble"));
    d->gpsImageInfoSorter     = new GPSImageInfoSorter(this);
    d->gpsImageInfoSorter->addToMapWidget(d->map);
    vlay2->addWidget(d->map);
    vlay2->setContentsMargins(QMargins());
    vlay2->setSpacing(0);

    // --------------------------------------------------------

    d->itemModel        = new QStandardItemModel(this);
    d->gpsModelHelper   = new ImageGPSModelHelper(d->itemModel, this);
    d->itemMarkerTiler  = new ItemMarkerTiler(d->gpsModelHelper, this);
    d->map->setGroupedModel(d->itemMarkerTiler);

    d->altLabel         = new QLabel(i18n("<b>Altitude</b>:"),  this);
    d->latLabel         = new QLabel(i18n("<b>Latitude</b>:"),  this);
    d->lonLabel         = new QLabel(i18n("<b>Longitude</b>:"), this);
    d->dateLabel        = new QLabel(i18n("<b>Date</b>:"),      this);
    d->altitude         = new DAdjustableLabel(this);
    d->latitude         = new DAdjustableLabel(this);
    d->longitude        = new DAdjustableLabel(this);
    d->date             = new DAdjustableLabel(this);
    d->altitude->setAlignment(Qt::AlignRight);
    d->latitude->setAlignment(Qt::AlignRight);
    d->longitude->setAlignment(Qt::AlignRight);
    d->date->setAlignment(Qt::AlignRight);

    // --------------------------------------------------------

    QWidget* const box            = new DHBox(this);
    QHBoxLayout* const hBoxLayout = reinterpret_cast<QHBoxLayout*>(box->layout());

    if (hBoxLayout)
    {
        hBoxLayout->addStretch();
    }

    d->detailsCombo = new QComboBox(box);
    d->detailsBtn   = new QToolButton(box);
    d->detailsBtn->setIcon(QIcon::fromTheme(QLatin1String("globe")));
    d->detailsBtn->setToolTip(i18n("See more information on the Internet"));
    d->detailsCombo->insertItem(MapQuest,      QLatin1String("MapQuest"));
    d->detailsCombo->insertItem(GoogleMaps,    QLatin1String("Google Maps"));
    d->detailsCombo->insertItem(BingMaps,      QLatin1String("Bing Maps"));
    d->detailsCombo->insertItem(OpenStreetMap, QLatin1String("OpenStreetMap"));
    d->detailsCombo->insertItem(LocAlizeMaps,  QLatin1String("loc.alize.us Maps"));

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
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);

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
    d->gpsImageInfoSorter->setSortOptions(GPSImageInfoSorter::SortOptions(group.readEntry(QLatin1String("Sort Order"),
                                          int(d->gpsImageInfoSorter->getSortOptions()))));
    setWebGPSLocator(group.readEntry(QLatin1String("Web GPS Locator"), getWebGPSLocator()));

    KConfigGroup groupMapWidget = KConfigGroup(&group, QLatin1String("Map Widget"));
    d->map->readSettingsFromGroup(&groupMapWidget);

}
void ImagePropertiesGPSTab::writeSettings(KConfigGroup& group)
{
    group.writeEntry(QLatin1String("Sort Order"),      int(d->gpsImageInfoSorter->getSortOptions()));
    group.writeEntry(QLatin1String("Web GPS Locator"), getWebGPSLocator());

    KConfigGroup groupMapWidget = KConfigGroup(&group, QLatin1String("Map Widget"));
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
            url.append(QLatin1String("http://www.mapquest.com/maps/map.adp?searchtype=address"
                                     "&formtype=address&latlongtype=decimal"));
            url.append(QLatin1String("&latitude="));
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append(QLatin1String("&longitude="));
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            break;
        }

        case GoogleMaps:
        {
            url.append(QLatin1String("http://maps.google.com/?q="));
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append(QLatin1String(","));
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            url.append(QLatin1String("&spn=0.05,0.05&t=h&om=1"));
            break;
        }

        case LocAlizeMaps:
        {
            url.append(QLatin1String("http://loc.alize.us/#/geo:"));
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append(QLatin1String(","));
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            url.append(QLatin1String(",15,/"));
            break;
        }

        case BingMaps:
        {
            url.append(QLatin1String("http://www.bing.com/maps/?v=2&where1="));
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append(QLatin1String(","));
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            break;
        }

        case OpenStreetMap:
        {
            // lat and lon would also work, but wouldn't show a marker
            url.append(QLatin1String("http://www.openstreetmap.org/?"));
            url.append(QLatin1String("mlat="));
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append(QLatin1String("&mlon="));
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            url.append(QLatin1String("&zoom=15"));
            break;
        }
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << url;
    QDesktopServices::openUrl(QUrl(url));

//    WebBrowserDlg* const browser = new WebBrowserDlg(QUrl(url), this);
//    browser->show();
}

void ImagePropertiesGPSTab::setCurrentURL(const QUrl& url)
{
    if (url.isEmpty())
    {
        clearGPSInfo();
        return;
    }

    const DMetadata meta(url.toLocalFile());

    setMetadata(meta, url);
}

void ImagePropertiesGPSTab::setMetadata(const DMetadata& meta, const QUrl& url)
{
    double lat, lng;
    const bool haveCoordinates = meta.getGPSLatitudeNumber(&lat) && meta.getGPSLongitudeNumber(&lng);

    if (haveCoordinates)
    {
        double alt;
        const bool haveAlt = meta.getGPSAltitude(&alt);

        GeoCoordinates coordinates(lat, lng);

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
    d->altitude->setAdjustedText();
    d->latitude->setAdjustedText();
    d->longitude->setAdjustedText();
    d->date->setAdjustedText();
    d->itemModel->clear();
    setEnabled(false);
}

void ImagePropertiesGPSTab::setGPSInfoList(const GPSImageInfo::List& list)
{
    // Clear info label
    d->altitude->setAdjustedText();
    d->latitude->setAdjustedText();
    d->longitude->setAdjustedText();
    d->date->setAdjustedText();
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
        const GPSImageInfo info                     = list.first();
        const GeoCoordinates& coordinates = info.coordinates;

        if (!coordinates.hasAltitude())
        {
            d->altitude->setAdjustedText(i18n("Undefined"));
        }
        else
        {
            d->altitude->setAdjustedText(QString::fromLatin1("%1 m").arg(QString::number(coordinates.alt())));
        }

        d->latitude->setAdjustedText(QString::number(coordinates.lat()));
        d->longitude->setAdjustedText(QString::number(coordinates.lon()));
        d->date->setAdjustedText(QLocale().toString(info.dateTime, QLocale::ShortFormat));
    }

    for (int i = 0 ; i < d->gpsInfoList.count() ; ++i)
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

} // namespace Digikam
