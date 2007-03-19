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

#define MAX_MATRIX_SIZE 25

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
#include <kurl.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>

// Local includes.

#include "ddebug.h"
#include "imageiface.h"
#include "dimgsharpen.h"
#include "unsharp.h"
#include "refocus.h"
#include "imageeffect_sharpen.h"
#include "imageeffect_sharpen.moc"

namespace DigikamImagesPluginCore
{

ImageEffect_Sharpen::ImageEffect_Sharpen(QWidget* parent)
                   : Digikam::CtrlPanelDlg(parent, i18n("Sharpening Photograph"), "sharpen",
                                           true, false, true)
 {
    setHelp("blursharpentool.anchor", KApplication::kApplication()->aboutData()->appName());

    // -------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 2, 1, 0, spacingHint());

    QLabel *label1 = new QLabel(i18n("Method:"), gboxSettings);

    m_sharpMethod = new QComboBox( false, gboxSettings );
    m_sharpMethod->insertItem( i18n("Simple sharp") );
    m_sharpMethod->insertItem( i18n("Unsharp mask") );
    m_sharpMethod->insertItem( i18n("Refocus") );
    QWhatsThis::add( m_sharpMethod, i18n("<p>Select here the sharping method to apply on image."));
    
    m_stack = new QWidgetStack(gboxSettings);

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_sharpMethod, 0, 0, 1, 1);
    gridSettings->addMultiCellWidget(new KSeparator(gboxSettings), 1, 1, 0, 1);
    gridSettings->addMultiCellWidget(m_stack, 2, 2, 0, 1);

    // -------------------------------------------------------------
    
    QWidget *simpleSharpSettings = new QWidget(m_stack);
    QGridLayout* grid1           = new QGridLayout( simpleSharpSettings, 2, 1, 0, spacingHint());

    QLabel *label = new QLabel(i18n("Sharpness:"), simpleSharpSettings);
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
    m_radiusInput2 = new KIntNumInput(unsharpMaskSettings);
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

    QWidget *refocusSettings = new QWidget(m_stack);
    QGridLayout* grid3       = new QGridLayout(refocusSettings, 10, 1, 0, spacingHint());
    
    QLabel *label5 = new QLabel(i18n("Circular sharpness:"), refocusSettings);
    m_radius       = new KDoubleNumInput(refocusSettings);
    m_radius->setPrecision(2);
    m_radius->setRange(0.0, 5.0, 0.01, true);
    QWhatsThis::add( m_radius, i18n("<p>This is the radius of the circular convolution. It is the most important "
                                    "parameter for using the plugin. For most images the default value of 1.0 "
                                    "should give good results. Select a higher value when your image is very blurred."));
    
    QLabel *label6 = new QLabel(i18n("Correlation:"), refocusSettings);
    m_correlation  = new KDoubleNumInput(refocusSettings);
    m_correlation->setPrecision(2);
    m_correlation->setRange(0.0, 1.0, 0.01, true);
    QWhatsThis::add( m_correlation, i18n("<p>Increasing the correlation may help to reduce artifacts. The correlation can "
                                         "range from 0-1. Useful values are 0.5 and values close to 1, e.g. 0.95 and 0.99. "
                                         "Using a high value for the correlation will reduce the sharpening effect of the "
                                         "plugin."));

    QLabel *label7 = new QLabel(i18n("Noise filter:"), refocusSettings);
    m_noise        = new KDoubleNumInput(refocusSettings);
    m_noise->setPrecision(3);
    m_noise->setRange(0.0, 1.0, 0.001, true);
    QWhatsThis::add( m_noise, i18n("<p>Increasing the noise filter parameter may help to reduce artifacts. The noise filter "
                                   "can range from 0-1 but values higher than 0.1 are rarely helpful. When the noise filter "
                                   "value is too low, e.g. 0.0 the image quality will be very poor. A useful value is 0.01. "
                                   "Using a high value for the noise filter will reduce the sharpening "
                                   "effect of the plugin."));

    QLabel *label8 = new QLabel(i18n("Gaussian sharpness:"), refocusSettings);
    m_gauss        = new KDoubleNumInput(refocusSettings);
    m_gauss->setPrecision(2);
    m_gauss->setRange(0.0, 1.0, 0.01, true);
    QWhatsThis::add( m_gauss, i18n("<p>This is the sharpness for the gaussian convolution. Use this parameter when your "
                                   "blurring is of gaussian type. In most cases you should set this parameter to 0, because "
                                   "it causes nasty artifacts. When you use non-zero values you will probably have to "
                                   "increase the correlation and/or noise filter parameters too."));

