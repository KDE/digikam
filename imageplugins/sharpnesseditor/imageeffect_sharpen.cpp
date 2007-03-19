/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <qcombobox.h>
#include <qwidgetstack.h>

// KDE includes.

#include <kaboutdata.h>
#include <knuminput.h>
#include <kcursor.h>
#include <klocale.h>
#include <kapplication.h>
#include <kseparator.h>
#include <kconfig.h>

// Local includes.

#include "ddebug.h"
#include "imageiface.h"
#include "dimgsharpen.h"
#include "unsharp.h"
#include "imageeffect_sharpen.h"
#include "imageeffect_sharpen.moc"

namespace DigikamImagesPluginCore
{

ImageEffect_Sharpen::ImageEffect_Sharpen(QWidget* parent)
                   : Digikam::CtrlPanelDlg(parent, i18n("Sharpening Photograph"), "sharpen")
 {
    setHelp("blursharpentool.anchor", KApplication::kApplication()->aboutData()->appName());

    // -------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 2, 1, 0, spacingHint());

    QLabel *label1 = new QLabel(i18n("Method:"), gboxSettings);

    m_sharpMethod = new QComboBox( false, gboxSettings );
    m_sharpMethod->insertItem( i18n("Simple sharp") );
    m_sharpMethod->insertItem( i18n("Unsharp mask") );
    QWhatsThis::add( m_sharpMethod, i18n("<p>Select here the sharping method to apply on image."));
    
    m_stack = new QWidgetStack(gboxSettings);

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_sharpMethod, 0, 0, 1, 1);
    gridSettings->addMultiCellWidget(new KSeparator(gboxSettings), 1, 1, 0, 1);
    gridSettings->addMultiCellWidget(m_stack, 2, 2, 0, 1);

    // -------------------------------------------------------------
    
    QWidget *simpleSharpSettings = new QWidget(m_stack);
    QGridLayout* grid1           = new QGridLayout( simpleSharpSettings, 2, 1, 0, spacingHint());
    QLabel *label                = new QLabel(i18n("Sharpness:"), simpleSharpSettings);

    m_radiusInput = new KIntNumInput(simpleSharpSettings);
    m_radiusInput->setRange(0, 100, 1, true);
    m_radiusInput->setValue(0);
    QWhatsThis::add( m_radiusInput, i18n("<p>A sharpness of 0 has no effect, "
                                         "1 and above determine the sharpen matrix radius "
                                         "that determines how much to sharpen the image."));

    grid1->addMultiCellWidget(label, 0, 0, 0, 1);
    grid1->addMultiCellWidget(m_radiusInput, 1, 1, 0, 1);
    grid1->setRowStretch(2, 10);
    m_stack->addWidget(simpleSharpSettings, SimpleSharp);

    // -------------------------------------------------------------

    QWidget *unsharpMaskSettings = new QWidget(m_stack);
    QGridLayout* grid2           = new QGridLayout( unsharpMaskSettings, 6, 1, 0, spacingHint());

    QLabel *label2 = new QLabel(i18n("Radius:"), unsharpMaskSettings);
    m_radiusInput2  = new KIntNumInput(unsharpMaskSettings);
    m_radiusInput2->setRange(1, 120, 1, true);
    QWhatsThis::add( m_radiusInput2, i18n("<p>Radius value is the gaussian blur matrix radius value "
                                         "used to determines how much to blur the image.") );
    
    QLabel *label3 = new QLabel(i18n("Amount:"), unsharpMaskSettings);
    m_amountInput  = new KDoubleNumInput(unsharpMaskSettings);
    m_amountInput->setPrecision(1);
    m_amountInput->setRange(0.0, 5.0, 0.1, true);
    QWhatsThis::add( m_amountInput, i18n("<p>The value of the difference between the "
                     "original and the blur image that is added back into the original.") );
    
    QLabel *label4   = new QLabel(i18n("Threshold:"), unsharpMaskSettings);
    m_thresholdInput = new KDoubleNumInput(unsharpMaskSettings);
    m_thresholdInput->setPrecision(2);
    m_thresholdInput->setRange(0.0, 1.0, 0.01, true);
    QWhatsThis::add( m_thresholdInput, i18n("<p>The threshold, as a fraction of the maximum "
                     "luminosity value, needed to apply the difference amount.") );


    grid2->addMultiCellWidget(label2, 0, 0, 0, 1);
    grid2->addMultiCellWidget(m_radiusInput2, 1, 1, 0, 1);
    grid2->addMultiCellWidget(label3, 2, 2, 0, 1);
    grid2->addMultiCellWidget(m_amountInput, 3, 3, 0, 1);
    grid2->addMultiCellWidget(label4, 4, 4, 0, 1);
    grid2->addMultiCellWidget(m_thresholdInput, 5, 5, 0, 1);
    grid2->setRowStretch(6, 10);
    m_stack->addWidget(unsharpMaskSettings, UnsharpMask);

    // -------------------------------------------------------------
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
        
