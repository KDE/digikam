/* ============================================================
 * File  : imageeffect_restoration.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-26
 * Description : a digiKam image editor plugin to restore 
 *               a photograph
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Some code come from the CImg Gimp plugin by Victor Stinner.
 * See: http://www.girouette-stinner.com/castor/gimp.html
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
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qtimer.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
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
#include "imageeffect_restoration.h"

using namespace cimg_library;

namespace DigikamRestorationImagesPlugin
{

ImageEffect_Restoration::ImageEffect_Restoration(QWidget* parent)
                   : KDialogBase(Plain, i18n("Restoration"),
                                 Help|User1|Ok|Cancel, Ok,
                                 parent, 0, true, true,
                                 i18n("&Reset Values")),
                     m_parent(parent)
{
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    m_cancel = false;
    m_dirty  = false;    
    m_timer  = 0;
    
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
                     "http://www.greyc.ensicaen.fr/~dtschump/greycstoration/index.html");
                        
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Restoration Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    mainTab->addTab( firstPage, i18n("Main") );

    QLabel *typeLabel = new QLabel(i18n("Restoration Type:"), firstPage);
    typeLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_restorationTypeCB = new QComboBox( false, firstPage ); 
    m_restorationTypeCB->insertItem( i18n("Filtering") );
    m_restorationTypeCB->insertItem( i18n("Inpainting") );
    QWhatsThis::add( m_restorationTypeCB, i18n("<p>Select here the photograph restoration type."));

    // Disable ComboBox until Inpainting method will be completed.
    m_restorationTypeCB->setEnabled(false);
        
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
    QWhatsThis::add( m_detailInput, i18n("<p>."));
    grid2->addMultiCellWidget(m_detailLabel, 0, 0, 0, 0);
    grid2->addMultiCellWidget(m_detailInput, 0, 0, 1, 1);

    m_gradientLabel = new QLabel(i18n("Gradient Factor:"), secondPage);
    m_gradientLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_gradientInput = new KDoubleNumInput(secondPage);
    m_gradientInput->setPrecision(2);
    m_gradientInput->setRange(0.0, 100.0, 0.01, true);
    QWhatsThis::add( m_gradientInput, i18n("<p>."));
    grid2->addMultiCellWidget(m_gradientLabel, 1, 1, 0, 0);
    grid2->addMultiCellWidget(m_gradientInput, 1, 1, 1, 1);

    m_timeStepLabel = new QLabel(i18n("Time Step:"), secondPage);
    m_timeStepLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_timeStepInput = new KDoubleNumInput(secondPage);
    m_timeStepInput->setPrecision(2);
    m_timeStepInput->setRange(0.0, 500.0, 0.01, true);
    QWhatsThis::add( m_timeStepInput, i18n("<p>."));
    grid2->addMultiCellWidget(m_timeStepLabel, 2, 2, 0, 0);
    grid2->addMultiCellWidget(m_timeStepInput, 2, 2, 1, 1);

    m_blurLabel = new QLabel(i18n("Blur Factor:"), secondPage);
    m_blurLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_blurInput = new KDoubleNumInput(secondPage);
    m_blurInput->setPrecision(2);
    m_blurInput->setRange(0.0, 10.0, 0.01, true);
    QWhatsThis::add( m_blurInput, i18n("<p>."));
    grid2->addMultiCellWidget(m_blurLabel, 0, 0, 3, 3);
    grid2->addMultiCellWidget(m_blurInput, 0, 0, 4, 4);
    
    m_blurItLabel = new QLabel(i18n("Blurring Iterations:"), secondPage);
    m_blurItLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_blurItInput = new KDoubleNumInput(secondPage);
    m_blurInput->setPrecision(1);
    m_blurItInput->setRange(1.0, 16.0, 1.0, true);
    QWhatsThis::add( m_blurItInput, i18n("<p>."));
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
    QWhatsThis::add( m_angularStepInput, i18n("<p>."));
    grid3->addMultiCellWidget(m_angularStepLabel, 0, 0, 0, 0);
    grid3->addMultiCellWidget(m_angularStepInput, 0, 0, 1, 1);

    m_integralStepLabel = new QLabel(i18n("Integral Step:"), thirdPage);
    m_integralStepLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_integralStepInput = new KDoubleNumInput(thirdPage);
    m_integralStepInput->setPrecision(2);
    m_integralStepInput->setRange(0.1, 10.0, 0.01, true);
    QWhatsThis::add( m_integralStepInput, i18n("<p>."));
    grid3->addMultiCellWidget(m_integralStepLabel, 1, 1, 0, 0);
    grid3->addMultiCellWidget(m_integralStepInput, 1, 1, 1, 1);

    m_gaussianLabel = new QLabel(i18n("Gaussian:"), thirdPage);
    m_gaussianLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_gaussianInput = new KDoubleNumInput(thirdPage);
    m_gaussianInput->setPrecision(2);
    m_gaussianInput->setRange(0.0, 500.0, 0.01, true);
    QWhatsThis::add( m_gaussianInput, i18n("<p>."));
    grid3->addMultiCellWidget(m_gaussianLabel, 2, 2, 0, 0);
    grid3->addMultiCellWidget(m_gaussianInput, 2, 2, 1, 1);
    
    m_linearInterpolationBox = new QCheckBox(i18n("Use Linear Interpolation"), thirdPage);
    QWhatsThis::add( m_linearInterpolationBox, i18n("<p>."));
    grid3->addMultiCellWidget(m_linearInterpolationBox, 0, 0, 3, 3);
    
    m_normalizeBox = new QCheckBox(i18n("Normalize Photograph"), thirdPage);
    QWhatsThis::add( m_normalizeBox, i18n("<p>."));
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
    if (m_timer)
       delete m_timer;
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

void ImageEffect_Restoration::slotRestorationTypeChanged(int type)
{
}

void ImageEffect_Restoration::slotUser1()
{
    if (m_dirty)
       {
       m_cancel = true;
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

       switch(m_restorationTypeCB->currentItem())
          {
          case FilteringMode:
            {
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
            restore  = true;
            inpaint  = false;
            break;
            }

          case InPaintingMode:
            {
            m_detailInput->setValue(0.1);
            m_gradientInput->setValue(100.0);
            m_timeStepInput->setValue(50.0);
            m_blurInput->setValue(2.0);
            m_blurItInput->setValue(100.0);
            m_angularStepInput->setValue(45.0);
            m_integralStepInput->setValue(0.8);
            m_gaussianInput->setValue(3.0);
            m_linearInterpolationBox->setChecked(true);
            m_normalizeBox->setChecked(false);
            restore  = false;
            inpaint  = true;
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
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_Restoration::slotHelp()
{
    KApplication::kApplication()->invokeHelp("restoration", "digikamimageplugins");
}

void ImageEffect_Restoration::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_Restoration::slotEffect()
{
    if (m_dirty) return;     // Computation already in procress.
    m_dirty = true;
    m_cancel = false;
    
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
    
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    uint* data   = (uint *)image.bits();
    int   w      = image.width();
    int   h      = image.height();
    
    m_progressBar->setValue(0); 
    processRestoration(data, w, h);
    
    if (!m_cancel)     
       m_imagePreviewWidget->setPreviewImageData(image);
       
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_progressBar->setValue(0);  
    
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all parameters to the default values.") );
    enableButton(Ok, true);    
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
}

void ImageEffect_Restoration::slotOk()
{
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
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
        
    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    
    m_progressBar->setValue(0);
    processRestoration(data, w, h);

    if ( !m_cancel )
       iface.putOriginalData(i18n("Restoration"), data);
       
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

void ImageEffect_Restoration::processRestoration(uint* data, int width, int height)
{
    // CImg filter mode (unused).
    resize   = false;
    visuflow = NULL;
    
    // Copy the src data into a CImg type image with three channels and no alpha. This means that a CImg is always rgba.

    img = CImg<>(width, height, 1, 3);
    eigen = CImgl<>(CImg<>(2,1), CImg<>(2,2));    

    register int x, y, i=0;
    
    uchar *d = (uchar *)data;

    for (y = 0; y < height; y++) 
       {
       for (x = 0; x < width; x++, i+=4) 
          {
          img(x, y, 0) = d[ i ];
          img(x, y, 1) = d[i+1];
          img(x, y, 2) = d[i+2];
          }
       }
       
    // Get the config data

    nb_iter    = (uint)m_blurItInput->value();
    dt         = m_timeStepInput->value();
    dlength    = m_integralStepInput->value();
    dtheta     = m_angularStepInput->value();
    sigma      = m_blurInput->value();
    power1     = m_detailInput->value();
    power2     = m_gradientInput->value();
    gauss_prec = m_gaussianInput->value();
    
    onormalize = m_normalizeBox->isChecked();
    linear     = m_linearInterpolationBox->isChecked();

    
    if (!process()) 
       {
       // Everything went wrong.
       kdDebug() << "Error during CImg filter computation!" << endl;
       return;
       }

    // Copy CImg onto destination.

    d = (uchar*)data;
    i = 0;

    for (y = 0; y < height; y++) 
       {
       for (x = 0; x < width; x++, i+=4) 
          {
          d[ i ] = (uchar)img(x, y, 0);
          d[i+1] = (uchar)img(x, y, 1);
          d[i+2] = (uchar)img(x, y, 2);
          }
       }
    
    // clean up memory.
    img = CImg<>();    
    eigen = CImgl<>(CImg<>(), CImg<>());
}

bool ImageEffect_Restoration::process()
{
    if (!prepare()) return false;

    // Begin regularization PDE iterations
        
     int counter = 0;
     
     for (unsigned int iter = 0; !m_cancel && (iter < nb_iter); iter++)
        {
        // Compute smoothed structure tensor field G
        compute_smoothed_tensor();

        // Compute normalized tensor field sqrt(T) in G
        compute_normalized_tensor();

        // Compute LIC's along different angle projections a_\alpha
        compute_LIC(counter);

        // Average all the LIC's
        compute_average_LIC();

        // Next step
        img = dest;
        }
    
    // Save result and end program
        
    if (visuflow) dest.mul(flow.get_norm_pointwise()).normalize(0, 255);
    if (onormalize) dest.normalize(0, 255);
        
    cleanup();
    
    if (m_cancel) 
      {
      kdDebug() << "CImg filter arborted!" << endl;
      return false;    
      }

    return true;
}

void ImageEffect_Restoration::get_geom(const char *geom, int &geom_w, int &geom_h) 
{
    char tmp[16];
    std::sscanf(geom,"%d%7[^0-9]%d%7[^0-9]",&geom_w,tmp,&geom_h,tmp+1);
    if (tmp[0]=='%') geom_w=-geom_w;
    if (tmp[1]=='%') geom_h=-geom_h;
}

void ImageEffect_Restoration::cleanup()
{
    img0 = flow = G = dest = sum= W = CImg<>();    
    mask = CImg<uchar> ();
}

bool ImageEffect_Restoration::prepare()
{
    if (!restore && !inpaint && !resize && !visuflow) 
       {
       kdDebug() << "Unspecified CImg filter computation Mode!" << endl;
       return false;
       }

    // Init algorithm parameters
    
    if (restore)  if (!prepare_restore())  return false;
    if (inpaint)  if (!prepare_inpaint())  return false;
    if (resize)   if (!prepare_resize())   return false;
    if (visuflow) if (!prepare_visuflow()) return false;

    if (!check_args()) return false;

    // Init images
    
    dest = CImg<>(img.width,img.height,1,img.dim);
    sum  = CImg<>(img.width,img.height,1);
    W    = CImg<>(img.width,img.height,1,2);
    
    return true;
}

bool ImageEffect_Restoration::check_args()
{
    if (power2 < power1)
       {
       kdDebug() << "Error : p2<p1 !" << endl;
       return false;
       }
       
    return true;
}

bool ImageEffect_Restoration::prepare_restore()
{
    CImgStats stats(img, false);
    img.normalize((float)stats.min, (float)stats.max);
    img0 = img;
    G = CImg<>(img.width,img.height,1,3);
    return true;
}

bool ImageEffect_Restoration::prepare_inpaint()
{
    const char *file_m = NULL; //cimg_option("-m",(const char*)NULL,"Input inpainting mask");
    
    if (!file_m) 
       {
       kdDebug() << "Unspecified CImg inpainting mask !" << endl;
       return false;
       }

    const unsigned int dilate  = 0; //cimg_option("-dilate",0,"Inpainting mask dilatation");
    const unsigned int ip_init = 3; //cimg_option("-init",3,"Inpainting init (0=black, 1=white, 2=noise, 3=unchanged, 4=interpol)");
    
    if (cimg::strncasecmp("block",file_m,5)) 
        mask = CImg<uchar>(file_m);
    else 
       {
       int l = 16;
       std::sscanf(file_m,"block%d",&l);
       mask = CImg<uchar>(img.width/l,img.height/l);
       cimg_mapXY(mask,x,y) mask(x,y)=(x+y)%2;
       }
       
    mask.resize(img.width,img.height,1,1);
    
    if (dilate) mask.dilate(dilate);
    
    switch (ip_init) 
    {
    case 0 : { cimg_mapXYV(img,x,y,k) if (mask(x,y)) img(x,y,k) = 0; } break;
    case 1 : { cimg_mapXYV(img,x,y,k) if (mask(x,y)) img(x,y,k) = 255; } break;
    case 2 : { cimg_mapXYV(img,x,y,k) if (mask(x,y)) img(x,y,k) = (float)(255*cimg::rand()); } break;
    case 3 : break;
    case 4 : {
        CImg<uchar> tmask(mask),ntmask(tmask);
        CImg_3x3(M,uchar);
        CImg_3x3(I,float);
        while (CImgStats(ntmask,false).max>0) {
            cimg_map3x3(tmask,x,y,0,0,M) if (Mcc && (!Mpc || !Mnc || !Mcp || !Mcn)) {
                const float ccp = Mcp?0.0f:1.0f, cpc = Mpc?0.0f:1.0f,
                    cnc = Mnc?0.0f:1.0f, ccn = Mcn?0.0f:1.0f, csum = ccp + cpc + cnc + ccn;
                cimg_mapV(img,k) {
                    cimg_get3x3(img,x,y,0,k,I);
                    img(x,y,k) = (ccp*Icp + cpc*Ipc + cnc*Inc + ccn*Icn)/csum;
                }
                ntmask(x,y) = 0;
            }
            tmask = ntmask;
        }
    } break;    
    
    default: break;
    }
    
    img0=img;
    G = CImg<>(img.width,img.height,1,3,0);
    CImg_3x3(g,uchar);
    CImg_3x3(I,float);
    cimg_map3x3(mask,x,y,0,0,g) if (!gcc && !(gnc-gcc) && !(gcc-gpc) && !(gcn-gcc) && !(gcc-gcp)) cimg_mapV(img,k) 
        {
        cimg_get3x3(img,x,y,0,k,I);
        const float ix = 0.5f*(Inc-Ipc), iy = 0.5f*(Icn-Icp);
        G(x,y,0)+= ix*ix; G(x,y,1)+= ix*iy; G(x,y,2)+= iy*iy;    
        }
    G.blur(sigma);
    { cimg_mapXY(G,x,y) 
        {
            G.get_tensor(x,y).symeigen(eigen(0),eigen(1));
            const float
                l1 = eigen(0)[0],
                l2 = eigen(0)[1],
                u  = eigen(1)[0],
                v  = eigen(1)[1],      
                ng = (float)std::sqrt(l1+l2),
                n1 = (float)(1.0/std::pow(1+ng,power1)),
                n2 = (float)(1.0/std::pow(1+ng,power2)),
               sr1 = (float)std::sqrt(n1),
               sr2 = (float)std::sqrt(n2);
          G(x,y,0) = sr1*u*u + sr2*v*v;
          G(x,y,1) = u*v*(sr1-sr2);
          G(x,y,2) = sr1*v*v + sr2*u*u;
        }    
    }
    return true;
}

bool ImageEffect_Restoration::prepare_resize()
{
    const char *geom  = NULL; //cimg_option("-g",(const char*)NULL,"Output image geometry");
    const bool anchor = true; //cimg_option("-anchor",true,"Anchor original pixels");
    
    if (!geom) throw CImgArgumentException("You need to specify an output geomety (option -g)");
    
    int w,h; get_geom(geom,w,h);
    mask = CImg<uchar>(img.width,img.height,1,1,255);
    
    if (!anchor) mask.resize(w,h,1,1,1); else mask = ~mask.resize(w,h,1,1,4);
    
    img0 = img.get_resize(w,h,1,-100,1);
    img.resize(w,h,1,-100,3);
    G = CImg<>(img.width,img.height,1,3);
    
    return true;
}

bool ImageEffect_Restoration::prepare_visuflow()
{
    const char *geom     = "100%x100%"; //cimg_option("-g","100%x100%","Output geometry");
    //const char *file_i   = (const char *)NULL; //cimg_option("-i",(const char*)NULL,"Input init image");
    const bool normalize = false; //cimg_option("-norm",false,"Normalize input flow");

    int w,h; get_geom(geom,w,h);
    
    if (!cimg::strcasecmp(visuflow,"circle")) { // Create a circular vector flow
        flow = CImg<>(400,400,1,2);
        cimg_mapXY(flow,x,y) {
            const float ang = (float)(std::atan2(y-0.5*flow.dimy(),x-0.5*flow.dimx()));
            flow(x,y,0) = -(float)std::sin(ang);
            flow(x,y,1) = (float)std::cos(ang);
        }
    }
    
    if (!cimg::strcasecmp(visuflow,"radial")) { // Create a radial vector flow
        flow = CImg<>(400,400,1,2);
        cimg_mapXY(flow,x,y) {
            const float ang = (float)(std::atan2(y-0.5*flow.dimy(),x-0.5*flow.dimx()));
            flow(x,y,0) = (float)std::cos(ang);
            flow(x,y,1) = (float)std::sin(ang);
        }
    }
    
    if (!flow.data) flow = CImg<>(visuflow);
    
    flow.resize(w,h,1,2,3);
    
    if (normalize) flow.orientation_pointwise();
    /*    if (file_i) img = CImg<>(file_i);
          else img = CImg<>(flow.width,flow.height,1,1,0).noise(100,2); */
    img0=img;
    img0.fill(0);
    float color[3]={255,255,255};
    img0.draw_quiver(flow,color,15,-10);
    G = CImg<>(img.width,img.height,1,3);
    
    return true;
}

