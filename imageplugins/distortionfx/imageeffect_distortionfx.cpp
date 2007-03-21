/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2005-02-11
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier 
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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
 
// Qt includes. 
 
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qimage.h>
#include <qspinbox.h>
#include <qcombobox.h>

// KDE includes.

#include <kconfig.h>
#include <klocale.h>
#include <kcursor.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kapplication.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <kprogress.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "distortionfx.h"
#include "imageeffect_distortionfx.h"
#include "imageeffect_distortionfx.moc"

namespace DigikamDistortionFXImagesPlugin
{

ImageEffect_DistortionFX::ImageEffect_DistortionFX(QWidget* parent)
                        : Digikam::ImageGuideDlg(parent, i18n("Distortion Effects"),
                                                 "distortionfx", false, true, false,
                                                 Digikam::ImageGuideWidget::HVGuideMode)
{
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikam",
                                       I18N_NOOP("Distortion Effects"), 
                                       digikam_version,
                                       I18N_NOOP("A digiKam image plugin to apply distortion effects to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier\n"
                                       "(c) 2006-2007, Gilles Caulier and Marcel Wiesweg", 
                                       0,
                                       "http://www.digikam.org");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Distortion algorithms"), 
                     "pieter dot voloshyn at gmail dot com"); 

    about->addAuthor("Marcel Wiesweg", I18N_NOOP("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);
        
    QWhatsThis::add( m_imagePreviewWidget, i18n("<p>This is the preview of the distortion effect "
                                                "applied to the photograph.") );
                                           
    // -------------------------------------------------------------
    
    QWidget *gboxSettings = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 5, 2, spacingHint());
    
    m_effectTypeLabel = new QLabel(i18n("Type:"), gboxSettings);
    
    m_effectType = new QComboBox( false, gboxSettings );
    m_effectType->insertItem( i18n("Fish Eyes") );
    m_effectType->insertItem( i18n("Twirl") );
    m_effectType->insertItem( i18n("Cylindrical Hor.") );
    m_effectType->insertItem( i18n("Cylindrical Vert.") );
    m_effectType->insertItem( i18n("Cylindrical H/V.") );
    m_effectType->insertItem( i18n("Caricature") );
    m_effectType->insertItem( i18n("Multiple Corners") );
    m_effectType->insertItem( i18n("Waves Hor.") );
    m_effectType->insertItem( i18n("Waves Vert.") );
    m_effectType->insertItem( i18n("Block Waves 1") );
    m_effectType->insertItem( i18n("Block Waves 2") );
    m_effectType->insertItem( i18n("Circular Waves 1") );
    m_effectType->insertItem( i18n("Circular Waves 2") );
    m_effectType->insertItem( i18n("Polar Coordinates") );    
    m_effectType->insertItem( i18n("Unpolar Coordinates") );    
    m_effectType->insertItem( i18n("Tile") );    
    QWhatsThis::add( m_effectType, i18n("<p>Select here the effect type to apply on image.<p>"
                                        "<b>Fish Eyes</b>: warps the photograph around a 3D spherical shape to "
                                        "reproduce the common photograph 'Fish Eyes' effect.<p>"
                                        "<b>Twirl</b>: spins the photograph to produce a Twirl pattern.<p>"
                                        "<b>Cylinder Hor.</b>: warps the photograph around a horizontal cylinder.<p>"
                                        "<b>Cylinder Vert.</b>: warps the photograph around a vertical cylinder.<p>"
                                        "<b>Cylinder H/V.</b>: warps the photograph around 2 cylinders, vertical "
                                        "and horizontal.<p>"
                                        "<b>Caricature</b>: distorts the photograph with the 'Fish Eyes' effect inverted.<p>"
                                        "<b>Multiple Corners</b>: splits the photograph like a multiple corners pattern.<p>"
                                        "<b>Waves Horizontal</b>: distorts the photograph with horizontal waves.<p>"
                                        "<b>Waves Vertical</b>: distorts the photograph with verticals waves.<p>"
                                        "<b>Block Waves 1</b>: divides the image into cells and makes it look as "
                                        "if it is being viewed through glass blocks.<p>"
                                        "<b>Block Waves 2</b>: like Block Waves 1 but with another version "
                                        "of glass blocks distortion.<p>"
                                        "<b>Circular Waves 1</b>: distorts the photograph with circular waves.<p>"
                                        "<b>Circular Waves 2</b>: another variation of Circular Waves effect.<p>"
                                        "<b>Polar Coordinates</b>: converts the photograph from rectangular "
                                        "to polar coordinates.<p>"
                                        "<b>Unpolar Coordinates</b>: Polar Coordinate effect inverted.<p>"
                                        "<b>Tile</b>: splits the photograph into square blocks and move "
                                        "them randomly inside the image.<p>"
                                        ));
    gridSettings->addMultiCellWidget(m_effectTypeLabel, 0, 0, 0, 2);
    gridSettings->addMultiCellWidget(m_effectType, 1, 1, 0, 2);
                                                  
    m_levelLabel = new QLabel(i18n("Level:"), gboxSettings);
    m_levelInput = new KIntNumInput(gboxSettings);
    m_levelInput->setRange(0, 100, 1, true);
    QWhatsThis::add( m_levelInput, i18n("<p>Set here the level of the effect."));
    
    gridSettings->addMultiCellWidget(m_levelLabel, 2, 2, 0, 2);
    gridSettings->addMultiCellWidget(m_levelInput, 3, 3, 0, 2);
        
    m_iterationLabel = new QLabel(i18n("Iteration:"), gboxSettings);
    m_iterationInput = new KIntNumInput(gboxSettings);
    m_iterationInput->setRange(0, 100, 1, true);
    QWhatsThis::add( m_iterationInput, i18n("<p>This value controls the iterations to use for Waves, "
                                            "Tile, and Neon effects."));
    
    gridSettings->addMultiCellWidget(m_iterationLabel, 4, 4, 0, 2);
    gridSettings->addMultiCellWidget(m_iterationInput, 5, 5, 0, 2);
    
    setUserAreaWidget(gboxSettings); 

    // -------------------------------------------------------------
    
    connect(m_effectType, SIGNAL(activated(int)),
            this, SLOT(slotEffectTypeChanged(int)));
    
    connect(m_levelInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));            
            
