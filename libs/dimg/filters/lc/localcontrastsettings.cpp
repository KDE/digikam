/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-09
 * Description : Local Contrast settings view.
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

#include "localcontrastsettings.moc"

// Qt includes

#include <QString>
#include <QButtonGroup>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegExp>
#include <QTextStream>
#include <QToolButton>
#include <QVBoxLayout>

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
#include <kcombobox.h>
#include <kseparator.h>
#include <kiconloader.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>

// Local includes

#include "rexpanderbox.h"

using namespace KDcrawIface;

namespace Digikam
{

class LocalContrastSettingsPriv
{

public:

    LocalContrastSettingsPriv() :
        configDarkInputEntry("Dark"),
        configBlackInputEntry("Black"),
        configMainExposureEntry("MainExposure"),
        configFineExposureEntry("FineExposure"),
        configGammaInputEntry("Gamma"),
        configSaturationInputEntry("Saturation"),
        configGreenInputEntry("Green"),
        configTemperatureInputEntry("Temperature"),
        pickTemperature(0),
        autoAdjustExposure(0),
        adjTemperatureLabel(0),
        temperaturePresetLabel(0),
        darkLabel(0),
        blackLabel(0),
        mainExposureLabel(0),
        fineExposureLabel(0),
        gammaLabel(0),
        saturationLabel(0),
        greenLabel(0),
        exposureLabel(0),
        temperatureLabel(0),
        temperaturePresetCB(0),
        temperatureInput(0),
        darkInput(0),
        blackInput(0),
        mainExposureInput(0),
        fineExposureInput(0),
        gammaInput(0),
        saturationInput(0),
        greenInput(0)
        {}

    const QString    configDarkInputEntry;
    const QString    configBlackInputEntry;
    const QString    configMainExposureEntry;
    const QString    configFineExposureEntry;
    const QString    configGammaInputEntry;
    const QString    configSaturationInputEntry;
    const QString    configGreenInputEntry;
    const QString    configTemperatureInputEntry;

    QToolButton*     pickTemperature;
    QToolButton*     autoAdjustExposure;

    QLabel*          adjTemperatureLabel;
    QLabel*          temperaturePresetLabel;
    QLabel*          darkLabel;
    QLabel*          blackLabel;
    QLabel*          mainExposureLabel;
    QLabel*          fineExposureLabel;
    QLabel*          gammaLabel;
    QLabel*          saturationLabel;
    QLabel*          greenLabel;
    QLabel*          exposureLabel;
    QLabel*          temperatureLabel;

    RComboBox*       temperaturePresetCB;

