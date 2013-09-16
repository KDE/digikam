/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-09-03
 * Description : HSL settings view.
 *
 * Copyright (C) 2013 by Sayantan Datta <sayantan dot knz at gmail dot com>
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "localadjustmentsettings.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>

// KDE includes

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <khuesaturationselect.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "colorgradientwidget.h"
#include "hspreviewwidget.h"
#include "localadjustmentfilter.h"

using namespace KDcrawIface;

namespace Digikam
{

class LASettings::Private
{
public:

    Private() :
        HSSelector(0),
        xInput(0),
        yInput(0),
        radius(0),
        hInput(0),
        sInput(0),
        vInput(0),
        lInput(0),
        rInput(0),
        gInput(0),
        bInput(0),
        aInput(0),
        HSPreview(0)
    {
    }

    static const QString    configXAdjustmentEntry;
    static const QString    configYAdjustmentEntry;
    static const QString    radiusAdjustmentEntry;
    static const QString    configHueAdjustmentEntry;
    static const QString    configSaturationAdjustmentEntry;
    static const QString    configVibranceAdjustmentEntry;
    static const QString    configLighnessAdjustmentEntry;
    static const QString    configRedAdjustmentEntry;
    static const QString    configGreenAdjustmentEntry;
    static const QString    configBlueAdjustmentEntry;
    static const QString    configAlphaAdjustmentEntry;
    static const QString    configRadiusAdjustmentEntry;

    KHueSaturationSelector* HSSelector;

    RIntNumInput*           xInput;
    RIntNumInput*           yInput;
    RIntNumInput*           radius;
    RDoubleNumInput*        hInput;
    RDoubleNumInput*        sInput;
    RDoubleNumInput*        vInput;
    RDoubleNumInput*        lInput;
    RDoubleNumInput*        rInput;
    RDoubleNumInput*        gInput;
    RDoubleNumInput*        bInput;
    RDoubleNumInput*        aInput;

