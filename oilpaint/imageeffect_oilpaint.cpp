/* ============================================================
 * File  : imageeffect_oilpaint.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-25
 * Description : a digiKam image editor plugin to simulate 
 *               an oil painting.
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
#include <qimage.h>
#include <qlayout.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <knuminput.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "oilpaint.h"
#include "imageeffect_oilpaint.h"

namespace DigikamOilPaintImagesPlugin
{

ImageEffect_OilPaint::ImageEffect_OilPaint(QWidget* parent)
                    : CtrlPanelDialog(parent, i18n("Oil Paint"), "oilpaint")
{
    QString whatsThis;
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Oil Paint"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("An oil painting image effect plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Oil paint algorithm"), 
                     "pieter_voloshyn at ame.com.br");                     
    
    setAboutData(about);
    
    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 2, 2, marginHint(), spacingHint());
    QLabel *label1 = new QLabel(i18n("Brush size:"), gboxSettings);
    
    m_brushSizeInput = new KIntNumInput(gboxSettings);
    m_brushSizeInput->setRange(1, 5, 1, true);
    QWhatsThis::add( m_brushSizeInput, i18n("<p>Set here the brush size to use for "
                                            "simulating the oil painting.") );
    
    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_brushSizeInput, 0, 0, 1, 1);
        
    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Smooth:"), gboxSettings);
    
    m_smoothInput = new KIntNumInput(gboxSettings);
    m_smoothInput->setRange(10, 255, 1, true);
    QWhatsThis::add( m_smoothInput, i18n("<p>This value controls the smoothing effect "
                                         "of the brush under the canvas.") );

    gridSettings->addMultiCellWidget(label2, 1, 1, 0, 0);
    gridSettings->addMultiCellWidget(m_smoothInput, 1, 1, 1, 1);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
            
    // -------------------------------------------------------------

    connect(m_brushSizeInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));            
            
    connect(m_smoothInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));         
}

ImageEffect_OilPaint::~ImageEffect_OilPaint()
{
}

void ImageEffect_OilPaint::renderingFinished()
{
    m_brushSizeInput->setEnabled(true);
    m_smoothInput->setEnabled(true);
}

void ImageEffect_OilPaint::resetValues()
{
    m_brushSizeInput->blockSignals(true);
    m_smoothInput->blockSignals(true);
    m_brushSizeInput->setValue(1);
    m_smoothInput->setValue(30);
    m_brushSizeInput->blockSignals(false);
    m_smoothInput->blockSignals(false);
} 

void ImageEffect_OilPaint::prepareEffect()
{
    m_brushSizeInput->setEnabled(false);
    m_smoothInput->setEnabled(false);
    
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    
    int b        = m_brushSizeInput->value();
    int s        = m_smoothInput->value();
        
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new OilPaint(&image, this, b, s));
}

void ImageEffect_OilPaint::prepareFinal()
{
    m_brushSizeInput->setEnabled(false);
    m_smoothInput->setEnabled(false);
    
    int b = m_brushSizeInput->value();
    int s = m_smoothInput->value();
        
    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
            
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new OilPaint(&orgImage, this, b, s));
    delete [] data;
}

void ImageEffect_OilPaint::putPreviewData(void)
{
    QImage imDest = m_threadedFilter->getTargetImage();
    m_imagePreviewWidget->setPreviewImageData(imDest);
}

void ImageEffect_OilPaint::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalData(i18n("Oil Paint"), 
                        (uint*)m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamOilPaintImagesPlugin

#include "imageeffect_oilpaint.moc"
