/* ============================================================
 * File  : imageeffect_whitebalance.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-11
 * Description : a digiKam image editor plugin to correct 
 *               image white balance 
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Some parts are inspired from RawPhoto implementation copyrighted 
 * 2004-2005 by Pawel T. Jochym <jochym at ifj edu pl>
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

// Max input sample. 
#define MAXSMPL 256 

// Max output sample.
#define MAXOUT 255
 
// C++ includes.

#include <cmath>
#include <cstdio>
#include <cstdlib>
 
// Qt includes. 
 
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcombobox.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qtooltip.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <knuminput.h>
#include <kdebug.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imageeffect_whitebalance.h"
#include "blackbody.h"

namespace DigikamWhiteBalanceImagesPlugin
{

ImageEffect_WhiteBalance::ImageEffect_WhiteBalance(QWidget* parent, uint *imageData, uint width, uint height)
                        : KDialogBase(Plain, i18n("White Balance"),
                                      Help|User1|Ok|Cancel, Ok,
                                      parent, 0, true, true,
                                      i18n("&Reset Values")),
                          m_parent(parent)
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    QString whatsThis;
    setButtonWhatsThis ( User1, i18n("<p>Reset all parameters to the default values.") );

    m_clipSat = true;
    m_overExp = false;
    m_WBind   = false;
    
    m_mr     = 1.0;
    m_mg     = 1.0;
    m_mb     = 1.0;
    m_BP     = 0;
    m_WP     = MAXSMPL;
    m_rgbMax = 256;
    
    m_destinationPreviewData = 0L;
    m_whiteBalanceCurves = new Digikam::ImageCurves();
    
    for (int i = Digikam::ImageHistogram::RedChannel ; i <= Digikam::ImageHistogram::BlueChannel ; i++)
       m_whiteBalanceCurves->setCurveType(i, Digikam::ImageCurves::CURVE_FREE);
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("White Balance"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to correct white color balance."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    about->addAuthor("Pawel T. Jochym", I18N_NOOP("White color balance algorithm"),
                     "jochym at ifj edu pl");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("White Balance Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------

    QGridLayout* topLayout = new QGridLayout( plainPage(), 2, 2 , marginHint(), spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("White Color Balance Correction"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addMultiCellWidget(headerFrame, 0, 0, 0, 1);

    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    // -------------------------------------------------------------
    
    QGroupBox *gbox = new QGroupBox(i18n("Settings"), plainPage());
    gbox->setFlat(false);
    QVBoxLayout* layout2 = new QVBoxLayout( gbox, 20 );

    QGridLayout *grid = new QGridLayout( layout2, 2, 4, spacingHint());
    
    QLabel *label1 = new QLabel(i18n("Channel:"), gbox);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( false, gbox );
    m_channelCB->insertItem( i18n("Red") );
    m_channelCB->insertItem( i18n("Green") );
    m_channelCB->insertItem( i18n("Blue") );
    QWhatsThis::add( m_channelCB, i18n("<p>Select here the histogram channel to display:<p>"
                                       "<b>Red</b>: display the red image-channel values.<p>"
                                       "<b>Green</b>: display the green image-channel values.<p>"
                                       "<b>Blue</b>: display the blue image-channel values.<p>"));

    QLabel *label2 = new QLabel(i18n("Scale:"), gbox);
    label2->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_scaleCB = new QComboBox( false, gbox );
    m_scaleCB->insertItem( i18n("Linear") );
    m_scaleCB->insertItem( i18n("Logarithmic") );
    m_scaleCB->setCurrentText( i18n("Logarithmic") );
    QWhatsThis::add( m_scaleCB, i18n("<p>Select here the histogram scale.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the "
                                     "graph."));

    grid->addMultiCellWidget(label1, 0, 0, 0, 0);
    grid->addMultiCellWidget(m_channelCB, 0, 0, 1, 1);
    grid->addMultiCellWidget(label2, 0, 0, 3, 3);
    grid->addMultiCellWidget(m_scaleCB, 0, 0, 4, 4);
        
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);

    m_histogramWidget = new Digikam::HistogramWidget(256, 140, imageData, width, height, frame, false);
    QWhatsThis::add( m_histogramWidget, i18n("<p>Here you can see the target preview image histogram drawing of the "
                                             "selected image channel. This one is re-computed at any filter "
                                             "settings changes."));
    l->addWidget(m_histogramWidget, 0);
    grid->addMultiCellWidget(frame, 1, 1, 0, 4);
    
    m_hGradient = new Digikam::ColorGradientWidget( KSelector::Horizontal, 20, gbox );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    grid->addMultiCellWidget(m_hGradient, 2, 2, 0, 4);
    
    // -------------------------------------------------------------

    m_grid = new QGridLayout( layout2, 6, 4, spacingHint());
    
    // Color Picker settings widgets.
    
    m_blackPickColorLabel = new QLabel(i18n("Black Tone:"), gbox);
    m_grayPickColorLabel = new QLabel(i18n("Gray Tone:"), gbox);
    m_whitePickColorLabel = new QLabel(i18n("White Tone:"), gbox);
    
    m_blackColorButton = new QPushButton( gbox );
    QWhatsThis::add( m_blackColorButton, i18n("<p>Set here the color from original image use to correct <b>Black</b> "
                                              "tones with white-balance filter using <b>Color Picker</b> method."));
    m_blackColorButton->setToggleButton(true);
    
    m_grayColorButton = new QPushButton( gbox );
    QWhatsThis::add( m_grayColorButton, i18n("<p>Set here the color from original image use to correct <b>Gray</b> "
                                              "tones with white-balance filter using <b>Color Picker</b> method."));
    m_grayColorButton->setToggleButton(true);
    
    m_whiteColorButton = new QPushButton( gbox );
    QWhatsThis::add( m_whiteColorButton, i18n("<p>Set here the color from original image use to correct <b>White</b> "
                                              "tones with white-balance filter using <b>Color Picker</b> method."));
    m_whiteColorButton->setToggleButton(true);
    m_whiteColorButton->setOn(true);
        
    // Correction Filter settings widgets.
    
    m_darkLabel = new QLabel(i18n("Shadows:"), gbox);
    m_darkInput = new KDoubleNumInput(gbox);
    m_darkInput->setPrecision(2);
    m_darkInput->setRange(0.0, 1.0, 0.01, true);
    QWhatsThis::add( m_darkInput, i18n("<p>Set here the shadows noise suppresion level."));
        
    m_blackLabel = new QLabel(i18n("Black Point:"), gbox);
    m_blackInput = new KDoubleNumInput(gbox);
    m_blackInput->setPrecision(2);
    m_blackInput->setRange(0.0, 0.05, 0.01, true);
    QWhatsThis::add( m_blackInput, i18n("<p>Set here the black level value."));
        
    m_exposureLabel = new QLabel(i18n("Exposure:"), gbox);
    m_exposureInput = new KDoubleNumInput(gbox);
    m_exposureInput->setPrecision(2);
    m_exposureInput->setRange(-6.0, 8.0, 0.01, true);
    QWhatsThis::add( m_exposureInput, i18n("<p>Set here the Exposure Value (EV)."));
    
    m_gammaLabel = new QLabel(i18n("Gamma:"), gbox);
    m_gammaInput = new KDoubleNumInput(gbox);
    m_gammaInput->setPrecision(2);
    m_gammaInput->setRange(0.01, 1.5, 0.01, true);
    QWhatsThis::add( m_gammaInput, i18n("<p>Set here the gamma corection value."));
        
    m_temperatureLabel = new QLabel(i18n("Temperature:"), gbox);
    m_temperatureInput = new KIntNumInput(gbox);
    m_temperatureInput->setRange(2200, 7000, 100, true);
    QWhatsThis::add( m_temperatureInput, i18n("<p>Set here the white balance colour temperature in Kelvin."));
        
    m_saturationLabel = new QLabel(i18n("Saturation:"), gbox);
    m_saturationInput = new KDoubleNumInput(gbox);
    m_saturationInput->setPrecision(2);
    m_saturationInput->setRange(0.0, 2.0, 0.1, true);
    QWhatsThis::add( m_saturationInput, i18n("<p>Set here the saturation value."));
        
    m_greenLabel = new QLabel(i18n("Green:"), gbox);
    m_greenInput = new KDoubleNumInput(gbox);
    m_greenInput->setPrecision(2);
    m_greenInput->setRange(0.2, 2.5, 0.1, true);
    QWhatsThis::add( m_greenInput, i18n("<p>Set here the magenta colour cast removal level."));

    layout2->addStretch();
        
    topLayout->addMultiCellWidget(gbox, 1, 1, 0, 0);
        
    // -------------------------------------------------------------

    QGroupBox *gbox4 = new QGroupBox(i18n("Preview"), plainPage());
    gbox4->setFlat(false);
    QGridLayout* grid2 = new QGridLayout( gbox4, 3, 2, 20, spacingHint());

    QFrame *frame2 = new QFrame(gbox4);
    frame2->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l2  = new QVBoxLayout(frame2, 5, 0);
    m_previewOriginalWidget = new Digikam::ImageGuideWidget(300, 200, frame2, true, Digikam::ImageGuideWidget::PickColorMode);
    QWhatsThis::add( m_previewOriginalWidget, i18n("<p>You can see here the original image. You can pick color on image to select "
                                                   "the tone to ajust image's white-balance with <b>Color Picker</b> method."));
    l2->addWidget(m_previewOriginalWidget, 0, Qt::AlignCenter);
    grid2->addMultiCellWidget(frame2, 0, 0, 0, 1);
    
    QLabel *label10 = new QLabel(i18n("Method:"), gbox4);
    m_wbMethod = new QComboBox( false, gbox4 );
    m_wbMethod->insertItem( i18n("Color Picker") );
    m_wbMethod->insertItem( i18n("Correction Filters") );
    QWhatsThis::add( m_wbMethod, i18n("<p>Select here the method to adjust white color balance:<p>"
                                      "<b>Color Picker</b>: method for picking the color of some brightly lit colored object "
                                      "in the original photograph and setting a correction filter from that.<p>"
                                      "<b>Correction Filters</b>: advanced method using all filter settings manualy.<p>"));
    
    grid2->addMultiCellWidget(label10, 1, 1, 0, 0);
    grid2->addMultiCellWidget(m_wbMethod, 1, 1, 1, 1);
            
    QFrame *frame3 = new QFrame(gbox4);
    frame3->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l3  = new QVBoxLayout(frame3, 5, 0);
    m_previewTargetWidget = new Digikam::ImageWidget(300, 200, frame3);
    QWhatsThis::add( m_previewTargetWidget, i18n("<p>You can see here the image's white-balance adjustments preview."));
    l3->addWidget(m_previewTargetWidget, 0, Qt::AlignCenter);
    grid2->addMultiCellWidget(frame3, 2, 2, 0, 1);

    topLayout->addMultiCellWidget(gbox4, 1, 1, 1, 1);
    
    // -------------------------------------------------------------
    slotMethodChanged(ColorPicker);
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
    adjustSize();
    disableResize();
    parentWidget()->setCursor( KCursor::arrowCursor()  );

    // -------------------------------------------------------------
 
    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));
    
    connect(m_scaleCB, SIGNAL(activated(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_wbMethod, SIGNAL(activated(int)),
            this, SLOT(slotMethodChanged(int)));
                
    connect(m_previewOriginalWidget, SIGNAL(spotColorChanged( const QColor & )),
            this, SLOT(slotColorSelectedFromImage( const QColor & ))); 

    // Color Picker Buttons.
    
    connect(m_blackColorButton, SIGNAL(toggled(bool)),
            this, SLOT(slotBlackColorPickerToggle(bool)));
    
    connect(m_grayColorButton, SIGNAL(toggled(bool)),
            this, SLOT(slotGrayColorPickerToggle(bool)));
                
    connect(m_whiteColorButton, SIGNAL(toggled(bool)),
            this, SLOT(slotWhiteColorPickerToggle(bool)));
            
    // Slider controls.
                        
    connect(m_temperatureInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));                       
            
    connect(m_darkInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));                       
    
    connect(m_blackInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));                       
    
    connect(m_exposureInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));                       
    
    connect(m_gammaInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));                       

    connect(m_saturationInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));                         

    connect(m_greenInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));                 
}

ImageEffect_WhiteBalance::~ImageEffect_WhiteBalance()
{
}

void ImageEffect_WhiteBalance::closeEvent(QCloseEvent *e)
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
       
    delete m_histogramWidget;
    delete m_whiteBalanceCurves;
    e->accept();
}

void ImageEffect_WhiteBalance::slotUser1()
{
    blockSignals(true);

    if ( m_wbMethod->currentItem() == CorrectionFilter )
       {
       // Neutral color temperature settings.
       m_temperatureInput->setValue(4750);                     
       m_darkInput->setValue(0.5);
       m_blackInput->setValue(0.0);
       m_exposureInput->setValue(0.0);
       m_gammaInput->setValue(0.6);  
       m_saturationInput->setValue(1.0);  
       m_greenInput->setValue(1.2);  
       }
    else
       {    
       m_blackColor = Qt::black;
       setBlackColor(m_blackColor);    
       m_grayColor = Qt::gray;
       setGrayColor(m_grayColor);    
       m_whiteColor = Qt::white;
       setWhiteColor(m_whiteColor);  
       m_whiteBalanceCurves->curvesReset();
       }
    
    m_previewOriginalWidget->resetSpotPosition();    
    m_channelCB->setCurrentItem(RedChannel);
    slotChannelChanged(RedChannel);
    
    blockSignals(false);
    slotEffect();  
} 

void ImageEffect_WhiteBalance::slotHelp()
{
    KApplication::kApplication()->invokeHelp("whitebalance", "digikamimageplugins");
}

void ImageEffect_WhiteBalance::slotBlackColorPickerToggle(bool b)
{
    m_grayColorButton->blockSignals(true);
    m_whiteColorButton->blockSignals(true);
    m_grayColorButton->setOn(!b);
    m_whiteColorButton->setOn(!b);
    m_grayColorButton->blockSignals(false);
    m_whiteColorButton->blockSignals(false);
}

void ImageEffect_WhiteBalance::slotGrayColorPickerToggle(bool g)
{
    m_blackColorButton->blockSignals(true);
    m_whiteColorButton->blockSignals(true);
    m_blackColorButton->setOn(!g);
    m_whiteColorButton->setOn(!g);
    m_blackColorButton->blockSignals(false);
    m_whiteColorButton->blockSignals(false);
}

void ImageEffect_WhiteBalance::slotWhiteColorPickerToggle(bool w)
{
    m_blackColorButton->blockSignals(true);
    m_grayColorButton->blockSignals(true);    
    m_blackColorButton->setOn(!w);
    m_grayColorButton->setOn(!w);
    m_blackColorButton->blockSignals(false);
    m_grayColorButton->blockSignals(false);    
}

void ImageEffect_WhiteBalance::slotMethodChanged(int method)
{
   switch(method)
       {
       case ColorPicker:
          {
          m_darkLabel->hide();
          m_darkInput->hide();
          m_blackLabel->hide();
          m_blackInput->hide();
          m_exposureLabel->hide();
          m_exposureInput->hide();
          m_gammaLabel->hide();
          m_gammaInput->hide();
          m_temperatureLabel->hide();
          m_temperatureInput->hide();
          m_saturationLabel->hide();
          m_saturationInput->hide();
          m_greenLabel->hide();
          m_greenInput->hide();
          m_grid->remove(m_darkLabel);
          m_grid->remove(m_darkInput);
          m_grid->remove(m_blackLabel);
          m_grid->remove(m_blackInput);
          m_grid->remove(m_exposureLabel);
          m_grid->remove(m_exposureInput);
          m_grid->remove(m_gammaLabel);
          m_grid->remove(m_gammaInput);
          m_grid->remove(m_temperatureLabel);
          m_grid->remove(m_temperatureInput);
          m_grid->remove(m_saturationLabel);
          m_grid->remove(m_saturationInput);
          m_grid->remove(m_greenLabel);
          m_grid->remove(m_greenInput);
          
          m_grid->addMultiCellWidget(m_blackPickColorLabel, 0, 0, 0, 0);
          m_blackPickColorLabel->show();
          m_grid->addMultiCellWidget(m_grayPickColorLabel, 1, 1, 0, 0);
          m_grayPickColorLabel->show();
          m_grid->addMultiCellWidget(m_whitePickColorLabel, 2, 2, 0, 0);
          m_whitePickColorLabel->show();
          m_grid->addMultiCellWidget(m_blackColorButton, 0, 0, 1, 1);
          m_blackColorButton->show();
          m_grid->addMultiCellWidget(m_grayColorButton, 1, 1, 1, 1);
          m_grayColorButton->show();
          m_grid->addMultiCellWidget(m_whiteColorButton, 2, 2, 1, 1);
          m_whiteColorButton->show();

          slotUser1();
          break;
          }

       case CorrectionFilter:
          {
          m_blackPickColorLabel->hide();
          m_grayPickColorLabel->hide();
          m_whitePickColorLabel->hide();
          m_blackColorButton->hide();
          m_grayColorButton->hide();
          m_whiteColorButton->hide();
          m_grid->remove(m_blackPickColorLabel);
          m_grid->remove(m_grayPickColorLabel);
          m_grid->remove(m_whitePickColorLabel);
          m_grid->remove(m_blackColorButton);
          m_grid->remove(m_grayColorButton);
          m_grid->remove(m_whiteColorButton);
          
          m_grid->addMultiCellWidget(m_darkLabel, 0, 0, 0, 0);
          m_grid->addMultiCellWidget(m_darkInput, 0, 0, 1, 4);
          m_grid->addMultiCellWidget(m_blackLabel, 1, 1, 0, 0);
          m_grid->addMultiCellWidget(m_blackInput, 1, 1, 1, 4);
          m_grid->addMultiCellWidget(m_exposureLabel, 2, 2, 0, 0);
          m_grid->addMultiCellWidget(m_exposureInput, 2, 2, 1, 4);
          m_grid->addMultiCellWidget(m_gammaLabel, 3, 3, 0, 0);
          m_grid->addMultiCellWidget(m_gammaInput, 3, 3, 1, 4);
          m_grid->addMultiCellWidget(m_temperatureLabel, 4, 4, 0, 0);
          m_grid->addMultiCellWidget(m_temperatureInput, 4, 4, 1, 4);
          m_grid->addMultiCellWidget(m_saturationLabel, 5, 5, 0, 0);
          m_grid->addMultiCellWidget(m_saturationInput, 5, 5, 1, 4);
          m_grid->addMultiCellWidget(m_greenLabel, 6, 6, 0, 0);
          m_grid->addMultiCellWidget(m_greenInput, 6, 6, 1, 4);
          m_darkLabel->show();
          m_darkInput->show();
          m_blackLabel->show();
          m_blackInput->show();
          m_exposureLabel->show();
          m_exposureInput->show();
          m_gammaLabel->show();
          m_gammaInput->show();
          m_temperatureLabel->show();
          m_temperatureInput->show();
          m_saturationLabel->show();
          m_saturationInput->show();
          m_greenLabel->show();
          m_greenInput->show();
          slotUser1();
          break;
          }
       }
}

void ImageEffect_WhiteBalance::slotColorSelectedFromImage( const QColor &color )
{
    if ( m_whiteColorButton->isOn() )
       setWhiteColor(color);
    else if ( m_grayColorButton->isOn() )
       setGrayColor(color);
    else if ( m_blackColorButton->isOn() )
       setBlackColor(color);

    slotEffect();  
}

void ImageEffect_WhiteBalance::setWhiteColor(QColor color)
{
    QPixmap pixmap(16 , 16);
    pixmap.fill(color);
    m_whiteColor = color;
    m_whiteColorButton->setPixmap(pixmap);
    QString numVal;
    
    QString tip = "<table cellspacing=0 cellpadding=0>";
    QString headBeg("<tr bgcolor=\"orange\"><td colspan=2><nobr><font size=-1 color=\"black\"><i>");
    QString headEnd("</i></font></nobr></td></nobr</tr>");
    QString cellBeg("<tr><td><nobr><font size=-1 color=\"black\">");
    QString cellMid("</font></nobr></td><td><nobr><font size=-1 color=\"black\">");
    QString cellEnd("</font></nobr></td></tr>");

    tip += headBeg + i18n("<b>Color Picker</b> method<br>White color tone:") + headEnd;
    tip += cellBeg + i18n("Red:") + cellMid;
    tip += numVal.setNum(m_whiteColor.red()) + cellEnd;
    tip += cellBeg + i18n("Green:") + cellMid;
    tip += numVal.setNum(m_whiteColor.green()) + cellEnd;
    tip += cellBeg + i18n("Blue:") + cellMid;
    tip += numVal.setNum(m_whiteColor.blue()) + cellEnd;
    tip += "</table>";
        
    QToolTip::add( m_whiteColorButton, tip);    
}

void ImageEffect_WhiteBalance::setGrayColor(QColor color)
{
    QPixmap pixmap(16 , 16);
    pixmap.fill(color);
    m_grayColor = color;
    m_grayColorButton->setPixmap(pixmap);
    QString numVal;
    
    QString tip = "<table cellspacing=0 cellpadding=0>";
    QString headBeg("<tr bgcolor=\"orange\"><td colspan=2><nobr><font size=-1 color=\"black\"><i>");
    QString headEnd("</i></font></nobr></td></nobr</tr>");
    QString cellBeg("<tr><td><nobr><font size=-1 color=\"black\">");
    QString cellMid("</font></nobr></td><td><nobr><font size=-1 color=\"black\">");
    QString cellEnd("</font></nobr></td></tr>");

    tip += headBeg + i18n("<b>Color Picker</b> method<br>Gray color tone:") + headEnd;
    tip += cellBeg + i18n("Red:") + cellMid;
    tip += numVal.setNum(m_grayColor.red()) + cellEnd;
    tip += cellBeg + i18n("Green:") + cellMid;
    tip += numVal.setNum(m_grayColor.green()) + cellEnd;
    tip += cellBeg + i18n("Blue:") + cellMid;
    tip += numVal.setNum(m_grayColor.blue()) + cellEnd;
    tip += "</table>";
        
    QToolTip::add( m_grayColorButton, tip);    
}

void ImageEffect_WhiteBalance::setBlackColor(QColor color)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(color);
    m_blackColor = color;
    m_blackColorButton->setPixmap(pixmap);
    QString numVal;
    
    QString tip = "<table cellspacing=0 cellpadding=0>";
    QString headBeg("<tr bgcolor=\"orange\"><td colspan=2><nobr><font size=-1 color=\"black\"><i>");
    QString headEnd("</i></font></nobr></td></nobr</tr>");
    QString cellBeg("<tr><td><nobr><font size=-1 color=\"black\">");
    QString cellMid("</font></nobr></td><td><nobr><font size=-1 color=\"black\">");
    QString cellEnd("</font></nobr></td></tr>");

    tip += headBeg + i18n("<b>Color Picker</b> method<br>Black color tone:") + headEnd;
    tip += cellBeg + i18n("Red:") + cellMid;
    tip += numVal.setNum(m_blackColor.red()) + cellEnd;
    tip += cellBeg + i18n("Green:") + cellMid;
    tip += numVal.setNum(m_blackColor.green()) + cellEnd;
    tip += cellBeg + i18n("Blue:") + cellMid;
    tip += numVal.setNum(m_blackColor.blue()) + cellEnd;
    tip += "</table>";
        
    QToolTip::add( m_blackColorButton, tip);    
}

void ImageEffect_WhiteBalance::slotScaleChanged(int scale)
{
    switch(scale)
       {
       case Linear:
          m_histogramWidget->m_scaleType = Digikam::HistogramWidget::LinScaleHistogram;
          break;
       
       case Logarithmic:
          m_histogramWidget->m_scaleType = Digikam::HistogramWidget::LogScaleHistogram;
          break;
       }

    m_histogramWidget->repaint(false);
}

void ImageEffect_WhiteBalance::slotChannelChanged(int channel)
{
    switch(channel)
       {
       case RedChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
          m_hGradient->setColors( QColor( "red" ), QColor( "black" ) );
          break;

       case GreenChannel:         
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
          m_hGradient->setColors( QColor( "green" ), QColor( "black" ) );
          break;

       case BlueChannel:         
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
          m_hGradient->setColors( QColor( "blue" ), QColor( "black" ) );
          break;
       }

    m_histogramWidget->repaint(false);
}

void ImageEffect_WhiteBalance::slotEffect()
{
    Digikam::ImageIface* iface = m_previewTargetWidget->imageIface();

    uint*  data   = iface->getPreviewData();
    int    w      = iface->previewWidth();
    int    h      = iface->previewHeight();
    
    // Create the new empty destination image data space.
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
    
    m_destinationPreviewData = new uint[w*h];
    memcpy (m_destinationPreviewData, data, w*h*4);  
    
    if ( m_wbMethod->currentItem() == CorrectionFilter )
       {
       // Update settings
       m_temperature = m_temperatureInput->value()/1000.0;
       m_dark        = m_darkInput->value();
       m_black       = m_blackInput->value();
       m_exposition  = m_exposureInput->value();
       m_gamma       = m_gammaInput->value();
       m_saturation  = m_saturationInput->value();
       m_green       = m_greenInput->value();
       
       // Set preview lut.
       setRGBmult();
       m_mg = 1.0;
       setLUTv();
       setRGBmult();
       
       // Apply White balance adjustments.
       whiteBalanceCorrectionFilter(m_destinationPreviewData, w, h);
       }
    else
       whiteBalanceColorPicker(m_destinationPreviewData, w, h, m_blackColor, m_grayColor, m_whiteColor);
           
    iface->putPreviewData(m_destinationPreviewData);       
    m_previewTargetWidget->update();
    
    // Update histogram.
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, 0, 0, 0, false); 
    
    delete [] data;
}

void ImageEffect_WhiteBalance::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
        
    uint*  data   = iface.getOriginalData();
    int    w      = iface.originalWidth();
    int    h      = iface.originalHeight();
        
    if ( m_wbMethod->currentItem() == CorrectionFilter )
       {
       // Update settings
       m_temperature = m_temperatureInput->value()/1000.0;
       m_dark        = m_darkInput->value();
       m_black       = m_blackInput->value();
       m_exposition  = m_exposureInput->value();
       m_gamma       = m_gammaInput->value();
       m_saturation  = m_saturationInput->value();
       m_green       = m_greenInput->value();
       
       // Set final lut.
       setRGBmult();
       m_mr = m_mb = 1.0;
       if (m_clipSat) m_mg = 1.0; 
       setLUTv();
       setRGBmult();
       
       // Apply White balance adjustments.
       whiteBalanceCorrectionFilter(data, w, h);
       }
    else
       whiteBalanceColorPicker(data, w, h, m_blackColor, m_grayColor, m_whiteColor);

    iface.putOriginalData(i18n("White Balance"), data);                   
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

void ImageEffect_WhiteBalance::setRGBmult(void)
{
    int   t;
    float mi;

    if ( m_temperature > 7.0 ) m_temperature = 7.0;
    
    t     = (int)(m_temperature * 100.0 - 200.0);
    m_mr  = 1.0 / bbWB[t][0];
    m_mg  = 1.0 / bbWB[t][1];
    m_mb  = 1.0 / bbWB[t][2];
    m_mg *= m_green;
    
    // Normalize to at least 1.0, so we are not dimming colors only bumping.
    mi    = QMIN(m_mr, m_mg);
    mi    = QMIN(mi, m_mb);
    m_mr /= mi;
    m_mg /= mi;
    m_mb /= mi;
}

void ImageEffect_WhiteBalance::setLUTv(void)
{
    double b, g;

    b    = m_mg * pow(2, m_exposition);
    g    = m_gamma;
    m_BP = (uint)(m_rgbMax * m_black);
    m_WP = (uint)(m_rgbMax / b);
    
    if (m_WP - m_BP < 1) m_WP = m_BP + 1;

    kdDebug() << k_funcinfo 
              << "T(K): " << m_temperature
              << " => R:" << m_mr
              << " G:"    << m_mg
              << " B:"    << m_mb
              << " BP:"   << m_BP
              << " WP:"   << m_WP
              << endl;
    
    curve[0] = 0;
    
    for (int i = 1; i < 256; i++)
        {
        float x = (float)(i - m_BP)/(m_WP - m_BP);
        curve[i]  = (i < m_BP) ? 0 : MAXOUT * pow(x, g);
        curve[i] *= (1 - m_dark * exp(-x * x / 0.002));
        curve[i] /= (float)i;
        }
}

void ImageEffect_WhiteBalance::whiteBalanceCorrectionFilter(uint *data, int width, int height)
{  
    int   x, y, c;
    uint   i;

    uchar *pOutBits = new uchar[width*height*4];
    uchar *rp = (uchar *)data;
    uchar *p  = pOutBits;
         
    for (y = 0 ; y < height ; y++)
        {
        for (x = 0 ; x < width ; x++, p += 4, rp += 4) 
            {
            int v, rv[3];
            
            rv[0] = (int)(rp[0] * m_mb);
            rv[1] = (int)(rp[1] * m_mg);
            rv[2] = (int)(rp[2] * m_mr);
            v = QMAX(rv[0], rv[1]);
            v = QMAX(v, rv[2]); 
            
            if (m_clipSat) v = QMIN(v, m_rgbMax);
            i = v;

            for (c = 0 ; c < 3 ; c++) 
                {
                int r, o;
                
                r = (m_clipSat && rv[c] > m_rgbMax) ? m_rgbMax : rv[c];

                if (v <= m_BP) 
                    p[c] = o = 0;
                else if (m_overExp && v > m_WP) 
                    {
                    if (m_WBind)
                        r = (rv[c] > m_WP) ? 0 : r;
                    else
                        r = 0;
                    }
                
                o = (int)((i - m_saturation*(i - r)) * curve[i]);
                o = QMAX(o, 0);
                o = (o > MAXOUT) ? 255 : o;
                
                p[c] = (uchar)o;
                }
            }
        }

    memcpy (data, pOutBits, width*height*4);       
            
    delete [] pOutBits;    
}

// The theory of this method inspired from tutorial from 'lasm' available at http://www.geocities.com/lasm.rm/wb2.html
// I have re-create a new curves computation (not based on linear color transformation) for to have better results.        

void ImageEffect_WhiteBalance::whiteBalanceColorPicker(uint *data, int width, int height, QColor bColor, QColor gColor, QColor wColor)
{  
    uchar* pOutBits = new uchar[width*height*4];    

    // Start curves point.  
            
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 0,  QPoint::QPoint(0, 0));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 0,  QPoint::QPoint(0, 0));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 0,  QPoint::QPoint(0, 0));      

    // Black curves point.
    
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 1,  QPoint::QPoint(bColor.red(), 35));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 1,  QPoint::QPoint(bColor.green(), 35));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 1,  QPoint::QPoint(bColor.blue(), 35));          
    
    // Gray curves point.
    
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 8,  QPoint::QPoint(gColor.red(), 128));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 8,  QPoint::QPoint(gColor.green(), 128));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 8,  QPoint::QPoint(gColor.blue(), 128));      
    
    // White curves point.
    
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 15,  QPoint::QPoint(wColor.red(), 220));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 15,  QPoint::QPoint(wColor.green(), 220));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 15,  QPoint::QPoint(wColor.blue(), 220));      
    
    // Final curves point.
    
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 16,  QPoint::QPoint(255, 255));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 16,  QPoint::QPoint(255, 255));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 16,  QPoint::QPoint(255, 255));      

    // Calculate Red, green, blue curves.
    
    for (int i = Digikam::ImageHistogram::RedChannel ; i <= Digikam::ImageHistogram::BlueChannel ; i++)
       m_whiteBalanceCurves->curvesCalculateCurve(i);
                  
    // Calculate lut and apply to image.
       
    m_whiteBalanceCurves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);
    m_whiteBalanceCurves->curvesLutProcess(data, (uint *)pOutBits, width, height);
    
    memcpy (data, pOutBits, width*height*4);       
       
    delete [] pOutBits;
}

}  // NameSpace DigikamWhiteBalanceImagesPlugin

#include "imageeffect_whitebalance.moc"
