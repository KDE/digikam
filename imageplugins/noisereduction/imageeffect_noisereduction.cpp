/* ============================================================
 * File  : imageeffect_noisereduction.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-08-24
 * Description : noise reduction image filter for digiKam 
 *               image editor.
 * 
 * Copyright 2004-2006 by Gilles Caulier
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
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qstring.h>
#include <qtabwidget.h>
#include <qimage.h>
#include <qlayout.h>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <knuminput.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "noisereduction.h"
#include "imageeffect_noisereduction.h"

namespace DigikamNoiseReductionImagesPlugin
{

ImageEffect_NoiseReduction::ImageEffect_NoiseReduction(QWidget* parent, QString title, QFrame* banner)
                          : Digikam::CtrlPanelDlg(parent, title, "noisereduction", false,
                                     false, true, Digikam::ImagePannelWidget::SeparateViewAll, banner)
{
    QString whatsThis;
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Noise Reduction"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A noise reduction image filter plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2006, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at kdemail dot net");

    about->addAuthor("Peter Heckert", I18N_NOOP("Original Noise Reduction algorithm author"),
                     "peter dot heckert at arcor dot de");
                     
    setAboutData(about);
   
    // -------------------------------------------------------------

    QTabWidget *mainTab = new QTabWidget(m_imagePreviewWidget);
    
    QWidget* firstPage = new QWidget( mainTab );
    QGridLayout* gridSettings = new QGridLayout( firstPage, 4, 1, marginHint());    
    mainTab->addTab( firstPage, i18n("General") );
    
    QLabel *label1 = new QLabel(i18n("Radius:"), firstPage);
    
    m_radiusInput = new KDoubleNumInput(firstPage);
    m_radiusInput->setPrecision(1);
    m_radiusInput->setRange(0.0, 3.0, 0.1, true);
    QWhatsThis::add( m_radiusInput, i18n("<p>Set here the <b>Radius of gaussian blur</b> in pixels used to "
                     "filter noise. In any case it must be about the same size as noise granularity ore "
                     "somewhat more. If it is set higher than necessary, then it can cause unwanted blur."));

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_radiusInput, 0, 0, 1, 1);
    
    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Luminance:"), firstPage);
    
    m_lumToleranceInput = new KDoubleNumInput(firstPage);
    m_lumToleranceInput->setPrecision(1);
    m_lumToleranceInput->setRange(0.1, 5.0, 0.1, true);
    QWhatsThis::add( m_lumToleranceInput, i18n("<p>Set here the <b>Luminance Tolerance</b> adjustement."));

    gridSettings->addMultiCellWidget(label2, 1, 1, 0, 0);
    gridSettings->addMultiCellWidget(m_lumToleranceInput, 1, 1, 1, 1);                         
    
    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Threshold:"), firstPage);
    
    m_thresholdInput = new KDoubleNumInput(firstPage);
    m_thresholdInput->setPrecision(1);
    m_thresholdInput->setRange(0.0, 5.0, 0.1, true);
    QWhatsThis::add( m_thresholdInput, i18n("<p>Set here the <b>Threshold</b> for 2nd derivative of "
                                            "luminance adjustment."));

    gridSettings->addMultiCellWidget(label3, 2, 2, 0, 0);
    gridSettings->addMultiCellWidget(m_thresholdInput, 2, 2, 1, 1);
                                              
    // -------------------------------------------------------------
  
    QLabel *label4 = new QLabel(i18n("Texture:"), firstPage);
    
    m_textureInput = new KDoubleNumInput(firstPage);
    m_textureInput->setPrecision(2);
    m_textureInput->setRange(-0.99, 0.99, 0.01, true);
    QWhatsThis::add( m_textureInput, i18n("<p>Set here the <b>Texture Accuracy</b> adjustment."));

    gridSettings->addMultiCellWidget(label4, 3, 3, 0, 0);
    gridSettings->addMultiCellWidget(m_textureInput, 3, 3, 1, 1);
  
    // -------------------------------------------------------------
  
    QLabel *label5 = new QLabel(i18n("Edge:"), firstPage);
    
    m_sharpnessInput = new KDoubleNumInput(firstPage);
    m_sharpnessInput->setPrecision(2);
    m_sharpnessInput->setRange(0.0, 2.0, 0.01, true);
    QWhatsThis::add( m_sharpnessInput, i18n("<p>Set here the <b>Edge Accuracy</b> adjustment of sharpness."));

    gridSettings->addMultiCellWidget(label5, 4, 4, 0, 0);
    gridSettings->addMultiCellWidget(m_sharpnessInput, 4, 4, 1, 1);

    // -------------------------------------------------------------

    QWidget* secondPage = new QWidget( mainTab );
    QGridLayout* gridSettings2 = new QGridLayout( secondPage, 4, 1, marginHint());    
    mainTab->addTab( secondPage, i18n("Advanced") );

    QLabel *label6 = new QLabel(i18n("Color:"), secondPage);
    
    m_csmoothInput = new KDoubleNumInput(secondPage);
    m_csmoothInput->setPrecision(1);
    m_csmoothInput->setRange(1.0, 5.0, 0.1, true);
    QWhatsThis::add( m_csmoothInput, i18n("<p>Set here the <b>Color Tolerance</b> adjustement."));

    gridSettings2->addMultiCellWidget(label6, 0, 0, 0, 0);
    gridSettings2->addMultiCellWidget(m_csmoothInput, 0, 0, 1, 1);
    
    // -------------------------------------------------------------

    QLabel *label7 = new QLabel(i18n("Sharpness:"), secondPage);
    
    m_lookaheadInput = new KDoubleNumInput(secondPage);
    m_lookaheadInput->setPrecision(1);
    m_lookaheadInput->setRange(0.5, 10.0, 0.5, true);
    QWhatsThis::add( m_lookaheadInput, i18n("<p>Set here the <b>Sharpness Level</b> adjustement."));
    
    gridSettings2->addMultiCellWidget(label7, 1, 1, 0, 0);
    gridSettings2->addMultiCellWidget(m_lookaheadInput, 1, 1, 1, 1);

    // -------------------------------------------------------------

    QLabel *label8 = new QLabel(i18n("Gamma:"), secondPage);
    
    m_gammaInput = new KDoubleNumInput(secondPage);
    m_gammaInput->setPrecision(1);
    m_gammaInput->setRange(1.0, 5.0, 0.1, true);
    QWhatsThis::add( m_gammaInput, i18n("<p>Set here the <b>Gamma</b> adjustement."));
    
    gridSettings2->addMultiCellWidget(label8, 2, 2, 0, 0);
    gridSettings2->addMultiCellWidget(m_gammaInput, 2, 2, 1, 1);

    // -------------------------------------------------------------

    QLabel *label9 = new QLabel(i18n("Damping:"), secondPage);
    
    m_dampingInput = new KDoubleNumInput(secondPage);
    m_dampingInput->setPrecision(1);
    m_dampingInput->setRange(0.5, 20.0, 0.5, true);
    QWhatsThis::add( m_dampingInput, i18n("<p>Set here the <b>Phase Jitter Damping</b> adjustement."));
    
    gridSettings2->addMultiCellWidget(label9, 3, 3, 0, 0);
    gridSettings2->addMultiCellWidget(m_dampingInput, 3, 3, 1, 1);

    // -------------------------------------------------------------

    QLabel *label10 = new QLabel(i18n("Erosion:"), secondPage);
    
    m_phaseInput = new KDoubleNumInput(secondPage);
    m_phaseInput->setPrecision(1);
    m_phaseInput->setRange(0.5, 20.0, 0.5, true);
    QWhatsThis::add( m_phaseInput, i18n("<p>Set here the <b>Phase Shift for Edges</b> adjustement."));
    
    gridSettings2->addMultiCellWidget(label10, 4, 4, 0, 0);
    gridSettings2->addMultiCellWidget(m_phaseInput, 4, 4, 1, 1);

    m_imagePreviewWidget->setUserAreaWidget(mainTab);
    
    // -------------------------------------------------------------
    
    connect(m_radiusInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            

    connect(m_lumToleranceInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            

    connect(m_thresholdInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            
           
    connect(m_textureInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            

    connect(m_sharpnessInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));           

    connect(m_csmoothInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            

    connect(m_lookaheadInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            

    connect(m_gammaInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            
           
    connect(m_dampingInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));            

    connect(m_phaseInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));   
}

ImageEffect_NoiseReduction::~ImageEffect_NoiseReduction()
{
}

void ImageEffect_NoiseReduction::renderingFinished()
{
    m_radiusInput->setEnabled(true);
    m_lumToleranceInput->setEnabled(true);
    m_thresholdInput->setEnabled(true);
    m_textureInput->setEnabled(true);
    m_sharpnessInput->setEnabled(true);

    m_csmoothInput->setEnabled(true);
    m_lookaheadInput->setEnabled(true);
    m_gammaInput->setEnabled(true);
    m_dampingInput->setEnabled(true);
    m_phaseInput->setEnabled(true);
}

void ImageEffect_NoiseReduction::resetValues()
{
    m_radiusInput->setEnabled(true);
    m_lumToleranceInput->setEnabled(true);
    m_thresholdInput->setEnabled(true);
    m_textureInput->setEnabled(true);
    m_sharpnessInput->setEnabled(true);

    m_csmoothInput->setEnabled(true);
    m_lookaheadInput->setEnabled(true);
    m_gammaInput->setEnabled(true);
    m_dampingInput->setEnabled(true);
    m_phaseInput->setEnabled(true);
                    
    m_radiusInput->setValue(1.0);
    m_lumToleranceInput->setValue(1.0);
    m_thresholdInput->setValue(0.08);
    m_textureInput->setValue(0.0);
    m_sharpnessInput->setValue(0.25);

    m_csmoothInput->setValue(1.0);
    m_lookaheadInput->setValue(2.0);
    m_gammaInput->setValue(1.0);
    m_dampingInput->setValue(5.0);
    m_phaseInput->setValue(1.0);
    
    m_radiusInput->setEnabled(false);
    m_lumToleranceInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    m_textureInput->setEnabled(false);
    m_sharpnessInput->setEnabled(false);

    m_csmoothInput->setEnabled(false);
    m_lookaheadInput->setEnabled(false);
    m_gammaInput->setEnabled(false);
    m_dampingInput->setEnabled(false);
    m_phaseInput->setEnabled(false);
}

void ImageEffect_NoiseReduction::prepareEffect()
{
    m_radiusInput->setEnabled(false);
    m_lumToleranceInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    m_textureInput->setEnabled(false);
    m_sharpnessInput->setEnabled(false);

    m_csmoothInput->setEnabled(false);
    m_lookaheadInput->setEnabled(false);
    m_gammaInput->setEnabled(false);
    m_dampingInput->setEnabled(false);
    m_phaseInput->setEnabled(false);
    
    double r  = m_radiusInput->value();
    double l  = m_lumToleranceInput->value();
    double th = m_thresholdInput->value();
    double tx = m_textureInput->value();
    double s  = m_sharpnessInput->value();

    double c  = m_csmoothInput->value();
    double a  = m_lookaheadInput->value();
    double g  = m_gammaInput->value();
    double d  = m_dampingInput->value();
    double p  = m_phaseInput->value();
    
    Digikam::DImg image = m_imagePreviewWidget->getOriginalRegionImage();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new NoiseReduction(&image,
                       this, r, l, th, tx, s, c, a, g, d, p));
}

void ImageEffect_NoiseReduction::prepareFinal()
{
    m_radiusInput->setEnabled(false);
    m_lumToleranceInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    m_textureInput->setEnabled(false);
    m_sharpnessInput->setEnabled(false);

    m_csmoothInput->setEnabled(false);
    m_lookaheadInput->setEnabled(false);
    m_gammaInput->setEnabled(false);
    m_dampingInput->setEnabled(false);
    m_phaseInput->setEnabled(false);
    
    double r  = m_radiusInput->value();
    double l  = m_lumToleranceInput->value();
    double th = m_thresholdInput->value();
    double tx = m_textureInput->value();
    double s  = m_sharpnessInput->value();

    double c  = m_csmoothInput->value();
    double a  = m_lookaheadInput->value();
    double g  = m_gammaInput->value();
    double d  = m_dampingInput->value();
    double p  = m_phaseInput->value();

    Digikam::ImageIface iface(0, 0);
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new NoiseReduction(iface.getOriginalImg(),
                       this, r, l, th, tx, s, c, a, g, d, p));
}

void ImageEffect_NoiseReduction::putPreviewData(void)
{
    m_imagePreviewWidget->setPreviewImage(m_threadedFilter->getTargetImage());
}

void ImageEffect_NoiseReduction::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Noise Reduction"), m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamNoiseReductionImagesPlugin

#include "imageeffect_noisereduction.moc"
