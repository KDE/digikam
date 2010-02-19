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
#include <QPushButton>

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
        currentChannel(RedChannel),
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

    int                 currentChannel;
    
    QPushButton*        resetButton;

    QCheckBox*          preserveLuminosity;
    QCheckBox*          monochrome;

    MixerContainer      mixerSettings;
    
    RDoubleNumInput*    redGain;
    RDoubleNumInput*    greenGain;
    RDoubleNumInput*    blueGain;
};

MixerSettings::MixerSettings(QWidget* parent)
             : QWidget(parent),
               d(new MixerSettingsPriv)
{
    QGridLayout* grid = new QGridLayout(parent);

    QLabel* redLabel  = new QLabel(i18n("Red:"));
    d->redGain        = new RDoubleNumInput;
    d->redGain->setDecimals(0);
    d->redGain->setRange(-200.0, 200.0, 1);
    d->redGain->setDefaultValue(0);
    d->redGain->setWhatsThis(i18n("Select the red color gain, as a percentage, "
                                 "for the current channel."));

    QLabel* greenLabel = new QLabel(i18n("Green:"));
    d->greenGain       = new RDoubleNumInput;
    d->greenGain->setDecimals(0);
    d->greenGain->setRange(-200.0, 200.0, 1);
    d->greenGain->setDefaultValue(0);
    d->greenGain->setWhatsThis(i18n("Select the green color gain, as a percentage, "
                                    "for the current channel."));

    QLabel* blueLabel = new QLabel(i18n("Blue:"));
    d->blueGain       = new RDoubleNumInput;
    d->blueGain->setDecimals(0);
    d->blueGain->setRange(-200.0, 200.0, 1);
    d->blueGain->setDefaultValue(0);
    d->blueGain->setWhatsThis(i18n("Select the blue color gain, as a percentage, "
                                   "for the current channel."));

    d->resetButton = new QPushButton(i18n("&Reset"));
    d->resetButton->setIcon(KIconLoader::global()->loadIcon("document-revert", KIconLoader::Toolbar));
    d->resetButton->setWhatsThis(i18n("Reset color channels' gains settings from "
                                      "the currently selected channel."));

    // -------------------------------------------------------------

    d->monochrome = new QCheckBox(i18n("Monochrome"));
    d->monochrome->setWhatsThis(i18n("Enable this option if you want the image rendered "
                                     "in monochrome mode. "
                                     "In this mode, the histogram will display only luminosity values."));

    d->preserveLuminosity = new QCheckBox(i18n("Preserve luminosity"));
    d->preserveLuminosity->setWhatsThis(i18n("Enable this option is you want preserve "
                                             "the image luminosity."));

    // -------------------------------------------------------------

    grid->addWidget(redLabel,              0, 0, 1, 1);
    grid->addWidget(d->redGain,            0, 1, 1, 4);
    grid->addWidget(greenLabel,            1, 0, 1, 1);
    grid->addWidget(d->greenGain,          1, 1, 1, 4);
    grid->addWidget(blueLabel,             2, 0, 1, 1);
    grid->addWidget(d->blueGain,           2, 1, 1, 4);
    grid->addWidget(d->resetButton,        3, 0, 1, 2);
    grid->addWidget(d->monochrome,         4, 0, 1, 5);
    grid->addWidget(d->preserveLuminosity, 5, 0, 1, 5);
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

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetCurrentChannel()));

    connect(d->monochrome, SIGNAL(toggled(bool)),
            this, SLOT(slotMonochromeActived(bool)));

    connect(d->preserveLuminosity, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));
}

MixerSettings::~MixerSettings()
{
    delete d;
}

void MixerSettings::setCurrentChannel(int channel)
{
    d->currentChannel = channel;
    updateSettingsWidgets();
}

int MixerSettings::currentChannel()
{
    return d->currentChannel;
}

