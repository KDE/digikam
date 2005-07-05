/* ============================================================
 * File  : imageeffect_unsharp.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-27
 * Description : Unsharp mask image filter for digiKam Image Editor
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
#include <qimage.h>
#include <qstring.h>
#include <qlayout.h>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "unsharp.h"
#include "imageeffect_unsharp.h"

namespace DigikamUnsharpMaskImagesPlugin
{

ImageEffect_Unsharp::ImageEffect_Unsharp(QWidget* parent)
                   : CtrlPanelDialog(parent, i18n("Unsharp Mask"), "unsharp")
{
    QString whatsThis;
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Unsharp Mask"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("An unsharp mask image filter plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Winston Chang", I18N_NOOP("Unsharp mask algorithm author from Gimp"),
                     "winstonc at cs.wisc.edu");
                        
    setAboutData(about);
    
    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 3, 2, marginHint(), spacingHint());
    QLabel *label1 = new QLabel(i18n("Radius:"), gboxSettings);
    
    m_radiusInput = new KDoubleNumInput(gboxSettings, "m_radiusInput");
    m_radiusInput->setPrecision(1);
    m_radiusInput->setRange(0.1, 120.0, 0.1, true);
            
    QWhatsThis::add( m_radiusInput, i18n("<p>A radius of 0 has no effect, "
                     "10 and above determine the blur matrix radius "
                     "that determines how much to blur the image.") );
    gridSettings->addWidget(label1, 0, 0);
    gridSettings->addWidget(m_radiusInput, 0, 1);
    
    // -------------------------------------------------------------
    
    QLabel *label2 = new QLabel(i18n("Amount:"), gboxSettings);
    
    m_amountInput = new KDoubleNumInput(gboxSettings, "m_amountInput");
    m_amountInput->setPrecision(2);
    m_amountInput->setRange(0.0, 5.0, 0.01, true);
            
    QWhatsThis::add( m_amountInput, i18n("<p>The value of the difference between the "
                     "original and the blur image that is added back into the original.") );
    gridSettings->addWidget(label2, 1, 0);
    gridSettings->addWidget(m_amountInput, 1, 1);
        
    // -------------------------------------------------------------
    
    QLabel *label3 = new QLabel(i18n("Threshold:"), gboxSettings);
    
    m_thresholdInput = new KIntNumInput(gboxSettings, "m_thresholdInput");
    m_thresholdInput->setRange(0, 255, 1, true);
        
    QWhatsThis::add( m_thresholdInput, i18n("<p>The threshold, as a fraction of the maximum "
                     "luminosity value, needed to apply the difference amount.") );
    gridSettings->addWidget(label3, 3, 0);
    gridSettings->addWidget(m_thresholdInput, 3, 1);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    
    connect(m_radiusInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                                                

    connect(m_amountInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                                                            
            
    connect(m_thresholdInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));                                                
}

ImageEffect_Unsharp::~ImageEffect_Unsharp()
{
}

void ImageEffect_Unsharp::renderingFinished()
{
    m_radiusInput->setEnabled(true);
    m_amountInput->setEnabled(true);
    m_thresholdInput->setEnabled(true);
}

void ImageEffect_Unsharp::resetValues()
{
    m_radiusInput->blockSignals(true);
    m_amountInput->blockSignals(true);
    m_thresholdInput->blockSignals(true);
    m_radiusInput->setValue(5.0);
    m_amountInput->setValue(0.5);
    m_thresholdInput->setValue(0);
    m_radiusInput->blockSignals(false);
    m_amountInput->blockSignals(false);
    m_thresholdInput->blockSignals(false);
} 

void ImageEffect_Unsharp::prepareEffect()
{
    m_radiusInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    
    QImage img = m_imagePreviewWidget->getOriginalClipImage();
   
    double r  = m_radiusInput->value();
    double a  = m_amountInput->value();
    int    th = m_thresholdInput->value();
    
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new UnsharpMask(&img, this, r, a, th));
}

void ImageEffect_Unsharp::prepareFinal()
{
    m_radiusInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
        
    double r  = m_radiusInput->value();
    double a  = m_amountInput->value();
    int    th = m_thresholdInput->value();
    
    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
            
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new UnsharpMask(&orgImage, this, r, a, th));
    delete [] data;
}

void ImageEffect_Unsharp::putPreviewData(void)
{
    QImage imDest = m_threadedFilter->getTargetImage();
    m_imagePreviewWidget->setPreviewImageData(imDest);
}

void ImageEffect_Unsharp::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalData(i18n("Unsharp Mask"), 
                        (uint*)m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamUnsharpMaskImagesPlugin

#include "imageeffect_unsharp.moc"
