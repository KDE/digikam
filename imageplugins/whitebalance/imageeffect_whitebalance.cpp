/* ============================================================
 * File  : imageeffect_whitebalance.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-11
 * Description : a digiKam image editor plugin to correct 
 *               image white balance 
 * 
 * Copyright 2005 by Gilles Caulier
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
#include <qhbuttongroup.h> 
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
    topLayout->addMultiCellWidget(headerFrame, 0, 0, 0, 2);

    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    // -------------------------------------------------------------
    
    QGroupBox *gbox = new QGroupBox(plainPage());
    gbox->setFlat(false);
    gbox->setTitle(i18n("Curves Corrections"));
    QGridLayout* grid = new QGridLayout( gbox, 11, 5, 20, spacingHint());
    
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

    grid->addMultiCellWidget(label1, 0, 0, 1, 1);
    grid->addMultiCellWidget(m_channelCB, 0, 0, 2, 2);
    grid->addMultiCellWidget(label2, 0, 0, 4, 4);
    grid->addMultiCellWidget(m_scaleCB, 0, 0, 5, 5);
    
    // -------------------------------------------------------------
        
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);

    m_vGradient = new Digikam::ColorGradientWidget( KSelector::Vertical, 20, gbox );
    m_vGradient->setColors( QColor( "white" ), QColor( "black" ) );
    grid->addMultiCellWidget(m_vGradient, 2, 2, 0, 0);

    m_whiteBalanceCurvesWidget = new Digikam::CurvesWidget(256, 128, imageData, width, height, 
                                                           m_whiteBalanceCurves, frame, true);
    QWhatsThis::add( m_whiteBalanceCurvesWidget, i18n("<p>This is the curve drawing of the selected image "
                                                      "histogram channel. No operation can be done here. "
                                                      "This graph is just an informative view witch will "
                                                      "inform you how the color curves auto-adjustments "
                                                      "will be processed on image."));
    l->addWidget(m_whiteBalanceCurvesWidget, 0);
    grid->addMultiCellWidget(frame, 2, 2, 1, 5);
    
    m_hGradient = new Digikam::ColorGradientWidget( KSelector::Horizontal, 20, gbox );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    grid->addMultiCellWidget(m_hGradient, 3, 3, 1, 5);
    
    // -------------------------------------------------------------
    
    QLabel *label3 = new QLabel(i18n("Shadows:"), gbox);
    m_darkInput = new KDoubleNumInput(gbox);
    m_darkInput->setPrecision(2);
    m_darkInput->setRange(0.0, 1.0, 0.01, true);
    QWhatsThis::add( m_darkInput, i18n("<p>Set here the shadows noise suppresion level."));
        
    grid->addMultiCellWidget(label3, 5, 5, 1, 1);
    grid->addMultiCellWidget(m_darkInput, 5, 5, 2, 5);

    QLabel *label4 = new QLabel(i18n("Black Point:"), gbox);
    m_blackInput = new KDoubleNumInput(gbox);
    m_blackInput->setPrecision(2);
    m_blackInput->setRange(0.0, 0.05, 0.01, true);
    QWhatsThis::add( m_blackInput, i18n("<p>Set here the black level value."));
        
    grid->addMultiCellWidget(label4, 6, 6, 1, 1);
    grid->addMultiCellWidget(m_blackInput, 6, 6, 2, 5);

    QLabel *label5 = new QLabel(i18n("Exposure (EV):"), gbox);
    m_exposureInput = new KDoubleNumInput(gbox);
    m_exposureInput->setPrecision(2);
    m_exposureInput->setRange(-6.0, 8.0, 0.01, true);
    QWhatsThis::add( m_exposureInput, i18n("<p>Set here the exposure value in EV."));
        
    grid->addMultiCellWidget(label5, 7, 7, 1, 1);
    grid->addMultiCellWidget(m_exposureInput, 7, 7, 2, 5);
    
    QLabel *label6 = new QLabel(i18n("Gamma:"), gbox);
    m_gammaInput = new KDoubleNumInput(gbox);
    m_gammaInput->setPrecision(2);
    m_gammaInput->setRange(0.01, 1.5, 0.01, true);
    QWhatsThis::add( m_gammaInput, i18n("<p>Set here the gamma corection value."));
        
    grid->addMultiCellWidget(label6, 8, 8, 1, 1);
    grid->addMultiCellWidget(m_gammaInput, 8, 8, 2, 5);
    
    QLabel *label7 = new QLabel(i18n("Temperature (K):"), gbox);
    m_temperatureInput = new KDoubleNumInput(gbox);
    m_temperatureInput->setPrecision(2);
    m_temperatureInput->setRange(2.2, 7.0, 0.1, true);
    QWhatsThis::add( m_temperatureInput, i18n("<p>Set here the white balance colour temperature in Kelvin."));
        
    grid->addMultiCellWidget(label7, 9, 9, 1, 1);
    grid->addMultiCellWidget(m_temperatureInput, 9, 9, 2, 5);
    
    QLabel *label8 = new QLabel(i18n("Saturation:"), gbox);
    m_saturationInput = new KDoubleNumInput(gbox);
    m_saturationInput->setPrecision(2);
    m_saturationInput->setRange(0.0, 2.0, 0.1, true);
    QWhatsThis::add( m_saturationInput, i18n("<p>Set here the saturation value."));
        
    grid->addMultiCellWidget(label8, 10, 10, 1, 1);
    grid->addMultiCellWidget(m_saturationInput, 10, 10, 2, 5);
    
    QLabel *label9 = new QLabel(i18n("Green:"), gbox);
    m_greenInput = new KDoubleNumInput(gbox);
    m_greenInput->setPrecision(2);
    m_greenInput->setRange(0.2, 2.5, 0.1, true);
    QWhatsThis::add( m_greenInput, i18n("<p>Set here the magenta colour cast removal level."));
        
    grid->addMultiCellWidget(label9, 11, 11, 1, 1);
    grid->addMultiCellWidget(m_greenInput, 11, 11, 2, 5);
    
    topLayout->addMultiCellWidget(gbox, 1, 1, 0, 0);
        
    // -------------------------------------------------------------

    QVGroupBox *gbox4 = new QVGroupBox(i18n("Preview"), plainPage());

    QFrame *frame2 = new QFrame(gbox4);
    frame2->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l2  = new QVBoxLayout(frame2, 5, 0);
    m_previewOriginalWidget = new Digikam::ImageGuideWidget(300, 200, frame2, true, Digikam::ImageGuideWidget::PickColorMode);
    QWhatsThis::add( m_previewOriginalWidget, i18n("<p>You can see here the original image. Click on image to select "
                                                   "the tone colors to ajust image's white-balance."));
    
    l2->addWidget(m_previewOriginalWidget, 0, Qt::AlignCenter);
    
    QHBoxLayout *hlay3 = new QHBoxLayout(l2);                                                   
                                                   
    QLabel *label10 = new QLabel(i18n("Method:"), frame2);
    m_wbMethod = new QComboBox( false, frame2 );
    m_wbMethod->insertItem( i18n("Color Picker") );
    m_wbMethod->insertItem( i18n("Correction Filters") );
    QWhatsThis::add( m_wbMethod, i18n("<p>Select here the method to adjust white color balance:<p>"
                                      "<b>Color Picker</b>: method for picking the color of some brightly lit colored object "
                                      "in the original photograph and setting a correction filter from that.<p>"
                                      "<b>Correction Filters</b>: advanced method using all filter settings manualy.<p>"));
    
    QHButtonGroup *bGroup = new QHButtonGroup(frame2);
    m_blackColorButton = new QPushButton( bGroup );
    QWhatsThis::add( m_blackColorButton, i18n("<p>Set here the color from original image use to correct <b>Black</b> "
                                              "tones with white-balance."));
    bGroup->insert(m_blackColorButton, WhiteColor);
    m_blackColorButton->setToggleButton(true);
    m_whiteColorButton = new QPushButton( bGroup );
    QWhatsThis::add( m_whiteColorButton, i18n("<p>Set here the color from original image use to correct <b>White</b> "
                                              "tones with white-balance."));
    bGroup->insert(m_whiteColorButton, BlackColor);
    m_whiteColorButton->setToggleButton(true);
    m_whiteColorButton->setOn(true);
    bGroup->setExclusive(true);
    bGroup->setFrameShape(QFrame::NoFrame);
    
    hlay3->addWidget(label10, 1);
    hlay3->addWidget(m_wbMethod, 2);
    hlay3->addWidget(bGroup, 2);
            
    QFrame *frame3 = new QFrame(gbox4);
    frame3->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l3  = new QVBoxLayout(frame3, 5, 0);
    m_previewTargetWidget = new Digikam::ImageWidget(300, 200, frame3);
    QWhatsThis::add( m_previewTargetWidget, i18n("<p>You can see here the image's white-balance adjustments preview."));
    l3->addWidget(m_previewTargetWidget, 0, Qt::AlignCenter);

    topLayout->addMultiCellWidget(gbox4, 1, 1, 1, 1);
    
    // -------------------------------------------------------------

    adjustSize();
    disableResize();
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
    parentWidget()->setCursor( KCursor::arrowCursor()  );

    // -------------------------------------------------------------
 
    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));
    
    connect(m_scaleCB, SIGNAL(activated(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_wbMethod, SIGNAL(activated(int)),
            this, SLOT(slotMethodChanged(int)));
                
    connect(m_previewOriginalWidget, SIGNAL(crossCenterColorChanged( const QColor & )),
            this, SLOT(slotColorSelectedFromImage( const QColor & ))); 

    // Slider controls.
                        
    connect(m_temperatureInput, SIGNAL(valueChanged (double)),
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
    delete m_whiteBalanceCurvesWidget;
    delete m_whiteBalanceCurves;
    e->accept();
}

void ImageEffect_WhiteBalance::slotUser1()
{
    blockSignals(true);

    m_blackColor = Qt::black;
    setBlackColor(m_blackColor);    
    m_whiteColor = Qt::white;
    setWhiteColor(m_whiteColor);  
    
    // Neutral color temperature settings.
    m_temperatureInput->setValue(4.75);                     
    m_darkInput->setValue(0.5);
    m_blackInput->setValue(0.0);
    m_exposureInput->setValue(0.0);
    m_gammaInput->setValue(0.6);  
    m_saturationInput->setValue(1.0);  
    m_greenInput->setValue(1.2);  
        
    m_whiteBalanceCurves->curvesReset();
    m_previewOriginalWidget->resetCrossPosition();    
    m_channelCB->setCurrentItem(0);
    slotChannelChanged(0);
    
    blockSignals(false);
    slotEffect();  
} 

void ImageEffect_WhiteBalance::slotHelp()
{
    KApplication::kApplication()->invokeHelp("whitebalance", "digikamimageplugins");
}

void ImageEffect_WhiteBalance::slotMethodChanged(int method)
{
   switch(method)
       {
       case ColorPicker:
          {
          break;
          }

       case CorrectionFilter:
          {
          break;
          }
       }

}

void ImageEffect_WhiteBalance::slotColorSelectedFromImage( const QColor &color )
{
    if ( m_whiteColorButton->isOn() )
       setWhiteColor(color);
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
    QToolTip::add( m_whiteColorButton, i18n( "<p>White color tone:<p>"
                                             "Red:   %1.<br>"
                                             "Green: %2<br>"
                                             "Blue:  %3")
                                             .arg(m_whiteColor.red())
                                             .arg(m_whiteColor.green())
                                             .arg(m_whiteColor.blue()) );
}

void ImageEffect_WhiteBalance::setBlackColor(QColor color)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(color);
    m_blackColor = color;
    m_blackColorButton->setPixmap(pixmap);
    QToolTip::add( m_blackColorButton, i18n( "<p>Black color tone:<p>"
                                             "Red:   %1.<br>"
                                             "Green: %2<br>"
                                             "Blue:  %3")
                                             .arg(m_blackColor.red())
                                             .arg(m_blackColor.green())
                                             .arg(m_blackColor.blue()) );
}

void ImageEffect_WhiteBalance::slotScaleChanged(int scale)
{
    switch(scale)
       {
       case 1:           // Log.
          m_whiteBalanceCurvesWidget->m_scaleType = Digikam::CurvesWidget::LogScaleHistogram;
          break;

       default:          // Lin.
          m_whiteBalanceCurvesWidget->m_scaleType = Digikam::CurvesWidget::LinScaleHistogram;
          break;
       }

    m_whiteBalanceCurvesWidget->repaint(false);
}

void ImageEffect_WhiteBalance::slotChannelChanged(int channel)
{
    switch(channel)
       {
       case 0:           // Red.
          m_whiteBalanceCurvesWidget->m_channelType = Digikam::CurvesWidget::RedChannelHistogram;
          m_vGradient->setColors( QColor( "red" ), QColor( "black" ) );
          break;

       case 1:           // Green.
          m_whiteBalanceCurvesWidget->m_channelType = Digikam::CurvesWidget::GreenChannelHistogram;
          m_vGradient->setColors( QColor( "green" ), QColor( "black" ) );
          break;

       case 2:           // Blue.
          m_whiteBalanceCurvesWidget->m_channelType = Digikam::CurvesWidget::BlueChannelHistogram;
          m_vGradient->setColors( QColor( "blue" ), QColor( "black" ) );
          break;
       }

    m_whiteBalanceCurvesWidget->repaint(false);
}

void ImageEffect_WhiteBalance::slotEffect()
{
    Digikam::ImageIface* iface = m_previewTargetWidget->imageIface();

    uint*  data   = iface->getPreviewData();
    int    w      = iface->previewWidth();
    int    h      = iface->previewHeight();
    m_temperature = m_temperatureInput->value();
    m_dark        = m_darkInput->value();
    m_black       = m_blackInput->value();
    m_exposition  = m_exposureInput->value();
    m_gamma       = m_gammaInput->value();
    m_saturation  = m_saturationInput->value();
    m_green       = m_greenInput->value();
    
    whiteBalance(data, w, h, m_blackColor, m_whiteColor);
    m_whiteBalanceCurvesWidget->repaint(false);
    
    iface->putPreviewData(data);       
    delete [] data;
    m_previewTargetWidget->update();
}

void ImageEffect_WhiteBalance::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
        
    uint*  data   = iface.getOriginalData();
    int    w      = iface.originalWidth();
    int    h      = iface.originalHeight();
    m_temperature = m_temperatureInput->value();
    m_dark        = m_darkInput->value();
    m_black       = m_blackInput->value();
    m_exposition  = m_exposureInput->value();
    m_gamma       = m_gammaInput->value();
    m_saturation  = m_saturationInput->value();
    m_green       = m_greenInput->value();
        
    whiteBalance(data, w, h, m_blackColor, m_whiteColor);

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

void ImageEffect_WhiteBalance::setLUTv(int mv)
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

    uchar lut;
    double p, v;
        
    for (uint j = 0; j < 256; j++) 
       {
       p = (m_clipSat && j > m_rgbMax) ? m_rgbMax : j;
       v = pow(p * b / MAXSMPL, m_gamma);
       v = (v > m_BP) ? MAXOUT * (v - m_BP) / (1 - m_BP) : 0;
       lut = (uchar)((v > 256) ? mv : v);
       m_whiteBalanceCurves->setCurveValue(Digikam::ImageHistogram::RedChannel, j, lut);              
       m_whiteBalanceCurves->setCurveValue(Digikam::ImageHistogram::GreenChannel, j, lut);              
       m_whiteBalanceCurves->setCurveValue(Digikam::ImageHistogram::BlueChannel, j, lut);              
       }
}

void ImageEffect_WhiteBalance::setLUT(void)
{
    setRGBmult();
    m_mg = 1.0;
    setLUTv(m_overExp ? 0 : 255);
    setRGBmult();
}

void ImageEffect_WhiteBalance::whiteBalance(uint *data, int width, int height, QColor bColor, QColor wColor)
{  
    int   x, y, c;
    uint   i;

    uchar *pOutBits = new uchar[width*height*4];
    uchar *rp = (uchar *)data;
    uchar *p  = pOutBits;
         
    setLUT();    
    
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

    
// The theory of this method inspired from tutorial from 'lasm' available at http://www.geocities.com/lasm.rm/wb2.html
// I have re-create a new curves computation (not based on linear color transformation) for to have better results.        
    
    /*    uchar* pOutBits = new uchar[w*h*4];    

    // Start curves points.  
            
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 0,  QPoint::QPoint(0, 0));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 0,  QPoint::QPoint(0, 0));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 0,  QPoint::QPoint(0, 0));      

    // Black curves points.
    
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 1,  QPoint::QPoint(bColor.red(), 35));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 1,  QPoint::QPoint(bColor.green(), 35));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 1,  QPoint::QPoint(bColor.blue(), 35));          
    // White curves points.
    
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 15,  QPoint::QPoint(wColor.red(), 220));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 15,  QPoint::QPoint(wColor.green(), 220));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 15,  QPoint::QPoint(wColor.blue(), 220));      
    
    // Final curves points
    
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 16,  QPoint::QPoint(255, 255));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 16,  QPoint::QPoint(255, 255));      
    m_whiteBalanceCurves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 16,  QPoint::QPoint(255, 255));      

    // Calculate Red, green, blue curves.
    
    for (int i = Digikam::ImageHistogram::RedChannel ; i <= Digikam::ImageHistogram::BlueChannel ; i++)
       m_whiteBalanceCurves->curvesCalculateCurve(i);
                  
    // Calculate lut and apply to image.
       
    m_whiteBalanceCurves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);
    m_whiteBalanceCurves->curvesLutProcess(data, (uint *)pOutBits, w, h);
    
    memcpy (data, pOutBits, w*h*4);       
       
    delete [] pOutBits;*/
    
    /*
    uchar* pInBits = (uchar*)data;
    
    
    for (int y = 0; y < h; y++)
        {
        for (int x = 0; x < w; x++)
            {        
            pInBits[i++] = (uchar)(pInBits[i] * mb);    // Blue.
            pInBits[i++] = (uchar)(pInBits[i] * mg);    // Green.
            pInBits[i++] = (uchar)(pInBits[i] * mr);    // Red.
            pInBits[i++] = pInBits[i];                  // Alpha.
            }
        }
        */
}

}  // NameSpace DigikamWhiteBalanceImagesPlugin

#include "imageeffect_whitebalance.moc"
