/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2007-09-13
 * Description : Greycstoration settings widgets
 *
 * Copyright 2007 by Gilles Caulier
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

// Qt includes.

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qstring.h>
#include <qtooltip.h>
#include <qtabwidget.h>

// KDE includes.

#include <kdialog.h>
#include <klocale.h>
#include <knuminput.h>

// Local includes.

#include "greycstorationwidget.h"
#include "greycstorationwidget.moc"

namespace DigikamImagePlugins
{

class GreycstorationWidgetPriv
{

public:

    GreycstorationWidgetPriv()
    {
        amplitudeLabel     = 0;
        iterationLabel     = 0;
        sharpnessLabel     = 0;
        anisotropyLabel    = 0;
        alphaLabel         = 0;
        sigmaLabel         = 0;
        gaussianPrecLabel  = 0;
        dlLabel            = 0;
        daLabel            = 0;
        tileLabel          = 0;
        btileLabel         = 0;
        interpolationLabel = 0;
        fastApproxCBox     = 0;
        interpolationBox   = 0;
        amplitudeInput     = 0;
        sharpnessInput     = 0;
        anisotropyInput    = 0;
        alphaInput         = 0;
        sigmaInput         = 0;
        gaussianPrecInput  = 0;
        dlInput            = 0;
        daInput            = 0;
        iterationInput     = 0;
        tileInput          = 0;
        btileInput         = 0;
    }

    QLabel          *amplitudeLabel;
    QLabel          *iterationLabel;
    QLabel          *sharpnessLabel;
    QLabel          *anisotropyLabel;
    QLabel          *alphaLabel;
    QLabel          *sigmaLabel;
    QLabel          *gaussianPrecLabel;
    QLabel          *dlLabel;
    QLabel          *daLabel;
    QLabel          *tileLabel;
    QLabel          *btileLabel;
    QLabel          *interpolationLabel;

    QCheckBox       *fastApproxCBox;

    QComboBox       *interpolationBox;

    KDoubleNumInput *amplitudeInput;
    KDoubleNumInput *sharpnessInput;
    KDoubleNumInput *anisotropyInput;
    KDoubleNumInput *alphaInput;
    KDoubleNumInput *sigmaInput;
    KDoubleNumInput *gaussianPrecInput;
    KDoubleNumInput *dlInput;
    KDoubleNumInput *daInput;