    RDoubleNumInput* temperatureInput;
    RDoubleNumInput* darkInput;
    RDoubleNumInput* blackInput;
    RDoubleNumInput* mainExposureInput;
    RDoubleNumInput* fineExposureInput;
    RDoubleNumInput* gammaInput;
    RDoubleNumInput* saturationInput;
    RDoubleNumInput* greenInput;

};

LocalContrastSettings::LocalContrastSettings(QWidget* parent)
                     : QWidget(parent),
                       d(new LocalContrastSettingsPriv)
{
    QGridLayout* grid = new QGridLayout(parent);

    d->temperatureLabel = new QLabel(i18n("<a href='http://en.wikipedia.org/wiki/Color_temperature'>"
                                          "Color Temperature</a> (K): "));
    d->temperatureLabel->setOpenExternalLinks(true);

    d->adjTemperatureLabel = new QLabel(i18n("Adjustment:"));
    d->temperatureInput    = new RDoubleNumInput;
    d->temperatureInput->setDecimals(1);
    d->temperatureInput->input()->setRange(1750.0, 12000.0, 10.0);
    d->temperatureInput->setWhatsThis( i18n("Set here the white balance color temperature in Kelvin."));

    d->temperaturePresetLabel = new QLabel(i18n("Preset:"));
    d->temperaturePresetCB    = new RComboBox;

    QString toolTip = QString("<p>%1</p>").arg(i18n("Select the white balance color temperature preset to use."));
    d->temperaturePresetCB->setToolTip(toolTip);

    d->pickTemperature = new QToolButton;
    d->pickTemperature->setIcon(KIcon("color-picker-grey"));
    d->pickTemperature->setCheckable(true);
    d->pickTemperature->setToolTip( i18n( "Temperature tone color picker." ) );
    d->pickTemperature->setWhatsThis(i18n("With this button, you can pick the color from the original "
                                          "image used to set the white color balance temperature and "
                                          "green component."));

    KSeparator* line = new KSeparator(Qt::Horizontal);

    // -------------------------------------------------------------

    d->blackLabel = new QLabel(i18n("Black point:"));
    d->blackInput = new RDoubleNumInput;
    d->blackInput->setDecimals(2);
    d->blackInput->input()->setRange(0.0, 0.05, 0.01, true);
    d->blackInput->setWhatsThis( i18n("Set here the black level value."));
    d->blackInput->setDefaultValue(0.0);

    d->darkLabel = new QLabel(i18n("Shadows:"));
    d->darkInput = new RDoubleNumInput;
    d->darkInput->setDecimals(2);
    d->darkInput->input()->setRange(0.0, 1.0, 0.01, true);
    d->darkInput->setDefaultValue(0.5);
    d->darkInput->setWhatsThis( i18n("Set here the shadow noise suppression level."));

    d->saturationLabel = new QLabel(i18n("Saturation:"));
    d->saturationInput = new RDoubleNumInput;
    d->saturationInput->setDecimals(2);
    d->saturationInput->input()->setRange(0.0, 2.0, 0.01, true);
    d->saturationInput->setDefaultValue(1.0);
    d->saturationInput->setWhatsThis( i18n("Set here the saturation value."));

    d->gammaLabel = new QLabel(i18n("Gamma:"));
    d->gammaInput = new RDoubleNumInput;
    d->gammaInput->setDecimals(2);
    d->gammaInput->input()->setRange(0.1, 3.0, 0.01, true);
    d->gammaInput->setDefaultValue(1.0);
    d->gammaInput->setWhatsThis( i18n("Set here the gamma correction value."));

    d->greenLabel = new QLabel(i18n("Green:"));
    d->greenInput = new RDoubleNumInput;
    d->greenInput->setDecimals(2);
    d->greenInput->input()->setRange(0.2, 2.5, 0.01, true);
    d->greenInput->setDefaultValue(1.0);
    d->greenInput->setWhatsThis(i18n("Set here the green component to control the magenta color "
                                     "cast removal level."));

    KSeparator* line2 = new KSeparator(Qt::Horizontal);

    // -------------------------------------------------------------

    d->exposureLabel = new QLabel(i18n("<a href='http://en.wikipedia.org/wiki/Exposure_value'>"
                                       "Exposure Compensation</a> (E.V): "));
    d->exposureLabel->setOpenExternalLinks(true);

    d->mainExposureLabel  = new QLabel(i18nc("main exposure value", "Main:"));
    d->autoAdjustExposure = new QToolButton;
    d->autoAdjustExposure->setIcon(KIconLoader::global()->loadIcon("system-run", KIconLoader::Toolbar));
    d->autoAdjustExposure->setToolTip( i18n( "Auto exposure adjustments" ) );
    d->autoAdjustExposure->setWhatsThis(i18n("With this button, you can automatically adjust Exposure "
                                             "and Black Point values."));
    d->mainExposureInput = new RDoubleNumInput;
    d->mainExposureInput->setDecimals(2);
    d->mainExposureInput->input()->setRange(-6.0, 8.0, 0.1, true);
    d->mainExposureInput->setDefaultValue(0.0);
    d->mainExposureInput->setWhatsThis( i18n("Set here the main exposure compensation value in E.V."));

    d->fineExposureLabel = new QLabel(i18nc("fine exposure adjustment", "Fine:"));
    d->fineExposureInput = new RDoubleNumInput;
    d->fineExposureInput->setDecimals(2);
    d->fineExposureInput->input()->setRange(-0.5, 0.5, 0.01, true);
    d->fineExposureInput->setDefaultValue(0.0);
    d->fineExposureInput->setWhatsThis(i18n("This value in E.V will be added to main exposure "
                                            "compensation value to set fine exposure adjustment."));

    // -------------------------------------------------------------

    grid->addWidget(d->temperatureLabel,        0, 0, 1, 6);
    grid->addWidget(d->adjTemperatureLabel,     1, 0, 1, 1);
    grid->addWidget(d->pickTemperature,         1, 1, 1, 1);
    grid->addWidget(d->temperatureInput,        1, 2, 1, 4);
    grid->addWidget(d->temperaturePresetLabel,  2, 0, 1, 1);
    grid->addWidget(d->temperaturePresetCB,     2, 2, 1, 4);
    grid->addWidget(line,                       3, 0, 1, 6);
    grid->addWidget(d->blackLabel,              4, 0, 1, 1);
    grid->addWidget(d->blackInput,              4, 1, 1, 5);
    grid->addWidget(d->darkLabel,               5, 0, 1, 1);
    grid->addWidget(d->darkInput,               5, 1, 1, 5);
    grid->addWidget(d->saturationLabel,         6, 0, 1, 1);
    grid->addWidget(d->saturationInput,         6, 1, 1, 5);
    grid->addWidget(d->gammaLabel,              7, 0, 1, 1);
    grid->addWidget(d->gammaInput,              7, 1, 1, 5);
    grid->addWidget(d->greenLabel,              8, 0, 1, 1);
    grid->addWidget(d->greenInput,              8, 1, 1, 5);
    grid->addWidget(line2,                      9, 0, 1, 6);
    grid->addWidget(d->exposureLabel,          10, 0, 1, 6);
    grid->addWidget(d->mainExposureLabel,      11, 0, 1, 1);
    grid->addWidget(d->autoAdjustExposure,     11, 1, 1, 1);
    grid->addWidget(d->mainExposureInput,      11, 2, 1, 4);
    grid->addWidget(d->fineExposureLabel,      12, 0, 1, 2);
    grid->addWidget(d->fineExposureInput,      12, 2, 1, 4);
    grid->setRowStretch(13, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    connect(d->temperaturePresetCB, SIGNAL(activated(int)),
            this, SLOT(slotTemperaturePresetChanged(int)));

    connect(d->temperatureInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTemperatureChanged(double)));

    connect(d->darkInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->blackInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->mainExposureInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->fineExposureInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->gammaInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->saturationInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->greenInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->autoAdjustExposure, SIGNAL(clicked()),
            this, SIGNAL(signalAutoAdjustExposure()));

    connect(d->pickTemperature, SIGNAL(released()),
            this, SIGNAL(signalPickerColorButtonActived()));
}

LocalContrastSettings::~LocalContrastSettings()
{
    delete d;
}

ToneMappingParameters LocalContrastSettings::settings() const
{
    ToneMappingParameters prm;
/*
    prm.black       = d->blackInput->value();
    prm.exposition  = d->mainExposureInput->value() + d->fineExposureInput->value();
    prm.temperature = d->temperatureInput->value();
    prm.green       = d->greenInput->value();
    prm.dark        = d->darkInput->value();
    prm.gamma       = d->gammaInput->value();
    prm.saturation  = d->saturationInput->value();
*/
    return prm;
}

void LocalContrastSettings::setSettings(const ToneMappingParameters& settings)
{
    blockSignals(true);
/*
    d->blackInput->setValue(settings.black);
    d->mainExposureInput->setValue(double(int(settings.exposition)));
    d->fineExposureInput->setValue(settings.exposition - d->mainExposureInput->value());
    d->temperatureInput->setValue(settings.temperature);
    d->greenInput->setValue(settings.green);
    d->darkInput->setValue(settings.dark);
    d->gammaInput->setValue(settings.gamma);
    d->saturationInput->setValue(settings.saturation);
*/
    blockSignals(false);
}

void LocalContrastSettings::resetToDefault()
{
    blockSignals(true);
/*
    d->blackInput->slotReset();
    d->darkInput->slotReset();
    d->fineExposureInput->slotReset();
    d->gammaInput->slotReset();
    d->greenInput->slotReset();
    d->mainExposureInput->slotReset();
    d->saturationInput->slotReset();
    d->temperatureInput->slotReset();
    d->temperaturePresetCB->slotReset();
*/
    blockSignals(false);
}

ToneMappingParameters LocalContrastSettings::defaultSettings() const
{
    ToneMappingParameters prm;
/*
    prm.black       = d->blackInput->defaultValue();
    prm.exposition  = d->mainExposureInput->defaultValue() + d->fineExposureInput->defaultValue();
    prm.temperature = d->temperatureInput->defaultValue();
    prm.green       = d->greenInput->defaultValue();
    prm.dark        = d->darkInput->defaultValue();
    prm.gamma       = d->gammaInput->defaultValue();
    prm.saturation  = d->saturationInput->defaultValue();
*/
    return prm;
}

void LocalContrastSettings::readSettings(KConfigGroup& group)
{
    ToneMappingParameters prm;
    ToneMappingParameters defaultPrm = defaultSettings();
/*
    prm.black       = group.readEntry(d->configBlackInputEntry,       d->blackInput->defaultValue());
    prm.temperature = group.readEntry(d->configTemperatureInputEntry, d->temperatureInput->defaultValue());
    prm.green       = group.readEntry(d->configGreenInputEntry,       d->greenInput->defaultValue());
    prm.dark        = group.readEntry(d->configDarkInputEntry,        d->darkInput->defaultValue());
    prm.gamma       = group.readEntry(d->configGammaInputEntry,       d->gammaInput->defaultValue());
    prm.saturation  = group.readEntry(d->configSaturationInputEntry,  d->saturationInput->defaultValue());
    double fineExpo = group.readEntry(d->configFineExposureEntry,     d->fineExposureInput->defaultValue());
    prm.exposition  = group.readEntry(d->configMainExposureEntry,     d->mainExposureInput->defaultValue()) + fineExpo;
*/
    setSettings(prm);
}

void LocalContrastSettings::writeSettings(KConfigGroup& group)
{
    ToneMappingParameters prm = settings();
/*
    group.writeEntry(d->configDarkInputEntry,        d->darkInput->value());
    group.writeEntry(d->configBlackInputEntry,       d->blackInput->value());
    group.writeEntry(d->configMainExposureEntry,     d->mainExposureInput->value());
    group.writeEntry(d->configFineExposureEntry,     d->fineExposureInput->value());
    group.writeEntry(d->configGammaInputEntry,       d->gammaInput->value());
    group.writeEntry(d->configSaturationInputEntry,  d->saturationInput->value());
    group.writeEntry(d->configGreenInputEntry,       d->greenInput->value());
    group.writeEntry(d->configTemperatureInputEntry, d->temperatureInput->value());
*/
}

void LocalContrastSettings::loadSettings()
{
/*
    KUrl loadWhiteBalanceFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), kapp->activeWindow(),
                                             QString(i18n("White Color Balance Settings File to Load")));
    if (loadWhiteBalanceFile.isEmpty())
        return;

    QFile file(loadWhiteBalanceFile.toLocalFile());

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);

        if (stream.readLine() != "# White Color Balance Configuration File V2")
        {
            KMessageBox::error(kapp->activeWindow(),
                               i18n("\"%1\" is not a White Color Balance settings text file.",
                               loadWhiteBalanceFile.fileName()));
            file.close();
            return;
        }

        blockSignals(true);
        d->temperatureInput->setValue(stream.readLine().toDouble());
        d->darkInput->setValue(stream.readLine().toDouble());
        d->blackInput->setValue(stream.readLine().toDouble());
        d->mainExposureInput->setValue(stream.readLine().toDouble());
        d->fineExposureInput->setValue(stream.readLine().toDouble());
        d->gammaInput->setValue(stream.readLine().toDouble());
        d->saturationInput->setValue(stream.readLine().toDouble());
        d->greenInput->setValue(stream.readLine().toDouble());
        slotTemperatureChanged(d->temperatureInput->value());
        blockSignals(false);
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot load settings from the White Color Balance text file."));
    }

    file.close();
*/
}

void LocalContrastSettings::saveAsSettings()
{
/*
    KUrl saveWhiteBalanceFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), kapp->activeWindow(),
                                             QString( i18n("White Color Balance Settings File to Save")));
    if ( saveWhiteBalanceFile.isEmpty() )
       return;

    QFile file(saveWhiteBalanceFile.toLocalFile());

    if ( file.open(QIODevice::WriteOnly) )
    {
        QTextStream stream( &file );
        stream << "# White Color Balance Configuration File V2\n";
        stream << d->temperatureInput->value() << "\n";
        stream << d->darkInput->value() << "\n";
        stream << d->blackInput->value() << "\n";
        stream << d->mainExposureInput->value() << "\n";
        stream << d->fineExposureInput->value() << "\n";
        stream << d->gammaInput->value() << "\n";
        stream << d->saturationInput->value() << "\n";
        stream << d->greenInput->value() << "\n";
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot save settings to the White Color Balance text file."));
    }

    file.close();
*/
}

}  // namespace Digikam