void ImageEffect_Restoration::compute_smoothed_tensor()
{
    if (visuflow || inpaint) return;
    
    CImg_3x3(I,float);
    G.fill(0);
    cimg_mapV(img,k) cimg_map3x3(img,x,y,0,k,I) 
        {
        const float ix = 0.5f*(Inc-Ipc), iy = 0.5f*(Icn-Icp);
        G(x,y,0)+= ix*ix; G(x,y,1)+= ix*iy; G(x,y,2)+= iy*iy;    
        }
    G.blur(sigma);
}

void ImageEffect_Restoration::compute_normalized_tensor()
{
    if (restore || resize) cimg_mapXY(G,x,y) 
        {
        G.get_tensor(x,y).symeigen(eigen(0),eigen(1));
        const float
            l1 = eigen(0)[0],
            l2 = eigen(0)[1],
            u = eigen(1)[0],
            v = eigen(1)[1],      
            n1 = (float)(1.0/std::pow(1.0f+l1+l2,0.5f*power1)),
            n2 = (float)(1.0/std::pow(1.0f+l1+l2,0.5f*power2));
        G(x,y,0) = n1*u*u + n2*v*v;
        G(x,y,1) = u*v*(n1-n2);
        G(x,y,2) = n1*v*v + n2*u*u;
        }    
    
    if (visuflow) cimg_mapXY(G,x,y) 
        {
        const float 
            u = flow(x,y,0),
            v = flow(x,y,1),
            n = (float)std::pow(u*u+v*v,0.25f),
            nn = n<1e-5?1:nn;
        G(x,y,0) = u*u/nn;
        G(x,y,1) = u*v/nn;
        G(x,y,2) = v*v/nn;
        }

    const CImgStats stats(G,false);
    G /= cimg::max(std::fabs(stats.max), std::fabs(stats.min));
}

