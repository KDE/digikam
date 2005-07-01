/* ============================================================
 * File  : imageeffect_unsharp.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-27
 * Description : Unsharp mask image filter for digiKam Image Editor
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

// Qt includes.

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qimage.h>
#include <qstring.h>
#include <qlayout.h>
#include <qframe.h>
#include <qtimer.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <knuminput.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kstandarddirs.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "unsharp.h"
#include "imageeffect_unsharp.h"

namespace DigikamUnsharpMaskImagesPlugin
{

ImageEffect_Unsharp::ImageEffect_Unsharp(QWidget* parent)
                   : KDialogBase(Plain, i18n("Unsharp Mask"), Help|User1|Ok|Cancel, Ok,
                                 parent, 0, true, true, i18n("&Reset Values")),
                     m_parent(parent)
{
    m_currentRenderingMode = NoneRendering;
    m_timer                = 0L;
    m_unsharpFilter        = 0L;
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Unsharp Mask"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("An unsharp mask image filter plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Winston Chang", I18N_NOOP("Unsharp mask algorithm author from Gimp"),
                     "winstonc at cs.wisc.edu");
                         
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Unsharp Mask Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Unsharp Mask"), headerFrame, "labelTitle" );
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
    
    m_imagePreviewWidget = new Digikam::ImagePannelWidget(480, 320, i18n("Preview"), plainPage(), true);
    hlay1->addWidget(m_imagePreviewWidget);
    
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setProgressWhatsThis(i18n("<p>This is the current percentage of the task completed."));
    
    // -------------------------------------------------------------

    QVGroupBox *gbox = m_imagePreviewWidget->settingsGroupBox();
    QLabel *label1 = new QLabel(i18n("Radius:"), gbox);
    
    m_radiusInput = new KDoubleNumInput(gbox, "m_radiusInput");
    m_radiusInput->setPrecision(1);
    m_radiusInput->setRange(0.1, 120.0, 0.1, true);
            
    QWhatsThis::add( m_radiusInput, i18n("<p>A radius of 0 has no effect, "
                     "10 and above determine the blur matrix radius "
                     "that determines how much to blur the image.") );
    
    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Amount:"), gbox);
    
    m_amountInput = new KDoubleNumInput(gbox, "m_amountInput");
    m_amountInput->setPrecision(2);
    m_amountInput->setRange(0.0, 5.0, 0.01, true);
            
    QWhatsThis::add( m_amountInput, i18n("<p>The value of the difference between the "
                     "original and the blur image that is added back into the original.") );

    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Threshold:"), gbox);
    
    m_thresholdInput = new KIntNumInput(gbox, "m_thresholdInput");
    m_thresholdInput->setRange(0, 255, 1, true);
        
    QWhatsThis::add( m_thresholdInput, i18n("<p>The threshold, as a fraction of the maximum "
                     "luminosity value, needed to apply the difference amount.") );


    // -------------------------------------------------------------
    
    resize(configDialogSize("UnSharpMask Tool Dialog"));         
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
            
    // -------------------------------------------------------------
        
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_radiusInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                                                

    connect(m_amountInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                                                            
            
    connect(m_thresholdInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));                                                
}

ImageEffect_Unsharp::~ImageEffect_Unsharp()
{
    saveDialogSize("UnSharpMask Tool Dialog");    
    
    if (m_unsharpFilter)
       delete m_unsharpFilter;    
    
    if (m_timer)
       delete m_timer;
}

void ImageEffect_Unsharp::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_radiusInput->setEnabled(true);
    m_amountInput->setEnabled(true);
    m_thresholdInput->setEnabled(true);
    m_imagePreviewWidget->setEnable(true);   
    enableButton(Ok, true);  
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all filter parameters to their default values.") );
}

void ImageEffect_Unsharp::slotHelp()
{
    KApplication::kApplication()->invokeHelp("unsharp", "digikamimageplugins");
}

void ImageEffect_Unsharp::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_unsharpFilter->stopComputation();
       m_parent->setCursor( KCursor::arrowCursor() );
       }
       
    e->accept();   
}

void ImageEffect_Unsharp::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_unsharpFilter->stopComputation();
       m_parent->setCursor( KCursor::arrowCursor() );
       }
       
    done(Cancel);
}

void ImageEffect_Unsharp::slotTimer()
{
    if (m_timer)
       {
       m_timer->stop();
       delete m_timer;
       }
    
    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL(timeout()),
             this, SLOT(slotEffect()) );
    m_timer->start(500, true);
}

void ImageEffect_Unsharp::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_unsharpFilter->stopComputation();
       }
    else
       {
       m_radiusInput->blockSignals(true);
       m_amountInput->blockSignals(true);
       m_thresholdInput->blockSignals(true);
                
       m_radiusInput->setValue(5.0);
       m_amountInput->setValue(0.5);
       m_thresholdInput->setValue(0);
                                     
       m_radiusInput->blockSignals(false);
       m_amountInput->blockSignals(false);
       m_thresholdInput->blockSignals(false);
       slotEffect();
       }
} 

void ImageEffect_Unsharp::slotEffect()
{
    if (m_currentRenderingMode == PreviewRendering) return;     // Computation already in process.
    
    m_currentRenderingMode = PreviewRendering;
    
    m_radiusInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
        
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    QImage img = m_imagePreviewWidget->getOriginalClipImage();
   
    double r  = m_radiusInput->value();
    double a  = m_amountInput->value();
    int    th = m_thresholdInput->value();
    
    m_imagePreviewWidget->setProgress(0);

    if (m_unsharpFilter)
       delete m_unsharpFilter;
        
    m_unsharpFilter = new UnsharpMask(&img, this, r, a, th);
}

void ImageEffect_Unsharp::slotOk()
{
    m_currentRenderingMode = FinalRendering;
    
    m_radiusInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    m_parent->setCursor( KCursor::waitCursor() );
    
    double r  = m_radiusInput->value();
    double a  = m_amountInput->value();
    int    th = m_thresholdInput->value();
    
    m_imagePreviewWidget->setProgress(0);       

    if (m_unsharpFilter)
       delete m_unsharpFilter;
    
    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
            
    m_unsharpFilter = new UnsharpMask(&orgImage, this, r, a, th);
           
    delete [] data;
}

void ImageEffect_Unsharp::customEvent(QCustomEvent *event)
{
    if (!event) return;

    UnsharpMask::EventData *d = (UnsharpMask::EventData*) event->data();

    if (!d) return;
    
    if (d->starting)           // Computation in progress !
        {
        m_imagePreviewWidget->setProgress(d->progress);
        }  
    else 
        {
        if (d->success)        // Computation Completed !
            {
            switch (m_currentRenderingMode)
              {
              case PreviewRendering:
                 {
                 kdDebug() << "Preview Unsharp Mask completed..." << endl;
                 
                 QImage imDest = m_unsharpFilter->getTargetImage();
                 m_imagePreviewWidget->setPreviewImageData(imDest);
    
                 abortPreview();
                 break;
                 }
              
              case FinalRendering:
                 {
                 kdDebug() << "Final Unsharp Mask completed..." << endl;
                 
                 Digikam::ImageIface iface(0, 0);
  
                 iface.putOriginalData(i18n("Unsharp Mask"), 
                                       (uint*)m_unsharpFilter->getTargetImage().bits());
                    
                 m_parent->setCursor( KCursor::arrowCursor() );
                 accept();
                 break;
                 }
              }
            }
        else                   // Computation Failed !
            {
            switch (m_currentRenderingMode)
                {
                case PreviewRendering:
                    {
                    kdDebug() << "Preview Unsharp Mask failed..." << endl;
                    // abortPreview() must be call here for set progress bar to 0 properly.
                    abortPreview();
                    break;
                    }
                
                case FinalRendering:
                    break;
                }
            }
        }

    delete d;        
}

}  // NameSpace DigikamUnsharpMaskImagesPlugin

#include "imageeffect_unsharp.moc"
