/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : setup tab for slideshow options.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupslideshow.h"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QStyle>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dnuminput.h"
#include "digikam_debug.h"
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
        showTags(0),
        showCapIfNoTitle(0),
        showProgress(0),
        screenPlacement(0),
        delayInput(0)
    {
    }

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
    QCheckBox*    showTags;
    QCheckBox*    showCapIfNoTitle;
    QCheckBox*    showProgress;

    QComboBox*    screenPlacement;
    DIntNumInput* delayInput;
};

// --------------------------------------------------------

SetupSlideShow::SetupSlideShow(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    QWidget* const panel      = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    const int spacing         = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    QVBoxLayout* const layout = new QVBoxLayout(panel);

    DHBox* const hbox1     = new DHBox(panel);
    QLabel* const lbl1     = new QLabel(i18n("Delay between images:"), hbox1);
    lbl1->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    hbox1->setStretchFactor(lbl1, 5);
    QWidget* const space   = new QWidget(hbox1);
    hbox1->setStretchFactor(space, 5);
    d->delayInput          = new DIntNumInput(hbox1);
    d->delayInput->setDefaultValue(5);
    d->delayInput->setRange(1, 3600, 1);
    d->delayInput->setWhatsThis(i18n("The delay, in seconds, between images."));
    hbox1->setStretchFactor(d->delayInput, 10);

    d->startWithCurrent    = new QCheckBox(i18n("Start with current image"), panel);
    d->startWithCurrent->setWhatsThis(i18n("If this option is enabled, the Slideshow will be started "
                                           "with the current image selected in the images list."));

    d->loopMode            = new QCheckBox(i18n("Slideshow runs in a loop"), panel);
    d->loopMode->setWhatsThis(i18n("Run the slideshow in a loop."));

    d->showProgress        = new QCheckBox(i18n("Show progress indicator"), panel);
    d->showProgress->setWhatsThis(i18n("Show a progress indicator with pending items to show and time progression."));

    d->showName            = new QCheckBox(i18n("Show image file name"), panel);
    d->showName->setWhatsThis(i18n("Show the image file name at the bottom of the screen."));

    d->showDate            = new QCheckBox(i18n("Show image creation date"), panel);
    d->showDate->setWhatsThis(i18n("Show the image creation time/date at the bottom of the screen."));

    d->showApertureFocal   = new QCheckBox(i18n("Show camera aperture and focal length"), panel);
    d->showApertureFocal->setWhatsThis(i18n("Show the camera aperture and focal length at the bottom of the screen."));

    d->showExpoSensitivity = new QCheckBox(i18n("Show camera exposure and sensitivity"), panel);
    d->showExpoSensitivity->setWhatsThis(i18n("Show the camera exposure and sensitivity at the bottom of the screen."));

    d->showMakeModel       = new QCheckBox(i18n("Show camera make and model"), panel);
    d->showMakeModel->setWhatsThis(i18n("Show the camera make and model at the bottom of the screen."));

    d->showComment         = new QCheckBox(i18n("Show image caption"), panel);
    d->showComment->setWhatsThis(i18n("Show the image caption at the bottom of the screen."));

    d->showTitle           = new QCheckBox(i18n("Show image title"), panel);
    d->showTitle->setWhatsThis(i18n("Show the image title at the bottom of the screen."));

    d->showCapIfNoTitle    = new QCheckBox(i18n("Show image caption if it hasn't title"), panel);
    d->showCapIfNoTitle->setWhatsThis(i18n("Show the image caption at the bottom of the screen if no titles existed."));

    d->showTags            = new QCheckBox(i18n("Show image tags"), panel);
    d->showTags->setWhatsThis(i18n("Show the digiKam image tag names at the bottom of the screen."));

    d->showLabels          = new QCheckBox(i18n("Show image labels"), panel);
    d->showLabels->setWhatsThis(i18n("Show the digiKam image color label, pick label, and rating at the bottom of the screen."));

    DHBox* const screenSelectBox = new DHBox(panel);
    new QLabel(i18n("Screen placement:"), screenSelectBox);
    d->screenPlacement           = new QComboBox(screenSelectBox);
    d->screenPlacement->setToolTip(i18n("In case of multi-screen computer, select here the monitor to slide contents."));

    QStringList choices;
    choices.append(i18nc("@label:listbox The current screen, for the presentation mode", "Current Screen"));
    choices.append(i18nc("@label:listbox The default screen for the presentation mode",  "Default Screen"));

    for (int i = 0 ; i < qApp->desktop()->numScreens() ; i++ )
    {
        choices.append(i18nc("@label:listbox %1 is the screen number (0, 1, ...)", "Screen %1", i));
    }

    d->screenPlacement->addItems(choices);

    // Disable and uncheck the "Show captions if no title" checkbox if the "Show comment" checkbox enabled
    connect(d->showComment, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetUnchecked(int)));

    connect(d->showComment, SIGNAL(toggled(bool)),
            d->showCapIfNoTitle, SLOT(setDisabled(bool)));

    // Only digiKam support this feature, showFoto do not support digiKam database information.
    if (qApp->applicationName() == QLatin1String("showfoto"))
    {
        d->showTitle->hide();
        d->showCapIfNoTitle->hide();
        d->showLabels->hide();
        d->showTags->hide();
    }

    layout->addWidget(hbox1);
    layout->addWidget(d->startWithCurrent);
    layout->addWidget(d->loopMode);
    layout->addWidget(d->showProgress);
    layout->addWidget(d->showName);
    layout->addWidget(d->showDate);
    layout->addWidget(d->showApertureFocal);
    layout->addWidget(d->showExpoSensitivity);
    layout->addWidget(d->showMakeModel);
    layout->addWidget(d->showComment);
    layout->addWidget(d->showTitle);
    layout->addWidget(d->showCapIfNoTitle);
    layout->addWidget(d->showTags);
    layout->addWidget(d->showLabels);
    layout->addWidget(screenSelectBox);
    layout->addStretch();
    layout->setContentsMargins(spacing, spacing, spacing, spacing);
    layout->setSpacing(spacing);

    readSettings();
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
    settings.delay                 = d->delayInput->value();
    settings.startWithCurrent      = d->startWithCurrent->isChecked();
    settings.loop                  = d->loopMode->isChecked();
    settings.printName             = d->showName->isChecked();
    settings.printDate             = d->showDate->isChecked();
    settings.printApertureFocal    = d->showApertureFocal->isChecked();
    settings.printExpoSensitivity  = d->showExpoSensitivity->isChecked();
    settings.printMakeModel        = d->showMakeModel->isChecked();
    settings.printComment          = d->showComment->isChecked();
    settings.printTitle            = d->showTitle->isChecked();
    settings.printCapIfNoTitle     = d->showCapIfNoTitle->isChecked();
    settings.printTags             = d->showTags->isChecked();
    settings.printLabels           = d->showLabels->isChecked();
    settings.showProgressIndicator = d->showProgress->isChecked();
    settings.slideScreen           = d->screenPlacement->currentIndex() - 2;
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
    d->showTags->setChecked(settings.printTags);
    d->showLabels->setChecked(settings.printLabels);
    d->showProgress->setChecked(settings.showProgressIndicator);

    const int screen = settings.slideScreen;

    if (screen >= -2 && screen < qApp->desktop()->numScreens())
    {
        d->screenPlacement->setCurrentIndex(screen + 2);
    }
    else
    {
        d->screenPlacement->setCurrentIndex(0);
        settings.slideScreen = -2;
        settings.writeToConfig();
    }
}

}   // namespace Digikam
