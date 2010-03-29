/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-10
 * Description : Film Grain settings view.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "filmgrainsettings.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>

// KDE includes

#include <kdebug.h>
#include <kurl.h>
#include <kdialog.h>
#include <klocale.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "rexpanderbox.h"

using namespace KDcrawIface;

namespace Digikam
{

class FilmGrainSettingsPriv
{
public:

    FilmGrainSettingsPriv() :
        configGrainSizeEntry("GrainSizeEntry"),
        configPhotoDistributionEntry("PhotoDistributionEntry"),
        configAddLumNoiseEntry("AddLumNoiseEntry"),
        configIntensityLumAdjustmentEntry("IntensityLumAdjustment"),
        configShadowsLumAdjustmentEntry("ShadowsLumAdjustment"),
        configMidtonesLumAdjustmentEntry("MidtonesLumAdjustment"),
        configHighlightsLumAdjustmentEntry("HighlightsLumAdjustment"),
        configAddChromaBlueNoiseEntry("AddChromaBlueNoiseEntry"),
        configIntensityChromaBlueAdjustmentEntry("IntensityChromaBlueAdjustment"),
        configShadowsChromaBlueAdjustmentEntry("ShadowsChromaBlueAdjustment"),
        configMidtonesChromaBlueAdjustmentEntry("MidtonesChromaBlueAdjustment"),
        configHighlightsChromaBlueAdjustmentEntry("HighlightsChromaBlueAdjustment"),
        configAddChromaRedNoiseEntry("AddChromaRedNoiseEntry"),
        configIntensityChromaRedAdjustmentEntry("IntensityChromaRedAdjustment"),
        configShadowsChromaRedAdjustmentEntry("ShadowsChromaRedAdjustment"),
        configMidtonesChromaRedAdjustmentEntry("MidtonesChromaRedAdjustment"),
        configHighlightsChromaRedAdjustmentEntry("HighlightsChromaRedAdjustment"),
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
        addLuminanceNoise(0),
        addChrominanceBlueNoise(0),
        addChrominanceRedNoise(0),
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
        highlightsChromaRedInput(0)
        {}

    const QString configGrainSizeEntry;
    const QString configPhotoDistributionEntry;
    const QString configAddLumNoiseEntry;
    const QString configIntensityLumAdjustmentEntry;
    const QString configShadowsLumAdjustmentEntry;
    const QString configMidtonesLumAdjustmentEntry;
    const QString configHighlightsLumAdjustmentEntry;
    const QString configAddChromaBlueNoiseEntry;
    const QString configIntensityChromaBlueAdjustmentEntry;
    const QString configShadowsChromaBlueAdjustmentEntry;
    const QString configMidtonesChromaBlueAdjustmentEntry;
    const QString configHighlightsChromaBlueAdjustmentEntry;
    const QString configAddChromaRedNoiseEntry;
    const QString configIntensityChromaRedAdjustmentEntry;
    const QString configShadowsChromaRedAdjustmentEntry;
    const QString configMidtonesChromaRedAdjustmentEntry;
    const QString configHighlightsChromaRedAdjustmentEntry;

    QLabel*       sizeLabel;
    QLabel*       label1;
    QLabel*       label2;
    QLabel*       label3;
    QLabel*       label4;
    QLabel*       label5;
    QLabel*       label6;
    QLabel*       label7;
    QLabel*       label8;
    QLabel*       label9;
    QLabel*       label10;
    QLabel*       label11;
    QLabel*       label12;

    QCheckBox*    photoDistribution;
    QCheckBox*    addLuminanceNoise;
    QCheckBox*    addChrominanceBlueNoise;
    QCheckBox*    addChrominanceRedNoise;

    RIntNumInput* grainSizeInput;
    RIntNumInput* intensityLumInput;
    RIntNumInput* shadowsLumInput; 
    RIntNumInput* midtonesLumInput;
    RIntNumInput* highlightsLumInput;
    RIntNumInput* intensityChromaBlueInput;
    RIntNumInput* shadowsChromaBlueInput; 
    RIntNumInput* midtonesChromaBlueInput;
    RIntNumInput* highlightsChromaBlueInput;
    RIntNumInput* intensityChromaRedInput;
    RIntNumInput* shadowsChromaRedInput; 
    RIntNumInput* midtonesChromaRedInput;
    RIntNumInput* highlightsChromaRedInput;

