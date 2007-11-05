/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-25
 * Description : a digiKam image plugin to reduce 
 *               vignetting on an image.
 * 
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
 
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QPen>
#include <QTabWidget>
#include <QGridLayout>

// KDE includes.

#include <kconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kseparator.h>
#include <kglobal.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "bcgmodifier.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "dimgimagefilters.h"
#include "antivignetting.h"
#include "imageeffect_antivignetting.h"
#include "imageeffect_antivignetting.moc"

namespace DigikamAntiVignettingImagesPlugin
{

ImageEffect_AntiVignetting::ImageEffect_AntiVignetting(QWidget* parent)
                          : Digikam::ImageGuideDlg(parent, i18n("Vignetting Correction"),
                            "antivignettings", false, true, false,
                            Digikam::ImageGuideWidget::HVGuideMode, 0, true)
{
    QString whatsThis;
    
    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Vignetting Correction"), 
                                       digikam_version,
                                       ki18n("A digiKam image plugin to reduce image vignetting."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2004-2007, Gilles Caulier"),
                                       KLocalizedString(),
                                       "http://www.digikam.org");
                                       
    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("John Walker"), ki18n("Anti Vignetting algorithm"), 0,
                     "http://www.fourmilab.ch/netpbm/pnmctrfilt"); 
        
    setAboutData(about);
    
    // -------------------------------------------------------------
    
    QWidget *gboxSettings     = new QWidget(mainWidget());
    QGridLayout* gridSettings = new QGridLayout(gboxSettings);

    m_maskPreviewLabel = new QLabel( gboxSettings );
    m_maskPreviewLabel->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    m_maskPreviewLabel->setPixmap( QPixmap(120, 120) );
    m_maskPreviewLabel->setWhatsThis( i18n("<p>You can see here a thumbnail preview of the anti-vignetting "
                                           "mask applied to the image.") );
        
    // -------------------------------------------------------------

    QLabel *label1 = new QLabel(i18n("Density:"), gboxSettings);
    
    m_densityInput = new KDoubleNumInput(gboxSettings);
    m_densityInput->setDecimals(1);
    m_densityInput->setRange(1.0, 20.0, 0.1, true);
    m_densityInput->setWhatsThis( i18n("<p>This value controls the degree of intensity attenuation "
                                       "by the filter at its point of maximum density."));

    // -------------------------------------------------------------
    
    QLabel *label2 = new QLabel(i18n("Power:"), gboxSettings);
    
    m_powerInput = new KDoubleNumInput(gboxSettings);
    m_powerInput->setDecimals(1);
    m_powerInput->setRange(0.1, 2.0, 0.1, true);
    m_powerInput->setWhatsThis( i18n("<p>This value is used as the exponent controlling the "
                                     "fall-off in density from the center of the filter to the periphery."));

    // -------------------------------------------------------------
    
    QLabel *label3 = new QLabel(i18n("Radius:"), gboxSettings);
    
    m_radiusInput = new KDoubleNumInput(gboxSettings);
    m_radiusInput->setDecimals(1);
    m_radiusInput->setRange(-100.0, 100.0, 0.1, true);
    m_radiusInput->setWhatsThis( i18n("<p>This value is the radius of the center filter. It is a "
                                      "multiple of the half-diagonal measure of the image, at which "
                                      "the density of the filter falls to zero."));
    
    KSeparator *line = new KSeparator (Qt::Horizontal, gboxSettings);

    // -------------------------------------------------------------

    QLabel *label4 = new QLabel(i18n("Brightness:"), gboxSettings);
    
    m_brightnessInput = new KIntNumInput(gboxSettings);
    m_brightnessInput->setRange(0, 100, 1);  
    m_brightnessInput->setSliderEnabled(true);
    m_brightnessInput->setWhatsThis( i18n("<p>Set here the brightness re-adjustment of the target image."));

    // -------------------------------------------------------------
    
    QLabel *label5 = new QLabel(i18n("Contrast:"), gboxSettings);
    
    m_contrastInput = new KIntNumInput(gboxSettings);
    m_contrastInput->setRange(0, 100, 1);
    m_contrastInput->setSliderEnabled(true);  
    m_contrastInput->setWhatsThis( i18n("<p>Set here the contrast re-adjustment of the target image."));

