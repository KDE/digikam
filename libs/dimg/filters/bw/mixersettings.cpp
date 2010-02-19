/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-18
 * Description : Channel mixer settings view.
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

#include "mixersettings.moc"

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

class MixerSettingsPriv
{
public:

    MixerSettingsPriv() :
        configMonochromeEntry("Monochrome"),
        configPreserveLuminosityEntry("PreserveLuminosity"),
        configRedRedGainEntry("RedRedGain"),
        configRedGreenGainEntry("RedGreenGain"),
        configRedBlueGainEntry("RedBlueGain"),
        configGreenRedGainEntry("GreenRedGain"),
        configGreenGreenGainEntry("GreenGreenGain"),
        configGreenBlueGainEntry("GreenBlueGain"),
        configBlueRedGainEntry("BlueRedGain"),
        configBlueGreenGainEntry("BlueGreenGain"),
        configBlueBlueGainEntry("BlueBlueGain"),
        configBlackRedGainEntry("BlackRedGain"),
        configBlackGreenGainEntry("BlackGreenGain"),
        configBlackBlueGainEntry("BlackBlueGain"),
        redRedGain(0.0),
        redGreenGain(0.0),
        redBlueGain(0.0),
        greenRedGain(0.0),
        greenGreenGain(0.0),
        greenBlueGain(0.0),
        blueRedGain(0.0),
        blueGreenGain(0.0),
        blueBlueGain(0.0),
        blackRedGain(0.0),
        blackGreenGain(0.0),
        blackBlueGain(0.0),
        resetButton(0),
        preserveLuminosity(0),
        monochrome(0),
        redGain(0),
        greenGain(0),
        blueGain(0)
        {}

    const QString       configMonochromeEntry;
    const QString       configPreserveLuminosityEntry;
    const QString       configRedRedGainEntry;
    const QString       configRedGreenGainEntry;
    const QString       configRedBlueGainEntry;
    const QString       configGreenRedGainEntry;
    const QString       configGreenGreenGainEntry;
    const QString       configGreenBlueGainEntry;
    const QString       configBlueRedGainEntry;
    const QString       configBlueGreenGainEntry;
    const QString       configBlueBlueGainEntry;
    const QString       configBlackRedGainEntry;
    const QString       configBlackGreenGainEntry;
    const QString       configBlackBlueGainEntry;

    double              redRedGain;
    double              redGreenGain;
    double              redBlueGain;
    double              greenRedGain;
    double              greenGreenGain;
    double              greenBlueGain;
    double              blueRedGain;
    double              blueGreenGain;
    double              blueBlueGain;
    double              blackRedGain;
    double              blackGreenGain;
    double              blackBlueGain;

    QPushButton*        resetButton;

    QCheckBox*          preserveLuminosity;
    QCheckBox*          monochrome;

    RDoubleNumInput*    redGain;
    RDoubleNumInput*    greenGain;
    RDoubleNumInput*    blueGain;
};

MixerSettings::MixerSettings(QWidget* parent)
             : QWidget(parent),
               d(new MixerSettingsPriv)
{
    QGridLayout* grid = new QGridLayout(parent);

    QLabel *redLabel = new QLabel(i18n("Red:"), d->gboxSettings->plainPage());
    d->redGain       = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->redGain->setDecimals(0);
    d->redGain->setRange(-200.0, 200.0, 1);
    d->redGain->setDefaultValue(0);
    d->redGain->setWhatsThis(i18n("Select the red color gain, as a percentage, "
                                 "for the current channel."));

    QLabel *greenLabel = new QLabel(i18n("Green:"), d->gboxSettings->plainPage());
    d->greenGain       = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->greenGain->setDecimals(0);
    d->greenGain->setRange(-200.0, 200.0, 1);
    d->greenGain->setDefaultValue(0);
    d->greenGain->setWhatsThis(i18n("Select the green color gain, as a percentage, "
                                    "for the current channel."));

    QLabel *blueLabel = new QLabel(i18n("Blue:"), d->gboxSettings->plainPage());
    d->blueGain       = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->blueGain->setDecimals(0);
    d->blueGain->setRange(-200.0, 200.0, 1);
    d->blueGain->setDefaultValue(0);
    d->blueGain->setWhatsThis(i18n("Select the blue color gain, as a percentage, "
                                   "for the current channel."));

    d->resetButton = new QPushButton(i18n("&Reset"), d->gboxSettings->plainPage());
    d->resetButton->setIcon(KIconLoader::global()->loadIcon("document-revert", KIconLoader::Toolbar));
    d->resetButton->setWhatsThis(i18n("Reset color channels' gains settings from "
                                      "the currently selected channel."));

    // -------------------------------------------------------------

    d->monochrome = new QCheckBox(i18n("Monochrome"), d->gboxSettings->plainPage());
    d->monochrome->setWhatsThis(i18n("Enable this option if you want the image rendered "
                                     "in monochrome mode. "
                                     "In this mode, the histogram will display only luminosity values."));

    d->preserveLuminosity = new QCheckBox(i18n("Preserve luminosity"), d->gboxSettings->plainPage());
    d->preserveLuminosity->setWhatsThis(i18n("Enable this option is you want preserve "
                                             "the image luminosity."));

    // -------------------------------------------------------------

    grid->addWidget(redLabel,               0, 0, 1, 1);
    grid->addWidget(d->redGain,             0, 1, 1, 4);
    grid->addWidget(greenLabel,             1, 0, 1, 1);
    grid->addWidget(d->greenGain,           1, 1, 1, 4);
    grid->addWidget(blueLabel,              2, 0, 1, 1);
    grid->addWidget(d->blueGain,            2, 1, 1, 4);
    grid->addWidget(d->resetButton,         3, 0, 1, 2);
    grid->addWidget(d->monochrome,          4, 0, 1, 5);
    grid->addWidget(d->preserveLuminosity,  5, 0, 1, 5);
    grid->setRowStretch(6, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    connect(d->redGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));

    connect(d->greenGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));

    connect(d->blueGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));

    connect(d->preserveLuminosity, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));

    connect(d->monochrome, SIGNAL(toggled (bool)),
            this, SLOT(slotMonochromeActived(bool)));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetCurrentChannel()));
}

