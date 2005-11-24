/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
 *
 * Copyright 2004-2005 by Gilles Caulier
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

ImagePropertiesEXIFTab::ImagePropertiesEXIFTab(QWidget* parent, bool navBar)
                      : QWidget(parent, 0, Qt::WDestructiveClose)
{
    QGridLayout *topLayout = new QGridLayout(this, 2, 2, KDialog::marginHint(), KDialog::spacingHint());

    m_navigateBar  = new NavigateBarWidget(this, navBar);
    topLayout->addMultiCellWidget(m_navigateBar, 0, 0, 0, 2);
        
    QLabel* levelLabel = new QLabel(i18n("Select level of detail:"), this);
    m_levelCombo       = new QComboBox(this);
    topLayout->addMultiCellWidget(levelLabel, 1, 1, 0, 1);
    topLayout->addMultiCellWidget(m_levelCombo, 1, 1, 2, 2);

    QWhatsThis::add( m_levelCombo, i18n("<p>Select here the Exif information level to display<p>"
                                        "<b>Simple</b>: display general information about the photograph "
                                        " (default).<p>"
                                        "<b>Full</b>: display all EXIF sections.") );  
    
    m_exifWidget = new KExifWidget(this);
    topLayout->addMultiCellWidget(m_exifWidget, 2, 2, 0, 2);

    m_levelCombo->insertItem(i18n("Simple"));
    m_levelCombo->insertItem(i18n("Full"));
    
    connect(m_levelCombo, SIGNAL(activated(int)),
            this, SLOT(slotLevelChanged(int)));
    
    connect(m_navigateBar, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(m_navigateBar, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(m_navigateBar, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(m_navigateBar, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));
           

    // -- read config ---------------------------------------------------------

    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    m_levelCombo->setCurrentItem(config->readNumEntry("EXIF Level", 0));
    m_currItem = config->readEntry("Current EXIF Item", QString());
    slotLevelChanged(0);
}

ImagePropertiesEXIFTab::~ImagePropertiesEXIFTab()
{
    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    config->writeEntry("EXIF Level", m_levelCombo->currentItem());
    config->writeEntry("Current EXIF Item", m_currItem);
}

void ImagePropertiesEXIFTab::setCurrentURL(const KURL& url, int itemType)
{
    if (url.isEmpty())
       {
       m_exifWidget->loadFile(url.path());
       m_navigateBar->setFileName("");
       setEnabled(false);
       return;
       }

    setEnabled(true);

    if (!m_exifWidget->getCurrentItemName().isNull())
        m_currItem = m_exifWidget->getCurrentItemName();
    
    m_exifWidget->loadFile(url.path());
    m_exifWidget->setCurrentItem(m_currItem);
    
    m_navigateBar->setFileName(url.filename());
    m_navigateBar->setButtonsState(itemType);
}

void ImagePropertiesEXIFTab::slotLevelChanged(int)
{
    if (m_levelCombo->currentText() == i18n("Simple"))
        m_exifWidget->setMode(KExifWidget::SIMPLE);
    else
        m_exifWidget->setMode(KExifWidget::FULL);
}

}  // NameSpace Digikam

#include "imagepropertiesexiftab.moc"
