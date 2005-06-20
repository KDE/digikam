/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-06
 * Description : Ratio crop tool for digiKam image editor
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
#include <qrect.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qcombobox.h>
#include <qimage.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qtabwidget.h>
#include <qcheckbox.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <knuminput.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kcolorbutton.h>

// Digikam includes.

#include <imageiface.h>
#include <imageselectionwidget.h>

// Local includes.

#include "imageeffect_ratiocrop.h"

ImageEffect_RatioCrop::ImageEffect_RatioCrop(QWidget* parent)
                     : KDialogBase(Plain, i18n("Aspect Ratio Crop"),
                                   Help|User1|User2|Ok|Cancel, Ok,
                                   parent, 0, true, true, 
                                   i18n("&Reset Values"), i18n("&Max. Aspect")),
                       m_parent(parent)
{
    setHelp("ratiocroptool.anchor", "digikam");
    setButtonWhatsThis ( User1, i18n("<p>Reset selection area to the default values.") );     
    setButtonWhatsThis ( User2, i18n("<p>Set selection area to the maximum size according "
                                     "to the current ratio.") );     
    
    // -------------------------------------------------------------
        
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QVGroupBox *gbox = new QVGroupBox(i18n("Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_imageSelectionWidget = new Digikam::ImageSelectionWidget(480, 320, frame);
    QWhatsThis::add( m_imageSelectionWidget, i18n("<p>You can see here the aspect ratio selection preview "
                                                  "used for cropping. You can use the mouse for moving and "
                                                  " resizing the crop area."));
    l->addWidget(m_imageSelectionWidget, 0, Qt::AlignCenter);
    topLayout->addWidget(gbox);
 
    // -------------------------------------------------------------
    
    m_mainTab = new QTabWidget( plainPage() );
    
    QWidget* firstPage = new QWidget( m_mainTab );
    QGridLayout* grid = new QGridLayout( firstPage, 3, 8, marginHint(), spacingHint());
    m_mainTab->addTab( firstPage, i18n("Crop Selection") );
    
    QLabel *label = new QLabel(i18n("Aspect ratio:"), firstPage);
    m_ratioCB = new QComboBox( false, firstPage );
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
    
    QLabel *label2 = new QLabel(i18n("Orientation:"), firstPage);
    m_orientCB = new QComboBox( false, firstPage );
    m_orientCB->insertItem( i18n("Landscape") );
    m_orientCB->insertItem( i18n("Portrait") );
    QWhatsThis::add( m_orientCB, i18n("<p>Select here constrained aspect ratio orientation."));
    
    grid->addMultiCellWidget(label, 0, 0, 0, 0);
    grid->addMultiCellWidget(m_ratioCB, 0, 0, 1, 3);
    grid->addMultiCellWidget(label2, 0, 0, 6, 6);
    grid->addMultiCellWidget(m_orientCB, 0, 0, 7, 8);
    
    // -------------------------------------------------------------
    
    m_customLabel1 = new QLabel(i18n("Custom ratio:"), firstPage);
    m_customLabel1->setAlignment(AlignLeft|AlignVCenter);
    m_customRatioNInput = new KIntSpinBox(1, 10000, 1, 1, 10, firstPage);
    QWhatsThis::add( m_customRatioNInput, i18n("<p>Set here the desired custom aspect numerator value."));
    m_customLabel2 = new QLabel(" : ", firstPage);
    m_customLabel2->setAlignment(AlignCenter|AlignVCenter);
    m_customRatioDInput = new KIntSpinBox(1, 10000, 1, 1, 10, firstPage);
    QWhatsThis::add( m_customRatioDInput, i18n("<p>Set here the desired custom aspect denominator value."));

    grid->addMultiCellWidget(m_customLabel1, 1, 1, 0, 0);
    grid->addMultiCellWidget(m_customRatioNInput, 1, 1, 1, 1);
    grid->addMultiCellWidget(m_customLabel2, 1, 1, 2, 2);
    grid->addMultiCellWidget(m_customRatioDInput, 1, 1, 3, 3);
    
    // -------------------------------------------------------------
    
    m_xInput = new KIntNumInput(firstPage);
    QWhatsThis::add( m_xInput, i18n("<p>Set here the top left selection corner position for cropping."));
    m_xInput->setLabel(i18n("X:"), AlignLeft|AlignVCenter);
    m_xInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth(), 1, true);
    m_widthInput = new KIntNumInput(firstPage);
    m_widthInput->setLabel(i18n("Width:"), AlignLeft|AlignVCenter);
    QWhatsThis::add( m_widthInput, i18n("<p>Set here the width selection for cropping."));
    m_widthInput->setRange(10, m_imageSelectionWidget->getOriginalImageWidth(), 1, true);
    m_centerWidth = new QPushButton(firstPage);
    KGlobal::dirs()->addResourceType("centerwidth", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("centerwidth", "centerwidth.png");
    m_centerWidth->setPixmap( QPixmap( directory + "centerwidth.png" ) );
    QWhatsThis::add( m_centerWidth, i18n("<p>Set width position to center."));
    
    grid->addMultiCellWidget(m_xInput, 2, 2, 0, 2);
    grid->addMultiCellWidget(m_widthInput, 2, 2, 5, 7);
    grid->addMultiCellWidget(m_centerWidth, 2, 2, 8, 8);
    
    // -------------------------------------------------------------
    
    m_yInput = new KIntNumInput(firstPage);
    m_yInput->setLabel(i18n("Y:"), AlignLeft|AlignVCenter);
    QWhatsThis::add( m_yInput, i18n("<p>Set here the top left selection corner position for cropping."));
    m_yInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth(), 1, true);
    m_heightInput = new KIntNumInput(firstPage);
    m_heightInput->setLabel(i18n("Height:"), AlignLeft|AlignVCenter);
    QWhatsThis::add( m_heightInput, i18n("<p>Set here the height selection for cropping."));
    m_heightInput->setRange(10, m_imageSelectionWidget->getOriginalImageHeight(), 1, true);
    m_centerHeight = new QPushButton(firstPage);
    KGlobal::dirs()->addResourceType("centerheight", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("centerheight", "centerheight.png");
    m_centerHeight->setPixmap( QPixmap( directory + "centerheight.png" ) );
    QWhatsThis::add( m_centerHeight, i18n("<p>Set height position to center."));
    
    grid->addMultiCellWidget(m_yInput, 3, 3, 0, 2);
    grid->addMultiCellWidget(m_heightInput, 3, 3, 5, 7);
    grid->addMultiCellWidget(m_centerHeight, 3, 3, 8, 8);
    
    // -------------------------------------------------------------
    
    QWidget* secondPage = new QWidget( m_mainTab );
    QGridLayout* grid2 = new QGridLayout( secondPage, 4, 4, marginHint(), spacingHint());
    m_mainTab->addTab( secondPage, i18n("Composition Guide") );
    
    QLabel *labelGuideLines = new QLabel(i18n("Guide Type:"), secondPage);
    m_guideLinesCB = new QComboBox( false, secondPage );
    m_guideLinesCB->insertItem( i18n("Rules Of Thirds") );
    m_guideLinesCB->insertItem( i18n("Harmonious Triangles") );
    m_guideLinesCB->insertItem( i18n("Golden Mean") );
    m_guideLinesCB->insertItem( i18n("None") );
    m_guideLinesCB->setCurrentText( i18n("None") );
    QWhatsThis::add( m_guideLinesCB, i18n("<p>With this option, you can display guide lines "
                                          "which help you to compose your photograph."));    
    
    m_goldenSectionBox = new QCheckBox(i18n("Golden Sections"), secondPage);
    QWhatsThis::add( m_goldenSectionBox, i18n("<p>Enable this option to show golden sections."));
    
    m_goldenSpiralSectionBox = new QCheckBox(i18n("Golden Spiral Sections"), secondPage);
    QWhatsThis::add( m_goldenSpiralSectionBox, i18n("<p>Enable this option to show golden spiral sections."));
    
    m_goldenSpiralBox = new QCheckBox(i18n("Golden Spiral"), secondPage);
    QWhatsThis::add( m_goldenSpiralBox, i18n("<p>Enable this option to show golden spiral guide."));
                     
    m_goldenTriangleBox = new QCheckBox(i18n("Golden Triangles"), secondPage);
    QWhatsThis::add( m_goldenTriangleBox, i18n("<p>Enable this option to show golden triangles."));

    m_flipHorBox = new QCheckBox(i18n("Flip Horizontally"), secondPage);
    QWhatsThis::add( m_flipHorBox, i18n("<p>Enable this option to flip horizontally golden guides."));

    m_flipVerBox = new QCheckBox(i18n("Flip Vertically"), secondPage);
    QWhatsThis::add( m_flipVerBox, i18n("<p>Enable this option to flip vertically golden guides."));

    QLabel *labelColorGuide = new QLabel(i18n("Color:"), secondPage);
    m_guideColorBt = new KColorButton( QColor( 250, 250, 255 ), secondPage );
    QWhatsThis::add( m_guideColorBt, i18n("<p>Set here the color used to draw composition guides."));
    
    grid2->addMultiCellWidget(labelGuideLines, 0, 0, 0, 0);
    grid2->addMultiCellWidget(m_guideLinesCB, 0, 0, 1, 1);
    grid2->addMultiCellWidget(m_goldenSectionBox, 1, 1, 0, 1);
    grid2->addMultiCellWidget(m_goldenSpiralSectionBox, 2, 2, 0, 1);
    grid2->addMultiCellWidget(m_goldenSpiralBox, 3, 3, 0, 1);
    grid2->addMultiCellWidget(m_goldenTriangleBox, 1, 1, 2, 2);
    grid2->addMultiCellWidget(m_flipHorBox, 2, 2, 2, 2);
    grid2->addMultiCellWidget(m_flipVerBox, 3, 3, 2, 2);
    grid2->addMultiCellWidget(labelColorGuide, 1, 1, 4, 4);
    grid2->addMultiCellWidget(m_guideColorBt, 1, 1, 5, 5);
    
    topLayout->addWidget(m_mainTab);
    
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

    connect(m_guideLinesCB, SIGNAL(activated(int)),
            this, SLOT(slotGuideTypeChanged(int)));            
        
    connect(m_goldenSectionBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));   
    
    connect(m_goldenSpiralSectionBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));   

    connect(m_goldenSpiralBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));   
                                    
    connect(m_goldenTriangleBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));   
  
    connect(m_flipHorBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));   
    
    connect(m_flipVerBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));   
    
    connect(m_guideColorBt, SIGNAL(changed(const QColor &)),
            m_imageSelectionWidget, SLOT(slotChangeGuideColor(const QColor &)));         
            
    connect(m_widthInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotWidthChanged(int)));

    connect(m_heightInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotHeightChanged(int)));
    
    connect(m_imageSelectionWidget, SIGNAL(signalSelectionWidthChanged(int)),
            this, SLOT(slotSelectionWidthChanged(int)));                                            

    connect(m_imageSelectionWidget, SIGNAL(signalSelectionHeightChanged(int)),
            this, SLOT(slotSelectionHeightChanged(int)));                                            
                        
    connect(m_imageSelectionWidget, SIGNAL(signalSelectionChanged(QRect)),
            this, SLOT(slotSelectionChanged(QRect)));       
                
    connect(m_imageSelectionWidget, SIGNAL(signalSelectionMoved(QRect)),
            this, SLOT(slotSelectionChanged(QRect)));      

    connect(m_centerWidth, SIGNAL(clicked()),
            this, SLOT(slotCenterWidth()));

    connect(m_centerHeight, SIGNAL(clicked()),
            this, SLOT(slotCenterHeight()));
    
    // -------------------------------------------------------------

    adjustSize();
    disableResize(); 
    readSettings(); 
}

