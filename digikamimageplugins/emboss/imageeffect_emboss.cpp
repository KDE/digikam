/* ============================================================
 * File  : imageeffect_emboss.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a Digikam image editor plugin for to emboss 
 *               an image.
 * 
 * Copyright 2004-2005 by Gilles Caulier
 *
 * Original Emboss algorithm copyrighted 2004 by 
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
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
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qlayout.h>
#include <qframe.h>
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

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imageeffect_emboss.h"

namespace DigikamEmbossImagesPlugin
{

ImageEffect_Emboss::ImageEffect_Emboss(QWidget* parent)
                  : KDialogBase(Plain, i18n("Emboss Image"),
                                Help|User1|Ok|Cancel, Ok,
                                parent, 0, true, true, i18n("&Reset Values")),
                    m_parent(parent)
{
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    m_cancel = false;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Emboss Image"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("An embossed image effect plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Emboss algorithm"), 
                     "pieter_voloshyn at ame.com.br");         
                                          
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Emboss Image Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Emboss Image"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addWidget(headerFrame);
    
    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePreviewWidget(240, 160, i18n("Preview"), plainPage(), true);
    hlay1->addWidget(m_imagePreviewWidget);
            
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setProgressWhatsThis(i18n("<p>This is the current percentage of the task completed."));
    
    // -------------------------------------------------------------
    
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label1 = new QLabel(i18n("Depth:"), plainPage());
    
    m_depthSlider = new QSlider(10, 300, 1, 30, Qt::Horizontal, plainPage(), "m_depthSlider");
    m_depthSlider->setTickmarks(QSlider::Below);
    m_depthSlider->setTickInterval(20);
    m_depthSlider->setTracking ( false );
    
    m_depthInput = new QSpinBox(10, 300, 1, plainPage(), "m_depthInput");
    m_depthInput->setValue(30);    
    whatsThis = i18n("<p>Set here the depth of the embossing image effect.");
        
    QWhatsThis::add( m_depthInput, whatsThis);
    QWhatsThis::add( m_depthSlider, whatsThis);

    hlay->addWidget(label1, 1);
    hlay->addWidget(m_depthSlider, 3);
    hlay->addWidget(m_depthInput, 1);
    
    // -------------------------------------------------------------
    
    adjustSize();
    disableResize(); 
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
              
    // -------------------------------------------------------------
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_depthSlider, SIGNAL(valueChanged(int)),
            m_depthInput, SLOT(setValue(int)));
    connect(m_depthSlider, SIGNAL(valueChanged(int)),
            m_depthSlider, SLOT(setValue(int)));            
    connect(m_depthInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect())); 
}

ImageEffect_Emboss::~ImageEffect_Emboss()
{
}

void ImageEffect_Emboss::slotUser1()
{
    m_depthInput->blockSignals(true);
    m_depthSlider->blockSignals(true);
    
    m_depthInput->setValue(30);
    m_depthSlider->setValue(30);
    
    m_depthInput->blockSignals(false);
    m_depthSlider->blockSignals(false);
    slotEffect();
} 

void ImageEffect_Emboss::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_Emboss::slotHelp()
{
    KApplication::kApplication()->invokeHelp("emboss", "digikamimageplugins");
}

void ImageEffect_Emboss::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_Emboss::slotEffect()
{
    m_imagePreviewWidget->setEnable(false);
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    m_depthInput->setEnabled(false);
    m_depthSlider->setEnabled(false);
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    uint* data  = (uint *)image.bits();
    int   w     = image.width();
    int   h     = image.height();
    int   depth = m_depthSlider->value();
            
    m_imagePreviewWidget->setProgress(0);
    Emboss(data, w, h, depth);
    
    if (m_cancel) return;
    
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageData(image);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_depthInput->setEnabled(true);
    m_depthSlider->setEnabled(true);
    m_imagePreviewWidget->setEnable(true);
}

void ImageEffect_Emboss::slotOk()
{
    m_depthInput->setEnabled(false);
    m_depthSlider->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
        
    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    int depth  = m_depthSlider->value();
    
    m_imagePreviewWidget->setProgress(0);
    Emboss(data, w, h, depth);
        
    if ( !m_cancel )
       iface.putOriginalData(i18n("Emboss"), data);
       
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to apply the Emboss effect                                             
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                          
 * d                => Emboss value                                                  
 *                                                                                
 * Theory           => This is an amazing effect. And the theory is very simple to 
 *                     understand. You get the diference between the colors and    
 *                     increase it. After this, get the gray tone            
 */

void ImageEffect_Emboss::Emboss(uint* data, int Width, int Height, int d)
{
    float Depth = d / 10.0;
    int LineWidth = Width * 4;
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);

    uchar *Bits = (uchar*) data;
    int    i = 0, j = 0;
    int    R = 0, G = 0, B = 0;
    uchar  Gray = 0;
    
    for (int h = 0 ; !m_cancel && (h < Height) ; ++h)
       {
       for (int w = 0 ; !m_cancel && (w < Width) ; ++w)
           {
           i = h * LineWidth + 4 * w;
           j = (h + Lim_Max (h, 1, Height)) * LineWidth + 4 * (w + Lim_Max (w, 1, Width));
               
           R = abs ((int)((Bits[i+2] - Bits[j+2]) * Depth + 128));
           G = abs ((int)((Bits[i+1] - Bits[j+1]) * Depth + 128));
           B = abs ((int)((Bits[ i ] - Bits[ j ]) * Depth + 128));

           Gray = CLAMP0255 ((R + G + B) / 3);
           
           Bits[i+2] = Gray;
           Bits[i+1] = Gray;
           Bits[ i ] = Gray;
           }
       
       // Update de progress bar in dialog.
       m_imagePreviewWidget->setProgress((int) (((double)h * 100.0) / Height));
       kapp->processEvents(); 
       }
}
       
// This method have been ported from Pieter Z. Voloshyn algorithm code.   
    
/* This function limits the max and min values     
 * defined by the developer                                    
 *                                                                              
 * Now               => Original value                                      
 * Up                => Increments                                              
 * Max               => Maximum value                                          
 *                                                                                  
 * Theory            => This function is used in some functions to limit the        
 *                      "for step". E.g. I have a picture with 309 pixels (width), and  
 *                      my "for step" is 5. All the code go alright until reachs the  
 *                      w = 305, because in the next step w will go to 310, but we want  
 *                      to analize all the pixels. So, this function will reduce the 
 *                      "for step", when necessary, until reach the last possible value  
 */
 
int ImageEffect_Emboss::Lim_Max (int Now, int Up, int Max)
{
    --Max;
    while (Now > Max - Up)
        --Up;
    return (Up);
}    
    
}  // NameSpace DigikamEmbossImagesPlugin

#include "imageeffect_emboss.moc"
