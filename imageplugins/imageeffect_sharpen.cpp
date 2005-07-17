/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-09
 * Description : Sharpen image filter for ImageEditor
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

#include <qlayout.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtimer.h>

// KDE includes.

#include <knuminput.h>
#include <kcursor.h>
#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>

// Digikam includes.

#include <imageiface.h>
#include <imagepannelwidget.h>
#include <imagefilters.h>
#include <sharpen.h>

// Local includes.

#include "imageeffect_sharpen.h"

ImageEffect_Sharpen::ImageEffect_Sharpen(QWidget* parent)
                   : KDialogBase(Plain, i18n("Sharpening a Photograph"),
                                 Help|User1|Ok|Cancel, Ok,
                                 parent, 0, true, true,
                                 i18n("&Reset Values")),                                 
                     m_parent(parent)
{
    m_currentRenderingMode = NoneRendering;
    m_timer                = 0L;
    m_threadedFilter       = 0L;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to their default values.") );
    setHelp("blursharpentool.anchor", "digikam");
    resize(configDialogSize("Sharpen Tool Dialog"));       
    
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    m_imagePreviewWidget = new Digikam::ImagePannelWidget(240, 160, plainPage(), true);
    hlay1->addWidget(m_imagePreviewWidget);

    // -------------------------------------------------------------
    
    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 1, 2, marginHint(), spacingHint());
    QLabel *label = new QLabel(i18n("Sharpness:"), gboxSettings);
    
    m_radiusInput = new KIntNumInput(gboxSettings);
    m_radiusInput->setRange(0, 100, 1, true);
    m_radiusInput->setValue(0);
    QWhatsThis::add( m_radiusInput, i18n("<p>A sharpness of 0 has no effect, "
                                         "1 and above determine the sharpen matrix radius "
                                         "that determines how much to sharpen the image."));
                                         
    gridSettings->addWidget(label, 0, 0);
    gridSettings->addWidget(m_radiusInput, 0, 1);
        
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    
    QTimer::singleShot(0, this, SLOT(slotUser1()));

    // -------------------------------------------------------------
                                                             
    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotFocusChanged()));
}

ImageEffect_Sharpen::~ImageEffect_Sharpen()
{
    saveDialogSize("Sharpen Tool Dialog");    
    
    if (m_timer)
       delete m_timer;
    
    if (m_threadedFilter)
       delete m_threadedFilter;    
}

void ImageEffect_Sharpen::slotTimer()
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

void ImageEffect_Sharpen::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_imagePreviewWidget->setEnable(true);    
    enableButton(Ok, true);  
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all filter parameters to their default values.") );
    m_radiusInput->setEnabled(true);
}
void ImageEffect_Sharpen::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_threadedFilter->stopComputation();
       }
    else
       {
       m_radiusInput->blockSignals(true);
       m_radiusInput->setValue(0);
       m_radiusInput->blockSignals(false);
       slotEffect();    
       }
} 

void ImageEffect_Sharpen::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_threadedFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    done(Cancel);
}

void ImageEffect_Sharpen::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_threadedFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    e->accept();    
}

void ImageEffect_Sharpen::slotFocusChanged(void)
{
    if (m_currentRenderingMode == FinalRendering)
       {
       m_imagePreviewWidget->update();
       return;
       }
    else if (m_currentRenderingMode == PreviewRendering)
       {
       m_threadedFilter->stopComputation();
       }
       
    QTimer::singleShot(0, this, SLOT(slotEffect()));        
}

void ImageEffect_Sharpen::slotEffect()
{
    // Computation already in process.
    if (m_currentRenderingMode == PreviewRendering) return;     
    
    m_currentRenderingMode = PreviewRendering;

    m_imagePreviewWidget->setEnable(false);
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok,    false);
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    m_imagePreviewWidget->setProgress(0);
    
    if (m_threadedFilter)
       delete m_threadedFilter;

    m_radiusInput->setEnabled(false);
    
    QImage img = m_imagePreviewWidget->getOriginalClipImage();
    
    int r = m_radiusInput->value();
            
    m_threadedFilter = new Digikam::Sharpen(&img, this, r);    
}

void ImageEffect_Sharpen::slotOk()
{
    m_currentRenderingMode = FinalRendering;

    m_imagePreviewWidget->setEnable(false);
    enableButton(Ok,    false);
    enableButton(User1, false);
    enableButton(User2, false);
    enableButton(User3, false);
    kapp->setOverrideCursor( KCursor::waitCursor() );
    m_imagePreviewWidget->setProgress(0);
    
    if (m_threadedFilter)
       delete m_threadedFilter;
    
    m_radiusInput->setEnabled(false);
    
    int r = m_radiusInput->value();
    
    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
            
    m_threadedFilter = new Digikam::Sharpen(&orgImage, this, r);    
    delete [] data;
}

void ImageEffect_Sharpen::customEvent(QCustomEvent *event)
{
    if (!event) return;

    Digikam::ThreadedFilter::EventData *d = (Digikam::ThreadedFilter::EventData*) event->data();

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
                 kdDebug() << "Preview Sharpen completed..." << endl;
                 QImage imDest = m_threadedFilter->getTargetImage();
                 m_imagePreviewWidget->setPreviewImageData(imDest);
                 abortPreview();
                 break;
                 }
              
              case FinalRendering:
                 {
                 kdDebug() << "Final Sharpen completed..." << endl;
                 Digikam::ImageIface iface(0, 0);
                 iface.putOriginalData(i18n("Sharpen"), 
                                       (uint*)m_threadedFilter->getTargetImage().bits());
                 kapp->restoreOverrideCursor();
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
                    kdDebug() << "Preview Sharpen failed..." << endl;
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

#include "imageeffect_sharpen.moc"
