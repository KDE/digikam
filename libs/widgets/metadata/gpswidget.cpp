/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2006-02-22
 * Description : a tab widget to display GPS info
 *
 * Copyright 2006-2007 by Gilles Caulier
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

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>

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

// LibExiv2 includes.

#include <exiv2/tags.hpp>
#include <exiv2/exif.hpp>

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

int GPSWidget::getWebGPSLocator(void)
{
    return ( d->detailsCombo->currentItem() );
}
    
void GPSWidget::setWebGPSLocator(int locator)
{
    d->detailsCombo->setCurrentItem(locator);
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
    
    KApplication::kApplication()->invokeBrowser(url);
}

QString GPSWidget::getMetadataTitle(void)
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
    try
    {
        Exiv2::ExifData exifData;
        if (exifData.load((Exiv2::byte*)getMetadata().data(), getMetadata().size()) != 0)
        {
            setMetadataEmpty();
            return false;
        }

        exifData.sortByKey();
        
        QString     ifDItemName;
        MetaDataMap metaDataMap;

        for (Exiv2::ExifData::iterator md = exifData.begin(); md != exifData.end(); ++md)
        {
            QString key = QString::fromAscii(md->key().c_str());

            // Decode the tag value with a user friendly output.
            std::ostringstream os;
            os << *md;

            // Exif tag contents can be an i18n strings, no only simple ascii.
            QString tagValue = QString::fromLocal8Bit(os.str().c_str());
            tagValue.replace("\n", " ");

            // We apply a filter to get only standard Exif tags, not maker notes.
            if (d->keysFilter.contains(key.section(".", 1, 1)))
                metaDataMap.insert(key, tagValue);
        }

        // Update all metadata contents.
        setMetadataMap(metaDataMap);
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
    catch (Exiv2::Error& e)
    {
        setMetadataEmpty();
        DMetadata::printExiv2ExceptionError("Cannot parse EXIF metadata using Exiv2 ", e);        
    }

    return false;
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
    try 
    {
        std::string exifkey(key.ascii());
        Exiv2::ExifKey ek(exifkey); 
        return QString::fromLocal8Bit( Exiv2::ExifTags::tagTitle(ek.tag(), ek.ifdId()) );
    }
    catch (Exiv2::Error& e) 
    {
        DMetadata::printExiv2ExceptionError("Cannot get metadata tag title using Exiv2 ", e);
    }

    return i18n("Unknown");
}

QString GPSWidget::getTagDescription(const QString& key)
{
    try 
    {
        std::string exifkey(key.ascii());
        Exiv2::ExifKey ek(exifkey); 
        return QString::fromLocal8Bit( Exiv2::ExifTags::tagDesc(ek.tag(), ek.ifdId()) );
    }
    catch (Exiv2::Error& e) 
    {   
        DMetadata::printExiv2ExceptionError("Cannot get metadata tag description using Exiv2 ", e);
    }

    return i18n("No description available");
}

bool GPSWidget::decodeGPSPosition(void)
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

void GPSWidget::slotSaveMetadataToFile(void)
{
    KURL url = saveMetadataToFile(i18n("EXIF File to Save"),
                                  QString("*.dat|"+i18n("EXIF binary Files (*.dat)")));
    storeMetadataToFile(url);
}

}  // namespace Digikam

