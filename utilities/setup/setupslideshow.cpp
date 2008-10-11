/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : setup tab for slideshow options.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>

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
        printRating          = 0;
    }

    QCheckBox    *startWithCurrent;
    QCheckBox    *loopMode;
    QCheckBox    *printName;
    QCheckBox    *printDate;
    QCheckBox    *printApertureFocal;
    QCheckBox    *printExpoSensitivity;
    QCheckBox    *printMakeModel;
    QCheckBox    *printComment;
    QCheckBox    *printRating;

    KIntNumInput *delayInput;
};

SetupSlideShow::SetupSlideShow(QWidget* parent)
              : QWidget(parent)
{
    d = new SetupSlideShowPriv;
    QVBoxLayout *layout = new QVBoxLayout(this);

    d->delayInput = new KIntNumInput(5, this);
    d->delayInput->setRange(1, 3600, 1);
    d->delayInput->setSliderEnabled(true);
    d->delayInput->setLabel( i18n("&Delay between images:"), Qt::AlignLeft|Qt::AlignTop );
    d->delayInput->setWhatsThis(i18n("The delay, in seconds, between images."));

    d->startWithCurrent = new QCheckBox(i18n("Start with current image"), this);
    d->startWithCurrent->setWhatsThis( i18n("If this option is enabled, the Slideshow will be started "
                                            "with the current image selected in the images list."));

    d->loopMode = new QCheckBox(i18n("Display in loop"), this);
    d->loopMode->setWhatsThis( i18n("Run the slideshow in a loop."));

    d->printName = new QCheckBox(i18n("Print image file name"), this);
    d->printName->setWhatsThis( i18n("Print the image file name at the bottom of the screen."));

    d->printDate = new QCheckBox(i18n("Print image creation date"), this);
    d->printDate->setWhatsThis( i18n("Print the image creation time/date at the bottom of the screen."));

    d->printApertureFocal = new QCheckBox(i18n("Print camera aperture and focal length"), this);
    d->printApertureFocal->setWhatsThis( i18n("Print the camera aperture and focal length at the bottom of the screen."));

    d->printExpoSensitivity = new QCheckBox(i18n("Print camera exposure and sensitivity"), this);
    d->printExpoSensitivity->setWhatsThis( i18n("Print the camera exposure and sensitivity at the bottom of the screen."));

    d->printMakeModel = new QCheckBox(i18n("Print camera make and model"), this);
    d->printMakeModel->setWhatsThis( i18n("Print the camera make and model at the bottom of the screen."));

    d->printComment = new QCheckBox(i18n("Print image caption"), this);
    d->printComment->setWhatsThis( i18n("Print the image caption at the bottom of the screen."));

    d->printRating = new QCheckBox(i18n("Print image rating"), this);
    d->printRating->setWhatsThis( i18n("Print the digiKam image rating at the bottom of the screen."));

    // Only digiKam support this feature, showFoto do not support digiKam database information.
    if (kapp->applicationName() == "showfoto")
        d->printRating->hide();

    layout->addWidget(d->delayInput);
    layout->addWidget(d->startWithCurrent);
    layout->addWidget(d->loopMode);
    layout->addWidget(d->printName);
    layout->addWidget(d->printDate);
    layout->addWidget(d->printApertureFocal);
    layout->addWidget(d->printExpoSensitivity);
    layout->addWidget(d->printMakeModel);
    layout->addWidget(d->printComment);
    layout->addWidget(d->printRating);
    layout->addStretch();
    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());

    readSettings();
}

SetupSlideShow::~SetupSlideShow()
{
    delete d;
}

void SetupSlideShow::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("ImageViewer Settings"));

    group.writeEntry("SlideShowDelay", d->delayInput->value());
    group.writeEntry("SlideShowStartCurrent", d->startWithCurrent->isChecked());
    group.writeEntry("SlideShowLoop", d->loopMode->isChecked());
    group.writeEntry("SlideShowPrintName", d->printName->isChecked());
    group.writeEntry("SlideShowPrintDate", d->printDate->isChecked());
    group.writeEntry("SlideShowPrintApertureFocal", d->printApertureFocal->isChecked());
    group.writeEntry("SlideShowPrintExpoSensitivity", d->printExpoSensitivity->isChecked());
    group.writeEntry("SlideShowPrintMakeModel", d->printMakeModel->isChecked());
    group.writeEntry("SlideShowPrintComment", d->printComment->isChecked());
    group.writeEntry("SlideShowPrintRating", d->printRating->isChecked());
    config->sync();
}

void SetupSlideShow::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("ImageViewer Settings"));

    d->delayInput->setValue(group.readEntry("SlideShowDelay", 5));
    d->startWithCurrent->setChecked(group.readEntry("SlideShowStartCurrent", false));
    d->loopMode->setChecked(group.readEntry("SlideShowLoop", false));
    d->printName->setChecked(group.readEntry("SlideShowPrintName", true));
    d->printDate->setChecked(group.readEntry("SlideShowPrintDate", false));
    d->printApertureFocal->setChecked(group.readEntry("SlideShowPrintApertureFocal", false));
    d->printExpoSensitivity->setChecked(group.readEntry("SlideShowPrintExpoSensitivity", false));
    d->printMakeModel->setChecked(group.readEntry("SlideShowPrintMakeModel", false));
    d->printComment->setChecked(group.readEntry("SlideShowPrintComment", false));
    d->printRating->setChecked(group.readEntry("SlideShowPrintRating", false));
}

}   // namespace Digikam
