/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-05-21
 * Description : setup tab for showfoto slideshow options.
 * 
 * Copyright 2005-2006 by Gilles Caulier
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

namespace Digikam
{

class SetupSlideShowPriv
{
public:

    SetupSlideShowPriv()
    {
        delayInput       = 0;
        startWithCurrent = 0;
        loopMode         = 0;
        fullScreenMode   = 0;
    }

    QCheckBox    *startWithCurrent;
    QCheckBox    *loopMode;
    QCheckBox    *fullScreenMode;
    
    KIntNumInput *delayInput;
};    
    
SetupSlideShow::SetupSlideShow(QWidget* parent )
              : QWidget(parent)
{
    d = new SetupSlideShowPriv;
    QVBoxLayout *layout = new QVBoxLayout( parent );
    
    d->delayInput = new KIntNumInput(5, parent);
    d->delayInput->setRange(1, 3600, 1, true );
    d->delayInput->setLabel( i18n("&Delay between images:"), AlignLeft|AlignTop );
    QWhatsThis::add( d->delayInput, i18n("<p>The delay in seconds between images."));
    
    d->startWithCurrent = new QCheckBox(i18n("Start with current image"), parent);
    QWhatsThis::add( d->startWithCurrent, i18n("<p>If this option is enabled, Slideshow will be started "
                                                "with current image selected from images list."));
    
    d->loopMode = new QCheckBox(i18n("Display in loop"), parent);
    QWhatsThis::add( d->loopMode, i18n("<p>Slideshow running in loop with all current images."));
    
    d->fullScreenMode = new QCheckBox(i18n("Fullscreen mode"), parent);
    QWhatsThis::add( d->fullScreenMode, i18n("<p>Toggle in fullScreen mode during Slideshow."));
    
    layout->addWidget( d->delayInput );
    layout->addWidget( d->startWithCurrent );
    layout->addWidget( d->loopMode );
    layout->addWidget( d->fullScreenMode );
    layout->addStretch();
    
    readSettings();
}

SetupSlideShow::~SetupSlideShow()
{
    delete d;
}

void SetupSlideShow::applySettings()
{
    KConfig* config = kapp->config();

    config->setGroup("ImageViewer Settings");
    config->writeEntry("SlideShowDelay", d->delayInput->value());
    config->writeEntry("SlideShowStartCurrent", d->startWithCurrent->isChecked());
    config->writeEntry("SlideShowLoop", d->loopMode->isChecked());
    config->writeEntry("SlideShowFullScreen", d->fullScreenMode->isChecked());
    config->sync();
}

void SetupSlideShow::readSettings()
{
    KConfig* config = kapp->config();
 
    config->setGroup("ImageViewer Settings");
    d->delayInput->setValue( config->readNumEntry("SlideShowDelay", 5) );
    d->startWithCurrent->setChecked(config->readBoolEntry("SlideShowStartCurrent", false));
    d->loopMode->setChecked(config->readBoolEntry("SlideShowLoop", false));
    d->fullScreenMode->setChecked(config->readBoolEntry("SlideShowFullScreen", true));
}

}   // namespace Digikam

#include "setupslideshow.moc"
