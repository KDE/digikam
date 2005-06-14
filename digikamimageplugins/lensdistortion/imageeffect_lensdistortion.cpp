/* ============================================================
 * File  : imageeffect_lensdistortion.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-27
 * Description : a digiKam image plugin for to reduce spherical
 *               aberration provide by lens on an image.
 * 
 * Copyright 2004-2005 by Gilles Caulier
 *
 * Original Distortion Correction algorithm copyrighted 
 * 2001-2003 David Hodson <hodsond@acm.org>
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
#include <qpixmap.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>
#include <qframe.h>
#include <qtimer.h>

// KDE includes.

#include <kdebug.h>
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
#include "lensdistortion.h"
#include "imageeffect_lensdistortion.h"

namespace DigikamLensDistortionImagesPlugin
{

ImageEffect_LensDistortion::ImageEffect_LensDistortion(QWidget* parent)
                          : KDialogBase(Plain, i18n("Lens Distortion Correction"),
                                        Help|User1|Ok|Cancel, Ok,
                                        parent, 0, true, true, i18n("&Reset Values")),
                            m_parent(parent)
{
    m_currentRenderingMode = NoneRendering;
    m_timer                = 0L;
    m_lensdistortionFilter = 0L;
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Lens Distortion Correction"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to reduce spherical aberration caused "
                                                 "by a lens to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("David Hodson", I18N_NOOP("Lens distortion correction algorithm."),
                     "hodsond at acm dot org");
                     
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Lens Distortion Correction Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------
        
    QGridLayout* topLayout = new QGridLayout( plainPage(), 1, 2 , marginHint(), spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Lens Distortion Correction"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addMultiCellWidget(headerFrame, 0, 0, 0, 2);

    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );

    // -------------------------------------------------------------
        
    QVGroupBox *gbox = new QVGroupBox(i18n("Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* l = new QHBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageGuideWidget(480, 320, frame);
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the preview of the spherical aberration correction. "
                                           "If you move the mouse cursor on this preview, "
                                           "a vertical and horizontal dashed line will be draw "
                                           "to guide you in adjusting the lens distortion correction. "
                                           "Press the left mouse button to freeze the dashed "
                                           "line's position."));
    topLayout->addMultiCellWidget(gbox, 1, 1, 0, 1);
    
    // -------------------------------------------------------------
                                                  
    QGroupBox *gbox2 = new QGroupBox(i18n("Settings"), plainPage());
    QGridLayout *gridBox2 = new QGridLayout( gbox2, 5, 2, 20, spacingHint());

    m_maskPreviewLabel = new QLabel( gbox2 );
    m_maskPreviewLabel->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    QWhatsThis::add( m_maskPreviewLabel, i18n("<p>You can see here a thumbnail preview of the distortion correction "
                                              "applied to a cross pattern.") );
    gridBox2->addMultiCellWidget(m_maskPreviewLabel, 0, 0, 0, 2);
        
    // -------------------------------------------------------------
    
    QLabel *label1 = new QLabel(i18n("Main:"), gbox2);
    
    m_mainInput = new KDoubleNumInput(gbox2);
    m_mainInput->setPrecision(1);
    m_mainInput->setRange(-100.0, 100.0, 0.1, true);
    QWhatsThis::add( m_mainInput, i18n("<p>This value controls the amount of distortion. Negative values correct lens barrel "
                                       "distortion, while positive values correct lens pincushion distortion."));

    gridBox2->addMultiCellWidget(label1, 1, 1, 0, 0);
    gridBox2->addMultiCellWidget(m_mainInput, 1, 1, 1, 2);
    
    // -------------------------------------------------------------
    
    QLabel *label2 = new QLabel(i18n("Edge:"), gbox2);
    
    m_edgeInput = new KDoubleNumInput(gbox2);
    m_edgeInput->setPrecision(1);
    m_edgeInput->setRange(-100.0, 100.0, 0.1, true);
    QWhatsThis::add( m_edgeInput, i18n("<p>This value controls in the same manner as the Main control, but has more effect "
                                       "at the edges of the image than at the center."));

    gridBox2->addMultiCellWidget(label2, 2, 2, 0, 0);
    gridBox2->addMultiCellWidget(m_edgeInput, 2, 2, 1, 2);
    
    // -------------------------------------------------------------
    
    QLabel *label3 = new QLabel(i18n("Zoom:"), gbox2);
    
    m_rescaleInput = new KDoubleNumInput(gbox2);
    m_rescaleInput->setPrecision(1);
    m_rescaleInput->setRange(-100.0, 100.0, 0.1, true);
    QWhatsThis::add( m_rescaleInput, i18n("<p>This value rescales the overall image size."));
    
    gridBox2->addMultiCellWidget(label3, 3, 3, 0, 0);
    gridBox2->addMultiCellWidget(m_rescaleInput, 3, 3, 1, 2);

    // -------------------------------------------------------------
    
    QLabel *label4 = new QLabel(i18n("Brighten:"), gbox2);
    
    m_brightenInput = new KDoubleNumInput(gbox2);
    m_brightenInput->setPrecision(1);
    m_brightenInput->setRange(-100.0, 100.0, 0.1, true);
    QWhatsThis::add( m_brightenInput, i18n("<p>This value adjust the brightness in image corners."));

    gridBox2->addMultiCellWidget(label4, 4, 4, 0, 0);
    gridBox2->addMultiCellWidget(m_brightenInput, 4, 4, 1, 2);
    
    // -------------------------------------------------------------
    
    m_progressBar = new KProgress(100, gbox2, "progressbar");
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    gridBox2->addMultiCellWidget(m_progressBar, 5, 5, 0, 2);

    topLayout->addMultiCellWidget(gbox2, 1, 1, 2, 2);
    
    // -------------------------------------------------------------

    adjustSize();
    disableResize();  
    QTimer::singleShot(0, this, SLOT(slotUser1()));     // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect(m_mainInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));            
            
    connect(m_edgeInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));            

    connect(m_rescaleInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));            

    connect(m_brightenInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));            
}

ImageEffect_LensDistortion::~ImageEffect_LensDistortion()
{
    if (m_lensdistortionFilter)
       delete m_lensdistortionFilter;    
    
    if (m_timer)
       delete m_timer;
}

void ImageEffect_LensDistortion::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_progressBar->setValue(0); 
    m_mainInput->setEnabled(true);
    m_edgeInput->setEnabled(true);
    m_rescaleInput->setEnabled(true);
    m_brightenInput->setEnabled(true);
    enableButton(Ok, true);  
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all filter parameters to their default values.") );
}

void ImageEffect_LensDistortion::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_lensdistortionFilter->stopComputation();
       m_parent->setCursor( KCursor::arrowCursor() );
       }
    
    done(Cancel);
}

void ImageEffect_LensDistortion::slotHelp()
{
    KApplication::kApplication()->invokeHelp("lensdistortion", "digikamimageplugins");
}

void ImageEffect_LensDistortion::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_lensdistortionFilter->stopComputation();
       m_parent->setCursor( KCursor::arrowCursor() );
       }
    
    e->accept();    
}

void ImageEffect_LensDistortion::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_lensdistortionFilter->stopComputation();
       }
    else
       {
       m_mainInput->blockSignals(true);
       m_edgeInput->blockSignals(true);
       m_rescaleInput->blockSignals(true);
       m_brightenInput->blockSignals(true);
        
       m_mainInput->setValue(0.0);
       m_edgeInput->setValue(0.0);
       m_rescaleInput->setValue(0.0);
       m_brightenInput->setValue(0.0);
        
       m_mainInput->blockSignals(false);
       m_edgeInput->blockSignals(false);
       m_rescaleInput->blockSignals(false);
       m_brightenInput->blockSignals(false);
    
       slotEffect();
       }
} 

void ImageEffect_LensDistortion::slotTimer()
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

void ImageEffect_LensDistortion::slotEffect()
{
    // Computation already in process.
    if (m_currentRenderingMode == PreviewRendering) return;     
    
    m_currentRenderingMode = PreviewRendering;

    m_mainInput->setEnabled(false);
    m_edgeInput->setEnabled(false);
    m_rescaleInput->setEnabled(false);
    m_brightenInput->setEnabled(false);
    
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);

    double m = m_mainInput->value();
    double e = m_edgeInput->value();
    double r = m_rescaleInput->value();
    double b = m_brightenInput->value();

    m_progressBar->setValue(0); 

    // Calc transform preview.    
    QImage preview(120, 120, 32);
    memset(preview.bits(), 255, preview.numBytes());
    QPixmap pix (preview);
    QPainter pt(&pix);
    pt.setPen( QPen::QPen(Qt::black, 1) ); 
    pt.fillRect( 0, 0, pix.width(), pix.height(), QBrush::QBrush(Qt::black, Qt::CrossPattern) );
    pt.drawRect( 0, 0, pix.width(), pix.height() );
    pt.end();
    QImage preview2(pix.convertToImage());
    LensDistortion transformPreview(&preview2, 0L, m, e, r, b, 0, 0);
    m_maskPreviewLabel->setPixmap(QPixmap::QPixmap(transformPreview.getTargetImage()));

    m_progressBar->setValue(0); 

    if (m_lensdistortionFilter)
       delete m_lensdistortionFilter;
    
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    QImage orgImage(iface->originalWidth(), iface->originalHeight(), 32);
    uint *data = iface->getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    m_lensdistortionFilter = new LensDistortion(&orgImage, this, m, e, r, b, 0, 0);
    
    delete [] data;
}

void ImageEffect_LensDistortion::slotOk()
{
    m_currentRenderingMode = FinalRendering;
    
    m_mainInput->setEnabled(false);
    m_edgeInput->setEnabled(false);
    m_rescaleInput->setEnabled(false);
    m_brightenInput->setEnabled(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    
    m_parent->setCursor( KCursor::waitCursor() );
    
    double m = m_mainInput->value();
    double e = m_edgeInput->value();
    double r = m_rescaleInput->value();
    double b = m_brightenInput->value();

    m_progressBar->setValue(0); 
        
    if (m_lensdistortionFilter)
       delete m_lensdistortionFilter;

    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    m_lensdistortionFilter = new LensDistortion(&orgImage, this, m, e, r, b, 0, 0);
           
    delete [] data;       
}

void ImageEffect_LensDistortion::customEvent(QCustomEvent *event)
{
    if (!event) return;

    LensDistortion::EventData *d = (LensDistortion::EventData*) event->data();

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
                 kdDebug() << "Preview LensDistortion completed..." << endl;
                 Digikam::ImageIface* iface = m_previewWidget->imageIface();
                 
                 QImage imDest = m_lensdistortionFilter->getTargetImage();
                 iface->putPreviewData((uint*)(imDest.smoothScale(iface->previewWidth(),
                                                                  iface->previewHeight())).bits());
                 
                 m_previewWidget->update();
                 abortPreview();
                 break;
                 }
              
              case FinalRendering:
                 {
                 kdDebug() << "Final LensDistortion completed..." << endl;
                 
                 Digikam::ImageIface iface(0, 0);    
     
                 iface.putOriginalData(i18n("Lens Distortion"), 
                                       (uint*)m_lensdistortionFilter->getTargetImage().bits());
                                                           
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
                    kdDebug() << "Preview LensDistortion failed..." << endl;
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

}  // NameSpace DigikamLensDistortionImagesPlugin

#include "imageeffect_lensdistortion.moc"