    QLabel *label9 = new QLabel(i18n("Matrix size:"), refocusSettings);
    m_matrixSize   = new KIntNumInput(refocusSettings);
    m_matrixSize->setRange(0, MAX_MATRIX_SIZE, 1, true);  
    QWhatsThis::add( m_matrixSize, i18n("<p>This parameter determines the size of the transformation matrix. "
                                        "Increasing the matrix width may give better results, especially when you have "
                                        "chosen large values for circular or gaussian sharpness."));

    grid3->addMultiCellWidget(label5, 0, 0, 0, 1);
    grid3->addMultiCellWidget(m_radius, 1, 1, 0, 1);
    grid3->addMultiCellWidget(label6, 2, 2, 0, 1);
    grid3->addMultiCellWidget(m_correlation, 3, 3, 0, 1);
    grid3->addMultiCellWidget(label7, 4, 4, 0, 1);
    grid3->addMultiCellWidget(m_noise, 5, 5, 0, 1);
    grid3->addMultiCellWidget(label8, 6, 6, 0, 1);
    grid3->addMultiCellWidget(m_gauss, 7, 7, 0, 1);
    grid3->addMultiCellWidget(label9, 8, 8, 0, 1);
    grid3->addMultiCellWidget(m_matrixSize, 9, 9, 0, 1);
    grid3->setRowStretch(10, 10);
    m_stack->addWidget(refocusSettings, Refocus);
    
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

