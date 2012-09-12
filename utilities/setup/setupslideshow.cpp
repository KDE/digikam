/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : setup tab for slideshow options.
 *
 * Copyright (C) 2005-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupslideshow.moc"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <knuminput.h>

// Local includes

#include "slideshowsettings.h"

namespace Digikam
{

class SetupSlideShow::Private
{
public:

    Private() :
        startWithCurrent(0),
        loopMode(0),
        showName(0),
        showDate(0),
        showApertureFocal(0),
        showExpoSensitivity(0),
        showMakeModel(0),
        showLabels(0),
        showComment(0),
        showTitle(0),
        showCapIfNoTitle(0),
        delayInput(0)
    {}

    QCheckBox*    startWithCurrent;
    QCheckBox*    loopMode;
    QCheckBox*    showName;
    QCheckBox*    showDate;
    QCheckBox*    showApertureFocal;
    QCheckBox*    showExpoSensitivity;
    QCheckBox*    showMakeModel;
    QCheckBox*    showLabels;
    QCheckBox*    showComment;
    QCheckBox*    showTitle;
    QCheckBox*    showCapIfNoTitle;

    KIntNumInput* delayInput;
};

// --------------------------------------------------------

SetupSlideShow::SetupSlideShow(QWidget* const parent)
    : QScrollArea(parent), d(new Private)
{
    QWidget* panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* layout = new QVBoxLayout(panel);

    d->delayInput = new KIntNumInput(5, panel);
    d->delayInput->setRange(1, 3600, 1);
    d->delayInput->setSliderEnabled(true);
    d->delayInput->setLabel(i18n("&Delay between images:"), Qt::AlignLeft | Qt::AlignTop);
    d->delayInput->setWhatsThis(i18n("The delay, in seconds, between images."));

    d->startWithCurrent = new QCheckBox(i18n("Start with current image"), panel);
    d->startWithCurrent->setWhatsThis(i18n("If this option is enabled, the Slideshow will be started "
                                           "with the current image selected in the images list."));

    d->loopMode = new QCheckBox(i18n("Slideshow runs in a loop"), panel);
    d->loopMode->setWhatsThis(i18n("Run the slideshow in a loop."));

    d->showName = new QCheckBox(i18n("Show image file name"), panel);
    d->showName->setWhatsThis(i18n("Show the image file name at the bottom of the screen."));

    d->showDate = new QCheckBox(i18n("Show image creation date"), panel);
    d->showDate->setWhatsThis(i18n("Show the image creation time/date at the bottom of the screen."));

    d->showApertureFocal = new QCheckBox(i18n("Show camera aperture and focal length"), panel);
    d->showApertureFocal->setWhatsThis(i18n("Show the camera aperture and focal length at the bottom of the screen."));

    d->showExpoSensitivity = new QCheckBox(i18n("Show camera exposure and sensitivity"), panel);
    d->showExpoSensitivity->setWhatsThis(i18n("Show the camera exposure and sensitivity at the bottom of the screen."));

    d->showMakeModel = new QCheckBox(i18n("Show camera make and model"), panel);
    d->showMakeModel->setWhatsThis(i18n("Show the camera make and model at the bottom of the screen."));

    d->showComment = new QCheckBox(i18n("Show image caption"), panel);
    d->showComment->setWhatsThis(i18n("Show the image caption at the bottom of the screen."));

    d->showTitle = new QCheckBox(i18n("Show image title"), panel);
    d->showTitle->setWhatsThis(i18n("Show the image title at the bottom of the screen."));

    d->showCapIfNoTitle = new QCheckBox(i18n("Show image caption if it hasn't title"), panel);
    d->showCapIfNoTitle->setWhatsThis(i18n("Show the image caption at the bottom of the screen if no titles existed."));

    d->showLabels = new QCheckBox(i18n("Show image labels"), panel);
    d->showLabels->setWhatsThis(i18n("Show the digiKam image color label, pick label, and rating at the bottom of the screen."));

    // Disable and uncheck the "Show captions if no title" checkbox if the "Show comment" checkbox enabled
    connect(d->showComment, SIGNAL(stateChanged(int)), 
            this, SLOT(slotSetUnchecked(int)));

    connect(d->showComment, SIGNAL(toggled(bool)),
            d->showCapIfNoTitle, SLOT(setDisabled(bool)));

    // Only digiKam support this feature, showFoto do not support digiKam database information.
    if (kapp->applicationName() == "showfoto")
    {
        d->showTitle->hide();
        d->showCapIfNoTitle->hide();
        d->showLabels->hide();
    }

    layout->addWidget(d->delayInput);
    layout->addWidget(d->startWithCurrent);
    layout->addWidget(d->loopMode);
    layout->addWidget(d->showName);
    layout->addWidget(d->showDate);
    layout->addWidget(d->showApertureFocal);
    layout->addWidget(d->showExpoSensitivity);
    layout->addWidget(d->showMakeModel);
    layout->addWidget(d->showComment);
    layout->addWidget(d->showTitle);
    layout->addWidget(d->showCapIfNoTitle);
    layout->addWidget(d->showLabels);
    layout->addStretch();
    layout->setMargin(KDialog::spacingHint());
    layout->setSpacing(KDialog::spacingHint());

    readSettings();

    // --------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);
}

SetupSlideShow::~SetupSlideShow()
{
    delete d;
}

void SetupSlideShow::slotSetUnchecked(int)
{
    d->showCapIfNoTitle->setCheckState(Qt::Unchecked);
}

void SetupSlideShow::applySettings()
{
    SlideShowSettings settings;
    settings.delay                = d->delayInput->value();
    settings.startWithCurrent     = d->startWithCurrent->isChecked();
    settings.loop                 = d->loopMode->isChecked();
    settings.printName            = d->showName->isChecked();
    settings.printDate            = d->showDate->isChecked();
    settings.printApertureFocal   = d->showApertureFocal->isChecked();
    settings.printExpoSensitivity = d->showExpoSensitivity->isChecked();
    settings.printMakeModel       = d->showMakeModel->isChecked();
    settings.printComment         = d->showComment->isChecked();
    settings.printTitle           = d->showTitle->isChecked();
    settings.printCapIfNoTitle    = d->showCapIfNoTitle->isChecked();
    settings.printLabels          = d->showLabels->isChecked();
    settings.writeToConfig();
}

void SetupSlideShow::readSettings()
{
    SlideShowSettings settings;
    settings.readFromConfig();
    d->delayInput->setValue(settings.delay);
    d->startWithCurrent->setChecked(settings.startWithCurrent);
    d->loopMode->setChecked(settings.loop);
    d->showName->setChecked(settings.printName);
    d->showDate->setChecked(settings.printDate);
    d->showApertureFocal->setChecked(settings.printApertureFocal);
    d->showExpoSensitivity->setChecked(settings.printExpoSensitivity);
    d->showMakeModel->setChecked(settings.printMakeModel);
    d->showComment->setChecked(settings.printComment);
    d->showTitle->setChecked(settings.printTitle);
    d->showCapIfNoTitle->setChecked(settings.printCapIfNoTitle);
    d->showLabels->setChecked(settings.printLabels);
}

}   // namespace Digikam
