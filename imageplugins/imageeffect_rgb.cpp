/* ============================================================
 * File  : imageeffect_rgb.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-11
 * Description : RGB adjustement plugin for ImageEditor
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

#include "imageeffect_rgb.h"

ImageEffect_RGB::ImageEffect_RGB(QWidget* parent)
               : KDialogBase(Plain, i18n("Colors balance"),
                             Help|User1|Ok|Cancel, Ok,
                             parent, 0, true, true, i18n("&Reset values"))
{
    setHelp("imageviewer.anchor", "digikam");
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(),
                                              0, spacingHint());

    QVGroupBox *gbox = new QVGroupBox(i18n("Colors balance"),
                                      plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l  = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480,320,frame);
    QWhatsThis::add( m_previewWidget, i18n("<p>You can see here the image color balance preview."));
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    topLayout->addWidget(gbox);

    QHBoxLayout *hlay  = 0;
    QLabel      *label = 0;

    hlay     = new QHBoxLayout(topLayout);
    label    = new QLabel(i18n("Red :"), plainPage());
    m_rInput = new KDoubleNumInput(plainPage());
    m_rInput->setRange(0.0, 2.0, 0.01, true);
    m_rInput->setPrecision(2);
    QWhatsThis::add( m_rInput, i18n("<p>Set here the red color adjustment of the image."));
    hlay->addWidget(label,1);
    hlay->addWidget(m_rInput,5);

    hlay     = new QHBoxLayout(topLayout);
    label    = new QLabel(i18n("Green :"), plainPage());
    m_gInput = new KDoubleNumInput(plainPage());
    m_gInput->setRange(0.0, 2.0, 0.01, true);
    m_gInput->setPrecision(2);
    QWhatsThis::add( m_gInput, i18n("<p>Set here the green color adjustment of the image."));
    hlay->addWidget(label,1);
    hlay->addWidget(m_gInput,5);

    hlay     = new QHBoxLayout(topLayout);
    label    = new QLabel(i18n("Blue :"), plainPage());
    m_bInput = new KDoubleNumInput(plainPage());
    m_bInput->setRange(0.0, 2.0, 0.01, true);
    m_bInput->setPrecision(2);
    QWhatsThis::add( m_bInput, i18n("<p>Set here the blue color adjustment of the image."));    
    hlay->addWidget(label,1);
    hlay->addWidget(m_bInput,5);

    m_rInput->setValue(1.0);
    m_gInput->setValue(1.0);
    m_bInput->setValue(1.0);

    connect(m_rInput, SIGNAL(valueChanged (double)),
            SLOT(slotEffect()));
            
    connect(m_gInput, SIGNAL(valueChanged (double)),
            SLOT(slotEffect()));
            
    connect(m_bInput, SIGNAL(valueChanged (double)),
            SLOT(slotEffect()));

    adjustSize();
}

ImageEffect_RGB::~ImageEffect_RGB()
{
}

void ImageEffect_RGB::slotUser1()
{
    m_rInput->setValue(1.0);
    m_gInput->setValue(1.0);
    m_bInput->setValue(1.0);
    slotEffect();
} 

void ImageEffect_RGB::slotEffect()
{
    Digikam::ImageIface* iface =
        m_previewWidget->imageIface();

    uint* data  = iface->getPreviewData();
    int   w     = iface->previewWidth();
    int   h     = iface->previewHeight();
        
    double r = m_rInput->value();
    double g = m_gInput->value();    
    double b = m_bInput->value();
    double a = 1.0;
    
    adjustRGB(r, g, b, a, data, w, h);    
                
    iface->putPreviewData(data);
    delete [] data;
    m_previewWidget->update();
}

void ImageEffect_RGB::slotOk()
{
    Digikam::ImageIface* iface =
        m_previewWidget->imageIface();

    uint* data  = iface->getOriginalData();
    int   w     = iface->originalWidth();
    int   h     = iface->originalHeight();
        
    double r = m_rInput->value();
    double g = m_gInput->value();    
    double b = m_bInput->value();
    double a = 1.0;

    adjustRGB(r, g, b, a, data, w, h);    

    iface->putOriginalData(data);
    delete [] data;
    accept();
}

void ImageEffect_RGB::adjustRGB(double r, double g, double b, double a, uint *data, int w, int h)
{
    uint* newData = new uint[w*h];
    memcpy(newData, data, w*h*sizeof(unsigned int));
    
    Imlib_Context context = imlib_context_new();
    imlib_context_push(context);

    Imlib_Image imTop = imlib_create_image_using_copied_data(w, h, newData);
    imlib_context_set_image(imTop);
    
    Imlib_Color_Modifier modifier;
    DATA8 r_table[256];
    DATA8 g_table[256];
    DATA8 b_table[256];
    DATA8 a_table[256];
    DATA8 dummy_table[256];

    if (r == 1.0 && g == 1.0 && b == 1.0 && a == 1.0) 
        return ;
    
    modifier = imlib_create_color_modifier();
    
    imlib_context_set_color_modifier(modifier);
    imlib_reset_color_modifier();
    
    if (r == g && r == b && r == a) 
       {
       imlib_modify_color_modifier_gamma(r);
       }
    else 
       {
       imlib_get_color_modifier_tables(r_table, g_table, b_table, a_table);

       if(r != 1.0) 
          {
          imlib_modify_color_modifier_gamma(r);
          imlib_get_color_modifier_tables(r_table, dummy_table, dummy_table, dummy_table);
          imlib_reset_color_modifier();
          }

       if(g != 1.0) 
          {
          imlib_modify_color_modifier_gamma(g);
          imlib_get_color_modifier_tables(dummy_table, g_table, dummy_table, dummy_table);
          imlib_reset_color_modifier();
          }

       if(b != 1.0) 
          {
          imlib_modify_color_modifier_gamma(b);
          imlib_get_color_modifier_tables(dummy_table, dummy_table, b_table, dummy_table);
          imlib_reset_color_modifier();
          }

       if(a != 1.0) 
          {
          imlib_modify_color_modifier_gamma(a);
          imlib_get_color_modifier_tables(dummy_table, dummy_table, dummy_table, a_table);
          imlib_reset_color_modifier();
          }

       imlib_set_color_modifier_tables(r_table, g_table, b_table, a_table);
       }

    imlib_apply_color_modifier();
    imlib_free_color_modifier();

    uint* ptr = imlib_image_get_data_for_reading_only();
    memcpy(data, ptr, w*h*sizeof(unsigned int));
    
    imlib_context_set_image(imTop);
    imlib_free_image_and_decache();
    
    imlib_context_pop();
    imlib_context_free(context);
}


#include "imageeffect_rgb.moc"
