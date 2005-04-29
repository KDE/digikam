/* ============================================================
 * File  : imageeffect_infrared.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-22
 * Description : a digiKam image editor plugin for simulate 
 *               infrared film.
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

#define INT_MULT(a,b,t)  ((t) = (a) * (b) + 0x80, ((((t) >> 8) + (t)) >> 8)) 
 
// C++ include.

#include <cstring>
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlcdnumber.h>
#include <qslider.h>
#include <qlayout.h>
#include <qframe.h>
#include <qdatetime.h> 
#include <qtimer.h>
#include <qpoint.h>
#include <qcheckbox.h>

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
#include "imageeffect_infrared.h"

namespace DigikamInfraredImagesPlugin
{

ImageEffect_Infrared::ImageEffect_Infrared(QWidget* parent)
                     : KDialogBase(Plain, i18n("Infrared Film"),
                                   Help|User1|Ok|Cancel, Ok,
                                   parent, 0, true, true,
                                   i18n("&Reset Values")),
                       m_parent(parent)
{
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    m_cancel = false;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Infrared Film"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to simulate infrared film."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Infrared Film Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Simulate Infrared Film to Image"), headerFrame, "labelTitle" );
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
    m_addFilmGrain = new QCheckBox( i18n("Add film grain"), plainPage());
    m_addFilmGrain->setChecked( true );
    QWhatsThis::add( m_addFilmGrain, i18n("<p>This option is adding infrared film grain to the image depending on ISO-sensitivity."));
    hlay->addWidget(m_addFilmGrain, 1);
    
    // -------------------------------------------------------------
    
    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label1 = new QLabel(i18n("Film sensibility (ISO):"), plainPage());
    
    m_sensibilitySlider = new QSlider(1, 7, 1, 1, Qt::Horizontal, plainPage(), "m_sensibilitySlider");
    m_sensibilitySlider->setTracking ( false );
    m_sensibilitySlider->setTickInterval(1);
    m_sensibilitySlider->setTickmarks(QSlider::Below);
    
    m_sensibilityLCDValue = new QLCDNumber (3, plainPage(), "m_sensibilityLCDValue");
    m_sensibilityLCDValue->setSegmentStyle ( QLCDNumber::Flat );
    m_sensibilityLCDValue->display( QString::number(200) );
    whatsThis = i18n("<p>Set here the ISO-sensitivity of the simulated infrared film. "
                     "Increasing this value will increase the portion of green color in the mix. " 
                     "It will also increase the halo effect on the hightlights, and the film graininess (if the box is checked).");
        
    QWhatsThis::add( m_sensibilityLCDValue, whatsThis);
    QWhatsThis::add( m_sensibilitySlider, whatsThis);

    hlay2->addWidget(label1, 1);
    hlay2->addWidget(m_sensibilitySlider, 3);
    hlay2->addWidget(m_sensibilityLCDValue, 1);
    
    // -------------------------------------------------------------
    
    adjustSize();
    disableResize(); 
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
    
    // -------------------------------------------------------------
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect( m_sensibilitySlider, SIGNAL(valueChanged(int)),
             this, SLOT(slotSensibilityChanged(int)) ); 
             
    connect(m_addFilmGrain, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));                        
}

ImageEffect_Infrared::~ImageEffect_Infrared()
{
}

void ImageEffect_Infrared::slotUser1()
{
    m_sensibilitySlider->blockSignals(true);
    m_sensibilitySlider->setValue(1);
    m_sensibilitySlider->blockSignals(false);
    slotEffect();    
} 

void ImageEffect_Infrared::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_Infrared::slotHelp()
{
    KApplication::kApplication()->invokeHelp("infrared", "digikamimageplugins");
}

void ImageEffect_Infrared::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_Infrared::slotSensibilityChanged(int v)
{
    m_sensibilityLCDValue->display( QString::number(100 + 100 * v) );
    slotEffect();
}

void ImageEffect_Infrared::slotEffect()
{
    m_imagePreviewWidget->setEnable(false);
    m_addFilmGrain->setEnabled(false);
    m_sensibilitySlider->setEnabled(false);
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    uint* data   = (uint *)image.bits();
    int   w      = image.width();
    int   h      = image.height();
    int   s      = 100 + 100 * m_sensibilitySlider->value();
    bool  g      = m_addFilmGrain->isChecked();

    m_imagePreviewWidget->setProgress(0);
    infrared(data, w, h, s, g);
    
    if (m_cancel) return;
    
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageData(image);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_sensibilitySlider->setEnabled(true);
    m_addFilmGrain->setEnabled(true);
    m_imagePreviewWidget->setEnable(true);
}

void ImageEffect_Infrared::slotOk()
{
    m_addFilmGrain->setEnabled(false);
    m_sensibilitySlider->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
        
    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    int s      = 100 + 100 * m_sensibilitySlider->value();
    bool  g    = m_addFilmGrain->isChecked();
    
    m_imagePreviewWidget->setProgress(0);
    infrared(data, w, h, s, g);

    if ( !m_cancel )
       iface.putOriginalData(i18n("Infrared Film"), data);
       
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

// This method is based on the Simulate Infrared Film tutorial from GimpGuru.org web site 
// available at this url : http://www.gimpguru.org/Tutorials/SimulatedInfrared/

void ImageEffect_Infrared::infrared(uint* data, int Width, int Height, int Sensibility, bool Grain)
{
    if (Sensibility <= 0) return;
    
    // Infrared film variables depending of Sensibility.
    // This way try to reproduce famous Ilford SFX200 infrared film
    // http://www.ilford.com/html/us_english/prod_html/sfx200/sfx200.html
    // Note : this film have a sensibility escursion from 200 to 800 ISO.
    
    int        Noise = (int)((Sensibility + 3000.0) / 10.0); // Infrared film grain.
    int   blurRadius = (int)((Sensibility / 200.0) + 1.0);   // Gaussian blur infrared hightlight effect [2 to 5].
    float greenBoost = 2.1 - (Sensibility / 2000.0);         // Infrared green color boost [1.7 to 2.0].
    
    int   nStride = GetStride(Width);
    register int h, w, i = 0;       
    int nRand;

    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int        BitCount = LineWidth * Height;
    uchar*      pInBits = (uchar*)data;
    uchar*      pBWBits = new uchar[BitCount];    // Black and White conversion.
    uchar*  pBWBlurBits = new uchar[BitCount];    // Black and White with blur.
    uchar*   pGrainBits = new uchar[BitCount];    // Grain blured without curves adjustment.
    uchar*    pMaskBits = new uchar[BitCount];    // Grain mask with curves adjustment.
    uchar* pOverlayBits = new uchar[BitCount];    // Overlay to merge with original converted in gray scale.
    uchar*     pOutBits = new uchar[BitCount];    // Destination image with grain mask and original image merged.
    
    //------------------------------------------
    // 1 - Create GrayScale green boosted image.
    //------------------------------------------
    
    // Convert to gray scale with boosting Green channel. 
    // Infrared film increase green color.

    memcpy (pBWBits, pInBits, BitCount);  
    
    Digikam::ImageFilters::channelMixerImage((uint *)pBWBits, Width, Height, // Image data.
                                             true,                           // Preserve luminosity.    
                                             true,                           // Monochrome.
                                             0.4, greenBoost, -0.8,          // Red channel gains.
                                             0.0, 1.0,         0.0,          // Green channel gains (not used).
                                             0.0, 0.0,         1.0);         // Blue channel gains (not used).
    m_imagePreviewWidget->setProgress(10);
    kapp->processEvents(); 
    if (m_cancel) return;

    // Apply a Gaussian blur to the black and white image.
    // This way simulate Infrared film dispersion for the highlights.

    memcpy (pBWBlurBits, pBWBits, BitCount);  
        
    Digikam::ImageFilters::gaussianBlurImage((uint *)pBWBlurBits, Width, Height, blurRadius);
    m_imagePreviewWidget->setProgress(20);
    kapp->processEvents(); 
    if (m_cancel) return;

    //-----------------------------------------------------------------
    // 2 - Create Gaussian blured averlay mask with grain if necessary.
    //-----------------------------------------------------------------
    
    // Create gray grain mask.
    
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    srand ((uint) dt.secsTo(Y2000));
    
    i = 0;
    
    for (h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < Width); w++)
            {
            if (Grain)
               {
               nRand = (rand() % Noise) - (Noise / 2);
               pGrainBits[i++] = LimitValues (128 + nRand);    // Blue.
               pGrainBits[i++] = LimitValues (128 + nRand);    // Green.
               pGrainBits[i++] = LimitValues (128 + nRand);    // Red.
               pGrainBits[i++] = 0;                            // Reset Alpha (not used here).
               }
            }
        
        // Update progress bar in dialog.
        m_imagePreviewWidget->setProgress((int) (30.0 + ((double)h * 10.0) / Height));
        kapp->processEvents(); 
        }

    // Smooth grain mask using gaussian blur.    
   
    if (Grain)
       Digikam::ImageFilters::gaussianBlurImage((uint *)pGrainBits, Width, Height, 3);
    
    m_imagePreviewWidget->setProgress(50);
    kapp->processEvents(); 
    if (m_cancel) return;
        
    // Normally, film grain tends to be most noticable in the midtones, and much less 
    // so in the shadows and highlights. Adjust histogram curve to adjust grain like this. 
    
    if (Grain)
       {
       Digikam::ImageCurves *grainCurves = new Digikam::ImageCurves();
    
       // We modify only global luminosity of the grain.
       grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 0,  QPoint::QPoint(0,   0));   
       grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 8,  QPoint::QPoint(128, 128));
       grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 16, QPoint::QPoint(255, 0));
    
       // Calculate curves and lut to apply on grain.
       grainCurves->curvesCalculateCurve(Digikam::ImageHistogram::ValueChannel);
       grainCurves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);
       grainCurves->curvesLutProcess((uint *)pGrainBits, (uint *)pMaskBits, Width, Height);
       delete grainCurves;
       }
    
    m_imagePreviewWidget->setProgress(60);
    kapp->processEvents(); 
    if (m_cancel) return;
       
    // Merge gray scale image with grain using shade coefficient.

    int Shade = 32; // This value control the shading pixel effect between original image and grain mask.
    i = 0;
    
    for (h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < Width); w++)
            {        
            if (Grain)  // Merging grain.
               {
               pOverlayBits[i++] = (pBWBlurBits[i] * (255 - Shade) + pMaskBits[i] * Shade) >> 8;    // Blue.
               pOverlayBits[i++] = (pBWBlurBits[i] * (255 - Shade) + pMaskBits[i] * Shade) >> 8;    // Green.
               pOverlayBits[i++] = (pBWBlurBits[i] * (255 - Shade) + pMaskBits[i] * Shade) >> 8;    // Red.
               pOverlayBits[i++] = pBWBlurBits[i];                                                  // Alpha.
               }               
            else        // Use gray scale image without grain.
               {
               pOverlayBits[i++] = pBWBlurBits[i];    // Blue.
               pOverlayBits[i++] = pBWBlurBits[i];    // Green.
               pOverlayBits[i++] = pBWBlurBits[i];    // Red.
               pOverlayBits[i++] = pBWBlurBits[i];    // Alpha.
               }
            }
            
        // Update progress bar in dialog.
        m_imagePreviewWidget->setProgress((int) (70.0 + ((double)h * 10.0) / Height));
        kapp->processEvents(); 
        }
    
    //------------------------------------------
    // 3 - Merge Grayscale image & overlay mask.
    //------------------------------------------
    
    // Merge overlay and gray scale image using 'Overlay' Gimp method for increase the highlight.
    // The result is usually a brighter picture. 
    // Overlay mode composite value computation is D =  A * (B + (2 * B) * (255 - A)).
    
    i = 0;
    uint tmp, tmpM;
        
    for (h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < Width); w++)
            {     
            pOutBits[i++] = INT_MULT(pBWBits[i], pBWBits[i] + 
                                     INT_MULT(2 * pOverlayBits[i], 255 - pBWBits[i], tmpM), tmp);  // Blue.
            pOutBits[i++] = INT_MULT(pBWBits[i], pBWBits[i] + 
                                     INT_MULT(2 * pOverlayBits[i], 255 - pBWBits[i], tmpM), tmp);  // Green.
            pOutBits[i++] = INT_MULT(pBWBits[i], pBWBits[i] + 
                                     INT_MULT(2 * pOverlayBits[i], 255 - pBWBits[i], tmpM), tmp);  // Red.
            pOutBits[i++] = pBWBits[i];                                                            // Alpha.
            }
        
        // Update progress bar in dialog.
        m_imagePreviewWidget->setProgress((int) (80.0 + ((double)h * 20.0) / Height));
        kapp->processEvents(); 
        }

    // Copy target image to destination.
    
    if (!m_cancel) 
       memcpy (data, pOutBits, BitCount);        
    
    delete [] pBWBits;
    delete [] pBWBlurBits;
    delete [] pGrainBits;    
    delete [] pMaskBits;
    delete [] pOverlayBits;
    delete [] pOutBits;
}

}  // NameSpace DigikamInfraredImagesPlugin

#include "imageeffect_infrared.moc"
