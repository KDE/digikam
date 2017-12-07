/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-10
 * Description : Film Grain settings view.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "filmgrainsettings.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QUrl>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dexpanderbox.h"
#include "dnuminput.h"
#include "digikam_debug.h"

namespace Digikam
{

class FilmGrainSettings::Private
{
public:

    Private() :
        sizeLabel(0),
        label1(0),
        label2(0),
        label3(0),
        label4(0),
        label5(0),
        label6(0),
        label7(0),
        label8(0),
        label9(0),
        label10(0),
        label11(0),
        label12(0),
        photoDistribution(0),
        grainSizeInput(0),
        intensityLumInput(0),
        shadowsLumInput(0),
        midtonesLumInput(0),
        highlightsLumInput(0),
        intensityChromaBlueInput(0),
        shadowsChromaBlueInput(0),
        midtonesChromaBlueInput(0),
        highlightsChromaBlueInput(0),
        intensityChromaRedInput(0),
        shadowsChromaRedInput(0),
        midtonesChromaRedInput(0),
        highlightsChromaRedInput(0),
        expanderBox(0)
    {}

    static const QString configGrainSizeEntry;
    static const QString configPhotoDistributionEntry;
    static const QString configAddLumNoiseEntry;
    static const QString configIntensityLumAdjustmentEntry;
    static const QString configShadowsLumAdjustmentEntry;
    static const QString configMidtonesLumAdjustmentEntry;
    static const QString configHighlightsLumAdjustmentEntry;
    static const QString configAddChromaBlueNoiseEntry;
    static const QString configIntensityChromaBlueAdjustmentEntry;
    static const QString configShadowsChromaBlueAdjustmentEntry;
    static const QString configMidtonesChromaBlueAdjustmentEntry;
    static const QString configHighlightsChromaBlueAdjustmentEntry;
    static const QString configAddChromaRedNoiseEntry;
    static const QString configIntensityChromaRedAdjustmentEntry;
    static const QString configShadowsChromaRedAdjustmentEntry;
    static const QString configMidtonesChromaRedAdjustmentEntry;
    static const QString configHighlightsChromaRedAdjustmentEntry;

    QLabel*              sizeLabel;
    QLabel*              label1;
    QLabel*              label2;
    QLabel*              label3;
    QLabel*              label4;
    QLabel*              label5;
    QLabel*              label6;
    QLabel*              label7;
    QLabel*              label8;
    QLabel*              label9;
    QLabel*              label10;
    QLabel*              label11;
    QLabel*              label12;

    QCheckBox*           photoDistribution;

    DIntNumInput*        grainSizeInput;
    DIntNumInput*        intensityLumInput;
    DIntNumInput*        shadowsLumInput;
    DIntNumInput*        midtonesLumInput;
    DIntNumInput*        highlightsLumInput;
    DIntNumInput*        intensityChromaBlueInput;
    DIntNumInput*        shadowsChromaBlueInput;
    DIntNumInput*        midtonesChromaBlueInput;
    DIntNumInput*        highlightsChromaBlueInput;
    DIntNumInput*        intensityChromaRedInput;
    DIntNumInput*        shadowsChromaRedInput;
    DIntNumInput*        midtonesChromaRedInput;
    DIntNumInput*        highlightsChromaRedInput;

