/* ============================================================
 * File  : imageeffect_freerotation.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-11-28
 * Description : a digiKam image editor plugin to process image 
 *               free rotation.
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

// C++ include.

#include <cstring>
#include <cmath>
#include <cstdlib>
 
// Qt includes. 
 
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kdebug.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "freerotation.h"
#include "imageeffect_freerotation.h"

namespace DigikamFreeRotationImagesPlugin
{

ImageEffect_FreeRotation::ImageEffect_FreeRotation(QWidget* parent)
                        : ImageGuideDialog(parent, i18n("Free Rotation"), 
                                           "freerotation", false, false, true)
{
    QString whatsThis;
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Free Rotation"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to process free image "
                                       "rotation."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
            
    setAboutData(about);
    
    // -------------------------------------------------------------
        
    QWidget *gboxSettings = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 3, 2, marginHint(), spacingHint());
    
    QLabel *label1 = new QLabel(i18n("New Width:"), gboxSettings);
    m_newWidthLabel = new QLabel(gboxSettings);
    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_newWidthLabel, 0, 0, 1, 1);
    
    QLabel *label2 = new QLabel(i18n("New Height:"), gboxSettings);
    m_newHeightLabel = new QLabel(gboxSettings);
    gridSettings->addMultiCellWidget(label2, 1, 1, 0, 0);
    gridSettings->addMultiCellWidget(m_newHeightLabel, 1, 1, 1, 1);
    
    QLabel *label3 = new QLabel(i18n("Angle (in Degrees):"), gboxSettings);
    m_angleInput = new KDoubleNumInput(gboxSettings);
    m_angleInput->setPrecision(1);
    m_angleInput->setRange(-180.0, 180.0, 0.1, true);
    m_angleInput->setValue(0.0);
    QWhatsThis::add( m_angleInput, i18n("<p>An angle in degrees by which to rotate the image. "
                                        "A positive angle rotates the image clockwise; "
                                        "a negative angle rotates it counter-clockwise."));
        
    gridSettings->addMultiCellWidget(label3, 2, 2, 0, 0);
    gridSettings->addMultiCellWidget(m_angleInput, 3, 3, 0, 1);
    
    setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    
    connect(m_angleInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));        
}

ImageEffect_FreeRotation::~ImageEffect_FreeRotation()
{
}

void ImageEffect_FreeRotation::renderingFinished()
{
    m_angleInput->setEnabled(true);
}

void ImageEffect_FreeRotation::resetValues()
{
    m_angleInput->blockSignals(true);
    m_angleInput->setValue(0.0);
    m_angleInput->blockSignals(false);
} 

void ImageEffect_FreeRotation::prepareEffect()
{
    m_angleInput->setEnabled(false);

    double angle = m_angleInput->value();

    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    int orgW = iface->originalWidth();
    int orgH = iface->originalHeight();
    QImage image(iface->previewWidth(), iface->previewHeight(), 32);
    uint *data = iface->getPreviewData();
    memcpy( image.bits(), data, image.numBytes() );
    
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(
                       new FreeRotation(&image, this, angle, orgW, orgH));    
    delete [] data;
}

void ImageEffect_FreeRotation::prepareFinal()
{
    m_angleInput->setEnabled(false);
        
    double angle = m_angleInput->value();

    Digikam::ImageIface iface(0, 0);
    int orgW = iface.originalWidth();
    int orgH = iface.originalHeight();
    QImage orgImage(orgW, orgH, 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(
                       new FreeRotation(&orgImage, this, angle, orgW, orgH));
    delete [] data;       
}

void ImageEffect_FreeRotation::putPreviewData(void)
{
    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    int w = iface->previewWidth();
    int h = iface->previewHeight();
        
    QImage imTemp = m_threadedFilter->getTargetImage().smoothScale(w, h, QImage::ScaleMin);
    QImage imDest( w, h, 32 );
    bitBlt( &imDest, (w-imTemp.width())/2, (h-imTemp.height())/2, 
            &imTemp, 0, 0, imTemp.width(), imTemp.height());
            
    iface->putPreviewData((uint*)(imDest.smoothScale(iface->previewWidth(),
                                                     iface->previewHeight())).bits());
                 
    m_imagePreviewWidget->update();    
    QSize newSize = dynamic_cast<FreeRotation *>(m_threadedFilter)->getNewSize();
    QString temp;
    m_newWidthLabel->setText(temp.setNum( newSize.width()) + i18n(" px") );
    m_newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
}

void ImageEffect_FreeRotation::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);    
    QImage targetImage = m_threadedFilter->getTargetImage();
    iface.putOriginalData(i18n("Free Rotation"), 
                          (uint*)targetImage.bits(), 
                          targetImage.width(), targetImage.height());
}

}  // NameSpace DigikamFreeRotationImagesPlugin

#include "imageeffect_freerotation.moc"
