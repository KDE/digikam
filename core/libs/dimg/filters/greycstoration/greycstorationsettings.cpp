/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-13
 * Description : Greycstoration settings widgets
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "greycstorationsettings.h"

// Qt includes

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QTextStream>
#include <QToolTip>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "dcombobox.h"

namespace Digikam
{

class GreycstorationSettings::GreycstorationSettingsPriv
{

public:

    GreycstorationSettingsPriv() :
        alphaLabel(0),
        amplitudeLabel(0),
        anisotropyLabel(0),
        btileLabel(0),
        daLabel(0),
        dlLabel(0),
        gaussianPrecLabel(0),
        interpolationLabel(0),
        iterationLabel(0),
        sharpnessLabel(0),
        sigmaLabel(0),
        tileLabel(0),
        advancedPage(0),
        generalPage(0),
        fastApproxCBox(0),
        parent(0),
        interpolationBox(0),
        alphaInput(0),
        amplitudeInput(0),
        anisotropyInput(0),
        daInput(0),
        dlInput(0),
        gaussianPrecInput(0),
        sharpnessInput(0),
        sigmaInput(0),
        btileInput(0),
        iterationInput(0),
        tileInput(0)
    {
    }

    QLabel*          alphaLabel;
    QLabel*          amplitudeLabel;
    QLabel*          anisotropyLabel;
    QLabel*          btileLabel;
    QLabel*          daLabel;
    QLabel*          dlLabel;
    QLabel*          gaussianPrecLabel;
    QLabel*          interpolationLabel;
    QLabel*          iterationLabel;
    QLabel*          sharpnessLabel;
    QLabel*          sigmaLabel;
    QLabel*          tileLabel;

    QWidget*         advancedPage;
    QWidget*         generalPage;

    QCheckBox*       fastApproxCBox;

    QTabWidget*      parent;

    DComboBox*       interpolationBox;

    DDoubleNumInput* alphaInput;
    DDoubleNumInput* amplitudeInput;
    DDoubleNumInput* anisotropyInput;
    DDoubleNumInput* daInput;
    DDoubleNumInput* dlInput;
    DDoubleNumInput* gaussianPrecInput;
    DDoubleNumInput* sharpnessInput;
    DDoubleNumInput* sigmaInput;

