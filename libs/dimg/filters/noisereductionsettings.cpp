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
#include <QCheckBox>

// KDE includes

#include <kdialog.h>
#include <klocale.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "rexpanderbox.h"

using namespace KDcrawIface;

namespace Digikam
{

class NoiseReductionSettingsPriv
{
public:

    NoiseReductionSettingsPriv() :
        configThresholdAdjustmentEntry("ThresholdAdjustment"),
        configSoftnessAdjustmentEntry("SoftnessAdjustment"),
        configAdvancedAdjustmentEntry("AdvancedAdjustment"),
        configThrLumInputAdjustmentEntry("ThrLumAdjustment"),
        configSoftLumInputAdjustmentEntry("SoftLumAdjustment"),
        configThrCrInputAdjustmentEntry("ThrCrAdjustment"),
        configSoftCrInputAdjustmentEntry("SoftCrAdjustment"),
        configThrCbInputAdjustmentEntry("ThrCbAdjustment"),
        configSoftCbInputAdjustmentEntry("SoftCbAdjustment"),
        advancedBox(0),
        generalBox(0),
        luminanceBox(0),
        chrominanceRedBox(0),
        chrominanceBlueBox(0),
        advExpanderBox(0),
        thresholdInput(0),
        softnessInput(0),
        thrLumInput(0),
        softLumInput(0),
        thrCrInput(0),
        softCrInput(0),
        thrCbInput(0),
        softCbInput(0)
        {}

    const QString    configThresholdAdjustmentEntry;
    const QString    configSoftnessAdjustmentEntry;
    const QString    configAdvancedAdjustmentEntry;
    const QString    configThrLumInputAdjustmentEntry;
    const QString    configSoftLumInputAdjustmentEntry;
    const QString    configThrCrInputAdjustmentEntry;
    const QString    configSoftCrInputAdjustmentEntry;
    const QString    configThrCbInputAdjustmentEntry;
    const QString    configSoftCbInputAdjustmentEntry;

    QCheckBox*       advancedBox;

    QWidget*         generalBox;
    QWidget*         luminanceBox;
    QWidget*         chrominanceRedBox;
    QWidget*         chrominanceBlueBox;
    
    RExpanderBox*    advExpanderBox;

