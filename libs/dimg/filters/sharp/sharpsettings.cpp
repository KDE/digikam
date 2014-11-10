/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-10
 * Description : sharp settings view.
 *
 * Copyright (C) 2010-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "sharpsettings.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QStackedWidget>

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
#include <kseparator.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rexpanderbox.h>

// Local includes

#include "config-digikam.h"

#ifdef HAVE_EIGEN3
#include "refocusfilter.h"
#endif // HAVE_EIGEN3

using namespace KDcrawIface;

namespace Digikam
{

class SharpSettings::Private
{

public:

    Private() :
        stack(0),
        sharpMethod(0),
        radiusInput(0),
        radiusInput2(0),
        amountInput(0),
        thresholdInput(0)
#ifdef HAVE_EIGEN3
        ,radius(0),
        correlation(0),
        noise(0),
        gauss(0),
        matrixSize(0)
#endif // HAVE_EIGEN3
    {
    }

    static const QString configSharpenMethodEntry;
    static const QString configSimpleSharpRadiusAdjustmentEntry;
    static const QString configUnsharpMaskRadiusAdjustmentEntry;
    static const QString configUnsharpMaskAmountAdjustmentEntry;
    static const QString configUnsharpMaskThresholdAdjustmentEntry;
    static const QString configRefocusRadiusAdjustmentEntry;
    static const QString configRefocusCorrelationAdjustmentEntry;
    static const QString configRefocusNoiseAdjustmentEntry;
    static const QString configRefocusGaussAdjustmentEntry;
    static const QString configRefocusMatrixSizeEntry;

    QStackedWidget*      stack;

    RComboBox*           sharpMethod;

    // Simple sharp.
    RIntNumInput*        radiusInput;

