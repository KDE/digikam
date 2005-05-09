/* ============================================================
 * File  : imageeffect_oilpaint.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-25
 * Description : a Digikam image editor plugin for to simulate 
 *               an oil painting.
 * 
 * Copyright 2004-2005 by Gilles Caulier
 *
 * Original OilPaint algorithm copyrighted 2004 by 
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

// Qt includes.

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qslider.h>
#include <qspinbox.h>
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
#include "imageeffect_oilpaint.h"

namespace DigikamOilPaintImagesPlugin
{

ImageEffect_OilPaint::ImageEffect_OilPaint(QWidget* parent)
                    : KDialogBase(Plain, i18n("Oil Paint"),
                                  Help|User1|Ok|Cancel, Ok,
                                  parent, 0, true, true, i18n("&Reset Values")),
                      m_parent(parent)
{
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    m_cancel = false;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Oil Paint"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("An oil painting image effect plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Oil paint algorithm"), 
                     "pieter_voloshyn at ame.com.br");                     
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Oil Paint Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Oil Paint"), headerFrame, "labelTitle" );
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

    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label1 = new QLabel(i18n("Brush size:"), plainPage());
    
    m_brushSizeSlider = new QSlider(1, 5, 1, 1, Qt::Horizontal, plainPage(), "m_brushSizeSlider");
    m_brushSizeSlider->setTickmarks(QSlider::Below);
    m_brushSizeSlider->setTickInterval(1);
    m_brushSizeSlider->setTracking ( false );
    
    m_brushSizeInput = new QSpinBox(1, 5, 1, plainPage(), "m_brushSizeInput");
    m_brushSizeInput->setValue(1);
        
    whatsThis = i18n("<p>Set here the brush size to use for simulating the oil painting.");
    QWhatsThis::add( m_brushSizeInput, whatsThis);
    QWhatsThis::add( m_brushSizeSlider, whatsThis);
    
    hlay2->addWidget(label1, 1);
    hlay2->addWidget(m_brushSizeSlider, 3);
    hlay2->addWidget(m_brushSizeInput, 1);
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay3 = new QHBoxLayout(topLayout);
    QLabel *label2 = new QLabel(i18n("Smooth:"), plainPage());
    
    m_smoothSlider = new QSlider(10, 255, 1, 30, Qt::Horizontal, plainPage(), "m_smoothSlider");
    m_smoothSlider->setTickmarks(QSlider::Below);
    m_smoothSlider->setTickInterval(20);
    m_smoothSlider->setTracking ( false );  
    
    m_smoothInput = new QSpinBox(10, 255, 1, plainPage(), "m_SmoothInput");
    m_smoothInput->setValue(30);
        
    whatsThis = i18n("<p>This value controls the smoothing effect of the brush under the canvas.");
    QWhatsThis::add( m_smoothInput, whatsThis);
    QWhatsThis::add( m_smoothSlider, whatsThis);                     
    
    hlay3->addWidget(label2, 1);
    hlay3->addWidget(m_smoothSlider, 3);
    hlay3->addWidget(m_smoothInput, 1);
    
    // -------------------------------------------------------------

    adjustSize();
    disableResize();
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
                
    // -------------------------------------------------------------
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_brushSizeSlider, SIGNAL(valueChanged(int)),
            m_brushSizeInput, SLOT(setValue(int)));
    connect(m_brushSizeInput, SIGNAL(valueChanged(int)),
            m_brushSizeSlider, SLOT(setValue(int)));            
    connect(m_brushSizeInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));            
            
    connect(m_smoothSlider, SIGNAL(valueChanged(int)),
            m_smoothInput, SLOT(setValue(int)));
    connect(m_smoothInput, SIGNAL(valueChanged(int)),
            m_smoothSlider, SLOT(setValue(int)));   
    connect(m_smoothInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));         
}

ImageEffect_OilPaint::~ImageEffect_OilPaint()
{
}

void ImageEffect_OilPaint::slotUser1()
{
    m_brushSizeInput->blockSignals(true);
    m_brushSizeSlider->blockSignals(true);
    m_smoothInput->blockSignals(true);
    m_smoothSlider->blockSignals(true);

    m_brushSizeInput->setValue(1);
    m_brushSizeSlider->setValue(1);
    m_smoothInput->setValue(30);
    m_smoothSlider->setValue(30);
        
    m_brushSizeInput->blockSignals(false);
    m_brushSizeSlider->blockSignals(false);
    m_smoothInput->blockSignals(false);
    m_smoothSlider->blockSignals(false);
    slotEffect();
} 

void ImageEffect_OilPaint::slotHelp()
{
    KApplication::kApplication()->invokeHelp("oilpaint", "digikamimageplugins");
}

void ImageEffect_OilPaint::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_OilPaint::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_OilPaint::slotEffect()
{
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    m_imagePreviewWidget->setEnable(false);
    m_brushSizeInput->setEnabled(false);
    m_brushSizeSlider->setEnabled(false);
    m_smoothInput->setEnabled(false);
    m_smoothSlider->setEnabled(false);
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    uint* data = (uint *)image.bits();
    int   w    = image.width();
    int   h    = image.height();
    int   b    = m_brushSizeSlider->value();
    int   s    = m_smoothSlider->value();
        
    m_imagePreviewWidget->setProgress(0);
    OilPaint(data, w, h, b, s);
    
    if (m_cancel) return;
    
    m_imagePreviewWidget->setProgress(0); 
    m_brushSizeInput->setEnabled(true);
    m_brushSizeSlider->setEnabled(true);
    m_smoothInput->setEnabled(true);
    m_smoothSlider->setEnabled(true);
    m_imagePreviewWidget->setPreviewImageData(image);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_imagePreviewWidget->setEnable(true);
}

void ImageEffect_OilPaint::slotOk()
{
    m_brushSizeInput->setEnabled(false);
    m_brushSizeSlider->setEnabled(false);
    m_smoothInput->setEnabled(false);
    m_smoothSlider->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
    
    uint* data = iface.getOriginalData();
    int   w    = iface.originalWidth();
    int   h    = iface.originalHeight();
    int   b    = m_brushSizeSlider->value();
    int   s    = m_smoothSlider->value();
        
    m_imagePreviewWidget->setProgress(0);
    OilPaint(data, w, h, b, s);
    
    if ( !m_cancel )
       iface.putOriginalData(i18n("Oilpaint"), data);
       
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to apply the OilPaint effect.                
 *                                                                                    
 * data             => The image data in RGBA mode.                            
 * w                => Width of image.                          
 * h                => Height of image.                          
 * BrushSize        => Brush size.
 * Smoothness       => Smooth value.                                                
 *                                                                                  
 * Theory           => Using MostFrequentColor function we take the main color in  
 *                     a matrix and simply write at the original position.            
 */                                                                                 
    