void ImageEffect_Restoration::compute_W(float cost, float sint)
{
    cimg_mapXY(W,x,y) 
        {
        const float 
            a = G(x,y,0),
            b = G(x,y,1),
            c = G(x,y,2),
            u = a*cost + b*sint,
            v = b*cost + c*sint;
        W(x,y,0) = u;
        W(x,y,1) = v;
        }
}

void ImageEffect_Restoration::compute_LIC_back_forward(int x, int y)
{
    float l, X,Y, cu, cv, lsum=0;
    const float fsigma2 = 2*dt*(W(x,y,0)*W(x,y,0) + W(x,y,1)*W(x,y,1));
    const float length = gauss_prec*(float)std::sqrt(fsigma2);

    if (linear) 
        {
        // Integrate with linear interpolation
        cu = W(x,y,0); cv = W(x,y,1); X=(float)x; Y=(float)y;
        
        for (l=0; !m_cancel && l<length && X>=0 && Y>=0 && X<=W.dimx()-1 && Y<=W.dimy()-1; l+=dlength) 
            {
            float u = (float)(W.linear_pix2d(X,Y,0)), v = (float)(W.linear_pix2d(X,Y,1));
            const float coef = (float)std::exp(-l*l/fsigma2);
            if ((cu*u+cv*v)<0) { u=-u; v=-v; }
            cimg_mapV(dest,k) dest(x,y,k)+=(float)(coef*img.linear_pix2d(X,Y,k));
            X+=dlength*u; Y+=dlength*v; cu=u; cv=v; lsum+=coef;
            }
            
        cu = W(x,y,0); cv = W(x,y,1); X=x-dlength*cu; Y=y-dlength*cv;
        
        for (l=dlength; !m_cancel && l<length && X>=0 && Y>=0 && X<=W.dimx()-1 && Y<=W.dimy()-1; l+=dlength) 
            {
            float u = (float)(W.linear_pix2d(X,Y,0)), v = (float)(W.linear_pix2d(X,Y,1));
            const float coef = (float)std::exp(-l*l/fsigma2);
            if ((cu*u+cv*v)<0) { u=-u; v=-v; }
            cimg_mapV(dest,k) dest(x,y,k)+=(float)(coef*img.linear_pix2d(X,Y,k));
            X-=dlength*u; Y-=dlength*v; cu=u; cv=v; lsum+=coef;
            }
        } 
    else 
        {
        // Integrate with non linear interpolation
        cu = W(x,y,0); cv = W(x,y,1); X=(float)x; Y=(float)y; 
        
        for (l=0; !m_cancel && l<length && X>=0 && Y>=0 && X<=W.dimx()-1 && Y<=W.dimy()-1; l+=dlength) 
            {
            float u = W((int)X,(int)Y,0), v = W((int)X,(int)Y,1);
            const float coef = (float)std::exp(-l*l/fsigma2);
            if ((cu*u+cv*v)<0) { u=-u; v=-v; }
            cimg_mapV(dest,k) dest(x,y,k)+=(float)(coef*img.linear_pix2d(X,Y,k));
            X+=dlength*u; Y+=dlength*v; cu=u; cv=v; lsum+=coef;
            }
                
        cu = W(x,y,0); cv = W(x,y,1); X=x-dlength*cu; Y=y-dlength*cv;
        
        for (l = dlength; !m_cancel && l<length && X>=0 && Y>=0 && X<=W.dimx()-1 && Y<=W.dimy()-1; l+=dlength) 
            {
            float u = W((int)X,(int)Y,0), v = W((int)X,(int)Y,1);
            const float coef = (float)std::exp(-l*l/fsigma2);
            if ((cu*u+cv*v)<0) { u=-u; v=-v; }
            cimg_mapV(dest,k) dest(x,y,k)+=(float)(coef*img.linear_pix2d(X,Y,k));
            X-=dlength*u; Y-=dlength*v; cu=u; cv=v; lsum+=coef;
            }
    }
    sum(x,y)+=lsum;
}

