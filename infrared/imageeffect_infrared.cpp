/* ============================================================
 * File  : imageeffect_infrared.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-22
 * Description : a digiKam image editor plugin for simulate 
 *               infrared film.
 * 
 * Copyright 2005 by Gilles Caulier
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

#include <qimage.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qlcdnumber.h>
#include <qslider.h>
#include <qlayout.h>
#include <qdatetime.h> 
#include <qcheckbox.h>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "infrared.h"
#include "imageeffect_infrared.h"

namespace DigikamInfraredImagesPlugin
{

ImageEffect_Infrared::ImageEffect_Infrared(QWidget* parent)
                     : CtrlPanelDialog(parent, i18n("Simulate Infrared Film to Photograph"), 
                                       "infrared")
{
    QString whatsThis;
        
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Infrared Film"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to simulate infrared film."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    setAboutData(about);
    
    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 3, 2, marginHint(), spacingHint());
    QLabel *label1 = new QLabel(i18n("Sensibility (ISO):"), gboxSettings);
    
    m_sensibilitySlider = new QSlider(1, 7, 1, 1, Qt::Horizontal, gboxSettings);
    m_sensibilitySlider->setTracking ( false );
    m_sensibilitySlider->setTickInterval(1);
    m_sensibilitySlider->setTickmarks(QSlider::Below);
    
    m_sensibilityLCDValue = new QLCDNumber (3, gboxSettings);
    m_sensibilityLCDValue->setSegmentStyle ( QLCDNumber::Flat );
    m_sensibilityLCDValue->display( QString::number(200) );
    whatsThis = i18n("<p>Set here the ISO-sensitivity of the simulated infrared film. "
                     "Increasing this value will increase the portion of green color in the mix. " 
                     "It will also increase the halo effect on the hightlights, and the film "
                     "graininess (if the box is checked).");
        
    QWhatsThis::add( m_sensibilityLCDValue, whatsThis);
    QWhatsThis::add( m_sensibilitySlider, whatsThis);

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_sensibilitySlider, 0, 0, 1, 1);
    gridSettings->addMultiCellWidget(m_sensibilityLCDValue, 0, 0, 2, 2);
    
    // -------------------------------------------------------------

    m_addFilmGrain = new QCheckBox( i18n("Add film grain"), gboxSettings);
    m_addFilmGrain->setChecked( true );
    QWhatsThis::add( m_addFilmGrain, i18n("<p>This option is adding infrared film grain to "
                                          "the image depending on ISO-sensitivity."));
    gridSettings->addMultiCellWidget(m_addFilmGrain, 1, 1, 0, 2);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
        
    // -------------------------------------------------------------

    connect( m_sensibilitySlider, SIGNAL(valueChanged(int)),
             this, SLOT(slotSensibilityChanged(int)) ); 
             
    connect( m_addFilmGrain, SIGNAL(toggled (bool)),
             this, SLOT(slotEffect()) );                        
}

ImageEffect_Infrared::~ImageEffect_Infrared()
{
}

void ImageEffect_Infrared::renderingFinished()
{
    m_sensibilitySlider->setEnabled(true);
    m_addFilmGrain->setEnabled(true);
}

void ImageEffect_Infrared::resetValues()
{
    m_sensibilitySlider->blockSignals(true);
    m_sensibilitySlider->setValue(1);
    slotSensibilityChanged(1);
    m_sensibilitySlider->blockSignals(false);
} 

void ImageEffect_Infrared::slotSensibilityChanged(int v)
{
    m_sensibilityLCDValue->display( QString::number(100 + 100 * v) );
    slotEffect();
}

void ImageEffect_Infrared::prepareEffect()
{
    m_addFilmGrain->setEnabled(false);
    m_sensibilitySlider->setEnabled(false);
        
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    int   s      = 100 + 100 * m_sensibilitySlider->value();
    bool  g      = m_addFilmGrain->isChecked();

    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new Infrared(&image, this, s, g));
}

void ImageEffect_Infrared::prepareFinal()
{
    m_addFilmGrain->setEnabled(false);
    m_sensibilitySlider->setEnabled(false);
    
    int  s    = 100 + 100 * m_sensibilitySlider->value();
    bool g    = m_addFilmGrain->isChecked();
               
    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new Infrared(&orgImage, this, s, g));
    delete [] data;
}

void ImageEffect_Infrared::putPreviewData(void)
{
    QImage imDest = m_threadedFilter->getTargetImage();
    m_imagePreviewWidget->setPreviewImageData(imDest);
}

void ImageEffect_Infrared::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalData(i18n("Infrared"), 
                        (uint*)m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamInfraredImagesPlugin

#include "imageeffect_infrared.moc"
