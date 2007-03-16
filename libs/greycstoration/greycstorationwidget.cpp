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
#include <qtooltip.h>
#include <qtabwidget.h>
#include <qtextstream.h>

// KDE includes.

#include <kdialog.h>
#include <klocale.h>
#include <knuminput.h>

// Local includes.

#include "greycstorationwidget.h"
#include "greycstorationwidget.moc"

namespace Digikam
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
        advancedPage       = 0;
        generalPage        = 0;
        parent             = 0;
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

    QWidget         *advancedPage;
    QWidget         *generalPage;

    QCheckBox       *fastApproxCBox;

    QComboBox       *interpolationBox;

    QTabWidget      *parent;

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
    d->parent = parent;

    // -------------------------------------------------------------
    
    d->generalPage     = new QWidget( parent );
    QGridLayout* grid1 = new QGridLayout(d->generalPage, 6, 2, KDialog::spacingHint());
    parent->addTab( d->generalPage, i18n("General") );
    
    d->sharpnessLabel = new QLabel(i18n("Detail preservation:"), d->generalPage);
    d->sharpnessInput = new KDoubleNumInput(d->generalPage);
    d->sharpnessInput->setPrecision(2);
    d->sharpnessInput->setRange(0.01, 1.0, 0.1, true);
    QWhatsThis::add( d->sharpnessInput, i18n("<p>Preservation of details to set the sharpening level "
                                             "of the small features in the target image. "
                                             "Higher values leave details sharp."));
    grid1->addMultiCellWidget(d->sharpnessLabel, 0, 0, 0, 0);
    grid1->addMultiCellWidget(d->sharpnessInput, 0, 0, 1, 1);

    d->anisotropyLabel = new QLabel(i18n("Anisotropy:"), d->generalPage);
    d->anisotropyInput = new KDoubleNumInput(d->generalPage);
    d->anisotropyInput->setPrecision(2);
    d->anisotropyInput->setRange(0.0, 1.0, 0.1, true);
    QWhatsThis::add( d->anisotropyInput, i18n("<p>Anisotropic (directional) modifier of the details. "
                                              "Keep it small for Gaussian noise."));
    grid1->addMultiCellWidget(d->anisotropyLabel, 1, 1, 0, 0);
    grid1->addMultiCellWidget(d->anisotropyInput, 1, 1, 1, 1);

    d->amplitudeLabel = new QLabel(i18n("Smoothing:"), d->generalPage);
    d->amplitudeInput = new KDoubleNumInput(d->generalPage);
    d->amplitudeInput->setPrecision(2);
    d->amplitudeInput->setRange(0.01, 500.0, 0.1, true);
    QWhatsThis::add( d->amplitudeInput, i18n("<p>Total smoothing power: if Detail Factor sets the relative "
                                             "smoothing and Anisotropy Factor the direction, "
                                             "Smoothing Factor sets the overall effect."));
    grid1->addMultiCellWidget(d->amplitudeLabel, 2, 2, 0, 0);
    grid1->addMultiCellWidget(d->amplitudeInput, 2, 2, 1, 1);

    d->sigmaLabel = new QLabel(i18n("Regularity:"), d->generalPage);
    d->sigmaInput = new KDoubleNumInput(d->generalPage);
    d->sigmaInput->setPrecision(2);
    d->sigmaInput->setRange(0.0, 10.0, 0.1, true);
    QWhatsThis::add( d->sigmaInput, i18n("<p>This value controls the smoothing regularity of the picture. "
                                         "Do not use a high value here, or the "
                                         "target image will be completely blurred."));
    grid1->addMultiCellWidget(d->sigmaLabel, 3, 3, 0, 0);
    grid1->addMultiCellWidget(d->sigmaInput, 3, 3, 1, 1);
    
    d->iterationLabel = new QLabel(i18n("Iterations:"), d->generalPage);
    d->iterationInput = new KIntNumInput(d->generalPage);
    d->iterationInput->setRange(1, 5000, 1, true);
    QWhatsThis::add( d->iterationInput, i18n("<p>Sets the number of times the filter is applied over "
                                             "the picture."));
    grid1->addMultiCellWidget(d->iterationLabel, 4, 4, 0, 0);
    grid1->addMultiCellWidget(d->iterationInput, 4, 4, 1, 1);

    d->alphaLabel = new QLabel(i18n("Noise:"), d->generalPage);
    d->alphaInput = new KDoubleNumInput(d->generalPage);
    d->alphaInput->setPrecision(2);
    d->alphaInput->setRange(0.01, 1.0, 0.1, true);
    QWhatsThis::add( d->alphaInput, i18n("<p>Sets the noise scale."));
    grid1->addMultiCellWidget(d->alphaLabel, 5, 5, 0, 0);
    grid1->addMultiCellWidget(d->alphaInput, 5, 5, 1, 1);
    grid1->setRowStretch(6, 10);
    
    // -------------------------------------------------------------
    
    d->advancedPage    = new QWidget( parent );
    QGridLayout* grid2 = new QGridLayout(d->advancedPage, 6, 2, KDialog::spacingHint());
    parent->addTab( d->advancedPage, i18n("Advanced Settings") );
    
    d->daLabel = new QLabel(i18n("Angular step:"), d->advancedPage);
    d->daInput = new KDoubleNumInput(d->advancedPage);
    d->daInput->setPrecision(2);
    d->daInput->setRange(0.0, 90.0, 1.0, true);
    QWhatsThis::add( d->daInput, i18n("<p>Set here the angular integration step in degrees "
                                      "in analogy to anisotropy."));
    grid2->addMultiCellWidget(d->daLabel, 0, 0, 0, 0);
    grid2->addMultiCellWidget(d->daInput, 0, 0, 1, 1);

    d->dlLabel = new QLabel(i18n("Integral step:"), d->advancedPage);
    d->dlInput = new KDoubleNumInput(d->advancedPage);
    d->dlInput->setPrecision(2);
    d->dlInput->setRange(0.0, 1.0, 0.1, true);
    QWhatsThis::add( d->dlInput, i18n("<p>Set here the spatial integral step."));
    grid2->addMultiCellWidget(d->dlLabel, 1, 1, 0, 0);
    grid2->addMultiCellWidget(d->dlInput, 1, 1, 1, 1);

    d->gaussianPrecLabel = new QLabel(i18n("Gaussian:"), d->advancedPage);
    d->gaussianPrecInput = new KDoubleNumInput(d->advancedPage);
    d->gaussianPrecInput->setPrecision(2);
    d->gaussianPrecInput->setRange(0.01, 20.0, 0.01, true);
    QWhatsThis::add( d->gaussianPrecInput, i18n("<p>Set here the precision of the Gaussian function."));
    grid2->addMultiCellWidget(d->gaussianPrecLabel, 2, 2, 0, 0);
    grid2->addMultiCellWidget(d->gaussianPrecInput, 2, 2, 1, 1);

    d->tileLabel = new QLabel(i18n("Tile size:"), d->advancedPage);
    d->tileInput = new KIntNumInput(d->advancedPage);
    d->tileInput->setRange(0, 2000, 1, true);
    QWhatsThis::add( d->tileInput, i18n("<p>Sets the size of tile."));
    grid2->addMultiCellWidget(d->tileLabel, 3, 3, 0, 0);
    grid2->addMultiCellWidget(d->tileInput, 3, 3, 1, 1);

    d->btileLabel = new QLabel(i18n("Tile border:"), d->advancedPage);
    d->btileInput = new KIntNumInput(d->advancedPage);
    d->btileInput->setRange(1, 20, 1, true);
    QWhatsThis::add( d->btileInput, i18n("<p>Sets the size of tile borders."));
    grid2->addMultiCellWidget(d->btileLabel, 4, 4, 0, 0);
    grid2->addMultiCellWidget(d->btileInput, 4, 4, 1, 1);
    
    d->interpolationLabel = new QLabel(i18n("Interpolation:"), d->advancedPage);
    d->interpolationBox   = new QComboBox(false, d->advancedPage);
    d->interpolationBox->insertItem( i18n("Nearest Neighbor"), GreycstorationSettings::NearestNeighbor );
    d->interpolationBox->insertItem( i18n("Linear"),           GreycstorationSettings::Linear );
    d->interpolationBox->insertItem( i18n("Runge-Kutta"),      GreycstorationSettings::RungeKutta);
    QWhatsThis::add( d->interpolationBox, i18n("<p>Select the right interpolation method to set "
                                               "the picture quality."));
    grid2->addMultiCellWidget(d->interpolationLabel, 5, 5, 0, 0);
    grid2->addMultiCellWidget(d->interpolationBox, 5, 5, 1, 1);
    
    d->fastApproxCBox = new QCheckBox(i18n("Fast approximation"), d->advancedPage);
    QWhatsThis::add( d->fastApproxCBox, i18n("<p>Enable fast approximation to render picture."));
    grid2->addMultiCellWidget(d->fastApproxCBox, 6, 6, 0, 1);
}

