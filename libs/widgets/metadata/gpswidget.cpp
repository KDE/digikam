/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a tab widget to display GPS info
 *
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes.

#include <khbox.h>
#include <kdialog.h>
#include <klocale.h>
#include <ktoolinvocation.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "metadatalistview.h"
#include "worldmapwidget.h"
#include "gpswidget.h"
#include "gpswidget.moc"

namespace Digikam
{

static const char* ExifGPSHumanList[] =
{
     "GPSLatitude",
     "GPSLongitude",
     "GPSAltitude",
     "-1"
};

// Standard Exif Entry list from to less important to the most important for photograph.
static const char* StandardExifGPSEntryList[] =
{
     "GPSInfo",
     "-1"
};

class GPSWidgetPriv
{

public:

    GPSWidgetPriv()
    {
        detailsButton = 0;
        detailsCombo  = 0;
        map           = 0;
    }

    QStringList     tagsfilter;
    QStringList     keysFilter;

    QPushButton    *detailsButton;

    QComboBox      *detailsCombo;

    WorldMapWidget *map;
};

GPSWidget::GPSWidget(QWidget* parent, const char* name)
         : MetadataWidget(parent, name)
{
    d = new GPSWidgetPriv;

    for (int i=0 ; QString(StandardExifGPSEntryList[i]) != QString("-1") ; i++)
        d->keysFilter << StandardExifGPSEntryList[i];

    for (int i=0 ; QString(ExifGPSHumanList[i]) != QString("-1") ; i++)
        d->tagsfilter << ExifGPSHumanList[i];

    // --------------------------------------------------------

    QWidget *gpsInfo    = new QWidget(this);
    QGridLayout *layout = new QGridLayout(gpsInfo);
    d->map              = new WorldMapWidget(256, 256, gpsInfo);

    // --------------------------------------------------------

    QWidget* box2           = new QWidget(gpsInfo);
    QGridLayout* box2Layout = new QGridLayout( box2 );

    d->detailsCombo  = new QComboBox( box2 );
    d->detailsButton = new QPushButton(i18n("More Info..."), box2);
    d->detailsCombo->insertItem(MapQuest,   QString("MapQuest"));
    d->detailsCombo->insertItem(GoogleMaps, QString("Google Maps"));
    d->detailsCombo->insertItem(MsnMaps,    QString("MSN Maps"));
    d->detailsCombo->insertItem(MultiMap,   QString("MultiMap"));

    box2Layout->addWidget( d->detailsCombo, 0, 0, 1, 1);
    box2Layout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                             QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 0, 0, 1, 1);
    box2Layout->addWidget( d->detailsButton, 0, 2, 1, 1);
    box2Layout->setColumnStretch(3, 10);
    box2Layout->setSpacing(0);
    box2Layout->setMargin(0);

    // --------------------------------------------------------

    layout->addWidget(d->map, 0, 0, 1, 3 );
    layout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                         QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 1, 0, 1, 3);
    layout->addWidget(box2, 2, 0, 1, 1);
    layout->setColumnStretch(2, 10);
    layout->setSpacing(0);
    layout->setMargin(0);

    // --------------------------------------------------------

    connect(d->detailsButton, SIGNAL(clicked()),
            this, SLOT(slotGPSDetails()));

    setUserAreaWidget(gpsInfo);
    decodeMetadata();
}

GPSWidget::~GPSWidget()
{
    delete d;
}

int GPSWidget::getWebGPSLocator(void)
{
    return ( d->detailsCombo->currentIndex() );
}

void GPSWidget::setWebGPSLocator(int locator)
{
    d->detailsCombo->setCurrentIndex(locator);
}

void GPSWidget::slotGPSDetails(void)
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

    KToolInvocation::self()->invokeBrowser(url);
}

QString GPSWidget::getMetadataTitle(void)
{
    return i18n("Global Positioning System Information");
}

bool GPSWidget::loadFromURL(const KUrl& url)
{
    setFileName(url.path());

    if (url.isEmpty())
    {
        setMetadata();
        return false;
    }
    else
    {
        DMetadata metadata(url.path());

        if (!metadata.hasExif())
        {
            setMetadata();
            return false;
        }
        else
            setMetadata(metadata);
    }

    return true;
}

bool GPSWidget::decodeMetadata()
{
    DMetadata data = getMetadata();
    if (!data.hasExif())
    {
        setMetadataEmpty();
        return false;
    }

    // Update all metadata contents.
    setMetadataMap(data.getExifTagsDataList(d->keysFilter));

    bool ret = decodeGPSPosition();
    if (!ret)
    {
        setMetadataEmpty();
        return false;
    }

    d->map->setEnabled(true);
    d->detailsButton->setEnabled(true);
    d->detailsCombo->setEnabled(true);
    return true;
}

void GPSWidget::setMetadataEmpty()
{
    MetadataWidget::setMetadataEmpty();
    d->map->setEnabled(false);
    d->detailsButton->setEnabled(false);
    d->detailsCombo->setEnabled(false);
}

void GPSWidget::buildView(void)
{

    if (getMode() == SIMPLE)
    {
        setIfdList(getMetadataMap(), d->keysFilter, d->tagsfilter);
    }
    else
    {
        setIfdList(getMetadataMap(), d->keysFilter, QStringList());
    }
}

QString GPSWidget::getTagTitle(const QString& key)
{
    QString title = DMetadata::getExifTagTitle(key.toAscii());

    if (title.isEmpty())
        return i18n("Unknown");

    return title;
}

QString GPSWidget::getTagDescription(const QString& key)
{
    QString desc = DMetadata::getExifTagDescription(key.toAscii());

    if (desc.isEmpty())
        return i18n("No description available");

    return desc;
}

bool GPSWidget::decodeGPSPosition(void)
{
    double latitude=0.0, longitude=0.0, altitude=0.0;

    DMetadata meta = getMetadata();

    if (meta.getGPSInfo(altitude, latitude, longitude))
        d->map->setGPSPosition(latitude, longitude);
    else 
        return false;

    return true;
}

void GPSWidget::slotSaveMetadataToFile(void)
{
    KUrl url = saveMetadataToFile(i18n("EXIF File to Save"),
                                  QString("*.dat|"+i18n("EXIF binary Files (*.dat)")));
    storeMetadataToFile(url, getMetadata().getExif());
}

}  // namespace Digikam
