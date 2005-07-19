/* ============================================================
 * File  : imageeffect_sheartool.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-23
 * Description : a digiKam image editor plugin to process 
 *               shearing image.
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
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qimage.h>

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
#include "sheartool.h"
#include "imageeffect_sheartool.h"

namespace DigikamShearToolImagesPlugin
{

ImageEffect_ShearTool::ImageEffect_ShearTool(QWidget* parent)
                     : ImageGuideDialog(parent, i18n("Shear Tool"), 
                                        "sheartool", false, false, true)
{
    QString whatsThis;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Shear Tool"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to shear an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    setAboutData(about);
    
    QWhatsThis::add( m_imagePreviewWidget, i18n("<p>This is the shearing image operation preview. "
                                           "If you move the mouse cursor on this preview, "
                                           "a vertical and horizontal dashed line will be drawn "
                                           "to guide you in adjusting the shearing correction. "
                                           "Release the left mouse button to freeze the dashed "
                                           "line's position."));
                                           
    // -------------------------------------------------------------
    
    QWidget *gboxSettings = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 5, 2, marginHint(), spacingHint());
    
    QLabel *label1 = new QLabel(i18n("New Width:"), gboxSettings);
    m_newWidthLabel = new QLabel(gboxSettings);
    m_newWidthLabel->setAlignment( AlignBottom | AlignRight );
    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_newWidthLabel, 0, 0, 1, 2);
    
    QLabel *label2 = new QLabel(i18n("New Height:"), gboxSettings);
    m_newHeightLabel = new QLabel(gboxSettings);
    m_newHeightLabel->setAlignment( AlignBottom | AlignRight );
    gridSettings->addMultiCellWidget(label2, 1, 1, 0, 0);
    gridSettings->addMultiCellWidget(m_newHeightLabel, 1, 1, 1, 2);
    
    QLabel *label3 = new QLabel(i18n("Horizontal Angle:"), gboxSettings);
    m_magnitudeX = new KDoubleNumInput(gboxSettings);
    m_magnitudeX->setPrecision(1);
    m_magnitudeX->setRange(-45.0, 45.0, 0.1, true);
    m_magnitudeX->setValue(0.0);
    QWhatsThis::add( m_magnitudeX, i18n("<p>The horizontal shearing angle, in degrees."));
    gridSettings->addMultiCellWidget(label3, 2, 2, 0, 2);
    gridSettings->addMultiCellWidget(m_magnitudeX, 3, 3, 0, 2);
            
    QLabel *label4 = new QLabel(i18n("Vertical Angle:"), gboxSettings);
    m_magnitudeY = new KDoubleNumInput(gboxSettings);
    m_magnitudeY->setPrecision(1);
    m_magnitudeY->setRange(-45.0, 45.0, 0.1, true);
    m_magnitudeY->setValue(0.0);
    QWhatsThis::add( m_magnitudeY, i18n("<p>The vertical shearing angle, in degrees."));
    gridSettings->addMultiCellWidget(label4, 4, 4, 0, 0);
    gridSettings->addMultiCellWidget(m_magnitudeY, 5, 5, 0, 2);
            
    setUserAreaWidget(gboxSettings); 

    // -------------------------------------------------------------
    
    connect(m_magnitudeX, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));
    
    connect(m_magnitudeY, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));
}

ImageEffect_ShearTool::~ImageEffect_ShearTool()
{
}

void ImageEffect_ShearTool::renderingFinished()
{
    m_magnitudeX->setEnabled(true);
    m_magnitudeY->setEnabled(true);
}

void ImageEffect_ShearTool::resetValues()
{
    m_magnitudeX->blockSignals(true);
    m_magnitudeY->blockSignals(true);
    m_magnitudeX->setValue(0.0);
    m_magnitudeY->setValue(0.0);
    m_magnitudeX->blockSignals(false);
    m_magnitudeY->blockSignals(false);
} 

void ImageEffect_ShearTool::prepareEffect()
{
    m_magnitudeX->setEnabled(false);
    m_magnitudeY->setEnabled(false);

    float hAngle = m_magnitudeX->value();
    float vAngle = m_magnitudeY->value();

    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    int orgW = iface->originalWidth();
    int orgH = iface->originalHeight();
    QImage image(iface->previewWidth(), iface->previewHeight(), 32);
    uint *data = iface->getPreviewData();
    memcpy( image.bits(), data, image.numBytes() );
    
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(
                       new ShearTool(&image, this, hAngle, vAngle, false, orgW, orgH));    
    delete [] data;
}

void ImageEffect_ShearTool::prepareFinal()
{
    m_magnitudeX->setEnabled(false);
    m_magnitudeY->setEnabled(false);

    float hAngle = m_magnitudeX->value();
    float vAngle = m_magnitudeY->value();
        
    Digikam::ImageIface iface(0, 0);
    int orgW = iface.originalWidth();
    int orgH = iface.originalHeight();
    QImage orgImage(orgW, orgH, 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(
                       new ShearTool(&orgImage, this, hAngle, vAngle, false, orgW, orgH));    
    delete [] data;       
}

void ImageEffect_ShearTool::putPreviewData(void)
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
    QSize newSize = dynamic_cast<ShearTool *>(m_threadedFilter)->getNewSize();
    QString temp;
    m_newWidthLabel->setText(temp.setNum( newSize.width()) + i18n(" px") );
    m_newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
}

void ImageEffect_ShearTool::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);    
    QImage targetImage = m_threadedFilter->getTargetImage();
    iface.putOriginalData(i18n("Shear Tool"), 
                          (uint*)targetImage.bits(), 
                          targetImage.width(), targetImage.height());
}

/*
void ImageEffect_ShearTool::slotEffect()
{
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data   = iface->getPreviewData();
    int   w      = iface->previewWidth();
    int   h      = iface->previewHeight();
    float hAngle = m_magnitudeX->value();
    float vAngle = m_magnitudeY->value();
   
    QImage src, dest;
    QWMatrix matrix;
    src.create( w, h, 32 );
    memcpy(src.bits(), data, src.numBytes());
    matrix.shear( tan(DEGREES_TO_RADIANS(hAngle) ), tan(DEGREES_TO_RADIANS(vAngle) ));
    src = src.xForm(matrix);
    QSize newSize = matrix.mapRect(QRect::QRect(0, 0, iface->originalWidth(), iface->originalHeight())).size();
    src = src.smoothScale(w, h, QImage::ScaleMin);
    dest.create( w, h, 32 );
    dest.fill(m_previewWidget->colorGroup().background().rgb());
    bitBlt( &dest, (w-src.width())/2, (h-src.height())/2, &src, 0, 0, src.width(), src.height());
    iface->putPreviewData((uint*)dest.bits());

    delete [] data;
    m_previewWidget->update();
    
    QString temp;
    m_newWidthLabel->setText(temp.setNum( newSize.width() ) + i18n(" px") );
    m_newHeightLabel->setText(temp.setNum( newSize.height() ) + i18n(" px") );
}

void ImageEffect_ShearTool::slotOk()
{
    accept(); 
    kapp->setOverrideCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
        
    uint* data   = iface.getOriginalData();
    int   w      = iface.originalWidth();
    int   h      = iface.originalHeight();
    float hAngle = m_magnitudeX->value();
    float vAngle = m_magnitudeY->value();

    QImage src;
    QWMatrix matrix;
    src.create( w, h, 32 );
    memcpy(src.bits(), data, src.numBytes());
    matrix.shear( tan(DEGREES_TO_RADIANS(hAngle) ), tan(DEGREES_TO_RADIANS(vAngle) ));
    src = src.xForm(matrix);
    Digikam::ImageFilters::gaussianBlurImage((uint*)src.bits(), src.width(), src.height(), 1);
    iface.putOriginalData(i18n("Shear Tool"), (uint*)src.bits(), src.width(), src.height());
        
    delete [] data;
    kapp->restoreOverrideCursor();
}
*/
}  // NameSpace DigikamShearToolImagesPlugin

#include "imageeffect_sheartool.moc"
