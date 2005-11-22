/* ============================================================
 * File  : imageeffect_filmgrain.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a digiKam image editor plugin for add film 
 *               grain on an image.
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
#include <qlcdnumber.h>
#include <qslider.h>
#include <qlayout.h>
#include <qimage.h>

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
#include "filmgrain.h"
#include "imageeffect_filmgrain.h"

namespace DigikamFilmGrainImagesPlugin
{

ImageEffect_FilmGrain::ImageEffect_FilmGrain(QWidget* parent)
                     : CtrlPanelDialog(parent, i18n("Add Film Grain to Photograph"), 
                                       "filmgrain")
{
    QString whatsThis;
        
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Film Grain"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to apply a film grain "
                                                 "effect to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    setAboutData(about);
    
    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 1, 2, marginHint(), spacingHint());
    QLabel *label1 = new QLabel(i18n("Sensibility (ISO):"), gboxSettings);
    
    m_sensibilitySlider = new QSlider(2, 30, 1, 12, Qt::Horizontal, gboxSettings);
    m_sensibilitySlider->setTracking ( false );
    m_sensibilitySlider->setTickInterval(1);
    m_sensibilitySlider->setTickmarks(QSlider::Below);
    
    m_sensibilityLCDValue = new QLCDNumber (4, gboxSettings);
    m_sensibilityLCDValue->setSegmentStyle ( QLCDNumber::Flat );
    m_sensibilityLCDValue->display( QString::number(2400) );
    whatsThis = i18n("<p>Set here the film ISO-sensitivity to use for simulating the film graininess.");
        
    QWhatsThis::add( m_sensibilityLCDValue, whatsThis);
    QWhatsThis::add( m_sensibilitySlider, whatsThis);

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_sensibilitySlider, 0, 0, 1, 1);
    gridSettings->addMultiCellWidget(m_sensibilityLCDValue, 0, 0, 2, 2);    
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------

    connect( m_sensibilitySlider, SIGNAL(valueChanged(int)),
             this, SLOT(slotSensibilityChanged(int)) ); 
}

ImageEffect_FilmGrain::~ImageEffect_FilmGrain()
{
}

void ImageEffect_FilmGrain::renderingFinished()
{
    m_sensibilitySlider->setEnabled(true);
}
 
void ImageEffect_FilmGrain::resetValues()
{
    m_sensibilitySlider->blockSignals(true);
    m_sensibilitySlider->setValue(12);
    slotSensibilityChanged(12);
    m_sensibilitySlider->blockSignals(false);
} 

void ImageEffect_FilmGrain::slotSensibilityChanged(int v)
{
    m_sensibilityLCDValue->display( QString::number(400+200*v) );
    slotEffect();
}

void ImageEffect_FilmGrain::prepareEffect()
{
    m_sensibilitySlider->setEnabled(false);
    
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    int s = 400 + 200 * m_sensibilitySlider->value();
        
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new FilmGrain(&image, this, s));
}

void ImageEffect_FilmGrain::prepareFinal()
{
    m_sensibilitySlider->setEnabled(false);
        
    int s = 400 + 200 * m_sensibilitySlider->value();
            
    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    m_threadedFilter = dynamic_cast<Digikam::ThreadedFilter *>(new FilmGrain(&orgImage, this, s));
    delete [] data;
}

void ImageEffect_FilmGrain::putPreviewData(void)
{
    QImage imDest = m_threadedFilter->getTargetImage();
    m_imagePreviewWidget->setPreviewImageData(imDest);
}

void ImageEffect_FilmGrain::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalData(i18n("Film Grain"), 
                        (uint*)m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamFilmGrainImagesPlugin

#include "imageeffect_filmgrain.moc"
