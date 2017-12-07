/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-22
 * Description : noise reduction settings view.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "nrsettings.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QUrl>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dexpanderbox.h"
#include "dfiledialog.h"
#include "dnuminput.h"
#include "digikam_debug.h"

namespace Digikam
{

class NRSettings::Private
{
public:

    Private() :
        luminanceBox(0),
        chrominanceRedBox(0),
        chrominanceBlueBox(0),
        checkAutoEst(0),
        advExpanderBox(0),
        thrLumInput(0),
        softLumInput(0),
        thrCrInput(0),
        softCrInput(0),
        thrCbInput(0),
        softCbInput(0)
    {}

    static const QString configThrLumInputAdjustmentEntry;
    static const QString configSoftLumInputAdjustmentEntry;
    static const QString configThrCrInputAdjustmentEntry;
    static const QString configSoftCrInputAdjustmentEntry;
    static const QString configThrCbInputAdjustmentEntry;
    static const QString configSoftCbInputAdjustmentEntry;
    static const QString configCheckAutoEstimationEntry;

    QWidget*             luminanceBox;
    QWidget*             chrominanceRedBox;
    QWidget*             chrominanceBlueBox;

    QCheckBox*           checkAutoEst;

    DExpanderBox*        advExpanderBox;

    DDoubleNumInput*     thrLumInput;
    DDoubleNumInput*     softLumInput;
    DDoubleNumInput*     thrCrInput;
    DDoubleNumInput*     softCrInput;
    DDoubleNumInput*     thrCbInput;
    DDoubleNumInput*     softCbInput;
};

const QString NRSettings::Private::configThrLumInputAdjustmentEntry(QLatin1String("ThrLumAdjustment"));
const QString NRSettings::Private::configSoftLumInputAdjustmentEntry(QLatin1String("SoftLumAdjustment"));
const QString NRSettings::Private::configThrCrInputAdjustmentEntry(QLatin1String("ThrCrAdjustment"));
const QString NRSettings::Private::configSoftCrInputAdjustmentEntry(QLatin1String("SoftCrAdjustment"));
const QString NRSettings::Private::configThrCbInputAdjustmentEntry(QLatin1String("ThrCbAdjustment"));
const QString NRSettings::Private::configSoftCbInputAdjustmentEntry(QLatin1String("SoftCbAdjustment"));
const QString NRSettings::Private::configCheckAutoEstimationEntry(QLatin1String("AutoNRAdjustment"));

// --------------------------------------------------------

NRSettings::NRSettings(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const grid = new QGridLayout(parent);

    QString thHelp = i18n("<b>Threshold</b>: Adjusts the threshold for denoising of "
                          "the image in a range from 0.0 (none) to 10.0. "
                          "The threshold is the value below which everything is considered noise.");

    QString soHelp = i18n("<b>Softness</b>: This adjusts the softness of the thresholding "
                          "(soft as opposed to hard thresholding). The higher the softness "
                          "the more noise remains in the image.");

    // -------------------------------------------------------------

    d->checkAutoEst     = new QCheckBox(i18n("Estimate Noise"));
    d->checkAutoEst->setWhatsThis( i18n("Compute automatically all noise reduction settings by a parse of "
                                        "noise contained in image."));

    d->advExpanderBox   = new DExpanderBox;
    d->advExpanderBox->setObjectName(QLatin1String("Noise Reduction Settings Expander"));

    // -------------------------------------------------------------

    d->luminanceBox           = new QWidget(d->advExpanderBox);
    QGridLayout* const lumLay = new QGridLayout(d->luminanceBox);

    QLabel* const label3      = new QLabel(i18n("Threshold:"), d->luminanceBox);
    d->thrLumInput            = new DDoubleNumInput(d->luminanceBox);
    d->thrLumInput->setDecimals(2);
    d->thrLumInput->setRange(0.0, 10.0, 0.1);
    d->thrLumInput->setDefaultValue(1.2);
    d->thrLumInput->setWhatsThis(thHelp);

    QLabel* const label4      = new QLabel(i18n("Softness:"), d->luminanceBox);
    d->softLumInput           = new DDoubleNumInput(d->luminanceBox);
    d->softLumInput->setDecimals(1);
    d->softLumInput->setRange(0.0, 1.0, 0.1);
    d->softLumInput->setDefaultValue(0.9);
    d->softLumInput->setWhatsThis(soHelp);

    lumLay->addWidget(label3,          0, 0, 1, 1);
    lumLay->addWidget(d->thrLumInput,  0, 1, 1, 1);
    lumLay->addWidget(label4,          1, 0, 1, 1);
    lumLay->addWidget(d->softLumInput, 1, 1, 1, 1);
    lumLay->setRowStretch(2, 10);
    lumLay->setContentsMargins(spacing, spacing, spacing, spacing);
    lumLay->setSpacing(0);

    // -------------------------------------------------------------

    d->chrominanceRedBox       = new QWidget(d->advExpanderBox);
    QGridLayout* const cRedLay = new QGridLayout(d->chrominanceRedBox);

    QLabel* const label5       = new QLabel(i18n("Threshold:"), d->chrominanceRedBox);
    d->thrCrInput              = new DDoubleNumInput(d->chrominanceRedBox);
    d->thrCrInput->setDecimals(2);
    d->thrCrInput->setRange(0.0, 10.0, 0.1);
    d->thrCrInput->setDefaultValue(1.2);
    d->thrCrInput->setWhatsThis(thHelp);

    QLabel* const label6       = new QLabel(i18n("Softness:"), d->chrominanceRedBox);
    d->softCrInput             = new DDoubleNumInput(d->chrominanceRedBox);
    d->softCrInput->setDecimals(1);
    d->softCrInput->setRange(0.0, 1.0, 0.1);
    d->softCrInput->setDefaultValue(0.9);
    d->softCrInput->setWhatsThis(soHelp);

    cRedLay->addWidget(label5,         0, 0, 1, 1);
    cRedLay->addWidget(d->thrCrInput,  0, 1, 1, 1);
    cRedLay->addWidget(label6,         1, 0, 1, 1);
    cRedLay->addWidget(d->softCrInput, 1, 1, 1, 1);
    cRedLay->setRowStretch(2, 10);
    cRedLay->setContentsMargins(spacing, spacing, spacing, spacing);
    cRedLay->setSpacing(0);

    // -------------------------------------------------------------

    d->chrominanceBlueBox       = new QWidget(d->advExpanderBox);
    QGridLayout* const cBlueLay = new QGridLayout(d->chrominanceBlueBox);

    QLabel* const label7        = new QLabel(i18n("Threshold:"), d->chrominanceBlueBox);
    d->thrCbInput               = new DDoubleNumInput(d->chrominanceBlueBox);
    d->thrCbInput->setDecimals(2);
    d->thrCbInput->setRange(0.0, 10.0, 0.1);
    d->thrCbInput->setDefaultValue(1.2);
    d->thrCbInput->setWhatsThis(thHelp);

    QLabel* const label8        = new QLabel(i18n("Softness:"), d->chrominanceBlueBox);
    d->softCbInput              = new DDoubleNumInput(d->chrominanceBlueBox);
    d->softCbInput->setDecimals(1);
    d->softCbInput->setRange(0.0, 1.0, 0.1);
    d->softCbInput->setDefaultValue(0.9);
    d->softCbInput->setWhatsThis(soHelp);

    cBlueLay->addWidget(label7,         0, 0, 1, 1);
    cBlueLay->addWidget(d->thrCbInput,  0, 1, 1, 1);
    cBlueLay->addWidget(label8,         1, 0, 1, 1);
    cBlueLay->addWidget(d->softCbInput, 1, 1, 1, 1);
    cBlueLay->setRowStretch(2, 10);
    cBlueLay->setContentsMargins(spacing, spacing, spacing, spacing);
    cBlueLay->setSpacing(0);

    // -------------------------------------------------------------

    d->advExpanderBox->addItem(d->luminanceBox, QIcon(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/colors-luma.png"))),
                               i18n("Luminance"),
                               QLatin1String("Luminance"), true);
    d->advExpanderBox->addItem(d->chrominanceBlueBox, QIcon(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/colors-chromablue.png"))),
                               i18n("Chrominance Blue"),
                               QLatin1String("ChrominanceBlue"), true);
    d->advExpanderBox->addItem(d->chrominanceRedBox, QIcon(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/colors-chromared.png"))),
                               i18n("Chrominance Red"),
                               QLatin1String("ChrominanceRed"), true);
    d->advExpanderBox->addStretch();

    // -------------------------------------------------------------

    grid->addWidget(d->checkAutoEst,   0, 0, 1, 2);
    grid->addWidget(d->advExpanderBox, 1, 0, 1, 2);
    grid->setRowStretch(1, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    connect(d->checkAutoEst, SIGNAL(clicked(bool)),
            this, SLOT(slotDisableParameters(bool)));

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
}

NRSettings::~NRSettings()
{
    delete d;
}

void NRSettings::setEstimateNoise(bool b)
{
    d->checkAutoEst->setChecked(b);
}

bool NRSettings::estimateNoise() const
{
    return d->checkAutoEst->isChecked();
}

void NRSettings::slotDisableParameters(bool b)
{
    d->luminanceBox->setDisabled(b);
    d->chrominanceRedBox->setDisabled(b);
    d->chrominanceBlueBox->setDisabled(b);
    qApp->processEvents();

    if (b)
    {
        emit signalEstimateNoise();
    }
}

NRContainer NRSettings::settings() const
{
    NRContainer prm;

    prm.thresholds[0] = d->thrLumInput->value();
    prm.thresholds[1] = d->thrCbInput->value();
    prm.thresholds[2] = d->thrCrInput->value();
    prm.softness[0]   = 1.0 - d->softLumInput->value();
    prm.softness[1]   = 1.0 - d->softCbInput->value();
    prm.softness[2]   = 1.0 - d->softCrInput->value();

    return prm;
}

void NRSettings::setSettings(const NRContainer& settings)
{
    blockSignals(true);
    d->thrLumInput->setValue(settings.thresholds[0]);
    d->thrCbInput->setValue(settings.thresholds[1]);
    d->thrCrInput->setValue(settings.thresholds[2]);
    d->softLumInput->setValue(1.0 - settings.softness[0]);
    d->softCbInput->setValue(1.0 - settings.softness[1]);
    d->softCrInput->setValue(1.0 - settings.softness[2]);
    blockSignals(false);
}

void NRSettings::resetToDefault()
{
    blockSignals(true);
    d->checkAutoEst->setChecked(false);
    d->thrLumInput->slotReset();
    d->softLumInput->slotReset();
    d->thrCbInput->slotReset();
    d->softCbInput->slotReset();
    d->thrCrInput->slotReset();
    d->softCrInput->slotReset();
    blockSignals(false);
}

NRContainer NRSettings::defaultSettings() const
{
    NRContainer prm;

    prm.thresholds[0] = d->thrLumInput->defaultValue();
    prm.thresholds[1] = d->thrCbInput->defaultValue();
    prm.thresholds[2] = d->thrCrInput->defaultValue();
    prm.softness[0]   = 1.0 - d->softLumInput->defaultValue();
    prm.softness[1]   = 1.0 - d->softCbInput->defaultValue();
    prm.softness[2]   = 1.0 - d->softCrInput->defaultValue();

    return prm;
}

void NRSettings::readSettings(KConfigGroup& group)
{
    NRContainer prm;
    NRContainer defaultPrm = defaultSettings();
    prm.thresholds[0]      = group.readEntry(d->configThrLumInputAdjustmentEntry,  defaultPrm.thresholds[0]);
    prm.thresholds[1]      = group.readEntry(d->configThrCbInputAdjustmentEntry,   defaultPrm.thresholds[1]);
    prm.thresholds[2]      = group.readEntry(d->configThrCrInputAdjustmentEntry,   defaultPrm.thresholds[2]);
    prm.softness[0]        = group.readEntry(d->configSoftLumInputAdjustmentEntry, defaultPrm.softness[0]);
    prm.softness[1]        = group.readEntry(d->configSoftCbInputAdjustmentEntry,  defaultPrm.softness[1]);
    prm.softness[2]        = group.readEntry(d->configSoftCrInputAdjustmentEntry,  defaultPrm.softness[2]);

    bool b                 = group.readEntry(d->configCheckAutoEstimationEntry,    false);
    d->checkAutoEst->setChecked(b);
    slotDisableParameters(b);

    if (!b)
        setSettings(prm);
}

void NRSettings::writeSettings(KConfigGroup& group)
{
    NRContainer prm = settings();

    group.writeEntry(d->configThrLumInputAdjustmentEntry,  prm.thresholds[0]);
    group.writeEntry(d->configThrCbInputAdjustmentEntry,   prm.thresholds[1]);
    group.writeEntry(d->configThrCrInputAdjustmentEntry,   prm.thresholds[2]);
    group.writeEntry(d->configSoftLumInputAdjustmentEntry, prm.softness[0]);
    group.writeEntry(d->configSoftCbInputAdjustmentEntry,  prm.softness[1]);
    group.writeEntry(d->configSoftCrInputAdjustmentEntry,  prm.softness[2]);

    group.writeEntry(d->configCheckAutoEstimationEntry,    d->checkAutoEst->isChecked());
}

void NRSettings::loadSettings()
{
    QUrl loadRestorationFile = DFileDialog::getOpenFileUrl(qApp->activeWindow(), i18n("Photograph Noise Reduction Settings File to Load"),
                                                           QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                           QLatin1String("*"));

    if (loadRestorationFile.isEmpty())
    {
        return;
    }

    QFile file(loadRestorationFile.toLocalFile());

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);

        if (stream.readLine() != QLatin1String("# Photograph Wavelets Noise Reduction Configuration File V2"))
        {
            QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                                  i18n("\"%1\" is not a Photograph Noise Reduction settings text file.",
                                       loadRestorationFile.fileName()));
            file.close();
            return;
        }

        blockSignals(true);

        d->thrLumInput->setValue(stream.readLine().toDouble());
        d->softLumInput->setValue(stream.readLine().toDouble());
        d->thrCrInput->setValue(stream.readLine().toDouble());
        d->softCrInput->setValue(stream.readLine().toDouble());
        d->thrCbInput->setValue(stream.readLine().toDouble());
        d->softCbInput->setValue(stream.readLine().toDouble());

        blockSignals(false);
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot load settings from the Photograph Noise Reduction text file."));
    }

    file.close();
}

void NRSettings::saveAsSettings()
{
    QUrl saveRestorationFile = DFileDialog::getSaveFileUrl(qApp->activeWindow(), i18n("Photograph Noise Reduction Settings File to Save"),
                                                           QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                           QLatin1String("*"));

    if (saveRestorationFile.isEmpty())
    {
        return;
    }

    QFile file(saveRestorationFile.toLocalFile());

    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << QLatin1String("# Photograph Wavelets Noise Reduction Configuration File V2\n");
        stream << d->thrLumInput->value()  << QLatin1String("\n");
        stream << d->softLumInput->value() << QLatin1String("\n");
        stream << d->thrCrInput->value()   << QLatin1String("\n");
        stream << d->softCrInput->value()  << QLatin1String("\n");
        stream << d->thrCbInput->value()   << QLatin1String("\n");
        stream << d->softCbInput->value()  << QLatin1String("\n");
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot save settings to the Photograph Noise Reduction text file."));
    }

    file.close();
}

}  // namespace Digikam