void MixerSettings::slotResetCurrentChannel()
{
    switch (d->currentChannel)
    {
        case GreenChannel:
        {
            d->mixerSettings.greenRedGain   = 0.0;
            d->mixerSettings.greenGreenGain = 1.0;
            d->mixerSettings.greenBlueGain  = 0.0;
            break;
        }
        
        case BlueChannel:
        {
            d->mixerSettings.blueRedGain   = 0.0;
            d->mixerSettings.blueGreenGain = 0.0;
            d->mixerSettings.blueBlueGain  = 1.0;
            break;
        }
            
        default:                        // Red or monochrome.
        {
            if (d->monochrome->isChecked())
            {
                d->mixerSettings.blackRedGain   = 1.0;
                d->mixerSettings.blackGreenGain = 0.0;
                d->mixerSettings.blackBlueGain  = 0.0;
            }
            else
            {
                d->mixerSettings.redRedGain   = 1.0;
                d->mixerSettings.redGreenGain = 0.0;
                d->mixerSettings.redBlueGain  = 0.0;
            }
            break;
        }
    }

    updateSettingsWidgets();
    emit signalSettingsChanged();
}

void MixerSettings::slotGainsChanged()
{
    switch(d->currentChannel)
    {
        case GreenChannel:
        {
            d->mixerSettings.greenRedGain   = d->redGain->value()   / 100.0;
            d->mixerSettings.greenGreenGain = d->greenGain->value() / 100.0;
            d->mixerSettings.greenBlueGain  = d->blueGain->value()  / 100.0;
            break;
        }

        case BlueChannel:
        {
            d->mixerSettings.blueRedGain   = d->redGain->value()   / 100.0;
            d->mixerSettings.blueGreenGain = d->greenGain->value() / 100.0;
            d->mixerSettings.blueBlueGain  = d->blueGain->value()  / 100.0;
            break;
        }

        default:                         // Red or monochrome.
        {
            if ( d->monochrome->isChecked() )
            {
              d->mixerSettings.blackRedGain   = d->redGain->value()   / 100.0;
              d->mixerSettings.blackGreenGain = d->greenGain->value() / 100.0;
              d->mixerSettings.blackBlueGain  = d->blueGain->value()  / 100.0;
            }
            else
            {
              d->mixerSettings.redRedGain   = d->redGain->value()   / 100.0;
              d->mixerSettings.redGreenGain = d->greenGain->value() / 100.0;
              d->mixerSettings.redBlueGain  = d->blueGain->value()  / 100.0;
            }
            break;
        }
    }

    emit signalSettingsChanged();
}

void MixerSettings::updateSettingsWidgets()
{
    d->monochrome->blockSignals(true);
    d->preserveLuminosity->blockSignals(true);
    d->redGain->blockSignals(true);
    d->greenGain->blockSignals(true);
    d->blueGain->blockSignals(true);

    switch(d->currentChannel)
    {
        case GreenChannel:
        {
            d->redGain->setDefaultValue(0);
            d->greenGain->setDefaultValue(100);
            d->blueGain->setDefaultValue(0);
            d->redGain->setValue(d->mixerSettings.greenRedGain     * 100.0);
            d->greenGain->setValue(d->mixerSettings.greenGreenGain * 100.0);
            d->blueGain->setValue(d->mixerSettings.greenBlueGain   * 100.0);
            break;
        }

        case BlueChannel:
        {
            d->redGain->setDefaultValue(0);
            d->greenGain->setDefaultValue(0);
            d->blueGain->setDefaultValue(100);
            d->redGain->setValue(d->mixerSettings.blueRedGain     * 100.0);
            d->greenGain->setValue(d->mixerSettings.blueGreenGain * 100.0);
            d->blueGain->setValue(d->mixerSettings.blueBlueGain   * 100.0);
            break;
        }

       default:          // Red or monochrome.
       {
            if ( d->monochrome->isChecked() )
            {
              d->redGain->setDefaultValue(100);
              d->greenGain->setDefaultValue(0);
              d->blueGain->setDefaultValue(0);
              d->redGain->setValue(d->mixerSettings.blackRedGain     * 100.0);
              d->greenGain->setValue(d->mixerSettings.blackGreenGain * 100.0);
              d->blueGain->setValue(d->mixerSettings.blackBlueGain   * 100.0);
            }
            else
            {
              d->redGain->setDefaultValue(100);
              d->greenGain->setDefaultValue(0);
              d->blueGain->setDefaultValue(0);
              d->redGain->setValue(d->mixerSettings.redRedGain     * 100.0);
              d->greenGain->setValue(d->mixerSettings.redGreenGain * 100.0);
              d->blueGain->setValue(d->mixerSettings.redBlueGain   * 100.0);
            }
            break;
        }
    }

    d->monochrome->setChecked(d->mixerSettings.bMonochrome);
    d->preserveLuminosity->setChecked(d->mixerSettings.bPreserveLum);

    d->monochrome->blockSignals(false);
    d->preserveLuminosity->blockSignals(false);
    d->redGain->blockSignals(false);
    d->greenGain->blockSignals(false);
    d->blueGain->blockSignals(false);
}

