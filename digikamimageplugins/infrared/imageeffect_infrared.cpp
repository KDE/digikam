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

// C++ include.

#include <cstring>
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qvgroupbox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlcdnumber.h>
#include <qslider.h>
#include <qlayout.h>
#include <qframe.h>
#include <qdatetime.h> 
#include <qtimer.h>
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
#include <kdebug.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "infrared.h"
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
    m_currentRenderingMode = NoneRendering;
    m_infraredFilter       = 0L;
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    resize(configDialogSize("Infrared Tool Dialog")); 
    
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
    QLabel *labelTitle = new QLabel( i18n("Simulate Infrared Film to Photograph"), headerFrame, "labelTitle" );
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
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 3, 2, marginHint(), spacingHint());
    QLabel *label1 = new QLabel(i18n("Sensibility (ISO):"), gboxSettings);
    
    m_sensibilitySlider = new QSlider(1, 7, 1, 1, Qt::Horizontal, gboxSettings);
    m_sensibilitySlider->setTracking ( false );
    m_sensibilitySlider->setTickInterval(1);
    m_sensibilitySlider->setTickmarks(QSlider::Below);
    
    m_sensibilityLCDValue = new QLCDNumber (3, gboxSettings);
    m_sensibilityLCDValue->setSegmentStyle ( QLCDNumber::Flat );
    m_sensibilityLCDValue->display( QString::number(200) );
    whatsThis = i18n("<p>Set here the ISO-sensitivity of the simulated infrared film. "
                     "Increasing this value will increase the portion of green color in the mix. " 
                     "It will also increase the halo effect on the hightlights, and the film "
                     "graininess (if the box is checked).");
        
    QWhatsThis::add( m_sensibilityLCDValue, whatsThis);
    QWhatsThis::add( m_sensibilitySlider, whatsThis);

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_sensibilitySlider, 0, 0, 1, 1);
    gridSettings->addMultiCellWidget(m_sensibilityLCDValue, 0, 0, 2, 2);
    
    // -------------------------------------------------------------

    m_addFilmGrain = new QCheckBox( i18n("Add film grain"), gboxSettings);
    m_addFilmGrain->setChecked( true );
    QWhatsThis::add( m_addFilmGrain, i18n("<p>This option is adding infrared film grain to "
                                          "the image depending on ISO-sensitivity."));
    gridSettings->addMultiCellWidget(m_addFilmGrain, 1, 1, 0, 2);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
        
    // -------------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
    
    // -------------------------------------------------------------
    
    connect( m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
             this, SLOT(slotFocusChanged()) );
    
    connect( m_sensibilitySlider, SIGNAL(valueChanged(int)),
             this, SLOT(slotSensibilityChanged(int)) ); 
             
    connect( m_addFilmGrain, SIGNAL(toggled (bool)),
             this, SLOT(slotEffect()) );                        
}

ImageEffect_Infrared::~ImageEffect_Infrared()
{
    saveDialogSize("Infrared Tool Dialog");    
    
    if (m_infraredFilter)
       delete m_infraredFilter;   
}

void ImageEffect_Infrared::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_sensibilitySlider->setEnabled(true);
    m_addFilmGrain->setEnabled(true);
    m_imagePreviewWidget->setEnable(true);    
    enableButton(Ok, true);  
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all filter parameters to their default values.") );
}

void ImageEffect_Infrared::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_infraredFilter->stopComputation();
       }
    else
       {
       m_sensibilitySlider->blockSignals(true);
       m_sensibilitySlider->setValue(1);
       m_sensibilitySlider->blockSignals(false);
       slotEffect();    
       }
} 

void ImageEffect_Infrared::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_infraredFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    done(Cancel);
}

void ImageEffect_Infrared::slotHelp()
{
    KApplication::kApplication()->invokeHelp("infrared", "digikamimageplugins");
}

void ImageEffect_Infrared::slotFocusChanged(void)
{
    if (m_currentRenderingMode == FinalRendering)
       {
       m_imagePreviewWidget->update();
       return;
       }
    else if (m_currentRenderingMode == PreviewRendering)
       {
       m_infraredFilter->stopComputation();
       }
       
    QTimer::singleShot(0, this, SLOT(slotEffect()));        
}

void ImageEffect_Infrared::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_infraredFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    e->accept();    
}

void ImageEffect_Infrared::slotSensibilityChanged(int v)
{
    m_sensibilityLCDValue->display( QString::number(100 + 100 * v) );
    slotEffect();
}

void ImageEffect_Infrared::slotEffect()
{
    // Computation already in process.
    if (m_currentRenderingMode == PreviewRendering) return;     
    
    m_currentRenderingMode = PreviewRendering;
    m_imagePreviewWidget->setEnable(false);
    m_addFilmGrain->setEnabled(false);
    m_sensibilitySlider->setEnabled(false);
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
        
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    int   s      = 100 + 100 * m_sensibilitySlider->value();
    bool  g      = m_addFilmGrain->isChecked();

    m_imagePreviewWidget->setProgress(0);
    
    if (m_infraredFilter)
       delete m_infraredFilter;
        
    m_infraredFilter = new Infrared(&image, this, s, g);
}

void ImageEffect_Infrared::slotOk()
{
    m_currentRenderingMode = FinalRendering;
    
    m_addFilmGrain->setEnabled(false);
    m_sensibilitySlider->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    kapp->setOverrideCursor( KCursor::waitCursor() );
    
    int  s    = 100 + 100 * m_sensibilitySlider->value();
    bool g    = m_addFilmGrain->isChecked();
    
    if (m_infraredFilter)
       delete m_infraredFilter;
               
    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    m_infraredFilter = new Infrared(&orgImage, this, s, g);
           
    delete [] data;
}

void ImageEffect_Infrared::customEvent(QCustomEvent *event)
{
    if (!event) return;

    Infrared::EventData *d = (Infrared::EventData*) event->data();

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
                 kdDebug() << "Preview Infrared completed..." << endl;
                 
                 QImage imDest = m_infraredFilter->getTargetImage();
                 m_imagePreviewWidget->setPreviewImageData(imDest);
    
                 abortPreview();
                 break;
                 }
              
              case FinalRendering:
                 {
                 kdDebug() << "Final Infrared completed..." << endl;
                 
                 Digikam::ImageIface iface(0, 0);
  
                 iface.putOriginalData(i18n("Infrared"), 
                                       (uint*)m_infraredFilter->getTargetImage().bits());
                    
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
                    kdDebug() << "Preview Infrared failed..." << endl;
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

}  // NameSpace DigikamInfraredImagesPlugin

#include "imageeffect_infrared.moc"