MixerSettings::~MixerSettings()
{
    delete d;
}

MixerContainer MixerSettings::settings() const
{
    MixerContainer prm;

    prm.thresholds[0] = d->thrLumInput->value();
    prm.thresholds[2] = d->thrCrInput->value();
    prm.thresholds[1] = d->thrCbInput->value();
    prm.softness[0]   = 1.0 - d->softLumInput->value();
    prm.softness[2]   = 1.0 - d->softCrInput->value();
    prm.softness[1]   = 1.0 - d->softCbInput->value();
    prm.advanced      = d->advancedBox->isChecked();
    prm.leadThreshold = d->thresholdInput->value();
    prm.leadSoftness  = 1.0 - d->softnessInput->value();

    return prm;
}

void MixerSettings::setSettings(const MixerContainer& settings)
{
    blockSignals(true);
    d->thrLumInput->setValue(settings.thresholds[0]);
    d->thrCrInput->setValue(settings.thresholds[2]);
    d->thrCbInput->setValue(settings.thresholds[1]);
    d->softLumInput->setValue(1.0 - settings.softness[0]);
    d->softCrInput->setValue(1.0 - settings.softness[2]);
    d->softCbInput->setValue(1.0 - settings.softness[1]);
    d->advancedBox->setChecked(settings.advanced);
    d->thresholdInput->setValue(settings.leadThreshold);
    d->softnessInput->setValue(1.0 - settings.leadSoftness);
    slotAdvancedEnabled(settings.advanced);
    blockSignals(false);
}

void MixerSettings::resetToDefault()
{
    blockSignals(true);
    d->thresholdInput->slotReset();
    d->softnessInput->slotReset();
    d->thrLumInput->slotReset();
    d->softLumInput->slotReset();
    d->thrCrInput->slotReset();
    d->softCrInput->slotReset();
    d->thrCbInput->slotReset();
    d->softCbInput->slotReset();
    d->advancedBox->setChecked(false);
    slotAdvancedEnabled(false);
    blockSignals(false);
}

MixerContainer MixerSettings::defaultSettings() const
{
    MixerContainer prm;

    prm.thresholds[0] = d->thrLumInput->defaultValue();
    prm.thresholds[2] = d->thrCrInput->defaultValue();
    prm.thresholds[1] = d->thrCbInput->defaultValue();
    prm.softness[0]   = 1.0 - d->softLumInput->defaultValue();
    prm.softness[2]   = 1.0 - d->softCrInput->defaultValue();
    prm.softness[1]   = 1.0 - d->softCbInput->defaultValue();
    prm.advanced      = false;
    prm.leadThreshold = d->thresholdInput->defaultValue();
    prm.leadSoftness  = 1.0 - d->softnessInput->defaultValue();

    return prm;
}