void MixerSettings::slotMonochromeActived(bool mono)
{
    d->mixerSettings.bMonochrome = d->monochrome->isChecked();
    emit signalMonochromeActived(mono);  
}
  
MixerContainer MixerSettings::settings() const
{
    MixerContainer prm;
/*
    prm.thresholds[0] = d->thrLumInput->value();
    prm.thresholds[2] = d->thrCrInput->value();
    prm.thresholds[1] = d->thrCbInput->value();
    prm.softness[0]   = 1.0 - d->softLumInput->value();
    prm.softness[2]   = 1.0 - d->softCrInput->value();
    prm.softness[1]   = 1.0 - d->softCbInput->value();
    prm.advanced      = d->advancedBox->isChecked();
    prm.leadThreshold = d->thresholdInput->value();
    prm.leadSoftness  = 1.0 - d->softnessInput->value();
*/
    return prm;
}

void MixerSettings::setSettings(const MixerContainer& settings)
{
    blockSignals(true);
/*    d->thrLumInput->setValue(settings.thresholds[0]);
    d->thrCrInput->setValue(settings.thresholds[2]);
    d->thrCbInput->setValue(settings.thresholds[1]);
    d->softLumInput->setValue(1.0 - settings.softness[0]);
    d->softCrInput->setValue(1.0 - settings.softness[2]);
    d->softCbInput->setValue(1.0 - settings.softness[1]);
    d->advancedBox->setChecked(settings.advanced);
    d->thresholdInput->setValue(settings.leadThreshold);
    d->softnessInput->setValue(1.0 - settings.leadSoftness);
*/
    blockSignals(false);
}

void MixerSettings::resetToDefault()
{
    blockSignals(true);
/*
    d->thresholdInput->slotReset();
    d->softnessInput->slotReset();
    d->thrLumInput->slotReset();
    d->softLumInput->slotReset();
    d->thrCrInput->slotReset();
    d->softCrInput->slotReset();
    d->thrCbInput->slotReset();
    d->softCbInput->slotReset();
    d->advancedBox->setChecked(false);
*/
    blockSignals(false);
}

MixerContainer MixerSettings::defaultSettings() const
{
    MixerContainer prm;
/*
    prm.thresholds[0] = d->thrLumInput->defaultValue();
    prm.thresholds[2] = d->thrCrInput->defaultValue();
    prm.thresholds[1] = d->thrCbInput->defaultValue();
    prm.softness[0]   = 1.0 - d->softLumInput->defaultValue();
    prm.softness[2]   = 1.0 - d->softCrInput->defaultValue();
    prm.softness[1]   = 1.0 - d->softCbInput->defaultValue();
    prm.advanced      = false;
    prm.leadThreshold = d->thresholdInput->defaultValue();
    prm.leadSoftness  = 1.0 - d->softnessInput->defaultValue();
*/
    return prm;
}

