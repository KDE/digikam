/* ============================================================
 * File  : imageeffect_filmgrain.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a digiKam image editor plugin for add film 
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
#include <qtimer.h>
#include <qimage.h>

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
#include "filmgrain.h"
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
    m_currentRenderingMode = NoneRendering;
    m_filmgrainFilter      = 0L;
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    resize(configDialogSize("Film Grain Tool Dialog")); 
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Film Grain"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to apply a film grain "
                                                 "effect to an image."),
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
    QLabel *labelTitle = new QLabel( i18n("Add Film Grain to Photograph"), headerFrame, "labelTitle" );
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
    m_imagePreviewWidget->setProgressWhatsThis(i18n("<p>This is the current percentage of "
                                                    "the task completed."));
    
    // -------------------------------------------------------------
    
    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 1, 2, marginHint(), spacingHint());
    QLabel *label1 = new QLabel(i18n("Sensibility (ISO):"), gboxSettings);
    
    m_sensibilitySlider = new QSlider(2, 30, 1, 12, Qt::Horizontal, gboxSettings);
    m_sensibilitySlider->setTracking ( false );
    m_sensibilitySlider->setTickInterval(1);
    m_sensibilitySlider->setTickmarks(QSlider::Below);
    
    m_sensibilityLCDValue = new QLCDNumber (4, gboxSettings);
    m_sensibilityLCDValue->setSegmentStyle ( QLCDNumber::Flat );
    m_sensibilityLCDValue->display( QString::number(2400) );
    whatsThis = i18n("<p>Set here the film ISO-sensitivity to use for simulating the film graininess.");
        
    QWhatsThis::add( m_sensibilityLCDValue, whatsThis);
    QWhatsThis::add( m_sensibilitySlider, whatsThis);

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_sensibilitySlider, 0, 0, 1, 1);
    gridSettings->addMultiCellWidget(m_sensibilityLCDValue, 0, 0, 2, 2);    
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect( m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
             this, SLOT(slotFocusChanged()) );
    
    connect( m_sensibilitySlider, SIGNAL(valueChanged(int)),
             this, SLOT(slotSensibilityChanged(int)) ); 
}

ImageEffect_FilmGrain::~ImageEffect_FilmGrain()
{
    saveDialogSize("Film Grain Tool Dialog");    
    
    if (m_filmgrainFilter)
       delete m_filmgrainFilter;   
}

void ImageEffect_FilmGrain::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_sensibilitySlider->setEnabled(true);
    m_imagePreviewWidget->setEnable(true);    
    enableButton(Ok, true);  
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all filter parameters to their default values.") );
}
 
void ImageEffect_FilmGrain::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_filmgrainFilter->stopComputation();
       }
    else
       {
       m_sensibilitySlider->blockSignals(true);
       m_sensibilitySlider->setValue(12);
       m_sensibilitySlider->blockSignals(false);
       slotEffect();    
       }
} 

void ImageEffect_FilmGrain::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_filmgrainFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    done(Cancel);
}

void ImageEffect_FilmGrain::slotHelp()
{
    KApplication::kApplication()->invokeHelp("filmgrain", "digikamimageplugins");
}

void ImageEffect_FilmGrain::slotFocusChanged(void)
{
    if (m_currentRenderingMode == FinalRendering)
       {
       m_imagePreviewWidget->update();
       return;
       }
    else if (m_currentRenderingMode == PreviewRendering)
       {
       m_filmgrainFilter->stopComputation();
       }
       
    QTimer::singleShot(0, this, SLOT(slotEffect()));        
}

void ImageEffect_FilmGrain::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_filmgrainFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    e->accept();    
}

void ImageEffect_FilmGrain::slotSensibilityChanged(int v)
{
    m_sensibilityLCDValue->display( QString::number(400+200*v) );
    slotEffect();
}

void ImageEffect_FilmGrain::slotEffect()
{
    // Computation already in process.
    if (m_currentRenderingMode == PreviewRendering) return;     
    
    m_currentRenderingMode = PreviewRendering;
    m_sensibilitySlider->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);  
    
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    int s = 400 + 200 * m_sensibilitySlider->value();
    
    m_imagePreviewWidget->setProgress(0);
    
    if (m_filmgrainFilter)
       delete m_filmgrainFilter;
        
    m_filmgrainFilter = new FilmGrain(&image, this, s);
}

void ImageEffect_FilmGrain::slotOk()
{
    m_currentRenderingMode = FinalRendering;

    m_sensibilitySlider->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    kapp->setOverrideCursor( KCursor::waitCursor() );
        
    int s = 400 + 200 * m_sensibilitySlider->value();
            
    if (m_filmgrainFilter)
       delete m_filmgrainFilter;
               
    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    m_filmgrainFilter = new FilmGrain(&orgImage, this, s);
           
    delete [] data;
}

void ImageEffect_FilmGrain::customEvent(QCustomEvent *event)
{
    if (!event) return;

    FilmGrain::EventData *d = (FilmGrain::EventData*) event->data();

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
                 kdDebug() << "Preview Film Grain completed..." << endl;
                 
                 QImage imDest = m_filmgrainFilter->getTargetImage();
                 m_imagePreviewWidget->setPreviewImageData(imDest);
    
                 abortPreview();
                 break;
                 }
              
              case FinalRendering:
                 {
                 kdDebug() << "Final Film Grain completed..." << endl;
                 
                 Digikam::ImageIface iface(0, 0);
  
                 iface.putOriginalData(i18n("Film Grain"), 
                                       (uint*)m_filmgrainFilter->getTargetImage().bits());
                    
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
                    kdDebug() << "Preview Film Grain failed..." << endl;
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

}  // NameSpace DigikamFilmGrainImagesPlugin

#include "imageeffect_filmgrain.moc"
