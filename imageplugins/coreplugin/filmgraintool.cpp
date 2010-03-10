/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor plugin for add film
 *               grain on an image.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "filmgraintool.moc"

// Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLabel>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "dimg.h"
#include "editortoolsettings.h"
#include "filmgrainfilter.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "rexpanderbox.h"

using namespace KDcrawIface;

namespace DigikamImagesPluginCore
{

class FilmGrainToolPriv
{
public:

    FilmGrainToolPriv() :
        configGroupName("filmgrain Tool"),
        configSensitivityAdjustmentEntry("SensitivityAdjustment"),
        sensibilityLumInput(0),
        sensibilityChromaInput(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString       configGroupName;
    const QString       configSensitivityAdjustmentEntry;

    RIntNumInput*       sensibilityLumInput;
    RIntNumInput*       shadowsLumInput; 
    RIntNumInput*       midtonesLumInput;
    RIntNumInput*       highlightsLumInput;
    RIntNumInput*       sensibilityChromaInput;
    RIntNumInput*       shadowsChromaInput; 
    RIntNumInput*       midtonesChromaInput;
    RIntNumInput*       highlightsChromaInput;
    
    RExpanderBox*       expanderBox;
    
    ImageRegionWidget*  previewWidget;
    EditorToolSettings* gboxSettings;
};

FilmGrainTool::FilmGrainTool(QObject* parent)
             : EditorToolThreaded(parent),
               d(new FilmGrainToolPriv)
{
    setObjectName("filmgrain");
    setToolName(i18n("Film Grain"));
    setToolIcon(SmallIcon("filmgrain"));

    // -------------------------------------------------------------

    d->gboxSettings  = new EditorToolSettings;
    d->previewWidget = new ImageRegionWidget;
    
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Try);

    // -------------------------------------------------------------
    
    QWidget* firstPage = new QWidget();
    QGridLayout* grid1 = new QGridLayout(firstPage);
    
    // -------------------------------------------------------------
    
    QLabel* label1         = new QLabel(i18n("Sensitivity (ISO):"), firstPage);
    d->sensibilityLumInput = new RIntNumInput(firstPage);
    d->sensibilityLumInput->setRange(800, 51200, 100);
    d->sensibilityLumInput->setSliderEnabled(true);
    d->sensibilityLumInput->setDefaultValue(2400);
    d->sensibilityLumInput->setWhatsThis(i18n("Set here the film ISO-sensitivity to use for "
                                              "simulating the film graininess."));
  
    // -------------------------------------------------------------

    QLabel* label2        = new QLabel(i18n("Shadows:"), firstPage);
    d->highlightsLumInput = new RIntNumInput(firstPage);
    d->highlightsLumInput->setRange(-100, 100, 1);
    d->highlightsLumInput->setSliderEnabled(true);
    d->highlightsLumInput->setDefaultValue(-100);
    d->highlightsLumInput->setWhatsThis(i18n("Set how much the filter affects highlights."));
    
    // -------------------------------------------------------------

    QLabel* label3      = new QLabel(i18n("Midtones:"), firstPage);
    d->midtonesLumInput = new RIntNumInput(firstPage);
    d->midtonesLumInput->setRange(-100, 100, 1);
    d->midtonesLumInput->setSliderEnabled(true);
    d->midtonesLumInput->setDefaultValue(0);
    d->midtonesLumInput->setWhatsThis(i18n("Set how much the filter affects midtones."));
    
 
    // -------------------------------------------------------------

    QLabel* label4     = new QLabel(i18n("Highlights:"), firstPage);
    d->shadowsLumInput = new RIntNumInput(firstPage);
    d->shadowsLumInput->setRange(-100, 100, 1);
    d->shadowsLumInput->setSliderEnabled(true);
    d->shadowsLumInput->setDefaultValue(-100);
    d->shadowsLumInput->setWhatsThis(i18n("Set how much the filter affects shadows."));
  
    grid1->addWidget(label1,                    1, 0, 1, 1);
    grid1->addWidget(d->sensibilityLumInput,    2, 0, 1, 1);
    grid1->addWidget(label2,                    3, 0, 1, 1);
    grid1->addWidget(d->shadowsLumInput,        4, 0, 1, 1);  
    grid1->addWidget(label3,                    5, 0, 1, 1);
    grid1->addWidget(d->midtonesLumInput,       6, 0, 1, 1); 
    grid1->addWidget(label4,                    7, 0, 1, 1); 
    grid1->addWidget(d->highlightsLumInput,     8, 0, 1, 1);
    grid1->setMargin(d->gboxSettings->spacingHint());
    grid1->setSpacing(d->gboxSettings->spacingHint());
    
    // -------------------------------------------------------------

    QWidget* secondPage = new QWidget();
    QGridLayout* grid2  = new QGridLayout( secondPage );

    QLabel* label5            = new QLabel(i18n("Sensitivity (ISO):"), secondPage);
    d->sensibilityChromaInput = new RIntNumInput(secondPage);
    d->sensibilityChromaInput->setRange(800, 51200, 100);
    d->sensibilityChromaInput->setSliderEnabled(true);
    d->sensibilityChromaInput->setDefaultValue(2400);
    d->sensibilityChromaInput->setWhatsThis(i18n("Set here the film ISO-sensitivity to use for "
                                                 "simulating the CCD noise."));
  
    // -------------------------------------------------------------

    QLabel* label6           = new QLabel(i18n("Shadows:"), secondPage);
    d->highlightsChromaInput = new RIntNumInput(secondPage);
    d->highlightsChromaInput->setRange(-100, 100, 1);
    d->highlightsChromaInput->setSliderEnabled(true);
    d->highlightsChromaInput->setDefaultValue(-100);
    d->highlightsChromaInput->setWhatsThis(i18n("Set how much the filter affects highlights."));
    
