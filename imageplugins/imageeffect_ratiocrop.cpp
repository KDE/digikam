/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-06
 * Description : Ratio crop tool for ImageEditor
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

// C++ include.

#include <cstring>

// Qt includes.

#include <qlayout.h>
#include <qframe.h>
#include <qrect.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtimer.h>
#include <qcombobox.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <knuminput.h>

// Digikam includes.

#include <imageiface.h>
#include <imagepaniconwidget.h>

// Local includes.

#include "imageeffect_ratiocrop.h"

ImageEffect_RatioCrop::ImageEffect_RatioCrop(QWidget* parent)
                     : KDialogBase(Plain, i18n("Ratio Crop"),
                                   Help|User1|Ok|Cancel, Ok,
                                   parent, 0, true, true, 
                                   i18n("&Reset Values")),
                       m_parent(parent)
{
    setHelp("ratiocroptool.anchor", "digikam");
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QVGroupBox *gbox = new QVGroupBox(i18n("Ratio Crop Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_imageSelectionWidget = new Digikam::ImagePanIconWidget(480, 320, frame);
    l->addWidget(m_imageSelectionWidget, 0, Qt::AlignCenter);
    topLayout->addWidget(gbox);
 
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label = new QLabel(i18n("Aspect Ratio:"), plainPage());
    m_ratioCB = new QComboBox( false, plainPage() );
    m_ratioCB->insertItem( i18n("6 x 8 cm") );
    m_ratioCB->insertItem( i18n("9 x 13 cm") );
    m_ratioCB->insertItem( i18n("10 x 15 cm") );
    m_ratioCB->insertItem( i18n("11 x 15 cm") );
    m_ratioCB->insertItem( i18n("13 x 18 cm") );
    m_ratioCB->insertItem( i18n("13 x 19 cm") );
    m_ratioCB->insertItem( i18n("15 x 20 cm") );
    m_ratioCB->insertItem( i18n("15 x 21 cm") );
    m_ratioCB->insertItem( i18n("18 x 24 cm") );
    m_ratioCB->insertItem( i18n("18 x 25 cm") );
    m_ratioCB->insertItem( i18n("20 x 27 cm") );
    m_ratioCB->insertItem( i18n("20 x 30 cm") );
    m_ratioCB->insertItem( i18n("21 x 30 cm") );
    m_ratioCB->insertItem( i18n("25 x 38 cm") );
    m_ratioCB->insertItem( i18n("30 x 40 cm") );
    m_ratioCB->insertItem( i18n("30 x 45 cm") );
    m_ratioCB->insertItem( i18n("40 x 50 cm") );
    QWhatsThis::add( m_ratioCB, i18n("<p>Select here your aspect ratio for cropping."));
    
    QLabel *label2 = new QLabel(i18n("Orientation:"), plainPage());
    m_orientCB = new QComboBox( false, plainPage() );
    m_orientCB->insertItem( i18n("Landscape") );
    m_orientCB->insertItem( i18n("Portrait") );
    
    hlay->addWidget(label, 1);
    hlay->addWidget(m_ratioCB, 2);
    hlay->addWidget(label2, 1);
    hlay->addWidget(m_orientCB, 2);

    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label3 = new QLabel(i18n("Width:"), plainPage());
    m_widthInput = new KIntNumInput(plainPage());
    m_widthInput->setRange(10, m_imageSelectionWidget->getOriginalImageWidth(), 1, true);
    hlay2->addWidget(label3, 1);
    hlay2->addWidget(m_widthInput, 5);
    
    QHBoxLayout *hlay3 = new QHBoxLayout(topLayout);
    QLabel *label4 = new QLabel(i18n("Height:"), plainPage());
    m_heightInput = new KIntNumInput(plainPage());
    m_heightInput->setRange(10, m_imageSelectionWidget->getOriginalImageHeight(), 1, true);
    hlay3->addWidget(label4, 1);
    hlay3->addWidget(m_heightInput, 5);
    
    connect(m_ratioCB, SIGNAL(activated(int)),
            this, SLOT(slotRatioChanged()));
    
    connect(m_orientCB, SIGNAL(activated(int)),
            this, SLOT(slotOrientChanged(int)));
        
    connect(m_widthInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotWidthChanged(int)));

    connect(m_heightInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotHeightChanged(int)));

    connect(m_imageSelectionWidget, SIGNAL(signalSelectionMoved(QRect, bool)),
            this, SLOT(slotSelectionMoved(QRect, bool)));
    
    connect(m_imageSelectionWidget, SIGNAL(signalSelectionChanged(QRect)),
            this, SLOT(slotSelectionChanged(QRect)));                                            
            
    QTimer::singleShot(0, this, SLOT(slotUser1()));
    adjustSize();
    disableResize();                  
}

