/* ============================================================
 * File  : imageeffect_restoration.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-26
 * Description : a digiKam image editor plugin to restore 
 *               a photograph
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

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cerrno>

// Qt includes.

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qevent.h>
#include <qfile.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kdebug.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "cimgiface.h"
#include "imageeffect_restoration.h"

namespace DigikamRestorationImagesPlugin
{

ImageEffect_Restoration::ImageEffect_Restoration(QWidget* parent)
                       : KDialogBase(Plain, i18n("Restoration"),
                                     Help|User1|User2|User3|Ok|Cancel, Ok,
                                     parent, 0, true, true,
                                     i18n("&Reset Values"),
                                     i18n("&Load..."),
                                     i18n("&Save...")),
                         m_parent(parent)
{
    QString whatsThis;
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    setButtonWhatsThis ( User2, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User3, i18n("<p>Save all filter parameters to settings text file.") );
    
    m_dirty                = false;    
    m_currentRenderingMode = NoneRendering;
    m_timer                = 0L;
    m_originalData         = 0L;
    m_cimgInterface        = 0L;

    m_iface          = new Digikam::ImageIface(0, 0);
    m_originalData   = m_iface->getOriginalData();
    m_originalWidth  = m_iface->originalWidth();
    m_originalHeight = m_iface->originalHeight();        
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Photograph Restoration"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to restore a photograph."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Victor Stinner", I18N_NOOP("CImg Gimp plugin"), 0,
                     "http://www.girouette-stinner.com/castor/gimp.html");

    about->addAuthor("David Tschumperle", I18N_NOOP("CImg library"), 0,
                     "http://cimg.sourceforge.net");
                        
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Photograph Restoration Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Photograph Restoration"), headerFrame, "labelTitle" );
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

    QVBoxLayout *vlay = new QVBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePreviewWidget(240, 160, i18n("Preview"), plainPage());
    vlay->addWidget(m_imagePreviewWidget);
    
    // -------------------------------------------------------------
    
    QTabWidget* mainTab = new QTabWidget( plainPage() );
    
    QWidget* firstPage = new QWidget( mainTab );
    QGridLayout* grid = new QGridLayout( firstPage, 1, 1, marginHint(), spacingHint());
    mainTab->addTab( firstPage, i18n("Preset") );

    QLabel *typeLabel = new QLabel(i18n("Filtering Type:"), firstPage);
    typeLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_restorationTypeCB = new QComboBox( false, firstPage ); 
    m_restorationTypeCB->insertItem( i18n("None") );
    m_restorationTypeCB->insertItem( i18n("Reduce Uniform Noise") );
    m_restorationTypeCB->insertItem( i18n("Reduce JPEG Artefacts") );
    m_restorationTypeCB->insertItem( i18n("Reduce Texturing") );
    QWhatsThis::add( m_restorationTypeCB, i18n("<p>Select here the filter preset to use for photograph restoration:<p>"
                                               "<b>None</b>: no preset values. Puts settings to default.<p>"
                                               "<b>Reduce Uniform Noise</b>: reduce small image artefacts like CCD noise.<p>"
                                               "<b>Reduce JPEG Artefacts</b>: reduce large image artefacts like JPEG compression mosaic.<p>"
                                               "<b>Reduce Texturing</b>: reduce image artefacts like paper texture "
                                               "of a scanned image.<p>"));

    grid->addMultiCellWidget(typeLabel, 0, 0, 0, 0);
    grid->addMultiCellWidget(m_restorationTypeCB, 0, 0, 1, 1);
    
    m_progressBar = new KProgress(100, firstPage);
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    grid->addMultiCellWidget(m_progressBar, 1, 1, 0, 1);
        
    // -------------------------------------------------------------
    
    QWidget* secondPage = new QWidget( mainTab );
    QGridLayout* grid2 = new QGridLayout( secondPage, 2, 4, marginHint(), spacingHint());
    mainTab->addTab( secondPage, i18n("Smoothing") );
    
    m_detailLabel = new QLabel(i18n("Detail Factor:"), secondPage);
    m_detailLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_detailInput = new KDoubleNumInput(secondPage);
    m_detailInput->setPrecision(2);
    m_detailInput->setRange(0.0, 100.0, 0.01, true);
    QWhatsThis::add( m_detailInput, i18n("<p>Relative smoothing factor used to set the sharpening level of the details in the target image." 
                         "Higher values leave details sharp."));
    grid2->addMultiCellWidget(m_detailLabel, 0, 0, 0, 0);
    grid2->addMultiCellWidget(m_detailInput, 0, 0, 1, 1);

    m_gradientLabel = new QLabel(i18n("Gradient Factor:"), secondPage);
    m_gradientLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_gradientInput = new KDoubleNumInput(secondPage);
    m_gradientInput->setPrecision(2);
    m_gradientInput->setRange(0.0, 100.0, 0.01, true);
    QWhatsThis::add( m_gradientInput, i18n("<p>Anisotropic (directional) modifier of the Detail Factor. Keep it small for Gaussian noise ."));
    grid2->addMultiCellWidget(m_gradientLabel, 1, 1, 0, 0);
    grid2->addMultiCellWidget(m_gradientInput, 1, 1, 1, 1);

    m_timeStepLabel = new QLabel(i18n("Time Step:"), secondPage);
    m_timeStepLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_timeStepInput = new KDoubleNumInput(secondPage);
    m_timeStepInput->setPrecision(2);
    m_timeStepInput->setRange(0.0, 500.0, 0.01, true);
    QWhatsThis::add( m_timeStepInput, i18n("<p>Total smoothing power: if Detail Factor sets the relative smoothing and Gradient Factor the "
                                     "direction, Time Step sets the overall effect."));
    grid2->addMultiCellWidget(m_timeStepLabel, 2, 2, 0, 0);
    grid2->addMultiCellWidget(m_timeStepInput, 2, 2, 1, 1);

    m_blurLabel = new QLabel(i18n("Blur Factor:"), secondPage);
    m_blurLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_blurInput = new KDoubleNumInput(secondPage);
    m_blurInput->setPrecision(2);
    m_blurInput->setRange(0.0, 10.0, 0.01, true);
    QWhatsThis::add( m_blurInput, i18n("<p>This value controls blurring level of the target image. Do not use an high value here else "
                                       "target image will be completly blurred."));
    grid2->addMultiCellWidget(m_blurLabel, 0, 0, 3, 3);
    grid2->addMultiCellWidget(m_blurInput, 0, 0, 4, 4);
    
    m_blurItLabel = new QLabel(i18n("Blurring Iterations:"), secondPage);
    m_blurItLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_blurItInput = new KDoubleNumInput(secondPage);
    m_blurInput->setPrecision(1);
    m_blurItInput->setRange(1.0, 16.0, 1.0, true);
    QWhatsThis::add( m_blurItInput, i18n("<p>Sets the number of times the filter is applied on the target image."));
    grid2->addMultiCellWidget(m_blurItLabel, 1, 1, 3, 3);
    grid2->addMultiCellWidget(m_blurItInput, 1, 1, 4, 4);
    
    // -------------------------------------------------------------
    
    QWidget* thirdPage = new QWidget( mainTab );
    QGridLayout* grid3 = new QGridLayout( thirdPage, 2, 3, marginHint(), spacingHint());
    mainTab->addTab( thirdPage, i18n("Advanced Settings") );
    
    m_angularStepLabel = new QLabel(i18n("Angular Step:"), thirdPage);
    m_angularStepLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_angularStepInput = new KDoubleNumInput(thirdPage);
    m_angularStepInput->setPrecision(2);
    m_angularStepInput->setRange(5.0, 90.0, 0.01, true);
    QWhatsThis::add( m_angularStepInput, i18n("<p>Set here the angular integration step in degrees in analogy to anisotropy."));
    grid3->addMultiCellWidget(m_angularStepLabel, 0, 0, 0, 0);
    grid3->addMultiCellWidget(m_angularStepInput, 0, 0, 1, 1);

    m_integralStepLabel = new QLabel(i18n("Integral Step:"), thirdPage);
    m_integralStepLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_integralStepInput = new KDoubleNumInput(thirdPage);
    m_integralStepInput->setPrecision(2);
    m_integralStepInput->setRange(0.1, 10.0, 0.01, true);
    QWhatsThis::add( m_integralStepInput, i18n("<p>Set here the spatial integral step. Stay below 1."));
    grid3->addMultiCellWidget(m_integralStepLabel, 1, 1, 0, 0);
    grid3->addMultiCellWidget(m_integralStepInput, 1, 1, 1, 1);

    m_gaussianLabel = new QLabel(i18n("Gaussian:"), thirdPage);
    m_gaussianLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_gaussianInput = new KDoubleNumInput(thirdPage);
    m_gaussianInput->setPrecision(2);
    m_gaussianInput->setRange(0.0, 500.0, 0.01, true);
    QWhatsThis::add( m_gaussianInput, i18n("<pSet here the precision of the gaussian function."));
    grid3->addMultiCellWidget(m_gaussianLabel, 2, 2, 0, 0);
    grid3->addMultiCellWidget(m_gaussianInput, 2, 2, 1, 1);
    
    m_linearInterpolationBox = new QCheckBox(i18n("Use Linear Interpolation"), thirdPage);
    QWhatsThis::add( m_linearInterpolationBox, i18n("<p>Enable this option to use a linear interpolation for integration."));
    grid3->addMultiCellWidget(m_linearInterpolationBox, 0, 0, 3, 3);
    
    m_normalizeBox = new QCheckBox(i18n("Normalize Photograph"), thirdPage);
    QWhatsThis::add( m_normalizeBox, i18n("<p>Enable this option to process an output image normalization."));
    grid3->addMultiCellWidget(m_normalizeBox, 1, 1, 3, 3);
    
    vlay->addWidget(mainTab);
    
    // -------------------------------------------------------------
    
    adjustSize();
    disableResize(); 
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));

    connect(m_restorationTypeCB, SIGNAL(activated(int)),
            this, SLOT(slotUser1()));
                        
    connect(m_detailInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 

    connect(m_gradientInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 

    connect(m_timeStepInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 

    connect(m_blurInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 

    connect(m_angularStepInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 

    connect(m_integralStepInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 
                                                
    connect(m_gaussianInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 

    connect(m_blurItInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));  

    connect(m_linearInterpolationBox, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));                        

    connect(m_normalizeBox, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));                        
}

ImageEffect_Restoration::~ImageEffect_Restoration()
{
    // No need to delete m_previewData because it's driving by QImage.

    if (m_cimgInterface)
       delete m_cimgInterface;
       
    if (m_originalData)
       delete [] m_originalData;
    
    if (m_timer)
       delete m_timer;
    
    delete m_iface;
}

void ImageEffect_Restoration::slotTimer()
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

void ImageEffect_Restoration::slotUser1()
{
    if (m_dirty)
       {
       m_cimgInterface->stopComputation();
       }
    else
       {
       m_detailInput->blockSignals(true);
       m_gradientInput->blockSignals(true);
       m_timeStepInput->blockSignals(true);
       m_blurInput->blockSignals(true);
       m_blurItInput->blockSignals(true);
       m_angularStepInput->blockSignals(true);
       m_integralStepInput->blockSignals(true);
       m_gaussianInput->blockSignals(true);
       m_linearInterpolationBox->blockSignals(true);
       m_normalizeBox->blockSignals(true);

       m_detailInput->setValue(0.1);
       m_gradientInput->setValue(0.9);
       m_timeStepInput->setValue(20.0);
       m_blurInput->setValue(1.4);
       m_blurItInput->setValue(1.0);
       m_angularStepInput->setValue(45.0);
       m_integralStepInput->setValue(0.8);
       m_gaussianInput->setValue(3.0);
       m_linearInterpolationBox->setChecked(true);
       m_normalizeBox->setChecked(false);
       
       switch(m_restorationTypeCB->currentItem())
          {
          case ReduceUniformNoise:
            {
            m_timeStepInput->setValue(40.0);
            break;
            }
          
          case ReduceJPEGArtefacts:
            {
            m_detailInput->setValue(0.3);
            m_blurInput->setValue(0.9);
            m_timeStepInput->setValue(100.0);
            break;
            }
          
          case ReduceTexturing:
            {
            m_timeStepInput->setValue(100.0);
            break;
            }
          }                       
                      
       m_detailInput->blockSignals(false);
       m_gradientInput->blockSignals(false);
       m_timeStepInput->blockSignals(false);
       m_blurInput->blockSignals(false);
       m_blurItInput->blockSignals(false);
       m_angularStepInput->blockSignals(false);
       m_integralStepInput->blockSignals(false);
       m_gaussianInput->blockSignals(false);
       m_linearInterpolationBox->blockSignals(false);
       m_normalizeBox->blockSignals(false);
        
       slotEffect();    
       }
} 

void ImageEffect_Restoration::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_cimgInterface->stopComputation();
       m_parent->setCursor( KCursor::arrowCursor() );
       }
       
    done(Cancel);
}

void ImageEffect_Restoration::slotHelp()
{
    KApplication::kApplication()->invokeHelp("restoration", "digikamimageplugins");
}

void ImageEffect_Restoration::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_cimgInterface->stopComputation();
       m_parent->setCursor( KCursor::arrowCursor() );
       }
       
    e->accept();
}

void ImageEffect_Restoration::slotEffect()
{
    if (m_dirty) return;     // Computation already in procress.
    
    m_currentRenderingMode = PreviewRendering;
    m_dirty                = true;
    
    m_imagePreviewWidget->setEnabled(false);
    m_restorationTypeCB->setEnabled(false);
    m_detailInput->setEnabled(false);
    m_gradientInput->setEnabled(false);
    m_timeStepInput->setEnabled(false);
    m_blurInput->setEnabled(false);
    m_blurItInput->setEnabled(false);
    m_angularStepInput->setEnabled(false);
    m_integralStepInput->setEnabled(false);
    m_gaussianInput->setEnabled(false);
    m_linearInterpolationBox->setEnabled(false);
    m_normalizeBox->setEnabled(false);
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
    enableButton(User2, false);
    enableButton(User3, false);
    
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    m_previewImage = m_imagePreviewWidget->getOriginalClipImage();
    uint *data     = (uint *)m_previewImage.bits();
    int w          = m_previewImage.width();
    int h          = m_previewImage.height();
    
    m_progressBar->setValue(0); 

    if (m_cimgInterface)
       delete m_cimgInterface;
        
    m_cimgInterface = new DigikamImagePlugins::CimgIface(data, w, h, 
                                    (uint)m_blurItInput->value(),
                                    m_timeStepInput->value(),
                                    m_integralStepInput->value(),
                                    m_angularStepInput->value(),
                                    m_blurInput->value(),
                                    m_detailInput->value(),
                                    m_gradientInput->value(),
                                    m_gaussianInput->value(),   
                                    m_normalizeBox->isChecked(),
                                    m_linearInterpolationBox->isChecked(),
                                    true, false, false, NULL, 0, this);
}

void ImageEffect_Restoration::slotOk()
{
    m_currentRenderingMode = FinalRendering;
    m_imagePreviewWidget->setEnabled(false);
    m_restorationTypeCB->setEnabled(false);
    m_detailInput->setEnabled(false);
    m_gradientInput->setEnabled(false);
    m_timeStepInput->setEnabled(false);
    m_blurInput->setEnabled(false);
    m_blurItInput->setEnabled(false);
    m_angularStepInput->setEnabled(false);
    m_integralStepInput->setEnabled(false);
    m_gaussianInput->setEnabled(false);
    m_linearInterpolationBox->setEnabled(false);
    m_normalizeBox->setEnabled(false);
    enableButton(Ok, false);
    enableButton(User1, false);
    enableButton(User2, false);
    enableButton(User3, false);
    m_parent->setCursor( KCursor::waitCursor() );
    m_progressBar->setValue(0);
    
    if (m_cimgInterface)
       delete m_cimgInterface;
       
    m_cimgInterface = new DigikamImagePlugins::CimgIface(m_originalData, m_originalWidth, m_originalHeight, 
                                    (uint)m_blurItInput->value(),
                                    m_timeStepInput->value(),
                                    m_integralStepInput->value(),
                                    m_angularStepInput->value(),
                                    m_blurInput->value(),
                                    m_detailInput->value(),
                                    m_gradientInput->value(),
                                    m_gaussianInput->value(),   
                                    m_normalizeBox->isChecked(),
                                    m_linearInterpolationBox->isChecked(),
                                    true, false, false, NULL, 0, this);
}

void ImageEffect_Restoration::customEvent(QCustomEvent *event)
{
    if (!event) return;

    DigikamImagePlugins::CimgIface::EventData *d = (DigikamImagePlugins::CimgIface::EventData*) event->data();

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
                 kdDebug() << "Preview Restoration completed..." << endl;
                 
                 m_imagePreviewWidget->setPreviewImageData(m_previewImage);
                 m_imagePreviewWidget->setPreviewImageWaitCursor(false);
                 m_progressBar->setValue(0);  
                
                 setButtonText(User1, i18n("&Reset Values"));
                 setButtonWhatsThis( User1, i18n("<p>Reset all parameters to the default values.") );
                 enableButton(Ok, true);    
                 m_imagePreviewWidget->setEnabled(true);                 
                 m_restorationTypeCB->setEnabled(true);
                 m_detailInput->setEnabled(true);
                 m_gradientInput->setEnabled(true);
                 m_timeStepInput->setEnabled(true);
                 m_blurInput->setEnabled(true);
                 m_blurItInput->setEnabled(true);
                 m_angularStepInput->setEnabled(true);
                 m_integralStepInput->setEnabled(true);
                 m_gaussianInput->setEnabled(true);
                 m_linearInterpolationBox->setEnabled(true);
                 m_normalizeBox->setEnabled(true);
                 m_dirty = false;   
                 break;
                 }
              
              case FinalRendering:
                 {
                 kdDebug() << "Final Restoration completed..." << endl;
                 Digikam::ImageIface iface(0, 0);
                 iface.putOriginalData(i18n("Restoration"), m_originalData);
       
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
                    kdDebug() << "Preview Restoration failed..." << endl;
                    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
                    m_progressBar->setValue(0);  
                    setButtonText(User1, i18n("&Reset Values"));
                    setButtonWhatsThis( User1, i18n("<p>Reset all parameters to the default values.") );
                    enableButton(Ok, true);    
                    m_imagePreviewWidget->setEnabled(true);                 
                    m_restorationTypeCB->setEnabled(true);
                    m_detailInput->setEnabled(true);
                    m_gradientInput->setEnabled(true);
                    m_timeStepInput->setEnabled(true);
                    m_blurInput->setEnabled(true);
                    m_blurItInput->setEnabled(true);
                    m_angularStepInput->setEnabled(true);
                    m_integralStepInput->setEnabled(true);
                    m_gaussianInput->setEnabled(true);
                    m_linearInterpolationBox->setEnabled(true);
                    m_normalizeBox->setEnabled(true);
                    m_dirty = false;   
                    break;
                    }
                
                case FinalRendering:
                    break;
                }
            }
        }
}

void ImageEffect_Restoration::slotUser2()
{
    KURL loadRestorationFile;
    FILE *fp = 0L;

    loadRestorationFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                                  QString( "*" ), this,
                                                  QString( i18n("Photograph Restoration Settings File to Load")) );
    if( loadRestorationFile.isEmpty() )
       return;

    fp = fopen(QFile::encodeName(loadRestorationFile.path()), "r");   
    
    if ( fp )
        {
        char buf1[1024];
        char buf2[1024];
        char buf3[1024];
        char buf4[1024];
        char buf5[1024];

        buf1[0] = '\0';

        fgets(buf1, 1023, fp);

        fscanf (fp, "%*s %s", buf1);
        
        // Normalize setting.
        fscanf (fp, "%*s %s", buf1);
          
        if (strcmp (buf1, "TRUE") == 0)
            m_normalizeBox->setChecked(true);
        else
            m_normalizeBox->setChecked(false);

        // Linear interpolation settings.     
        fscanf (fp, "%*s %s", buf1);
        
        if (strcmp (buf1, "TRUE") == 0)
            m_linearInterpolationBox->setChecked(true);
        else
            m_linearInterpolationBox->setChecked(false);

        // Smoothing settings.
        fscanf (fp, "%*s %s %s %s %s %s", buf1, buf2, buf3, buf4, buf5);
        m_detailInput->setValue( atof(buf1) );
        m_gradientInput->setValue( atof(buf2) );
        m_timeStepInput->setValue( atof(buf3) );
        m_blurInput->setValue( atof(buf4) );
        m_blurItInput->setValue( atof(buf5) );
        
        // Advanced settings.
        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        m_angularStepInput->setValue( atof(buf1) );
        m_integralStepInput->setValue( atof(buf2) );
        m_gaussianInput->setValue( atof(buf3) );

        fclose(fp);
        }
    else
        {
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Restoration text file."));
        return;
        }
}

void ImageEffect_Restoration::slotUser3()
{
    KURL saveRestorationFile;
    FILE *fp = 0L;

    saveRestorationFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                                  QString( "*" ), this,
                                                  QString( i18n("Photograph Restoration Settings File to Save")) );
    if( saveRestorationFile.isEmpty() )
       return;

    fp = fopen(QFile::encodeName(saveRestorationFile.path()), "w");   
    
    if ( fp )
        {       
        const char *str = 0L;
        char        buf1[256];
        char        buf2[256];
        char        buf3[256];
        char        buf4[256];
        char        buf5[256];

        fprintf (fp, "# Photograph Restoration Configuration File\n");

        fprintf (fp, "NORMALIZE: %s\n",
                 m_normalizeBox->isChecked() ? "TRUE" : "FALSE");
        fprintf (fp, "LINEAR_INTERPOLATION: %s\n",
                 m_linearInterpolationBox->isChecked() ? "TRUE" : "FALSE");

        sprintf (buf1, "%5.3f", m_detailInput->value());                 
        sprintf (buf2, "%5.3f", m_gradientInput->value());
        sprintf (buf3, "%5.3f", m_timeStepInput->value());
        sprintf (buf4, "%5.3f", m_blurInput->value());
        sprintf (buf5, "%5.3f", m_blurItInput->value());
        fprintf (fp, "SMOOTHING: %s %s %s %s %s\n", buf1, buf2, buf3, buf4, buf5);

        sprintf (buf1, "%5.3f", m_angularStepInput->value());
        sprintf (buf2, "%5.3f", m_integralStepInput->value());
        sprintf (buf3, "%5.3f", m_gaussianInput->value());
        fprintf (fp, "ADVANCED: %s %s %s\n", buf1, buf2, buf3);

        fclose (fp);
        }
    else
        {
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Restoration text file."));
        return;
        }
}

}  // NameSpace DigikamRestorationImagesPlugin

#include "imageeffect_restoration.moc"
