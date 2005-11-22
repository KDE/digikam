/* ============================================================
 * File  : imageeffect_lensdistortion.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-27
 * Description : a digiKam image plugin for to reduce spherical
 *               aberration provide by lens on an image.
 * 
 * Copyright 2004-2005 by Gilles Caulier
 *
 * Original Distortion Correction algorithm copyrighted 
 * 2001-2003 David Hodson <hodsond@acm.org>
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
#include <kconfig.h>
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
#include "lensdistortion.h"
#include "imageeffect_lensdistortion.h"

namespace DigikamLensDistortionImagesPlugin
{

ImageEffect_LensDistortion::ImageEffect_LensDistortion(QWidget* parent)
                          : ImageGuideDialog(parent, i18n("Lens Distortion Correction"), 
                                             "lensdistortion")
{
    QString whatsThis;
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Lens Distortion Correction"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to reduce spherical aberration caused "
                                                 "by a lens to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("David Hodson", I18N_NOOP("Lens distortion correction algorithm."),
                     "hodsond at acm dot org");
            
    setAboutData(about);
    
    // -------------------------------------------------------------
        
    QWidget *gboxSettings = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 5, 2, marginHint(), spacingHint());
    
    m_maskPreviewLabel = new QLabel( gboxSettings );
    m_maskPreviewLabel->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    QWhatsThis::add( m_maskPreviewLabel, i18n("<p>You can see here a thumbnail preview of the distortion correction "
                                              "applied to a cross pattern.") );
    gridSettings->addMultiCellWidget(m_maskPreviewLabel, 0, 0, 0, 2);
        
    // -------------------------------------------------------------
    
    QLabel *label1 = new QLabel(i18n("Main:"), gboxSettings);
    
    m_mainInput = new KDoubleNumInput(gboxSettings);
    m_mainInput->setPrecision(1);
    m_mainInput->setRange(-100.0, 100.0, 0.1, true);
    QWhatsThis::add( m_mainInput, i18n("<p>This value controls the amount of distortion. Negative values correct lens barrel "
                                       "distortion, while positive values correct lens pincushion distortion."));

    gridSettings->addMultiCellWidget(label1, 1, 1, 0, 0);
    gridSettings->addMultiCellWidget(m_mainInput, 1, 1, 1, 2);
    
    // -------------------------------------------------------------
    
    QLabel *label2 = new QLabel(i18n("Edge:"), gboxSettings);
    
    m_edgeInput = new KDoubleNumInput(gboxSettings);
    m_edgeInput->setPrecision(1);
    m_edgeInput->setRange(-100.0, 100.0, 0.1, true);
    QWhatsThis::add( m_edgeInput, i18n("<p>This value controls in the same manner as the Main control, but has more effect "
                                       "at the edges of the image than at the center."));

    gridSettings->addMultiCellWidget(label2, 2, 2, 0, 0);
    gridSettings->addMultiCellWidget(m_edgeInput, 2, 2, 1, 2);
    
    // -------------------------------------------------------------
    
    QLabel *label3 = new QLabel(i18n("Zoom:"), gboxSettings);
    
    m_rescaleInput = new KDoubleNumInput(gboxSettings);
    m_rescaleInput->setPrecision(1);
    m_rescaleInput->setRange(-100.0, 100.0, 0.1, true);
    QWhatsThis::add( m_rescaleInput, i18n("<p>This value rescales the overall image size."));
    
    gridSettings->addMultiCellWidget(label3, 3, 3, 0, 0);
    gridSettings->addMultiCellWidget(m_rescaleInput, 3, 3, 1, 2);

    // -------------------------------------------------------------
    
    QLabel *label4 = new QLabel(i18n("Brighten:"), gboxSettings);
    
    m_brightenInput = new KDoubleNumInput(gboxSettings);
    m_brightenInput->setPrecision(1);
    m_brightenInput->setRange(-100.0, 100.0, 0.1, true);
    QWhatsThis::add( m_brightenInput, i18n("<p>This value adjust the brightness in image corners."));

    gridSettings->addMultiCellWidget(label4, 4, 4, 0, 0);
    gridSettings->addMultiCellWidget(m_brightenInput, 4, 4, 1, 2);
    
    setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    
    connect(m_mainInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));            
            
    connect(m_edgeInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));            

    connect(m_rescaleInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));            

    connect(m_brightenInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));           
}

ImageEffect_LensDistortion::~ImageEffect_LensDistortion()
{
}

void ImageEffect_LensDistortion::readUserSettings(void)
{
    m_mainInput->blockSignals(true);
    m_edgeInput->blockSignals(true);
    m_rescaleInput->blockSignals(true);
    
    KConfig *config = kapp->config();
    config->setGroup("Lens Distortion Tool Settings");

    m_mainInput->setValue( config->readDoubleNumEntry( "2nd order distortion", 0.0 ) );
    m_edgeInput->setValue( config->readDoubleNumEntry("4th order distortion",0.0) );
    m_rescaleInput->setValue( config->readDoubleNumEntry( "Zoom factor", 0.0 ) );
    kdDebug() << "Reading LensDistortion settings" << endl;
    
    m_mainInput->blockSignals(false);
    m_edgeInput->blockSignals(false);
    m_rescaleInput->blockSignals(false);
    
    slotEffect();
}

void ImageEffect_LensDistortion::writeUserSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("Lens Distortion Tool Settings");
    config->writeEntry( "2nd order distortion", m_mainInput->value() );
    config->writeEntry( "4th order distortion", m_edgeInput->value() );
    config->writeEntry( "Zoom factor", m_rescaleInput->value() );
    config->sync();
    kdDebug() << "Writing LensDistortion settings" << endl;
}

void ImageEffect_LensDistortion::renderingFinished()
{
    m_mainInput->setEnabled(true);
    m_edgeInput->setEnabled(true);
    m_rescaleInput->setEnabled(true);
    m_brightenInput->setEnabled(true);
}

void ImageEffect_LensDistortion::resetValues()
{
    m_mainInput->blockSignals(true);
    m_edgeInput->blockSignals(true);
    m_rescaleInput->blockSignals(true);
    m_brightenInput->blockSignals(true);
    
    m_mainInput->setValue(0.0);
    m_edgeInput->setValue(0.0);
    m_rescaleInput->setValue(0.0);
    m_brightenInput->setValue(0.0);
    
    m_mainInput->blockSignals(false);
    m_edgeInput->blockSignals(false);
    m_rescaleInput->blockSignals(false);
    m_brightenInput->blockSignals(false);
} 

void ImageEffect_LensDistortion::prepareEffect()
{
    m_mainInput->setEnabled(false);
    m_edgeInput->setEnabled(false);
    m_rescaleInput->setEnabled(false);
    m_brightenInput->setEnabled(false);

    double m = m_mainInput->value();
    double e = m_edgeInput->value();
    double r = m_rescaleInput->value();
    double b = m_brightenInput->value();

    // Calc transform preview.    
    QImage preview(120, 120, 32);
    memset(preview.bits(), 255, preview.numBytes());
    QPixmap pix (preview);
    QPainter pt(&pix);
    pt.setPen( QPen::QPen(Qt::black, 1) ); 
    pt.fillRect( 0, 0, pix.width(), pix.height(), QBrush::QBrush(Qt::black, Qt::CrossPattern) );
    pt.drawRect( 0, 0, pix.width(), pix.height() );
    pt.end();
    QImage preview2(pix.convertToImage());
    LensDistortion transformPreview(&preview2, 0L, m, e, r, b, 0, 0);
    m_maskPreviewLabel->setPixmap(QPixmap::QPixmap(transformPreview.getTargetImage()));

    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    QImage orgImage(iface->originalWidth(), iface->originalHeight(), 32);
    uint *data = iface->getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(
                       new LensDistortion(&orgImage, this, m, e, r, b, 0, 0));    
    delete [] data;
}

void ImageEffect_LensDistortion::prepareFinal()
{
    m_mainInput->setEnabled(false);
    m_edgeInput->setEnabled(false);
    m_rescaleInput->setEnabled(false);
    m_brightenInput->setEnabled(false);
        
    double m = m_mainInput->value();
    double e = m_edgeInput->value();
    double r = m_rescaleInput->value();
    double b = m_brightenInput->value();

    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(
                       new LensDistortion(&orgImage, this, m, e, r, b, 0, 0));
    delete [] data;       
}

void ImageEffect_LensDistortion::putPreviewData(void)
{
    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    
    QImage imDest = m_threadedFilter->getTargetImage();
    iface->putPreviewData((uint*)(imDest.smoothScale(iface->previewWidth(),
                                                     iface->previewHeight())).bits());
                 
    m_imagePreviewWidget->updatePreview();    
}

void ImageEffect_LensDistortion::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);    

    iface.putOriginalData(i18n("Lens Distortion"), 
                          (uint*)m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamLensDistortionImagesPlugin

#include "imageeffect_lensdistortion.moc"