    DExpanderBox*        expanderBox;
};

const QString FilmGrainSettings::Private::configGrainSizeEntry(QLatin1String("GrainSizeEntry"));
const QString FilmGrainSettings::Private::configPhotoDistributionEntry(QLatin1String("PhotoDistributionEntry"));
const QString FilmGrainSettings::Private::configAddLumNoiseEntry(QLatin1String("AddLumNoiseEntry"));
const QString FilmGrainSettings::Private::configIntensityLumAdjustmentEntry(QLatin1String("IntensityLumAdjustment"));
const QString FilmGrainSettings::Private::configShadowsLumAdjustmentEntry(QLatin1String("ShadowsLumAdjustment"));
const QString FilmGrainSettings::Private::configMidtonesLumAdjustmentEntry(QLatin1String("MidtonesLumAdjustment"));
const QString FilmGrainSettings::Private::configHighlightsLumAdjustmentEntry(QLatin1String("HighlightsLumAdjustment"));
const QString FilmGrainSettings::Private::configAddChromaBlueNoiseEntry(QLatin1String("AddChromaBlueNoiseEntry"));
const QString FilmGrainSettings::Private::configIntensityChromaBlueAdjustmentEntry(QLatin1String("IntensityChromaBlueAdjustment"));
const QString FilmGrainSettings::Private::configShadowsChromaBlueAdjustmentEntry(QLatin1String("ShadowsChromaBlueAdjustment"));
const QString FilmGrainSettings::Private::configMidtonesChromaBlueAdjustmentEntry(QLatin1String("MidtonesChromaBlueAdjustment"));
const QString FilmGrainSettings::Private::configHighlightsChromaBlueAdjustmentEntry(QLatin1String("HighlightsChromaBlueAdjustment"));
const QString FilmGrainSettings::Private::configAddChromaRedNoiseEntry(QLatin1String("AddChromaRedNoiseEntry"));
const QString FilmGrainSettings::Private::configIntensityChromaRedAdjustmentEntry(QLatin1String("IntensityChromaRedAdjustment"));
const QString FilmGrainSettings::Private::configShadowsChromaRedAdjustmentEntry(QLatin1String("ShadowsChromaRedAdjustment"));
const QString FilmGrainSettings::Private::configMidtonesChromaRedAdjustmentEntry(QLatin1String("MidtonesChromaRedAdjustment"));
const QString FilmGrainSettings::Private::configHighlightsChromaRedAdjustmentEntry(QLatin1String("HighlightsChromaRedAdjustment"));

// --------------------------------------------------------

FilmGrainSettings::FilmGrainSettings(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const grid = new QGridLayout(parent);

    // -------------------------------------------------------------

    QWidget* const commonPage = new QWidget();
    QGridLayout* const grid0  = new QGridLayout(commonPage);

    d->sizeLabel        = new QLabel(i18n("Grain Size:"), commonPage);
    d->grainSizeInput   = new DIntNumInput(commonPage);
    d->grainSizeInput->setRange(1, 5, 1);
    d->grainSizeInput->setDefaultValue(1);
    d->grainSizeInput->setWhatsThis(i18n("Set here the graininess size of film."));

    d->photoDistribution = new QCheckBox(i18n("Photographic Distribution"), commonPage);
    d->photoDistribution->setWhatsThis(i18n("Set on this option to render grain using a photon statistic distribution. "
                                            "This require more computation and can take a while."));

    grid0->addWidget(d->sizeLabel,         0, 0, 1, 1);
    grid0->addWidget(d->grainSizeInput,    1, 0, 1, 1);
    grid0->addWidget(d->photoDistribution, 2, 0, 1, 1);
    grid0->setContentsMargins(spacing, spacing, spacing, spacing);

    // -------------------------------------------------------------

    QWidget* const firstPage   = new QWidget();
    QGridLayout* const grid1   = new QGridLayout(firstPage);

    d->label1            = new QLabel(i18n("Intensity:"), firstPage);
    d->intensityLumInput = new DIntNumInput(firstPage);
    d->intensityLumInput->setRange(1, 100, 1);
    d->intensityLumInput->setDefaultValue(25);
    d->intensityLumInput->setWhatsThis(i18n("Set here the film ISO-sensitivity to use for "
                                            "simulating the film graininess."));

    // -------------------------------------------------------------

    d->label2          = new QLabel(i18n("Shadows:"), firstPage);
    d->shadowsLumInput = new DIntNumInput(firstPage);
    d->shadowsLumInput->setRange(-100, 100, 1);
    d->shadowsLumInput->setDefaultValue(-100);
    d->shadowsLumInput->setWhatsThis(i18n("Set how much the filter affects highlights."));

    // -------------------------------------------------------------

    d->label3           = new QLabel(i18n("Midtones:"), firstPage);
    d->midtonesLumInput = new DIntNumInput(firstPage);
    d->midtonesLumInput->setRange(-100, 100, 1);
    d->midtonesLumInput->setDefaultValue(0);
    d->midtonesLumInput->setWhatsThis(i18n("Set how much the filter affects midtones."));

    // -------------------------------------------------------------

    d->label4             = new QLabel(i18n("Highlights:"), firstPage);
    d->highlightsLumInput = new DIntNumInput(firstPage);
    d->highlightsLumInput->setRange(-100, 100, 1);
    d->highlightsLumInput->setDefaultValue(-100);
    d->highlightsLumInput->setWhatsThis(i18n("Set how much the filter affects shadows."));

    grid1->addWidget(d->label1,             0, 0, 1, 1);
    grid1->addWidget(d->intensityLumInput,  1, 0, 1, 1);
    grid1->addWidget(d->label2,             2, 0, 1, 1);
    grid1->addWidget(d->shadowsLumInput,    3, 0, 1, 1);
    grid1->addWidget(d->label3,             4, 0, 1, 1);
    grid1->addWidget(d->midtonesLumInput,   5, 0, 1, 1);
    grid1->addWidget(d->label4,             6, 0, 1, 1);
    grid1->addWidget(d->highlightsLumInput, 7, 0, 1, 1);
    grid1->setContentsMargins(spacing, spacing, spacing, spacing);
    grid1->setSpacing(spacing);

    // -------------------------------------------------------------

    QWidget* const secondPage = new QWidget();
    QGridLayout* const grid2  = new QGridLayout(secondPage);

    d->label5                   = new QLabel(i18n("Intensity:"), secondPage);
    d->intensityChromaBlueInput = new DIntNumInput(secondPage);
    d->intensityChromaBlueInput->setRange(1, 100, 1);
    d->intensityChromaBlueInput->setDefaultValue(25);
    d->intensityChromaBlueInput->setWhatsThis(i18n("Set here the film sensitivity to use for "
                                                   "simulating the CCD blue noise."));

    // -------------------------------------------------------------

    d->label6                 = new QLabel(i18n("Shadows:"), secondPage);
    d->shadowsChromaBlueInput = new DIntNumInput(secondPage);
    d->shadowsChromaBlueInput->setRange(-100, 100, 1);
    d->shadowsChromaBlueInput->setDefaultValue(-100);
    d->shadowsChromaBlueInput->setWhatsThis(i18n("Set how much the filter affects highlights."));

    // -------------------------------------------------------------

    d->label7                  = new QLabel(i18n("Midtones:"), secondPage);
    d->midtonesChromaBlueInput = new DIntNumInput(secondPage);
    d->midtonesChromaBlueInput->setRange(-100, 100, 1);
    d->midtonesChromaBlueInput->setDefaultValue(0);
    d->midtonesChromaBlueInput->setWhatsThis(i18n("Set how much the filter affects midtones."));

    // -------------------------------------------------------------

    d->label8                    = new QLabel(i18n("Highlights:"), secondPage);
    d->highlightsChromaBlueInput = new DIntNumInput(secondPage);
    d->highlightsChromaBlueInput->setRange(-100, 100, 1);
    d->highlightsChromaBlueInput->setDefaultValue(-100);
    d->highlightsChromaBlueInput->setWhatsThis(i18n("Set how much the filter affects shadows."));

    grid2->addWidget(d->label5,                    0, 0, 1, 1);
    grid2->addWidget(d->intensityChromaBlueInput,  1, 0, 1, 1);
    grid2->addWidget(d->label6,                    2, 0, 1, 1);
    grid2->addWidget(d->shadowsChromaBlueInput,    3, 0, 1, 1);
    grid2->addWidget(d->label7,                    4, 0, 1, 1);
    grid2->addWidget(d->midtonesChromaBlueInput,   5, 0, 1, 1);
    grid2->addWidget(d->label8,                    6, 0, 1, 1);
    grid2->addWidget(d->highlightsChromaBlueInput, 7, 0, 1, 1);
    grid2->setContentsMargins(spacing, spacing, spacing, spacing);
    grid2->setSpacing(spacing);

    // -------------------------------------------------------------

    QWidget* const thirdPage = new QWidget();
    QGridLayout* const grid3 = new QGridLayout(thirdPage);

    d->label9                  = new QLabel(i18n("Intensity:"), thirdPage);
    d->intensityChromaRedInput = new DIntNumInput(thirdPage);
    d->intensityChromaRedInput->setRange(1, 100, 1);
    d->intensityChromaRedInput->setDefaultValue(25);
    d->intensityChromaRedInput->setWhatsThis(i18n("Set here the film sensitivity to use for "
                                                  "simulating the CCD red noise."));

    // -------------------------------------------------------------

    d->label10               = new QLabel(i18n("Shadows:"), thirdPage);
    d->shadowsChromaRedInput = new DIntNumInput(thirdPage);
    d->shadowsChromaRedInput->setRange(-100, 100, 1);
    d->shadowsChromaRedInput->setDefaultValue(-100);
    d->shadowsChromaRedInput->setWhatsThis(i18n("Set how much the filter affects highlights."));

    // -------------------------------------------------------------

    d->label11                = new QLabel(i18n("Midtones:"), thirdPage);
    d->midtonesChromaRedInput = new DIntNumInput(thirdPage);
    d->midtonesChromaRedInput->setRange(-100, 100, 1);
    d->midtonesChromaRedInput->setDefaultValue(0);
    d->midtonesChromaRedInput->setWhatsThis(i18n("Set how much the filter affects midtones."));

    // -------------------------------------------------------------

    d->label12                  = new QLabel(i18n("Highlights:"), thirdPage);
    d->highlightsChromaRedInput = new DIntNumInput(thirdPage);
    d->highlightsChromaRedInput->setRange(-100, 100, 1);
    d->highlightsChromaRedInput->setDefaultValue(-100);
    d->highlightsChromaRedInput->setWhatsThis(i18n("Set how much the filter affects shadows."));

    grid3->addWidget(d->label9,                   0, 0, 1, 1);
    grid3->addWidget(d->intensityChromaRedInput,  1, 0, 1, 1);
    grid3->addWidget(d->label10,                  2, 0, 1, 1);
    grid3->addWidget(d->shadowsChromaRedInput,    3, 0, 1, 1);
    grid3->addWidget(d->label11,                  4, 0, 1, 1);
    grid3->addWidget(d->midtonesChromaRedInput,   5, 0, 1, 1);
    grid3->addWidget(d->label12,                  6, 0, 1, 1);
    grid3->addWidget(d->highlightsChromaRedInput, 7, 0, 1, 1);
    grid3->setContentsMargins(spacing, spacing, spacing, spacing);
    grid3->setSpacing(spacing);

    // -------------------------------------------------------------

    d->expanderBox = new DExpanderBox();
    d->expanderBox->setObjectName(QLatin1String("Noise Expander"));

    d->expanderBox->addItem(commonPage, QIcon::fromTheme(QLatin1String("system-run")),
                            i18n("Common Settings"),
                            QLatin1String("CommonSettingsContainer"), true);
    d->expanderBox->addItem(firstPage, QIcon(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/colors-luma.png"))),
                            i18n("Luminance Noise"),
                            QLatin1String("LuminanceSettingsContainer"), true);
    d->expanderBox->addItem(secondPage, QIcon(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/colors-chromablue.png"))),
                            i18n("Chrominance Blue Noise"),
                            QLatin1String("ChrominanceBlueSettingsContainer"), true);
    d->expanderBox->addItem(thirdPage, QIcon(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/colors-chromared.png"))),
                            i18n("Chrominance Red Noise"),
                            QLatin1String("ChrominanceRedSettingsContainer"), true);

    d->expanderBox->addStretch();
    d->expanderBox->setCheckBoxVisible(1, true);
    d->expanderBox->setCheckBoxVisible(2, true);
    d->expanderBox->setCheckBoxVisible(3, true);

    grid->addWidget(d->expanderBox, 0, 0, 1, 1);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    connect(d->grainSizeInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->photoDistribution, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->expanderBox, SIGNAL(signalItemToggled(int,bool)),
            this, SLOT(slotItemToggled(int,bool)));

    connect(d->intensityLumInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->shadowsLumInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->midtonesLumInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->highlightsLumInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->intensityChromaBlueInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->shadowsChromaBlueInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->midtonesChromaBlueInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->highlightsChromaBlueInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->intensityChromaRedInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->shadowsChromaRedInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->midtonesChromaRedInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->highlightsChromaRedInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

FilmGrainSettings::~FilmGrainSettings()
{
    delete d;
}

void FilmGrainSettings::slotItemToggled(int index, bool b)
{
    switch (index)
    {
        case 1:
        {
            d->label1->setEnabled(b);
            d->label2->setEnabled(b);
            d->label3->setEnabled(b);
            d->label4->setEnabled(b);
            d->intensityLumInput->setEnabled(b);
            d->shadowsLumInput->setEnabled(b);
            d->midtonesLumInput->setEnabled(b);
            d->highlightsLumInput->setEnabled(b);
            break;
        }

        case 2:
        {
            d->label5->setEnabled(b);
            d->label6->setEnabled(b);
            d->label7->setEnabled(b);
            d->label8->setEnabled(b);
            d->intensityChromaBlueInput->setEnabled(b);
            d->shadowsChromaBlueInput->setEnabled(b);
            d->midtonesChromaBlueInput->setEnabled(b);
            d->highlightsChromaBlueInput->setEnabled(b);
            break;
        }

        case 3:
        {
            d->label9->setEnabled(b);
            d->label10->setEnabled(b);
            d->label11->setEnabled(b);
            d->label12->setEnabled(b);
            d->intensityChromaRedInput->setEnabled(b);
            d->shadowsChromaRedInput->setEnabled(b);
            d->midtonesChromaRedInput->setEnabled(b);
            d->highlightsChromaRedInput->setEnabled(b);
            break;
        }
    }

    emit signalSettingsChanged();
}

FilmGrainContainer FilmGrainSettings::settings() const
{
    FilmGrainContainer prm;
    prm.grainSize               = d->grainSizeInput->value();
    prm.photoDistribution       = d->photoDistribution->isChecked();
    prm.addLuminanceNoise       = d->expanderBox->isChecked(1);
    prm.lumaIntensity           = d->intensityLumInput->value();
    prm.lumaShadows             = d->shadowsLumInput->value();
    prm.lumaMidtones            = d->midtonesLumInput->value();
    prm.lumaHighlights          = d->highlightsLumInput->value();
    prm.addChrominanceBlueNoise = d->expanderBox->isChecked(2);
    prm.chromaBlueIntensity     = d->intensityChromaBlueInput->value();
    prm.chromaBlueShadows       = d->shadowsChromaBlueInput->value();
    prm.chromaBlueMidtones      = d->midtonesChromaBlueInput->value();
    prm.chromaBlueHighlights    = d->highlightsChromaBlueInput->value();
    prm.addChrominanceRedNoise  = d->expanderBox->isChecked(3);
    prm.chromaRedIntensity      = d->intensityChromaRedInput->value();
    prm.chromaRedShadows        = d->shadowsChromaRedInput->value();
    prm.chromaRedMidtones       = d->midtonesChromaRedInput->value();
    prm.chromaRedHighlights     = d->highlightsChromaRedInput->value();
    return prm;
}

void FilmGrainSettings::setSettings(const FilmGrainContainer& settings)
{
    blockSignals(true);

    d->grainSizeInput->setValue(settings.grainSize);
    d->photoDistribution->setChecked(settings.photoDistribution);
    d->expanderBox->setChecked(1, settings.addLuminanceNoise);
    d->intensityLumInput->setValue(settings.lumaIntensity);
    d->shadowsLumInput->setValue(settings.lumaShadows);
    d->midtonesLumInput->setValue(settings.lumaMidtones);
    d->highlightsLumInput->setValue(settings.lumaHighlights);
    d->expanderBox->setChecked(2, settings.addChrominanceBlueNoise);
    d->intensityChromaBlueInput->setValue(settings.chromaBlueIntensity);
    d->shadowsChromaBlueInput->setValue(settings.chromaBlueShadows);
    d->midtonesChromaBlueInput->setValue(settings.chromaBlueMidtones);
    d->highlightsChromaBlueInput->setValue(settings.chromaBlueHighlights);
    d->expanderBox->setChecked(3, settings.addChrominanceRedNoise);
    d->intensityChromaRedInput->setValue(settings.chromaRedIntensity);
    d->shadowsChromaRedInput->setValue(settings.chromaRedShadows);
    d->midtonesChromaRedInput->setValue(settings.chromaRedMidtones);
    d->highlightsChromaRedInput->setValue(settings.chromaRedHighlights);
    slotItemToggled(1, settings.addLuminanceNoise);
    slotItemToggled(2, settings.addChrominanceBlueNoise);
    slotItemToggled(3, settings.addChrominanceRedNoise);

    blockSignals(false);
}

void FilmGrainSettings::resetToDefault()
{
    setSettings(defaultSettings());
}

FilmGrainContainer FilmGrainSettings::defaultSettings() const
{
    return FilmGrainContainer();
}

void FilmGrainSettings::readSettings(KConfigGroup& group)
{
    FilmGrainContainer prm;
    FilmGrainContainer defaultPrm = defaultSettings();

    prm.grainSize               = group.readEntry(d->configGrainSizeEntry,                      defaultPrm.grainSize);
    prm.photoDistribution       = group.readEntry(d->configPhotoDistributionEntry,              defaultPrm.photoDistribution);
    prm.addLuminanceNoise       = group.readEntry(d->configAddLumNoiseEntry,                    defaultPrm.addLuminanceNoise);
    prm.lumaIntensity           = group.readEntry(d->configIntensityLumAdjustmentEntry,         defaultPrm.lumaIntensity);
    prm.lumaShadows             = group.readEntry(d->configShadowsLumAdjustmentEntry,           defaultPrm.lumaShadows);
    prm.lumaMidtones            = group.readEntry(d->configMidtonesLumAdjustmentEntry,          defaultPrm.lumaMidtones);
    prm.lumaHighlights          = group.readEntry(d->configHighlightsLumAdjustmentEntry,        defaultPrm.lumaHighlights);
    prm.addChrominanceBlueNoise = group.readEntry(d->configAddChromaBlueNoiseEntry,             defaultPrm.addChrominanceBlueNoise);
    prm.chromaBlueIntensity     = group.readEntry(d->configIntensityChromaBlueAdjustmentEntry,  defaultPrm.chromaBlueIntensity);
    prm.chromaBlueShadows       = group.readEntry(d->configShadowsChromaBlueAdjustmentEntry,    defaultPrm.chromaBlueShadows);
    prm.chromaBlueMidtones      = group.readEntry(d->configMidtonesChromaBlueAdjustmentEntry,   defaultPrm.chromaBlueMidtones);
    prm.chromaBlueHighlights    = group.readEntry(d->configHighlightsChromaBlueAdjustmentEntry, defaultPrm.chromaBlueHighlights);
    prm.addChrominanceRedNoise  = group.readEntry(d->configAddChromaRedNoiseEntry,              defaultPrm.addChrominanceRedNoise);
    prm.chromaRedIntensity      = group.readEntry(d->configIntensityChromaRedAdjustmentEntry,   defaultPrm.chromaRedIntensity);
    prm.chromaRedShadows        = group.readEntry(d->configShadowsChromaRedAdjustmentEntry,     defaultPrm.chromaRedShadows);
    prm.chromaRedMidtones       = group.readEntry(d->configMidtonesChromaRedAdjustmentEntry,    defaultPrm.chromaRedMidtones);
    prm.chromaRedHighlights     = group.readEntry(d->configHighlightsChromaRedAdjustmentEntry,  defaultPrm.chromaRedHighlights);

    setSettings(prm);
}

void FilmGrainSettings::writeSettings(KConfigGroup& group)
{
    FilmGrainContainer prm = settings();

    group.writeEntry(d->configGrainSizeEntry,                      prm.grainSize);
    group.writeEntry(d->configPhotoDistributionEntry,              prm.photoDistribution);

    group.writeEntry(d->configAddLumNoiseEntry,                    prm.addLuminanceNoise);
    group.writeEntry(d->configIntensityLumAdjustmentEntry,         prm.lumaIntensity);
    group.writeEntry(d->configShadowsLumAdjustmentEntry,           prm.lumaShadows);
    group.writeEntry(d->configMidtonesLumAdjustmentEntry,          prm.lumaMidtones);
    group.writeEntry(d->configHighlightsLumAdjustmentEntry,        prm.lumaHighlights);

    group.writeEntry(d->configAddChromaBlueNoiseEntry,             prm.addChrominanceBlueNoise);
    group.writeEntry(d->configIntensityChromaBlueAdjustmentEntry,  prm.chromaBlueIntensity);
    group.writeEntry(d->configShadowsChromaBlueAdjustmentEntry,    prm.chromaBlueShadows);
    group.writeEntry(d->configMidtonesChromaBlueAdjustmentEntry,   prm.chromaBlueMidtones);
    group.writeEntry(d->configHighlightsChromaBlueAdjustmentEntry, prm.chromaBlueHighlights);

    group.writeEntry(d->configAddChromaRedNoiseEntry,              prm.addChrominanceRedNoise);
    group.writeEntry(d->configIntensityChromaRedAdjustmentEntry,   prm.chromaRedIntensity);
    group.writeEntry(d->configShadowsChromaRedAdjustmentEntry,     prm.chromaRedShadows);
    group.writeEntry(d->configMidtonesChromaRedAdjustmentEntry,    prm.chromaRedMidtones);
    group.writeEntry(d->configHighlightsChromaRedAdjustmentEntry,  prm.chromaRedHighlights);
}

}  // namespace Digikam
