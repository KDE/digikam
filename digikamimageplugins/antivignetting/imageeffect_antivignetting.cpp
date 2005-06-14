/* ============================================================
 * File  : imageeffect_antivignetting.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-25
 * Description : a digiKam image plugin for to reduce 
 *               vignetting on an image.
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
#include <qimage.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qpen.h>
#include <qtimer.h>
#include <qtabwidget.h>

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
#include "antivignetting.h"
#include "imageeffect_antivignetting.h"


namespace DigikamAntiVignettingImagesPlugin
{

ImageEffect_AntiVignetting::ImageEffect_AntiVignetting(QWidget* parent)
                          : KDialogBase(Plain, i18n("Anti Vignetting"),
                                        Help|User1|Ok|Cancel, Ok,
                                        parent, 0, true, true, i18n("&Reset Values")),
                            m_parent(parent)
{
    m_currentRenderingMode = NoneRendering;
    m_timer                = 0L;
    m_antivignettingFilter = 0L;
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Anti-Vignetting"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to reduce image vignetting."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("John Walker", I18N_NOOP("Anti Vignetting algorithm"), 0,
                     "http://www.fourmilab.ch/netpbm/pnmctrfilt"); 
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Anti Vignetting Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Anti Vignetting"), headerFrame, "labelTitle" );
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
    
    QVGroupBox *gbox = new QVGroupBox(i18n("Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320, frame);
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the anti-vignetting filter preview.") );
    topLayout->addWidget(gbox);
    
    // -------------------------------------------------------------
    
    m_mainTab = new QTabWidget( plainPage() );
    
    QWidget* firstPage = new QWidget( m_mainTab );
    QGridLayout* grid = new QGridLayout( firstPage, 3, 3, marginHint(), spacingHint());
    m_mainTab->addTab( firstPage, i18n("Filter Settings") );
    
    m_maskPreviewLabel = new QLabel( firstPage );
    QWhatsThis::add( m_maskPreviewLabel, i18n("<p>You can see here a thumbnail preview of the anti-vignetting "
                                              "mask applied to the image.") );
    grid->addMultiCellWidget(m_maskPreviewLabel, 0, 2, 0, 0);
    
    // -------------------------------------------------------------
                      
    QLabel *label1 = new QLabel(i18n("Density:"), firstPage);

    m_densityInput = new KDoubleNumInput(firstPage);
    m_densityInput->setPrecision(1);
    m_densityInput->setRange(1.0, 20.0, 0.1, true);
    QWhatsThis::add( m_densityInput, i18n("<p>This value controls the degree of intensity attenuation by the filter "
                                          "at its point of maximum density."));
    
    grid->addMultiCellWidget(label1, 0, 0, 1, 1);
    grid->addMultiCellWidget(m_densityInput, 0, 0, 2, 3);
    
    // -------------------------------------------------------------
    
    QLabel *label2 = new QLabel(i18n("Power:"), firstPage);
    
    m_powerInput = new KDoubleNumInput(firstPage);
    m_powerInput->setPrecision(1);
    m_powerInput->setRange(0.1, 2.0, 0.1, true);
    QWhatsThis::add( m_powerInput, i18n("<p>This value is used as the exponent controlling the fall-off in density "
                                        "from the center of the filter to the periphery."));
    
    grid->addMultiCellWidget(label2, 1, 1, 1, 1);
    grid->addMultiCellWidget(m_powerInput, 1, 1, 2, 3);
    
    // -------------------------------------------------------------
    
    QLabel *label3 = new QLabel(i18n("Radius:"), firstPage);
    
    m_radiusInput = new KDoubleNumInput(firstPage);
    m_radiusInput->setPrecision(1);
    m_radiusInput->setRange(0.1, 2.0, 0.1, true);
    QWhatsThis::add( m_radiusInput, i18n("<p>This value is the radius of the center filter. It is a multiple of the "
                                         "half-diagonal measure of the image, at which the density of the filter falls to zero."));
    
    grid->addMultiCellWidget(label3, 2, 2, 1, 1);
    grid->addMultiCellWidget(m_radiusInput, 2, 2, 2, 3);
    
    // -------------------------------------------------------------

    QWidget* secondPage = new QWidget( m_mainTab );
    QGridLayout* grid2 = new QGridLayout( secondPage, 3, 3, marginHint(), spacingHint());
    m_mainTab->addTab( secondPage, i18n("Exposure Re-Adjustment") );
    
    QLabel *label4 = new QLabel(i18n("Brightness:"), secondPage);
    
    m_brightnessInput = new KIntNumInput(secondPage);
    m_brightnessInput->setRange(0, 100, 1, true);  
    QWhatsThis::add( m_brightnessInput, i18n("<p>Set here the brightness re-adjustment of the target image."));

    grid2->addMultiCellWidget(label4, 0, 0, 0, 0);
    grid2->addMultiCellWidget(m_brightnessInput, 0, 0, 1, 2);
    
    // -------------------------------------------------------------
    
    QLabel *label5 = new QLabel(i18n("Contrast:"), secondPage);
    
    m_contrastInput = new KIntNumInput(secondPage);
    m_contrastInput->setRange(0, 100, 1, true);  
    QWhatsThis::add( m_contrastInput, i18n("<p>Set here the contrast re-adjustment of the target image."));

    grid2->addMultiCellWidget(label5, 1, 1, 0, 0);
    grid2->addMultiCellWidget(m_contrastInput, 1, 1, 1, 2);
    
    // -------------------------------------------------------------

    QLabel *label6 = new QLabel(i18n("Gamma:"), secondPage);
    
    m_gammaInput = new KIntNumInput(secondPage);
    m_gammaInput->setRange(0, 100, 1, true);  
    QWhatsThis::add( m_gammaInput, i18n("<p>Set here the gamma re-adjustment of the target image."));

    grid2->addMultiCellWidget(label6, 2, 2, 0, 0);
    grid2->addMultiCellWidget(m_gammaInput, 2, 2, 1, 2);

    topLayout->addWidget(m_mainTab);
    
    // -------------------------------------------------------------
            
    m_progressBar = new KProgress(100, plainPage(), "m_progressbar");
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    topLayout->addWidget(m_progressBar);
    
    // -------------------------------------------------------------

    adjustSize();
    disableResize();  
    QTimer::singleShot(0, this, SLOT(slotUser1()));     // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect(m_densityInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));            

    connect(m_powerInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));            

    connect(m_radiusInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));            

    connect(m_brightnessInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));            

    connect(m_contrastInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));            

    connect(m_gammaInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));            
}

ImageEffect_AntiVignetting::~ImageEffect_AntiVignetting()
{
    if (m_antivignettingFilter)
       delete m_antivignettingFilter;    
    
    if (m_timer)
       delete m_timer;
}

void ImageEffect_AntiVignetting::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_progressBar->setValue(0); 
    m_densityInput->setEnabled(true);
    m_powerInput->setEnabled(true);
    m_radiusInput->setEnabled(true);
    m_brightnessInput->setEnabled(true);
    m_contrastInput->setEnabled(true);
    m_gammaInput->setEnabled(true);
    enableButton(Ok, true);  
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all filter parameters to their default values.") );
}

void ImageEffect_AntiVignetting::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_antivignettingFilter->stopComputation();
       }
    else
       {
       m_densityInput->blockSignals(true);
       m_powerInput->blockSignals(true);
       m_radiusInput->blockSignals(true);
       m_brightnessInput->blockSignals(true);
       m_contrastInput->blockSignals(true);
       m_gammaInput->blockSignals(true);
      
       m_densityInput->setValue(2.0);
       m_powerInput->setValue(1.0);
       m_radiusInput->setValue(1.0);
       m_brightnessInput->setValue(0);
       m_contrastInput->setValue(0);
       m_gammaInput->setValue(0);
    
       m_densityInput->blockSignals(false);
       m_powerInput->blockSignals(false);
       m_radiusInput->blockSignals(false);
       m_brightnessInput->blockSignals(false);
       m_contrastInput->blockSignals(false);
       m_gammaInput->blockSignals(false);

       slotEffect();
       }
} 

void ImageEffect_AntiVignetting::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_antivignettingFilter->stopComputation();
       m_parent->setCursor( KCursor::arrowCursor() );
       }
       
    done(Cancel);
}

void ImageEffect_AntiVignetting::slotHelp()
{
    KApplication::kApplication()->invokeHelp("antivignettings", "digikamimageplugins");
}

void ImageEffect_AntiVignetting::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_antivignettingFilter->stopComputation();
       m_parent->setCursor( KCursor::arrowCursor() );
       }
    
    e->accept();    
}

void ImageEffect_AntiVignetting::slotTimer()
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

void ImageEffect_AntiVignetting::slotEffect()
{
    // Computation already in process.
    if (m_currentRenderingMode == PreviewRendering) return;     
    
    m_currentRenderingMode = PreviewRendering;
    
    m_densityInput->setEnabled(false);
    m_powerInput->setEnabled(false);
    m_radiusInput->setEnabled(false);
    m_brightnessInput->setEnabled(false);
    m_contrastInput->setEnabled(false);
    m_gammaInput->setEnabled(false);
    
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
    
    double d = m_densityInput->value();
    double p = m_powerInput->value();
    double r = m_radiusInput->value();

    // Calc mask preview.    
    QImage preview(90, 90, 32);
    memset(preview.bits(), 255, preview.numBytes());
    AntiVignetting maskPreview(&preview, 0L, d, p, r, 0, 0, false);
    QPixmap pix (maskPreview.getTargetImage());
    QPainter pt(&pix);
    pt.setPen( QPen::QPen(Qt::black, 1) ); 
    pt.drawRect( 0, 0, pix.width(), pix.height() );
    pt.end();
    m_maskPreviewLabel->setPixmap( pix );
    
    m_progressBar->setValue(0); 

    if (m_antivignettingFilter)
       delete m_antivignettingFilter;
    
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    QImage orgImage(iface->originalWidth(), iface->originalHeight(), 32);
    uint *data = iface->getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    m_antivignettingFilter = new AntiVignetting(&orgImage, this, d, p, r, 0, 0, true);
    
    delete [] data;
}

void ImageEffect_AntiVignetting::slotOk()
{
    m_currentRenderingMode = FinalRendering;
    
    m_densityInput->setEnabled(false);
    m_powerInput->setEnabled(false);
    m_radiusInput->setEnabled(false);
    m_brightnessInput->setEnabled(false);
    m_contrastInput->setEnabled(false);
    m_gammaInput->setEnabled(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    
    m_parent->setCursor( KCursor::waitCursor() );

    double d = m_densityInput->value();
    double p = m_powerInput->value();
    double r = m_radiusInput->value();

    m_progressBar->setValue(0); 

    if (m_antivignettingFilter)
       delete m_antivignettingFilter;

    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    
    m_antivignettingFilter = new AntiVignetting(&orgImage, this, d, p, r, 0, 0, true);
           
    delete [] data;       
}

void ImageEffect_AntiVignetting::customEvent(QCustomEvent *event)
{
    if (!event) return;

    AntiVignetting::EventData *d = (AntiVignetting::EventData*) event->data();

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
                 kdDebug() << "Preview AntiVignetting completed..." << endl;
                 Digikam::ImageIface* iface = m_previewWidget->imageIface();
                 
                 QImage imDest = m_antivignettingFilter->getTargetImage();
                 iface->putPreviewData((uint*)(imDest.smoothScale(iface->previewWidth(),
                                                                  iface->previewHeight())).bits());
                 
                 double b   = (double)(m_brightnessInput->value() / 100.0);
                 double c   = (double)(m_contrastInput->value() / 100.0) + (double)(1.00);    
                 double g   = (double)(m_gammaInput->value() / 100.0) + (double)(1.00);
    
                 // Adjust Image BCG.
                 iface->setPreviewBCG(b, c, g);
                 
                 m_previewWidget->update();
                 abortPreview();
                 break;
                 }
              
              case FinalRendering:
                 {
                 kdDebug() << "Final AntiVignetting completed..." << endl;
                 
                 Digikam::ImageIface iface(0, 0);    
     
                 iface.putOriginalData(i18n("Anti-Vignetting"), 
                                       (uint*)m_antivignettingFilter->getTargetImage().bits());
                 
                 double b   = (double)(m_brightnessInput->value() / 100.0);
                 double c   = (double)(m_contrastInput->value() / 100.0) + (double)(1.00);    
                 double g   = (double)(m_gammaInput->value() / 100.0) + (double)(1.00);
    
                 // Adjust Image BCG.
                 iface.setPreviewBCG(b, c, g);
                                                           
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
                    kdDebug() << "Preview AntiVignetting failed..." << endl;
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

}  // NameSpace DigikamAntiVignettingImagesPlugin

#include "imageeffect_antivignetting.moc"
