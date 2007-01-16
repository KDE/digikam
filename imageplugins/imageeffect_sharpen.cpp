/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2004-07-09
 * Description : a tool to sharp an image
 * 
 * Copyright 2004-2007 by Gilles Caulier
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

// C++ includes. 
 
#include <cmath>

// Qt includes.

#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kaboutdata.h>
#include <knuminput.h>
#include <kcursor.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>


// Digikam includes.

#include "ddebug.h"
#include "imageiface.h"
#include "dimgsharpen.h"

// Local includes.

#include "imageeffect_sharpen.h"
#include "imageeffect_sharpen.moc"

namespace DigikamImagesPluginCore
{

ImageEffect_Sharpen::ImageEffect_Sharpen(QWidget* parent)
                   : Digikam::CtrlPanelDlg(parent, i18n("Sharpening Photograph"),
                                           "sharpen")
 {
    setHelp("blursharpentool.anchor", KApplication::kApplication()->aboutData()->appName());
    
    QWidget *gboxSettings     = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 1, 1, 0, spacingHint());
    QLabel *label             = new QLabel(i18n("Sharpness:"), gboxSettings);
    
    m_radiusInput = new KIntNumInput(gboxSettings);
    m_radiusInput->setRange(0, 100, 1, true);
    m_radiusInput->setValue(0);
    QWhatsThis::add( m_radiusInput, i18n("<p>A sharpness of 0 has no effect, "
                                         "1 and above determine the sharpen matrix radius "
                                         "that determines how much to sharpen the image."));

    gridSettings->addMultiCellWidget(label, 0, 0, 0, 1);
    gridSettings->addMultiCellWidget(m_radiusInput, 1, 1, 0, 1);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
        
    // -------------------------------------------------------------
    
    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));
}

ImageEffect_Sharpen::~ImageEffect_Sharpen()
{
}

void ImageEffect_Sharpen::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("sharpen Tool Dialog");
    m_radiusInput->setValue(config->readNumEntry("RadiusAjustment", 0));
}

void ImageEffect_Sharpen::writeUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("sharpen Tool Dialog");
    config->writeEntry("RadiusAjustment", m_radiusInput->value());
    config->sync();
}

void ImageEffect_Sharpen::resetValues(void)
{
    m_radiusInput->blockSignals(true);
    m_radiusInput->setValue(0);
    m_radiusInput->blockSignals(false);
} 

void ImageEffect_Sharpen::prepareEffect()
{
    m_radiusInput->setEnabled(false);
    
    Digikam::DImg img = m_imagePreviewWidget->getOriginalRegionImage();
        
    double radius = m_radiusInput->value()/10.0;
    double sigma;

    if (radius < 1.0) sigma = radius;
    else sigma = sqrt(radius);
    
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>
                       (new Digikam::DImgSharpen(&img, this, radius, sigma ));
}

void ImageEffect_Sharpen::prepareFinal()
{
    m_radiusInput->setEnabled(false);

    double radius = m_radiusInput->value()/10.0;
    double sigma;

    if (radius < 1.0) sigma = radius;
    else sigma = sqrt(radius);
    
    Digikam::ImageIface iface(0, 0);
    uchar *data     = iface.getOriginalImage();
    int w           = iface.originalWidth();
    int h           = iface.originalHeight();
    bool sixteenBit = iface.originalSixteenBit();
    bool hasAlpha   = iface.originalHasAlpha();
    Digikam::DImg orgImage = Digikam::DImg(w, h, sixteenBit, hasAlpha ,data);
    delete [] data;
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>
                       (new Digikam::DImgSharpen(&orgImage, this, radius, sigma ));
}

void ImageEffect_Sharpen::putPreviewData(void)
{
    Digikam::DImg imDest = m_threadedFilter->getTargetImage();
    m_imagePreviewWidget->setPreviewImage(imDest);
}

void ImageEffect_Sharpen::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
    Digikam::DImg imDest = m_threadedFilter->getTargetImage();
    iface.putOriginalImage(i18n("Sharpen"), imDest.bits());
}

void ImageEffect_Sharpen::renderingFinished(void)
{
    m_radiusInput->setEnabled(true);
}

}  // NameSpace DigikamImagesPluginCore

