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
#include <qcombobox.h>
#include <qcheckbox.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <knuminput.h>
#include <kapplication.h>
#include <kconfig.h>

// Digikam includes.

#include <imageiface.h>
#include <imageselectionwidget.h>

// Local includes.

#include "imageeffect_ratiocrop.h"

ImageEffect_RatioCrop::ImageEffect_RatioCrop(QWidget* parent)
                     : KDialogBase(Plain, i18n("Aspect Ratio Crop"),
                                   Help|User1|Ok|Cancel, Ok,
                                   parent, 0, true, true, 
                                   i18n("&Reset Values")),
                       m_parent(parent)
{
    setHelp("ratiocroptool.anchor", "digikam");
    setButtonWhatsThis ( User1, i18n("<p>Reset all parameters to the default values.") );     
    
    // -------------------------------------------------------------
        
    QGridLayout* topLayout = new QGridLayout( plainPage(), 4, 5 , marginHint(), spacingHint());

    QVGroupBox *gbox = new QVGroupBox(i18n("Aspect Ratio Crop Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_imageSelectionWidget = new Digikam::ImageSelectionWidget(480, 320, frame);
    QWhatsThis::add( m_imageSelectionWidget, i18n("<p>You can see here the aspect ratio selection preview used for "
                                                  "cropping. You can use the mouse for moving and resizing the crop "
                                                  "area."));
    l->addWidget(m_imageSelectionWidget, 0, Qt::AlignCenter);
    topLayout->addMultiCellWidget(gbox, 0, 0, 0, 4);
 
    // -------------------------------------------------------------
    
    QLabel *label = new QLabel(i18n("Aspect Ratio:"), plainPage());
    m_ratioCB = new QComboBox( false, plainPage() );
    m_ratioCB->insertItem( i18n("Custom") );
    m_ratioCB->insertItem( "1:1" );
    m_ratioCB->insertItem( "2:3" );
    m_ratioCB->insertItem( "3:4" );
    m_ratioCB->insertItem( "4:5" );
    m_ratioCB->insertItem( "5:7" );
    m_ratioCB->insertItem( "7:10" );
    m_ratioCB->insertItem( i18n("None") );
    m_ratioCB->setCurrentText( "1:1" );
    QWhatsThis::add( m_ratioCB, i18n("<p>Select here your constrained aspect ratio for cropping."));
    
    QLabel *label2 = new QLabel(i18n("Orientation:"), plainPage());
    m_orientCB = new QComboBox( false, plainPage() );
    m_orientCB->insertItem( i18n("Landscape") );
    m_orientCB->insertItem( i18n("Portrait") );
    QWhatsThis::add( m_orientCB, i18n("<p>Select here constrained aspect ratio orientation."));
    
    topLayout->addMultiCellWidget(label, 1, 1, 0, 0);
    topLayout->addMultiCellWidget(m_ratioCB, 1, 1, 1, 1);
    topLayout->addMultiCellWidget(label2, 1, 1, 3, 3);
    topLayout->addMultiCellWidget(m_orientCB, 1, 1, 4, 4);

    QHBoxLayout* l2 = new QHBoxLayout((QWidget*)0, 1, 0);
    m_customLabel1 = new QLabel(i18n("Custom Ratio: "), plainPage());
    m_customLabel1->setAlignment(AlignLeft|AlignVCenter);
    m_customRatioNInput = new KIntSpinBox(1, 100, 1, 1, 10, plainPage());
    QWhatsThis::add( m_customRatioNInput, i18n("<p>Set here the desired custom aspect numerator value."));
    m_customLabel2 = new QLabel(" : ", plainPage());
    m_customLabel2->setAlignment(AlignCenter|AlignVCenter);
    m_customRatioDInput = new KIntSpinBox(1, 100, 1, 1, 10, plainPage());
    QWhatsThis::add( m_customRatioDInput, i18n("<p>Set here the desired custom aspect denominator value."));

    m_useRuleThirdLines = new QCheckBox( i18n("Show Rule Third Lines"), plainPage());
    QWhatsThis::add( m_useRuleThirdLines, i18n("<p>With this option, you can display the rule third lines "
                                               "who help you to compose your photograph."));
    
    l2->addWidget( m_customLabel1 );
    l2->addWidget( m_customRatioNInput );
    l2->addWidget( m_customLabel2 );
    l2->addWidget( m_customRatioDInput );
    l2->addStretch();
    l2->addWidget( m_useRuleThirdLines );
    topLayout->addMultiCellLayout(l2, 2, 2, 0, 4);

    m_xInput = new KIntNumInput(plainPage());
    QWhatsThis::add( m_xInput, i18n("<p>Set here the top left selection corner position for cropping."));
    m_xInput->setLabel(i18n("X:"), AlignLeft|AlignVCenter);
    m_xInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth(), 1, true);
    m_widthInput = new KIntNumInput(plainPage());
    m_widthInput->setLabel(i18n("Width:"), AlignLeft|AlignVCenter);
    QWhatsThis::add( m_widthInput, i18n("<p>Set here the width selection for cropping."));
    m_widthInput->setRange(10, m_imageSelectionWidget->getOriginalImageWidth(), 1, true);
    topLayout->addMultiCellWidget(m_xInput, 3, 3, 0, 1);
    topLayout->addMultiCellWidget(m_widthInput, 3, 3, 3, 4);
    
    m_yInput = new KIntNumInput(plainPage());
    m_yInput->setLabel(i18n("Y:"), AlignLeft|AlignVCenter);
    QWhatsThis::add( m_yInput, i18n("<p>Set here the top left selection corner position for cropping."));
    m_yInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth(), 1, true);
    m_heightInput = new KIntNumInput(plainPage());
    m_heightInput->setLabel(i18n("Height:"), AlignLeft|AlignVCenter);
    QWhatsThis::add( m_heightInput, i18n("<p>Set here the height selection for cropping."));
    m_heightInput->setRange(10, m_imageSelectionWidget->getOriginalImageHeight(), 1, true);
    topLayout->addMultiCellWidget(m_yInput, 4, 4, 0, 1);
    topLayout->addMultiCellWidget(m_heightInput, 4, 4, 3, 4);
    
    // -------------------------------------------------------------
    
    connect(m_ratioCB, SIGNAL(activated(int)),
            this, SLOT(slotRatioChanged(int)));
    
    connect(m_orientCB, SIGNAL(activated(int)),
            this, SLOT(slotOrientChanged(int)));

    connect(m_xInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotXChanged(int)));

    connect(m_yInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotYChanged(int)));                                    

    connect(m_customRatioNInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotCustomRatioChanged()));

    connect(m_customRatioDInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotCustomRatioChanged()));

    connect(m_useRuleThirdLines, SIGNAL(toggled (bool)),
            m_imageSelectionWidget, SLOT(slotRuleThirdLines(bool)));            
                                                                                
    connect(m_widthInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotWidthChanged(int)));

    connect(m_heightInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotHeightChanged(int)));
    
    connect(m_imageSelectionWidget, SIGNAL(signalSelectionChanged(QRect)),
            this, SLOT(slotSelectionChanged(QRect)));                                            

    connect(m_imageSelectionWidget, SIGNAL(signalSelectionWidthChanged(int)),
            this, SLOT(slotSelectionWidthChanged(int)));                                            

    connect(m_imageSelectionWidget, SIGNAL(signalSelectionHeightChanged(int)),
            this, SLOT(slotSelectionHeightChanged(int)));                                            
                        
    connect(m_imageSelectionWidget, SIGNAL(signalSelectionMoved(QRect, bool)),
            this, SLOT(slotSelectionChanged(QRect)));      

    // -------------------------------------------------------------
                
    readSettings();             
    adjustSize();
    disableResize();     
}

