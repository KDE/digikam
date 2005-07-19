/* ============================================================
 * File  : imageeffect_distortionfx.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-11
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Original Distortion algorithms copyrighted 2004-2005 by 
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
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

#include <klocale.h>
#include <kcursor.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kapplication.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <kprogress.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "distortionfx.h"
#include "imageeffect_distortionfx.h"

namespace DigikamDistortionFXImagesPlugin
{

ImageEffect_DistortionFX::ImageEffect_DistortionFX(QWidget* parent)
                        : ImageGuideDialog(parent, i18n("Distortion Effects"), 
                                           "distortionfx", false, true, false)                          
{
    QString whatsThis;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Distortion Effects"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to apply distortion effect to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Distortion algorithms"), 
                     "pieter_voloshyn at ame.com.br"); 
    
    setAboutData(about);
        
    QWhatsThis::add( m_imagePreviewWidget, i18n("<p>This is the preview of the distortion effect "
                                                "applied to the photograph.") );
                                           
    // -------------------------------------------------------------
    
    QWidget *gboxSettings = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 5, 2, marginHint(), spacingHint());
    
    m_effectTypeLabel = new QLabel(i18n("Type:"), gboxSettings);
    
    m_effectType = new QComboBox( false, gboxSettings );
    m_effectType->insertItem( i18n("Fish Eyes") );
    m_effectType->insertItem( i18n("Twirl") );
    m_effectType->insertItem( i18n("Cilindrical Hor.") );
    m_effectType->insertItem( i18n("Cilindrical Vert.") );
    m_effectType->insertItem( i18n("Cilindrical H/V.") );
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
    m_effectType->insertItem( i18n("Neon") );    
    m_effectType->insertItem( i18n("Find Edges") );    
    QWhatsThis::add( m_effectType, i18n("<p>Select here the effect type to apply on image.<p>"
                                        "<b>Fish Eyes</b>: warps the photograph around a 3D spherical shape to "
                                        "reproduce the common photograph 'Fish Eyes' effect.<p>"
                                        "<b>Twirl</b>: spins the photograph to produce a Twirl pattern.<p>"
                                        "<b>Cylinder Hor.</b>: warps the photograph around a horizontal cylinder.<p>"
                                        "<b>Cylinder Vert.</b>: warps the photograph around a vertical cylinder.<p>"
                                        "<b>Cylinder H/V.</b>: warps the photograph around a 2 cylinders, vertical "
                                        "and horizontal.<p>"
                                        "<b>Caricature</b>: distorts photograph with 'Fish Eyes' effect inverted.<p>"
                                        "<b>Multiple Corners</b>: splits the photograph like a multiple corners pattern.<p>"
                                        "<b>Waves Horizontal</b>: distorts the photograph with horizontal waves.<p>"
                                        "<b>Waves Vertical</b>: distorts the photograph with verticals waves.<p>"
                                        "<b>Block Waves 1</b>: divides the image into cells and makes it look as "
                                        "if it is being viewed through glass blocks.<p>"
                                        "<b>Block Waves 2</b>: like Block Waves 1 but with another version "
                                        "of glass blocks distorsion.<p>"
                                        "<b>Circular Waves 1</b>: distorts the photograph with circular waves.<p>"
                                        "<b>Circular Waves 2</b>: other variation of Circular Waves effect.<p>"
                                        "<b>Polar Coordinates</b>: converts the photograph from rectangular "
                                        "to polar coordinates.<p>"
                                        "<b>Unpolar Coordinates</b>: Polar Coordinate effect inverted.<p>"
                                        "<b>Tile</b>: splits the photograph into square blocks and move "
                                        "them randomly inside the image.<p>"
                                        "<b>Neon</b>: sub-coloring the edges in a photograph to reproduce a "
                                        "neon highlightment.<p>"
                                        "<b>Find Edges</b>: detects the edges in a photograph and their strength."
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
    whatsThis = i18n("<p>This value controls the iterations to use for Waves, Tile, and Neon effects.");
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
       case 0:  // Fish Eye.
       case 1:  // Twirl.
       case 2:  // Cilindrical Hor.
       case 3:  // Cilindrical Vert.
       case 4:  // Cilindrical H/V.
       case 5:  // Caricature.
       case 6:  // Multiple Corners.          
          break;
       
       case 13: // Polar Coordinates.
       case 14: // Unpolar Coordinates.
          m_levelInput->setEnabled(false);
          m_levelLabel->setEnabled(false);
          break;

       case 7:  // Waves Horizontal.
       case 8:  // Waves Vertical.
       case 9:  // Block Waves 1.
       case 10: // Block Waves 2.
       case 11: // Circular Waves 1.
       case 12: // Circular Waves 2.
       case 15: // Tile.
       case 16: // Neon.
       case 17: // Find Edges.
          m_iterationInput->setEnabled(true);
          m_iterationLabel->setEnabled(true);
          break;
       }
}

void ImageEffect_DistortionFX::resetValues()
{
    m_effectType->blockSignals(true);
    m_effectType->setCurrentItem(0);
    slotEffectTypeChanged(0);
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
       case 1:  // Twirl.
          m_levelInput->setRange(-50, 50, 1, true);
          m_levelInput->setValue(10);
          break;

       case 0:  // Fish Eye.
       case 2:  // Cilindrical Hor.
       case 3:  // Cilindrical Vert.
       case 4:  // Cilindrical H/V.
       case 5:  // Caricature.
          m_levelInput->setRange(0, 200, 1, true);
          m_levelInput->setValue(50);
          break;

       case 6:  // Multiple Corners.
          m_levelInput->setRange(1, 10, 1, true);
          m_levelInput->setValue(4);
          break;
                                                  
       case 7:  // Waves Horizontal.
       case 8:  // Waves Vertical.
       case 9: // Block Waves 1.
       case 10: // Block Waves 2.
       case 11: // Circular Waves 1.
       case 12: // Circular Waves 2.
       case 15: // Tile.
          m_iterationInput->setEnabled(true);
          m_iterationLabel->setEnabled(true);
          m_iterationInput->setRange(0, 200, 1, true);
          m_iterationInput->setValue(10);
          break;

       case 13: // Polar Coordinates.
       case 14: // Unpolar Coordinates.
          m_levelInput->setEnabled(false);
          m_levelLabel->setEnabled(false);
          break;
                 
       case 16: // Neon.
       case 17: // Find Edges.
          m_levelInput->setRange(0, 5, 1, true);
          m_levelInput->setValue(3);
          m_iterationInput->setEnabled(true);
          m_iterationLabel->setEnabled(true);
          m_iterationInput->setRange(0, 5, 1, true);
          m_iterationInput->setValue(2);
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
    QImage image(iface->previewWidth(), iface->previewHeight(), 32);
    uint *data = iface->getPreviewData();
    memcpy( image.bits(), data, image.numBytes() );
    
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(
                       new DistortionFX(&image, this, e, l, f));    
    delete [] data;
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
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );

    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(
                       new DistortionFX(&orgImage, this, e, l, f));            
    delete [] data;
}

void ImageEffect_DistortionFX::putPreviewData(void)
{
    QImage imDest = m_threadedFilter->getTargetImage();
    
    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    
    iface->putPreviewData((uint*)(imDest.smoothScale(iface->previewWidth(),
                                                     iface->previewHeight())).bits());
                 
    m_imagePreviewWidget->update();  
}

void ImageEffect_DistortionFX::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
  
    iface.putOriginalData(i18n("Distortion Effects"), 
         (uint*)m_threadedFilter->getTargetImage().bits());
}


/*
void ImageEffect_DistortionFX::slotEffect()
{
    m_dirty = true;
    m_parent->setCursor( KCursor::waitCursor() );
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
        
    m_effectTypeLabel->setEnabled(false);
    m_effectType->setEnabled(false);
    m_levelInput->setEnabled(false);
    m_levelLabel->setEnabled(false);
    m_iterationInput->setEnabled(false);
    m_iterationLabel->setEnabled(false);

    
    
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    // Preview image size.
    int wp      = iface->previewWidth();
    int hp      = iface->previewHeight();
    
    // All data from the image
    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();
    
    int l       = m_levelInput->value();
    int f       = m_iterationInput->value();

    m_progressBar->setValue(0); 

    switch (m_effectType->currentItem())
       {
       case 0: // Fish Eye.
          fisheye(data, w, h, (double)(l/5.0), false);
          break;
       
       case 1: // Twirl.
          twirl(data, w, h, l, false);
          break;

       case 2: // Cilindrical Hor.
          cilindrical(data, w, h, (double)l, true, false, false);
          break;

       case 3: // Cilindrical Vert.
          cilindrical(data, w, h, (double)l, false, true, false);
          break;
                    
       case 4: // Cilindrical H/V.
          cilindrical(data, w, h, (double)l, true, true, false);
          break;
       
       case 5: // Caricature.
          fisheye(data, w, h, (double)(-l/5.0), false);
          break;
          
       case 6: // Multiple Corners.          
          multipleCorners(data, w, h, l, false);
          break;
       
       case 7: // Waves Horizontal.
          waves(data, w, h, l, f, true, false);
          break;
       
       case 8: // Waves Vertical.
          waves(data, w, h, l, f, true, true);
          break;
       
       case 9: // Block Waves 1.
          blockWaves(data, w, h, l, f, false);
          break;
       
       case 10: // Block Waves 2.
          blockWaves(data, w, h, l, f, true);
          break;
       
       case 11: // Circular Waves 1.
          circularWaves(data, w, h, w/2, h/2, (double)l, (double)f, 0.0, false, false);
          break;
       
       case 12: // Circular Waves 2.
          circularWaves(data, w, h, w/2, h/2, (double)l, (double)f, 25.0, true, false);
          break;
       
       case 13: // Polar Coordinates.
          polarCoordinates(data, w, h, true, false);
          break;

       case 14: // Unpolar Coordinates.
          polarCoordinates(data, w, h, false, false);
          break;
                    
       case 15:  // Tile.
          tile(data, w, h, 200-f, 200-f, l);
          break;

       case 16: // Neon.
          neon(data, w, h, l, f);
          break;
          
       case 17: // Find Edges.
          findEdges(data, w, h, l, f);
          break;
       }
    
    if ( !m_cancel ) 
       {
       QImage newImg((uchar*)data, w, h, 32, 0, 0, QImage::IgnoreEndian);
       QImage destImg = newImg.smoothScale(wp, hp);
       iface->putPreviewData((uint*)destImg.bits());
       }
                  
    delete [] data;
    m_progressBar->setValue(0); 
    m_previewWidget->update();

    m_effectTypeLabel->setEnabled(true);
    m_effectType->setEnabled(true);
    m_levelInput->setEnabled(true);
    m_levelLabel->setEnabled(true);
    
    switch (m_effectType->currentItem())
       {
       case 0:  // Fish Eye.
       case 1:  // Twirl.
       case 2:  // Cilindrical Hor.
       case 3:  // Cilindrical Vert.
       case 4:  // Cilindrical H/V.
       case 5:  // Caricature.
       case 6:  // Multiple Corners.          
          break;
       
       case 13: // Polar Coordinates.
       case 14: // Unpolar Coordinates.
          m_levelInput->setEnabled(false);
          m_levelLabel->setEnabled(false);
          break;

       case 7:  // Waves Horizontal.
       case 8:  // Waves Vertical.
       case 9:  // Block Waves 1.
       case 10: // Block Waves 2.
       case 11: // Circular Waves 1.
       case 12: // Circular Waves 2.
       case 15: // Tile.
       case 16: // Neon.
       case 17: // Find Edges.
          m_iterationInput->setEnabled(true);
          m_iterationLabel->setEnabled(true);
          break;
       }
    
    m_cancel = false;
    m_dirty = false;
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all parameters to the default values.") );
    enableButton(Ok, true);
    m_parent->setCursor( KCursor::arrowCursor() );
}

void ImageEffect_DistortionFX::slotOk()
{
    m_effectTypeLabel->setEnabled(false);
    m_effectType->setEnabled(false);
    m_levelInput->setEnabled(false);
    m_levelLabel->setEnabled(false);
    m_iterationInput->setEnabled(false);
    m_iterationLabel->setEnabled(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();
    int l       = m_levelInput->value();
    int f       = m_iterationInput->value();

    m_progressBar->setValue(0); 
        
    if (data) 
       {
       switch (m_effectType->currentItem())
          {
          case 0: // Fish Eye.
             fisheye(data, w, h, (double)(l/5.0));
             break;
       
          case 1: // Twirl.
             twirl(data, w, h, l);
             break;

          case 2: // Cilindrical Hor.
             cilindrical(data, w, h, (double)l, true, false);
             break;
          
          case 3: // Cilindrical Vert.
             cilindrical(data, w, h, (double)l, false, true);
             break;
          
          case 4: // Cilindrical H/V.
             cilindrical(data, w, h, (double)l, false, true);
             break;
          
          case 5: // Caricature.
             fisheye(data, w, h, (double)(-l/5.0));
             break;
          
          case 6: // Multiple Corners.          
             multipleCorners(data, w, h, l);
             break;
          
          case 7: // Waves Horizontal.
             waves(data, w, h, l, f, true, false);
             break;
       
          case 8: // Waves Vertical.
             waves(data, w, h, l, f, true, true);
             break;

          case 9: // Block Waves 1.
             blockWaves(data, w, h, l, f, false);
             break;
          
          case 10: // Block Waves 2.
             blockWaves(data, w, h, l, f, true);
             break;
       
          case 11: // Circular Waves 1.
             circularWaves(data, w, h, w/2, h/2, (double)l, (double)f, 0.0, false);
             break;
          
          case 12: // Circular Waves 2.
             circularWaves(data, w, h, w/2, h/2, (double)l, (double)f, 25.0, true);
             break;
          
          case 13: // Polar Coordinates.
             polarCoordinates(data, w, h, true);
             break;

          case 14: // Unpolar Coordinates.
             polarCoordinates(data, w, h, false);
             break;
                          
          case 15:  // Tile.
             tile(data, w, h, 200-f, 200-f, l);
             break;

          case 16: // Neon.
             neon(data, w, h, l, f);
             break;
          
          case 17: // Find Edges.
             findEdges(data, w, h, l, f);
             break;
          }
       
       if ( !m_cancel ) iface->putOriginalData(i18n("Distortion Effects"), data);
       }
    
    delete [] data;    
    m_parent->setCursor( KCursor::arrowCursor() );        
    accept();
}
*/

}  // NameSpace DigikamDistortionFXImagesPlugin

#include "imageeffect_distortionfx.moc"