void MixerSettings::readSettings(KConfigGroup& group)
{
    MixerContainer prm;
    MixerContainer defaultPrm = defaultSettings();
/*
    prm.thresholds[0] = group.readEntry(d->configThrLumInputAdjustmentEntry,  defaultPrm.thresholds[0]);
    prm.thresholds[2] = group.readEntry(d->configThrCrInputAdjustmentEntry,   defaultPrm.thresholds[2]);
    prm.thresholds[1] = group.readEntry(d->configThrCbInputAdjustmentEntry,   defaultPrm.thresholds[1]);
    prm.softness[0]   = group.readEntry(d->configSoftLumInputAdjustmentEntry, defaultPrm.softness[0]);
    prm.softness[2]   = group.readEntry(d->configSoftCrInputAdjustmentEntry,  defaultPrm.softness[2]);
    prm.softness[1]   = group.readEntry(d->configSoftCbInputAdjustmentEntry,  defaultPrm.softness[1]);
    prm.advanced      = group.readEntry(d->configAdvancedAdjustmentEntry,     defaultPrm.advanced);
    prm.leadThreshold = group.readEntry(d->configThresholdAdjustmentEntry,    defaultPrm.leadThreshold);
    prm.leadSoftness  = group.readEntry(d->configSoftnessAdjustmentEntry,     defaultPrm.leadSoftness);
*/    
    setSettings(prm);
}

void MixerSettings::writeSettings(KConfigGroup& group)
{
    MixerContainer prm = settings();
/*
    group.writeEntry(d->configThrLumInputAdjustmentEntry,  prm.thresholds[0]);
    group.writeEntry(d->configThrCrInputAdjustmentEntry,   prm.thresholds[2]);
    group.writeEntry(d->configThrCbInputAdjustmentEntry,   prm.thresholds[1]);
    group.writeEntry(d->configSoftLumInputAdjustmentEntry, prm.softness[0]);
    group.writeEntry(d->configSoftCrInputAdjustmentEntry,  prm.softness[2]);
    group.writeEntry(d->configSoftCbInputAdjustmentEntry,  prm.softness[1]);
    group.writeEntry(d->configAdvancedAdjustmentEntry,     prm.advanced);
    group.writeEntry(d->configThresholdAdjustmentEntry,    prm.leadThreshold);
    group.writeEntry(d->configSoftnessAdjustmentEntry,     prm.leadSoftness);
*/    
}

void MixerSettings::loadSettings()
{
    KUrl  loadGainsFileUrl;
    FILE* fp = 0L;

    loadGainsFileUrl = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Select Gimp Gains Mixer File to Load")) );
    if ( loadGainsFileUrl.isEmpty() )
       return;

    fp = fopen(QFile::encodeName(loadGainsFileUrl.toLocalFile()), "r");

    if ( fp )
    {
        ChannelType currentOutputChannel = RedChannel;
        char buf1[1024];
        char buf2[1024];
        char buf3[1024];

        buf1[0] = '\0';

        fgets(buf1, 1023, fp);

        fscanf (fp, "%*s %s", buf1);

        // Get the current output channel in dialog.

        if (strcmp (buf1, "RED") == 0)
            currentOutputChannel = RedChannel;
        else if (strcmp (buf1, "GREEN") == 0)
            currentOutputChannel = GreenChannel;
        else if (strcmp (buf1, "BLUE") == 0)
            currentOutputChannel = BlueChannel;

        fscanf (fp, "%*s %s", buf1); // preview flag, preserved for compatibility

        fscanf (fp, "%*s %s", buf1);

        if (strcmp (buf1, "true") == 0)
            d->mixerSettings.bMonochrome = true;
        else
            d->mixerSettings.bMonochrome = false;

        fscanf (fp, "%*s %s", buf1);

        if (strcmp (buf1, "true") == 0)
            d->mixerSettings.bPreserveLum = true;
        else
            d->mixerSettings.bPreserveLum = false;

        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        d->mixerSettings.redRedGain   = atof(buf1);
        d->mixerSettings.redGreenGain = atof(buf2);
        d->mixerSettings.redBlueGain  = atof(buf3);

        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        d->mixerSettings.greenRedGain   = atof(buf1);
        d->mixerSettings.greenGreenGain = atof(buf2);
        d->mixerSettings.greenBlueGain  = atof(buf3);

        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        d->mixerSettings.blueRedGain   = atof(buf1);
        d->mixerSettings.blueGreenGain = atof(buf2);
        d->mixerSettings.blueBlueGain  = atof(buf3);

        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        d->mixerSettings.blackRedGain   = atof(buf1);
        d->mixerSettings.blackGreenGain = atof(buf2);
        d->mixerSettings.blackBlueGain  = atof(buf3);

        fclose(fp);

        // Refresh settings.
        updateSettingsWidgets();
        slotMonochromeActived(d->mixerSettings.bMonochrome);
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot load settings from the Gains Mixer text file."));
        return;
    }
}

