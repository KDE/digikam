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
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcombobox.h>

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

ImageEffect_WhiteBalance::ImageEffect_WhiteBalance(QWidget* parent)
                     : KDialogBase(Plain, i18n("White Balance"),
                                   Help|User1|Ok|Cancel, Ok,
                                   parent, 0, true, true,
                                   i18n("&Reset Values")),
                       m_parent(parent)
{
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all parameters to the default values.") );
    
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

    QGridLayout* topLayout = new QGridLayout( plainPage(), 4, 4 , marginHint(), spacingHint());

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
    topLayout->addMultiCellWidget(headerFrame, 0, 0, 0, 4);

    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    // -------------------------------------------------------------
    
    QHGroupBox *gbox = new QHGroupBox(i18n("Image Preview"), plainPage());

    QLabel *label1 = new QLabel(i18n("Original:"), gbox);
    label1->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    QFrame *frame2 = new QFrame(gbox);
    frame2->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l2  = new QVBoxLayout(frame2, 5, 0);
    m_previewOriginalWidget = new Digikam::ImageGuideWidget(320, 240, frame2, true, Digikam::ImageGuideWidget::PickColorMode);
    QWhatsThis::add( m_previewOriginalWidget, i18n("<p>You can see here the original image."));
    l2->addWidget(m_previewOriginalWidget, 0, Qt::AlignCenter);

    QLabel *label2 = new QLabel(i18n("Target:"), gbox);
    label2->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    QFrame *frame3 = new QFrame(gbox);
    frame3->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l3  = new QVBoxLayout(frame3, 5, 0);
    m_previewTargetWidget = new Digikam::ImageWidget(320, 240, frame3);
    QWhatsThis::add( m_previewTargetWidget, i18n("<p>You can see here the image's white balance adjustments preview."));
    l3->addWidget(m_previewTargetWidget, 0, Qt::AlignCenter);

    topLayout->addMultiCellWidget(gbox, 1, 1, 0, 4);
    
    // -------------------------------------------------------------
    
    QLabel *label3 = new QLabel(i18n("Target Color:"), plainPage());
    m_targetColor = new QComboBox( false, plainPage() );
    m_targetColor->insertItem( i18n("White") );
    m_targetColor->insertItem( i18n("Black") );
    QWhatsThis::add( m_targetColor, i18n("<p>Select here the target balance color to correct."));

    topLayout->addMultiCellWidget(label3, 2, 2, 0, 1);
    topLayout->addMultiCellWidget(m_targetColor, 2, 2, 2, 4);
                
    QLabel *label4 = new QLabel(i18n("Foreground Color:"), plainPage());
    m_foregroundColorButton = new KColorButton( QColor::QColor( 255, 255, 255 ), plainPage() );
    QWhatsThis::add( m_foregroundColorButton, i18n("<p>Set here the foreground color from original image to correct black-white balance."));
        
    topLayout->addMultiCellWidget(label4, 3, 3, 0, 1);
    topLayout->addMultiCellWidget(m_foregroundColorButton, 3, 3, 2, 4);
        
    adjustSize();
    disableResize();

    // -------------------------------------------------------------
 
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

void ImageEffect_WhiteBalance::slotUser1()
{
    blockSignals(true);
                   
    // TODO
    
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

void ImageEffect_WhiteBalance::slotEffect()
{
    Digikam::ImageIface* iface = m_previewTargetWidget->imageIface();

    uint*  data = iface->getPreviewData();
    int    w    = iface->previewWidth();
    int    h    = iface->previewHeight();
    int    t    = m_targetColor->currentItem();
    QColor f    = m_foregroundColorButton->color();

    bwBalance(data, w, h, t, f);
    
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

    Digikam::ImageCurves *balanceCurves = new Digikam::ImageCurves();
    
    for (int i = 0 ; i < 256; i++)
       {
       balanceCurves->setCurveValue(Digikam::ImageHistogram::RedChannel, i, curvePoint(tColor, red, i));
       balanceCurves->setCurveValue(Digikam::ImageHistogram::GreenChannel, i, curvePoint(tColor, green, i));
       balanceCurves->setCurveValue(Digikam::ImageHistogram::BlueChannel, i, curvePoint(tColor, blue, i));
       }
       
    balanceCurves->curvesCalculateCurve(Digikam::ImageHistogram::AlphaChannel);
    balanceCurves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);
    balanceCurves->curvesLutProcess(data, (uint *)pOutBits, w, h);
    
    memcpy (data, pOutBits, w*h*4);           
       
    delete balanceCurves;
    delete [] pOutBits;
}

}  // NameSpace DigikamWhiteBalanceImagesPlugin

#include "imageeffect_whitebalance.moc"
