/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-10
 * Description : sharp settings view.
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

#include "sharpsettings.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QStackedWidget>
#include <QUrl>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dexpanderbox.h"
#include "dfiledialog.h"
#include "dlayoutbox.h"
#include "dnuminput.h"
#include "digikam_debug.h"
#include "digikam_config.h"
#include "dcombobox.h"

#ifdef HAVE_EIGEN3
#   include "refocusfilter.h"
#endif // HAVE_EIGEN3

namespace Digikam
{

class SharpSettings::Private
{

public:

    Private() :
        stack(0),
        sharpMethod(0),
        radiusInput(0),
#ifdef HAVE_EIGEN3
        radius(0),
        correlation(0),
        noise(0),
        gauss(0),
        matrixSize(0),
#endif // HAVE_EIGEN3
        radiusInput2(0),
        amountInput(0),
        thresholdInput(0),
        luma(0)
    {
    }

    static const QString configSharpenMethodEntry;
    static const QString configSimpleSharpRadiusAdjustmentEntry;
    static const QString configUnsharpMaskRadiusAdjustmentEntry;
    static const QString configUnsharpMaskAmountAdjustmentEntry;
    static const QString configUnsharpMaskThresholdAdjustmentEntry;
    static const QString configUnsharpMaskLumaEntry;
    static const QString configRefocusRadiusAdjustmentEntry;
    static const QString configRefocusCorrelationAdjustmentEntry;
    static const QString configRefocusNoiseAdjustmentEntry;
    static const QString configRefocusGaussAdjustmentEntry;
    static const QString configRefocusMatrixSizeEntry;

    QStackedWidget*      stack;

    DComboBox*           sharpMethod;

    // Simple sharp.
    DIntNumInput*        radiusInput;

#ifdef HAVE_EIGEN3
    // Refocus.
    DDoubleNumInput*     radius;
    DDoubleNumInput*     correlation;
    DDoubleNumInput*     noise;
    DDoubleNumInput*     gauss;
    DIntNumInput*        matrixSize;
#endif // HAVE_EIGEN3