void ImageEffect_OilPaint::OilPaint(uint* data, int w, int h, int BrushSize, int Smoothness)
{
    int LineWidth = w * 4;
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
      
    uchar* newBits = (uchar*)data;
    int i = 0;
    uint color;
    
    for (int h2 = 0; !m_cancel && (h2 < h); h2++)
       {
       for (int w2 = 0; !m_cancel && (w2 < w); w2++)
          {
          i = h2 * LineWidth + 4*w2;
          color = MostFrequentColor ((uchar*)data, w, h, w2, h2, BrushSize, Smoothness);
          
          newBits[i+3] = (uchar)(color >> 24);
          newBits[i+2] = (uchar)(color >> 16);
          newBits[i+1] = (uchar)(color >> 8);
          newBits[ i ] = (uchar)(color);
          }
       
       // Update de progress bar in dialog.
       m_imagePreviewWidget->setProgress((int) (((double)h2 * 100.0) / h));  
       kapp->processEvents();          
       }
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to determine the most frequent color in a matrix                        
 *                                                                                
 * Bits             => Bits array                                                    
 * Width            => Image width                                                   
 * Height           => Image height                                                 
 * X                => Position horizontal                                           
 * Y                => Position vertical                                            
 * Radius           => Is the radius of the matrix to be analized                  
 * Intensity        => Intensity to calcule                                         
 *                                                                                  
 * Theory           => This function creates a matrix with the analized pixel in   
 *                     the center of this matrix and find the most frequenty color   
 */

uint ImageEffect_OilPaint::MostFrequentColor (uchar* Bits, int Width, int Height, int X, 
                                              int Y, int Radius, int Intensity)
{
    int i, w, h, I;
    uint color;
    
    double Scale = Intensity / 255.0;
    int LineWidth = 4 * Width;
    
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);   // Don't take off this step
        
    // Alloc some arrays to be used
    uchar *IntensityCount = new uchar[(Intensity + 1) * sizeof (uchar)];
    uint  *AverageColorR  = new uint[(Intensity + 1)  * sizeof (uint)];
    uint  *AverageColorG  = new uint[(Intensity + 1)  * sizeof (uint)];
    uint  *AverageColorB  = new uint[(Intensity + 1)  * sizeof (uint)];

    // Erase the array
    memset(IntensityCount, 0, (Intensity + 1) * sizeof (uchar));

    for (w = X - Radius; w <= X + Radius; w++)
        {
        for (h = Y - Radius; h <= Y + Radius; h++)
            {
            // This condition helps to identify when a point doesn't exist
            
            if ((w >= 0) && (w < Width) && (h >= 0) && (h < Height))
                {
                // You'll see a lot of times this formula
                i = h * LineWidth + 4 * w;
                I = (uint)(GetIntensity (Bits[i+2], Bits[i+1], Bits[i]) * Scale);
                IntensityCount[I]++;

                if (IntensityCount[I] == 1)
                    {
                    AverageColorR[I] = Bits[i+2];
                    AverageColorG[I] = Bits[i+1];
                    AverageColorB[I] = Bits[ i ];
                    }
                else
                    {
                    AverageColorR[I] += Bits[i+2];
                    AverageColorG[I] += Bits[i+1];
                    AverageColorB[I] += Bits[ i ];
                    }
                }
            }
        }

    I = 0;
    int MaxInstance = 0;

    for (i = 0 ; i <= Intensity ; i++)
       {
       if (IntensityCount[i] > MaxInstance)
          {
          I = i;
          MaxInstance = IntensityCount[i];
          }
       }

    int R, G, B;
    R = AverageColorR[I] / MaxInstance;
    G = AverageColorG[I] / MaxInstance;
    B = AverageColorB[I] / MaxInstance;
    color = qRgb (R, G, B);

    delete [] IntensityCount;        // free all the arrays
    delete [] AverageColorR;
    delete [] AverageColorG;
    delete [] AverageColorB;

    return (color);                    // return the most frequenty color
}

}  // NameSpace DigikamOilPaintImagesPlugin

#include "imageeffect_oilpaint.moc"
