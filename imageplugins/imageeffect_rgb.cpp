/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-11
 * Description : RGB adjustement plugin for ImageEditor
 *
 * Copyright 2004 by Gilles Caulier
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

// Imlib2 include.

#define X_DISPLAY_MISSING 1
#include <Imlib2.h>

// Qt includes.

#include <qlayout.h>
#include <qframe.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qspinbox.h>
#include <qslider.h>

// KDE includes.

#include <klocale.h>

// Digikam includes.

#include <imageiface.h>
#include <imagewidget.h>

// Local includes.

#include "imageeffect_rgb.h"

ImageEffect_RGB::ImageEffect_RGB(QWidget* parent)
               : KDialogBase(Plain, i18n("Color Balance"),
                             Help|User1|Ok|Cancel, Ok,
                             parent, 0, true, true, i18n("&Reset Values"))
{
    setHelp("colorbalancetool.anchor", "digikam");
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(),
                                              0, spacingHint());

    QVGroupBox *gbox = new QVGroupBox(i18n("Color Balance"),
                                      plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l  = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320,frame);
    QWhatsThis::add( m_previewWidget, i18n("<p>You can see here the image color-balance preview."));
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    topLayout->addWidget(gbox);

    QHBoxLayout *hlay  = 0;
    QLabel      *label = 0;

    hlay     = new QHBoxLayout(topLayout);
    label    = new QLabel(i18n("Cyan"), plainPage());
    label->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    hlay->addWidget(label, 1);
    m_rSlider = new QSlider(-100, 100, 1, 0, Qt::Horizontal, plainPage(), "m_rSlider");
    m_rSlider->setTickmarks(QSlider::Below);
    m_rSlider->setTickInterval(20);
    QWhatsThis::add( m_rSlider, i18n("<p>Set here the cyan/red color adjustment of the image."));
    hlay->addWidget(m_rSlider, 5);
    label    = new QLabel(i18n("Red"), plainPage());
    label->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter );
    hlay->addWidget(label, 1);
    m_rInput = new QSpinBox(-100, 100, 1, plainPage(), "m_rInput");
    hlay->addWidget(m_rInput, 1);

    hlay     = new QHBoxLayout(topLayout);
    label    = new QLabel(i18n("Magenta"), plainPage());
    label->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    hlay->addWidget(label, 1);
    m_gSlider = new QSlider(-100, 100, 1, 0, Qt::Horizontal, plainPage(), "m_gSlider");
    m_gSlider->setTickmarks(QSlider::Below);
    m_gSlider->setTickInterval(20);
    QWhatsThis::add( m_gSlider, i18n("<p>Set here the magenta/green color adjustment of the image."));
    hlay->addWidget(m_gSlider, 5);
    label    = new QLabel(i18n("Green"), plainPage());
    label->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter );
    hlay->addWidget(label, 1);
    m_gInput = new QSpinBox(-100, 100, 1, plainPage(), "m_gInput");
    hlay->addWidget(m_gInput, 1);

    hlay     = new QHBoxLayout(topLayout);
    label    = new QLabel(i18n("Yellow"), plainPage());
    label->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    hlay->addWidget(label, 1);
    m_bSlider = new QSlider(-100, 100, 1, 0, Qt::Horizontal, plainPage(), "m_bSlider");
    m_bSlider->setTickmarks(QSlider::Below);
    m_bSlider->setTickInterval(20);
    QWhatsThis::add( m_bSlider, i18n("<p>Set here the yellow/blue color adjustment of the image."));
    hlay->addWidget(m_bSlider, 5);
    label    = new QLabel(i18n("Blue"), plainPage());
    label->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter );
    hlay->addWidget(label, 1);
    m_bInput = new QSpinBox(-100, 100, 1, plainPage(), "m_bInput");
    hlay->addWidget(m_bInput, 1);

    m_rInput->setValue(0);
    m_gInput->setValue(0);
    m_bInput->setValue(0);

    connect(m_rSlider, SIGNAL(valueChanged(int)),
            m_rInput, SLOT(setValue(int)));
    connect(m_rInput, SIGNAL(valueChanged (int)),
            m_rSlider, SLOT(setValue(int)));
    connect(m_rInput, SIGNAL(valueChanged (int)),
            SLOT(slotEffect()));

    connect(m_gSlider, SIGNAL(valueChanged(int)),
            m_gInput, SLOT(setValue(int)));
    connect(m_gInput, SIGNAL(valueChanged (int)),
            m_gSlider, SLOT(setValue(int)));
    connect(m_gInput, SIGNAL(valueChanged (int)),
            SLOT(slotEffect()));

    connect(m_bSlider, SIGNAL(valueChanged(int)),
            m_bInput, SLOT(setValue(int)));
    connect(m_bInput, SIGNAL(valueChanged (int)),
            m_bSlider, SLOT(setValue(int)));
    connect(m_bInput, SIGNAL(valueChanged (int)),
            SLOT(slotEffect()));

    enableButtonOK( false );
    adjustSize();
    disableResize();
}

ImageEffect_RGB::~ImageEffect_RGB()
{
}

void ImageEffect_RGB::slotUser1()
{
    adjustSliders(0, 0, 0);
    slotEffect();
}

void ImageEffect_RGB::slotEffect()
{
    enableButtonOK(m_rInput->value() != 0 ||
                   m_gInput->value() != 0 ||
                   m_bInput->value() != 0);

    Digikam::ImageIface* iface =
        m_previewWidget->imageIface();

    uint* data  = iface->getPreviewData();
    int   w     = iface->previewWidth();
    int   h     = iface->previewHeight();

    double r = ((double)m_rInput->value() + 100.0)/100.0;
    double g = ((double)m_gInput->value() + 100.0)/100.0;
    double b = ((double)m_bInput->value() + 100.0)/100.0;
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

    double r = ((double)m_rInput->value() + 100.0)/100.0;
    double g = ((double)m_gInput->value() + 100.0)/100.0;
    double b = ((double)m_bInput->value() + 100.0)/100.0;
    double a = 1.0;

    adjustRGB(r, g, b, a, data, w, h);

    iface->putOriginalData(i18n("RGB"), data);
    delete [] data;
    accept();
}

void ImageEffect_RGB::adjustSliders(int r, int g, int b)
{
    blockSignals(true);
    m_rSlider->setValue(r);
    m_gSlider->setValue(g);
    m_bSlider->setValue(b);
    blockSignals(false);
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
    delete [] newData;
}


#include "imageeffect_rgb.moc"
