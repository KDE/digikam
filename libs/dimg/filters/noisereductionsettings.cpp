/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-22
 * Description : noise reduction settings view.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "noisereductionsettings.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>

// KDE includes

#include <kdialog.h>
#include <klocale.h>

using namespace KDcrawIface;

namespace Digikam
{

class NoiseReductionSettingsPriv
{
public:

    NoiseReductionSettingsPriv() :
        softnessInput(0),
        thresholdInput(0)
        {}

    RDoubleNumInput* softnessInput;
    RDoubleNumInput* thresholdInput;
};

NoiseReductionSettings::NoiseReductionSettings(QWidget* parent)
                      : QWidget(parent),
                        d(new NoiseReductionSettingsPriv)
{
    QGridLayout* grid = new QGridLayout(parent);

    // -------------------------------------------------------------

    QLabel *label1    = new QLabel(i18n("Threshold:"));
    d->thresholdInput = new RDoubleNumInput;
    d->thresholdInput->setDecimals(2);
    d->thresholdInput->input()->setRange(0.0, 10.0, 0.1, true);
    d->thresholdInput->setDefaultValue(1.2);
    d->thresholdInput->setWhatsThis(i18n("<b>Threshold</b>: Adjusts the threshold for denoising of "
                                         "the image in a range from 0.0 (none) to 10.0. "
                                         "The threshold is the value below which everything is considered noise."));

    // -------------------------------------------------------------

    QLabel *label2   = new QLabel(i18n("Softness:"));
    d->softnessInput = new RDoubleNumInput;
    d->softnessInput->setDecimals(1);
    d->softnessInput->input()->setRange(0.0, 1.0, 0.1, true);
    d->softnessInput->setDefaultValue(0.1);
    d->softnessInput->setWhatsThis(i18n("<b>Softness</b>: This adjusts the softness of the thresholding "
                                        "(soft as opposed to hard thresholding). The higher the softness "
                                        "the more noise remains in the image."));

    grid->addWidget(label1,            0, 0, 1, 1);
    grid->addWidget(d->thresholdInput, 0, 1, 1, 1);
    grid->addWidget(label2,            1, 0, 1, 1);
    grid->addWidget(d->softnessInput,  1, 1, 1, 1);
    grid->setRowStretch(2, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    connect(d->thresholdInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->softnessInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));
}

NoiseReductionSettings::~NoiseReductionSettings()
{
    delete d;
}

RDoubleNumInput* NoiseReductionSettings::thresholdInput() const
{
    return d->thresholdInput;
}

RDoubleNumInput* NoiseReductionSettings::softnessInput() const
{
    return d->softnessInput;
}

void NoiseReductionSettings::resetToDefault()
{
    d->thresholdInput->slotReset();
    d->softnessInput->slotReset();
}

}  // namespace Digikam