    RDoubleNumInput* thresholdInput;
    RDoubleNumInput* softnessInput;
    RDoubleNumInput* thrLumInput;
    RDoubleNumInput* softLumInput;
    RDoubleNumInput* thrCrInput;
    RDoubleNumInput* softCrInput;
    RDoubleNumInput* thrCbInput;
    RDoubleNumInput* softCbInput;
};

NoiseReductionSettings::NoiseReductionSettings(QWidget* parent)
                      : QWidget(parent),
                        d(new NoiseReductionSettingsPriv)
{
    QGridLayout* grid = new QGridLayout(parent);

    // -------------------------------------------------------------

    d->generalBox       = new QWidget;
    QGridLayout* genLay = new QGridLayout(d->generalBox);
        
    QLabel *label1      = new QLabel(i18n("Threshold:"), d->generalBox);
    d->thresholdInput   = new RDoubleNumInput(d->generalBox);
    d->thresholdInput->setDecimals(2);
    d->thresholdInput->input()->setRange(0.0, 10.0, 0.1, true);
    d->thresholdInput->setDefaultValue(1.2);
    d->thresholdInput->setWhatsThis(i18n("<b>Threshold</b>: Adjusts the threshold for denoising of "
                                         "the image in a range from 0.0 (none) to 10.0. "
                                         "The threshold is the value below which everything is considered noise."));

    QLabel *label2      = new QLabel(i18n("Softness:"), d->generalBox);
    d->softnessInput    = new RDoubleNumInput(d->generalBox);
    d->softnessInput->setDecimals(1);
    d->softnessInput->input()->setRange(0.0, 1.0, 0.1, true);
    d->softnessInput->setDefaultValue(0.1);
    d->softnessInput->setWhatsThis(i18n("<b>Softness</b>: This adjusts the softness of the thresholding "
                                        "(soft as opposed to hard thresholding). The higher the softness "
                                        "the more noise remains in the image."));

    genLay->addWidget(label1,            0, 0, 1, 1);
    genLay->addWidget(d->thresholdInput, 0, 1, 1, 1);
    genLay->addWidget(label2,            1, 0, 1, 1);
    genLay->addWidget(d->softnessInput,  1, 1, 1, 1);
    genLay->setRowStretch(2, 10);
    genLay->setSpacing(0);
    genLay->setMargin(KDialog::spacingHint());

    // -------------------------------------------------------------
                                        
    d->advancedBox      = new QCheckBox(i18n("Advanced adjustements"));                                        

    d->advExpanderBox   = new RExpanderBox;
    d->advExpanderBox->setObjectName("Advanced Settings Expander");

    // -------------------------------------------------------------

    d->luminanceBox     = new QWidget(d->advExpanderBox);
    QGridLayout* lumLay = new QGridLayout(d->luminanceBox);
    
    QLabel *label3      = new QLabel(i18n("Threshold:"), d->luminanceBox);
    d->thrLumInput      = new RDoubleNumInput(d->luminanceBox);
    d->thrLumInput->setDecimals(2);
    d->thrLumInput->input()->setRange(0.0, 10.0, 0.1, true);
    d->thrLumInput->setDefaultValue(1.2);
/*    d->thrLumInput->setWhatsThis(i18n("<b>Threshold</b>: Adjusts the threshold for denoising of "
                                         "the image in a range from 0.0 (none) to 10.0. "
                                         "The threshold is the value below which everything is considered noise."));*/

    QLabel *label4      = new QLabel(i18n("Softness:"), d->luminanceBox);
    d->softLumInput     = new RDoubleNumInput(d->luminanceBox);
    d->softLumInput->setDecimals(1);
    d->softLumInput->input()->setRange(0.0, 1.0, 0.1, true);
    d->softLumInput->setDefaultValue(0.1);
/*    d->softLumInput->setWhatsThis(i18n("<b>Softness</b>: This adjusts the softness of the thresholding "
                                        "(soft as opposed to hard thresholding). The higher the softness "
                                        "the more noise remains in the image."));*/

    lumLay->addWidget(label3,          0, 0, 1, 1);
    lumLay->addWidget(d->thrLumInput,  0, 1, 1, 1);
    lumLay->addWidget(label4,          1, 0, 1, 1);
    lumLay->addWidget(d->softLumInput, 1, 1, 1, 1);
    lumLay->setRowStretch(2, 10);
    lumLay->setSpacing(0);
    lumLay->setMargin(KDialog::spacingHint());

    // -------------------------------------------------------------

    d->chrominanceRedBox = new QWidget(d->advExpanderBox);
    QGridLayout* cRedLay = new QGridLayout(d->chrominanceRedBox);
    
    QLabel *label5       = new QLabel(i18n("Threshold:"), d->chrominanceRedBox);
    d->thrCrInput        = new RDoubleNumInput(d->chrominanceRedBox);
    d->thrCrInput->setDecimals(2);
    d->thrCrInput->input()->setRange(0.0, 10.0, 0.1, true);
    d->thrCrInput->setDefaultValue(1.2);
/*    d->thrCrInput->setWhatsThis(i18n("<b>Threshold</b>: Adjusts the threshold for denoising of "
                                         "the image in a range from 0.0 (none) to 10.0. "
                                         "The threshold is the value below which everything is considered noise."));*/

    QLabel *label6       = new QLabel(i18n("Softness:"), d->chrominanceRedBox);
    d->softCrInput       = new RDoubleNumInput(d->chrominanceRedBox);
    d->softCrInput->setDecimals(1);
    d->softCrInput->input()->setRange(0.0, 1.0, 0.1, true);
    d->softCrInput->setDefaultValue(0.1);
/*    d->softCrInput->setWhatsThis(i18n("<b>Softness</b>: This adjusts the softness of the thresholding "
                                        "(soft as opposed to hard thresholding). The higher the softness "
                                        "the more noise remains in the image."));*/

    cRedLay->addWidget(label5,         0, 0, 1, 1);
    cRedLay->addWidget(d->thrCrInput,  0, 1, 1, 1);
    cRedLay->addWidget(label6,         1, 0, 1, 1);
    cRedLay->addWidget(d->softCrInput, 1, 1, 1, 1);
    cRedLay->setRowStretch(2, 10);
    cRedLay->setSpacing(0);
    cRedLay->setMargin(KDialog::spacingHint());

    // -------------------------------------------------------------

    d->chrominanceBlueBox = new QWidget(d->advExpanderBox);
    QGridLayout* cBlueLay = new QGridLayout(d->chrominanceBlueBox);
    
    QLabel *label7       = new QLabel(i18n("Threshold:"), d->chrominanceBlueBox);
    d->thrCbInput        = new RDoubleNumInput(d->chrominanceBlueBox);
    d->thrCbInput->setDecimals(2);
    d->thrCbInput->input()->setRange(0.0, 10.0, 0.1, true);
    d->thrCbInput->setDefaultValue(1.2);
/*    d->thrCbInput->setWhatsThis(i18n("<b>Threshold</b>: Adjusts the threshold for denoising of "
                                         "the image in a range from 0.0 (none) to 10.0. "
                                         "The threshold is the value below which everything is considered noise."));*/

    QLabel *label8       = new QLabel(i18n("Softness:"), d->chrominanceBlueBox);
    d->softCbInput       = new RDoubleNumInput(d->chrominanceBlueBox);
    d->softCbInput->setDecimals(1);
    d->softCbInput->input()->setRange(0.0, 1.0, 0.1, true);
    d->softCbInput->setDefaultValue(0.1);
/*    d->softCbInput->setWhatsThis(i18n("<b>Softness</b>: This adjusts the softness of the thresholding "
                                        "(soft as opposed to hard thresholding). The higher the softness "
                                        "the more noise remains in the image."));*/

    cBlueLay->addWidget(label7,         0, 0, 1, 1);
    cBlueLay->addWidget(d->thrCbInput,  0, 1, 1, 1);
    cBlueLay->addWidget(label8,         1, 0, 1, 1);
    cBlueLay->addWidget(d->softCbInput, 1, 1, 1, 1);
    cBlueLay->setRowStretch(2, 10);
    cBlueLay->setSpacing(0);
    cBlueLay->setMargin(KDialog::spacingHint());

    // -------------------------------------------------------------

    d->advExpanderBox->addItem(d->luminanceBox,       i18n("Luminance"),        QString("Luminance"),       true);
    d->advExpanderBox->addItem(d->chrominanceRedBox,  i18n("Chrominance Red"),  QString("ChrominanceRed"),  true);
    d->advExpanderBox->addItem(d->chrominanceBlueBox, i18n("Chrominance Blue"), QString("ChrominanceBlue"), true);
    d->advExpanderBox->addStretch();
        
    // -------------------------------------------------------------

    grid->addWidget(d->generalBox,     0, 0, 1, 2);
    grid->addWidget(d->advancedBox,    1, 0, 1, 2);
    grid->addWidget(d->advExpanderBox, 2, 0, 1, 2);
    grid->setRowStretch(3, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------
        
    connect(d->thresholdInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->softnessInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->thrLumInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->softLumInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->thrCrInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->softCrInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->thrCbInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->softCbInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));
            
    connect(d->advancedBox, SIGNAL(toggled(bool)),
            this, SLOT(slotAdvancedEnabled(bool)));           
}

NoiseReductionSettings::~NoiseReductionSettings()
{
    delete d;
}

WaveletsNRContainer NoiseReductionSettings::settings() const
{
    WaveletsNRContainer settings;
    settings.thresholds[0] = d->thrLumInput->value();
    settings.thresholds[1] = d->thrCrInput->value();
    settings.thresholds[2] = d->thrCbInput->value();
    settings.softness[0]   = 1.0 - d->softLumInput->value();
    settings.softness[1]   = 1.0 - d->softCrInput->value();
    settings.softness[2]   = 1.0 - d->softCbInput->value();
    settings.advanced      = d->advancedBox->isChecked();
    settings.leadThreshold = d->thresholdInput->value();
    settings.leadSoftness  = 1.0 - d->softnessInput->value();
    return settings;
}

void NoiseReductionSettings::setSettings(const WaveletsNRContainer& settings)
{
    d->thrLumInput->setValue(settings.thresholds[0]);
    d->thrCrInput->setValue(settings.thresholds[1]);
    d->thrCbInput->setValue(settings.thresholds[2]);
    d->softLumInput->setValue(1.0 - settings.softness[0]); 
    d->softCrInput->setValue(1.0 - settings.softness[1]);
    d->softCbInput->setValue(1.0 - settings.softness[2]);
    d->advancedBox->setChecked(settings.advanced);
    d->thresholdInput->setValue(settings.leadThreshold);
    d->softnessInput->setValue(1.0 - settings.leadSoftness);
}

void NoiseReductionSettings::resetToDefault()
{
    d->thresholdInput->slotReset();
    d->softnessInput->slotReset();
    d->thrLumInput->slotReset();
    d->softLumInput->slotReset();
    d->thrCrInput->slotReset();
    d->softCrInput->slotReset();
    d->thrCbInput->slotReset();
    d->softCbInput->slotReset();
    d->advancedBox->setChecked(false);
}

WaveletsNRContainer NoiseReductionSettings::defaultSettings() const
{
    WaveletsNRContainer settings;
    settings.thresholds[0] = d->thrLumInput->defaultValue();
    settings.thresholds[1] = d->thrCrInput->defaultValue();
    settings.thresholds[2] = d->thrCbInput->defaultValue();
    settings.softness[0]   = 1.0 - d->softLumInput->defaultValue();
    settings.softness[1]   = 1.0 - d->softCrInput->defaultValue();
    settings.softness[2]   = 1.0 - d->softCbInput->defaultValue();
    settings.advanced      = false;
    settings.leadThreshold = d->thresholdInput->defaultValue();
    settings.leadSoftness  = 1.0 - d->softnessInput->defaultValue();
    return settings;
}

void NoiseReductionSettings::slotAdvancedEnabled(bool b)
{
    d->generalBox->setEnabled(!b);
    d->advExpanderBox->setEnabled(b);
}

void NoiseReductionSettings::readSettings(KConfigGroup& group)
{
    WaveletsNRContainer settings;
    WaveletsNRContainer defaultPrm = defaultSettings();

    settings.thresholds[0] = group.readEntry(d->configThrLumInputAdjustmentEntry,  defaultPrm.thresholds[0]);
    settings.thresholds[1] = group.readEntry(d->configThrCrInputAdjustmentEntry,   defaultPrm.thresholds[1]);
    settings.thresholds[2] = group.readEntry(d->configThrCbInputAdjustmentEntry,   defaultPrm.thresholds[2]); 
    settings.softness[0]   = group.readEntry(d->configSoftLumInputAdjustmentEntry, defaultPrm.softness[0]);
    settings.softness[1]   = group.readEntry(d->configSoftCrInputAdjustmentEntry,  defaultPrm.softness[1]);
    settings.softness[2]   = group.readEntry(d->configSoftCbInputAdjustmentEntry,  defaultPrm.softness[2]);
    settings.advanced      = group.readEntry(d->configAdvancedAdjustmentEntry,     defaultPrm.advanced);
    settings.leadThreshold = group.readEntry(d->configThresholdAdjustmentEntry,    defaultPrm.leadThreshold);
    settings.leadSoftness  = group.readEntry(d->configSoftnessAdjustmentEntry,     defaultPrm.leadSoftness);
    setSettings(settings);
}

void NoiseReductionSettings::saveSettings(KConfigGroup& group)
{
    WaveletsNRContainer prm;
    
    group.writeEntry(d->configThrLumInputAdjustmentEntry,  prm.thresholds[0]);
    group.writeEntry(d->configThrCrInputAdjustmentEntry,   prm.thresholds[1]);
    group.writeEntry(d->configThrCbInputAdjustmentEntry,   prm.thresholds[2]);
    group.writeEntry(d->configSoftLumInputAdjustmentEntry, prm.softness[0]);
    group.writeEntry(d->configSoftCrInputAdjustmentEntry,  prm.softness[1]);
    group.writeEntry(d->configSoftCbInputAdjustmentEntry,  prm.softness[2]);
    group.writeEntry(d->configAdvancedAdjustmentEntry,     prm.advanced);
    group.writeEntry(d->configThresholdAdjustmentEntry,    prm.leadThreshold);
    group.writeEntry(d->configSoftnessAdjustmentEntry,     prm.leadSoftness);
}

}  // namespace Digikam
