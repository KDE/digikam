/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-30
 * Description : a plugin to add rain drop over an image
 * 
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include <QFrame>
#include <QImage>
#include <QGridLayout>

// KDE includes.

#include <kconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kglobal.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "raindrop.h"
#include "imageeffect_raindrop.h"
#include "imageeffect_raindrop.moc"

namespace DigikamRainDropImagesPlugin
{

ImageEffect_RainDrop::ImageEffect_RainDrop(QWidget* parent)
                    : Digikam::ImageGuideDlg(parent, i18n("Add Raindrops to Photograph"), 
                                             "raindrops", false, true, false,
                                             Digikam::ImageGuideWidget::HVGuideMode)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Raindrops"), 
                                       digikam_version,
                                       ki18n("A digiKam image plugin to add raindrops to an image."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2004-2005, Gilles Caulier\n"
                                       "(c) 2006-2007, Gilles Caulier and Marcel Wiesweg"),
                                       KLocalizedString(),
                                       "http://www.digikam.org");

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("Pieter Z. Voloshyn"), ki18n("Raindrops algorithm"), 
                     "pieter dot voloshyn at gmail dot com"); 

    about->addAuthor(ki18n("Marcel Wiesweg"), ki18n("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);

    m_imagePreviewWidget->setWhatsThis( i18n("<p>This previews the Raindrop effect."
                                           "<p>Note: if you have previously selected an area in the editor, "
                                           "this will be unaffected by the filter. You can use this method to "
                                           "disable the Raindrops effect on a human face, for example.") );
    
    // -------------------------------------------------------------
   
    QWidget *gboxSettings     = new QWidget(mainWidget());
    QGridLayout* gridSettings = new QGridLayout(gboxSettings);
                                                  
    QLabel *label1 = new QLabel(i18n("Drop size:"), gboxSettings);
    
    m_dropInput = new KIntNumInput(gboxSettings);
    m_dropInput->setRange(0, 200, 1);
    m_dropInput->setSliderEnabled(true);
    m_dropInput->setValue(80);
    m_dropInput->setWhatsThis( i18n("<p>Set here the raindrops' size."));
   
    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Number:"), gboxSettings);
    
    m_amountInput = new KIntNumInput(gboxSettings);
    m_amountInput->setRange(1, 500, 1);
    m_amountInput->setSliderEnabled(true);
    m_amountInput->setValue(150);
    m_amountInput->setWhatsThis( i18n("<p>This value controls the maximum number of raindrops.")); 
    
    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Fish eyes:"), gboxSettings);
    
    m_coeffInput = new KIntNumInput(gboxSettings);
    m_coeffInput->setRange(1, 100, 1);
    m_coeffInput->setSliderEnabled(true);
    m_coeffInput->setValue(30);
    m_coeffInput->setWhatsThis( i18n("<p>This value is the fish-eye-effect optical "
                                     "distortion coefficient."));     
    
    gridSettings->addWidget(label1, 0, 0, 1, 3 );
    gridSettings->addWidget(m_dropInput, 1, 0, 1, 3 );
    gridSettings->addWidget(label2, 2, 0, 1, 3 );
    gridSettings->addWidget(m_amountInput, 3, 0, 1, 3 );
    gridSettings->addWidget(label3, 4, 0, 1, 3 );
    gridSettings->addWidget(m_coeffInput, 5, 0, 1, 3 );
    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(0);
    
    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------
    
    connect(m_dropInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));  
    
    connect(m_amountInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));  
    
    connect(m_coeffInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));  
}

ImageEffect_RainDrop::~ImageEffect_RainDrop()
{
}

void ImageEffect_RainDrop::renderingFinished()
{
    m_dropInput->setEnabled(true);
    m_amountInput->setEnabled(true);
    m_coeffInput->setEnabled(true);
}

void ImageEffect_RainDrop::readUserSettings(void)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("raindrops Tool Dialog");

    m_dropInput->blockSignals(true);
    m_amountInput->blockSignals(true);
    m_coeffInput->blockSignals(true);
    
    m_dropInput->setValue(group.readEntry("DropAdjustment", 80));
    m_amountInput->setValue(group.readEntry("AmountAdjustment", 150));
    m_coeffInput->setValue(group.readEntry("CoeffAdjustment", 30));
    
    m_dropInput->blockSignals(false);
    m_amountInput->blockSignals(false);
    m_coeffInput->blockSignals(false);
    
    slotEffect();
}

void ImageEffect_RainDrop::writeUserSettings(void)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("raindrops Tool Dialog");
    group.writeEntry("DropAdjustment", m_dropInput->value());
    group.writeEntry("AmountAdjustment", m_amountInput->value());
    group.writeEntry("CoeffAdjustment", m_coeffInput->value());
    group.sync();
}

void ImageEffect_RainDrop::resetValues()
{
    m_dropInput->blockSignals(true);
    m_amountInput->blockSignals(true);
    m_coeffInput->blockSignals(true);
    
    m_dropInput->setValue(80);
    m_amountInput->setValue(150);
    m_coeffInput->setValue(30);

    m_dropInput->blockSignals(false);
    m_amountInput->blockSignals(false);
    m_coeffInput->blockSignals(false);
} 

void ImageEffect_RainDrop::prepareEffect()
{
    m_dropInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_coeffInput->setEnabled(false);
    
    int d        = m_dropInput->value();
    int a        = m_amountInput->value();
    int c        = m_coeffInput->value();

    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();

    // Selected data from the image
    QRect selection( iface->selectedXOrg(), iface->selectedYOrg(),
                     iface->selectedWidth(), iface->selectedHeight() );

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new RainDrop(iface->getOriginalImg(), this, d, a, c, &selection));
}

void ImageEffect_RainDrop::prepareFinal()
{
    m_dropInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_coeffInput->setEnabled(false);
    
    int d       = m_dropInput->value();
    int a       = m_amountInput->value();
    int c       = m_coeffInput->value();

    Digikam::ImageIface iface(0, 0);

    // Selected data from the image
    QRect selection( iface.selectedXOrg(), iface.selectedYOrg(),
                     iface.selectedWidth(), iface.selectedHeight() );

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new RainDrop(iface.getOriginalImg(), this, d, a, c, &selection));
}

void ImageEffect_RainDrop::putPreviewData(void)
{
    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();

    Digikam::DImg imDest = m_threadedFilter->getTargetImage()
            .smoothScale(iface->previewWidth(), iface->previewHeight());
    iface->putPreviewImage(imDest.bits());

    m_imagePreviewWidget->updatePreview();
}

void ImageEffect_RainDrop::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("RainDrop"),
                           m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamRainDropImagesPlugin
