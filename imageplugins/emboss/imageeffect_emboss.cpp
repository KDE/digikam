/* ============================================================
 * File  : imageeffect_emboss.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a Digikam image editor plugin for to emboss 
 *               an image.
 * 
 * Copyright 2004 by Gilles Caulier
 * 
 * Emboss algorithm copyrighted by Daniel M. Duley.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#define MagickPI      3.14159265358979323846264338327950288419716939937510
#define MagickSQ2PI   2.50662827463100024161235523934010416269302368164062
#define MagickEpsilon 1.0e-12
 
// Imlib2 include.

#define X_DISPLAY_MISSING 1
#include <Imlib2.h>

// C++ include.

#include <cstring>
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qslider.h>
#include <qlayout.h>
#include <qframe.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kimageeffect.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <kdebug.h>

// Digikam includes.

#include <digikam/imageiface.h>
#include <digikam/imagepreviewwidget.h>

// Local includes.

#include "version.h"
#include "imageeffect_emboss.h"

namespace DigikamEmbossImagesPlugin
{

ImageEffect_Emboss::ImageEffect_Emboss(QWidget* parent)
                  : KDialogBase(Plain, i18n("Emboss Image"),
                                Help|User1|Ok|Cancel, Ok,
                                parent, 0, true, true, i18n("&Reset values")),
                    m_parent(parent)
{
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    m_cancel = false;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Emboss Image"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("An embossed image effect plugin for Digikam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Gilles Caulier", 
                                       0,
                                       "http://digikam.sourceforge.net");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Oil paint algorithm"), 
                     "pieter_voloshyn at ame.com.br");         
                                          
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Emboss Image handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left", "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePreviewWidget(240, 160, 
                                                           i18n("Emboss image preview"),
                                                           plainPage());
    hlay1->addWidget(m_imagePreviewWidget);
    
    // -------------------------------------------------------------
    
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label1 = new QLabel(i18n("Radius:"), plainPage());
    
    m_radiusSlider = new QSlider(1, 100, 1, 10, Qt::Horizontal, plainPage(), "m_radiusSlider");
    m_radiusSlider->setTickmarks(QSlider::Below);
    m_radiusSlider->setTickInterval(10);
    m_radiusSlider->setTracking ( false );
    
    m_radiusInput = new KDoubleSpinBox(0.1, 10.0, 0.1, 1.0, 1, plainPage(), "m_radiusInput");
    whatsThis = i18n("<p>Set here the radius of the Gaussian, not counting the center pixel.");
        
    QWhatsThis::add( m_radiusInput, whatsThis);
    QWhatsThis::add( m_radiusSlider, whatsThis);

    hlay->addWidget(label1, 1);
    hlay->addWidget(m_radiusSlider, 3);
    hlay->addWidget(m_radiusInput, 1);
    
    // -------------------------------------------------------------
    
    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label2 = new QLabel(i18n("Sigma:"), plainPage());
    
    m_sigmaSlider = new QSlider(1, 100, 1, 10, Qt::Horizontal, plainPage(), "m_sigmaSlider");
    m_sigmaSlider->setTickmarks(QSlider::Below);
    m_sigmaSlider->setTickInterval(10);
    m_sigmaSlider->setTracking ( false );
    
    m_sigmaInput = new KDoubleSpinBox(0.1, 10.0, 0.1, 1.0, 1, plainPage(), "m_sigmaInput");
    whatsThis = i18n("<p>Set here the standard deviation of the Gaussian.");
    
    QWhatsThis::add( m_sigmaSlider, whatsThis);
    QWhatsThis::add( m_sigmaInput, whatsThis);
                 
    hlay2->addWidget(label2, 1);
    hlay2->addWidget(m_sigmaSlider, 3);
    hlay2->addWidget(m_sigmaInput, 1);
    
    // -------------------------------------------------------------
        
    QHBoxLayout *hlay6 = new QHBoxLayout(topLayout);
    m_progressBar = new KProgress(100, plainPage(), "progressbar");
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    hlay6->addWidget(m_progressBar, 1);

    // -------------------------------------------------------------
    
    adjustSize();
    slotUser1();    // Reset all parameters to the default values.
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_radiusSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderRadiusChanged(int)));
    connect(m_radiusInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotSpinBoxRadiusChanged(double)));            
    connect(m_radiusInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));   

    connect(m_sigmaSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderSigmaChanged(int)));
    connect(m_sigmaInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotSpinBoxSigmaChanged(double)));            
    connect(m_sigmaInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));
}

ImageEffect_Emboss::~ImageEffect_Emboss()
{
}

void ImageEffect_Emboss::slotUser1()
{
    blockSignals(true);
    m_radiusInput->setValue(3);
    m_radiusSlider->setValue(3);
    m_sigmaInput->setValue(2);
    m_sigmaSlider->setValue(2);
    slotEffect();
    blockSignals(false);
} 

void ImageEffect_Emboss::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_Emboss::slotSliderRadiusChanged(int v)
{
    blockSignals(true);
    m_radiusInput->setValue((double)v/10.0);
    blockSignals(false);
}

void ImageEffect_Emboss::slotSpinBoxRadiusChanged(double v)
{
    blockSignals(true);
    m_radiusSlider->setValue((int)(v*10.0));
    blockSignals(false);
}

void ImageEffect_Emboss::slotSliderSigmaChanged(int v)
{
    blockSignals(true);
    m_sigmaInput->setValue((double)v/10.0);
    blockSignals(false);
}

void ImageEffect_Emboss::slotSpinBoxSigmaChanged(double v)
{
    blockSignals(true);
    m_sigmaSlider->setValue((int)(v*10.0));
    blockSignals(false);
}

void ImageEffect_Emboss::slotHelp()
{
    KApplication::kApplication()->invokeHelp("emboss",
                                             "digikamimageplugins");
}

void ImageEffect_Emboss::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_Emboss::slotEffect()
{
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    uint* data = (uint *)image.bits();
    int   w    = image.width();
    int   h    = image.height();
    double radius = m_radiusInput->value();
    //double sigma  = m_sigmaInput->value();
            
    m_progressBar->setValue(0); 
    Emboss(data, w, h, (float)radius);
    
    if (m_cancel) return;
    
    m_progressBar->setValue(0);  
    memcpy(image.bits(), (uchar *)data, image.numBytes());
    m_imagePreviewWidget->setPreviewImageData(image);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
}

void ImageEffect_Emboss::slotOk()
{
    enableButton(Ok, false);
    enableButton(User1, false);
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
        
    uint* data    = iface.getOriginalData();
    int w         = iface.originalWidth();
    int h         = iface.originalHeight();
    double radius = m_radiusInput->value();
    //double sigma  = m_sigmaInput->value();
    
    m_progressBar->setValue(0);
    Emboss(data, w, h, (float)radius);
        
    if ( !m_cancel )
       iface.putOriginalData(data);
       
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to apply the Emboss effect                                             
 *                                                                                  
 * PicDestDC        => Destiny PictureBox's Device Context                          
 * PicSrcDC            => Source PictureBox's Device Context                         
 * Depth            => Emboss value                                                  
 *                                                                                
 * Theory            => This is an amazing effect. And the theory is very simple to 
 *                    understand. You get the diference between the colors and    
 *                    increase it. After this, get the gray tone            
 */

void ImageEffect_Emboss::Emboss(uint* data, int w, int h, float Depth)
{
    int LineWidth = w * 4;
    
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);

    uint*  newData = new uint[w*h*4];
    uchar* newBits = (uchar*) newData;
    uchar* bits    = (uchar*) data;
    
    int i = 0, j = 0;
    int R = 0, G = 0, B = 0;
    uchar Gray = 0;
    
    for (int h2 = 0 ; !m_cancel && (h2 < h) ; ++h2)
        {
        for (int w2 = 0 ; !m_cancel && (w2 < w) ; ++w2)
            {
            i = h2 * LineWidth + 4*w2;
            j = (h2 + Lim_Max (h2, 1, h)) * LineWidth + 4 * (w2 + Lim_Max (w2, 1, w));

            R = abs ((int)((bits[ i ] - bits[ j ]) * Depth + 128));
            G = abs ((int)((bits[i+1] - bits[j+1]) * Depth + 128));
            B = abs ((int)((bits[i+2] - bits[j+2]) * Depth + 128));

            Gray = LimitValues ((uint) ((double)(R + G + B) / 3.0));
            
            //kdWarning() << Gray << endl;

            if ( i < w*h*4){ 
            newBits[i+2] = Gray;
            newBits[i+1] = Gray;
            newBits[ i ] = Gray;
            }
            else
              kdWarning() << "pipo" << endl;
            }
            
       // Update de progress bar in dialog.
       m_progressBar->setValue((int) (((double)h2 * 100.0) / h));
       kapp->processEvents();          
       }   
            
    memcpy(data, newData, w*h*4);             
    delete [] newData;
    }

 // This method have been ported from Pieter Z. Voloshyn algorithm code.   
    
  /* This function limits the max and min values     
 * defined by the developer                                    
 *                                                                              
 * Now                => Original value                                      
 * Up                => Increments                                              
 * Max                => Maximum value                                          
 *                                                                                  
 * Theory            => This function is used in some functions to limit the        
 *                    "for step". E.g. I have a picture with 309 pixels (width), and  
 *                    my "for step" is 5. All the code go alright until reachs the  
 *                    w = 305, because in the next step w will go to 310, but we want  
 *                    to analize all the pixels. So, this function will reduce the 
 *                    "for step", when necessary, until reach the last possible value  
 */
 