    connect(m_iterationInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));            
}

ImageEffect_DistortionFX::~ImageEffect_DistortionFX()
{
}

void ImageEffect_DistortionFX::renderingFinished()
{
    m_effectTypeLabel->setEnabled(true);
    m_effectType->setEnabled(true);
    m_levelInput->setEnabled(true);
    m_levelLabel->setEnabled(true);
    m_iterationInput->setEnabled(true);
    m_iterationLabel->setEnabled(true);

    switch (m_effectType->currentItem())
       {
       case DistortionFX::FishEye:
       case DistortionFX::Twirl: 
       case DistortionFX::CilindricalHor:  
       case DistortionFX::CilindricalVert:  
       case DistortionFX::CilindricalHV:  
       case DistortionFX::Caricature: 
       case DistortionFX::MultipleCorners:           
          break;
       
       case DistortionFX::PolarCoordinates: 
       case DistortionFX::UnpolarCoordinates: 
          m_levelInput->setEnabled(false);
          m_levelLabel->setEnabled(false);
          break;

       case DistortionFX::WavesHorizontal: 
       case DistortionFX::WavesVertical:  
       case DistortionFX::BlockWaves1:  
       case DistortionFX::BlockWaves2: 
       case DistortionFX::CircularWaves1: 
       case DistortionFX::CircularWaves2: 
       case DistortionFX::Tile: 
          m_iterationInput->setEnabled(true);
          m_iterationLabel->setEnabled(true);
          break;
       }
}

void ImageEffect_DistortionFX::readUserSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("distortionfx Tool Dialog");

    m_effectType->blockSignals(true);
    m_iterationInput->blockSignals(true);
    m_levelInput->blockSignals(true);

    m_effectType->setCurrentItem(config->readNumEntry("EffectType", DistortionFX::FishEye));
    m_iterationInput->setValue(config->readNumEntry("IterationAjustment", 10));
    m_levelInput->setValue(config->readNumEntry("LevelAjustment", 50));

    m_effectType->blockSignals(false);
    m_iterationInput->blockSignals(false);
    m_levelInput->blockSignals(false);

    slotEffect();
}

