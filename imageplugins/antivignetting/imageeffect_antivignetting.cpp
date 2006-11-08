/* ============================================================
 * File  : imageeffect_antivignetting.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-12-25
 * Description : a digiKam image plugin to reduce 
 *               vignetting on an image.
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
#include <qlayout.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qpen.h>
#include <qtabwidget.h>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <knuminput.h>

// Local includes.

#include "version.h"
#include "antivignetting.h"
#include "imageeffect_antivignetting.h"

namespace DigikamAntiVignettingImagesPlugin
{

ImageEffect_AntiVignetting::ImageEffect_AntiVignetting(QWidget* parent, QString title, QFrame* banner)
                          : Digikam::ImageGuideDlg(parent, title, "antivignettings",
                            false, true, false, Digikam::ImageGuideWidget::HVGuideMode, banner, true)
{
    QString whatsThis;
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Anti-Vignetting"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to reduce image vignetting."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2006, Gilles Caulier",
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at kdemail dot net");

    about->addAuthor("John Walker", I18N_NOOP("Anti Vignetting algorithm"), 0,
                     "http://www.fourmilab.ch/netpbm/pnmctrfilt"); 
        
    setAboutData(about);
    
    // -------------------------------------------------------------
    
    QWidget *gboxSettings = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 12, 2, marginHint(), spacingHint());

    m_maskPreviewLabel = new QLabel( gboxSettings );
    m_maskPreviewLabel->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    QWhatsThis::add( m_maskPreviewLabel, i18n("<p>You can see here a thumbnail preview of the anti-vignetting "
                                              "mask applied to the image.") );
    gridSettings->addMultiCellWidget(m_maskPreviewLabel, 0, 0, 0, 2);
        
    // -------------------------------------------------------------

    QLabel *label1 = new QLabel(i18n("Density:"), gboxSettings);
    
    m_densityInput = new KDoubleNumInput(gboxSettings);
    m_densityInput->setPrecision(1);
    m_densityInput->setRange(1.0, 20.0, 0.1, true);
    QWhatsThis::add( m_densityInput, i18n("<p>This value controls the degree of intensity attenuation by the filter "
                                          "at its point of maximum density."));

    gridSettings->addMultiCellWidget(label1, 1, 1, 0, 2);
    gridSettings->addMultiCellWidget(m_densityInput, 2, 2, 0, 2);
                          
    // -------------------------------------------------------------
    
    QLabel *label2 = new QLabel(i18n("Power:"), gboxSettings);
    
    m_powerInput = new KDoubleNumInput(gboxSettings);
    m_powerInput->setPrecision(1);
    m_powerInput->setRange(0.1, 2.0, 0.1, true);
    QWhatsThis::add( m_powerInput, i18n("<p>This value is used as the exponent controlling the fall-off in density "
                                        "from the center of the filter to the periphery."));

    gridSettings->addMultiCellWidget(label2, 3, 3, 0, 2);
    gridSettings->addMultiCellWidget(m_powerInput, 4, 4, 0, 2);
    
    // -------------------------------------------------------------
    
    QLabel *label3 = new QLabel(i18n("Radius:"), gboxSettings);
    
    m_radiusInput = new KDoubleNumInput(gboxSettings);
    m_radiusInput->setPrecision(1);
    m_radiusInput->setRange(-100.0, 100.0, 0.1, true);
    QWhatsThis::add( m_radiusInput, i18n("<p>This value is the radius of the center filter. It is a multiple of the "
                                          "half-diagonal measure of the image, at which the density of the filter falls "
                                          "to zero."));
    
    gridSettings->addMultiCellWidget(label3, 5, 5, 0, 2);
    gridSettings->addMultiCellWidget(m_radiusInput, 6, 6, 0, 2);
    
    // -------------------------------------------------------------

    QLabel *label4 = new QLabel(i18n("Brightness:"), gboxSettings);
    
    m_brightnessInput = new KIntNumInput(gboxSettings);
    m_brightnessInput->setRange(0, 100, 1, true);  
    QWhatsThis::add( m_brightnessInput, i18n("<p>Set here the brightness re-adjustment of the target image."));

    gridSettings->addMultiCellWidget(label4, 7, 7, 0, 2);
    gridSettings->addMultiCellWidget(m_brightnessInput, 8, 8, 0, 2);
        
    // -------------------------------------------------------------
    
    QLabel *label5 = new QLabel(i18n("Contrast:"), gboxSettings);
    
    m_contrastInput = new KIntNumInput(gboxSettings);
    m_contrastInput->setRange(0, 100, 1, true);  
    QWhatsThis::add( m_contrastInput, i18n("<p>Set here the contrast re-adjustment of the target image."));

    gridSettings->addMultiCellWidget(label5, 9, 9, 0, 2);
    gridSettings->addMultiCellWidget(m_contrastInput, 10, 10, 0, 2);
    
    // -------------------------------------------------------------

    QLabel *label6 = new QLabel(i18n("Gamma:"), gboxSettings);
    
    m_gammaInput = new KIntNumInput(gboxSettings);
    m_gammaInput->setRange(0, 100, 1, true);  
    QWhatsThis::add( m_gammaInput, i18n("<p>Set here the gamma re-adjustment of the target image."));

    gridSettings->addMultiCellWidget(label6, 11, 11, 0, 2);
    gridSettings->addMultiCellWidget(m_gammaInput, 12, 12, 0, 2);

    setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    
    connect(m_densityInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));            

    connect(m_powerInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));            

    connect(m_radiusInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));            

    connect(m_brightnessInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));            

    connect(m_contrastInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));            

    connect(m_gammaInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));      
}

ImageEffect_AntiVignetting::~ImageEffect_AntiVignetting()
{
}

void ImageEffect_AntiVignetting::renderingFinished()
{
    m_densityInput->setEnabled(true);
    m_powerInput->setEnabled(true);
    m_radiusInput->setEnabled(true);
    m_brightnessInput->setEnabled(true);
    m_contrastInput->setEnabled(true);
    m_gammaInput->setEnabled(true);
}

void ImageEffect_AntiVignetting::resetValues()
{
    m_densityInput->blockSignals(true);
    m_powerInput->blockSignals(true);
    m_radiusInput->blockSignals(true);
    m_brightnessInput->blockSignals(true);
    m_contrastInput->blockSignals(true);
    m_gammaInput->blockSignals(true);
    
    m_densityInput->setValue(2.0);
    m_powerInput->setValue(1.0);
    m_radiusInput->setValue(1.0);
    m_brightnessInput->setValue(0);
    m_contrastInput->setValue(0);
    m_gammaInput->setValue(0);

    m_densityInput->blockSignals(false);
    m_powerInput->blockSignals(false);
    m_radiusInput->blockSignals(false);
    m_brightnessInput->blockSignals(false);
    m_contrastInput->blockSignals(false);
    m_gammaInput->blockSignals(false);
} 

void ImageEffect_AntiVignetting::prepareEffect()
{
    m_densityInput->setEnabled(false);
    m_powerInput->setEnabled(false);
    m_radiusInput->setEnabled(false);
    m_brightnessInput->setEnabled(false);
    m_contrastInput->setEnabled(false);
    m_gammaInput->setEnabled(false);
    
    double d = m_densityInput->value();
    double p = m_powerInput->value();
    double r = m_radiusInput->value();

    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    uchar *data   = iface->getOriginalImage();
    int orgWidth  = iface->originalWidth();
    int orgHeight = iface->originalHeight();
    QSize ps(orgWidth, orgHeight);
    ps.scale( QSize(120, 120), QSize::ScaleMin );    

    // Calc mask preview.    
    Digikam::DImg preview(ps.width(), ps.height(), false);
    memset(preview.bits(), 255, preview.numBytes());
    AntiVignetting maskPreview(&preview, 0L, d, p, r, 0, 0, false);
    QPixmap pix = maskPreview.getTargetImage().convertToPixmap();
    QPainter pt(&pix);
    pt.setPen( QPen(Qt::black, 1) ); 
    pt.drawRect( 0, 0, pix.width(), pix.height() );
    pt.end();
    m_maskPreviewLabel->setPixmap( pix );
    
    Digikam::DImg orgImage(orgWidth, orgHeight, iface->originalSixteenBit(),
                           iface->originalHasAlpha(), data);
    delete [] data;

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new AntiVignetting(&orgImage, this, d, p, r, 0, 0, true));
}

void ImageEffect_AntiVignetting::prepareFinal()
{
    m_densityInput->setEnabled(false);
    m_powerInput->setEnabled(false);
    m_radiusInput->setEnabled(false);
    m_brightnessInput->setEnabled(false);
    m_contrastInput->setEnabled(false);
    m_gammaInput->setEnabled(false);

    double d = m_densityInput->value();
    double p = m_powerInput->value();
    double r = m_radiusInput->value();

    Digikam::ImageIface iface(0, 0);

    uchar *data = iface.getOriginalImage();
    Digikam::DImg orgImage(iface.originalWidth(), iface.originalHeight(), iface.originalSixteenBit(),
                           iface.originalHasAlpha(), data);
    delete [] data;
    
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new AntiVignetting(&orgImage, this, d, p, r, 0, 0, true));
}

void ImageEffect_AntiVignetting::putPreviewData(void)
{
    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    
    Digikam::DImg imDest = m_threadedFilter->getTargetImage();
    
    // Adjust Image BCG.

    double b = (double)(m_brightnessInput->value() / 100.0);
    double c = (double)(m_contrastInput->value()   / 100.0) + (double)(1.00);
    double g = (double)(m_gammaInput->value()      / 100.0) + (double)(1.00);

    Digikam::BCGModifier cmod;
    cmod.setGamma(g);
    cmod.setBrightness(b);
    cmod.setContrast(c);
    cmod.applyBCG(imDest);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(),
                                               iface->previewHeight())).bits());
    m_imagePreviewWidget->updatePreview();
}

void ImageEffect_AntiVignetting::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);    

    iface.putOriginalImage(i18n("Anti-Vignetting"),
                           m_threadedFilter->getTargetImage().bits());
    
    double b = (double)(m_brightnessInput->value() / 100.0);
    double c = (double)(m_contrastInput->value()   / 100.0) + (double)(1.00);
    double g = (double)(m_gammaInput->value()      / 100.0) + (double)(1.00);

    // Adjust Image BCG.
    iface.setOriginalBCG(b, c, g);
}

}  // NameSpace DigikamAntiVignettingImagesPlugin

#include "imageeffect_antivignetting.moc"
