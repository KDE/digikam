/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-09
 * Description : Blur image filter for ImageEditor
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

#include "imageiface.h"
#include "imagepannelwidget.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "dimggaussianblur.h"

// Local includes.

#include "imageeffect_blur.h"

ImageEffect_Blur::ImageEffect_Blur(QWidget* parent)
                : KDialogBase(Plain, i18n("Apply Gaussian Blur on Photograph"),
                              Help|Default|User1|Ok|Cancel, Ok,
                              parent, 0, true, true,
                              i18n("&Abort")),
                  m_parent(parent)
{
    m_currentRenderingMode = NoneRendering;
    m_timer                = 0L;
    m_threadedFilter       = 0L;
    
    setButtonWhatsThis( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    setHelp("blursharpentool.anchor", "digikam");
    resize(configDialogSize("Blur Tool Dialog"));         
    
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePannelWidget(240, 160, "Blur Tool Dialog", plainPage(), true);
    hlay1->addWidget(m_imagePreviewWidget);

    // -------------------------------------------------------------
    
    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 1, 2, marginHint(), spacingHint());
    QLabel *label = new QLabel(i18n("Smoothness:"), gboxSettings);
    
    m_radiusInput = new KIntNumInput(gboxSettings);
    m_radiusInput->setRange(0, 20, 1, true);
    m_radiusInput->setValue(0);
    QWhatsThis::add( m_radiusInput, i18n("<p>A smoothness of 0 has no effect, "
                                         "1 and above determine the Gaussian blur matrix radius "
                                         "that determines how much to blur the image."));

    gridSettings->addWidget(label, 0, 0);
    gridSettings->addWidget(m_radiusInput, 0, 1);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
        
    // -------------------------------------------------------------
    
    QTimer::singleShot(0, this, SLOT(slotDefault()));
                                             
    // -------------------------------------------------------------
    
    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));
     
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotFocusChanged()));
}

ImageEffect_Blur::~ImageEffect_Blur()
{
    saveDialogSize("Blur Tool Dialog");    

    if (m_timer)
       delete m_timer;
    
    if (m_threadedFilter)
       delete m_threadedFilter;    
}

void ImageEffect_Blur::slotTimer()
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

void ImageEffect_Blur::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_imagePreviewWidget->setEnable(true);    
    enableButton(Ok, true);  
    enableButton(Default, true);  
    enableButton(User1, false);  
    m_radiusInput->setEnabled(true);
}

void ImageEffect_Blur::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
       m_threadedFilter->stopComputation();
}

void ImageEffect_Blur::slotDefault()
{
    m_radiusInput->blockSignals(true);
    m_radiusInput->setValue(0);
    m_radiusInput->blockSignals(false);
    slotEffect();    
} 

void ImageEffect_Blur::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
    {
       m_threadedFilter->stopComputation();
       kapp->restoreOverrideCursor();
    }
       
    done(Cancel);
}

void ImageEffect_Blur::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
    {
       m_threadedFilter->stopComputation();
       kapp->restoreOverrideCursor();
    }
       
    e->accept();    
}

void ImageEffect_Blur::slotFocusChanged(void)
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

void ImageEffect_Blur::slotEffect()
{
    // Computation already in process.
    if (m_currentRenderingMode == PreviewRendering) return;     
    
    m_currentRenderingMode = PreviewRendering;

    m_imagePreviewWidget->setEnable(false);
    enableButton(Default, false);  
    enableButton(User1,   true);  
    enableButton(Ok,      false);
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    m_imagePreviewWidget->setProgress(0);
    
    if (m_threadedFilter)
       delete m_threadedFilter;

    m_radiusInput->setEnabled(false);
    
    Digikam::DImg img = m_imagePreviewWidget->getOriginalRegionImage();
        
    m_threadedFilter = new Digikam::DImgGaussianBlur(&img, this, m_radiusInput->value());    
}

void ImageEffect_Blur::slotOk()
{
    m_currentRenderingMode = FinalRendering;

    m_imagePreviewWidget->setEnable(false);
    enableButton(Ok,    false);
    enableButton(User1, false);
    enableButton(Default, false);  
    kapp->setOverrideCursor( KCursor::waitCursor() );
    m_imagePreviewWidget->setProgress(0);
    
    if (m_threadedFilter)
       delete m_threadedFilter;
    
    m_radiusInput->setEnabled(false);
    
    Digikam::ImageIface iface(0, 0);
    uchar *data     = iface.getOriginalImage();
    int w           = iface.originalWidth();
    int h           = iface.originalHeight();
    bool sixteenBit = iface.originalSixteenBit();
    bool hasAlpha   = iface.originalHasAlpha();
    Digikam::DImg orgImage = Digikam::DImg(w, h, sixteenBit, hasAlpha ,data);
    delete [] data;
    m_threadedFilter = new Digikam::DImgGaussianBlur(&orgImage, this, m_radiusInput->value());
}

void ImageEffect_Blur::customEvent(QCustomEvent *event)
{
    if (!event) return;

    Digikam::DImgThreadedFilter::EventData *d = (Digikam::DImgThreadedFilter::EventData*) event->data();

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
                    kdDebug() << "Preview Gaussian Blur completed..." << endl;
                    Digikam::DImg imDest = m_threadedFilter->getTargetImage();
                    m_imagePreviewWidget->setPreviewImage(imDest);
                    abortPreview();
                    break;
                }
                
                case FinalRendering:
                {
                    kdDebug() << "Final Gaussian Blur completed..." << endl;
                    Digikam::ImageIface iface(0, 0);
                    Digikam::DImg imDest = m_threadedFilter->getTargetImage();
                    iface.putOriginalImage(i18n("Gaussian Blur"), imDest.bits());
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
                    kdDebug() << "Preview Gaussian Blur failed..." << endl;
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

// Backport KDialog::keyPressEvent() implementation from KDELibs to ignore Enter/Return Key events 
// to prevent any conflicts between dialog keys events and SpinBox keys events.

void ImageEffect_Blur::keyPressEvent(QKeyEvent *e)
{
    if ( e->state() == 0 )
    {
        switch ( e->key() )
        {
        case Key_Escape:
            e->accept();
            reject();
        break;
        case Key_Enter:            
        case Key_Return:     
            e->ignore();              
        break;
        default:
            e->ignore();
            return;
        }
    }
    else
    {
        // accept the dialog when Ctrl-Return is pressed
        if ( e->state() == ControlButton &&
            (e->key() == Key_Return || e->key() == Key_Enter) )
        {
            e->accept();
            accept();
        }
        else
        {
            e->ignore();
        }
    }
}

#include "imageeffect_blur.moc"
