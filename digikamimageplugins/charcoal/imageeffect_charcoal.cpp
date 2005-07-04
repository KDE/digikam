/* ============================================================
 * File  : imageeffect_charcoal.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a digikam image editor plugin for 
 *               simulate charcoal drawing.
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

// Qt includes.

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
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
#include <knuminput.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "charcoal.h"
#include "imageeffect_charcoal.h"

namespace DigikamCharcoalImagesPlugin
{

ImageEffect_Charcoal::ImageEffect_Charcoal(QWidget* parent)
                    : KDialogBase(Plain, i18n("Charcoal Drawing"),
                                  Help|User1|Ok|Cancel, Ok,
                                  parent, 0, true, true, i18n("&Reset Values")),
                      m_parent(parent)
{
    m_currentRenderingMode = NoneRendering;
    m_charcoalFilter       = 0L;
    m_timer                = 0;
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    resize(configDialogSize("Charcoal Tool Dialog")); 
        
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Charcoal Drawing"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A charcoal drawing image effect plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Charcoal Drawing Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Charcoal Drawing"), headerFrame, "labelTitle" );
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
    
    m_imagePreviewWidget = new Digikam::ImagePannelWidget(240, 160, plainPage(), true);
    hlay1->addWidget(m_imagePreviewWidget);
    
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setProgressWhatsThis(i18n("<p>This is the current percentage of the task completed."));
    
    // -------------------------------------------------------------
    
    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 2, 2, marginHint(), spacingHint());
    QLabel *label1 = new QLabel(i18n("Pencil size:"), gboxSettings);
    
    m_pencilInput = new KIntNumInput(gboxSettings);
    m_pencilInput->setRange(1, 100, 1, true);  
    m_pencilInput->setValue(5);
    QWhatsThis::add( m_pencilInput, i18n("<p>Set here the charcoal pencil size used to simulate the drawing."));

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_pencilInput, 0, 0, 1, 1);
    
    // -------------------------------------------------------------
    
    QLabel *label2 = new QLabel(i18n("Smooth:"), gboxSettings);
    
    m_smoothInput = new KIntNumInput(gboxSettings);
    m_smoothInput->setRange(1, 100, 1, true);  
    m_smoothInput->setValue(10);
    QWhatsThis::add( m_smoothInput, i18n("<p>This value controls the smoothing effect of the pencil "
                                         "under the canvas."));

    gridSettings->addMultiCellWidget(label2, 1, 1, 0, 0);
    gridSettings->addMultiCellWidget(m_smoothInput, 1, 1, 1, 1);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
    
    // -------------------------------------------------------------
        
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotFocusChanged()));
    
    connect(m_pencilInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));      
                
    connect(m_smoothInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));      
}

ImageEffect_Charcoal::~ImageEffect_Charcoal()
{
    saveDialogSize("Charcoal Tool Dialog"); 
    
    if (m_charcoalFilter)
       delete m_charcoalFilter;       
    
    if (m_timer)
       delete m_timer;
}

void ImageEffect_Charcoal::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_pencilInput->setEnabled(true);
    m_smoothInput->setEnabled(true);
    m_imagePreviewWidget->setEnable(true);    
    enableButton(Ok, true);  
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all filter parameters to their default values.") );
}

void ImageEffect_Charcoal::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_charcoalFilter->stopComputation();
       }
    else
       {
       m_pencilInput->blockSignals(true);
       m_smoothInput->blockSignals(true);
                
       m_pencilInput->setValue(5);
       m_smoothInput->setValue(10);
        
       m_pencilInput->blockSignals(false);
       m_smoothInput->blockSignals(false);
       slotEffect();
       }
} 

void ImageEffect_Charcoal::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_charcoalFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    done(Cancel);
}

void ImageEffect_Charcoal::slotHelp()
{
    KApplication::kApplication()->invokeHelp("charcoal", "digikamimageplugins");
}

void ImageEffect_Charcoal::slotFocusChanged(void)
{
    if (m_currentRenderingMode == FinalRendering)
       {
       m_imagePreviewWidget->update();
       return;
       }
    else if (m_currentRenderingMode == PreviewRendering)
       {
       m_charcoalFilter->stopComputation();
       }
       
    QTimer::singleShot(0, this, SLOT(slotEffect()));        
}

void ImageEffect_Charcoal::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_charcoalFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
           
    e->accept();    
}

void ImageEffect_Charcoal::slotTimer()
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

void ImageEffect_Charcoal::slotEffect()
{
    // Computation already in process.
    if (m_currentRenderingMode == PreviewRendering) return;     
    
    m_currentRenderingMode = PreviewRendering;
    m_pencilInput->setEnabled(false);
    m_smoothInput->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
            
    double pencil = (double)m_pencilInput->value();
    double smooth = (double)m_smoothInput->value();
    m_imagePreviewWidget->setProgress(0);
    
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    
    if (m_charcoalFilter)
       delete m_charcoalFilter;
        
    m_charcoalFilter = new Charcoal(&image, this, pencil, smooth);
}

void ImageEffect_Charcoal::slotOk()
{
    m_currentRenderingMode = FinalRendering;
    
    m_pencilInput->setEnabled(false);
    m_smoothInput->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    kapp->setOverrideCursor( KCursor::waitCursor() );
    
    double pencil = (double)m_pencilInput->value();
    double smooth = (double)m_smoothInput->value();

    m_imagePreviewWidget->setProgress(0);
        
    if (m_charcoalFilter)
       delete m_charcoalFilter;
       
    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    m_charcoalFilter = new Charcoal(&orgImage, this, pencil, smooth);
           
    delete [] data;
}

void ImageEffect_Charcoal::customEvent(QCustomEvent *event)
{
    if (!event) return;

    Charcoal::EventData *d = (Charcoal::EventData*) event->data();

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
                 kdDebug() << "Preview Charcoal completed..." << endl;
                 
                 QImage imDest = m_charcoalFilter->getTargetImage();
                 m_imagePreviewWidget->setPreviewImageData(imDest);
    
                 abortPreview();
                 break;
                 }
              
              case FinalRendering:
                 {
                 kdDebug() << "Final Charcoal completed..." << endl;
                 
                 Digikam::ImageIface iface(0, 0);
  
                 iface.putOriginalData(i18n("Charcoal"), 
                                       (uint*)m_charcoalFilter->getTargetImage().bits());
                    
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
                    kdDebug() << "Preview Charcoal failed..." << endl;
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

}  // NameSpace DigikamCharcoalImagesPlugin

#include "imageeffect_charcoal.moc"