ImageEffect_RatioCrop::~ImageEffect_RatioCrop()
{
    writeSettings();
}

void ImageEffect_RatioCrop::readSettings(void)
{
    QColor *defaultGuideColor = new QColor( 250, 250, 255 );
    KConfig *config = kapp->config();
    config->setGroup("Aspect Ratio Crop Tool Settings");
    
    // No guide lines per default.
    m_guideLinesCB->setCurrentItem( config->readNumEntry("Guide Lines Type", 
                                    Digikam::ImageSelectionWidget::GuideNone) );   
    m_goldenSectionBox->setChecked( config->readBoolEntry("Golden Section", true) );
    m_goldenSpiralSectionBox->setChecked( config->readBoolEntry("Golden Spiral Section", false) );
    m_goldenSpiralBox->setChecked( config->readBoolEntry("Golden Spiral", false) );
    m_goldenTriangleBox->setChecked( config->readBoolEntry("Golden Triangle", false) );
    m_flipHorBox->setChecked( config->readBoolEntry("Golden Flip Horizontal", false) );
    m_flipVerBox->setChecked( config->readBoolEntry("Golden Flip Vertical", false) );
    m_guideColorBt->setColor(config->readColorEntry("Guide Color", defaultGuideColor));
    m_imageSelectionWidget->slotGuideLines(m_guideLinesCB->currentItem());            
                                    
    m_xInput->setValue( config->readNumEntry("Custom Aspect Ratio Xpos", 50) );
    m_yInput->setValue( config->readNumEntry("Custom Aspect Ratio Ypos", 50) );
    
    // 3:4 per default.
    m_ratioCB->setCurrentItem( config->readNumEntry("Aspect Ratio", 3) );                 
    m_customRatioNInput->setValue( config->readNumEntry("Custom Aspect Ratio Num", 1) );
    m_customRatioDInput->setValue( config->readNumEntry("Custom Aspect Ratio Den", 1) );
    
    applyRatioChanges(m_ratioCB->currentItem());

    // Paysage per default.
    m_orientCB->setCurrentItem( config->readNumEntry("Aspect Ratio Orientation", 0) );    
    
    if ( m_ratioCB->currentItem() == Digikam::ImageSelectionWidget::RATIONONE )
       {
       m_widthInput->setValue( config->readNumEntry("Custom Aspect Ratio Width", 800) );
       m_heightInput->setValue( config->readNumEntry("Custom Aspect Ratio Height", 600) );
       }
    else       
       {
       m_widthInput->setValue( 1 ); // It will be recalculed automaticly with the ratio.
       m_heightInput->setValue( config->readNumEntry("Custom Aspect Ratio Height", 600) );
       }
    
    m_imageSelectionWidget->setSelectionOrientation(m_orientCB->currentItem());    
    delete defaultGuideColor;   
}
    