int ImageEffect_Emboss::Lim_Max (int Now, int Up, int Max)
{
    Max--;
    while (Now > Max - Up)
        Up--;
    return (Up);
}    

// This method have been ported from Pieter Z. Voloshyn algorithm code.  
 
/* This function limits the RGB values                        
 *                                                                         
 * ColorValue        => Here, is an RGB value to be analized                   
 *                                                                             
 * Theory            => A color is represented in RGB value (e.g. 0xFFFFFF is     
 *                    white color). But R, G and B values has 256 values to be used   
 *                    so, this function analize the value and limits to this range   
 */   
                     
uchar ImageEffect_Emboss::LimitValues (int ColorValue)
{
    if (ColorValue > 255)        // MAX = 255
        ColorValue = 255;        
    if (ColorValue < 0)            // MIN = 0
        ColorValue = 0;
    return ((uchar) ColorValue);
}
    


/*
QImage ImageEffect_Emboss::Emboss(QImage &image, double radius, double sigma)
{
    double alpha;
    int j, width;
    register long i, u, v;
    QImage dest;

    width = getOptimalKernelWidth(radius, sigma);
    
    double* kernel = new double[width*width*sizeof(double)];
    
    i = 0;
    j = width/2;
    
    for (v=(-width/2) ; !m_cancel && (v <= (width/2)) ; ++v)
       {
       for (u=(-width/2) ; u <= (width/2) ; ++u)
          {
          alpha = exp(-((double) u*u+v*v)/(2.0*sigma*sigma));
          kernel[i] = ((u < 0) || (v < 0) ? -8.0 : 8.0) * alpha/(2.0 * MagickPI * sigma * sigma);
          if (u == j) kernel[i]=0.0;
          ++i;
          }
       
       --j;
        
       // Update de progress bar in dialog.
       m_progressBar->setValue((int) (((double)(v + (width/2)) * 100.0) / width));
       kapp->processEvents();  
       }
        
    convolveImage(&image, &dest, width, kernel);
    delete [] kernel;
    if (m_cancel) return(image);
    
    KImageEffect::equalize(dest);
    return(dest);
}


int ImageEffect_Emboss::getOptimalKernelWidth(double radius, double sigma)
{
    double normalize, value;
    long width;
    register long u;

    if(radius > 0.0)
        return((int)(2.0*ceil(radius)+1.0));
        
    for(width=5; ;){
        normalize=0.0;
        for(u=(-width/2); u <= (width/2); u++)
            normalize+=exp(-((double) u*u)/(2.0*sigma*sigma))/(MagickSQ2PI*sigma);
        u=width/2;
        value=exp(-((double) u*u)/(2.0*sigma*sigma))/(MagickSQ2PI*sigma)/normalize;
        if((long)(65535*value) <= 0)
            break;
        width+=2;
    }
    return((int)width-2);
}

bool ImageEffect_Emboss::convolveImage(QImage *image, QImage *dest,
                                       const unsigned int order,
                                       const double *kernel)
{
    long width;
    double red, green, blue, alpha;
    double normalize, *normal_kernel;
    register const double *k;
    register unsigned int *q;
    int x, y, mx, my, sx, sy;
    long i;
    int mcx, mcy;

    width = order;
    if((width % 2) == 0){
        qWarning("KImageEffect: Kernel width must be an odd number!");
        return(false);
    }
    normal_kernel = new double[width*width*sizeof(double)];
    
    if(!normal_kernel){
        qWarning("KImageEffect: Unable to allocate memory!");
        return(false);
    }
    dest->reset();
    dest->create(image->width(), image->height(), 32);
    if(image->depth() < 32)
        *image = image->convertDepth(32);

    normalize=0.0;
    for(i=0; i < (width*width); i++)
        normalize += kernel[i];
    if(fabs(normalize) <= MagickEpsilon)
        normalize=1.0;
    normalize=1.0/normalize;
    for(i=0; i < (width*width); i++)
        normal_kernel[i] = normalize*kernel[i];

    unsigned int **jumpTable = (unsigned int **)image->jumpTable();
    for(y=0; y < dest->height(); ++y){
        sy = y-(width/2);
        q = (unsigned int *)dest->scanLine(y);
        for(x=0; x < dest->width(); ++x){
            k = normal_kernel;
            red = green = blue = alpha = 0;
            sy = y-(width/2);
            for(mcy=0; mcy < width; ++mcy, ++sy){
                my = sy < 0 ? 0 : sy > image->height()-1 ?
                    image->height()-1 : sy;
                sx = x+(-width/2);
                for(mcx=0; mcx < width; ++mcx, ++sx){
                    mx = sx < 0 ? 0 : sx > image->width()-1 ?
                        image->width()-1 : sx;
                    red += (*k)*(qRed(jumpTable[my][mx])*257);
                    green += (*k)*(qGreen(jumpTable[my][mx])*257);
                    blue += (*k)*(qBlue(jumpTable[my][mx])*257);
                    alpha += (*k)*(qAlpha(jumpTable[my][mx])*257);
                    ++k;
                }
            }

            red = red < 0 ? 0 : red > 65535 ? 65535 : red+0.5;
            green = green < 0 ? 0 : green > 65535 ? 65535 : green+0.5;
            blue = blue < 0 ? 0 : blue > 65535 ? 65535 : blue+0.5;
            alpha = alpha < 0 ? 0 : alpha > 65535 ? 65535 : alpha+0.5;

            *q++ = qRgba((unsigned char)(red/257UL),
                         (unsigned char)(green/257UL),
                         (unsigned char)(blue/257UL),
                         (unsigned char)(alpha/257UL));
        }
    }
    delete [] normal_kernel;
    return(true);
}
*/

}  // NameSpace DigikamEmbossImagesPlugin

#include "imageeffect_emboss.moc"