    RExpanderBox* expanderBox;
};

FilmGrainSettings::FilmGrainSettings(QWidget* parent)
                 : QWidget(parent),
                   d(new FilmGrainSettingsPriv)
{
    QGridLayout* grid = new QGridLayout(parent);

    // -------------------------------------------------------------
    
    QWidget* commonPage = new QWidget();
    QGridLayout* grid0  = new QGridLayout(commonPage);

    d->sizeLabel        = new QLabel(i18n("Grain Size:"), commonPage);
    d->grainSizeInput   = new RIntNumInput(commonPage);
    d->grainSizeInput->setRange(1, 5, 1);
    d->grainSizeInput->setSliderEnabled(true);
    d->grainSizeInput->setDefaultValue(1);
    d->grainSizeInput->setWhatsThis(i18n("Set here the graininess size of film."));

    d->photoDistribution = new QCheckBox(i18n("Photographic Distribution"), commonPage);
    d->photoDistribution->setWhatsThis(i18n("Set on this option to render grain using a photon statistic distribution. "
                                            "This require more computation and can take a while."));    

    grid0->addWidget(d->sizeLabel,         0, 0, 1, 1);
    grid0->addWidget(d->grainSizeInput,    1, 0, 1, 1);
    grid0->addWidget(d->photoDistribution, 2, 0, 1, 1);
    grid0->setMargin(KDialog::spacingHint());
    grid0->setSpacing(KDialog::spacingHint());
    
    // -------------------------------------------------------------
    
    QWidget* firstPage   = new QWidget();
    QGridLayout* grid1   = new QGridLayout(firstPage);

    d->addLuminanceNoise = new QCheckBox(i18n("Add Luminance Noise"), firstPage);
    d->label1            = new QLabel(i18n("Intensity:"), firstPage);
    d->intensityLumInput = new RIntNumInput(firstPage);
    d->intensityLumInput->setRange(1, 100, 1);
    d->intensityLumInput->setSliderEnabled(true);
    d->intensityLumInput->setDefaultValue(25);
    d->intensityLumInput->setWhatsThis(i18n("Set here the film ISO-sensitivity to use for "
                                            "simulating the film graininess."));

    // -------------------------------------------------------------

    d->label2          = new QLabel(i18n("Shadows:"), firstPage);
    d->shadowsLumInput = new RIntNumInput(firstPage);
    d->shadowsLumInput->setRange(-100, 100, 1);
    d->shadowsLumInput->setSliderEnabled(true);
    d->shadowsLumInput->setDefaultValue(-100);
    d->shadowsLumInput->setWhatsThis(i18n("Set how much the filter affects highlights."));

    // -------------------------------------------------------------

    d->label3           = new QLabel(i18n("Midtones:"), firstPage);
    d->midtonesLumInput = new RIntNumInput(firstPage);
    d->midtonesLumInput->setRange(-100, 100, 1);
    d->midtonesLumInput->setSliderEnabled(true);
    d->midtonesLumInput->setDefaultValue(0);
    d->midtonesLumInput->setWhatsThis(i18n("Set how much the filter affects midtones."));

    // -------------------------------------------------------------

    d->label4             = new QLabel(i18n("Highlights:"), firstPage);
    d->highlightsLumInput = new RIntNumInput(firstPage);
    d->highlightsLumInput->setRange(-100, 100, 1);
    d->highlightsLumInput->setSliderEnabled(true);
    d->highlightsLumInput->setDefaultValue(-100);
    d->highlightsLumInput->setWhatsThis(i18n("Set how much the filter affects shadows."));

    grid1->addWidget(d->addLuminanceNoise,  0, 0, 1, 1);
    grid1->addWidget(d->label1,             1, 0, 1, 1);
    grid1->addWidget(d->intensityLumInput,  2, 0, 1, 1);
    grid1->addWidget(d->label2,             3, 0, 1, 1);
    grid1->addWidget(d->shadowsLumInput,    4, 0, 1, 1);
    grid1->addWidget(d->label3,             5, 0, 1, 1);
    grid1->addWidget(d->midtonesLumInput,   6, 0, 1, 1);
    grid1->addWidget(d->label4,             7, 0, 1, 1);
    grid1->addWidget(d->highlightsLumInput, 8, 0, 1, 1);
    grid1->setMargin(KDialog::spacingHint());
    grid1->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    QWidget* secondPage = new QWidget();
    QGridLayout* grid2  = new QGridLayout( secondPage );

    d->addChrominanceBlueNoise  = new QCheckBox(i18n("Add Chrominance Blue Noise"), secondPage);
    d->label5                   = new QLabel(i18n("Intensity:"), secondPage);
    d->intensityChromaBlueInput = new RIntNumInput(secondPage);
    d->intensityChromaBlueInput->setRange(1, 100, 1);
    d->intensityChromaBlueInput->setSliderEnabled(true);
    d->intensityChromaBlueInput->setDefaultValue(25);
    d->intensityChromaBlueInput->setWhatsThis(i18n("Set here the film sensitivity to use for "
                                                   "simulating the CCD blue noise."));

    // -------------------------------------------------------------

    d->label6                 = new QLabel(i18n("Shadows:"), secondPage);
    d->shadowsChromaBlueInput = new RIntNumInput(secondPage);
    d->shadowsChromaBlueInput->setRange(-100, 100, 1);
    d->shadowsChromaBlueInput->setSliderEnabled(true);
    d->shadowsChromaBlueInput->setDefaultValue(-100);
    d->shadowsChromaBlueInput->setWhatsThis(i18n("Set how much the filter affects highlights."));

    // -------------------------------------------------------------

    d->label7                  = new QLabel(i18n("Midtones:"), secondPage);
    d->midtonesChromaBlueInput = new RIntNumInput(secondPage);
    d->midtonesChromaBlueInput->setRange(-100, 100, 1);
    d->midtonesChromaBlueInput->setSliderEnabled(true);
    d->midtonesChromaBlueInput->setDefaultValue(0);
    d->midtonesChromaBlueInput->setWhatsThis(i18n("Set how much the filter affects midtones."));

    // -------------------------------------------------------------

    d->label8                    = new QLabel(i18n("Highlights:"), secondPage);
    d->highlightsChromaBlueInput = new RIntNumInput(secondPage);
    d->highlightsChromaBlueInput->setRange(-100, 100, 1);
    d->highlightsChromaBlueInput->setSliderEnabled(true);
    d->highlightsChromaBlueInput->setDefaultValue(-100);
    d->highlightsChromaBlueInput->setWhatsThis(i18n("Set how much the filter affects shadows."));

    grid2->addWidget(d->addChrominanceBlueNoise,   0, 0, 1, 1);
    grid2->addWidget(d->label5,                    1, 0, 1, 1);
    grid2->addWidget(d->intensityChromaBlueInput,  2, 0, 1, 1);
    grid2->addWidget(d->label6,                    3, 0, 1, 1);
    grid2->addWidget(d->shadowsChromaBlueInput,    4, 0, 1, 1);  
    grid2->addWidget(d->label7,                    5, 0, 1, 1);
    grid2->addWidget(d->midtonesChromaBlueInput,   6, 0, 1, 1); 
    grid2->addWidget(d->label8,                    7, 0, 1, 1); 
    grid2->addWidget(d->highlightsChromaBlueInput, 8, 0, 1, 1);
    grid2->setMargin(KDialog::spacingHint());
    grid2->setSpacing(KDialog::spacingHint());    
    
    // -------------------------------------------------------------

    QWidget* thirdPage = new QWidget();
    QGridLayout* grid3 = new QGridLayout( thirdPage );

    d->addChrominanceRedNoise  = new QCheckBox(i18n("Add Chrominance Red Noise"), thirdPage);
    d->label9                  = new QLabel(i18n("Intensity:"), thirdPage);
    d->intensityChromaRedInput = new RIntNumInput(thirdPage);
    d->intensityChromaRedInput->setRange(1, 100, 1);
    d->intensityChromaRedInput->setSliderEnabled(true);
    d->intensityChromaRedInput->setDefaultValue(25);
    d->intensityChromaRedInput->setWhatsThis(i18n("Set here the film sensitivity to use for "
                                                  "simulating the CCD red noise."));

    // -------------------------------------------------------------

    d->label10               = new QLabel(i18n("Shadows:"), thirdPage);
    d->shadowsChromaRedInput = new RIntNumInput(thirdPage);
    d->shadowsChromaRedInput->setRange(-100, 100, 1);
    d->shadowsChromaRedInput->setSliderEnabled(true);
    d->shadowsChromaRedInput->setDefaultValue(-100);
    d->shadowsChromaRedInput->setWhatsThis(i18n("Set how much the filter affects highlights."));

    // -------------------------------------------------------------

    d->label11                = new QLabel(i18n("Midtones:"), thirdPage);
    d->midtonesChromaRedInput = new RIntNumInput(thirdPage);
    d->midtonesChromaRedInput->setRange(-100, 100, 1);
    d->midtonesChromaRedInput->setSliderEnabled(true);
    d->midtonesChromaRedInput->setDefaultValue(0);
    d->midtonesChromaRedInput->setWhatsThis(i18n("Set how much the filter affects midtones."));

    // -------------------------------------------------------------

    d->label12                  = new QLabel(i18n("Highlights:"), thirdPage);
    d->highlightsChromaRedInput = new RIntNumInput(thirdPage);
    d->highlightsChromaRedInput->setRange(-100, 100, 1);
    d->highlightsChromaRedInput->setSliderEnabled(true);
    d->highlightsChromaRedInput->setDefaultValue(-100);
    d->highlightsChromaRedInput->setWhatsThis(i18n("Set how much the filter affects shadows."));

    grid3->addWidget(d->addChrominanceRedNoise,   0, 0, 1, 1);
    grid3->addWidget(d->label9,                   1, 0, 1, 1);
    grid3->addWidget(d->intensityChromaRedInput,  2, 0, 1, 1);
    grid3->addWidget(d->label10,                  3, 0, 1, 1);
    grid3->addWidget(d->shadowsChromaRedInput,    4, 0, 1, 1);  
    grid3->addWidget(d->label11,                  5, 0, 1, 1);
    grid3->addWidget(d->midtonesChromaRedInput,   6, 0, 1, 1); 
    grid3->addWidget(d->label12,                  7, 0, 1, 1); 
    grid3->addWidget(d->highlightsChromaRedInput, 8, 0, 1, 1);
    grid3->setMargin(KDialog::spacingHint());
    grid3->setSpacing(KDialog::spacingHint());    

    // -------------------------------------------------------------

    d->expanderBox = new RExpanderBox();
    d->expanderBox->setObjectName("Noise Expander");
    
    d->expanderBox->addItem(commonPage, SmallIcon("system-run"), 
                            i18n("Common Settings"),
                            QString("CommonSettingsContainer"), true);
    d->expanderBox->addItem(firstPage, KStandardDirs::locate("data", "digikam/data/colors-luma.png"),
                            i18n("Luminance Noise"),
                            QString("LuminanceSettingsContainer"), true);
    d->expanderBox->addItem(secondPage, KStandardDirs::locate("data", "digikam/data/colors-chromablue.png"), 
                            i18n("Chrominance Blue Noise"),
                            QString("ChrominanceBlueSettingsContainer"), true);
    d->expanderBox->addItem(thirdPage, KStandardDirs::locate("data", "digikam/data/colors-chromared.png"), 
                            i18n("Chrominance Red Noise"),
                            QString("ChrominanceRedSettingsContainer"), true);
    d->expanderBox->addStretch();

    grid->addWidget(d->expanderBox, 0, 0, 1, 1);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    connect(d->grainSizeInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->photoDistribution, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));
            
