/* ============================================================
 * File  : imageeffect_charcoal.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a digikam image editor plugin for 
 *               simulate charcoal drawing.
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
#include <knuminput.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "charcoal.h"
#include "imageeffect_charcoal.h"

namespace DigikamCharcoalImagesPlugin
{

ImageEffect_Charcoal::ImageEffect_Charcoal(QWidget* parent)
                    : CtrlPanelDialog(parent, i18n("Charcoal Drawing"), "charcoal")
{
    QString whatsThis;
        
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Charcoal Drawing"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A charcoal drawing image effect plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    setAboutData(about);
        
    // -------------------------------------------------------------
    
    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 2, 2, marginHint(), spacingHint());
    QLabel *label1 = new QLabel(i18n("Pencil size:"), gboxSettings);
    
    m_pencilInput = new KIntNumInput(gboxSettings);
    m_pencilInput->setRange(1, 100, 1, true);  
    m_pencilInput->setValue(5);
    QWhatsThis::add( m_pencilInput, i18n("<p>Set here the charcoal pencil size used to simulate the drawing."));

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_pencilInput, 0, 0, 1, 1);
    
    // -------------------------------------------------------------
    
    QLabel *label2 = new QLabel(i18n("Smooth:"), gboxSettings);
    
    m_smoothInput = new KIntNumInput(gboxSettings);
    m_smoothInput->setRange(1, 100, 1, true);  
    m_smoothInput->setValue(10);
    QWhatsThis::add( m_smoothInput, i18n("<p>This value controls the smoothing effect of the pencil "
                                         "under the canvas."));

    gridSettings->addMultiCellWidget(label2, 1, 1, 0, 0);
    gridSettings->addMultiCellWidget(m_smoothInput, 1, 1, 1, 1);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    
    connect(m_pencilInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));      
                
    connect(m_smoothInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));      
}

ImageEffect_Charcoal::~ImageEffect_Charcoal()
{
}

void ImageEffect_Charcoal::renderingFinished()
{
    m_pencilInput->setEnabled(true);
    m_smoothInput->setEnabled(true);
}

void ImageEffect_Charcoal::resetValues()
{
    m_pencilInput->blockSignals(true);
    m_smoothInput->blockSignals(true);
    m_pencilInput->setValue(5);
    m_smoothInput->setValue(10);
    m_pencilInput->blockSignals(false);
    m_smoothInput->blockSignals(false);
} 

void ImageEffect_Charcoal::prepareEffect()
{
    m_pencilInput->setEnabled(false);
    m_smoothInput->setEnabled(false);
            
    double pencil = (double)m_pencilInput->value();
    double smooth = (double)m_smoothInput->value();
    
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new Charcoal(&image, this, pencil, smooth));
}

void ImageEffect_Charcoal::prepareFinal()
{
    m_pencilInput->setEnabled(false);
    m_smoothInput->setEnabled(false);
    
    double pencil = (double)m_pencilInput->value();
    double smooth = (double)m_smoothInput->value();

    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new Charcoal(&orgImage, this, pencil, smooth));
    delete [] data;
}

void ImageEffect_Charcoal::putPreviewData(void)
{
    QImage imDest = m_threadedFilter->getTargetImage();
    m_imagePreviewWidget->setPreviewImageData(imDest);
}

void ImageEffect_Charcoal::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalData(i18n("Charcoal"), 
                        (uint*)m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamCharcoalImagesPlugin

#include "imageeffect_charcoal.moc"
