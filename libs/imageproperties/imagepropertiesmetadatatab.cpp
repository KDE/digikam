/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2004-11-17
 * Description : a tab to display metadata image informations
 *
 * Copyright 2004-2006 by Gilles Caulier
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
#include <kdebug.h>
#include <ktabwidget.h>

// Local includes.

#include "dmetadata.h"
#include "exifwidget.h"
#include "makernotewidget.h"
#include "iptcwidget.h"
#include "gpswidget.h"
#include "navigatebarwidget.h"
#include "imagepropertiesmetadatatab.h"

namespace Digikam
{

class ImagePropertiesMetadataTabPriv
{
public:

    ImagePropertiesMetadataTabPriv()
    {
        exifWidget      = 0;
        makernoteWidget = 0;
        iptcWidget      = 0;
        navigateBar     = 0;
        gpsWidget       = 0;
    }

    ExifWidget        *exifWidget;
    MakerNoteWidget   *makernoteWidget;
    IptcWidget        *iptcWidget;
    GPSWidget         *gpsWidget;
        
    NavigateBarWidget *navigateBar;
};

ImagePropertiesMetaDataTab::ImagePropertiesMetaDataTab(QWidget* parent, bool navBar)
                          : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImagePropertiesMetadataTabPriv;

    QVBoxLayout *vLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    d->navigateBar       = new NavigateBarWidget(this, navBar);
    KTabWidget *tab      = new KTabWidget(this);
    vLayout->addWidget(d->navigateBar);
    vLayout->addWidget(tab);
    
    // Exif tab area -----------------------------------------------------

    d->exifWidget = new ExifWidget(tab);
    tab->addTab(d->exifWidget, i18n("Exif") );

    // Makernote tab area -----------------------------------------------------

    d->makernoteWidget = new MakerNoteWidget(tab);
    tab->addTab(d->makernoteWidget, i18n("Makernote") );

    // IPTC tab area ---------------------------------------
    
    d->iptcWidget = new IptcWidget(tab);
    tab->addTab(d->iptcWidget, i18n("Iptc") );

    // GPS tab area ---------------------------------------
    
    d->gpsWidget = new GPSWidget(tab);
    tab->addTab(d->gpsWidget, i18n("GPS") );

    // -------------------------------------------------------------
            
    connect(d->navigateBar, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(d->navigateBar, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(d->navigateBar, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->navigateBar, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));

    // -- read config ---------------------------------------------------------

    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    d->exifWidget->setMode(config->readNumEntry("EXIF Level", ExifWidget::SIMPLE));
    d->makernoteWidget->setMode(config->readNumEntry("MAKERNOTE Level", MakerNoteWidget::SIMPLE));
    d->iptcWidget->setMode(config->readNumEntry("IPTC Level", IptcWidget::SIMPLE));
    d->gpsWidget->setMode(config->readNumEntry("GPS Level", GPSWidget::SIMPLE));    d->exifWidget->setCurrentItemByKey(config->readEntry("Current EXIF Item", QString()));
    d->makernoteWidget->setCurrentItemByKey(config->readEntry("Current MAKERNOTE Item", QString()));
    d->iptcWidget->setCurrentItemByKey(config->readEntry("Current IPTC Item", QString()));    d->gpsWidget->setCurrentItemByKey(config->readEntry("Current GPS Item", QString()));
}

ImagePropertiesMetaDataTab::~ImagePropertiesMetaDataTab()
{
    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    config->writeEntry("EXIF Level", d->exifWidget->getMode());
    config->writeEntry("MAKERNOTE Level", d->makernoteWidget->getMode());
    config->writeEntry("IPTC Level", d->iptcWidget->getMode());
    config->writeEntry("GPS Level", d->gpsWidget->getMode());
    config->writeEntry("Current EXIF Item", d->exifWidget->getCurrentItemKey());
    config->writeEntry("Current MAKERNOTE Item", d->makernoteWidget->getCurrentItemKey());
    config->writeEntry("Current IPTC Item", d->iptcWidget->getCurrentItemKey());
    config->writeEntry("Current GPS Item", d->gpsWidget->getCurrentItemKey());
    delete d;
}

void ImagePropertiesMetaDataTab::setCurrentURL(const KURL& url, int itemType)
{
    if (url.isEmpty())
    {
        d->exifWidget->loadFromURL(url);
        d->makernoteWidget->loadFromURL(url);
        d->iptcWidget->loadFromURL(url);
        d->gpsWidget->loadFromURL(url);
        d->navigateBar->setFileName();
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

    d->navigateBar->setFileName(url.filename());
    d->navigateBar->setButtonsState(itemType);
}
    
void ImagePropertiesMetaDataTab::setCurrentData(const QByteArray& exifData, 
                                                const QByteArray& iptcData, 
                                                const QString& filename, int itemType)
{
    if (exifData.isEmpty() && iptcData.isEmpty())
    {
        d->exifWidget->loadFromData(filename, exifData);
        d->makernoteWidget->loadFromData(filename, exifData);
        d->iptcWidget->loadFromData(filename, iptcData);
        d->gpsWidget->loadFromData(filename, exifData);
        d->navigateBar->setFileName();
        setEnabled(false);
        return;
    }

    setEnabled(true);
    
    d->exifWidget->loadFromData(filename, exifData);
    d->makernoteWidget->loadFromData(filename, exifData);
    d->iptcWidget->loadFromData(filename, iptcData);
    d->gpsWidget->loadFromData(filename, exifData);
        
    d->navigateBar->setFileName(filename);
    d->navigateBar->setButtonsState(itemType);
}

}  // NameSpace Digikam

#include "imagepropertiesmetadatatab.moc"
