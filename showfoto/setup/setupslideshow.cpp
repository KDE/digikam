/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-21
 * Description : setup tab for showfoto slideshow options.
 * 
 * Copyright 2005 by Gilles Caulier
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

// QT includes.

#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>

// Local includes.

#include "setupslideshow.h"

namespace ShowFoto
{

SetupSlideShow::SetupSlideShow(QWidget* parent )
              : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout( parent );

   m_delayInput = new KIntNumInput(5, parent);
   m_delayInput->setRange(1, 3600, 1, true );
   m_delayInput->setLabel( i18n("&Delay between images:"), AlignLeft|AlignVCenter );
   QWhatsThis::add( m_delayInput, i18n("<p>The delay in seconds between images."));
   
   m_startWithCurrent = new QCheckBox(i18n("Start with current image"), parent);
   QWhatsThis::add( m_startWithCurrent, i18n("<p>If this option is enabled, Slideshow will be started "
                                             "with current image selected from images list."));
   
   m_loopMode = new QCheckBox(i18n("Display in loop"), parent);
   QWhatsThis::add( m_loopMode, i18n("<p>Slideshow running in loop with all current images."));
   
   m_fullScreenMode = new QCheckBox(i18n("Fullscreen mode"), parent);
   QWhatsThis::add( m_fullScreenMode, i18n("<p>Toogle in fullScreen mode during Slideshow."));

   layout->addWidget( m_delayInput );
   layout->addWidget( m_startWithCurrent );
   layout->addWidget( m_loopMode );
   layout->addWidget( m_fullScreenMode );
   layout->addStretch();
   
   readSettings();
}

SetupSlideShow::~SetupSlideShow()
{
}

void SetupSlideShow::applySettings()
{
    KConfig* config = kapp->config();

    config->setGroup("ImageViewer Settings");
    config->writeEntry("SlideShowDelay", m_delayInput->value());
    config->writeEntry("SlideShowStartCurrent", m_startWithCurrent->isChecked());
    config->writeEntry("SlideShowLoop", m_loopMode->isChecked());
    config->writeEntry("SlideShowFullScreen", m_fullScreenMode->isChecked());
    config->sync();
}

void SetupSlideShow::readSettings()
{
    KConfig* config = kapp->config();
 
    config->setGroup("ImageViewer Settings");
    m_delayInput->setValue( config->readNumEntry("SlideShowDelay", 5) );
    m_startWithCurrent->setChecked(config->readBoolEntry("SlideShowStartCurrent", false));
    m_loopMode->setChecked(config->readBoolEntry("SlideShowLoop", false));
    m_fullScreenMode->setChecked(config->readBoolEntry("SlideShowFullScreen", true));
}

}   // namespace ShowFoto

#include "setupslideshow.moc"
