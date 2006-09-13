/* ============================================================
 * File  : imageeffect_unsharp.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-08-27
 * Description : Unsharp mask image filter for digiKam Image Editor
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

// Local includes.

#include "version.h"
#include "unsharp.h"
#include "imageeffect_unsharp.h"

namespace DigikamUnsharpMaskImagesPlugin
{

ImageEffect_Unsharp::ImageEffect_Unsharp(QWidget* parent, QString title, QFrame* banner)
                   : Digikam::CtrlPanelDlg(parent, title, "unsharp", false,
                                           false, true, Digikam::ImagePannelWidget::SeparateViewAll, 
                                           banner)
{
    QString whatsThis;
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Unsharp Mask"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("An unsharp mask image filter plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2006, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at kdemail dot net");

    setAboutData(about);
    
    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 5, 1, 0, spacingHint());

    QLabel *label1 = new QLabel(i18n("Radius:"), gboxSettings);
    
    m_radiusInput = new KIntNumInput(gboxSettings, "m_radiusInput");
    m_radiusInput->setRange(1, 120, 1, true);
            
    QWhatsThis::add( m_radiusInput, i18n("<p>Radius value is the gaussian blur matrix radius value "
                                         "used to determines how much to blur the image.") );
    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 1);
    gridSettings->addMultiCellWidget(m_radiusInput, 1, 1, 0, 1);
    
    // -------------------------------------------------------------
    
    QLabel *label2 = new QLabel(i18n("Amount:"), gboxSettings);
    
    m_amountInput = new KDoubleNumInput(gboxSettings, "m_amountInput");
    m_amountInput->setPrecision(1);
    m_amountInput->setRange(0.0, 5.0, 0.1, true);
            
    QWhatsThis::add( m_amountInput, i18n("<p>The value of the difference between the "
                     "original and the blur image that is added back into the original.") );
    gridSettings->addMultiCellWidget(label2, 2, 2, 0, 1);
    gridSettings->addMultiCellWidget(m_amountInput, 3, 3, 0, 1);
        
    // -------------------------------------------------------------
    
    QLabel *label3 = new QLabel(i18n("Threshold:"), gboxSettings);
    
    m_thresholdInput = new KDoubleNumInput(gboxSettings, "m_thresholdInput");
    m_thresholdInput->setPrecision(2);
    m_thresholdInput->setRange(0.0, 1.0, 0.01, true);
        
    QWhatsThis::add( m_thresholdInput, i18n("<p>The threshold, as a fraction of the maximum "
                     "luminosity value, needed to apply the difference amount.") );
    gridSettings->addMultiCellWidget(label3, 4, 4, 0, 1);
    gridSettings->addMultiCellWidget(m_thresholdInput, 5, 5, 0, 1);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    
    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));                                                

    connect(m_amountInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                                                            
            
    connect(m_thresholdInput, SIGNAL(valueChanged (double)),
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
    m_radiusInput->setValue(1);
    m_amountInput->setValue(1.0);
    m_thresholdInput->setValue(0.05);
    m_radiusInput->blockSignals(false);
    m_amountInput->blockSignals(false);
    m_thresholdInput->blockSignals(false);
} 

void ImageEffect_Unsharp::prepareEffect()
{
    m_radiusInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    
    Digikam::DImg img = m_imagePreviewWidget->getOriginalRegionImage();

    int    r  = m_radiusInput->value();
    double a  = m_amountInput->value();
    double th = m_thresholdInput->value();
    
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>
                       (new UnsharpMask(&img, this, r, a, th));
}

void ImageEffect_Unsharp::prepareFinal()
{
    m_radiusInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
        
    int    r  = m_radiusInput->value();
    double a  = m_amountInput->value();
    double th = m_thresholdInput->value();
    
    Digikam::ImageIface iface(0, 0);
    uchar *data     = iface.getOriginalImage();
    int w           = iface.originalWidth();
    int h           = iface.originalHeight();
    bool sixteenBit = iface.originalSixteenBit();
    bool hasAlpha   = iface.originalHasAlpha();
    Digikam::DImg orgImage = Digikam::DImg(w, h, sixteenBit, hasAlpha ,data);
    delete [] data;
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>
                       (new UnsharpMask(&orgImage, this, r, a, th));
}

void ImageEffect_Unsharp::putPreviewData(void)
{
    Digikam::DImg imDest = m_threadedFilter->getTargetImage();
    m_imagePreviewWidget->setPreviewImage(imDest);
}

void ImageEffect_Unsharp::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
    Digikam::DImg imDest = m_threadedFilter->getTargetImage();
    iface.putOriginalImage(i18n("Unsharp Mask"), imDest.bits());
}

}  // NameSpace DigikamUnsharpMaskImagesPlugin

#include "imageeffect_unsharp.moc"