void ImageEffect_RatioCrop::writeSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("Aspect Ratio Crop Tool Settings");
    config->writeEntry( "Aspect Ratio", m_ratioCB->currentItem() );
    config->writeEntry( "Aspect Ratio Orientation", m_orientCB->currentItem() );
    config->writeEntry( "Custom Aspect Ratio Num", m_customRatioNInput->value() );
    config->writeEntry( "Custom Aspect Ratio Den", m_customRatioDInput->value() );
    
    config->writeEntry( "Custom Aspect Ratio Xpos", m_xInput->value() );
    config->writeEntry( "Custom Aspect Ratio Ypos", m_yInput->value() );
    config->writeEntry( "Custom Aspect Ratio Width", m_widthInput->value() );
    config->writeEntry( "Custom Aspect Ratio Height", m_heightInput->value() );
    
    config->writeEntry( "Guide Lines Type", m_guideLinesCB->currentItem() );
    config->writeEntry( "Golden Section", m_goldenSectionBox->isChecked() );
    config->writeEntry( "Golden Spiral Section", m_goldenSpiralSectionBox->isChecked() );
    config->writeEntry( "Golden Spiral", m_goldenSpiralBox->isChecked() );
    config->writeEntry( "Golden Triangle", m_goldenTriangleBox->isChecked() );
    config->writeEntry( "Golden Flip Horizontal", m_flipHorBox->isChecked() );
    config->writeEntry( "Golden Flip Vertical", m_flipVerBox->isChecked() );
    config->writeEntry( "Guide Color", m_guideColorBt->color() );
    config->sync();
}

