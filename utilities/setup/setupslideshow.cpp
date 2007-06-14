/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : setup tab for slideshow options.
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <q3whatsthis.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>

// Local includes.

#include "setupslideshow.h"
#include "setupslideshow.moc"

namespace Digikam
{

class SetupSlideShowPriv
{
public:

    SetupSlideShowPriv()
    {
        delayInput           = 0;
        startWithCurrent     = 0;
        loopMode             = 0;
        printName            = 0;
        printDate            = 0;
        printApertureFocal   = 0;
        printExpoSensitivity = 0;
        printMakeModel       = 0;
        printComment         = 0;
    }

    QCheckBox    *startWithCurrent;
    QCheckBox    *loopMode;
    QCheckBox    *printName;
    QCheckBox    *printDate;
    QCheckBox    *printApertureFocal;
    QCheckBox    *printExpoSensitivity;
    QCheckBox    *printMakeModel;
    QCheckBox    *printComment;
    
    KIntNumInput *delayInput;
};    
    
SetupSlideShow::SetupSlideShow(QWidget* parent )
              : QWidget(parent)
{
    d = new SetupSlideShowPriv;
    Q3VBoxLayout *layout = new Q3VBoxLayout( parent );
    
    d->delayInput = new KIntNumInput(5, parent);
    d->delayInput->setRange(1, 3600, 1, true );
    d->delayInput->setLabel( i18n("&Delay between images:"), Qt::AlignLeft|Qt::AlignTop );
    Q3WhatsThis::add( d->delayInput, i18n("<p>The delay, in seconds, between images."));
    
    d->startWithCurrent = new QCheckBox(i18n("Start with current image"), parent);
    Q3WhatsThis::add( d->startWithCurrent, i18n("<p>If this option is enabled, Slideshow will be started "
                                               "with current image selected from the images list."));
    
    d->loopMode = new QCheckBox(i18n("Display in loop"), parent);
    Q3WhatsThis::add( d->loopMode, i18n("<p>Run the slideshow in a loop."));
    
    d->printName = new QCheckBox(i18n("Print image file name"), parent);
    Q3WhatsThis::add( d->printName, i18n("<p>Print image file name to the screen bottom."));

    d->printDate = new QCheckBox(i18n("Print image creation date"), parent);
    Q3WhatsThis::add( d->printDate, i18n("<p>Print image creation to the screen bottom."));

    d->printApertureFocal = new QCheckBox(i18n("Print camera aperture and focal length"), parent);
    Q3WhatsThis::add( d->printApertureFocal, i18n("<p>Print camera aperture and focal length to the screen bottom."));

    d->printExpoSensitivity = new QCheckBox(i18n("Print camera exposure and sensitivity"), parent);
    Q3WhatsThis::add( d->printExpoSensitivity, i18n("<p>Print camera exposure and sensitivity to the screen  bottom."));

    d->printMakeModel = new QCheckBox(i18n("Print camera make and model"), parent);
    Q3WhatsThis::add( d->printMakeModel, i18n("<p>Print camera make and model to the screen bottom."));

    d->printComment = new QCheckBox(i18n("Print image comment"), parent);
    Q3WhatsThis::add( d->printComment, i18n("<p>Print image comment on bottom of screen."));
    
    layout->addWidget(d->delayInput);
    layout->addWidget(d->startWithCurrent);
    layout->addWidget(d->loopMode);
    layout->addWidget(d->printName);
    layout->addWidget(d->printDate);
    layout->addWidget(d->printApertureFocal);
    layout->addWidget(d->printExpoSensitivity);
    layout->addWidget(d->printMakeModel);
    layout->addWidget(d->printComment);
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
    config->writeEntry("SlideShowPrintName", d->printName->isChecked());
    config->writeEntry("SlideShowPrintDate", d->printDate->isChecked());
    config->writeEntry("SlideShowPrintApertureFocal", d->printApertureFocal->isChecked());
    config->writeEntry("SlideShowPrintExpoSensitivity", d->printExpoSensitivity->isChecked());
    config->writeEntry("SlideShowPrintMakeModel", d->printMakeModel->isChecked());
    config->writeEntry("SlideShowPrintComment", d->printComment->isChecked());
    config->sync();
}

void SetupSlideShow::readSettings()
{
    KConfig* config = kapp->config();
 
    config->setGroup("ImageViewer Settings");
    d->delayInput->setValue(config->readNumEntry("SlideShowDelay", 5));
    d->startWithCurrent->setChecked(config->readBoolEntry("SlideShowStartCurrent", false));
    d->loopMode->setChecked(config->readBoolEntry("SlideShowLoop", false));
    d->printName->setChecked(config->readBoolEntry("SlideShowPrintName", true));
    d->printDate->setChecked(config->readBoolEntry("SlideShowPrintDate", false));
    d->printApertureFocal->setChecked(config->readBoolEntry("SlideShowPrintApertureFocal", false));
    d->printExpoSensitivity->setChecked(config->readBoolEntry("SlideShowPrintExpoSensitivity", false));
    d->printMakeModel->setChecked(config->readBoolEntry("SlideShowPrintMakeModel", false));
    d->printComment->setChecked(config->readBoolEntry("SlideShowPrintComment", false));
}

}   // namespace Digikam

