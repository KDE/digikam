/* ============================================================
 * File  : imageeffect_emboss.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

// Imlib2 include.

#define X_DISPLAY_MISSING 1
#include <Imlib2.h>

// C++ include.

#include <cstring>

// Qt includes.

#include <qlayout.h>
#include <qframe.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qimage.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kimageeffect.h>
#include <knuminput.h>

// Digikam includes.

#include <imageiface.h>
#include <imagepreviewwidget.h>

// Local includes.

#include "imageeffect_emboss.h"

namespace DigikamEmbossImagesPlugin
{

ImageEffect_Emboss::ImageEffect_Emboss(QWidget* parent)
                    : KDialogBase(Plain, i18n("Emboss"),
                                  Help|Ok|Cancel, Ok,
                                  parent, 0, true, true),
                      m_parent(parent)
{
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Emboss"), 
                                       "0.7.0-cvs",
                                       I18N_NOOP("An embossed image effect plugin for Digikam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Gilles Caulier", 
                                       0,
                                       "http://digikam.sourceforge.net");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Emboss handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePreviewWidget(240, 160, 
                                                           i18n("Emboss image preview"),
                                                           plainPage());
    hlay1->addWidget(m_imagePreviewWidget);
    
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label = new QLabel(i18n("Radius :"), plainPage());
    m_radiusInput = new KDoubleNumInput(plainPage());
    m_radiusInput->setPrecision(1);
    m_radiusInput->setRange(0.1, 10.0, 0.1, true);
    m_radiusInput->setValue(1.0);
    hlay->addWidget(label, 1);
    hlay->addWidget(m_radiusInput, 5);

    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label2 = new QLabel(i18n("Sigma :"), plainPage());
    m_sigmaInput = new KDoubleNumInput(plainPage());
    m_sigmaInput->setPrecision(1);
    m_sigmaInput->setRange(0.1, 10.0, 0.1, true);
    m_sigmaInput->setValue(1.0);
    hlay2->addWidget(label2, 1);
    hlay2->addWidget(m_sigmaInput, 5);

    // -------------------------------------------------------------
    
    adjustSize();
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_radiusInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));            
    
    connect(m_sigmaInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));            

}

ImageEffect_Emboss::~ImageEffect_Emboss()
{
}

void ImageEffect_Emboss::slotHelp()
{
    KApplication::kApplication()->invokeHelp("emboss",
                                             "digikamimageplugins");
}

void ImageEffect_Emboss::closeEvent(QCloseEvent *e)
{
    delete m_radiusInput;
    delete m_imagePreviewWidget;
    e->accept();    
}

void ImageEffect_Emboss::slotEffect()
{
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    double radius = m_radiusInput->value();
    double sigma  = m_sigmaInput->value();
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    QImage newImage = KImageEffect::emboss(image, radius, sigma);
    m_imagePreviewWidget->setPreviewImageData(newImage);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
}

void ImageEffect_Emboss::slotOk()
{
    accept();
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
    
    uint* data    = iface.getOriginalData();
    int w         = iface.originalWidth();
    int h         = iface.originalHeight();
    double radius = m_radiusInput->value();
    double sigma  = m_sigmaInput->value();

    if (data) 
        {
        QImage image;
        image.create( w, h, 32 );
        image.setAlphaBuffer(true) ;
        memcpy(image.bits(), data, image.numBytes());

        QImage newImage = KImageEffect::emboss(image, radius, sigma);
    
        memcpy(data, newImage.bits(), newImage.numBytes());
        iface.putOriginalData(data);
        delete [] data;
        }
    
    m_parent->setCursor( KCursor::arrowCursor() );        
}

}  // NameSpace DigikamEmbossImagesPlugin

#include "imageeffect_emboss.moc"