void ImageEffect_RatioCrop::slotUser1()
{
    m_imageSelectionWidget->resetSelection();
} 

void ImageEffect_RatioCrop::slotUser2()
{
    m_imageSelectionWidget->maxAspectSelection();
} 

void ImageEffect_RatioCrop::slotCenterWidth()
{
    m_imageSelectionWidget->setCenterSelection(Digikam::ImageSelectionWidget::CenterWidth);
} 

void ImageEffect_RatioCrop::slotCenterHeight()
{
    m_imageSelectionWidget->setCenterSelection(Digikam::ImageSelectionWidget::CenterHeight);
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
    
    m_xInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth() - rect.width(), 1, true);
    m_yInput->setRange(0, m_imageSelectionWidget->getOriginalImageHeight() - rect.height(), 1, true);
    m_widthInput->setRange(m_imageSelectionWidget->getMinWidthRange(),
                           m_imageSelectionWidget->getOriginalImageWidth() - rect.x() , 1, true);
    m_heightInput->setRange(m_imageSelectionWidget->getMinHeightRange(), 
                            m_imageSelectionWidget->getOriginalImageHeight() - rect.y(), 1, true);
    
    m_xInput->blockSignals(false);
    m_yInput->blockSignals(false);
    m_widthInput->blockSignals(false);
    m_heightInput->blockSignals(false);
}

