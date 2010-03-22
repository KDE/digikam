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
        configAddLumNoiseEntry("AddLumNoiseEntry"),
        configIntensityLumAdjustmentEntry("IntensityLumAdjustment"),
        configShadowsLumAdjustmentEntry("ShadowsLumAdjustment"),
        configMidtonesLumAdjustmentEntry("MidtonesLumAdjustment"),
        configHighlightsLumAdjustmentEntry("HighlightsLumAdjustment"),
        configAddChromaNoiseEntry("AddChromaNoiseEntry"),
        configIntensityChromaAdjustmentEntry("IntensityChromaAdjustment"),
        configShadowsChromaAdjustmentEntry("ShadowsChromaAdjustment"),
        configMidtonesChromaAdjustmentEntry("MidtonesChromaAdjustment"),
        configHighlightsChromaAdjustmentEntry("HighlightsChromaAdjustment"),
        label1(0),
        label2(0),
        label3(0),
        label4(0),
        label5(0),
        label6(0),
        label7(0),
        label8(0),
        addLuminanceNoise(0),
        addChrominanceNoise(0),
        intensityLumInput(0),
        shadowsLumInput(0),
        midtonesLumInput(0),
        highlightsLumInput(0),
        intensityChromaInput(0),
        shadowsChromaInput(0),
        midtonesChromaInput(0),
        highlightsChromaInput(0)
        {}

    const QString configAddLumNoiseEntry;
    const QString configIntensityLumAdjustmentEntry;
    const QString configShadowsLumAdjustmentEntry;
    const QString configMidtonesLumAdjustmentEntry;
    const QString configHighlightsLumAdjustmentEntry;
    const QString configAddChromaNoiseEntry;
    const QString configIntensityChromaAdjustmentEntry;
    const QString configShadowsChromaAdjustmentEntry;
    const QString configMidtonesChromaAdjustmentEntry;
    const QString configHighlightsChromaAdjustmentEntry;

    QLabel*       label1;
    QLabel*       label2;
    QLabel*       label3;
    QLabel*       label4;
    QLabel*       label5;
    QLabel*       label6;
    QLabel*       label7;
    QLabel*       label8;

    QCheckBox*    addLuminanceNoise;
    QCheckBox*    addChrominanceNoise;

    RIntNumInput* intensityLumInput;
    RIntNumInput* shadowsLumInput; 
    RIntNumInput* midtonesLumInput;
    RIntNumInput* highlightsLumInput;
    RIntNumInput* intensityChromaInput;
    RIntNumInput* shadowsChromaInput; 
    RIntNumInput* midtonesChromaInput;
    RIntNumInput* highlightsChromaInput;

    RExpanderBox* expanderBox;
};

