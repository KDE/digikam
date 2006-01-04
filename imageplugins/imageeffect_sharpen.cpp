/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-09
 * Description : Sharpen image filter for ImageEditor
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

#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kaboutdata.h>
#include <knuminput.h>
#include <kcursor.h>
#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>

// Digikam includes.

#include "imageiface.h"
#include "dimgsharpen.h"

// Local includes.

#include "imageeffect_sharpen.h"

namespace DigikamImagesPluginCore
{

ImageEffect_Sharpen::ImageEffect_Sharpen(QWidget* parent)
                   : Digikam::CtrlPanelDlg(parent, i18n("Sharpening Photograph"),
                                           "sharpen")
 {
    setHelp("blursharpentool.anchor", KApplication::kApplication()->aboutData()->appName());
    
    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 1, 2, marginHint(), spacingHint());
    QLabel *label = new QLabel(i18n("Sharpness:"), gboxSettings);
    
    m_radiusInput = new KIntNumInput(gboxSettings);
    m_radiusInput->setRange(0, 20, 1, true);
    m_radiusInput->setValue(0);
    QWhatsThis::add( m_radiusInput, i18n("<p>A sharpness of 0 has no effect, "
                                         "1 and above determine the sharpen matrix radius "
                                         "that determines how much to sharpen the image."));

    gridSettings->addWidget(label, 0, 0);
    gridSettings->addWidget(m_radiusInput, 0, 1);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
        
    // -------------------------------------------------------------
    
    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));
}

ImageEffect_Sharpen::~ImageEffect_Sharpen()
{
}

void ImageEffect_Sharpen::renderingFinished(void)
{
    m_radiusInput->setEnabled(true);
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
        
    m_threadedFilter = new Digikam::DImgSharpen(&img, this, m_radiusInput->value()*4);
}

void ImageEffect_Sharpen::prepareFinal()
{
    m_radiusInput->setEnabled(false);

    Digikam::ImageIface iface(0, 0);
    uchar *data     = iface.getOriginalImage();
    int w           = iface.originalWidth();
    int h           = iface.originalHeight();
    bool sixteenBit = iface.originalSixteenBit();
    bool hasAlpha   = iface.originalHasAlpha();
    Digikam::DImg orgImage = Digikam::DImg(w, h, sixteenBit, hasAlpha ,data);
    delete [] data;
    m_threadedFilter = new Digikam::DImgSharpen(&orgImage, this, m_radiusInput->value()*4);
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

}  // NameSpace DigikamImagesPluginCore

#include "imageeffect_sharpen.moc"