    connect(d->addLuminanceNoise, SIGNAL(toggled(bool)),
            this, SLOT(slotAddLuminanceNoise(bool)));

    connect(d->intensityLumInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->shadowsLumInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->midtonesLumInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->highlightsLumInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->addChrominanceBlueNoise, SIGNAL(toggled(bool)),
            this, SLOT(slotAddChrominanceBlueNoise(bool)));

    connect(d->intensityChromaBlueInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->shadowsChromaBlueInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->midtonesChromaBlueInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->highlightsChromaBlueInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->addChrominanceRedNoise, SIGNAL(toggled(bool)),
            this, SLOT(slotAddChrominanceRedNoise(bool)));

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

void FilmGrainSettings::slotAddLuminanceNoise(bool b)
{
    d->label1->setEnabled(b);
    d->label2->setEnabled(b);
    d->label3->setEnabled(b);
    d->label4->setEnabled(b);
    d->intensityLumInput->setEnabled(b);
    d->shadowsLumInput->setEnabled(b);
    d->midtonesLumInput->setEnabled(b);
    d->highlightsLumInput->setEnabled(b);
    emit signalSettingsChanged();
}

void FilmGrainSettings::slotAddChrominanceBlueNoise(bool b)
{
    d->label5->setEnabled(b);
    d->label6->setEnabled(b);
    d->label7->setEnabled(b);
    d->label8->setEnabled(b);
    d->intensityChromaBlueInput->setEnabled(b);
    d->shadowsChromaBlueInput->setEnabled(b);
    d->midtonesChromaBlueInput->setEnabled(b);
    d->highlightsChromaBlueInput->setEnabled(b);
    emit signalSettingsChanged();
}

void FilmGrainSettings::slotAddChrominanceRedNoise(bool b)
{
    d->label9->setEnabled(b);
    d->label10->setEnabled(b);
    d->label11->setEnabled(b);
    d->label12->setEnabled(b);
    d->intensityChromaRedInput->setEnabled(b);
    d->shadowsChromaRedInput->setEnabled(b);
    d->midtonesChromaRedInput->setEnabled(b);
    d->highlightsChromaRedInput->setEnabled(b);
    emit signalSettingsChanged();
}

FilmGrainContainer FilmGrainSettings::settings() const
{
    FilmGrainContainer prm;
    prm.grainSize               = d->grainSizeInput->value();
    prm.photoDistribution       = d->photoDistribution->isChecked();
    prm.addLuminanceNoise       = d->addLuminanceNoise->isChecked();
    prm.lumaIntensity           = d->intensityLumInput->value();
    prm.lumaShadows             = d->shadowsLumInput->value();
    prm.lumaMidtones            = d->midtonesLumInput->value();
    prm.lumaHighlights          = d->highlightsLumInput->value();
    prm.addChrominanceBlueNoise = d->addChrominanceBlueNoise->isChecked();
    prm.chromaBlueIntensity     = d->intensityChromaBlueInput->value();
    prm.chromaBlueShadows       = d->shadowsChromaBlueInput->value(); 
    prm.chromaBlueMidtones      = d->midtonesChromaBlueInput->value();
    prm.chromaBlueHighlights    = d->highlightsChromaBlueInput->value();
    prm.addChrominanceRedNoise  = d->addChrominanceRedNoise->isChecked();
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
    d->addLuminanceNoise->setChecked(settings.addLuminanceNoise);
    d->intensityLumInput->setValue(settings.lumaIntensity);
    d->shadowsLumInput->setValue(settings.lumaShadows);
    d->midtonesLumInput->setValue(settings.lumaMidtones);
    d->highlightsLumInput->setValue(settings.lumaHighlights);
    d->addChrominanceBlueNoise->setChecked(settings.addChrominanceBlueNoise);
    d->intensityChromaBlueInput->setValue(settings.chromaBlueIntensity);
    d->shadowsChromaBlueInput->setValue(settings.chromaBlueShadows); 
    d->midtonesChromaBlueInput->setValue(settings.chromaBlueMidtones);
    d->highlightsChromaBlueInput->setValue(settings.chromaBlueHighlights);
    d->addChrominanceRedNoise->setChecked(settings.addChrominanceRedNoise);
    d->intensityChromaRedInput->setValue(settings.chromaRedIntensity);
    d->shadowsChromaRedInput->setValue(settings.chromaRedShadows); 
    d->midtonesChromaRedInput->setValue(settings.chromaRedMidtones);
    d->highlightsChromaRedInput->setValue(settings.chromaRedHighlights);
    slotAddLuminanceNoise(settings.addLuminanceNoise);
    slotAddChrominanceBlueNoise(settings.addChrominanceBlueNoise);
    slotAddChrominanceRedNoise(settings.addChrominanceRedNoise);

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
