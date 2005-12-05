/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-16
 * Description : digiKam image editor Hue/Saturation/Lightness 
 *               correction tool
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
#include <qtimer.h>

// KDE includes.

#include <knuminput.h>
#include <klocale.h>
#include <kapplication.h>
#include <kcursor.h>

// Local includes.

#include "imageiface.h"
#include "imagewidget.h"
#include "dimg.h"
#include "hslmodifier.h"
#include "imageeffect_hsl.h"

ImageEffect_HSL::ImageEffect_HSL(QWidget* parent)
               : KDialogBase(Plain, i18n("Hue/Saturation/Lightness"),
                             Help|User1|Ok|Cancel, Ok,
                             parent, 0, true, true, i18n("&Reset Values"))
{
    m_timer = 0L;
    setHelp("hsladjusttool.anchor", "digikam");
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());
    
    QFrame *frame = new QFrame(plainPage());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320, frame);
    QWhatsThis::add( m_previewWidget, i18n("<p>Here you can see the image Hue/Saturation/Lightness adjustments preview."));
    l->addWidget(m_previewWidget, 0);
    topLayout->addWidget(frame);
        
    // -------------------------------------------------------------            
    
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
    
    // -------------------------------------------------------------

    connect(m_hInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));
            
    connect(m_sInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));
            
    connect(m_lInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));
    
    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));              
            
    // -------------------------------------------------------------            

    enableButtonOK( false );
    resize(configDialogSize("HSL Correction Tool Dialog"));                
}

ImageEffect_HSL::~ImageEffect_HSL()
{
    saveDialogSize("HSL Correction Tool Dialog");
}

void ImageEffect_HSL::closeEvent(QCloseEvent *e)
{
    if (m_timer)
       delete m_timer;
           
    delete m_previewWidget;
    e->accept();
}

void ImageEffect_HSL::slotTimer()
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
    kapp->setOverrideCursor( KCursor::waitCursor() );    
    double hu  = m_hInput->value();
    double sa  = m_sInput->value();    
    double lu  = m_lInput->value();
    
    enableButtonOK( hu != 0.0 || sa != 0.0 || lu != 0.0);
    
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    Digikam::DImg preview = iface->getPreviewImage(); 
    Digikam::HSLModifier cmod;
    cmod.setHue(hu);
    cmod.setSaturation(sa);
    cmod.setLightness(lu);
    cmod.applyHSL(preview);
    iface->putPreviewImage(preview);
    
    m_previewWidget->update();
    kapp->restoreOverrideCursor();
}

void ImageEffect_HSL::slotOk()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

    double hu  = m_hInput->value();
    double sa  = m_sInput->value();    
    double lu  = m_lInput->value();

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    Digikam::DImg original = iface->getOriginalImage(); 
    Digikam::HSLModifier cmod;
    cmod.setHue(hu);
    cmod.setSaturation(sa);
    cmod.setLightness(lu);
    cmod.applyHSL(original);

    iface->putOriginalImage(i18n("HSL Adjustments"), original);
    kapp->restoreOverrideCursor();
    accept();
}

#include "imageeffect_hsl.moc"