void ImageEffect_Restoration::compute_LIC(int &counter)
{
    dest.fill(0);
    sum.fill(0);
    
    for (float theta = (180%(int)dtheta)/2.0f; !m_cancel && (theta < 180); theta += dtheta) 
        {
        const float rad = (float)(theta*cimg::PI/180.0);
        const float cost = (float)std::cos(rad);
        const float sint = (float)std::sin(rad);

        // Compute vector field w = sqrt(T)*a_alpha
        compute_W(cost, sint);

        // Compute the LIC along w in backward and forward directions
        cimg_mapXY(dest,x,y) 
           {
           counter++;

           // Update the progress bar in dialog.
           double progress = counter;
           progress /= (double)dest.width * dest.height * nb_iter * (180 / dtheta);
           m_progressBar->setValue( (int)(100*progress) );
           kapp->processEvents(); 
        
           if (!mask.data || mask(x,y)) compute_LIC_back_forward(x,y);
           }
        }
}

void ImageEffect_Restoration::compute_average_LIC()
{
    cimg_mapXY(dest,x,y) 
    {
        if (sum(x,y)>0) 
            cimg_mapV(dest,k) dest(x,y,k) /= sum(x,y); 
        else 
            cimg_mapV(dest,k) dest(x,y,k) = img(x,y,k);
    }
}

}  // NameSpace DigikamRestorationImagesPlugin

#include "imageeffect_restoration.moc"
