/* ============================================================
 * File  : imageeffect_despeckle.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-24
 * Description : noise reduction image filter for digiKam 
 *               image editor.
 * 
 * Copyright 2004-2005 by Gilles Caulier
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
#include "despeckle.h"
#include "imageeffect_despeckle.h"

namespace DigikamNoiseReductionImagesPlugin
{

ImageEffect_Despeckle::ImageEffect_Despeckle(QWidget* parent)
                     : CtrlPanelDialog(parent, i18n("Noise Reduction"), "despeckle")
{
    QString whatsThis;
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Noise Reduction"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A despeckle image filter plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Michael Sweet", I18N_NOOP("Despeckle algorithm author from Gimp"),
                     "mike at easysw.com");
                         
    setAboutData(about);
    
    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 5, 2, marginHint(), spacingHint());    
    
    QLabel *label1 = new QLabel(i18n("Radius:"), gboxSettings);
    
    m_radiusInput = new KIntNumInput(gboxSettings, "m_radiusInput");
    m_radiusInput->setRange(1, 20, 1, true);
    
    QWhatsThis::add( m_radiusInput, i18n("<p>A radius of 0 has no effect, "
                     "1 and above determine the blur matrix radius "
                     "that determines how much to blur the image.") );
    
    gridSettings->addWidget(label1, 0, 0);
    gridSettings->addWidget(m_radiusInput, 0, 1);
    
    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Black level:"), gboxSettings);
    
    m_blackLevelInput = new KIntNumInput(gboxSettings, "m_blackLevelInput");
    m_blackLevelInput->setRange(0, 255, 1, true);
    
    QWhatsThis::add( m_blackLevelInput, i18n("<p>This value controls the black "
                     "levels used by the adaptive filter to "
                     "adjust the filter radius.") );

    gridSettings->addWidget(label2, 1, 0);
    gridSettings->addWidget(m_blackLevelInput, 1, 1);                         
    
    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("White level:"), gboxSettings);
    
    m_whiteLevelInput = new KIntNumInput(gboxSettings, "m_blackLevelInput");
    m_whiteLevelInput->setRange(0, 255, 1, true);
    
    QWhatsThis::add( m_whiteLevelInput, i18n("<p>This value controls the white "
                     "levels used by the adaptive filter to "
                     "adjust the filter radius.") );

    gridSettings->addWidget(label3, 3, 0);
    gridSettings->addWidget(m_whiteLevelInput, 3, 1);
                                              
    // -------------------------------------------------------------
    
    m_useAdaptativeMethod = new QCheckBox( i18n("Adaptive"), gboxSettings);
    QWhatsThis::add( m_useAdaptativeMethod, i18n("<p>This option use an adaptive median filter type."));
    
    m_useRecursiveMethod = new QCheckBox( i18n("Recursive"), gboxSettings);
    QWhatsThis::add( m_useRecursiveMethod, i18n("<p>This option use a recursive median filter type.")); 
    
    gridSettings->addMultiCellWidget(m_useAdaptativeMethod, 4, 4, 0, 1);
    gridSettings->addMultiCellWidget(m_useRecursiveMethod, 4, 4, 1, 1);    
        
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    
    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));            
            
    connect(m_blackLevelInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_whiteLevelInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));                                                
            
    connect(m_useAdaptativeMethod, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));             
    
    connect(m_useRecursiveMethod, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));             
}

ImageEffect_Despeckle::~ImageEffect_Despeckle()
{
}

void ImageEffect_Despeckle::renderingFinished()
{
    m_radiusInput->setEnabled(true);
    m_blackLevelInput->setEnabled(true);
    m_whiteLevelInput->setEnabled(true);
    m_useAdaptativeMethod->setEnabled(true);
    m_useRecursiveMethod->setEnabled(true);
}

void ImageEffect_Despeckle::resetValues()
{
    m_radiusInput->blockSignals(true);
    m_blackLevelInput->blockSignals(true);
    m_whiteLevelInput->blockSignals(true);
    m_useAdaptativeMethod->blockSignals(true);
    m_useRecursiveMethod->blockSignals(true);
                    
    m_radiusInput->setValue(3);
    m_blackLevelInput->setValue(7);
    m_whiteLevelInput->setValue(248);
    m_useAdaptativeMethod->setChecked(true);
    m_useRecursiveMethod->setChecked(false);
    
    m_radiusInput->blockSignals(false);
    m_blackLevelInput->blockSignals(false);
    m_whiteLevelInput->blockSignals(false);
    m_useAdaptativeMethod->blockSignals(false);
    m_useRecursiveMethod->blockSignals(false);
} 

void ImageEffect_Despeckle::prepareEffect()
{
    m_radiusInput->setEnabled(false);
    m_blackLevelInput->setEnabled(false);
    m_whiteLevelInput->setEnabled(false);
    m_useAdaptativeMethod->setEnabled(false);
    m_useRecursiveMethod->setEnabled(false);
    
    QImage img = m_imagePreviewWidget->getOriginalClipImage();
   
    int  r  = m_radiusInput->value();
    int  bl = m_blackLevelInput->value();
    int  wl = m_whiteLevelInput->value();
    bool af = m_useAdaptativeMethod->isChecked();
    bool rf = m_useRecursiveMethod->isChecked();
    
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new Despeckle(&img, this, r, bl, wl, af, rf));
}

void ImageEffect_Despeckle::prepareFinal()
{
    m_radiusInput->setEnabled(false);
    m_blackLevelInput->setEnabled(false);
    m_whiteLevelInput->setEnabled(false);
    m_useAdaptativeMethod->setEnabled(false);
    m_useRecursiveMethod->setEnabled(false);
    
    int  r  = m_radiusInput->value();
    int  bl = m_blackLevelInput->value();
    int  wl = m_whiteLevelInput->value();
    bool af = m_useAdaptativeMethod->isChecked();
    bool rf = m_useRecursiveMethod->isChecked();

    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
            
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new Despeckle(&orgImage, 
                                                               this, r, bl, wl, af, rf));
    delete [] data;
}

void ImageEffect_Despeckle::putPreviewData(void)
{
    QImage imDest = m_threadedFilter->getTargetImage();
    m_imagePreviewWidget->setPreviewImageData(imDest);
}

void ImageEffect_Despeckle::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalData(i18n("Noise Reduction"), 
                        (uint*)m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamNoiseReductionImagesPlugin

#include "imageeffect_despeckle.moc"
