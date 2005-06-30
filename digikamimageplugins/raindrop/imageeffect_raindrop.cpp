/* ============================================================
 * File  : imageeffect_raindrop.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-09-30
 * Description : a digiKam image plugin for to add
 *               raindrops on an image.
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
#include <qlayout.h>
#include <qframe.h>
#include <qslider.h>
#include <qimage.h>
#include <qspinbox.h>
#include <qtimer.h>

// KDE includes.

#include <klocale.h>
#include <kcursor.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <knuminput.h>
#include <kdebug.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "raindrop.h"
#include "imageeffect_raindrop.h"


namespace DigikamRainDropImagesPlugin
{

ImageEffect_RainDrop::ImageEffect_RainDrop(QWidget* parent)
                    : KDialogBase(Plain, i18n("Raindrops"),
                                  Help|User1|Ok|Cancel, Ok,
                                  parent, 0, true, true, i18n("&Reset Values")),
                      m_parent(parent)
{
    m_currentRenderingMode = NoneRendering;
    m_timer                = 0L;
    m_raindropFilter       = 0L;
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all parameters to the default values.") );
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Raindrops"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to add raindrops to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Raindrops algorithm"), 
                     "pieter_voloshyn at ame.com.br"); 
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Raindrops Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Add Raindrops to Image"), headerFrame, "labelTitle" );
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
        
    QFrame *frame = new QFrame(plainPage());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320, frame);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the preview of the Raindrop effect."
                                           "<p>Note: if you have previously selected an image part on editor, "
                                           "this part will be unused by the filter. You can use this way to "
                                           "disable the Raindrops effect on a human face for example.") );
    l->addWidget(m_previewWidget, 0);
    topLayout->addWidget(frame, 10);
    
    // -------------------------------------------------------------
                                                  
    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label1 = new QLabel(i18n("Drop Size:"), plainPage());
    
    m_dropInput = new KIntNumInput(plainPage());
    m_dropInput->setRange(0, 200, 1, true);
    m_dropInput->setValue(80);
    QWhatsThis::add( m_dropInput, i18n("<p>Set here the raindrops' size."));
    
    hlay2->addWidget(label1, 1);
    hlay2->addWidget(m_dropInput, 3);
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay3 = new QHBoxLayout(topLayout);
    QLabel *label2 = new QLabel(i18n("Number:"), plainPage());
    
    m_amountInput = new KIntNumInput(plainPage());
    m_amountInput->setRange(1, 500, 1, true);
    m_amountInput->setValue(150);
    QWhatsThis::add( m_amountInput, i18n("<p>This value controls the maximum number of raindrops.")); 
    hlay3->addWidget(label2, 1);
    hlay3->addWidget(m_amountInput, 3);
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay4 = new QHBoxLayout(topLayout);
    QLabel *label3 = new QLabel(i18n("Fish Eyes:"), plainPage());
    
    m_coeffInput = new KIntNumInput(plainPage());
    m_coeffInput->setRange(1, 100, 1, true);
    m_coeffInput->setValue(30);
    QWhatsThis::add( m_coeffInput, i18n("<p>This value is the fish-eye-effect optical "
                                        "distortion coefficient."));     
    
    hlay4->addWidget(label3, 1);
    hlay4->addWidget(m_coeffInput, 3);
    
    // -------------------------------------------------------------
        
    QHBoxLayout *hlay6 = new QHBoxLayout(topLayout);
    m_progressBar = new KProgress(100, plainPage(), "progressbar");
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    hlay6->addWidget(m_progressBar, 1);

    // -------------------------------------------------------------
    // Prevent both computation (resize event and Reset to default settings).
    m_previewWidget->blockSignals(true);
    resize(configDialogSize("Rain Drops Tool Dialog")); 
    m_previewWidget->blockSignals(false); 
    
    QTimer::singleShot(0, this, SLOT(slotUser1()));     // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect(m_dropInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));  
    
    connect(m_amountInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));  
    
    connect(m_coeffInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));  
            
    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotResized()));              
}

ImageEffect_RainDrop::~ImageEffect_RainDrop()
{
    if (m_raindropFilter)
       delete m_raindropFilter;    

    if (m_timer)
       delete m_timer;
    
    saveDialogSize("Rain Drops Tool Dialog");       
}

void ImageEffect_RainDrop::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_progressBar->setValue(0); 
    m_dropInput->setEnabled(true);
    m_amountInput->setEnabled(true);
    m_coeffInput->setEnabled(true);
    enableButton(Ok, true);  
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all filter parameters to their default values.") );
    kapp->restoreOverrideCursor();
}

void ImageEffect_RainDrop::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_raindropFilter->stopComputation();
       }
    else
       {
       m_dropInput->blockSignals(true);
       m_amountInput->blockSignals(true);
       m_coeffInput->blockSignals(true);
      
       m_dropInput->setValue(80);
       m_amountInput->setValue(150);
       m_coeffInput->setValue(30);

       m_dropInput->blockSignals(false);
       m_amountInput->blockSignals(false);
       m_coeffInput->blockSignals(false);
       slotEffect();
       }
} 

void ImageEffect_RainDrop::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_raindropFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    done(Cancel);
}
 
void ImageEffect_RainDrop::slotHelp()
{
    KApplication::kApplication()->invokeHelp("raindrops", "digikamimageplugins");
}

void ImageEffect_RainDrop::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_raindropFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    e->accept();    
}

void ImageEffect_RainDrop::slotResized(void)
{
    if (m_currentRenderingMode == FinalRendering)
       {
       m_previewWidget->update();
       return;
       }
    else if (m_currentRenderingMode == PreviewRendering)
       {
       m_raindropFilter->stopComputation();
       }
       
    QTimer::singleShot(0, this, SLOT(slotEffect()));        
}

void ImageEffect_RainDrop::slotTimer()
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

void ImageEffect_RainDrop::slotEffect()
{
    // Computation already in process.
    if (m_currentRenderingMode == PreviewRendering) return;     
    
    m_currentRenderingMode = PreviewRendering;
    
    m_dropInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_coeffInput->setEnabled(false);
    
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
    kapp->setOverrideCursor( KCursor::waitCursor() );
        
    int d        = m_dropInput->value();
    int a        = m_amountInput->value();
    int c        = m_coeffInput->value();

    m_progressBar->setValue(0); 
    
    if (m_raindropFilter)
       delete m_raindropFilter;
    
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    QImage orgImage(iface->originalWidth(), iface->originalHeight(), 32);
    uint *data = iface->getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    // Selected data from the image
    QRect selection( iface->selectedXOrg(), iface->selectedYOrg(),
                     iface->selectedWidth(), iface->selectedHeight() );
            
    m_raindropFilter = new RainDrop(&orgImage, this, d, a, c, &selection);
    
    delete [] data;
}   

void ImageEffect_RainDrop::slotOk()
{
    m_currentRenderingMode = FinalRendering;
    
    m_dropInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_coeffInput->setEnabled(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    kapp->setOverrideCursor( KCursor::waitCursor() );
    
    int d       = m_dropInput->value();
    int a       = m_amountInput->value();
    int c       = m_coeffInput->value();

    m_progressBar->setValue(0); 
        
    if (m_raindropFilter)
       delete m_raindropFilter;
       
    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    // Selected data from the image
    QRect selection( iface.selectedXOrg(), iface.selectedYOrg(),
                     iface.selectedWidth(), iface.selectedHeight() );
                     
    m_raindropFilter = new RainDrop(&orgImage, this, d, a, c, &selection);
           
    delete [] data;
}

void ImageEffect_RainDrop::customEvent(QCustomEvent *event)
{
    if (!event) return;

    RainDrop::EventData *d = (RainDrop::EventData*) event->data();

    if (!d) return;
    
    if (d->starting)           // Computation in progress !
        {
        m_progressBar->setValue(d->progress);
        }  
    else 
        {
        if (d->success)        // Computation Completed !
            {
            switch (m_currentRenderingMode)
              {
              case PreviewRendering:
                 {
                 kdDebug() << "Preview RainDrop completed..." << endl;
                 Digikam::ImageIface* iface = m_previewWidget->imageIface();
                 
                 QImage imDest = m_raindropFilter->getTargetImage();
                 iface->putPreviewData((uint*)(imDest.smoothScale(iface->previewWidth(),
                                                                  iface->previewHeight())).bits());
                 m_previewWidget->update();
                 abortPreview();
                 break;
                 }
              
              case FinalRendering:
                 {
                 kdDebug() << "Final RainDrop completed..." << endl;
                 
                 Digikam::ImageIface iface(0, 0);    
     
                 iface.putOriginalData(i18n("RainDrop"), 
                                       (uint*)m_raindropFilter->getTargetImage().bits());
                    
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
                    kdDebug() << "Preview RainDrop failed..." << endl;
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

}  // NameSpace DigikamRainDropImagesPlugin

#include "imageeffect_raindrop.moc"
