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
#include <qtimer.h>

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
#include <kcolorbutton.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imageeffect_whitebalance.h"

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
    
    m_whiteBalanceCurves = new Digikam::ImageCurves();
        
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("White Balance"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to correct image white balance."),
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
    QLabel *labelTitle = new QLabel( i18n("Image White Balance Correction"), headerFrame, "labelTitle" );
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
    gbox->setTitle(i18n("Settings"));
    QGridLayout* grid = new QGridLayout( gbox, 4, 6, 20, spacingHint());
    
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
    
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);

    m_vGradient = new Digikam::ColorGradientWidget( KSelector::Vertical, 20, gbox );
    m_vGradient->setColors( QColor( "white" ), QColor( "black" ) );
    grid->addMultiCellWidget(m_vGradient, 2, 2, 0, 0);

    m_whiteBalanceCurvesWidget = new Digikam::CurvesWidget(256, 256, imageData, width, height, 
                                                           m_whiteBalanceCurves, frame, true);
    QWhatsThis::add( m_whiteBalanceCurvesWidget, i18n("<p>This is the curve drawing of the selected image "
                                                      "histogram channel"));
    l->addWidget(m_whiteBalanceCurvesWidget, 0);
    grid->addMultiCellWidget(frame, 2, 2, 1, 5);
    
    m_hGradient = new Digikam::ColorGradientWidget( KSelector::Horizontal, 20, gbox );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    grid->addMultiCellWidget(m_hGradient, 3, 3, 1, 5);
    
    QLabel *label3 = new QLabel( i18n("Target Color:"), gbox );
    m_targetColor = new QComboBox( false, gbox );
    m_targetColor->insertItem( i18n("White") );
    m_targetColor->insertItem( i18n("Black") );
    QWhatsThis::add( m_targetColor, i18n("<p>Select here the target balance color to correct."));

    grid->addMultiCellWidget(label3, 4, 4, 1, 2);
    grid->addMultiCellWidget(m_targetColor, 4, 4, 3, 5);
                
    QLabel *label4 = new QLabel( i18n("Foreground Color:"), gbox );
    m_foregroundColorButton = new KColorButton( QColor::QColor( 255, 255, 255 ), gbox );
    QWhatsThis::add( m_foregroundColorButton, i18n("<p>Set here the foreground color from original image to correct black-white balance."));
        
    grid->addMultiCellWidget(label4, 5, 5, 1, 2);
    grid->addMultiCellWidget(m_foregroundColorButton, 5, 5, 3, 5);
    
    topLayout->addMultiCellWidget(gbox, 1, 1, 0, 0);

    // -------------------------------------------------------------

    QVGroupBox *gbox4 = new QVGroupBox(i18n("Image Preview"), plainPage());

    QLabel *label5 = new QLabel(i18n("Original:"), gbox4);
    label5->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    QFrame *frame2 = new QFrame(gbox4);
    frame2->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l2  = new QVBoxLayout(frame2, 5, 0);
    m_previewOriginalWidget = new Digikam::ImageGuideWidget(240, 160, frame2, true, Digikam::ImageGuideWidget::PickColorMode);
    QWhatsThis::add( m_previewOriginalWidget, i18n("<p>You can see here the original image. Click on image to select "
                                                   "the foreground color to use for image's white-balance adjustments"));
    l2->addWidget(m_previewOriginalWidget, 0, Qt::AlignCenter);

    QLabel *label6 = new QLabel(i18n("Target:"), gbox4);
    label6->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    QFrame *frame3 = new QFrame(gbox4);
    frame3->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l3  = new QVBoxLayout(frame3, 5, 0);
    m_previewTargetWidget = new Digikam::ImageWidget(240, 160, frame3);
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
    
    connect(m_targetColor, SIGNAL(activated(int)),
            this, SLOT(slotEffect())); 
               
    connect(m_foregroundColorButton, SIGNAL(changed(const QColor &)),
            this, SLOT(slotEffect()));
            
    connect(m_previewOriginalWidget, SIGNAL(crossCenterColorChanged( const QColor & )),
            this, SLOT(slotColorSelectedFromImage( const QColor & )));            
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
                   
    m_whiteBalanceCurves->curvesReset();
    m_targetColor->setCurrentItem(WhiteColor);
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

void ImageEffect_WhiteBalance::slotColorSelectedFromImage( const QColor &color )
{
    m_foregroundColorButton->setColor(color);
    slotEffect();  
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

    uint*  data = iface->getPreviewData();
    int    w    = iface->previewWidth();
    int    h    = iface->previewHeight();
    int    t    = m_targetColor->currentItem();
    QColor f    = m_foregroundColorButton->color();

    bwBalance(data, w, h, t, f);
    m_whiteBalanceCurvesWidget->repaint(false);
    
    iface->putPreviewData(data);       
    delete [] data;
    m_previewTargetWidget->update();
}

void ImageEffect_WhiteBalance::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
        
    uint*  data = iface.getOriginalData();
    int    w    = iface.originalWidth();
    int    h    = iface.originalHeight();
    int    t    = m_targetColor->currentItem();
    QColor f    = m_foregroundColorButton->color();

    bwBalance(data, w, h, t, f);

    iface.putOriginalData(i18n("Black-White Balance"), data);                   
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

// This code is based on tutorial from 'lasm' available at http://www.geocities.com/lasm.rm/wb2.html

uchar ImageEffect_WhiteBalance::curvePoint(int target, uchar channel, int i)
{
    double point;
    
    if (target == WhiteColor)
       point = QMAX( QMIN( floor((255.0 / channel) * i), 255) , 0 );
    else
       point = QMAX( QMIN( floor((255.0 / (255.0 - channel)) * (i - channel)), 255) , 0);
    
    return((uchar)point);
}

void ImageEffect_WhiteBalance::bwBalance(uint *data, int w, int h, int tColor, QColor fColor)
{
    uchar red   = fColor.red();
    uchar green = fColor.green();
    uchar blue  = fColor.blue();
    
    uchar* pOutBits = new uchar[w*h*4];    

    for (int i = 0 ; i < 256; i++)
       {
       m_whiteBalanceCurves->setCurveValue(Digikam::ImageHistogram::RedChannel, i, curvePoint(tColor, red, i));
       m_whiteBalanceCurves->setCurveValue(Digikam::ImageHistogram::GreenChannel, i, curvePoint(tColor, green, i));
       m_whiteBalanceCurves->setCurveValue(Digikam::ImageHistogram::BlueChannel, i, curvePoint(tColor, blue, i));
       }
       
    m_whiteBalanceCurves->curvesCalculateCurve(Digikam::ImageHistogram::AlphaChannel);
    m_whiteBalanceCurves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);
    m_whiteBalanceCurves->curvesLutProcess(data, (uint *)pOutBits, w, h);
    
    memcpy (data, pOutBits, w*h*4);           
       
    delete [] pOutBits;
}

}  // NameSpace DigikamWhiteBalanceImagesPlugin

#include "imageeffect_whitebalance.moc"