ImageEffect_RatioCrop::~ImageEffect_RatioCrop()
{
}

void ImageEffect_RatioCrop::slotUser1()
{
    m_widthInput->blockSignals(true);
    m_heightInput->blockSignals(true);
    m_widthInput->setValue((int)(m_imageSelectionWidget->getOriginalImageWidth() / 2.0));
    m_heightInput->setValue((int)(m_imageSelectionWidget->getOriginalImageHeight() / 2.0));
    slotWidthChanged((int)(m_imageSelectionWidget->getOriginalImageWidth() / 2.0));
    m_imageSelectionWidget->setCenterSelection();
    m_widthInput->blockSignals(false);
    m_heightInput->blockSignals(false);
} 

void ImageEffect_RatioCrop::slotSelectionMoved(QRect rect, bool target)
{
    if (!target) return;
    
    updateSelectionSize(rect);
}

void ImageEffect_RatioCrop::slotSelectionChanged(QRect rect)
{
    updateSelectionSize(rect);
}

void ImageEffect_RatioCrop::updateSelectionSize(QRect rect)
{
    m_widthInput->blockSignals(true);
    m_heightInput->blockSignals(true);
    m_widthInput->setValue(rect.width());
    m_heightInput->setValue(rect.height());
    m_widthInput->blockSignals(false);
    m_heightInput->blockSignals(false);
}

void ImageEffect_RatioCrop::slotWidthChanged(int w)
{
    QRect currentPos = m_imageSelectionWidget->getRegionSelection();
    int r = m_ratioCB->currentItem();
    int o = m_orientCB->currentItem();

    currentPos.setWidth(w);
    
    switch(r)
       {
       case CMS06X08:  
       case CMS15x20:   
       case CMS18x24:             
       case CMS30x40:
          if ( o )  
             currentPos.setHeight((int)(w * 1.3333333333333));   // Landscape
          else                       
             currentPos.setHeight((int)(w * 0.75));              // Portrait
          break;
       
       case CMS09X13:           
          if ( o )  
             currentPos.setHeight((int)(w * 1.4444444444444));   // Landscape
          else                       
             currentPos.setHeight((int)(w * 0.69230769230769));  // Portrait
          break;
       
       case CMS10x15:           
       case CMS20x30: 
       case CMS30x45:          
          if ( o )  
             currentPos.setHeight((int)(w * 1.5));               // Landscape
          else                       
             currentPos.setHeight((int)(w * 0.66666666666667));  // Portrait
          break;

       case CMS11x15:           
          if ( o )  
             currentPos.setHeight((int)(w * 1.3636363636364));   // Landscape
          else                       
             currentPos.setHeight((int)(w * 0.73333333333333));  // Portrait
          break;
                 
       case CMS13x18:          
          if ( o )  
             currentPos.setHeight((int)(w * 1.3846153846154));   // Landscape
          else                       
             currentPos.setHeight((int)(w * 0.72222222222222));  // Portrait
          break;
          
       case CMS13x19:          
          if ( o )  
             currentPos.setHeight((int)(w * 1.4615384615385));   // Landscape
          else                       
             currentPos.setHeight((int)(w * 0.68421052631579));  // Portrait
          break;
          
       case CMS15x21:          
          if ( o )  
             currentPos.setHeight((int)(w * 1.4));               // Landscape
          else                       
             currentPos.setHeight((int)(w * 0.71428571428571));  // Portrait
          break;          
       
       case CMS18x25:          
          if ( o )  
             currentPos.setHeight((int)(w * 1.3888888888889));   // Landscape
          else                       
             currentPos.setHeight((int)(w * 0.72));              // Portrait
          break;    

       case CMS20x27:          
          if ( o )  
             currentPos.setHeight((int)(w * 1.35));              // Landscape
          else                       
             currentPos.setHeight((int)(w * 0.74074074074074));  // Portrait
          break;                            
       
       case CMS21x30:          
          if ( o )  
             currentPos.setHeight((int)(w * 1.4285714285714));   // Landscape
          else                       
             currentPos.setHeight((int)(w * 0.7));               // Portrait
          break;                            

       case CMS25x38:          
          if ( o )  
             currentPos.setHeight((int)(w * 1.52));              // Landscape
          else                       
             currentPos.setHeight((int)(w * 0.65789473684211));  // Portrait
          break;                  

       case CMS40x50:          
          if ( o )  
             currentPos.setHeight((int)(w * 1.25));              // Landscape
          else                       
             currentPos.setHeight((int)(w * 0.8));               // Portrait
          break;                                         
       }
    
    m_imageSelectionWidget->setRegionSelection(currentPos);
}

