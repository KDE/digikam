/* ============================================================
 * File  : imageeffect_sharpen.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-09
 * Description : Sharpen image filter for ImageEditor
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
#include <qwhatsthis.h>

// KDE includes.

#include <knuminput.h>
#include <klocale.h>

// Digikam includes.

#include <imageiface.h>
#include <imagewidget.h>

// Local includes.

#include "imageeffect_sharpen.h"

ImageEffect_Sharpen::ImageEffect_Sharpen(QWidget* parent)
                   : KDialogBase(Plain, i18n("Sharpen image"),
                                 Help|Ok|Cancel, Ok,
                                 parent, 0, true, true)
{
    setHelp("imageviewer.anchor", "digikam");
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(),
                                              0, spacingHint());

    /*QVGroupBox *gbox = new QVGroupBox(i18n("Sharpen image"),
                                      plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l  = new QVBoxLayout(frame, 5, 0);
    
    m_previewWidget = new Digikam::ImageWidget(480, 320, frame);
    QWhatsThis::add( m_previewWidget, i18n("<p>You can see here the image sharpen preview."));
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    topLayout->addWidget(gbox);*/
                                                  
    QHBoxLayout *hlay  = 0;
    QLabel      *label = 0;

    hlay          = new QHBoxLayout(topLayout);
    label         = new QLabel(i18n("Radius :"), plainPage());
    
    m_radiusInput = new KIntNumInput(plainPage());
    m_radiusInput->setRange(0, 10, 1, true);
    QWhatsThis::add( m_radiusInput, i18n("<p>A radius of 0 has no effect, "
                                         "1 and above determine the sharpen matrix radius "
                                         "that determines how much to sharpen the image."));
    
    hlay->addWidget(label, 1);
    hlay->addWidget(m_radiusInput, 5);

    m_radiusInput->setValue(0);
    
/*    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            SLOT(slotEffect()));*/
                
    adjustSize();
}

ImageEffect_Sharpen::~ImageEffect_Sharpen()
{
}

void ImageEffect_Sharpen::slotEffect()
{
/*    Digikam::ImageIface* iface =
        m_previewWidget->imageIface();
   
    uint* data = iface->getPreviewData();
    int   w    = iface->previewWidth();
    int   h    = iface->previewHeight();
    int   r    = m_radiusInput->value();
        
    sharpen(data, w, h, r);
           
    iface->putPreviewData(data);
    delete [] data;
    m_previewWidget->update();*/
}

void ImageEffect_Sharpen::slotOk()
{
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    int r      = m_radiusInput->value();
            
    sharpen(data, w, h, r);
           
    iface.putOriginalData(data);
    delete [] data;
    accept();
}

void ImageEffect_Sharpen::sharpen(uint* data, int w, int h, int r)
{
    Imlib_Context context = imlib_context_new();
    imlib_context_push(context);

    Imlib_Image imTop = imlib_create_image_using_copied_data(w, h, data);
    imlib_context_set_image(imTop);
    
    imlib_image_sharpen( r );
    
    uint* ptr = imlib_image_get_data_for_reading_only();
    memcpy(data, ptr, w*h*sizeof(unsigned int));
    
    imlib_context_set_image(imTop);
    imlib_free_image_and_decache();
    
    imlib_context_pop();
    imlib_context_free(context);
}

#include "imageeffect_sharpen.moc"