    // -------------------------------------------------------------
    
    QLabel* label7         = new QLabel(i18n("Midtones:"), secondPage);
    d->midtonesChromaInput = new RIntNumInput(secondPage);
    d->midtonesChromaInput->setRange(-100, 100, 1);
    d->midtonesChromaInput->setSliderEnabled(true);
    d->midtonesChromaInput->setDefaultValue(0);
    d->midtonesChromaInput->setWhatsThis(i18n("Set how much the filter affects midtones."));
    
 
    // -------------------------------------------------------------

    QLabel* label8        = new QLabel(i18n("Highlights:"), secondPage);
    d->shadowsChromaInput = new RIntNumInput(secondPage);
    d->shadowsChromaInput->setRange(-100, 100, 1);
    d->shadowsChromaInput->setSliderEnabled(true);
    d->shadowsChromaInput->setDefaultValue(-100);
    d->shadowsChromaInput->setWhatsThis(i18n("Set how much the filter affects shadows."));
      
    grid2->addWidget(label5,                    1, 0, 1, 1);
    grid2->addWidget(d->sensibilityChromaInput, 2, 0, 1, 1);
    grid2->addWidget(label6,                    3, 0, 1, 1);
    grid2->addWidget(d->shadowsChromaInput,     4, 0, 1, 1);  
    grid2->addWidget(label7,                    5, 0, 1, 1);
    grid2->addWidget(d->midtonesChromaInput,    6, 0, 1, 1); 
    grid2->addWidget(label8,                    7, 0, 1, 1); 
    grid2->addWidget(d->highlightsChromaInput,  8, 0, 1, 1);
    grid2->setMargin(d->gboxSettings->spacingHint());
    grid2->setSpacing(d->gboxSettings->spacingHint());
    
    // -------------------------------------------------------------

    d->expanderBox = new RExpanderBox();
    d->expanderBox->setObjectName("Noise Expander");
    d->expanderBox->addItem(firstPage, SmallIcon("filmgrain"), i18n("Luminance noise"),
                            QString("LuminanceSettingsContainer"), true);
    d->expanderBox->addItem(secondPage, SmallIcon("camera-photo"), i18n("Chrominance noise"),
                            QString("ChrominanceSettingsContainer"), true);
    d->expanderBox->addStretch();

    QGridLayout* grid = new QGridLayout;
    grid->addWidget(d->expanderBox, 0, 0, 1, 1);
    grid->setRowStretch(0, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(grid);
    
    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);    
    init();

    // -------------------------------------------------------------

    connect(d->sensibilityLumInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
            
    connect(d->shadowsLumInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
            
    connect(d->midtonesLumInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
            
    connect(d->highlightsLumInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer())); 
            
    connect(d->sensibilityChromaInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
            
    connect(d->shadowsChromaInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
            
    connect(d->midtonesChromaInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
            
    connect(d->highlightsChromaInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
            
    // -------------------------------------------------------------

    slotTimer();
}

FilmGrainTool::~FilmGrainTool()
{
    delete d;
}

void FilmGrainTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->sensibilityLumInput->blockSignals(true);
    d->sensibilityLumInput->setValue(group.readEntry(d->configSensitivityAdjustmentEntry, 
                                                     d->sensibilityLumInput->defaultValue()));
    d->sensibilityLumInput->blockSignals(false);
}

void FilmGrainTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configSensitivityAdjustmentEntry, d->sensibilityLumInput->value());
    config->sync();
}

void FilmGrainTool::slotResetSettings()
{
    d->sensibilityLumInput->blockSignals(true);
    d->sensibilityLumInput->slotReset();
    d->sensibilityLumInput->blockSignals(false);
}

void FilmGrainTool::prepareEffect()
{
    toolSettings()->setEnabled(false);
    toolView()->setEnabled(false);
    
    FilmGrainContainer prm;
    prm.lum_sensibility    = d->sensibilityLumInput->value();
    prm.lum_shadows        = d->shadowsLumInput->value();
    prm.lum_midtones       = d->midtonesLumInput->value();
    prm.lum_highlights     = d->highlightsLumInput->value();
    prm.chroma_sensibility = d->sensibilityChromaInput->value();
    prm.chroma_shadows     = d->shadowsChromaInput->value(); 
    prm.chroma_midtones    = d->midtonesChromaInput->value();
    prm.chroma_highlights  = d->highlightsChromaInput->value();
    DImg image             = d->previewWidget->getOriginalRegionImage(true);
    
    setFilter(new FilmGrainFilter(&image, this, prm));
}

void FilmGrainTool::prepareFinal()
{
    toolSettings()->setEnabled(false);
    toolView()->setEnabled(false);    

    FilmGrainContainer prm;
    prm.lum_sensibility    = d->sensibilityLumInput->value();
    prm.lum_shadows        = d->shadowsLumInput->value();
    prm.lum_midtones       = d->midtonesLumInput->value();
    prm.lum_highlights     = d->highlightsLumInput->value();
    prm.chroma_sensibility = d->sensibilityChromaInput->value();
    prm.chroma_shadows     = d->shadowsChromaInput->value(); 
    prm.chroma_midtones    = d->midtonesChromaInput->value();
    prm.chroma_highlights  = d->highlightsChromaInput->value();
    
    ImageIface iface(0, 0);
    setFilter(new FilmGrainFilter(iface.getOriginalImg(), this, prm));
}

void FilmGrainTool::putPreviewData()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void FilmGrainTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Film Grain"), filter()->getTargetImage().bits());
}

void FilmGrainTool::renderingFinished()
{
    toolSettings()->setEnabled(true);
    toolView()->setEnabled(true);
}

}  // namespace DigikamImagesPluginCore
