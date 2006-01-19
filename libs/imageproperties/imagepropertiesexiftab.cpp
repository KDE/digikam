/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description : A tab to display Exif image informations
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
#include <qlabel.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kfileitem.h>

// LibKExif includes.

#include <libkexif/kexifwidget.h>

// Local includes.

#include "navigatebarwidget.h"
#include "imagepropertiesexiftab.h"

namespace Digikam
{

class ImagePropertiesEXIFTabPriv
{
public:

    ImagePropertiesEXIFTabPriv(){}

    QComboBox         *levelCombo;

    QString            currentItem;
    
    KExifWidget       *exifWidget;
    
    NavigateBarWidget *navigateBar;
};

ImagePropertiesEXIFTab::ImagePropertiesEXIFTab(QWidget* parent, bool navBar)
                      : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImagePropertiesEXIFTabPriv;
    QGridLayout *topLayout = new QGridLayout(this, 2, 2, KDialog::marginHint(), KDialog::spacingHint());

    d->navigateBar  = new NavigateBarWidget(this, navBar);
    topLayout->addMultiCellWidget(d->navigateBar, 0, 0, 0, 2);
        
    QLabel* levelLabel = new QLabel(i18n("Level of detail:"), this);
    d->levelCombo      = new QComboBox(this);
    topLayout->addMultiCellWidget(levelLabel, 1, 1, 0, 1);
    topLayout->addMultiCellWidget(d->levelCombo, 1, 1, 2, 2);

    QWhatsThis::add( d->levelCombo, i18n("<p>Select here the Exif information level to display<p>"
                                        "<b>Simple</b>: display general information about the photograph "
                                        " (default).<p>"
                                        "<b>Full</b>: display all EXIF sections.") );  
    
    d->exifWidget = new KExifWidget(this);
    topLayout->addMultiCellWidget(d->exifWidget, 2, 2, 0, 2);

    d->levelCombo->insertItem(i18n("Simple"));
    d->levelCombo->insertItem(i18n("Full"));
    
    connect(d->levelCombo, SIGNAL(activated(int)),
            this, SLOT(slotLevelChanged(int)));
    
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
    d->levelCombo->setCurrentItem(config->readNumEntry("EXIF Level", 0));
    d->currentItem = config->readEntry("Current EXIF Item", QString());
    slotLevelChanged(0);
}

ImagePropertiesEXIFTab::~ImagePropertiesEXIFTab()
{
    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    config->writeEntry("EXIF Level", d->levelCombo->currentItem());
    config->writeEntry("Current EXIF Item", d->currentItem);
    delete d;
}

void ImagePropertiesEXIFTab::setCurrentURL(const KURL& url, int itemType)
{
    if (url.isEmpty())
    {
       d->exifWidget->loadFile(url.path());
       d->navigateBar->setFileName();
       setEnabled(false);
       return;
    }

    setEnabled(true);

    if (!d->exifWidget->getCurrentItemName().isNull())
        d->currentItem = d->exifWidget->getCurrentItemName();
    
    d->exifWidget->loadFile(url.path());
    d->exifWidget->setCurrentItem(d->currentItem);
    
    d->navigateBar->setFileName(url.filename());
    d->navigateBar->setButtonsState(itemType);
}

void ImagePropertiesEXIFTab::slotLevelChanged(int)
{
    if (d->levelCombo->currentText() == i18n("Simple"))
        d->exifWidget->setMode(KExifWidget::SIMPLE);
    else
        d->exifWidget->setMode(KExifWidget::FULL);
}

}  // NameSpace Digikam

#include "imagepropertiesexiftab.moc"