void MixerSettings::saveAsSettings()
{
    KUrl  saveGainsFileUrl;
    FILE* fp = 0L;

    saveGainsFileUrl = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                               QString( "*" ), kapp->activeWindow(),
                                               QString( i18n("Gimp Gains Mixer File to Save")) );
    if ( saveGainsFileUrl.isEmpty() )
       return;

    fp = fopen(QFile::encodeName(saveGainsFileUrl.toLocalFile()), "w");

    if ( fp )
    {
        const char* str = 0L;
        char        buf1[256];
        char        buf2[256];
        char        buf3[256];

        switch (d->currentChannel)
        {
           case RedChannel:
              str = "RED";
              break;
           case GreenChannel:
              str = "GREEN";
              break;
           case BlueChannel:
              str = "BLUE";
              break;
           default:
              kWarning() <<  "Unknown Color channel gains";
              break;
        }

        fprintf (fp, "# Channel Mixer Configuration File\n");

        fprintf (fp, "CHANNEL: %s\n", str);
        fprintf (fp, "PREVIEW: %s\n", "true"); // preserved for compatibility
        fprintf (fp, "MONOCHROME: %s\n",
                 d->mixerSettings.bMonochrome ? "true" : "false");
        fprintf (fp, "PRESERVE_LUMINOSITY: %s\n",
                 d->mixerSettings.bPreserveLum ? "true" : "false");

        sprintf (buf1, "%5.3f", d->mixerSettings.redRedGain);
        sprintf (buf2, "%5.3f", d->mixerSettings.redGreenGain);
        sprintf (buf3, "%5.3f", d->mixerSettings.redBlueGain);
        fprintf (fp, "RED: %s %s %s\n", buf1, buf2,buf3);

        sprintf (buf1, "%5.3f", d->mixerSettings.greenRedGain);
        sprintf (buf2, "%5.3f", d->mixerSettings.greenGreenGain);
        sprintf (buf3, "%5.3f", d->mixerSettings.greenBlueGain);
        fprintf (fp, "GREEN: %s %s %s\n", buf1, buf2,buf3);

        sprintf (buf1, "%5.3f", d->mixerSettings.blueRedGain);
        sprintf (buf2, "%5.3f", d->mixerSettings.blueGreenGain);
        sprintf (buf3, "%5.3f", d->mixerSettings.blueBlueGain);
        fprintf (fp, "BLUE: %s %s %s\n", buf1, buf2,buf3);

        sprintf (buf1, "%5.3f", d->mixerSettings.blackRedGain);
        sprintf (buf2, "%5.3f", d->mixerSettings.blackGreenGain);
        sprintf (buf3, "%5.3f", d->mixerSettings.blackBlueGain);
        fprintf (fp, "BLACK: %s %s %s\n", buf1, buf2,buf3);

        fclose (fp);
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot save settings to the Gains Mixer text file."));
        return;
    }
}

}  // namespace Digikam
