/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-09
 * Description : Blur image filter for ImageEditor
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

// C++ include.

#include <cstring>

// Qt includes.

#include <qlayout.h>
#include <qframe.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtimer.h>

// KDE includes.

#include <knuminput.h>
#include <kcursor.h>
#include <klocale.h>
#include <kapplication.h>

// Digikam includes.

#include <imageiface.h>
#include <imagepannelwidget.h>
#include <imagefilters.h>

// Local includes.

#include "imageeffect_blur.h"

ImageEffect_Blur::ImageEffect_Blur(QWidget* parent)
                : KDialogBase(Plain, i18n("Blur Image"),
                              Help|Ok|Cancel, Ok,
                              parent, 0, true, true),
                  m_parent(parent)
{
    m_timer = 0L;
    setHelp("blursharpentool.anchor", "digikam");
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePannelWidget(480, 320, i18n("Preview"), plainPage());
    hlay1->addWidget(m_imagePreviewWidget);

    // -------------------------------------------------------------
    
    QVGroupBox *gbox = m_imagePreviewWidget->settingsGroupBox();
    QLabel *label = new QLabel(i18n("Smoothness:"), gbox);
    
    m_radiusInput = new KIntNumInput(gbox);
    m_radiusInput->setRange(0, 20, 1, true);
    m_radiusInput->setValue(0);
    QWhatsThis::add( m_radiusInput, i18n("<p>A smoothness of 0 has no effect, "
                                         "1 and above determine the Gaussian blur matrix radius "
                                         "that determines how much to blur the image."));
    
    // -------------------------------------------------------------
    
    resize(configDialogSize("Blur Tool Dialog"));         
    QTimer::singleShot(0, this, SLOT(slotEffect()));
                                             
    // -------------------------------------------------------------
    
    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));
     
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));    
}

ImageEffect_Blur::~ImageEffect_Blur()
{
    saveDialogSize("Blur Tool Dialog");    
}

void ImageEffect_Blur::slotTimer()
{
    if (m_timer)
       {
       m_timer->stop();
       delete m_timer;
       }
    
    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL(timeout()),
             this, SLOT(slotEffect()) );
    m_timer->start(500, true);
}

void ImageEffect_Blur::slotEffect()
{
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    enableButtonOK(m_radiusInput->value() > 0);
    
    QImage img = m_imagePreviewWidget->getOriginalClipImage();
   
    uint* data = (uint *)img.bits();
    int   w    = img.width();
    int   h    = img.height();
    int   r    = m_radiusInput->value();
        
    Digikam::ImageFilters::gaussianBlurImage(data, w, h, r);

    m_imagePreviewWidget->setPreviewImageData(img);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
}

void ImageEffect_Blur::slotOk()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );               
    Digikam::ImageIface iface(0, 0);
    
    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    int r      = m_radiusInput->value();
            
    Digikam::ImageFilters::gaussianBlurImage(data, w, h, r);
           
    iface.putOriginalData(i18n("Blur"), data);
    delete [] data;
    kapp->restoreOverrideCursor(); 
    accept();
}

#include "imageeffect_blur.moc"