ImageEffect_RatioCrop::~ImageEffect_RatioCrop()
{
    writeSettings();
}

void ImageEffect_RatioCrop::readSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("ImageViewer Settings");
    
    m_xInput->setValue( config->readNumEntry("Custom Aspect Ratio Xpos", 50) );
    m_yInput->setValue( config->readNumEntry("Custom Aspect Ratio Ypos", 50) );
    
    m_ratioCB->setCurrentItem( config->readNumEntry("Aspect Ratio", 3) );                 // 3:4 per default.
    m_customRatioNInput->setValue( config->readNumEntry("Custom Aspect Ratio Num", 1) );
    m_customRatioDInput->setValue( config->readNumEntry("Custom Aspect Ratio Den", 1) );
    slotRatioChanged(m_ratioCB->currentItem());

    m_orientCB->setCurrentItem( config->readNumEntry("Aspect Ratio Orientation", 0) );    // Paysage per default.
    
    if ( m_orientCB->currentItem() == Digikam::ImageSelectionWidget::Paysage )
       {
       m_widthInput->setValue( config->readNumEntry("Custom Aspect Ratio Width", 800) );
       m_heightInput->setValue( 1 );
       }
    else       
       {
       m_widthInput->setValue( 1 );
       m_heightInput->setValue( config->readNumEntry("Custom Aspect Ratio Height", 600) );
       }
    
    slotOrientChanged(m_orientCB->currentItem());
    
    m_useRuleThirdLines->setChecked( config->readBoolEntry("Use Rule Third Lines", false) );
}
    
