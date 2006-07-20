/* ============================================================
 * File  : imageeffect_emboss.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a digiKam image editor plugin to emboss 
 *               an image.
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
#include <qlayout.h>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <knuminput.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "emboss.h"
#include "imageeffect_emboss.h"

namespace DigikamEmbossImagesPlugin
{

ImageEffect_Emboss::ImageEffect_Emboss(QWidget* parent)
                    : CtrlPanelDialog(parent, i18n("Emboss Image"), "emboss")
{
    QString whatsThis;
        
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Emboss Image"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("An embossed image effect plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Emboss algorithm"), 
                     "pieter_voloshyn at ame.com.br");         
                                          
    setAboutData(about);
    
    // -------------------------------------------------------------
    
    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 1, 2, marginHint(), spacingHint());
    QLabel *label1 = new QLabel(i18n("Depth:"), gboxSettings);
    
    m_depthInput = new KIntNumInput(gboxSettings);
    m_depthInput->setRange(10, 300, 1, true);
    QWhatsThis::add( m_depthInput, i18n("<p>Set here the depth of the embossing image effect.") );
                                            
    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_depthInput, 0, 0, 1, 1);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    
    connect(m_depthInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer())); 
}

ImageEffect_Emboss::~ImageEffect_Emboss()
{
}

void ImageEffect_Emboss::renderingFinished()
{
    m_depthInput->setEnabled(true);
}

void ImageEffect_Emboss::resetValues()
{
    m_depthInput->blockSignals(true);
    m_depthInput->setValue(30);
    m_depthInput->blockSignals(false);
} 

void ImageEffect_Emboss::prepareEffect()
{
    m_depthInput->setEnabled(false);
    
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    
    int depth = m_depthInput->value();
            
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new Emboss(&image, this, depth));
}

void ImageEffect_Emboss::prepareFinal()
{
    m_depthInput->setEnabled(false);
    
    int depth = m_depthInput->value();
    
    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
            
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new Emboss(&orgImage, this, depth));
    delete [] data;
}

void ImageEffect_Emboss::putPreviewData(void)
{
    QImage imDest = m_threadedFilter->getTargetImage();
    m_imagePreviewWidget->setPreviewImageData(imDest);
}

void ImageEffect_Emboss::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalData(i18n("Emboss"), 
                        (uint*)m_threadedFilter->getTargetImage().bits());
}
    
}  // NameSpace DigikamEmbossImagesPlugin

#include "imageeffect_emboss.moc"
