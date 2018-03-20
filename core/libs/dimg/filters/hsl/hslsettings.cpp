/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-11
 * Description : HSL settings view.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Julien Narboux <julien at narboux dot fr>
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

#include "hslsettings.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "digikam_debug.h"
#include "colorgradientwidget.h"
#include "hspreviewwidget.h"
#include "dhuesaturationselect.h"

namespace Digikam
{

class HSLSettings::Private
{
public:

    Private() :
        HSSelector(0),
        hInput(0),
        sInput(0),
        vInput(0),
        lInput(0),
        HSPreview(0)
    {
    }

    static const QString    configHueAdjustmentEntry;
    static const QString    configSaturationAdjustmentEntry;
    static const QString    configVibranceAdjustmentEntry;
    static const QString    configLighnessAdjustmentEntry;

    DHueSaturationSelector* HSSelector;

    DDoubleNumInput*        hInput;
    DDoubleNumInput*        sInput;
    DDoubleNumInput*        vInput;
    DDoubleNumInput*        lInput;

    HSPreviewWidget*        HSPreview;
};

const QString HSLSettings::Private::configHueAdjustmentEntry(QLatin1String("HueAdjustment"));
const QString HSLSettings::Private::configSaturationAdjustmentEntry(QLatin1String("SaturationAdjustment"));
const QString HSLSettings::Private::configVibranceAdjustmentEntry(QLatin1String("VibranceAdjustment"));
const QString HSLSettings::Private::configLighnessAdjustmentEntry(QLatin1String("LighnessAdjustment"));

// --------------------------------------------------------

HSLSettings::HSLSettings(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const grid = new QGridLayout(parent);

    d->HSSelector = new DHueSaturationSelector();
    d->HSSelector->setWhatsThis(i18n("Select the hue and saturation adjustments of the image."));
    d->HSSelector->setMinimumSize(256, 142);

    d->HSPreview = new HSPreviewWidget();
    d->HSPreview->setWhatsThis(i18n("You can see here a color preview of the hue and "
                                    "saturation adjustments."));
    d->HSPreview->setMinimumSize(256, 15);

    QLabel* const label2 = new QLabel(i18n("Hue:"));
    d->hInput            = new DDoubleNumInput();
    d->hInput->setDecimals(0);
    d->hInput->setRange(-180.0, 180.0, 1.0);
    d->hInput->setDefaultValue(0.0);
    d->hInput->setWhatsThis(i18n("Set here the hue adjustment of the image."));

    QLabel* const label3 = new QLabel(i18n("Saturation:"));
    d->sInput            = new DDoubleNumInput();
    d->sInput->setDecimals(2);
    d->sInput->setRange(-100.0, 100.0, 0.01);
    d->sInput->setDefaultValue(0.0);
    d->sInput->setWhatsThis(i18n("Set here the saturation adjustment of the image."));

    QLabel* const label4 = new QLabel(i18n("Vibrance:"));
    d->vInput            = new DDoubleNumInput();
    d->vInput->setDecimals(2);
    d->vInput->setRange(-100.0, 100.0, 0.01);
    d->vInput->setDefaultValue(0.0);
    d->vInput->setWhatsThis(i18n("Set here the vibrance adjustment of the image."
                                 "Vibrance performs selective saturation on less saturated colors and avoiding skin tones."));

    QLabel* const label5 = new QLabel(i18n("Lightness:"));
    d->lInput            = new DDoubleNumInput();
    d->lInput->setDecimals(2);
    d->lInput->setRange(-100.0, 100.0, 0.01);
    d->lInput->setDefaultValue(0.0);
    d->lInput->setWhatsThis(i18n("Set here the lightness adjustment of the image."));

    // -------------------------------------------------------------

    grid->addWidget(d->HSSelector, 0, 0, 1, 5);
    grid->addWidget(d->HSPreview,  1, 0, 1, 5);
    grid->addWidget(label2,        2, 0, 1, 1);
    grid->addWidget(d->hInput,     2, 1, 1, 4);
    grid->addWidget(label3,        3, 0, 1, 1);
    grid->addWidget(d->sInput,     3, 1, 1, 4);
    grid->addWidget(label4,        4, 0, 1, 1);
    grid->addWidget(d->vInput,     4, 1, 1, 4);
    grid->addWidget(label5,        5, 0, 1, 1);
    grid->addWidget(d->lInput,     5, 1, 1, 4);
    grid->setRowStretch(6, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    connect(d->HSSelector, SIGNAL(valueChanged(int,int)),
            this, SLOT(slotHSChanged(int,int)));

    connect(d->hInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->hInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotHChanged(double)));

    connect(d->sInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->vInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->lInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));
}