void ImageEffect_RatioCrop::writeSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("ImageViewer Settings");
    config->writeEntry( "Aspect Ratio", m_ratioCB->currentItem() );
    config->writeEntry( "Aspect Ratio Orientation", m_orientCB->currentItem() );
    config->writeEntry( "Custom Aspect Ratio Num", m_customRatioNInput->value() );
    config->writeEntry( "Custom Aspect Ratio Den", m_customRatioDInput->value() );
    
    config->writeEntry( "Custom Aspect Ratio Xpos", m_xInput->value() );
    config->writeEntry( "Custom Aspect Ratio Ypos", m_yInput->value() );
    config->writeEntry( "Custom Aspect Ratio Width", m_widthInput->value() );
    config->writeEntry( "Custom Aspect Ratio Height", m_heightInput->value() );
    
    config->writeEntry( "Use Rule Third Lines", m_useRuleThirdLines->isChecked() );
    config->sync();
}

void ImageEffect_RatioCrop::slotUser1()
{
    m_imageSelectionWidget->resetSelection();
} 

void ImageEffect_RatioCrop::slotSelectionChanged(QRect rect)
{
    m_xInput->blockSignals(true);
    m_yInput->blockSignals(true);
    m_widthInput->blockSignals(true);
    m_heightInput->blockSignals(true);
    
    m_xInput->setValue(rect.x());
    m_yInput->setValue(rect.y());
    m_widthInput->setValue(rect.width());
    m_heightInput->setValue(rect.height());
    
    m_xInput->blockSignals(false);
    m_yInput->blockSignals(false);
    m_widthInput->blockSignals(false);
    m_heightInput->blockSignals(false);
}

void ImageEffect_RatioCrop::slotSelectionWidthChanged(int newWidth)
{
    m_widthInput->blockSignals(true);
    m_widthInput->setValue(newWidth);
    m_widthInput->blockSignals(false);
}

void ImageEffect_RatioCrop::slotSelectionHeightChanged(int newHeight)
{
    m_heightInput->blockSignals(true);
    m_heightInput->setValue(newHeight);
    m_heightInput->blockSignals(false);
}

void ImageEffect_RatioCrop::slotXChanged(int x)
{
    m_imageSelectionWidget->setSelectionX(x);
}

void ImageEffect_RatioCrop::slotYChanged(int y)
{
    m_imageSelectionWidget->setSelectionY(y);
}

void ImageEffect_RatioCrop::slotWidthChanged(int w)
{
    m_imageSelectionWidget->setSelectionWidth(w);
}

void ImageEffect_RatioCrop::slotHeightChanged(int h)
{
    m_imageSelectionWidget->setSelectionHeight(h);
}

void ImageEffect_RatioCrop::slotOrientChanged(int o)
{
    m_imageSelectionWidget->setSelectionOrientation(o);
}

void ImageEffect_RatioCrop::slotRatioChanged(int a)
{
    m_imageSelectionWidget->setSelectionAspectRatioType(a);
    
    if ( a == Digikam::ImageSelectionWidget::RATIOCUSTOM ) 
       {
       m_customLabel1->setEnabled(true);
       m_customLabel2->setEnabled(true);
       m_customRatioNInput->setEnabled(true);
       m_customRatioDInput->setEnabled(true);
       m_orientCB->setEnabled(true);
       slotCustomRatioChanged();
       }
    else if ( a == Digikam::ImageSelectionWidget::RATIONONE )
       {
       m_orientCB->setEnabled(false);
       m_customLabel1->setEnabled(false);
       m_customLabel2->setEnabled(false);
       m_customRatioNInput->setEnabled(false);
       m_customRatioDInput->setEnabled(false);
       }
    else        // Pre-config ratio selected.
       {
       m_orientCB->setEnabled(true);
       m_customLabel1->setEnabled(false);
       m_customLabel2->setEnabled(false);
       m_customRatioNInput->setEnabled(false);
       m_customRatioDInput->setEnabled(false);
       }
}

void ImageEffect_RatioCrop::slotCustomRatioChanged(void)
{
    m_imageSelectionWidget->setSelectionAspectRatioValue(
            (float)(m_customRatioNInput->value()) / (float)(m_customRatioDInput->value()) );
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