void ImageEffect_RatioCrop::slotSelectionWidthChanged(int newWidth)
{
    m_widthInput->blockSignals(true);
    m_widthInput->setValue(newWidth);
    m_widthInput->setRange(m_imageSelectionWidget->getMinWidthRange(),
                           m_imageSelectionWidget->getOriginalImageWidth() -
                           m_imageSelectionWidget->getRegionSelection().x(), 1, true);
    m_widthInput->blockSignals(false);
}

void ImageEffect_RatioCrop::slotSelectionHeightChanged(int newHeight)
{
    m_heightInput->blockSignals(true);
    m_heightInput->setValue(newHeight);
    m_heightInput->setRange(m_imageSelectionWidget->getMinHeightRange(),
                            m_imageSelectionWidget->getOriginalImageHeight() - 
                            m_imageSelectionWidget->getRegionSelection().y(), 1, true);
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
    
    // Reset selection area.
    slotUser1(); 
}

void ImageEffect_RatioCrop::slotRatioChanged(int a)
{
    applyRatioChanges(a);
       
    // Reset selection area.
    slotUser1(); 
}

void ImageEffect_RatioCrop::applyRatioChanges(int a)
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

void ImageEffect_RatioCrop::slotGuideTypeChanged(int t)
{
    if ( t == Digikam::ImageSelectionWidget::RulesOfThirds || 
         t == Digikam::ImageSelectionWidget::GuideNone ) 
       {
       m_goldenSectionBox->setEnabled(false);
       m_goldenSpiralSectionBox->setEnabled(false);
       m_goldenSpiralBox->setEnabled(false);
       m_goldenTriangleBox->setEnabled(false);
       m_flipHorBox->setEnabled(false);
       m_flipVerBox->setEnabled(false);
       }
    else if ( t == Digikam::ImageSelectionWidget::HarmoniousTriangles ) 
       {
       m_goldenSectionBox->setEnabled(false);
       m_goldenSpiralSectionBox->setEnabled(false);
       m_goldenSpiralBox->setEnabled(false);
       m_goldenTriangleBox->setEnabled(false);
       m_flipHorBox->setEnabled(true);
       m_flipVerBox->setEnabled(true);
       }
    else        
       {
       m_goldenSectionBox->setEnabled(true);
       m_goldenSpiralSectionBox->setEnabled(true);
       m_goldenSpiralBox->setEnabled(true);
       m_goldenTriangleBox->setEnabled(true);
       m_flipHorBox->setEnabled(true);
       m_flipVerBox->setEnabled(true);
       }
    
    m_imageSelectionWidget->setGoldenGuideTypes(m_goldenSectionBox->isChecked(),
                                                m_goldenSpiralSectionBox->isChecked(), 
                                                m_goldenSpiralBox->isChecked(), 
                                                m_goldenTriangleBox->isChecked(), 
                                                m_flipHorBox->isChecked(), 
                                                m_flipVerBox->isChecked());
    m_imageSelectionWidget->slotGuideLines(t);                   
}

void ImageEffect_RatioCrop::slotGoldenGuideTypeChanged(void)
{
    slotGuideTypeChanged(m_guideLinesCB->currentItem());
}
    
void ImageEffect_RatioCrop::slotCustomRatioChanged(void)
{
    m_imageSelectionWidget->setSelectionAspectRatioValue(
            (float)(m_customRatioNInput->value()) / (float)(m_customRatioDInput->value()) );
    
    // Reset selection area.
    slotUser1(); 
}

void ImageEffect_RatioCrop::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
    
    uint* data       = iface.getOriginalData();
    int w            = iface.originalWidth();
    int h            = iface.originalHeight();
    QRect currentPos = m_imageSelectionWidget->getRegionSelection();

    QImage* imOrg;
    QImage  imDest;
    imOrg = new QImage((uchar*)data, w, h, 32, 0, 0, QImage::IgnoreEndian);
    imDest = imOrg->copy(currentPos);
    delete imOrg;

    iface.putOriginalData(i18n("Aspect Ratio Crop"), (uint*)imDest.bits(), imDest.width(), imDest.height());
    
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();
}

#include "imageeffect_ratiocrop.moc"