    HSPreviewWidget*        HSPreview;
};

const QString LASettings::Private::configXAdjustmentEntry("LocalAdjustmentPointX");
const QString LASettings::Private::configYAdjustmentEntry("LocalAdjustmentPointY");
const QString LASettings::Private::configRadiusAdjustmentEntry("Radius");
const QString LASettings::Private::configHueAdjustmentEntry("HueAdjustment");
const QString LASettings::Private::configSaturationAdjustmentEntry("SaturationAdjustment");
const QString LASettings::Private::configVibranceAdjustmentEntry("VibranceAdjustment");
const QString LASettings::Private::configLighnessAdjustmentEntry("LighnessAdjustment");
const QString LASettings::Private::configRedAdjustmentEntry("RedAdjustment");
const QString LASettings::Private::configBlueAdjustmentEntry("BlueAdjustment");
const QString LASettings::Private::configGreenAdjustmentEntry("GreenAdjustment");
const QString LASettings::Private::configAlphaAdjustmentEntry("AlphaAdjustment");

// --------------------------------------------------------

LASettings::LASettings(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* const grid = new QGridLayout(parent);

    d->HSSelector = new KHueSaturationSelector();
    d->HSSelector->setWhatsThis(i18n("Select the hue, saturation and color local adjustments of the image."));
    d->HSSelector->setMinimumSize(256, 142);

    d->HSPreview = new HSPreviewWidget();
    d->HSPreview->setWhatsThis(i18n("You can see here a color preview of the hue and "
                                    "saturation adjustments."));
    d->HSPreview->setMinimumSize(256, 15);

    //the X axis and Y axis values and Radius will be depreciated, once the new canvas is complete,
    //and the drawCircle tool is implemented.

    QLabel* const label2 = new QLabel(i18n("X Point"));
    d->xInput            = new RIntNumInput();
    d->xInput->input()->setRange(0,8000,1.0);
    d->xInput->setSliderEnabled(true);
    d->xInput->setDefaultValue(0);
    d->xInput->setWhatsThis(i18n("Set the X axis value of the local adjustment point of the image."));

    QLabel* const label3 = new QLabel(i18n("Radius"));
    d->radius            = new RIntNumInput();
    d->radius->input()->setRange(0,1000,1.0);
    d->radius->setSliderEnabled(true);
    d->radius->setDefaultValue(0);
    d->radius->setWhatsThis(i18n("Set the radius of the local adjustment point of the image."));

    QLabel* const label4 = new QLabel(i18n("Y Point"));
    d->yInput            = new RIntNumInput();
    d->yInput->input()->setRange(0,8000,1.0);
    d->yInput->setSliderEnabled(true);
    d->yInput->setDefaultValue(0);
    d->yInput->setWhatsThis(i18n("Set the Y axis value of the local adjustment point of the image."));

    QLabel* const label5 = new QLabel(i18n("Hue:"));
    d->hInput            = new RDoubleNumInput();
    d->hInput->setDecimals(0);
    d->hInput->input()->setRange(-180.0, 180.0, 1.0, true);
    d->hInput->setDefaultValue(0.0);
    d->hInput->setWhatsThis(i18n("Set here the hue adjustment of the image."));

    QLabel* const label6 = new QLabel(i18n("Saturation:"));
    d->sInput            = new RDoubleNumInput();
    d->sInput->setDecimals(2);
    d->sInput->input()->setRange(-100.0, 100.0, 0.01, true);
    d->sInput->setDefaultValue(0.0);
    d->sInput->setWhatsThis(i18n("Set here the saturation adjustment of the image."));

    QLabel* const label7 = new QLabel(i18n("Vibrance:"));
    d->vInput            = new RDoubleNumInput();
    d->vInput->setDecimals(2);
    d->vInput->input()->setRange(-100.0, 100.0, 0.01, true);
    d->vInput->setDefaultValue(0.0);
    d->vInput->setWhatsThis(i18n("Set here the vibrance adjustment of the image."
                                 "Vibrance performs selective saturation on less saturated colors and avoiding skin tones."));

    QLabel* const label8 = new QLabel(i18n("Lightness:"));
    d->lInput            = new RDoubleNumInput();
    d->lInput->setDecimals(2);
    d->lInput->input()->setRange(-100.0, 100.0, 0.01, true);
    d->lInput->setDefaultValue(0.0);
    d->lInput->setWhatsThis(i18n("Set here the lightness adjustment of the image."));

    QLabel* const label9 = new QLabel(i18n("Red:"));
    d->rInput            = new RDoubleNumInput();
    d->rInput->setDecimals(2);
    d->rInput->input()->setRange(-100.0, 100.0, 0.01, true);
    d->rInput->setDefaultValue(0.0);
    d->rInput->setWhatsThis(i18n("Set here the red adjustment of the selection."));

    QLabel* const label10 = new QLabel(i18n("Blue:"));
    d->bInput            = new RDoubleNumInput();
    d->bInput->setDecimals(2);
    d->bInput->input()->setRange(-100.0, 100.0, 0.01, true);
    d->bInput->setDefaultValue(0.0);
    d->bInput->setWhatsThis(i18n("Set here the lightness adjustment of the selection."));

    QLabel* const label11 = new QLabel(i18n("Green:"));
    d->gInput            = new RDoubleNumInput();
    d->gInput->setDecimals(2);
    d->gInput->input()->setRange(-100.0, 100.0, 0.01, true);
    d->gInput->setDefaultValue(0.0);
    d->gInput->setWhatsThis(i18n("Set here the lightness adjustment of the selection."));

    QLabel* const label12 = new QLabel(i18n("Green:"));
    d->gInput            = new RDoubleNumInput();
    d->gInput->setDecimals(2);
    d->gInput->input()->setRange(-100.0, 0.0, 0.01, true);
    d->gInput->setDefaultValue(0.0);
    d->gInput->setWhatsThis(i18n("Set here the alpha adjustment of the selection."));

    // -------------------------------------------------------------

    grid->addWidget(d->HSSelector, 0,  0, 1, 5);
    grid->addWidget(d->HSPreview,  1,  0, 1, 5);
    grid->addWidget(label2,        2,  0, 1, 1);
    grid->addWidget(d->xInput,     2,  1, 1, 4);
    grid->addWidget(label3,        3,  0, 1, 1);
    grid->addWidget(d->yInput,     3,  1, 1, 4);
    grid->addWidget(label4,        4,  0, 1, 1);
    grid->addWidget(d->radius,     4,  1, 1, 4);
    grid->addWidget(label5,        5,  0, 1, 1);
    grid->addWidget(d->hInput,     5,  1, 1, 4);
    grid->addWidget(label6,        6,  0, 1, 1);
    grid->addWidget(d->sInput,     6,  1, 1, 4);
    grid->addWidget(label7,        7,  0, 1, 1);
    grid->addWidget(d->vInput,     7,  1, 1, 4);
    grid->addWidget(label8,        8,  0, 1, 1);
    grid->addWidget(d->lInput,     8,  1, 1, 4);
    grid->addWidget(label9,        9,  0, 1, 1);
    grid->addWidget(d->rInput,     9,  1, 1, 4);
    grid->addWidget(label10,       10, 0, 1, 1);
    grid->addWidget(d->bInput,     10, 1, 1, 4);
    grid->addWidget(label11,       11, 0, 1, 1);
    grid->addWidget(d->gInput,     11, 1, 1, 4);
    grid->addWidget(label12,       11, 0, 1, 1);
    grid->addWidget(d->aInput,     11, 1, 1, 4);
    grid->setRowStretch(6, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

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

    connect(d->xInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->yInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->radius, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->rInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->gInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->bInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->aInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));
}

LASettings::~LASettings()
{
    delete d;
}

void LASettings::slotHSChanged(int h, int s)
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

void LASettings::slotHChanged(double h)
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

void LASettings::slotSChanged(double s)
{
    int sat = (int)((s + 100.0) * (255.0 / 200.0));

    d->HSSelector->blockSignals(true);
    d->HSSelector->setYValue(sat);
    d->HSSelector->blockSignals(false);

    d->HSPreview->setHS(settings().hue, settings().saturation);
}

LAContainer LASettings::settings() const
{
    LAContainer prm;

    prm.center.setX( d->xInput->value());
    prm.center.setX( d->yInput->value());
    prm.red        = d->rInput->value();
    prm.green      = d->gInput->value();
    prm.blue       = d->bInput->value();
    prm.alpha      = d->aInput->value();
    prm.radius     = d->radius->value();
    prm.hue        = d->hInput->value();
    prm.saturation = d->sInput->value();
    prm.vibrance   = d->vInput->value();
    prm.lightness  = d->lInput->value();

    return prm;
}

void LASettings::setSettings(const LAContainer& settings)
{
    blockSignals(true);
    d->xInput->setValue(settings.center.x());
    d->yInput->setValue(settings.center.y());
    d->rInput->setValue(settings.red);
    d->gInput->setValue(settings.green);
    d->bInput->setValue(settings.blue);
    d->aInput->setValue(settings.alpha);
    d->radius->setValue(settings.radius);
    d->hInput->setValue(settings.hue);
    d->sInput->setValue(settings.saturation);
    d->vInput->setValue(settings.vibrance);
    d->lInput->setValue(settings.lightness);
    slotHChanged(settings.hue);
    slotSChanged(settings.saturation);
    blockSignals(false);
}

void LASettings::resetToDefault()
{
    blockSignals(true);
    d->xInput->slotReset();
    d->xInput->slotReset();
    d->rInput->slotReset();
    d->gInput->slotReset();
    d->bInput->slotReset();
    d->aInput->slotReset();
    d->radius->slotReset();
    d->hInput->slotReset();
    d->sInput->slotReset();
    d->vInput->slotReset();
    d->lInput->slotReset();
    blockSignals(false);
}

LAContainer LASettings::defaultSettings() const
{
    LAContainer prm;

    prm.center.setX( d->xInput->defaultValue());
    prm.center.setX( d->yInput->defaultValue());
    prm.red        = d->rInput->defaultValue();
    prm.green      = d->gInput->defaultValue();
    prm.blue       = d->bInput->defaultValue();
    prm.alpha      = d->aInput->defaultValue();
    prm.radius     = d->radius->defaultValue();
    prm.hue        = d->hInput->defaultValue();
    prm.saturation = d->sInput->defaultValue();
    prm.vibrance   = d->vInput->defaultValue();
    prm.lightness  = d->lInput->defaultValue();

    return prm;
}

void LASettings::readSettings(KConfigGroup& group)
{
    LAContainer prm;
    LAContainer defaultPrm = defaultSettings();

    prm.center.setX(group.readEntry(d->configXAdjustmentEntry,      defaultPrm.center.x()));
    prm.center.setY(group.readEntry(d->configYAdjustmentEntry,      defaultPrm.center.y()));
    prm.red        = group.readEntry(d->configHueAdjustmentEntry,        defaultPrm.hue);
    prm.green      = group.readEntry(d->configHueAdjustmentEntry,        defaultPrm.hue);
    prm.blue       = group.readEntry(d->configHueAdjustmentEntry,        defaultPrm.hue);
    prm.alpha      = group.readEntry(d->configHueAdjustmentEntry,        defaultPrm.hue);
    prm.radius     = group.readEntry(d->configHueAdjustmentEntry,        defaultPrm.hue);
    prm.hue        = group.readEntry(d->configHueAdjustmentEntry,        defaultPrm.hue);
    prm.saturation = group.readEntry(d->configSaturationAdjustmentEntry, defaultPrm.saturation);
    prm.vibrance   = group.readEntry(d->configVibranceAdjustmentEntry,   defaultPrm.vibrance);
    prm.lightness  = group.readEntry(d->configLighnessAdjustmentEntry,   defaultPrm.lightness);

    setSettings(prm);
}

void LASettings::writeSettings(KConfigGroup& group)
{
    LAContainer prm = settings();

    group.writeEntry(d->configXAdjustmentEntry,     prm.center.x());
    group.writeEntry(d->configYAdjustmentEntry,     prm.center.y());
    group.writeEntry(d->configRadiusAdjustmentEntry,     prm.radius);
    group.writeEntry(d->configRedAdjustmentEntry,        prm.red);
    group.writeEntry(d->configBlueAdjustmentEntry,       prm.blue);
    group.writeEntry(d->configGreenAdjustmentEntry,      prm.green);
    group.writeEntry(d->configAlphaAdjustmentEntry,      prm.alpha);
    group.writeEntry(d->configRadiusAdjustmentEntry,     prm.radius);
    group.writeEntry(d->configHueAdjustmentEntry,        prm.hue);
    group.writeEntry(d->configSaturationAdjustmentEntry, prm.saturation);
    group.writeEntry(d->configVibranceAdjustmentEntry,   prm.vibrance);
    group.writeEntry(d->configLighnessAdjustmentEntry,   prm.lightness);
}

}  // namespace Digikam
