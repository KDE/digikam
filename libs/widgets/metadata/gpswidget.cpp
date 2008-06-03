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
Any good explainations about GPS (in French) can be found at this url :
http://www.gpspassion.com/forumsen/topic.asp?TOPIC_ID=16593
*/

// Qt includes.

#include <qlayout.h>
#include <qpushbutton.h>
#include <qmap.h>
#include <qhbox.h>
#include <qfile.h>
#include <qcombobox.h>
#include <qgroupbox.h>

// KDE includes.

#include <kdialogbase.h>
#include <klocale.h>
#include <kapplication.h>

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
    QGridLayout *layout = new QGridLayout(gpsInfo, 3, 2);
    d->map              = new WorldMapWidget(256, 256, gpsInfo);

    // --------------------------------------------------------
    
    QGroupBox* box2 = new QGroupBox( 0, Qt::Vertical, gpsInfo );
    box2->setInsideMargin(0);
    box2->setInsideSpacing(0);    
    box2->setFrameStyle( QFrame::NoFrame );
    QGridLayout* box2Layout = new QGridLayout( box2->layout(), 0, 2, KDialog::spacingHint() );

    d->detailsCombo  = new QComboBox( false, box2 );
    d->detailsButton = new QPushButton(i18n("More Info..."), box2);
    d->detailsCombo->insertItem(QString("MapQuest"), MapQuest);
    d->detailsCombo->insertItem(QString("Google Maps"), GoogleMaps);
    d->detailsCombo->insertItem(QString("MSN Maps"), MsnMaps);
    d->detailsCombo->insertItem(QString("MultiMap"), MultiMap);
    
    box2Layout->addMultiCellWidget( d->detailsCombo, 0, 0, 0, 0 );
    box2Layout->addMultiCellWidget( d->detailsButton, 0, 0, 1, 1 );
    box2Layout->setColStretch(2, 10);

    // --------------------------------------------------------
    
    layout->addMultiCellWidget(d->map, 0, 0, 0, 2);
    layout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                         QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 1, 1, 0, 2);
    layout->addMultiCellWidget(box2, 2, 2, 0, 0);
    layout->setColStretch(2, 10);
    layout->setRowStretch(3, 10);

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

int GPSWidget::getWebGPSLocator()
{
    return ( d->detailsCombo->currentItem() );
}
    
void GPSWidget::setWebGPSLocator(int locator)
{
    d->detailsCombo->setCurrentItem(locator);
}
    
void GPSWidget::slotGPSDetails()
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
    
    KApplication::kApplication()->invokeBrowser(url);
}

QString GPSWidget::getMetadataTitle()
{
    return i18n("Global Positioning System Information");
}

bool GPSWidget::loadFromURL(const KURL& url)
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
        QByteArray exifData = metadata.getExif();

        if (exifData.isEmpty())
        {
            setMetadata();
            return false;
        }
        else
            setMetadata(exifData);
    }

    return true;
}

bool GPSWidget::decodeMetadata()
{
    DMetadata metaData;
    if (!metaData.setExif(getMetadata()))
    {
        setMetadataEmpty();
        return false;
    }

    // Update all metadata contents.
    setMetadataMap(metaData.getExifTagsDataList(d->keysFilter));

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

void GPSWidget::buildView()
{    
    if (getMode() == SIMPLE)
    {
        setIfdList(getMetadataMap(), d->keysFilter, d->tagsfilter);
    }
    else
    {
        setIfdList(getMetadataMap(), d->keysFilter, QStringList());
    }

    MetadataWidget::buildView();
}

QString GPSWidget::getTagTitle(const QString& key)
{
    DMetadata meta;
    QString title = meta.getExifTagTitle(key.ascii());

    if (title.isEmpty())
        return key.section('.', -1);

    return title;
}

QString GPSWidget::getTagDescription(const QString& key)
{
    DMetadata meta;
    QString desc = meta.getExifTagDescription(key.ascii());

    if (desc.isEmpty())
        return i18n("No description available");

    return desc;
}

bool GPSWidget::decodeGPSPosition()
{
    double latitude=0.0, longitude=0.0, altitude=0.0;
    
    DMetadata meta;
    meta.setExif(getMetadata());

    if (meta.getGPSInfo(altitude, latitude, longitude))
        d->map->setGPSPosition(latitude, longitude);
    else 
        return false;

    return true;
}

void GPSWidget::slotSaveMetadataToFile()
{
    KURL url = saveMetadataToFile(i18n("EXIF File to Save"),
                                  QString("*.exif|"+i18n("EXIF binary Files (*.exif)")));
    storeMetadataToFile(url);
}

}  // namespace Digikam