    // Unsharp mask.
    DDoubleNumInput*     radiusInput2;
    DDoubleNumInput*     amountInput;
    DDoubleNumInput*     thresholdInput;
    QCheckBox*           luma;
};

const QString SharpSettings::Private::configSharpenMethodEntry(QLatin1String("SharpenMethod"));
const QString SharpSettings::Private::configSimpleSharpRadiusAdjustmentEntry(QLatin1String("SimpleSharpRadiusAdjustment"));
const QString SharpSettings::Private::configUnsharpMaskRadiusAdjustmentEntry(QLatin1String("UnsharpMaskRadiusAdjustment"));
const QString SharpSettings::Private::configUnsharpMaskAmountAdjustmentEntry(QLatin1String("UnsharpMaskAmountAdjustment"));
const QString SharpSettings::Private::configUnsharpMaskThresholdAdjustmentEntry(QLatin1String("UnsharpMaskThresholdAdjustment"));
const QString SharpSettings::Private::configUnsharpMaskLumaEntry(QLatin1String("UnsharpMaskApplyOnLumaOnLy"));
const QString SharpSettings::Private::configRefocusRadiusAdjustmentEntry(QLatin1String("RefocusRadiusAdjustment"));
const QString SharpSettings::Private::configRefocusCorrelationAdjustmentEntry(QLatin1String("RefocusCorrelationAdjustment"));
const QString SharpSettings::Private::configRefocusNoiseAdjustmentEntry(QLatin1String("RefocusNoiseAdjustment"));
const QString SharpSettings::Private::configRefocusGaussAdjustmentEntry(QLatin1String("RefocusGaussAdjustment"));
const QString SharpSettings::Private::configRefocusMatrixSizeEntry(QLatin1String("RefocusMatrixSize"));

// --------------------------------------------------------

SharpSettings::SharpSettings(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing       = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    QGridLayout* const grid = new QGridLayout(parent);

    QLabel* const label1 = new QLabel(i18n("Method:"));
    d->sharpMethod       = new DComboBox;
    d->sharpMethod->addItem(i18n("Simple sharp"));
    d->sharpMethod->addItem(i18n("Unsharp mask"));
#ifdef HAVE_EIGEN3
    d->sharpMethod->addItem(i18n("Refocus"));
#endif // HAVE_EIGEN3
    d->sharpMethod->setDefaultIndex(SharpContainer::SimpleSharp);
    d->sharpMethod->setWhatsThis(i18n("Select the sharpening method to apply to the image."));

    d->stack = new QStackedWidget;

    grid->addWidget(label1,                                  0, 0, 1, 1);
    grid->addWidget(d->sharpMethod,                          0, 1, 1, 1);
    grid->addWidget(new DLineWidget(Qt::Horizontal),         1, 0, 1, 2);
    grid->addWidget(d->stack,                                2, 0, 1, 2);
    grid->setRowStretch(3, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    QWidget* const simpleSharpSettings = new QWidget(d->stack);
    QGridLayout* const grid1           = new QGridLayout(simpleSharpSettings);

    QLabel* const label = new QLabel(i18n("Sharpness:"), simpleSharpSettings);
    d->radiusInput      = new DIntNumInput(simpleSharpSettings);
    d->radiusInput->setRange(0, 100, 1);
    d->radiusInput->setDefaultValue(0);
    d->radiusInput->setWhatsThis(i18n("A sharpness of 0 has no effect, "
                                      "1 and above determine the sharpen matrix radius "
                                      "that determines how much to sharpen the image."));

    grid1->addWidget(label,          0, 0, 1, 2);
    grid1->addWidget(d->radiusInput, 1, 0, 1, 2);
    grid1->setRowStretch(2, 10);
    grid1->setContentsMargins(QMargins());
    grid1->setSpacing(0);

    d->stack->insertWidget(SharpContainer::SimpleSharp, simpleSharpSettings);

    // -------------------------------------------------------------

    QWidget* const unsharpMaskSettings = new QWidget(d->stack);
    QGridLayout* const grid2           = new QGridLayout(unsharpMaskSettings);

    QLabel* const label2 = new QLabel(i18n("Radius:"), unsharpMaskSettings);
    d->radiusInput2      = new DDoubleNumInput(unsharpMaskSettings);
    d->radiusInput2->setRange(0.1, 12.0, 0.1);
    d->radiusInput2->setDecimals(1);
    d->radiusInput2->setDefaultValue(1.0);
    d->radiusInput2->setWhatsThis(i18n("Radius value is the Gaussian blur matrix radius value "
                                       "used to determines how much to blur the image."));

    QLabel* const label3 = new QLabel(i18n("Amount:"), unsharpMaskSettings);
    d->amountInput       = new DDoubleNumInput(unsharpMaskSettings);
    d->amountInput->setDecimals(1);
    d->amountInput->setRange(0.0, 5.0, 0.1);
    d->amountInput->setDefaultValue(1.0);
    d->amountInput->setWhatsThis(i18n("The value of the difference between the "
                                      "original and the blur image that is added back into the original."));

    QLabel* const label4 = new QLabel(i18n("Threshold:"), unsharpMaskSettings);
    d->thresholdInput    = new DDoubleNumInput(unsharpMaskSettings);
    d->thresholdInput->setDecimals(2);
    d->thresholdInput->setRange(0.0, 1.0, 0.01);
    d->thresholdInput->setDefaultValue(0.05);
    d->thresholdInput->setWhatsThis(i18n("The threshold, as a fraction of the maximum "
                                         "luminosity value, needed to apply the difference amount."));

    d->luma = new QCheckBox(unsharpMaskSettings);
    d->luma->setText(i18n("Suppress color noise."));
    d->luma->setCheckable(true);
    d->luma->setWhatsThis(i18n("An option to apply filter on luminosity channel only "
                               "used to suppress chroma noise amplification."));

    grid2->addWidget(label2,            0, 0, 1, 2);
    grid2->addWidget(d->radiusInput2,   1, 0, 1, 2);
    grid2->addWidget(label3,            2, 0, 1, 2);
    grid2->addWidget(d->amountInput,    3, 0, 1, 2);
    grid2->addWidget(label4,            4, 0, 1, 2);
    grid2->addWidget(d->thresholdInput, 5, 0, 1, 2);
    grid2->addWidget(d->luma,           6, 0, 1, 2);
    grid2->setRowStretch(7, 10);
    grid2->setContentsMargins(QMargins());
    grid2->setSpacing(0);

    d->stack->insertWidget(SharpContainer::UnsharpMask, unsharpMaskSettings);

    // -------------------------------------------------------------

#ifdef HAVE_EIGEN3

    QWidget* const refocusSettings = new QWidget(d->stack);
    QGridLayout* const grid3       = new QGridLayout(refocusSettings);

    QLabel* const label5 = new QLabel(i18n("Circular sharpness:"), refocusSettings);
    d->radius            = new DDoubleNumInput(refocusSettings);
    d->radius->setDecimals(2);
    d->radius->setRange(0.0, 5.0, 0.01);
    d->radius->setDefaultValue(1.0);
    d->radius->setWhatsThis(i18n("This is the radius of the circular convolution. It is the most important "
                                 "parameter for using this tool. For most images the default value of 1.0 "
                                 "should give good results. Select a higher value when your image is very blurred."));

    QLabel* const label6 = new QLabel(i18n("Correlation:"), refocusSettings);
    d->correlation       = new DDoubleNumInput(refocusSettings);
    d->correlation->setDecimals(2);
    d->correlation->setRange(0.0, 1.0, 0.01);
    d->correlation->setDefaultValue(0.5);
    d->correlation->setWhatsThis(i18n("Increasing the correlation may help to reduce artifacts. The correlation can "
                                      "range from 0-1. Useful values are 0.5 and values close to 1, e.g. 0.95 and 0.99. "
                                      "Using a high value for the correlation will reduce the sharpening effect of the "
                                      "tool."));

    QLabel* const label7 = new QLabel(i18n("Noise filter:"), refocusSettings);
    d->noise             = new DDoubleNumInput(refocusSettings);
    d->noise->setDecimals(3);
    d->noise->setRange(0.0, 1.0, 0.001);
    d->noise->setDefaultValue(0.03);
    d->noise->setWhatsThis(i18n("Increasing the noise filter parameter may help to reduce artifacts. The noise filter "
                                "can range from 0-1 but values higher than 0.1 are rarely helpful. When the noise filter "
                                "value is too low, e.g. 0.0 the image quality will be very poor. A useful value is 0.01. "
                                "Using a high value for the noise filter will reduce the sharpening "
                                "effect of the tool."));

    QLabel* const label8 = new QLabel(i18n("Gaussian sharpness:"), refocusSettings);
    d->gauss             = new DDoubleNumInput(refocusSettings);
    d->gauss->setDecimals(2);
    d->gauss->setRange(0.0, 1.0, 0.01);
    d->gauss->setDefaultValue(0.0);
    d->gauss->setWhatsThis(i18n("This is the sharpness for the Gaussian convolution. Use this parameter when your "
                                "blurring is of a Gaussian type. In most cases you should set this parameter to 0, because "
                                "it causes nasty artifacts. When you use non-zero values, you will probably also have to "
                                "increase the correlation and/or noise filter parameters."));

    QLabel* const label9 = new QLabel(i18n("Matrix size:"), refocusSettings);
    d->matrixSize        = new DIntNumInput(refocusSettings);
    d->matrixSize->setRange(0, RefocusFilter::maxMatrixSize(), 1);
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
    grid3->setContentsMargins(QMargins());
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

    connect(d->luma, SIGNAL(stateChanged(int)),
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
    prm.umLumaOnly    = d->luma->isChecked();


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
    d->luma->setChecked(settings.umLumaOnly);

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
    d->luma->setChecked(false);

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
    prm.umLumaOnly    = false;

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

    prm.method                = group.readEntry(d->configSharpenMethodEntry,                  defaultPrm.method);

    prm.ssRadius              = group.readEntry(d->configSimpleSharpRadiusAdjustmentEntry,    defaultPrm.ssRadius);

    prm.umRadius              = group.readEntry(d->configUnsharpMaskRadiusAdjustmentEntry,    defaultPrm.umRadius);
    prm.umAmount              = group.readEntry(d->configUnsharpMaskAmountAdjustmentEntry,    defaultPrm.umAmount);
    prm.umThreshold           = group.readEntry(d->configUnsharpMaskThresholdAdjustmentEntry, defaultPrm.umThreshold);
    prm.umLumaOnly            = group.readEntry(d->configUnsharpMaskLumaEntry,                defaultPrm.umLumaOnly);

#ifdef HAVE_EIGEN3
    prm.rfRadius              = group.readEntry(d->configRefocusRadiusAdjustmentEntry,        defaultPrm.rfRadius);
    prm.rfCorrelation         = group.readEntry(d->configRefocusCorrelationAdjustmentEntry,   defaultPrm.rfCorrelation);
    prm.rfNoise               = group.readEntry(d->configRefocusNoiseAdjustmentEntry,         defaultPrm.rfNoise);
    prm.rfGauss               = group.readEntry(d->configRefocusGaussAdjustmentEntry,         defaultPrm.rfGauss);
    prm.rfMatrix              = group.readEntry(d->configRefocusMatrixSizeEntry,              defaultPrm.rfMatrix);
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
    group.writeEntry(d->configUnsharpMaskLumaEntry,                prm.umLumaOnly);

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
    QUrl loadRestorationFile = DFileDialog::getOpenFileUrl(qApp->activeWindow(), i18n("Photograph Refocus Settings File to Load"),
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

        if (stream.readLine() != QLatin1String("# Photograph Refocus Configuration File"))
        {
            QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
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
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot load settings from the Photograph Refocus text file."));
    }

    file.close();
}

void SharpSettings::saveAsSettings()
{
    QUrl saveRestorationFile = DFileDialog::getSaveFileUrl(qApp->activeWindow(), i18n("Photograph Refocus Settings File to Save"),
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
        stream << QLatin1String("# Photograph Refocus Configuration File\n");

#ifdef HAVE_EIGEN3
        stream << d->matrixSize->value()  << QLatin1String("\n");
        stream << d->radius->value()      << QLatin1String("\n");
        stream << d->gauss->value()       << QLatin1String("\n");
        stream << d->correlation->value() << QLatin1String("\n");
        stream << d->noise->value()       << QLatin1String("\n");
#endif // HAVE_EIGEN3

    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot save settings to the Photograph Refocus text file."));
    }

    file.close();
}

}  // namespace Digikam