    DIntNumInput*    btileInput;
    DIntNumInput*    iterationInput;
    DIntNumInput*    tileInput;
};

GreycstorationSettings::GreycstorationSettings(QTabWidget* parent)
    : QObject(static_cast<QObject*>(parent)),
      d(new GreycstorationSettingsPriv)
{
    d->parent = parent;

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    // -------------------------------------------------------------

    d->generalPage     = new QWidget(parent);
    QGridLayout* grid1 = new QGridLayout(d->generalPage);
    parent->addTab(d->generalPage, i18n("General"));

    d->sharpnessLabel = new QLabel(i18n("Detail preservation:"), d->generalPage);
    d->sharpnessInput = new DDoubleNumInput(d->generalPage);
    d->sharpnessInput->setDecimals(2);
    d->sharpnessInput->setRange(0.01, 1.0, 0.1);
    d->sharpnessInput->setWhatsThis(i18n("Preservation of details to set the sharpening level "
                                         "of the small features in the target image. "
                                         "Higher values leave details sharp."));

    d->anisotropyLabel = new QLabel(i18n("Anisotropy:"), d->generalPage);
    d->anisotropyInput = new DDoubleNumInput(d->generalPage);
    d->anisotropyInput->setDecimals(2);
    d->anisotropyInput->setRange(0.0, 1.0, 0.1);
    d->anisotropyInput->setWhatsThis(i18n("Anisotropic (directional) modifier of the details. "
                                          "Keep it small for Gaussian noise."));

    d->amplitudeLabel = new QLabel(i18n("Smoothing:"), d->generalPage);
    d->amplitudeInput = new DDoubleNumInput(d->generalPage);
    d->amplitudeInput->setDecimals(2);
    d->amplitudeInput->setRange(0.01, 500.0, 0.1);
    d->amplitudeInput->setWhatsThis(i18n("Total smoothing power: if the Detail Factor sets the relative "
                                         "smoothing and the Anisotropy Factor the direction, "
                                         "the Smoothing Factor sets the overall effect."));

    d->sigmaLabel = new QLabel(i18n("Regularity:"), d->generalPage);
    d->sigmaInput = new DDoubleNumInput(d->generalPage);
    d->sigmaInput->setDecimals(2);
    d->sigmaInput->setRange(0.0, 10.0, 0.1);
    d->sigmaInput->setWhatsThis(i18n("This value controls the evenness of smoothing to the image. "
                                     "Do not use a high value here, or the "
                                     "target image will be completely blurred."));

    d->iterationLabel = new QLabel(i18n("Iterations:"), d->generalPage);
    d->iterationInput = new DIntNumInput(d->generalPage);
    d->iterationInput->setRange(1, 5000, 1);
    d->iterationInput->setWhatsThis(i18n("Sets the number of times the filter is applied to the image."));

    d->alphaLabel = new QLabel(i18n("Noise:"), d->generalPage);
    d->alphaInput = new DDoubleNumInput(d->generalPage);
    d->alphaInput->setDecimals(2);
    d->alphaInput->setRange(0.01, 1.0, 0.1);
    d->alphaInput->setWhatsThis(i18n("Sets the noise scale."));

    grid1->addWidget(d->sharpnessLabel,     0, 0, 1, 1);
    grid1->addWidget(d->sharpnessInput,     0, 1, 1, 1);
    grid1->addWidget(d->anisotropyLabel,    1, 0, 1, 1);
    grid1->addWidget(d->anisotropyInput,    1, 1, 1, 1);
    grid1->addWidget(d->amplitudeLabel,     2, 0, 1, 1);
    grid1->addWidget(d->amplitudeInput,     2, 1, 1, 1);
    grid1->addWidget(d->sigmaLabel,         3, 0, 1, 1);
    grid1->addWidget(d->sigmaInput,         3, 1, 1, 1);
    grid1->addWidget(d->iterationLabel,     4, 0, 1, 1);
    grid1->addWidget(d->iterationInput,     4, 1, 1, 1);
    grid1->addWidget(d->alphaLabel,         5, 0, 1, 1);
    grid1->addWidget(d->alphaInput,         5, 1, 1, 1);
    grid1->setRowStretch(6, 10);
    grid1->setContentsMargins(spacing, spacing, spacing, spacing);
    grid1->setSpacing(spacing);

    // -------------------------------------------------------------

    d->advancedPage    = new QWidget(parent);
    QGridLayout* grid2 = new QGridLayout(d->advancedPage);
    parent->addTab(d->advancedPage, i18n("Advanced Settings"));

    d->daLabel = new QLabel(i18n("Angular step:"), d->advancedPage);
    d->daInput = new DDoubleNumInput(d->advancedPage);
    d->daInput->setDecimals(2);
    d->daInput->setRange(0.0, 90.0, 1.0);
    d->daInput->setWhatsThis(i18n("Set here the angular integration step (in degrees) "
                                  "analogous to anisotropy."));

    d->dlLabel = new QLabel(i18n("Integral step:"), d->advancedPage);
    d->dlInput = new DDoubleNumInput(d->advancedPage);
    d->dlInput->setDecimals(2);
    d->dlInput->setRange(0.0, 1.0, 0.1);
    d->dlInput->setWhatsThis(i18n("Set here the spatial integral step."));

    d->gaussianPrecLabel = new QLabel(i18n("Gaussian:"), d->advancedPage);
    d->gaussianPrecInput = new DDoubleNumInput(d->advancedPage);
    d->gaussianPrecInput->setDecimals(2);
    d->gaussianPrecInput->setRange(0.01, 20.0, 0.01);
    d->gaussianPrecInput->setWhatsThis(i18n("Set here the precision of the Gaussian function."));

    d->tileLabel = new QLabel(i18n("Tile size:"), d->advancedPage);
    d->tileInput = new DIntNumInput(d->advancedPage);
    d->tileInput->setRange(0, 2000, 1);
    d->tileInput->setWhatsThis(i18n("Sets the tile size."));

    d->btileLabel = new QLabel(i18n("Tile border:"), d->advancedPage);
    d->btileInput = new DIntNumInput(d->advancedPage);
    d->btileInput->setRange(1, 20, 1);
    d->btileInput->setWhatsThis(i18n("Sets the size of each tile border."));

    d->interpolationLabel = new QLabel(i18n("Interpolation:"), d->advancedPage);
    d->interpolationBox   = new DComboBox(d->advancedPage);
    d->interpolationBox->insertItem(GreycstorationContainer::NearestNeighbor, i18n("Nearest Neighbor"));
    d->interpolationBox->insertItem(GreycstorationContainer::Linear, i18n("Linear"));
    d->interpolationBox->insertItem(GreycstorationContainer::RungeKutta, i18n("Runge-Kutta"));
    d->interpolationBox->setWhatsThis(i18n("Select the right interpolation method for the "
                                           "desired image quality."));

    d->fastApproxCBox = new QCheckBox(i18n("Fast approximation"), d->advancedPage);
    d->fastApproxCBox->setWhatsThis(i18n("Enable fast approximation when rendering images."));

    grid2->addWidget(d->daLabel,            0, 0, 1, 1);
    grid2->addWidget(d->daInput,            0, 1, 1, 1);
    grid2->addWidget(d->dlLabel,            1, 0, 1, 1);
    grid2->addWidget(d->dlInput,            1, 1, 1, 1);
    grid2->addWidget(d->gaussianPrecLabel,  2, 0, 1, 1);
    grid2->addWidget(d->gaussianPrecInput,  2, 1, 1, 1);
    grid2->addWidget(d->tileLabel,          3, 0, 1, 1);
    grid2->addWidget(d->tileInput,          3, 1, 1, 1);
    grid2->addWidget(d->btileLabel,         4, 0, 1, 1);
    grid2->addWidget(d->btileInput,         4, 1, 1, 1);
    grid2->addWidget(d->interpolationLabel, 5, 0, 1, 1);
    grid2->addWidget(d->interpolationBox,   5, 1, 1, 1);
    grid2->addWidget(d->fastApproxCBox,     6, 0, 1, 2);
    grid2->setContentsMargins(spacing, spacing, spacing, spacing);
    grid2->setSpacing(spacing);
}

GreycstorationSettings::~GreycstorationSettings()
{
    delete d;
}

void GreycstorationSettings::setEnabled(bool b)
{
    d->generalPage->setEnabled(b);
    d->advancedPage->setEnabled(b);
    d->parent->setTabEnabled(d->parent->indexOf(d->generalPage), b);
    d->parent->setTabEnabled(d->parent->indexOf(d->advancedPage), b);
}

void GreycstorationSettings::setSettings(const GreycstorationContainer& settings)
{
    blockSignals(true);
    d->alphaInput->setValue(settings.alpha);
    d->amplitudeInput->setValue(settings.amplitude);
    d->anisotropyInput->setValue(settings.anisotropy);
    d->btileInput->setValue(settings.btile);
    d->daInput->setValue(settings.da);
    d->dlInput->setValue(settings.dl);
    d->fastApproxCBox->setChecked(settings.fastApprox);
    d->gaussianPrecInput->setValue(settings.gaussPrec);
    d->interpolationBox->setCurrentIndex(settings.interp);
    d->iterationInput->setValue(settings.nbIter);
    d->sharpnessInput->setValue(settings.sharpness);
    d->sigmaInput->setValue(settings.sigma);
    d->tileInput->setValue(settings.tile);
    blockSignals(false);
}

void GreycstorationSettings::setDefaultSettings(const GreycstorationContainer& settings)
{
    blockSignals(true);
    d->alphaInput->setDefaultValue(settings.alpha);
    d->amplitudeInput->setDefaultValue(settings.amplitude);
    d->anisotropyInput->setDefaultValue(settings.anisotropy);
    d->btileInput->setDefaultValue(settings.btile);
    d->daInput->setDefaultValue(settings.da);
    d->dlInput->setDefaultValue(settings.dl);
    d->fastApproxCBox->setChecked(settings.fastApprox);
    d->gaussianPrecInput->setDefaultValue(settings.gaussPrec);
    d->interpolationBox->setDefaultIndex(settings.interp);
    d->iterationInput->setDefaultValue(settings.nbIter);
    d->sharpnessInput->setDefaultValue(settings.sharpness);
    d->sigmaInput->setDefaultValue(settings.sigma);
    d->tileInput->setDefaultValue(settings.tile);
    blockSignals(false);
}

GreycstorationContainer GreycstorationSettings::settings() const
{
    GreycstorationContainer settings;

    settings.fastApprox = d->fastApproxCBox->isChecked();
    settings.interp     = d->interpolationBox->currentIndex();
    settings.amplitude  = d->amplitudeInput->value();
    settings.sharpness  = d->sharpnessInput->value();
    settings.anisotropy = d->anisotropyInput->value();
    settings.alpha      = d->alphaInput->value();
    settings.sigma      = d->sigmaInput->value();
    settings.gaussPrec  = d->gaussianPrecInput->value();
    settings.dl         = d->dlInput->value();
    settings.da         = d->daInput->value();
    settings.nbIter     = d->iterationInput->value();
    settings.tile       = d->tileInput->value();
    settings.btile      = d->btileInput->value();

    return settings;
}

bool GreycstorationSettings::loadSettings(QFile& file, const QString& header)
{
    QTextStream stream(&file);

    if (stream.readLine() != header)
    {
        return false;
    }

    blockSignals(true);

    GreycstorationContainer prm;
    prm.fastApprox = stream.readLine().toInt();
    prm.interp     = stream.readLine().toInt();
    prm.amplitude  = stream.readLine().toDouble();
    prm.sharpness  = stream.readLine().toDouble();
    prm.anisotropy = stream.readLine().toDouble();
    prm.alpha      = stream.readLine().toDouble();
    prm.sigma      = stream.readLine().toDouble();
    prm.gaussPrec  = stream.readLine().toDouble();
    prm.dl         = stream.readLine().toDouble();
    prm.da         = stream.readLine().toDouble();
    prm.nbIter     = stream.readLine().toInt();
    prm.tile       = stream.readLine().toInt();
    prm.btile      = stream.readLine().toInt();
    setSettings(prm);

    blockSignals(false);
    return true;
}

void GreycstorationSettings::saveSettings(QFile& file, const QString& header)
{
    GreycstorationContainer prm = settings();
    QTextStream stream(&file);
    stream << header << "\n";
    stream << prm.fastApprox << "\n";
    stream << prm.interp << "\n";
    stream << prm.amplitude << "\n";
    stream << prm.sharpness << "\n";
    stream << prm.anisotropy << "\n";
    stream << prm.alpha << "\n";
    stream << prm.sigma << "\n";
    stream << prm.gaussPrec << "\n";
    stream << prm.dl << "\n";
    stream << prm.da << "\n";
    stream << prm.nbIter << "\n";
    stream << prm.tile << "\n";
    stream << prm.btile << "\n";
}

} // namespace Digikam