void MixerSettings::readSettings(KConfigGroup& group)
{
    MixerContainer prm;
    MixerContainer defaultPrm = defaultSettings();

    prm.thresholds[0] = group.readEntry(d->configThrLumInputAdjustmentEntry,  defaultPrm.thresholds[0]);
    prm.thresholds[2] = group.readEntry(d->configThrCrInputAdjustmentEntry,   defaultPrm.thresholds[2]);
    prm.thresholds[1] = group.readEntry(d->configThrCbInputAdjustmentEntry,   defaultPrm.thresholds[1]);
    prm.softness[0]   = group.readEntry(d->configSoftLumInputAdjustmentEntry, defaultPrm.softness[0]);
    prm.softness[2]   = group.readEntry(d->configSoftCrInputAdjustmentEntry,  defaultPrm.softness[2]);
    prm.softness[1]   = group.readEntry(d->configSoftCbInputAdjustmentEntry,  defaultPrm.softness[1]);
    prm.advanced      = group.readEntry(d->configAdvancedAdjustmentEntry,     defaultPrm.advanced);
    prm.leadThreshold = group.readEntry(d->configThresholdAdjustmentEntry,    defaultPrm.leadThreshold);
    prm.leadSoftness  = group.readEntry(d->configSoftnessAdjustmentEntry,     defaultPrm.leadSoftness);
    setSettings(prm);
}

void MixerSettings::writeSettings(KConfigGroup& group)
{
    MixerContainer prm = settings();

    group.writeEntry(d->configThrLumInputAdjustmentEntry,  prm.thresholds[0]);
    group.writeEntry(d->configThrCrInputAdjustmentEntry,   prm.thresholds[2]);
    group.writeEntry(d->configThrCbInputAdjustmentEntry,   prm.thresholds[1]);
    group.writeEntry(d->configSoftLumInputAdjustmentEntry, prm.softness[0]);
    group.writeEntry(d->configSoftCrInputAdjustmentEntry,  prm.softness[2]);
    group.writeEntry(d->configSoftCbInputAdjustmentEntry,  prm.softness[1]);
    group.writeEntry(d->configAdvancedAdjustmentEntry,     prm.advanced);
    group.writeEntry(d->configThresholdAdjustmentEntry,    prm.leadThreshold);
    group.writeEntry(d->configSoftnessAdjustmentEntry,     prm.leadSoftness);
}

void MixerSettings::loadSettings()
{
    KUrl loadRestorationFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                               QString( "*" ), kapp->activeWindow(),
                               QString( i18n("Photograph Noise Reduction Settings File to Load")) );
    if ( loadRestorationFile.isEmpty() )
        return;

    QFile file(loadRestorationFile.toLocalFile());

    if ( file.open(QIODevice::ReadOnly) )
    {
        QTextStream stream( &file );
        if ( stream.readLine() != "# Photograph Wavelets Noise Reduction Configuration File" )
        {
            KMessageBox::error(kapp->activeWindow(),
                               i18n("\"%1\" is not a Photograph Noise Reduction settings text file.",
                                    loadRestorationFile.fileName()));
            file.close();
            return;
        }

        blockSignals(true);

        d->thresholdInput->setValue( stream.readLine().toDouble() );
        d->softnessInput->setValue( stream.readLine().toDouble() );
        d->advancedBox->setChecked( (bool)stream.readLine().toInt() );

        d->thrLumInput->setValue( stream.readLine().toDouble() );
        d->softLumInput->setValue( stream.readLine().toDouble() );
        d->thrCrInput->setValue( stream.readLine().toDouble() );
        d->softCrInput->setValue( stream.readLine().toDouble() );
        d->thrCbInput->setValue( stream.readLine().toDouble() );
        d->softCbInput->setValue( stream.readLine().toDouble() );

        slotAdvancedEnabled(d->advancedBox->isChecked());

        blockSignals(false);
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Photograph Noise Reduction text file."));
    }

    file.close();
}

void MixerSettings::saveAsSettings()
{
    KUrl saveRestorationFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                               QString( "*" ), kapp->activeWindow(),
                               QString( i18n("Photograph Noise Reduction Settings File to Save")) );
    if ( saveRestorationFile.isEmpty() )
        return;

    QFile file(saveRestorationFile.toLocalFile());

    if ( file.open(QIODevice::WriteOnly) )
    {
        QTextStream stream( &file );
        stream << "# Photograph Wavelets Noise Reduction Configuration File\n";
        stream << d->thresholdInput->value()  << "\n";
        stream << d->softnessInput->value()   << "\n";
        stream << d->advancedBox->isChecked() << "\n";

        stream << d->thrLumInput->value()     << "\n";
        stream << d->softLumInput->value()    << "\n";
        stream << d->thrCrInput->value()      << "\n";
        stream << d->softCrInput->value()     << "\n";
        stream << d->thrCbInput->value()      << "\n";
        stream << d->softCbInput->value()     << "\n";
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Noise Reduction text file."));
    }

    file.close();
}

}  // namespace Digikam
