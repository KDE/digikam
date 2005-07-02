/* ============================================================
 * File  : imageeffect_despeckle.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-24
 * Description : noise reduction image filter for digiKam 
 *               image editor.
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
#include <qcheckbox.h>
#include <qstring.h>
#include <qimage.h>
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
#include <kdebug.h>
#include <kstandarddirs.h>
#include <knuminput.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "despeckle.h"
#include "imageeffect_despeckle.h"

namespace DigikamNoiseReductionImagesPlugin
{

ImageEffect_Despeckle::ImageEffect_Despeckle(QWidget* parent)
                     : KDialogBase(Plain, i18n("Noise Reduction"), Help|User1|Ok|Cancel, Ok,
                                   parent, 0, true, true, i18n("&Reset Values")),
                       m_parent(parent)
{
    m_currentRenderingMode = NoneRendering;
    m_timer                = 0L;
    m_despeckleFilter      = 0L;
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
        
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Noise Reduction"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A despeckle image filter plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Michael Sweet", I18N_NOOP("Despeckle algorithm author from Gimp"),
                     "mike at easysw.com");
                         
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Noise Reduction Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Noise Reduction"), headerFrame, "labelTitle" );
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

    QGroupBox *gboxSettings = new QGroupBox(i18n("Settings"), m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 5, 2, 20, spacingHint());    
    
    QLabel *label1 = new QLabel(i18n("Radius:"), gboxSettings);
    
    m_radiusInput = new KIntNumInput(gboxSettings, "m_radiusInput");
    m_radiusInput->setRange(1, 20, 1, true);
    
    QWhatsThis::add( m_radiusInput, i18n("<p>A radius of 0 has no effect, "
                     "1 and above determine the blur matrix radius "
                     "that determines how much to blur the image.") );
    
    gridSettings->addWidget(label1, 0, 0);
    gridSettings->addWidget(m_radiusInput, 0, 1);
    
    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Black level:"), gboxSettings);
    
    m_blackLevelInput = new KIntNumInput(gboxSettings, "m_blackLevelInput");
    m_blackLevelInput->setRange(0, 255, 1, true);
    
    QWhatsThis::add( m_blackLevelInput, i18n("<p>This value controls the black "
                     "levels used by the adaptive filter to "
                     "adjust the filter radius.") );

    gridSettings->addWidget(label2, 1, 0);
    gridSettings->addWidget(m_blackLevelInput, 1, 1);                         
    
    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("White level:"), gboxSettings);
    
    m_whiteLevelInput = new KIntNumInput(gboxSettings, "m_blackLevelInput");
    m_whiteLevelInput->setRange(0, 255, 1, true);
    
    QWhatsThis::add( m_whiteLevelInput, i18n("<p>This value controls the white "
                     "levels used by the adaptive filter to "
                     "adjust the filter radius.") );

    gridSettings->addWidget(label3, 3, 0);
    gridSettings->addWidget(m_whiteLevelInput, 3, 1);
                                              
    // -------------------------------------------------------------
    
    m_useAdaptativeMethod = new QCheckBox( i18n("Adaptive"), gboxSettings);
    QWhatsThis::add( m_useAdaptativeMethod, i18n("<p>This option use an adaptive median filter type."));
    
    m_useRecursiveMethod = new QCheckBox( i18n("Recursive"), gboxSettings);
    QWhatsThis::add( m_useRecursiveMethod, i18n("<p>This option use a recursive median filter type.")); 
    
    gridSettings->addMultiCellWidget(m_useAdaptativeMethod, 4, 4, 0, 1);
    gridSettings->addMultiCellWidget(m_useRecursiveMethod, 4, 4, 1, 1);    
        
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
            
    // To prevent both computation (resize event and Reset to default settings).
    m_imagePreviewWidget->blockSignals(true);
    resize(configDialogSize("Noise Reduction Tool Dialog"));     
    m_imagePreviewWidget->blockSignals(false);     
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
    
    // -------------------------------------------------------------
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));            
            
    connect(m_blackLevelInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_whiteLevelInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));                                                
            
    connect(m_useAdaptativeMethod, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));             
    
    connect(m_useRecursiveMethod, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));             
}

ImageEffect_Despeckle::~ImageEffect_Despeckle()
{
    saveDialogSize("Noise Reduction Tool Dialog");   

    if (m_despeckleFilter)
       delete m_despeckleFilter;    
    
    if (m_timer)
       delete m_timer;
}

void ImageEffect_Despeckle::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_radiusInput->setEnabled(true);
    m_blackLevelInput->setEnabled(true);
    m_whiteLevelInput->setEnabled(true);
    m_useAdaptativeMethod->setEnabled(true);
    m_useRecursiveMethod->setEnabled(true);
    m_imagePreviewWidget->setEnable(true);   
    enableButton(Ok, true);  
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all filter parameters to their default values.") );
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
}

void ImageEffect_Despeckle::slotHelp()
{
    KApplication::kApplication()->invokeHelp("despeckle", "digikamimageplugins");
}

void ImageEffect_Despeckle::slotFocusChanged(void)
{
    if (m_currentRenderingMode == FinalRendering)
       {
       m_imagePreviewWidget->update();
       return;
       }
    else if (m_currentRenderingMode == PreviewRendering)
       {
       m_despeckleFilter->stopComputation();
       }
       
    QTimer::singleShot(0, this, SLOT(slotEffect()));        
}

void ImageEffect_Despeckle::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_despeckleFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    e->accept();   
}

void ImageEffect_Despeckle::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_despeckleFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    done(Cancel);
}

void ImageEffect_Despeckle::slotTimer()
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

void ImageEffect_Despeckle::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_despeckleFilter->stopComputation();
       }
    else
       {
       m_radiusInput->blockSignals(true);
       m_blackLevelInput->blockSignals(true);
       m_whiteLevelInput->blockSignals(true);
       m_useAdaptativeMethod->blockSignals(true);
       m_useRecursiveMethod->blockSignals(true);
                       
       m_radiusInput->setValue(3);
       m_blackLevelInput->setValue(7);
       m_whiteLevelInput->setValue(248);
       m_useAdaptativeMethod->setChecked( true );
       m_useRecursiveMethod->setChecked(false);
        
       m_radiusInput->blockSignals(false);
       m_blackLevelInput->blockSignals(false);
       m_whiteLevelInput->blockSignals(false);
       m_useAdaptativeMethod->blockSignals(false);
       m_useRecursiveMethod->blockSignals(false);
       slotEffect();
       }
} 

void ImageEffect_Despeckle::slotEffect()
{
    // Computation already in progress.
    if (m_currentRenderingMode == PreviewRendering) return;     
    
    m_currentRenderingMode = PreviewRendering;
    
    m_radiusInput->setEnabled(false);
    m_blackLevelInput->setEnabled(false);
    m_whiteLevelInput->setEnabled(false);
    m_useAdaptativeMethod->setEnabled(false);
    m_useRecursiveMethod->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
        
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    QImage img = m_imagePreviewWidget->getOriginalClipImage();
   
    int  r  = m_radiusInput->value();
    int  bl = m_blackLevelInput->value();
    int  wl = m_whiteLevelInput->value();
    bool af = m_useAdaptativeMethod->isChecked();
    bool rf = m_useRecursiveMethod->isChecked();
    
    m_imagePreviewWidget->setProgress(0);          

    if (m_despeckleFilter)
       delete m_despeckleFilter;
        
    m_despeckleFilter = new Despeckle(&img, this, r, bl, wl, af, rf);
}

void ImageEffect_Despeckle::slotOk()
{
    m_currentRenderingMode = FinalRendering;
    
    m_radiusInput->setEnabled(false);
    m_blackLevelInput->setEnabled(false);
    m_whiteLevelInput->setEnabled(false);
    m_useAdaptativeMethod->setEnabled(false);
    m_useRecursiveMethod->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    kapp->setOverrideCursor( KCursor::waitCursor() );
    
    int  r  = m_radiusInput->value();
    int  bl = m_blackLevelInput->value();
    int  wl = m_whiteLevelInput->value();
    bool af = m_useAdaptativeMethod->isChecked();
    bool rf = m_useRecursiveMethod->isChecked();

    m_imagePreviewWidget->setProgress(0);             
    
    if (m_despeckleFilter)
       delete m_despeckleFilter;
    
    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
            
    m_despeckleFilter = new Despeckle(&orgImage, this, r, bl, wl, af, rf);
           
    delete [] data;
}

void ImageEffect_Despeckle::customEvent(QCustomEvent *event)
{
    if (!event) return;

    Despeckle::EventData *d = (Despeckle::EventData*) event->data();

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
                 kdDebug() << "Preview Noise Reduction completed..." << endl;
                 
                 QImage imDest = m_despeckleFilter->getTargetImage();
                 m_imagePreviewWidget->setPreviewImageData(imDest);
    
                 abortPreview();
                 break;
                 }
              
              case FinalRendering:
                 {
                 kdDebug() << "Final Noise Reduction completed..." << endl;
                 
                 Digikam::ImageIface iface(0, 0);
  
                 iface.putOriginalData(i18n("Noise Reduction"), 
                                       (uint*)m_despeckleFilter->getTargetImage().bits());
                    
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
                    kdDebug() << "Preview Noise Reduction failed..." << endl;
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

}  // NameSpace DigikamNoiseReductionImagesPlugin

#include "imageeffect_despeckle.moc"