    // -------------------------------------------------------------
    
    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_radiusInput2, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));                                                

    connect(m_amountInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                                                            
            
    connect(m_thresholdInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(m_sharpMethod, SIGNAL(activated(int)),
            this, SLOT(slotSharpMethodActived(int)));
}

ImageEffect_Sharpen::~ImageEffect_Sharpen()
{
}

void ImageEffect_Sharpen::slotSharpMethodActived(int w)
{
    m_stack->raiseWidget(w);
    slotEffect();
}

void ImageEffect_Sharpen::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("sharpen Tool Dialog");
    m_radiusInput->blockSignals(true);
    m_radiusInput2->blockSignals(true);
    m_amountInput->blockSignals(true);
    m_thresholdInput->blockSignals(true);
    m_radiusInput->setValue(config->readNumEntry("RadiusAjustment", 0));
    m_radiusInput2->setValue(config->readNumEntry("RadiusAjustment2", 1));
    m_amountInput->setValue(config->readDoubleNumEntry("AmountAjustment", 1.0));
    m_thresholdInput->setValue(config->readDoubleNumEntry("ThresholdAjustment", 0.05));
    m_radiusInput->blockSignals(false);
    m_radiusInput2->blockSignals(false);
    m_amountInput->blockSignals(false);
    m_thresholdInput->blockSignals(false);
}

void ImageEffect_Sharpen::writeUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("sharpen Tool Dialog");
    config->writeEntry("RadiusAjustment", m_radiusInput->value());
    config->writeEntry("RadiusAjustment2", m_radiusInput2->value());
    config->writeEntry("AmountAjustment", m_amountInput->value());
    config->writeEntry("ThresholdAjustment", m_thresholdInput->value());
    config->sync();
}

void ImageEffect_Sharpen::resetValues(void)
{
    switch (m_stack->id(m_stack->visibleWidget()))
    {
        case SimpleSharp:
        {    
            m_radiusInput->blockSignals(true);
            m_radiusInput->setValue(0);
            m_radiusInput->blockSignals(false);
        }

        case UnsharpMask:
        {
            m_radiusInput2->blockSignals(true);
            m_amountInput->blockSignals(true);
            m_thresholdInput->blockSignals(true);
            m_radiusInput2->setValue(1);
            m_amountInput->setValue(1.0);
            m_thresholdInput->setValue(0.05);
            m_radiusInput2->blockSignals(false);
            m_amountInput->blockSignals(false);
            m_thresholdInput->blockSignals(false);
        }
    }
} 

void ImageEffect_Sharpen::prepareEffect()
{
    switch (m_stack->id(m_stack->visibleWidget()))
    {
        case SimpleSharp:
        {    
            m_radiusInput->setEnabled(false);
            
            Digikam::DImg img = m_imagePreviewWidget->getOriginalRegionImage();
                
            double radius = m_radiusInput->value()/10.0;
            double sigma;
        
            if (radius < 1.0) sigma = radius;
            else sigma = sqrt(radius);
            
            m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>
                               (new Digikam::DImgSharpen(&img, this, radius, sigma ));
            break;
        }
        case UnsharpMask:
        {
            m_radiusInput2->setEnabled(false);
            m_amountInput->setEnabled(false);
            m_thresholdInput->setEnabled(false);
            
            Digikam::DImg img = m_imagePreviewWidget->getOriginalRegionImage();
        
            int    r  = m_radiusInput2->value();
            double a  = m_amountInput->value();
            double th = m_thresholdInput->value();
            
            m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>
                               (new DigikamImagesPluginCore::UnsharpMask(&img, this, r, a, th));
            break;
        }
    }        
}

void ImageEffect_Sharpen::prepareFinal()
{
    switch (m_stack->id(m_stack->visibleWidget()))
    {
        case SimpleSharp:
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
        case UnsharpMask:
        {
            m_radiusInput2->setEnabled(false);
            m_amountInput->setEnabled(false);
            m_thresholdInput->setEnabled(false);
                
            int    r  = m_radiusInput2->value();
            double a  = m_amountInput->value();
            double th = m_thresholdInput->value();
            
            Digikam::ImageIface iface(0, 0);
            uchar *data     = iface.getOriginalImage();
            int w           = iface.originalWidth();
            int h           = iface.originalHeight();
            bool sixteenBit = iface.originalSixteenBit();
            bool hasAlpha   = iface.originalHasAlpha();
            Digikam::DImg orgImage = Digikam::DImg(w, h, sixteenBit, hasAlpha ,data);
            delete [] data;
            m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>
                               (new DigikamImagesPluginCore::UnsharpMask(&orgImage, this, r, a, th));
            break;
        }
    }        
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

    switch (m_stack->id(m_stack->visibleWidget()))
    {
        case SimpleSharp:
            iface.putOriginalImage(i18n("Sharpen"), imDest.bits());
            break;

        case UnsharpMask:
            iface.putOriginalImage(i18n("Unsharp Mask"), imDest.bits());
            break;
    }
}

void ImageEffect_Sharpen::renderingFinished(void)
{
    switch (m_stack->id(m_stack->visibleWidget()))
    {
        case SimpleSharp:
            m_radiusInput->setEnabled(true);
            break;

        case UnsharpMask:
            m_radiusInput2->setEnabled(true);
            m_amountInput->setEnabled(true);
            m_thresholdInput->setEnabled(true);
            break;
    }
}

}  // NameSpace DigikamImagesPluginCore

