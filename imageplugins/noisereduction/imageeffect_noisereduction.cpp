/* ============================================================
 * File  : imageeffect_noisereduction.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-08-24
 * Description : noise reduction image filter for digiKam 
 *               image editor.
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

#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qstring.h>
#include <qimage.h>
#include <qlayout.h>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <knuminput.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "noisereduction.h"
#include "imageeffect_noisereduction.h"

namespace DigikamNoiseReductionImagesPlugin
{

ImageEffect_NoiseReduction::ImageEffect_NoiseReduction(QWidget* parent, QString title, QFrame* banner)
                          : Digikam::CtrlPanelDlg(parent, title, "noisereduction", false,
                                     false, true, Digikam::ImagePannelWidget::SeparateViewAll, banner)
{
    QString whatsThis;
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Noise Reduction"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A noise reduction image filter plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2006, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at kdemail dot net");

    about->addAuthor("Michael Sweet", I18N_NOOP("Original Despeckle algorithm author"),
                     "mike at easysw.com");
                     
    setAboutData(about);
    
    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 6, 1, marginHint(), spacingHint());    
    
    QLabel *label1 = new QLabel(i18n("Radius:"), gboxSettings);
    
    m_radiusInput = new KIntNumInput(gboxSettings, "m_radiusInput");
    m_radiusInput->setRange(1, 20, 1, true);
    
    QWhatsThis::add( m_radiusInput, i18n("<p>This slider sets the size of action window witch be moved over the image to remove artefacts. "
                                         "The color in it is smoothed, so imperfections are removed. This setting is only avaialble when "
                                         "<b>Adaptive</b> method is disabled.") );
    
    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 1);
    gridSettings->addMultiCellWidget(m_radiusInput, 1, 1, 0, 1);
    
    // -------------------------------------------------------------

    Digikam::ImageIface iface(0, 0);
    m_maxLevel = iface.originalSixteenBit() ? 65535 : 255;

    QLabel *label2 = new QLabel(i18n("Black level:"), gboxSettings);
    
    m_blackLevelInput = new KIntNumInput(gboxSettings, "m_blackLevelInput");
    m_blackLevelInput->setRange(0, m_maxLevel, 1, true);
    
    QWhatsThis::add( m_blackLevelInput, i18n("<p>This value controls the black luminosity "
                     "levels used by the adaptive filter to "
                     "adjust the filter radius.") );

    gridSettings->addMultiCellWidget(label2, 2, 2, 0, 1);
    gridSettings->addMultiCellWidget(m_blackLevelInput, 3, 3, 0, 1);                         
    
    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("White level:"), gboxSettings);
    
    m_whiteLevelInput = new KIntNumInput(gboxSettings, "m_blackLevelInput");
    m_whiteLevelInput->setRange(0, m_maxLevel, 1, true);
    
    QWhatsThis::add( m_whiteLevelInput, i18n("<p>This value controls the white luminosity "
                     "levels used by the adaptive filter to "
                     "adjust the filter radius.") );

    gridSettings->addMultiCellWidget(label3, 4, 4, 0, 1);
    gridSettings->addMultiCellWidget(m_whiteLevelInput, 5, 5, 0, 1);
                                              
    // -------------------------------------------------------------
    
    m_useAdaptativeMethod = new QCheckBox( i18n("Adaptive"), gboxSettings);
    QWhatsThis::add( m_useAdaptativeMethod, i18n("<p>This option use an adaptive median filter type to "
                                                 "adapts radius value to image content using Histogram. "
                                                 "If this option is checked, radius slider is not efficient. "
                                                 "It renders a result smoother than with radius alone."));
    
    m_useRecursiveMethod = new QCheckBox( i18n("Recursive"), gboxSettings);
    QWhatsThis::add( m_useRecursiveMethod, i18n("<p>This option use a recursive median filter type. "
                                                "Repeats filter action which gets stronger."));
    
    gridSettings->addMultiCellWidget(m_useAdaptativeMethod, 6, 6, 0, 0);
    gridSettings->addMultiCellWidget(m_useRecursiveMethod, 6, 6, 1, 1);    
        
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    
    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));            
            
    connect(m_blackLevelInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_whiteLevelInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));                                                
            
    connect(m_useAdaptativeMethod, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));
    
    connect(m_useRecursiveMethod, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));             
}

ImageEffect_NoiseReduction::~ImageEffect_NoiseReduction()
{
}

void ImageEffect_NoiseReduction::renderingFinished()
{
    m_radiusInput->setEnabled(!m_useAdaptativeMethod->isChecked());
    m_blackLevelInput->setEnabled(true);
    m_whiteLevelInput->setEnabled(true);
    m_useAdaptativeMethod->setEnabled(true);
    m_useRecursiveMethod->setEnabled(true);
}

void ImageEffect_NoiseReduction::resetValues()
{
    m_radiusInput->blockSignals(true);
    m_blackLevelInput->blockSignals(true);
    m_whiteLevelInput->blockSignals(true);
    m_useAdaptativeMethod->blockSignals(true);
    m_useRecursiveMethod->blockSignals(true);
                    
    m_radiusInput->setValue(3);
    m_blackLevelInput->setValue(0);
    m_whiteLevelInput->setValue(m_maxLevel);
    m_useAdaptativeMethod->setChecked(true);
    m_useRecursiveMethod->setChecked(false);
    m_radiusInput->setEnabled(!m_useAdaptativeMethod->isChecked());
    
    m_radiusInput->blockSignals(false);
    m_blackLevelInput->blockSignals(false);
    m_whiteLevelInput->blockSignals(false);
    m_useAdaptativeMethod->blockSignals(false);
    m_useRecursiveMethod->blockSignals(false);
}

void ImageEffect_NoiseReduction::prepareEffect()
{
    m_radiusInput->setEnabled(false);
    m_blackLevelInput->setEnabled(false);
    m_whiteLevelInput->setEnabled(false);
    m_useAdaptativeMethod->setEnabled(false);
    m_useRecursiveMethod->setEnabled(false);
    
    int  r  = m_radiusInput->value();
    int  bl = m_blackLevelInput->value();
    int  wl = m_whiteLevelInput->value();
    bool af = m_useAdaptativeMethod->isChecked();
    bool rf = m_useRecursiveMethod->isChecked();
    
    Digikam::DImg image = m_imagePreviewWidget->getOriginalRegionImage();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new NoiseReduction(&image,
                       this, r, bl, wl, af, rf));
}

void ImageEffect_NoiseReduction::prepareFinal()
{
    m_radiusInput->setEnabled(false);
    m_blackLevelInput->setEnabled(false);
    m_whiteLevelInput->setEnabled(false);
    m_useAdaptativeMethod->setEnabled(false);
    m_useRecursiveMethod->setEnabled(false);
    
    int  r  = m_radiusInput->value();
    int  bl = m_blackLevelInput->value();
    int  wl = m_whiteLevelInput->value();
    bool af = m_useAdaptativeMethod->isChecked();
    bool rf = m_useRecursiveMethod->isChecked();

    Digikam::ImageIface iface(0, 0);
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new NoiseReduction(iface.getOriginalImg(),
                       this, r, bl, wl, af, rf));
}

void ImageEffect_NoiseReduction::putPreviewData(void)
{
    m_imagePreviewWidget->setPreviewImage(m_threadedFilter->getTargetImage());
}

void ImageEffect_NoiseReduction::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Noise Reduction"), m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamNoiseReductionImagesPlugin

#include "imageeffect_noisereduction.moc"