GreycstorationWidget::~GreycstorationWidget()
{
    delete d;
}

void GreycstorationWidget::setEnabled(bool b)
{
    d->generalPage->setEnabled(b);
    d->advancedPage->setEnabled(b);
    d->parent->setTabEnabled(d->generalPage, b);
    d->parent->setTabEnabled(d->advancedPage, b);
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

bool GreycstorationWidget::loadSettings(QFile& file, const QString& header)
{
    QTextStream stream( &file );
    
    if (stream.readLine() != header)
        return false;
    
    blockSignals(true);

    GreycstorationSettings settings;
    settings.fastApprox = stream.readLine().toInt();
    settings.interp     = stream.readLine().toInt();
    settings.amplitude  = stream.readLine().toDouble();
    settings.sharpness  = stream.readLine().toDouble();
    settings.anisotropy = stream.readLine().toDouble();
    settings.alpha      = stream.readLine().toDouble();
    settings.sigma      = stream.readLine().toDouble();
    settings.gaussPrec  = stream.readLine().toDouble();
    settings.dl         = stream.readLine().toDouble();
    settings.da         = stream.readLine().toDouble();
    settings.nbIter     = stream.readLine().toInt();
    settings.tile       = stream.readLine().toInt();
    settings.btile      = stream.readLine().toInt();   
    setSettings(settings);

    blockSignals(false);
    return true;
}

void GreycstorationWidget::saveSettings(QFile& file, const QString& header)
{
    GreycstorationSettings settings = getSettings();
    QTextStream stream( &file );        
    stream << header << "\n";    
    stream << settings.fastApprox << "\n";    
    stream << settings.interp << "\n";    
    stream << settings.amplitude << "\n";    
    stream << settings.sharpness << "\n";    
    stream << settings.anisotropy << "\n";    
    stream << settings.alpha << "\n";    
    stream << settings.sigma << "\n";    
    stream << settings.gaussPrec << "\n";    
    stream << settings.dl << "\n";    
    stream << settings.da << "\n";    
    stream << settings.nbIter << "\n";    
    stream << settings.tile << "\n";    
    stream << settings.btile << "\n";    
}

} // NameSpace Digikam