    // Unsharp mask.
    RDoubleNumInput*     radiusInput2;
    RDoubleNumInput*     amountInput;
    RDoubleNumInput*     thresholdInput;

#ifdef HAVE_EIGEN3
    // Refocus.
    RDoubleNumInput*     radius;
    RDoubleNumInput*     correlation;
    RDoubleNumInput*     noise;
    RDoubleNumInput*     gauss;
    RIntNumInput*        matrixSize;
#endif // HAVE_EIGEN3
};

const QString SharpSettings::Private::configSharpenMethodEntry("SharpenMethod");
const QString SharpSettings::Private::configSimpleSharpRadiusAdjustmentEntry("SimpleSharpRadiusAdjustment");
const QString SharpSettings::Private::configUnsharpMaskRadiusAdjustmentEntry("UnsharpMaskRadiusAdjustment");
const QString SharpSettings::Private::configUnsharpMaskAmountAdjustmentEntry("UnsharpMaskAmountAdjustment");
const QString SharpSettings::Private::configUnsharpMaskThresholdAdjustmentEntry("UnsharpMaskThresholdAdjustment");
const QString SharpSettings::Private::configRefocusRadiusAdjustmentEntry("RefocusRadiusAdjustment");
const QString SharpSettings::Private::configRefocusCorrelationAdjustmentEntry("RefocusCorrelationAdjustment");
const QString SharpSettings::Private::configRefocusNoiseAdjustmentEntry("RefocusNoiseAdjustment");
const QString SharpSettings::Private::configRefocusGaussAdjustmentEntry("RefocusGaussAdjustment");
const QString SharpSettings::Private::configRefocusMatrixSizeEntry("RefocusMatrixSize");

// --------------------------------------------------------

SharpSettings::SharpSettings(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* const grid = new QGridLayout(parent);

    QLabel* const label1 = new QLabel(i18n("Method:"), parent);
    d->sharpMethod = new RComboBox(parent);
    d->sharpMethod->addItem(i18n("Simple sharp"));
    d->sharpMethod->addItem(i18n("Unsharp mask"));
#ifdef HAVE_EIGEN3
    d->sharpMethod->addItem(i18n("Refocus"));
#endif // HAVE_EIGEN3
    d->sharpMethod->setDefaultIndex(SharpContainer::SimpleSharp);
    d->sharpMethod->setWhatsThis(i18n("Select the sharpening method to apply to the image."));

    d->stack = new QStackedWidget(parent);

    grid->addWidget(label1,                 0, 0, 1, 1);
    grid->addWidget(d->sharpMethod,         0, 1, 1, 1);
    grid->addWidget(new KSeparator(parent), 1, 0, 1, 2);
    grid->addWidget(d->stack,               2, 0, 1, 2);
    grid->setRowStretch(3, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    QWidget* const simpleSharpSettings = new QWidget(d->stack);
    QGridLayout* const grid1           = new QGridLayout(simpleSharpSettings);

    QLabel* const label  = new QLabel(i18n("Sharpness:"), simpleSharpSettings);
    d->radiusInput = new RIntNumInput(simpleSharpSettings);
    d->radiusInput->setRange(0, 100, 1);
    d->radiusInput->setSliderEnabled(true);
    d->radiusInput->setDefaultValue(0);
    d->radiusInput->setWhatsThis(i18n("A sharpness of 0 has no effect, "
                                      "1 and above determine the sharpen matrix radius "
                                      "that determines how much to sharpen the image."));

    grid1->addWidget(label,          0, 0, 1, 2);
    grid1->addWidget(d->radiusInput, 1, 0, 1, 2);
    grid1->setRowStretch(2, 10);
    grid1->setMargin(0);
    grid1->setSpacing(0);

    d->stack->insertWidget(SharpContainer::SimpleSharp, simpleSharpSettings);

    // -------------------------------------------------------------

    QWidget* const unsharpMaskSettings = new QWidget(d->stack);
    QGridLayout* const grid2           = new QGridLayout(unsharpMaskSettings);

    QLabel* const label2  = new QLabel(i18n("Radius:"), unsharpMaskSettings);
    d->radiusInput2 = new RDoubleNumInput(unsharpMaskSettings);
    d->radiusInput2->setRange(0.1, 12.0, 0.1, true);
    d->radiusInput2->setDecimals(1);
    d->radiusInput2->setDefaultValue(1.0);
    d->radiusInput2->setWhatsThis(i18n("Radius value is the Gaussian blur matrix radius value "
                                       "used to determines how much to blur the image."));

    QLabel* const label3 = new QLabel(i18n("Amount:"), unsharpMaskSettings);
    d->amountInput = new RDoubleNumInput(unsharpMaskSettings);
    d->amountInput->setDecimals(1);
    d->amountInput->input()->setRange(0.0, 5.0, 0.1, true);
    d->amountInput->setDefaultValue(1.0);
    d->amountInput->setWhatsThis(i18n("The value of the difference between the "
                                      "original and the blur image that is added back into the original."));

    QLabel* const label4    = new QLabel(i18n("Threshold:"), unsharpMaskSettings);
    d->thresholdInput = new RDoubleNumInput(unsharpMaskSettings);
    d->thresholdInput->setDecimals(2);
    d->thresholdInput->input()->setRange(0.0, 1.0, 0.01, true);
    d->thresholdInput->setDefaultValue(0.05);
    d->thresholdInput->setWhatsThis(i18n("The threshold, as a fraction of the maximum "
                                         "luminosity value, needed to apply the difference amount."));

    grid2->addWidget(label2,            0, 0, 1, 2);
    grid2->addWidget(d->radiusInput2,   1, 0, 1, 2);
    grid2->addWidget(label3,            2, 0, 1, 2);
    grid2->addWidget(d->amountInput,    3, 0, 1, 2);
    grid2->addWidget(label4,            4, 0, 1, 2);
    grid2->addWidget(d->thresholdInput, 5, 0, 1, 2);
    grid2->setRowStretch(6, 10);
    grid2->setMargin(0);
    grid2->setSpacing(0);

    d->stack->insertWidget(SharpContainer::UnsharpMask, unsharpMaskSettings);

    // -------------------------------------------------------------

#ifdef HAVE_EIGEN3

    QWidget* const refocusSettings = new QWidget(d->stack);
    QGridLayout* const grid3       = new QGridLayout(refocusSettings);

    QLabel* const label5 = new QLabel(i18n("Circular sharpness:"), refocusSettings);
    d->radius            = new RDoubleNumInput(refocusSettings);
    d->radius->setDecimals(2);
    d->radius->input()->setRange(0.0, 5.0, 0.01, true);
    d->radius->setDefaultValue(1.0);
    d->radius->setWhatsThis(i18n("This is the radius of the circular convolution. It is the most important "
                                 "parameter for using this plugin. For most images the default value of 1.0 "
                                 "should give good results. Select a higher value when your image is very blurred."));

    QLabel* const label6 = new QLabel(i18n("Correlation:"), refocusSettings);
    d->correlation       = new RDoubleNumInput(refocusSettings);
    d->correlation->setDecimals(2);
    d->correlation->input()->setRange(0.0, 1.0, 0.01, true);
    d->correlation->setDefaultValue(0.5);
    d->correlation->setWhatsThis(i18n("Increasing the correlation may help to reduce artifacts. The correlation can "
                                      "range from 0-1. Useful values are 0.5 and values close to 1, e.g. 0.95 and 0.99. "
                                      "Using a high value for the correlation will reduce the sharpening effect of the "
                                      "plugin."));

    QLabel* const label7 = new QLabel(i18n("Noise filter:"), refocusSettings);
    d->noise             = new RDoubleNumInput(refocusSettings);
    d->noise->setDecimals(3);
    d->noise->input()->setRange(0.0, 1.0, 0.001, true);
    d->noise->setDefaultValue(0.03);
    d->noise->setWhatsThis(i18n("Increasing the noise filter parameter may help to reduce artifacts. The noise filter "
                                "can range from 0-1 but values higher than 0.1 are rarely helpful. When the noise filter "
                                "value is too low, e.g. 0.0 the image quality will be very poor. A useful value is 0.01. "
                                "Using a high value for the noise filter will reduce the sharpening "
                                "effect of the plugin."));

    QLabel* const label8 = new QLabel(i18n("Gaussian sharpness:"), refocusSettings);
    d->gauss             = new RDoubleNumInput(refocusSettings);
    d->gauss->setDecimals(2);
    d->gauss->input()->setRange(0.0, 1.0, 0.01, true);
    d->gauss->setDefaultValue(0.0);
    d->gauss->setWhatsThis(i18n("This is the sharpness for the Gaussian convolution. Use this parameter when your "
                                "blurring is of a Gaussian type. In most cases you should set this parameter to 0, because "
                                "it causes nasty artifacts. When you use non-zero values, you will probably also have to "
                                "increase the correlation and/or noise filter parameters."));

    QLabel* const label9 = new QLabel(i18n("Matrix size:"), refocusSettings);
    d->matrixSize        = new RIntNumInput(refocusSettings);
    d->matrixSize->setRange(0, RefocusFilter::maxMatrixSize(), 1);
    d->matrixSize->setSliderEnabled(true);
    d->matrixSize->setDefaultValue(5);
    d->matrixSize->setWhatsThis(i18n("This parameter determines the size of the transformation matrix. "
                                     "Increasing the matrix width may give better results, especially when you have "
                                     "chosen large values for circular or Gaussian sharpness."));

    grid3->addWidget(label5,         0, 0, 1, 2);
    grid3->addWidget(d->radius,      1, 0, 1, 2);
    grid3->addWidget(label6,         2, 0, 1, 2);
    grid3->addWidget(d->correlation, 3, 0, 1, 2);
    grid3->addWidget(label7,         4, 0, 1, 2);
    grid3->addWidget(d->noise,       5, 0, 1, 2);
    grid3->addWidget(label8,         6, 0, 1, 2);
    grid3->addWidget(d->gauss,       7, 0, 1, 2);
    grid3->addWidget(label9,         8, 0, 1, 2);
    grid3->addWidget(d->matrixSize,  9, 0, 1, 2);
    grid3->setRowStretch(10, 10);
    grid3->setMargin(0);
    grid3->setSpacing(0);

    d->stack->insertWidget(SharpContainer::Refocus, refocusSettings);
#endif // HAVE_EIGEN3

    // -------------------------------------------------------------

    connect(d->sharpMethod, SIGNAL(activated(int)),
            this, SLOT(slotSharpMethodChanged(int)));

    connect(d->radiusInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->radiusInput2, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->amountInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->thresholdInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

#ifdef HAVE_EIGEN3

    connect(d->radius, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->correlation, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->noise, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->gauss, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->matrixSize, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

#endif // HAVE_EIGEN3
}

SharpSettings::~SharpSettings()
{
    delete d;
}

void SharpSettings::slotSharpMethodChanged(int w)
{
    d->stack->setCurrentWidget(d->stack->widget(w));
    emit signalSettingsChanged();
}

SharpContainer SharpSettings::settings() const
{
    SharpContainer prm;

    prm.method        = d->sharpMethod->currentIndex();

    prm.ssRadius      = d->radiusInput->value();

    prm.umRadius      = d->radiusInput2->value();
    prm.umAmount      = d->amountInput->value();
    prm.umThreshold   = d->thresholdInput->value();

#ifdef HAVE_EIGEN3
    prm.rfRadius      = d->radius->value();
    prm.rfCorrelation = d->correlation->value();
    prm.rfNoise       = d->noise->value();
    prm.rfGauss       = d->gauss->value();
    prm.rfMatrix      = d->matrixSize->value();
#endif // HAVE_EIGEN3

    return prm;
}

void SharpSettings::setSettings(const SharpContainer& settings)
{
    blockSignals(true);

    d->sharpMethod->setCurrentIndex(settings.method);
    d->stack->setCurrentWidget(d->stack->widget(settings.method));

    d->radiusInput->setValue(settings.ssRadius);

    d->radiusInput2->setValue(settings.umRadius);
    d->amountInput->setValue(settings.umAmount);
    d->thresholdInput->setValue(settings.umThreshold);

#ifdef HAVE_EIGEN3
    d->radius->setValue(settings.rfRadius);
    d->correlation->setValue(settings.rfCorrelation);
    d->noise->setValue(settings.rfNoise);
    d->gauss->setValue(settings.rfGauss);
    d->matrixSize->setValue(settings.rfMatrix);
#endif // HAVE_EIGEN3

    blockSignals(false);
}

void SharpSettings::resetToDefault()
{
    blockSignals(true);

    d->sharpMethod->slotReset();
    d->stack->setCurrentWidget(d->stack->widget(d->sharpMethod->defaultIndex()));

    d->radiusInput->slotReset();

    d->radiusInput2->slotReset();
    d->amountInput->slotReset();
    d->thresholdInput->slotReset();

#ifdef HAVE_EIGEN3
    d->radius->slotReset();
    d->correlation->slotReset();
    d->noise->slotReset();
    d->gauss->slotReset();
    d->matrixSize->slotReset();
#endif // HAVE_EIGEN3

    blockSignals(false);
}

SharpContainer SharpSettings::defaultSettings() const
{
    SharpContainer prm;

    prm.method        = d->sharpMethod->defaultIndex();

    prm.ssRadius      = d->radiusInput->defaultValue();

    prm.umRadius      = d->radiusInput2->defaultValue();
    prm.umAmount      = d->amountInput->defaultValue();
    prm.umThreshold   = d->thresholdInput->defaultValue();

#ifdef HAVE_EIGEN3
    prm.rfRadius      = d->radius->defaultValue();
    prm.rfCorrelation = d->correlation->defaultValue();
    prm.rfNoise       = d->noise->defaultValue();
    prm.rfGauss       = d->gauss->defaultValue();
    prm.rfMatrix      = d->matrixSize->defaultValue();
#endif // HAVE_EIGEN3

    return prm;
}

void SharpSettings::readSettings(KConfigGroup& group)
{
    SharpContainer prm;
    SharpContainer defaultPrm = defaultSettings();

    prm.method        = group.readEntry(d->configSharpenMethodEntry,                  defaultPrm.method);

    prm.ssRadius      = group.readEntry(d->configSimpleSharpRadiusAdjustmentEntry,    defaultPrm.ssRadius);

    prm.umRadius      = group.readEntry(d->configUnsharpMaskRadiusAdjustmentEntry,    defaultPrm.umRadius);
    prm.umAmount      = group.readEntry(d->configUnsharpMaskAmountAdjustmentEntry,    defaultPrm.umAmount);
    prm.umThreshold   = group.readEntry(d->configUnsharpMaskThresholdAdjustmentEntry, defaultPrm.umThreshold);

#ifdef HAVE_EIGEN3
    prm.rfRadius      = group.readEntry(d->configRefocusRadiusAdjustmentEntry,        defaultPrm.rfRadius);
    prm.rfCorrelation = group.readEntry(d->configRefocusCorrelationAdjustmentEntry,   defaultPrm.rfCorrelation);
    prm.rfNoise       = group.readEntry(d->configRefocusNoiseAdjustmentEntry,         defaultPrm.rfNoise);
    prm.rfGauss       = group.readEntry(d->configRefocusGaussAdjustmentEntry,         defaultPrm.rfGauss);
    prm.rfMatrix      = group.readEntry(d->configRefocusMatrixSizeEntry,              defaultPrm.rfMatrix);
#endif // HAVE_EIGEN3

    setSettings(prm);
}

void SharpSettings::writeSettings(KConfigGroup& group)
{
    SharpContainer prm = settings();

    group.writeEntry(d->configSharpenMethodEntry,                  prm.method);

    group.writeEntry(d->configSimpleSharpRadiusAdjustmentEntry,    prm.ssRadius);

    group.writeEntry(d->configUnsharpMaskRadiusAdjustmentEntry,    prm.umRadius);
    group.writeEntry(d->configUnsharpMaskAmountAdjustmentEntry,    prm.umAmount);
    group.writeEntry(d->configUnsharpMaskThresholdAdjustmentEntry, prm.umThreshold);

#ifdef HAVE_EIGEN3
    group.writeEntry(d->configRefocusRadiusAdjustmentEntry,        prm.rfRadius);
    group.writeEntry(d->configRefocusCorrelationAdjustmentEntry,   prm.rfCorrelation);
    group.writeEntry(d->configRefocusNoiseAdjustmentEntry,         prm.rfNoise);
    group.writeEntry(d->configRefocusGaussAdjustmentEntry,         prm.rfGauss);
    group.writeEntry(d->configRefocusMatrixSizeEntry,              prm.rfMatrix);
#endif // HAVE_EIGEN3
}

void SharpSettings::loadSettings()
{
    KUrl loadRestorationFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                                       QString("*"), kapp->activeWindow(),
                                                       QString(i18n("Photograph Refocus Settings File to Load")));

    if (loadRestorationFile.isEmpty())
    {
        return;
    }

    QFile file(loadRestorationFile.toLocalFile());

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);

        if (stream.readLine() != "# Photograph Refocus Configuration File")
        {
            KMessageBox::error(kapp->activeWindow(),
                               i18n("\"%1\" is not a Photograph Refocus settings text file.",
                                    loadRestorationFile.fileName()));
            file.close();
            return;
        }

        blockSignals(true);
#ifdef HAVE_EIGEN3
        d->matrixSize->setValue(stream.readLine().toInt());
        d->radius->setValue(stream.readLine().toDouble());
        d->gauss->setValue(stream.readLine().toDouble());
        d->correlation->setValue(stream.readLine().toDouble());
        d->noise->setValue(stream.readLine().toDouble());
#endif // HAVE_EIGEN3
        blockSignals(false);
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Photograph Refocus text file."));
    }

    file.close();
}

void SharpSettings::saveAsSettings()
{
    KUrl saveRestorationFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                                       QString("*"), kapp->activeWindow(),
                                                       QString(i18n("Photograph Refocus Settings File to Save")));

    if (saveRestorationFile.isEmpty())
    {
        return;
    }

    QFile file(saveRestorationFile.toLocalFile());

    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << "# Photograph Refocus Configuration File\n";
#ifdef HAVE_EIGEN3
        stream << d->matrixSize->value() << "\n";
        stream << d->radius->value() << "\n";
        stream << d->gauss->value() << "\n";
        stream << d->correlation->value() << "\n";
        stream << d->noise->value() << "\n";
#endif // HAVE_EIGEN3
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Refocus text file."));
    }

    file.close();
}

}  // namespace Digikam
