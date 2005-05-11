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

// Digikam includes.

#include <imageiface.h>
#include <imagepreviewwidget.h>
#include <imagefilters.h>

// Local includes.

#include "imageeffect_blur.h"

ImageEffect_Blur::ImageEffect_Blur(QWidget* parent)
                : KDialogBase(Plain, i18n("Blur Image"),
                              Help|Ok|Cancel, Ok,
                              parent, 0, true, true),
                  m_parent(parent)
{
    setHelp("blursharpentool.anchor", "digikam");
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePreviewWidget(240, 160, i18n("Blur Image Preview Effect"), plainPage());
    hlay1->addWidget(m_imagePreviewWidget);
 
    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label = new QLabel(i18n("Smoothness:"), plainPage());
    
    m_radiusInput = new KIntNumInput(plainPage());
    m_radiusInput->setRange(0, 20, 1, true);
    QWhatsThis::add( m_radiusInput, i18n("<p>A smoothness of 0 has no effect, "
                                         "1 and above determine the Gaussian blur matrix radius "
                                         "that determines how much to blur the image."));
    
    hlay2->addWidget(label, 1);
    hlay2->addWidget(m_radiusInput, 5);

    m_radiusInput->setValue(0);
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));
    
    adjustSize();
    disableResize();                  
    QTimer::singleShot(0, this, SLOT(slotEffect()));
}

ImageEffect_Blur::~ImageEffect_Blur()
{
}

void ImageEffect_Blur::slotEffect()
{
    enableButtonOK(m_radiusInput->value() > 0);
    
    QImage img = m_imagePreviewWidget->getOriginalClipImage();
   
    uint* data = (uint *)img.bits();
    int   w    = img.width();
    int   h    = img.height();
    int   r    = m_radiusInput->value();
        
    Digikam::ImageFilters::gaussianBlurImage(data, w, h, r);

    m_imagePreviewWidget->setPreviewImageData(img);
}

void ImageEffect_Blur::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
    
    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    int r      = m_radiusInput->value();
            
    Digikam::ImageFilters::gaussianBlurImage(data, w, h, r);
           
    iface.putOriginalData(i18n("Blur"), data);
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();
}

#include "imageeffect_blur.moc"