    // -------------------------------------------------------------

    QLabel *label6 = new QLabel(i18n("Gamma:"), gboxSettings);
    
    m_gammaInput = new KDoubleNumInput(gboxSettings);
    m_gammaInput->setDecimals(2);
    m_gammaInput->setRange(0.1, 3.0, 0.01, true);
    m_gammaInput->setValue(1.0);
    m_gammaInput->setWhatsThis( i18n("<p>Set here the gamma re-adjustment of the target image."));

    // -------------------------------------------------------------

    gridSettings->addWidget(m_maskPreviewLabel, 0, 0, 1, 3 );
    gridSettings->addWidget(label1, 1, 0, 1, 3 );
    gridSettings->addWidget(m_densityInput, 2, 0, 1, 3 );
    gridSettings->addWidget(label2, 3, 0, 1, 3 );
    gridSettings->addWidget(m_powerInput, 4, 0, 1, 3 );
    gridSettings->addWidget(label3, 5, 0, 1, 3 );
    gridSettings->addWidget(m_radiusInput, 6, 0, 1, 3 );
    gridSettings->addWidget(line, 7, 0, 1, 3 );
    gridSettings->addWidget(label4, 8, 0, 1, 3 );
    gridSettings->addWidget(m_brightnessInput, 9, 0, 1, 3 );
    gridSettings->addWidget(label5, 10, 0, 1, 3 );
    gridSettings->addWidget(m_contrastInput, 11, 0, 1, 3 );
    gridSettings->addWidget(label6, 12, 0, 1, 3 );
    gridSettings->addWidget(m_gammaInput, 13, 0, 1, 3 );
    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(spacingHint());

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

    connect(m_gammaInput, SIGNAL(valueChanged (double)),
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

void ImageEffect_AntiVignetting::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("antivignettings Tool Dialog");

    m_densityInput->blockSignals(true);
    m_powerInput->blockSignals(true);
    m_radiusInput->blockSignals(true);
    m_brightnessInput->blockSignals(true);
    m_contrastInput->blockSignals(true);
    m_gammaInput->blockSignals(true);

    m_densityInput->setValue(group.readEntry("DensityAjustment", 2.0));
    m_powerInput->setValue(group.readEntry("PowerAjustment", 1.0));
    m_radiusInput->setValue(group.readEntry("RadiusAjustment", 1.0));
    m_brightnessInput->setValue(group.readEntry("BrightnessAjustment", 0));
    m_contrastInput->setValue(group.readEntry("ContrastAjustment", 0));
    m_gammaInput->setValue(group.readEntry("GammaAjustment", 1.0));

    m_densityInput->blockSignals(false);
    m_powerInput->blockSignals(false);
    m_radiusInput->blockSignals(false);
    m_brightnessInput->blockSignals(false);
    m_contrastInput->blockSignals(false);
    m_gammaInput->blockSignals(false);

    slotEffect();
}

void ImageEffect_AntiVignetting::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("antivignettings Tool Dialog");
    group.writeEntry("DensityAjustment", m_densityInput->value());
    group.writeEntry("PowerAjustment", m_powerInput->value());
    group.writeEntry("RadiusAjustment", m_radiusInput->value());
    group.writeEntry("BrightnessAjustment", m_brightnessInput->value());
    group.writeEntry("ContrastAjustment", m_contrastInput->value());
    group.writeEntry("GammaAjustment", m_gammaInput->value());
    group.sync();
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
    m_gammaInput->setValue(1.0);

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
    ps.scale( QSize(120, 120), Qt::ScaleMin );    

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
    double g = m_gammaInput->value();

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

    iface.putOriginalImage(i18n("Vignetting Correction"),
                           m_threadedFilter->getTargetImage().bits());
    
    double b = (double)(m_brightnessInput->value() / 100.0);
    double c = (double)(m_contrastInput->value()   / 100.0) + (double)(1.00);
    double g = m_gammaInput->value();

    // Adjust Image BCG.
    iface.setOriginalBCG(b, c, g);
}

}  // NameSpace DigikamAntiVignettingImagesPlugin
