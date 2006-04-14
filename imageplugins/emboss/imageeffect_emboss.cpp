/* ============================================================
 * File  : imageeffect_emboss.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2004-08-26
 * Description : a digiKam image editor plugin to emboss 
 *               an image.
 * 
 * Copyright 2004-2005 by Gilles Caulier
 * Copyright 2006 by Gilles Caulier and Marcel Wiesweg
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

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <knuminput.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "emboss.h"
#include "imageeffect_emboss.h"

namespace DigikamEmbossImagesPlugin
{

ImageEffect_Emboss::ImageEffect_Emboss(QWidget* parent, QString title, QFrame* banner)
                  : Digikam::CtrlPanelDlg(parent, title, "emboss", false, false, true,
                                          Digikam::ImagePannelWidget::SeparateViewAll, banner)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Emboss Image"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("An embossed image effect plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2006, Gilles Caulier\n"
                                       "(c) 2006, Gilles Caulier and Marcel Wiesweg",
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at kdemail dot net");

    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Emboss algorithm"), 
                     "pieter dot voloshyn at gmail dot com");         

    about->addAuthor("Marcel Wiesweg", I18N_NOOP("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);
    
    // -------------------------------------------------------------
    
    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 1, 1, 0, spacingHint());
    QLabel *label1 = new QLabel(i18n("Depth:"), gboxSettings);
    
    m_depthInput = new KIntNumInput(gboxSettings);
    m_depthInput->setRange(10, 300, 1, true);
    QWhatsThis::add( m_depthInput, i18n("<p>Set here the depth of the embossing image effect.") );
                                            
    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 1);
    gridSettings->addMultiCellWidget(m_depthInput, 1, 1, 0, 1);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    
    connect(m_depthInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer())); 
}

ImageEffect_Emboss::~ImageEffect_Emboss()
{
}

void ImageEffect_Emboss::renderingFinished()
{
    m_depthInput->setEnabled(true);
}

void ImageEffect_Emboss::resetValues()
{
    m_depthInput->blockSignals(true);
    m_depthInput->setValue(30);
    m_depthInput->blockSignals(false);
} 

void ImageEffect_Emboss::prepareEffect()
{
    m_depthInput->setEnabled(false);

    Digikam::DImg image = m_imagePreviewWidget->getOriginalRegionImage();

    int depth = m_depthInput->value();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new Emboss(&image, this, depth));
}

void ImageEffect_Emboss::prepareFinal()
{
    m_depthInput->setEnabled(false);

    int depth = m_depthInput->value();

    Digikam::ImageIface iface(0, 0);
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new Emboss(iface.getOriginalImg(), this, depth));
}

void ImageEffect_Emboss::putPreviewData(void)
{
    m_imagePreviewWidget->setPreviewImage(m_threadedFilter->getTargetImage());
}

void ImageEffect_Emboss::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Emboss"),
                           m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamEmbossImagesPlugin

#include "imageeffect_emboss.moc"