FilmGrainSettings::FilmGrainSettings(QWidget* parent)
                 : QWidget(parent),
                   d(new FilmGrainSettingsPriv)
{
    QGridLayout* grid  = new QGridLayout(parent);
    QWidget* firstPage = new QWidget();
    QGridLayout* grid1 = new QGridLayout(firstPage);

    // -------------------------------------------------------------

    d->addLuminanceNoise = new QCheckBox(i18n("Add Luminance Noise"));
    d->label1            = new QLabel(i18n("Intensity:"), firstPage);
    d->intensityLumInput = new RIntNumInput(firstPage);
    d->intensityLumInput->setRange(1, 100, 1);
    d->intensityLumInput->setSliderEnabled(true);
    d->intensityLumInput->setDefaultValue(25);
    d->intensityLumInput->setWhatsThis(i18n("Set here the film ISO-sensitivity to use for "
                                            "simulating the film graininess."));

    // -------------------------------------------------------------

    d->label2             = new QLabel(i18n("Shadows:"), firstPage);
    d->highlightsLumInput = new RIntNumInput(firstPage);
    d->highlightsLumInput->setRange(-100, 100, 1);
    d->highlightsLumInput->setSliderEnabled(true);
    d->highlightsLumInput->setDefaultValue(-100);
    d->highlightsLumInput->setWhatsThis(i18n("Set how much the filter affects highlights."));

    // -------------------------------------------------------------

    d->label3           = new QLabel(i18n("Midtones:"), firstPage);
    d->midtonesLumInput = new RIntNumInput(firstPage);
    d->midtonesLumInput->setRange(-100, 100, 1);
    d->midtonesLumInput->setSliderEnabled(true);
    d->midtonesLumInput->setDefaultValue(0);
    d->midtonesLumInput->setWhatsThis(i18n("Set how much the filter affects midtones."));

    // -------------------------------------------------------------

    d->label4          = new QLabel(i18n("Highlights:"), firstPage);
    d->shadowsLumInput = new RIntNumInput(firstPage);
    d->shadowsLumInput->setRange(-100, 100, 1);
    d->shadowsLumInput->setSliderEnabled(true);
    d->shadowsLumInput->setDefaultValue(-100);
    d->shadowsLumInput->setWhatsThis(i18n("Set how much the filter affects shadows."));

    grid1->addWidget(d->addLuminanceNoise,      0, 0, 1, 1);
    grid1->addWidget(d->label1,                 1, 0, 1, 1);
    grid1->addWidget(d->intensityLumInput,      2, 0, 1, 1);
    grid1->addWidget(d->label2,                 3, 0, 1, 1);
    grid1->addWidget(d->shadowsLumInput,        4, 0, 1, 1);
    grid1->addWidget(d->label3,                 5, 0, 1, 1);
    grid1->addWidget(d->midtonesLumInput,       6, 0, 1, 1);
    grid1->addWidget(d->label4,                 7, 0, 1, 1);
    grid1->addWidget(d->highlightsLumInput,     8, 0, 1, 1);
    grid1->setMargin(KDialog::spacingHint());
    grid1->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    QWidget* secondPage = new QWidget();
    QGridLayout* grid2  = new QGridLayout( secondPage );

    d->addChrominanceNoise  = new QCheckBox(i18n("Add Chrominance Noise"));
    d->label5               = new QLabel(i18n("Intensity:"), secondPage);
    d->intensityChromaInput = new RIntNumInput(secondPage);
    d->intensityChromaInput->setRange(1, 100, 1);
    d->intensityChromaInput->setSliderEnabled(true);
    d->intensityChromaInput->setDefaultValue(25);
    d->intensityChromaInput->setWhatsThis(i18n("Set here the film ISO-sensitivity to use for "
                                               "simulating the CCD noise."));

    // -------------------------------------------------------------

    d->label6                = new QLabel(i18n("Shadows:"), secondPage);
    d->highlightsChromaInput = new RIntNumInput(secondPage);
    d->highlightsChromaInput->setRange(-100, 100, 1);
    d->highlightsChromaInput->setSliderEnabled(true);
    d->highlightsChromaInput->setDefaultValue(-100);
    d->highlightsChromaInput->setWhatsThis(i18n("Set how much the filter affects highlights."));

    // -------------------------------------------------------------

    d->label7              = new QLabel(i18n("Midtones:"), secondPage);
    d->midtonesChromaInput = new RIntNumInput(secondPage);
    d->midtonesChromaInput->setRange(-100, 100, 1);
    d->midtonesChromaInput->setSliderEnabled(true);
    d->midtonesChromaInput->setDefaultValue(0);
    d->midtonesChromaInput->setWhatsThis(i18n("Set how much the filter affects midtones."));

    // -------------------------------------------------------------

    d->label8             = new QLabel(i18n("Highlights:"), secondPage);
    d->shadowsChromaInput = new RIntNumInput(secondPage);
    d->shadowsChromaInput->setRange(-100, 100, 1);
    d->shadowsChromaInput->setSliderEnabled(true);
    d->shadowsChromaInput->setDefaultValue(-100);
    d->shadowsChromaInput->setWhatsThis(i18n("Set how much the filter affects shadows."));

    grid2->addWidget(d->addChrominanceNoise,    0, 0, 1, 1);
    grid2->addWidget(d->label5,                 1, 0, 1, 1);
    grid2->addWidget(d->intensityChromaInput,   2, 0, 1, 1);
    grid2->addWidget(d->label6,                 3, 0, 1, 1);
    grid2->addWidget(d->shadowsChromaInput,     4, 0, 1, 1);  
    grid2->addWidget(d->label7,                 5, 0, 1, 1);
    grid2->addWidget(d->midtonesChromaInput,    6, 0, 1, 1); 
    grid2->addWidget(d->label8,                 7, 0, 1, 1); 
    grid2->addWidget(d->highlightsChromaInput,  8, 0, 1, 1);
    grid2->setMargin(KDialog::spacingHint());
    grid2->setSpacing(KDialog::spacingHint());    

    // -------------------------------------------------------------

    d->expanderBox = new RExpanderBox();
    d->expanderBox->setObjectName("Noise Expander");
    d->expanderBox->addItem(firstPage, SmallIcon("filmgrain"), i18n("Luminance noise"),
                            QString("LuminanceSettingsContainer"), true);
    d->expanderBox->addItem(secondPage, SmallIcon("camera-photo"), i18n("Chrominance noise"),
                            QString("ChrominanceSettingsContainer"), true);
    d->expanderBox->addStretch();

    grid->addWidget(d->expanderBox, 0, 0, 1, 1);
    grid->setRowStretch(0, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

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

    connect(d->addChrominanceNoise, SIGNAL(toggled(bool)),
            this, SLOT(slotAddChrominanceNoise(bool)));

    connect(d->intensityChromaInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->shadowsChromaInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->midtonesChromaInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->highlightsChromaInput, SIGNAL(valueChanged(int)),
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

void FilmGrainSettings::slotAddChrominanceNoise(bool b)
{
    d->label5->setEnabled(b);
    d->label6->setEnabled(b);
    d->label7->setEnabled(b);
    d->label8->setEnabled(b);
    d->intensityChromaInput->setEnabled(b);
    d->shadowsChromaInput->setEnabled(b);
    d->midtonesChromaInput->setEnabled(b);
    d->highlightsChromaInput->setEnabled(b);
    emit signalSettingsChanged();
}

FilmGrainContainer FilmGrainSettings::settings() const
{
    FilmGrainContainer prm;
    prm.addLuminanceNoise   = d->addLuminanceNoise->isChecked();
    prm.lum_intensity       = d->intensityLumInput->value();
    prm.lum_shadows         = d->shadowsLumInput->value();
    prm.lum_midtones        = d->midtonesLumInput->value();
    prm.lum_highlights      = d->highlightsLumInput->value();
    prm.addChrominanceNoise = d->addChrominanceNoise->isChecked();
    prm.chroma_intensity    = d->intensityChromaInput->value();
    prm.chroma_shadows      = d->shadowsChromaInput->value(); 
    prm.chroma_midtones     = d->midtonesChromaInput->value();
    prm.chroma_highlights   = d->highlightsChromaInput->value();
    return prm;
}

void FilmGrainSettings::setSettings(const FilmGrainContainer& settings)
{
    blockSignals(true);

    d->addLuminanceNoise->setChecked(settings.addLuminanceNoise);
    d->intensityLumInput->setValue(settings.lum_intensity);
    d->shadowsLumInput->setValue(settings.lum_shadows);
    d->midtonesLumInput->setValue(settings.lum_midtones);
    d->highlightsLumInput->setValue(settings.lum_highlights);
    d->addChrominanceNoise->setChecked(settings.addChrominanceNoise);
    d->intensityChromaInput->setValue(settings.chroma_intensity);
    d->shadowsChromaInput->setValue(settings.chroma_shadows); 
    d->midtonesChromaInput->setValue(settings.chroma_midtones);
    d->highlightsChromaInput->setValue(settings.chroma_highlights);
    slotAddLuminanceNoise(settings.addLuminanceNoise);
    slotAddChrominanceNoise(settings.addChrominanceNoise);

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

    prm.addLuminanceNoise   = group.readEntry(d->configAddLumNoiseEntry,                true);
    prm.lum_intensity       = group.readEntry(d->configIntensityLumAdjustmentEntry,     defaultPrm.lum_intensity);
    prm.lum_shadows         = group.readEntry(d->configShadowsLumAdjustmentEntry,       defaultPrm.lum_shadows);
    prm.lum_midtones        = group.readEntry(d->configMidtonesLumAdjustmentEntry,      defaultPrm.lum_midtones);
    prm.lum_highlights      = group.readEntry(d->configHighlightsLumAdjustmentEntry,    defaultPrm.lum_highlights);
    prm.addChrominanceNoise = group.readEntry(d->configAddChromaNoiseEntry,             false);
    prm.chroma_intensity    = group.readEntry(d->configIntensityChromaAdjustmentEntry,  defaultPrm.chroma_intensity);
    prm.chroma_shadows      = group.readEntry(d->configShadowsChromaAdjustmentEntry,    defaultPrm.chroma_shadows);
    prm.chroma_midtones     = group.readEntry(d->configMidtonesChromaAdjustmentEntry,   defaultPrm.chroma_midtones);
    prm.chroma_highlights   = group.readEntry(d->configHighlightsChromaAdjustmentEntry, defaultPrm.chroma_highlights);

    setSettings(prm);
}

void FilmGrainSettings::writeSettings(KConfigGroup& group)
{
    FilmGrainContainer prm = settings();

    group.writeEntry(d->configAddLumNoiseEntry,                 prm.addLuminanceNoise);
    group.writeEntry(d->configIntensityLumAdjustmentEntry,      prm.lum_intensity);
    group.writeEntry(d->configShadowsLumAdjustmentEntry,        prm.lum_shadows);
    group.writeEntry(d->configMidtonesLumAdjustmentEntry,       prm.lum_midtones);
    group.writeEntry(d->configHighlightsLumAdjustmentEntry,     prm.lum_highlights);

    group.writeEntry(d->configAddChromaNoiseEntry,              prm.addChrominanceNoise);
    group.writeEntry(d->configIntensityChromaAdjustmentEntry,   prm.chroma_intensity);
    group.writeEntry(d->configShadowsChromaAdjustmentEntry,     prm.chroma_shadows);
    group.writeEntry(d->configMidtonesChromaAdjustmentEntry,    prm.chroma_midtones);
    group.writeEntry(d->configHighlightsChromaAdjustmentEntry,  prm.chroma_highlights);
}

}  // namespace Digikam