void ImageEffect_RatioCrop::slotHeightChanged(int h)
{
    QRect currentPos = m_imageSelectionWidget->getRegionSelection();
    int r = m_ratioCB->currentItem();
    int o = m_orientCB->currentItem();

    currentPos.setHeight(h);
    
    switch(r)
       {
       case CMS06X08:      
       case CMS15x20:   
       case CMS18x24: 
       case CMS30x40:
          if ( o )  
             currentPos.setWidth((int)(h * 0.75));             // Landscape
          else                       
             currentPos.setWidth((int)(h * 1.3333333333333));  // Portrait
          break;
       
       case CMS09X13:         
          if ( o )  
             currentPos.setWidth((int)(h * 0.69230769230769)); // Landscape
          else                       
             currentPos.setWidth((int)(h * 1.4444444444444));  // Portrait
          break;
          
       case CMS10x15:    
       case CMS20x30:                     
       case CMS30x45:          
          if ( o )  
             currentPos.setWidth((int)(h * 0.66666666666667)); // Landscape
          else                       
             currentPos.setWidth((int)(h * 1.5));              // Portrait
          break;
       
       case CMS11x15:       
          if ( o )  
             currentPos.setWidth((int)(h * 0.73333333333333)); // Landscape
          else                       
             currentPos.setWidth((int)(h * 1.3636363636364));  // Portrait
          break;
                    
       case CMS13x18:          
          if ( o )  
             currentPos.setWidth((int)(h * 0.72222222222222)); // Landscape
          else                       
             currentPos.setWidth((int)(h * 1.3846153846154));  // Portrait
          break;
          
       case CMS13x19:          
          if ( o )  
             currentPos.setWidth((int)(h * 0.68421052631579)); // Landscape
          else                       
             currentPos.setWidth((int)(h * 1.4615384615385));  // Portrait
          break;
          
       case CMS15x21:          
          if ( o )  
             currentPos.setWidth((int)(h * 0.71428571428571)); // Landscape
          else                       
             currentPos.setWidth((int)(h * 1.4));              // Portrait
          break; 

       case CMS18x25:          
          if ( o )  
             currentPos.setWidth((int)(h * 0.72));             // Landscape
          else                       
             currentPos.setWidth((int)(h * 1.3888888888889));  // Portrait
          break;          

       case CMS20x27:          
          if ( o )  
             currentPos.setWidth((int)(h * 0.74074074074074)); // Landscape
          else                       
             currentPos.setWidth((int)(h * 1.35));             // Portrait
          break;                                                 
       
       case CMS21x30:          
          if ( o )  
             currentPos.setWidth((int)(h * 0.7));              // Landscape
          else                       
             currentPos.setWidth((int)(h * 1.4285714285714));  // Portrait
          break;           
       
       case CMS25x38:          
          if ( o )  
             currentPos.setWidth((int)(h * 0.65789473684211)); // Landscape
          else                       
             currentPos.setWidth((int)(h * 1.52));             // Portrait
          break;        
       
       case CMS40x50:          
          if ( o )  
             currentPos.setWidth((int)(h * 0.8));              // Landscape
          else                       
             currentPos.setWidth((int)(h * 1.25));             // Portrait
          break;        
          }
    
    m_imageSelectionWidget->setRegionSelection(currentPos);
}

void ImageEffect_RatioCrop::slotOrientChanged(int o)
{
    if ( o )
       slotWidthChanged(m_widthInput->value());    // Landscape.
    else
       slotHeightChanged(m_heightInput->value());  // Portrait.    
}

void ImageEffect_RatioCrop::slotRatioChanged(void)
{
    if ( m_orientCB->currentItem() )
       slotWidthChanged(m_widthInput->value());    // Landscape.
    else
       slotHeightChanged(m_heightInput->value());  // Portrait.    
}

void ImageEffect_RatioCrop::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
    
    uint* data       = iface.getOriginalData();
    int w            = iface.originalWidth();
    int h            = iface.originalHeight();
    QRect currentPos = m_imageSelectionWidget->getRegionSelection();

    Imlib_Context context = imlib_context_new();
    imlib_context_push(context);

    Imlib_Image imOrg = imlib_create_image_using_copied_data(w, h, data);
    imlib_context_set_image(imOrg);
    
    Imlib_Image imDest = imlib_create_cropped_image(currentPos.left(), currentPos.top(),
                                                    currentPos.width(), currentPos.height());
    imlib_context_set_image(imDest);    
    
    uint* ptr  = imlib_image_get_data_for_reading_only();
    int   newW = imlib_image_get_width();
    int   newH = imlib_image_get_height();

    iface.putOriginalData(ptr, newW, newH);   
    
    imlib_context_set_image(imOrg);
    imlib_free_image_and_decache();

    imlib_context_set_image(imDest);
    imlib_free_image_and_decache();
    
    imlib_context_pop();
    imlib_context_free(context);       
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();
}

#include "imageeffect_ratiocrop.moc"