void ImageEffect_DistortionFX::writeUserSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("distortionfx Tool Dialog");
    config->writeEntry("EffectType", m_effectType->currentItem());
    config->writeEntry("IterationAjustment", m_iterationInput->value());
    config->writeEntry("LevelAjustment", m_levelInput->value());
    config->sync();
}

void ImageEffect_DistortionFX::resetValues()
{
    m_effectType->blockSignals(true);
    m_effectType->setCurrentItem(DistortionFX::FishEye);
    slotEffectTypeChanged(DistortionFX::FishEye);
    m_effectType->blockSignals(false);
} 

void ImageEffect_DistortionFX::slotEffectTypeChanged(int type)
{
    m_levelInput->setEnabled(true);
    m_levelLabel->setEnabled(true);
    
    m_levelInput->blockSignals(true);
    m_iterationInput->blockSignals(true);
    m_levelInput->setRange(0, 100, 1, true);
    m_levelInput->setValue(25);
          
    switch (type)
       {
       case DistortionFX::Twirl: 
          m_levelInput->setRange(-50, 50, 1, true);
          m_levelInput->setValue(10);
          break;

       case DistortionFX::FishEye: 
       case DistortionFX::CilindricalHor:  
       case DistortionFX::CilindricalVert:  
       case DistortionFX::CilindricalHV:  
       case DistortionFX::Caricature:  
          m_levelInput->setRange(0, 200, 1, true);
          m_levelInput->setValue(50);
          break;

       case DistortionFX::MultipleCorners: 
          m_levelInput->setRange(1, 10, 1, true);
          m_levelInput->setValue(4);
          break;
                                                  
       case DistortionFX::WavesHorizontal: 
       case DistortionFX::WavesVertical:  
       case DistortionFX::BlockWaves1: 
       case DistortionFX::BlockWaves2: 
       case DistortionFX::CircularWaves1: 
       case DistortionFX::CircularWaves2: 
       case DistortionFX::Tile: 
          m_iterationInput->setEnabled(true);
          m_iterationLabel->setEnabled(true);
          m_iterationInput->setRange(0, 200, 1, true);
          m_iterationInput->setValue(10);
          break;

       case DistortionFX::PolarCoordinates:
       case DistortionFX::UnpolarCoordinates: 
          m_levelInput->setEnabled(false);
          m_levelLabel->setEnabled(false);
          break;
       }

    m_levelInput->blockSignals(false);
    m_iterationInput->blockSignals(false);
       
    slotEffect();
}

void ImageEffect_DistortionFX::prepareEffect()
{
    m_effectTypeLabel->setEnabled(false);
    m_effectType->setEnabled(false);
    m_levelInput->setEnabled(false);
    m_levelLabel->setEnabled(false);
    m_iterationInput->setEnabled(false);
    m_iterationLabel->setEnabled(false);

    int l = m_levelInput->value();
    int f = m_iterationInput->value();
    int e = m_effectType->currentItem();

    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();

    uchar *data = iface->getPreviewImage();
    Digikam::DImg image(iface->previewWidth(), iface->previewHeight(), iface->previewSixteenBit(),
                        iface->previewHasAlpha(), data);
    delete [] data;

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new DistortionFX(&image, this, e, l, f));
}

void ImageEffect_DistortionFX::prepareFinal()
{
    m_effectTypeLabel->setEnabled(false);
    m_effectType->setEnabled(false);
    m_levelInput->setEnabled(false);
    m_levelLabel->setEnabled(false);
    m_iterationInput->setEnabled(false);
    m_iterationLabel->setEnabled(false);

    int l = m_levelInput->value();
    int f = m_iterationInput->value();
    int e = m_effectType->currentItem();

    Digikam::ImageIface iface(0, 0);

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new DistortionFX(iface.getOriginalImg(), this, e, l, f));
}

void ImageEffect_DistortionFX::putPreviewData(void)
{
    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();

    Digikam::DImg imDest = m_threadedFilter->getTargetImage()
            .smoothScale(iface->previewWidth(), iface->previewHeight());
    iface->putPreviewImage(imDest.bits());

    m_imagePreviewWidget->updatePreview();
}

void ImageEffect_DistortionFX::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Distortion Effects"), 
                           m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamDistortionFXImagesPlugin

