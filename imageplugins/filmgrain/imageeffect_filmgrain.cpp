/* ============================================================
 * File  : imageeffect_filmgrain.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a digiKam image editor plugin for to add film 
 *               grain on an image.
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
#include "imageeffect_filmgrain.h"

namespace DigikamFilmGrainImagesPlugin
{

ImageEffect_FilmGrain::ImageEffect_FilmGrain(QWidget* parent)
                     : KDialogBase(Plain, i18n("Film Grain"),
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
                                       I18N_NOOP("Film Grain"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to apply a film grain effect to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Film Grain Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Add Film Grain to Image"), headerFrame, "labelTitle" );
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
    QLabel *label1 = new QLabel(i18n("Film sensibility (ISO):"), plainPage());
    
    m_sensibilitySlider = new QSlider(2, 30, 1, 12, Qt::Horizontal, plainPage(), "m_sensibilitySlider");
    m_sensibilitySlider->setTracking ( false );
    m_sensibilitySlider->setTickInterval(1);
    m_sensibilitySlider->setTickmarks(QSlider::Below);
    
    m_sensibilityLCDValue = new QLCDNumber (4, plainPage(), "m_sensibilityLCDValue");
    m_sensibilityLCDValue->setSegmentStyle ( QLCDNumber::Flat );
    m_sensibilityLCDValue->display( QString::number(2400) );
    whatsThis = i18n("<p>Set here the film ISO-sensitivity to use for simulating the film graininess.");
        
    QWhatsThis::add( m_sensibilityLCDValue, whatsThis);
    QWhatsThis::add( m_sensibilitySlider, whatsThis);

    hlay->addWidget(label1, 1);
    hlay->addWidget(m_sensibilitySlider, 3);
    hlay->addWidget(m_sensibilityLCDValue, 1);
    
    // -------------------------------------------------------------
        
    adjustSize();
    disableResize(); 
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect( m_sensibilitySlider, SIGNAL(valueChanged(int)),
             this, SLOT(slotSensibilityChanged(int)) ); 
}

ImageEffect_FilmGrain::~ImageEffect_FilmGrain()
{
}

void ImageEffect_FilmGrain::slotUser1()
{
    m_sensibilitySlider->blockSignals(true);
    m_sensibilitySlider->setValue(12);
    m_sensibilitySlider->blockSignals(false);
    slotEffect();    
} 

void ImageEffect_FilmGrain::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_FilmGrain::slotHelp()
{
    KApplication::kApplication()->invokeHelp("filmgrain", "digikamimageplugins");
}

void ImageEffect_FilmGrain::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_FilmGrain::slotSensibilityChanged(int v)
{
    m_sensibilityLCDValue->display( QString::number(400+200*v) );
    slotEffect();
}

void ImageEffect_FilmGrain::slotEffect()
{
    m_imagePreviewWidget->setEnable(false);
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    uint* data   = (uint *)image.bits();
    int   w      = image.width();
    int   h      = image.height();
    int   s      = 400 + 200 * m_sensibilitySlider->value();
            
    m_imagePreviewWidget->setProgress(0);
    FilmGrain(data, w, h, s);
    
    if (m_cancel) return;
    
    m_imagePreviewWidget->setProgress(0);  
    m_imagePreviewWidget->setPreviewImageData(image);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_imagePreviewWidget->setEnable(true);
}

void ImageEffect_FilmGrain::slotOk()
{
    m_sensibilitySlider->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
        
    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    int s      = 400 + 200 * m_sensibilitySlider->value();
    
    m_imagePreviewWidget->setProgress(0);
    FilmGrain(data, w, h, s);

    if ( !m_cancel )
       iface.putOriginalData(i18n("Film Grain"), data);
       
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

// This method is based on the Simulate Film grain tutorial from GimpGuru.org web site 
// available at this url : http://www.gimpguru.org/Tutorials/FilmGrain

void ImageEffect_FilmGrain::FilmGrain(uint* data, int Width, int Height, int Sensibility)
{
    if (Sensibility <= 0) return;
    
    int Noise = (int)(Sensibility / 10.0);
    int nStride = GetStride(Width);
    register int h, w, i = 0;       
    int nRand;

    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int      BitCount = LineWidth * Height;
    uchar*    pInBits = (uchar*)data;
    uchar* pGrainBits = new uchar[BitCount];    // Grain blured without curves adjustment.
    uchar*  pMaskBits = new uchar[BitCount];    // Grain mask with curves adjustment.
    uchar*   pOutBits = new uchar[BitCount];    // Destination image with grain mask and original image merged.
    
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    srand ((uint) dt.secsTo(Y2000));
    
    // Make gray grain mask.
    
    for (h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < Width); w++)
            {
            nRand = (rand() % Noise) - (Noise / 2);
            
            pGrainBits[i++] = LimitValues (128 + nRand);    // Red.
            pGrainBits[i++] = LimitValues (128 + nRand);    // Green.
            pGrainBits[i++] = LimitValues (128 + nRand);    // Blue.
            pGrainBits[i++] = 0;                            // Reset Alpha (not used here).
            }
        
        // Update de progress bar in dialog.
        m_imagePreviewWidget->setProgress((int) (((double)h * 25.0) / Height));
        kapp->processEvents(); 
        }

    // Smooth grain mask using gaussian blur.    
    
    Digikam::ImageFilters::gaussianBlurImage((uint *)pGrainBits, Width, Height, 3);
            
    // Normally, film grain tends to be most noticable in the midtones, and much less 
    // so in the shadows and highlights. Adjust histogram curve to adjust grain like this. 

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
    
    // Merge src image with grain using shade coefficient.

    int Shade = 32; // This value control the shading pixel effect between original image and grain mask.
    i = 0;
        
    for (h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < Width); w++)
            {        
            pOutBits[i++] = (pInBits[i] * (255 - Shade) + pMaskBits[i] * Shade) >> 8;    // Red.
            pOutBits[i++] = (pInBits[i] * (255 - Shade) + pMaskBits[i] * Shade) >> 8;    // Green.
            pOutBits[i++] = (pInBits[i] * (255 - Shade) + pMaskBits[i] * Shade) >> 8;    // Blue.
            pOutBits[i++] = pInBits[i];                                                  // Alpha.
            }
        
        // Update de progress bar in dialog.
        m_imagePreviewWidget->setProgress((int) (50.0 + ((double)h * 50.0) / Height));
        kapp->processEvents();             
        }
    
    // Copy target image to destination.
    
    if (!m_cancel) 
       memcpy (data, pOutBits, BitCount);        
                
    delete [] pGrainBits;    
    delete [] pMaskBits;
    delete [] pOutBits;
}

}  // NameSpace DigikamFilmGrainImagesPlugin

#include "imageeffect_filmgrain.moc"
