/* ============================================================
 * File  : imageeffect_hsl.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-16
 * Description : HSL adjustement plugin for ImageEditor
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

#include "imageeffect_hsl.h"

ImageEffect_HSL::ImageEffect_HSL(QWidget* parent)
               : KDialogBase(Plain, i18n("HSL balance"),
                             Help|User1|Ok|Cancel, Ok,
                             parent, 0, true, true, i18n("&Reset values"))
{
    setHelp("imageviewer.anchor", "digikam");
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(),
                                              0, spacingHint());

    QVGroupBox *gbox = new QVGroupBox(i18n("Hue/Saturation/Lightness balance"),
                                      plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l  = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320,frame);
    QWhatsThis::add( m_previewWidget, i18n("<p>You can see here the image HSL balance preview."));
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    topLayout->addWidget(gbox);

    QHBoxLayout *hlay  = 0;
    QLabel      *label = 0;

    hlay     = new QHBoxLayout(topLayout);
    label    = new QLabel(i18n("Hue:"), plainPage());
    m_hInput = new KDoubleNumInput(plainPage());
    m_hInput->setPrecision(0);
    m_hInput->setRange(0.0, 360.0, 1.0, true);
    QWhatsThis::add( m_hInput, i18n("<p>Set here the hue adjustment of the image."));
    hlay->addWidget(label,1);
    hlay->addWidget(m_hInput,5);

    hlay     = new QHBoxLayout(topLayout);
    label    = new QLabel(i18n("Saturation:"), plainPage());
    m_sInput = new KDoubleNumInput(plainPage());
    m_sInput->setPrecision(2);
    m_sInput->setRange(0.0, 1.0, 0.01, true);
    QWhatsThis::add( m_sInput, i18n("<p>Set here the saturation adjustment of the image."));
    hlay->addWidget(label,1);
    hlay->addWidget(m_sInput,5);

    hlay     = new QHBoxLayout(topLayout);
    label    = new QLabel(i18n("Lightness:"), plainPage());
    m_lInput = new KDoubleNumInput(plainPage());
    m_lInput->setPrecision(2);
    m_lInput->setRange(0.0, 1.0, 0.01, true);
    QWhatsThis::add( m_lInput, i18n("<p>Set here the lightness adjustment of the image."));    
    hlay->addWidget(label,1);
    hlay->addWidget(m_lInput,5);

    m_hInput->setValue(180.0);
    m_sInput->setValue(0.0);
    m_lInput->setValue(0.5);

    connect(m_hInput, SIGNAL(valueChanged (double)),
            SLOT(slotEffect()));
            
    connect(m_sInput, SIGNAL(valueChanged (double)),
            SLOT(slotEffect()));
            
    connect(m_lInput, SIGNAL(valueChanged (double)),
            SLOT(slotEffect()));

    adjustSize();
}

ImageEffect_HSL::~ImageEffect_HSL()
{
}

void ImageEffect_HSL::slotUser1()
{
    blockSignals(true);
    m_hInput->setValue(180.0);
    m_sInput->setValue(0.0);
    m_lInput->setValue(0.5);
    blockSignals(false);
    slotEffect();
} 

void ImageEffect_HSL::slotEffect()
{
    Digikam::ImageIface* iface =
        m_previewWidget->imageIface();

    uint* data  = iface->getPreviewData();
    int   w     = iface->previewWidth();
    int   h     = iface->previewHeight();
        
    double hu = m_hInput->value();
    double sa = m_sInput->value();    
    double lu = m_lInput->value();
    int    al = 0;
    
    adjustHSL(hu, sa, lu, al, data, w, h);    
                
    iface->putPreviewData(data);
    delete [] data;
    m_previewWidget->update();
}

void ImageEffect_HSL::slotOk()
{
    Digikam::ImageIface* iface =
        m_previewWidget->imageIface();

    uint* data  = iface->getOriginalData();
    int   w     = iface->originalWidth();
    int   h     = iface->originalHeight();
        
    double hu = m_hInput->value();
    double sa = m_sInput->value();    
    double lu = m_lInput->value();
    int    al = 0;

    adjustHSL(hu, sa, lu, al, data, w, h);    

    iface->putOriginalData(data);
    delete [] data;
    accept();
}

void ImageEffect_HSL::adjustHSL(double hu, double sa, double lu, int al, uint *data, int w, int h)
{
    int red, green, blue, alpha;
    uint* newData = new uint[w*h];
    memcpy(newData, data, w*h*sizeof(unsigned int));
    
    Imlib_Context context = imlib_context_new();
    imlib_context_push(context);

    Imlib_Image imTop = imlib_create_image_using_copied_data(w, h, newData);
    imlib_context_set_image(imTop);
    
    imlib_context_set_color_hlsa((float)hu, (float)lu, (float)sa, al);
    imlib_context_get_color(&red, &green, &blue, &alpha); 
    
    double r = (double)red   * (2.0 / 255.0);
    double g = (double)green * (2.0 / 255.0);
    double b = (double)blue  * (2.0 / 255.0);
    double a = (double)alpha * (2.0 / 255.0);
        
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
    delete [] newData;
}


#include "imageeffect_hsl.moc"
