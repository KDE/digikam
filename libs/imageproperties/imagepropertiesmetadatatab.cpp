/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : a tab to display metadata information of images
 *
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qlayout.h>
#include <qfile.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qfileinfo.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kfileitem.h>
#include <ktabwidget.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "exifwidget.h"
#include "makernotewidget.h"
#include "iptcwidget.h"
#include "gpswidget.h"
#include "navigatebarwidget.h"
#include "imagepropertiesmetadatatab.h"
#include "imagepropertiesmetadatatab.moc"

namespace Digikam
{

class ImagePropertiesMetadataTabPriv
{
public:

    enum MetadataTab
    {
        EXIF=0,
        MAKERNOTE,
        IPTC,
        GPS
    };

    ImagePropertiesMetadataTabPriv()
    {
        exifWidget      = 0;
        makernoteWidget = 0;
        iptcWidget      = 0;
        gpsWidget       = 0;
        tab             = 0;
    }

    KTabWidget        *tab;

    ExifWidget        *exifWidget;

    MakerNoteWidget   *makernoteWidget;

    IptcWidget        *iptcWidget;

    GPSWidget         *gpsWidget;
};

ImagePropertiesMetaDataTab::ImagePropertiesMetaDataTab(QWidget* parent, bool navBar)
                          : NavigateBarTab(parent)
{
    d = new ImagePropertiesMetadataTabPriv;

    setupNavigateBar(navBar);
    d->tab = new KTabWidget(this);
    m_navigateBarLayout->addWidget(d->tab);

    // Exif tab area -----------------------------------------------------

    d->exifWidget = new ExifWidget(d->tab);
    d->tab->insertTab(d->exifWidget, i18n("EXIF"), ImagePropertiesMetadataTabPriv::EXIF);

    // Makernote tab area -----------------------------------------------------

    d->makernoteWidget = new MakerNoteWidget(d->tab);
    d->tab->insertTab(d->makernoteWidget, i18n("Makernote"), ImagePropertiesMetadataTabPriv::MAKERNOTE);

    // IPTC tab area ---------------------------------------
    
    d->iptcWidget = new IptcWidget(d->tab);
    d->tab->insertTab(d->iptcWidget, i18n("IPTC"), ImagePropertiesMetadataTabPriv::IPTC);

    // GPS tab area ---------------------------------------
    
    d->gpsWidget = new GPSWidget(d->tab);
    d->tab->insertTab(d->gpsWidget, i18n("GPS"), ImagePropertiesMetadataTabPriv::GPS);

    // -- read config ---------------------------------------------------------

    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    d->tab->setCurrentPage(config->readNumEntry("ImagePropertiesMetaData Tab",
                           ImagePropertiesMetadataTabPriv::EXIF));
    d->exifWidget->setMode(config->readNumEntry("EXIF Level", ExifWidget::SIMPLE));
    d->makernoteWidget->setMode(config->readNumEntry("MAKERNOTE Level", MakerNoteWidget::SIMPLE));
    d->iptcWidget->setMode(config->readNumEntry("IPTC Level", IptcWidget::SIMPLE));
    d->gpsWidget->setMode(config->readNumEntry("GPS Level", GPSWidget::SIMPLE));
    d->exifWidget->setCurrentItemByKey(config->readEntry("Current EXIF Item", QString()));
    d->makernoteWidget->setCurrentItemByKey(config->readEntry("Current MAKERNOTE Item", QString()));
    d->iptcWidget->setCurrentItemByKey(config->readEntry("Current IPTC Item", QString()));
    d->gpsWidget->setCurrentItemByKey(config->readEntry("Current GPS Item", QString()));
    d->gpsWidget->setWebGPSLocator(config->readNumEntry("Current Web GPS Locator", GPSWidget::MapQuest));
}

ImagePropertiesMetaDataTab::~ImagePropertiesMetaDataTab()
{
    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    config->writeEntry("ImagePropertiesMetaData Tab", d->tab->currentPageIndex());
    config->writeEntry("EXIF Level", d->exifWidget->getMode());
    config->writeEntry("MAKERNOTE Level", d->makernoteWidget->getMode());
    config->writeEntry("IPTC Level", d->iptcWidget->getMode());
    config->writeEntry("GPS Level", d->gpsWidget->getMode());
    config->writeEntry("Current EXIF Item", d->exifWidget->getCurrentItemKey());
    config->writeEntry("Current MAKERNOTE Item", d->makernoteWidget->getCurrentItemKey());
    config->writeEntry("Current IPTC Item", d->iptcWidget->getCurrentItemKey());
    config->writeEntry("Current GPS Item", d->gpsWidget->getCurrentItemKey());
    config->writeEntry("Current Web GPS Locator", d->gpsWidget->getWebGPSLocator());
    config->sync();
    
    delete d;
}

void ImagePropertiesMetaDataTab::setCurrentURL(const KURL& url)
{
    if (url.isEmpty())
    {
        d->exifWidget->loadFromURL(url);
        d->makernoteWidget->loadFromURL(url);
        d->iptcWidget->loadFromURL(url);
        d->gpsWidget->loadFromURL(url);
        setEnabled(false);
        return;
    }

    setEnabled(true);
    DMetadata metadata(url.path());

    QByteArray exifData = metadata.getExif(); 
    QByteArray iptcData = metadata.getIptc();

    d->exifWidget->loadFromData(url.filename(), exifData);
    d->makernoteWidget->loadFromData(url.filename(), exifData);
    d->iptcWidget->loadFromData(url.filename(), iptcData);
    d->gpsWidget->loadFromData(url.filename(), exifData);
}
    
void ImagePropertiesMetaDataTab::setCurrentData(const QByteArray& exifData, 
                                                const QByteArray& iptcData, 
                                                const QString& filename)
{
    if (exifData.isEmpty() && iptcData.isEmpty())
    {
        d->exifWidget->loadFromData(filename, exifData);
        d->makernoteWidget->loadFromData(filename, exifData);
        d->iptcWidget->loadFromData(filename, iptcData);
        d->gpsWidget->loadFromData(filename, exifData);
        setEnabled(false);
        return;
    }

    setEnabled(true);
    
    d->exifWidget->loadFromData(filename, exifData);
    d->makernoteWidget->loadFromData(filename, exifData);
    d->iptcWidget->loadFromData(filename, iptcData);
    d->gpsWidget->loadFromData(filename, exifData);
}

}  // NameSpace Digikam

