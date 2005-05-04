/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-16
 * Description : HSL adjustement plugin for ImageEditor
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
#include <imagefilters.h>

// Local includes.

#include "imageeffect_hsl.h"

ImageEffect_HSL::ImageEffect_HSL(QWidget* parent)
               : KDialogBase(Plain, i18n("Hue/Saturation/Lightness"),
                             Help|User1|Ok|Cancel, Ok,
                             parent, 0, true, true, i18n("&Reset Values"))
{
    setHelp("hsladjusttool.anchor", "digikam");
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(),
                                              0, spacingHint());

    QVGroupBox *gbox = new QVGroupBox(i18n("Hue/Saturation/Lightness Adjustments"),
                                      plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l  = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320,frame);
    QWhatsThis::add( m_previewWidget, i18n("<p>You can see here the image Hue/Saturation/Lightness adjustments preview."));
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    topLayout->addWidget(gbox);

    QHBoxLayout *hlay  = 0;
    QLabel      *label = 0;

    hlay     = new QHBoxLayout(topLayout);
    label    = new QLabel(i18n("Hue:"), plainPage());
    m_hInput = new KDoubleNumInput(plainPage());
    m_hInput->setPrecision(0);
    m_hInput->setRange(-180.0, 180.0, 1.0, true);
    QWhatsThis::add( m_hInput, i18n("<p>Set here the hue adjustment of the image."));
    hlay->addWidget(label,1);
    hlay->addWidget(m_hInput,5);

    hlay     = new QHBoxLayout(topLayout);
    label    = new QLabel(i18n("Saturation:"), plainPage());
    m_sInput = new KDoubleNumInput(plainPage());
    m_sInput->setPrecision(2);
    m_sInput->setRange(-100.0, 100.0, 0.01, true);
    QWhatsThis::add( m_sInput, i18n("<p>Set here the saturation adjustment of the image."));
    hlay->addWidget(label,1);
    hlay->addWidget(m_sInput,5);

    hlay     = new QHBoxLayout(topLayout);
    label    = new QLabel(i18n("Lightness:"), plainPage());
    m_lInput = new KDoubleNumInput(plainPage());
    m_lInput->setPrecision(2);
    m_lInput->setRange(-100.0, 100.0, 0.01, true);
    QWhatsThis::add( m_lInput, i18n("<p>Set here the lightness adjustment of the image."));    
    hlay->addWidget(label,1);
    hlay->addWidget(m_lInput,5);

    m_hInput->setValue(0.0);
    m_sInput->setValue(0.0);
    m_lInput->setValue(0.0);

    connect(m_hInput, SIGNAL(valueChanged (double)),
            SLOT(slotEffect()));
            
    connect(m_sInput, SIGNAL(valueChanged (double)),
            SLOT(slotEffect()));
            
    connect(m_lInput, SIGNAL(valueChanged (double)),
            SLOT(slotEffect()));

    enableButtonOK( false );
    adjustSize();
    disableResize();                  
}

ImageEffect_HSL::~ImageEffect_HSL()
{
}

void ImageEffect_HSL::slotUser1()
{
    blockSignals(true);
    m_hInput->setValue(0.0);
    m_sInput->setValue(0.0);
    m_lInput->setValue(0.0);
    blockSignals(false);
    slotEffect();
} 

void ImageEffect_HSL::slotEffect()
{
    double hu  = m_hInput->value();
    double sa  = m_sInput->value();    
    double lu  = m_lInput->value();
    
    enableButtonOK( hu != 0.0  || sa != 0.0 || lu != 0.0);
    
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data = iface->getPreviewData();
    int   w    = iface->previewWidth();
    int   h    = iface->previewHeight();
        
    Digikam::ImageFilters::hueSaturationLightnessImage(data, w, h, hu, sa, lu);
                
    iface->putPreviewData(data);
    delete [] data;
    m_previewWidget->update();
}

void ImageEffect_HSL::slotOk()
{
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data = iface->getOriginalData();
    int   w    = iface->originalWidth();
    int   h    = iface->originalHeight();
        
    double hu  = m_hInput->value();
    double sa  = m_sInput->value();    
    double lu  = m_lInput->value();

    Digikam::ImageFilters::hueSaturationLightnessImage(data, w, h, hu, sa, lu);

    iface->putOriginalData(i18n("HSL"), data);
    delete [] data;
    accept();
}


#include "imageeffect_hsl.moc"