    connect(m_matrixSize, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));                        
    
    connect(m_radius, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));                        

    connect(m_gauss, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));                        

    connect(m_correlation, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));                        

    connect(m_noise, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));                        

    connect(m_sharpMethod, SIGNAL(activated(int)),
            this, SLOT(slotSharpMethodActived(int)));

    // -------------------------------------------------------------
    
    // Image creation with dummy borders (mosaic mode) used by Refocus method. It needs to do 
    // it before to apply deconvolution filter on original image border pixels including 
    // on matrix size area. This way limit artifacts on image border.
    
    Digikam::ImageIface iface(0, 0);
        
    uchar* data = iface.getOriginalImage();
    int    w    = iface.originalWidth();
    int    h    = iface.originalHeight();
    bool   sb   = iface.originalSixteenBit();
    bool   a    = iface.originalHasAlpha();
    
    m_img = Digikam::DImg( w + 4*MAX_MATRIX_SIZE, h + 4*MAX_MATRIX_SIZE, sb, a);
    
    Digikam::DImg tmp;
    Digikam::DImg org(w, h, sb, a, data);

    // Copy original.
    m_img.bitBltImage(&org, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
   
    // Create dummy top border
    tmp = org.copy(0, 0, w, 2*MAX_MATRIX_SIZE);
    tmp.flip(Digikam::DImg::VERTICAL);
    m_img.bitBltImage(&tmp, 2*MAX_MATRIX_SIZE, 0);

    // Create dummy bottom border
    tmp = org.copy(0, h-2*MAX_MATRIX_SIZE, w, 2*MAX_MATRIX_SIZE);
    tmp.flip(Digikam::DImg::VERTICAL);
    m_img.bitBltImage(&tmp, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE+h);

    // Create dummy left border
    tmp = org.copy(0, 0, 2*MAX_MATRIX_SIZE, h);
    tmp.flip(Digikam::DImg::HORIZONTAL);
    m_img.bitBltImage(&tmp, 0, 2*MAX_MATRIX_SIZE);
    
    // Create dummy right border
    tmp = org.copy(w-2*MAX_MATRIX_SIZE, 0, 2*MAX_MATRIX_SIZE, h);
    tmp.flip(Digikam::DImg::HORIZONTAL);
    m_img.bitBltImage(&tmp, w+2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    
    // Create dummy top/left corner
    tmp = org.copy(0, 0, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(Digikam::DImg::HORIZONTAL);
    tmp.flip(Digikam::DImg::VERTICAL);
    m_img.bitBltImage(&tmp, 0, 0);
    
    // Create dummy top/right corner
    tmp = org.copy(w-2*MAX_MATRIX_SIZE, 0, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(Digikam::DImg::HORIZONTAL);
    tmp.flip(Digikam::DImg::VERTICAL);
    m_img.bitBltImage(&tmp, w+2*MAX_MATRIX_SIZE, 0);

    // Create dummy bottom/left corner
    tmp = org.copy(0, h-2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(Digikam::DImg::HORIZONTAL);
    tmp.flip(Digikam::DImg::VERTICAL);
    m_img.bitBltImage(&tmp, 0, h+2*MAX_MATRIX_SIZE);
    
    // Create dummy bottom/right corner
    tmp = org.copy(w-2*MAX_MATRIX_SIZE, h-2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(Digikam::DImg::HORIZONTAL);
    tmp.flip(Digikam::DImg::VERTICAL);
    m_img.bitBltImage(&tmp, w+2*MAX_MATRIX_SIZE, h+2*MAX_MATRIX_SIZE);
 
    delete [] data;
}

ImageEffect_Sharpen::~ImageEffect_Sharpen()
{
}

void ImageEffect_Sharpen::renderingFinished(void)
{
    switch (m_stack->id(m_stack->visibleWidget()))
    {
        case SimpleSharp:
        {
            m_radiusInput->setEnabled(true);
            enableButton(User2, false);
            enableButton(User3, false);
            break;
        }

        case UnsharpMask:
        {
            m_radiusInput2->setEnabled(true);
            m_amountInput->setEnabled(true);
            m_thresholdInput->setEnabled(true);
            enableButton(User2, false);
            enableButton(User3, false);
            break;
        }

        case Refocus:
        {
            m_matrixSize->setEnabled(true);
            m_radius->setEnabled(true);
            m_gauss->setEnabled(true);
            m_correlation->setEnabled(true);
            m_noise->setEnabled(true);
            break;
        }
    }
}

void ImageEffect_Sharpen::slotSharpMethodActived(int w)
{
    m_stack->raiseWidget(w);
    if (w == Refocus)
    {
        enableButton(User2, true);
        enableButton(User3, true);
    }
    else
    {
        enableButton(User2, false);
        enableButton(User3, false);
    }

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
    m_matrixSize->blockSignals(true);
    m_radius->blockSignals(true);
    m_gauss->blockSignals(true);
    m_correlation->blockSignals(true);
    m_noise->blockSignals(true);
    m_sharpMethod->blockSignals(true);
    m_radiusInput->setValue(config->readNumEntry("SimpleSharpRadiusAjustment", 0));
    m_radiusInput2->setValue(config->readNumEntry("UnsharpMaskRadiusAjustment", 1));
    m_amountInput->setValue(config->readDoubleNumEntry("UnsharpMaskAmountAjustment", 1.0));
    m_thresholdInput->setValue(config->readDoubleNumEntry("UnsharpMaskThresholdAjustment", 0.05));
    m_matrixSize->setValue(config->readNumEntry("RefocusMatrixSize", 5));
    m_radius->setValue(config->readDoubleNumEntry("RefocusRadiusAjustment", 1.0));
    m_gauss->setValue(config->readDoubleNumEntry("RefocusGaussAjustment", 0.0));
    m_correlation->setValue(config->readDoubleNumEntry("RefocusCorrelationAjustment", 0.5));
    m_noise->setValue(config->readDoubleNumEntry("RefocusNoiseAjustment", 0.03));
    m_sharpMethod->setCurrentItem(config->readNumEntry("SharpenMethod", SimpleSharp));
    m_radiusInput->blockSignals(false);
    m_radiusInput2->blockSignals(false);
    m_amountInput->blockSignals(false);
    m_thresholdInput->blockSignals(false);
    m_matrixSize->blockSignals(false);
    m_radius->blockSignals(false);
    m_gauss->blockSignals(false);
    m_correlation->blockSignals(false);
    m_noise->blockSignals(false);
    m_sharpMethod->blockSignals(false);
    slotSharpMethodActived(m_sharpMethod->currentItem());
}

void ImageEffect_Sharpen::writeUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("sharpen Tool Dialog");
    config->writeEntry("SimpleSharpRadiusAjustment", m_radiusInput->value());
    config->writeEntry("UnsharpMaskRadiusAjustment", m_radiusInput2->value());
    config->writeEntry("UnsharpMaskAmountAjustment", m_amountInput->value());
    config->writeEntry("UnsharpMaskThresholdAjustment", m_thresholdInput->value());
    config->writeEntry("RefocusMatrixSize", m_matrixSize->value());
    config->writeEntry("RefocusRadiusAjustment", m_radius->value());
    config->writeEntry("RefocusGaussAjustment", m_gauss->value());
    config->writeEntry("RefocusCorrelationAjustment", m_correlation->value());
    config->writeEntry("RefocusNoiseAjustment", m_noise->value());
    config->writeEntry("SharpenMethod", m_sharpMethod->currentItem());
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
            break;
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
            break;
        }

        case Refocus:
        {
            m_matrixSize->blockSignals(true);
            m_radius->blockSignals(true);
            m_gauss->blockSignals(true);
            m_correlation->blockSignals(true);
            m_noise->blockSignals(true);
            m_matrixSize->setValue(5);
            m_radius->setValue(1.0);
            m_gauss->setValue(0.0);
            m_correlation->setValue(0.5);
            m_noise->setValue(0.03);
            m_matrixSize->blockSignals(false);
            m_radius->blockSignals(false);
            m_gauss->blockSignals(false);
            m_correlation->blockSignals(false);
            m_noise->blockSignals(false);
            break;
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

        case Refocus:
        {
            m_matrixSize->setEnabled(false);
            m_radius->setEnabled(false);
            m_gauss->setEnabled(false);
            m_correlation->setEnabled(false);
            m_noise->setEnabled(false);
            
            int    ms     = m_matrixSize->value();
            double r      = m_radius->value();
            double g      = m_gauss->value();
            double c      = m_correlation->value();
            double n      = m_noise->value();
        
            QRect area    = m_imagePreviewWidget->getOriginalImageRegionToRender();
            QRect tmpRect;
            tmpRect.setLeft(area.left()-2*ms);
            tmpRect.setRight(area.right()+2*ms);
            tmpRect.setTop(area.top()-2*ms);
            tmpRect.setBottom(area.bottom()+2*ms);
            tmpRect.moveBy(2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
            Digikam::DImg imTemp = m_img.copy(tmpRect);
                
            m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>
                               (new DigikamImagesPluginCore::Refocus(&imTemp, this, ms, r, g, c, n));
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
            break;
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

        case Refocus:
        {

            m_matrixSize->setEnabled(false);
            m_radius->setEnabled(false);
            m_gauss->setEnabled(false);
            m_correlation->setEnabled(false);
            m_noise->setEnabled(false);
            
            int    ms   = m_matrixSize->value();
            double r    = m_radius->value();
            double g    = m_gauss->value();
            double c    = m_correlation->value();
            double n    = m_noise->value();
            
            m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>
                               (new DigikamImagesPluginCore::Refocus(&m_img, this, ms, r, g, c, n));
            break;
        }
    }        
}

void ImageEffect_Sharpen::putPreviewData(void)
{
    switch (m_stack->id(m_stack->visibleWidget()))
    {
        case SimpleSharp:
        case UnsharpMask:
        {    
            Digikam::DImg imDest = m_threadedFilter->getTargetImage();
            m_imagePreviewWidget->setPreviewImage(imDest);
            break;
        }
        
        case Refocus:
        {
            int   ms   = m_matrixSize->value();
            QRect area = m_imagePreviewWidget->getOriginalImageRegionToRender();
        
            Digikam::DImg imDest = m_threadedFilter->getTargetImage()
                                    .copy(2*ms, 2*ms, area.width(), area.height());
            m_imagePreviewWidget->setPreviewImage(imDest);
            break;
        }
    }
}

void ImageEffect_Sharpen::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
    Digikam::DImg imDest = m_threadedFilter->getTargetImage();

    switch (m_stack->id(m_stack->visibleWidget()))
    {
        case SimpleSharp:
        {
            iface.putOriginalImage(i18n("Sharpen"), imDest.bits());
            break;
        }

        case UnsharpMask:
        {
            iface.putOriginalImage(i18n("Unsharp Mask"), imDest.bits());
            break;
        }
        
        case Refocus:
        {
            QRect area = m_imagePreviewWidget->getOriginalImageRegionToRender();
            Digikam::ImageIface iface(0, 0);
        
            iface.putOriginalImage(i18n("Refocus"), m_threadedFilter->getTargetImage()
                                                        .copy(2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE,
                                                                iface.originalWidth(),
                                                                iface.originalHeight())
                                                        .bits());
            break;
        }
    }
}

void ImageEffect_Sharpen::slotUser3()
{
    KURL loadRestorationFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Refocus Settings File to Load")) );
    if( loadRestorationFile.isEmpty() )
       return;

    QFile file(loadRestorationFile.path());
    
    if ( file.open(IO_ReadOnly) )   
    {
        QTextStream stream( &file );
        if ( stream.readLine() != "# Photograph Refocus Configuration File" )
        {
           KMessageBox::error(this, 
                        i18n("\"%1\" is not a Photograph Refocus settings text file.")
                        .arg(loadRestorationFile.fileName()));
           file.close();            
           return;
        }
        
        blockSignals(true);
        m_matrixSize->setValue( stream.readLine().toInt() );
        m_radius->setValue( stream.readLine().toDouble() );
        m_gauss->setValue( stream.readLine().toDouble() );
        m_correlation->setValue( stream.readLine().toDouble() );
        m_noise->setValue( stream.readLine().toDouble() );
        blockSignals(false);
        slotEffect();  
    }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Refocus text file."));

    file.close();             
}

void ImageEffect_Sharpen::slotUser2()
{
    KURL saveRestorationFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Refocus Settings File to Save")) );
    if( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.path());
    
    if ( file.open(IO_WriteOnly) )   
    {
        QTextStream stream( &file );        
        stream << "# Photograph Refocus Configuration File\n";    
        stream << m_matrixSize->value() << "\n";    
        stream << m_radius->value() << "\n";    
        stream << m_gauss->value() << "\n";    
        stream << m_correlation->value() << "\n";    
        stream << m_noise->value() << "\n";    
    }
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Refocus text file."));
    
    file.close();        
}

}  // NameSpace DigikamImagesPluginCore