HSLSettings::~HSLSettings()
{
    delete d;
}

void HSLSettings::slotHSChanged(int h, int s)
{
    double hue = (double)(h);

    if (h >= 180 && h <= 359)
    {
        hue = (double)(h) - 359.0;
    }

    double sat = ((double)s * (200.0 / 255.0)) - 100.0;

    d->hInput->blockSignals(true);
    d->sInput->blockSignals(true);
    d->hInput->setValue(hue);
    d->sInput->setValue(sat);
    d->HSPreview->setHS(hue, sat);
    d->hInput->blockSignals(false);
    d->sInput->blockSignals(false);

    emit signalSettingsChanged();
}

void HSLSettings::slotHChanged(double h)
{
    int hue = (int)(h);

    if (h >= -180 && h < 0)
    {
        hue = (int)(h) + 359;
    }

    d->HSSelector->blockSignals(true);
    d->HSSelector->setXValue(hue);
    d->HSSelector->blockSignals(false);

    d->HSPreview->setHS(settings().hue, settings().saturation);
}

void HSLSettings::slotSChanged(double s)
{
    int sat = (int)((s + 100.0) * (255.0 / 200.0));

    d->HSSelector->blockSignals(true);
    d->HSSelector->setYValue(sat);
    d->HSSelector->blockSignals(false);

    d->HSPreview->setHS(settings().hue, settings().saturation);
}

HSLContainer HSLSettings::settings() const
{
    HSLContainer prm;

    prm.hue        = d->hInput->value();
    prm.saturation = d->sInput->value();
    prm.vibrance   = d->vInput->value();
    prm.lightness  = d->lInput->value();

    return prm;
}

void HSLSettings::setSettings(const HSLContainer& settings)
{
    blockSignals(true);
    d->hInput->setValue(settings.hue);
    d->sInput->setValue(settings.saturation);
    d->vInput->setValue(settings.vibrance);
    d->lInput->setValue(settings.lightness);
    slotHChanged(settings.hue);
    slotSChanged(settings.saturation);
    blockSignals(false);
}

void HSLSettings::resetToDefault()
{
    blockSignals(true);
    d->hInput->slotReset();
    d->sInput->slotReset();
    d->vInput->slotReset();
    d->lInput->slotReset();
    blockSignals(false);
}

HSLContainer HSLSettings::defaultSettings() const
{
    HSLContainer prm;

    prm.hue        = d->hInput->defaultValue();
    prm.saturation = d->sInput->defaultValue();
    prm.vibrance   = d->vInput->defaultValue();
    prm.lightness  = d->lInput->defaultValue();

    return prm;
}

void HSLSettings::readSettings(KConfigGroup& group)
{
    HSLContainer prm;
    HSLContainer defaultPrm = defaultSettings();

    prm.hue        = group.readEntry(d->configHueAdjustmentEntry,        defaultPrm.hue);
    prm.saturation = group.readEntry(d->configSaturationAdjustmentEntry, defaultPrm.saturation);
    prm.vibrance   = group.readEntry(d->configVibranceAdjustmentEntry,   defaultPrm.vibrance);
    prm.lightness  = group.readEntry(d->configLighnessAdjustmentEntry,   defaultPrm.lightness);

    setSettings(prm);
}

void HSLSettings::writeSettings(KConfigGroup& group)
{
    HSLContainer prm = settings();

    group.writeEntry(d->configHueAdjustmentEntry,        prm.hue);
    group.writeEntry(d->configSaturationAdjustmentEntry, prm.saturation);
    group.writeEntry(d->configVibranceAdjustmentEntry,   prm.vibrance);
    group.writeEntry(d->configLighnessAdjustmentEntry,   prm.lightness);
}

}  // namespace Digikam