    KIntNumInput    *iterationInput;
    KIntNumInput    *tileInput;
    KIntNumInput    *btileInput;    
};

GreycstorationWidget::GreycstorationWidget(QTabWidget *parent)
                    : QObject(static_cast<QObject*>(parent))
{
    d = new GreycstorationWidgetPriv;

    // -------------------------------------------------------------
    
    QWidget* smoothingPage = new QWidget( parent );
    QGridLayout* grid2     = new QGridLayout( smoothingPage, 6, 2, 
                                              KDialog::marginHint(), KDialog::spacingHint());
    parent->addTab( smoothingPage, i18n("Smoothing") );
    
    d->sharpnessLabel = new QLabel(i18n("Sharpness:"), smoothingPage);
    d->sharpnessInput = new KDoubleNumInput(smoothingPage);
    d->sharpnessInput->setPrecision(2);
    d->sharpnessInput->setRange(0.01, 1.0, 0.1, true);
    QWhatsThis::add( d->sharpnessInput, i18n("<p>Contour preservation of picture."));
    grid2->addMultiCellWidget(d->sharpnessLabel, 0, 0, 0, 0);
    grid2->addMultiCellWidget(d->sharpnessInput, 0, 0, 1, 1);

    d->anisotropyLabel = new QLabel(i18n("Anisotropy:"), smoothingPage);
    d->anisotropyInput = new KDoubleNumInput(smoothingPage);
    d->anisotropyInput->setPrecision(2);
    d->anisotropyInput->setRange(0.0, 1.0, 0.1, true);
    QWhatsThis::add( d->anisotropyInput, i18n("<p>Anisotropic (directional) modifier of the details. "
                                              "Keep it small for Gaussian noise."));
    grid2->addMultiCellWidget(d->anisotropyLabel, 1, 1, 0, 0);
    grid2->addMultiCellWidget(d->anisotropyInput, 1, 1, 1, 1);

    d->dlLabel = new QLabel(i18n("Smoothing:"), smoothingPage);
    d->dlInput = new KDoubleNumInput(smoothingPage);
    d->dlInput->setPrecision(2);
    d->dlInput->setRange(0.0, 1.0, 0.1, true);
    QWhatsThis::add( d->dlInput, i18n("<p>Total smoothing power: if Detail Factor sets the "
                                      "relative smoothing and Gradient Factor the "
                                      "direction, Time Step sets the overall effect."));
    grid2->addMultiCellWidget(d->dlLabel, 2, 2, 0, 0);
    grid2->addMultiCellWidget(d->dlInput, 2, 2, 1, 1);

    d->sigmaLabel = new QLabel(i18n("Sigma:"), smoothingPage);
    d->sigmaInput = new KDoubleNumInput(smoothingPage);
    d->sigmaInput->setPrecision(2);
    d->sigmaInput->setRange(0.0, 10.0, 0.1, true);
    QWhatsThis::add( d->sigmaInput, i18n("<p>This value controls the smoothing regularity of the picture. "
                                         "Do not use a high value here, or the "
                                         "target image will be completely blurred."));
    grid2->addMultiCellWidget(d->sigmaLabel, 3, 3, 0, 0);
    grid2->addMultiCellWidget(d->sigmaInput, 3, 3, 1, 1);
    
    d->iterationLabel = new QLabel(i18n("Iterations:"), smoothingPage);
    d->iterationInput = new KIntNumInput(smoothingPage);
    d->iterationInput->setRange(1, 5000, 1, true);
    QWhatsThis::add( d->iterationInput, i18n("<p>Sets the number of times the filter is applied over "
                                             "the picture."));
    grid2->addMultiCellWidget(d->iterationLabel, 4, 4, 0, 0);
    grid2->addMultiCellWidget(d->iterationInput, 4, 4, 1, 1);

    d->amplitudeLabel = new QLabel(i18n("Strength:"), smoothingPage);
    d->amplitudeInput = new KDoubleNumInput(smoothingPage);
    d->amplitudeInput->setPrecision(2);
    d->amplitudeInput->setRange(0.01, 100.0, 0.1, true);
    QWhatsThis::add( d->amplitudeInput, i18n("<p>Sets the Regularization strength used per iteration."));
    grid2->addMultiCellWidget(d->amplitudeLabel, 5, 5, 0, 0);
    grid2->addMultiCellWidget(d->amplitudeInput, 5, 5, 1, 1);

    d->alphaLabel = new QLabel(i18n("Alpha:"), smoothingPage);
    d->alphaInput = new KDoubleNumInput(smoothingPage);
    d->alphaInput->setPrecision(2);
    d->alphaInput->setRange(0.01, 1.0, 0.1, true);
    QWhatsThis::add( d->alphaInput, i18n("<p>Sets the noise scale."));
    grid2->addMultiCellWidget(d->alphaLabel, 6, 6, 0, 0);
    grid2->addMultiCellWidget(d->alphaInput, 6, 6, 1, 1);
    
    // -------------------------------------------------------------
    
    QWidget* advancedPage = new QWidget( parent );
    QGridLayout* grid3    = new QGridLayout( advancedPage, 6, 2, 
                                             KDialog::marginHint(), KDialog::spacingHint());
    parent->addTab( advancedPage, i18n("Advanced Settings") );
    
    d->daLabel = new QLabel(i18n("Angular step:"), advancedPage);
    d->daInput = new KDoubleNumInput(advancedPage);
    d->daInput->setPrecision(2);
    d->daInput->setRange(0.0, 90.0, 1.0, true);
    QWhatsThis::add( d->daInput, i18n("<p>Set here the angular integration step in degrees "
                                      "in analogy to anisotropy."));
    grid3->addMultiCellWidget(d->daLabel, 0, 0, 0, 0);
    grid3->addMultiCellWidget(d->daInput, 0, 0, 1, 1);

    d->dlLabel = new QLabel(i18n("Integral step:"), advancedPage);
    d->dlInput = new KDoubleNumInput(advancedPage);
    d->dlInput->setPrecision(2);
    d->dlInput->setRange(0.01, 1.0, 0.1, true);
    QWhatsThis::add( d->dlInput, i18n("<p>Set here the spatial integral step."));
    grid3->addMultiCellWidget(d->dlLabel, 1, 1, 0, 0);
    grid3->addMultiCellWidget(d->dlInput, 1, 1, 1, 1);

    d->gaussianPrecLabel = new QLabel(i18n("Gaussian:"), advancedPage);
    d->gaussianPrecInput = new KDoubleNumInput(advancedPage);
    d->gaussianPrecInput->setPrecision(2);
    d->gaussianPrecInput->setRange(0.01, 20.0, 0.01, true);
    QWhatsThis::add( d->gaussianPrecInput, i18n("<p>Set here the precision of the Gaussian function."));
    grid3->addMultiCellWidget(d->gaussianPrecLabel, 2, 2, 0, 0);
    grid3->addMultiCellWidget(d->gaussianPrecInput, 2, 2, 1, 1);

    d->tileLabel = new QLabel(i18n("Tile size:"), advancedPage);
    d->tileInput = new KIntNumInput(advancedPage);
    d->tileInput->setRange(0, 2000, 1, true);
    QWhatsThis::add( d->tileInput, i18n("<p>Sets the size of tile."));
    grid3->addMultiCellWidget(d->tileLabel, 3, 3, 0, 0);
    grid3->addMultiCellWidget(d->tileInput, 3, 3, 1, 1);

    d->btileLabel = new QLabel(i18n("Tile border:"), advancedPage);
    d->btileInput = new KIntNumInput(advancedPage);
    d->btileInput->setRange(1, 20, 1, true);
    QWhatsThis::add( d->btileInput, i18n("<p>Sets the size of tile borders."));
    grid3->addMultiCellWidget(d->btileLabel, 4, 4, 0, 0);
    grid3->addMultiCellWidget(d->btileInput, 4, 4, 1, 1);
    
    d->interpolationLabel = new QLabel(i18n("Interpolation:"), advancedPage);
    d->interpolationBox   = new QComboBox(false, advancedPage);
    d->interpolationBox->insertItem( i18n("Nearest Neighbor"), GreycstorationSettings::NearestNeighbor );
    d->interpolationBox->insertItem( i18n("Linear"),           GreycstorationSettings::Linear );
    d->interpolationBox->insertItem( i18n("Runge-Kutta"),      GreycstorationSettings::RungeKutta);
    QWhatsThis::add( d->interpolationBox, i18n("<p>Select the right interpolation method to set "
                                               "the picture quality."));
    grid3->addMultiCellWidget(d->interpolationLabel, 5, 5, 0, 0);
    grid3->addMultiCellWidget(d->interpolationBox, 5, 5, 1, 1);
    
    d->fastApproxCBox = new QCheckBox(i18n("Fast approximation"), advancedPage);
    QWhatsThis::add( d->fastApproxCBox, i18n("<p>Enable fast approximation to render picture."));
    grid3->addMultiCellWidget(d->fastApproxCBox, 6, 6, 0, 1);
}

GreycstorationWidget::~GreycstorationWidget()
{
    delete d;
}

void GreycstorationWidget::setSettings(GreycstorationSettings settings)
{
    blockSignals(true);
    d->fastApproxCBox->setChecked(settings.fastApprox);
    d->interpolationBox->setCurrentItem(settings.interp);
    d->amplitudeInput->setValue(settings.amplitude);
    d->sharpnessInput->setValue(settings.sharpness);
    d->anisotropyInput->setValue(settings.anisotropy);
    d->alphaInput->setValue(settings.alpha);
    d->sigmaInput->setValue(settings.sigma);
    d->gaussianPrecInput->setValue(settings.gaussPrec);
    d->dlInput->setValue(settings.dl);
    d->daInput->setValue(settings.da);
    d->iterationInput->setValue(settings.nbIter);
    d->tileInput->setValue(settings.tile);
    d->btileInput->setValue(settings.btile);
    blockSignals(false);    
}

GreycstorationSettings GreycstorationWidget::getSettings()
{
    GreycstorationSettings settings;

    settings.fastApprox = d->fastApproxCBox->isChecked();
    settings.interp     = d->interpolationBox->currentItem();
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

} // NameSpace DigikamImagePlugins
