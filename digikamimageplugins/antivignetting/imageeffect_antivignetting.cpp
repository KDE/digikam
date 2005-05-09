/* ============================================================
 * File  : imageeffect_antivignetting.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-25
 * Description : a digiKam image plugin for to reduce 
 *               vignetting on an image.
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * Original AntiVignetting algorithm copyrighted 2003 by 
 * John Walker from 'pnmctrfilt' implementation. See 
 * http://www.fourmilab.ch/netpbm/pnmctrfilt for more 
 * informations.
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
#include <qpixmap.h>
#include <qpainter.h>
#include <qpen.h>
#include <qspinbox.h>
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

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imageeffect_antivignetting.h"


namespace DigikamAntiVignettingImagesPlugin
{

ImageEffect_AntiVignetting::ImageEffect_AntiVignetting(QWidget* parent)
                          : KDialogBase(Plain, i18n("Anti Vignetting"),
                                        Help|User1|Ok|Cancel, Ok,
                                        parent, 0, true, true, i18n("&Reset Values")),
                            m_parent(parent)
{
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    m_cancel = false;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Anti Vignetting"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to reduce image vignetting."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Gilles Caulier", 
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
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the preview of the anti vignetting effect.") );
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
                          
    QLabel *label1 = new QLabel(i18n("Density:"), firstPage);
    
    m_densitySlider = new QSlider(10, 200, 1, 20, Qt::Horizontal, firstPage, "m_densitySlider");
    m_densitySlider->setTickmarks(QSlider::Below);
    m_densitySlider->setTickInterval(40);
    m_densitySlider->setTracking( false );
    
    m_densitySpinBox = new QSpinBox(10, 200, 1, firstPage, "m_densitySpinBox");
    m_densitySpinBox->setValue(20);
        
    whatsThis = i18n("<p>This value controls the degree of intensity attenuation by the filter "
                     "at its point of maximum density.");
    QWhatsThis::add( m_densitySpinBox, whatsThis);
    QWhatsThis::add( m_densitySlider, whatsThis);
    
    grid->addMultiCellWidget(label1, 0, 0, 1, 1);
    grid->addMultiCellWidget(m_densitySlider, 0, 0, 2, 2);
    grid->addMultiCellWidget(m_densitySpinBox, 0, 0, 3, 3);
    
    QLabel *label2 = new QLabel(i18n("Power:"), firstPage);
    
    m_powerSlider = new QSlider(1, 100, 1, 10, Qt::Horizontal, firstPage, "m_powerSlider");
    m_powerSlider->setTickmarks(QSlider::Below);
    m_powerSlider->setTickInterval(20);
    m_powerSlider->setTracking ( false );  
    
    m_powerSpinBox = new QSpinBox(1, 100, 1, firstPage, "m_powerSpinBox");
    m_powerSpinBox->setValue(10);
        
    whatsThis = i18n("<p>This value is used as the exponent controlling the fall-off in density "
                     "from the center of the filter to the periphery.");
    QWhatsThis::add( m_powerSpinBox, whatsThis);
    QWhatsThis::add( m_powerSlider, whatsThis);                     
    
    grid->addMultiCellWidget(label2, 1, 1, 1, 1);
    grid->addMultiCellWidget(m_powerSlider, 1, 1, 2, 2);
    grid->addMultiCellWidget(m_powerSpinBox, 1, 1, 3, 3);
    
    QLabel *label3 = new QLabel(i18n("Radius:"), firstPage);
    
    m_radiusSlider = new QSlider(1, 100, 1, 10, Qt::Horizontal, firstPage, "m_radiusSlider");
    m_radiusSlider->setTickmarks(QSlider::Below);
    m_radiusSlider->setTickInterval(20);
    m_radiusSlider->setTracking ( false );  
    
    m_radiusSpinBox = new QSpinBox(1, 100, 1, firstPage, "m_radiusSpinBox");
    m_radiusSpinBox->setValue(10);
    
    whatsThis = i18n("<p>This value is the radius of the center filter. It is a multiple of the "
                     "half-diagonal measure of the image, at which the density of the filter falls to zero.");
    QWhatsThis::add( m_radiusSpinBox, whatsThis);
    QWhatsThis::add( m_radiusSlider, whatsThis);                     
    
    grid->addMultiCellWidget(label3, 2, 2, 1, 1);
    grid->addMultiCellWidget(m_radiusSlider, 2, 2, 2, 2);
    grid->addMultiCellWidget(m_radiusSpinBox, 2, 2, 3, 3);
    
    // -------------------------------------------------------------

    QWidget* secondPage = new QWidget( m_mainTab );
    QGridLayout* grid2 = new QGridLayout( secondPage, 3, 3, marginHint(), spacingHint());
    m_mainTab->addTab( secondPage, i18n("Exposure Re-Adjustment") );
    
    QLabel *label4 = new QLabel(i18n("Brightness:"), secondPage);
    
    m_brightnessSlider = new QSlider(0, 100, 1, 0, Qt::Horizontal, secondPage, "m_brightnessSlider");
    m_brightnessSlider->setTickmarks(QSlider::Below);
    m_brightnessSlider->setTickInterval(20);
    m_brightnessSlider->setTracking( false );
    
    m_brightnessSpinBox = new QSpinBox(0, 100, 1, secondPage, "m_brightnessSpinBox");
    m_brightnessSpinBox->setValue(0);
   
    whatsThis = i18n("<p>Set here the brightness re-adjustment of the target image.");
    QWhatsThis::add( m_brightnessSpinBox, whatsThis);
    QWhatsThis::add( m_brightnessSlider, whatsThis);
    
    grid2->addMultiCellWidget(label4, 0, 0, 0, 0);
    grid2->addMultiCellWidget(m_brightnessSlider, 0, 0, 1, 1);
    grid2->addMultiCellWidget(m_brightnessSpinBox, 0, 0, 2, 2);
    
    QLabel *label5 = new QLabel(i18n("Contrast:"), secondPage);
    
    m_contrastSlider = new QSlider(0, 100, 1, 0, Qt::Horizontal, secondPage, "m_contrastSlider");
    m_contrastSlider->setTickmarks(QSlider::Below);
    m_contrastSlider->setTickInterval(20);
    m_contrastSlider->setTracking ( false );  
    
    m_contrastSpinBox = new QSpinBox(0, 100, 1, secondPage, "m_contrastSpinBox");
    m_contrastSpinBox->setValue(0);
        
    whatsThis = i18n("<p>Set here the contrast re-adjustment of the target image.");
    QWhatsThis::add( m_contrastSpinBox, whatsThis);
    QWhatsThis::add( m_contrastSlider, whatsThis);                     
    
    grid2->addMultiCellWidget(label5, 1, 1, 0, 0);
    grid2->addMultiCellWidget(m_contrastSlider, 1, 1, 1, 1);
    grid2->addMultiCellWidget(m_contrastSpinBox, 1, 1, 2, 2);
    
    QLabel *label6 = new QLabel(i18n("Gamma:"), secondPage);
    
    m_gammaSlider = new QSlider(0, 100, 1, 0, Qt::Horizontal, secondPage, "m_gammaSlider");
    m_gammaSlider->setTickmarks(QSlider::Below);
    m_gammaSlider->setTickInterval(20);
    m_gammaSlider->setTracking ( false );  
    
    m_gammaSpinBox = new QSpinBox(0, 100, 1, secondPage, "m_gammaSpinBox");
    m_gammaSpinBox->setValue(0);
    
    whatsThis = i18n("<p>Set here the gamma re-adjustment of the target image.");
    QWhatsThis::add( m_gammaSpinBox, whatsThis);
    QWhatsThis::add( m_gammaSlider, whatsThis);                     
    
    grid2->addMultiCellWidget(label6, 2, 2, 0, 0);
    grid2->addMultiCellWidget(m_gammaSlider, 2, 2, 1, 1);
    grid2->addMultiCellWidget(m_gammaSpinBox, 2, 2, 2, 2);

    topLayout->addWidget(m_mainTab);
    
    // -------------------------------------------------------------
            
    m_progressBar = new KProgress(100, plainPage(), "m_progressbar");
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    topLayout->addWidget(m_progressBar);

    adjustSize();
    disableResize();  
    
    QTimer::singleShot(0, this, SLOT(slotUser1()));     // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect(m_densitySlider, SIGNAL(valueChanged(int)),
            m_densitySpinBox, SLOT(setValue(int)));
    connect(m_densitySpinBox, SIGNAL(valueChanged(int)),
            m_densitySlider, SLOT(setValue(int)));            
    connect(m_densitySpinBox, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));            
            
    connect(m_powerSlider, SIGNAL(valueChanged(int)),
            m_powerSpinBox, SLOT(setValue(int)));
    connect(m_powerSpinBox, SIGNAL(valueChanged(int)),
            m_powerSlider, SLOT(setValue(int)));   
    connect(m_powerSpinBox, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));     

    connect(m_radiusSlider, SIGNAL(valueChanged(int)),
            m_radiusSpinBox, SLOT(setValue(int)));
    connect(m_radiusSpinBox, SIGNAL(valueChanged(int)),
            m_radiusSlider, SLOT(setValue(int)));   
    connect(m_radiusSpinBox, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));     

    connect(m_brightnessSlider, SIGNAL(valueChanged(int)),
            m_brightnessSpinBox, SLOT(setValue(int)));
    connect(m_brightnessSpinBox, SIGNAL(valueChanged(int)),
            m_brightnessSlider, SLOT(setValue(int)));   
    connect(m_brightnessSpinBox, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));     

    connect(m_contrastSlider, SIGNAL(valueChanged(int)),
            m_contrastSpinBox, SLOT(setValue(int)));
    connect(m_contrastSpinBox, SIGNAL(valueChanged(int)),
            m_contrastSlider, SLOT(setValue(int)));   
    connect(m_contrastSpinBox, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));     

    connect(m_gammaSlider, SIGNAL(valueChanged(int)),
            m_gammaSpinBox, SLOT(setValue(int)));
    connect(m_gammaSpinBox, SIGNAL(valueChanged(int)),
            m_gammaSlider, SLOT(setValue(int)));   
    connect(m_gammaSpinBox, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));     
}

ImageEffect_AntiVignetting::~ImageEffect_AntiVignetting()
{
}

void ImageEffect_AntiVignetting::slotHelp()
{
    KApplication::kApplication()->invokeHelp("antivignettings",
                                             "digikamimageplugins");
}

void ImageEffect_AntiVignetting::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_AntiVignetting::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_AntiVignetting::slotUser1()
{
    m_densitySlider->blockSignals(true);
    m_densitySpinBox->blockSignals(true);
    m_powerSlider->blockSignals(true);
    m_powerSpinBox->blockSignals(true);
    m_radiusSlider->blockSignals(true);
    m_radiusSpinBox->blockSignals(true);
    m_brightnessSlider->blockSignals(true);
    m_brightnessSpinBox->blockSignals(true);
    m_contrastSlider->blockSignals(true);
    m_contrastSpinBox->blockSignals(true);
    m_gammaSlider->blockSignals(true);
    m_gammaSpinBox->blockSignals(true);
      
    m_densitySlider->setValue(20);
    m_densitySpinBox->setValue(20);
    m_powerSlider->setValue(10);
    m_powerSpinBox->setValue(10);
    m_radiusSlider->setValue(10);
    m_radiusSpinBox->setValue(10);
    m_brightnessSlider->setValue(0);
    m_brightnessSpinBox->setValue(0);
    m_contrastSlider->setValue(0);
    m_contrastSpinBox->setValue(0);
    m_gammaSlider->setValue(0);
    m_gammaSpinBox->setValue(0);
    
    m_densitySlider->blockSignals(false);
    m_densitySpinBox->blockSignals(false);
    m_powerSlider->blockSignals(false);
    m_powerSpinBox->blockSignals(false);
    m_radiusSlider->blockSignals(false);
    m_radiusSpinBox->blockSignals(false);
    m_brightnessSlider->blockSignals(false);
    m_brightnessSpinBox->blockSignals(false);
    m_contrastSlider->blockSignals(false);
    m_contrastSpinBox->blockSignals(false);
    m_gammaSlider->blockSignals(false);
    m_gammaSpinBox->blockSignals(false);

    slotEffect();
} 

void ImageEffect_AntiVignetting::slotEffect()
{
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    // All data from the image
    uint* data = iface->getPreviewData();
    int w      = iface->previewWidth();
    int h      = iface->previewHeight();
    double d   = (double)(m_densitySlider->value() /10.0);
    double p   = (double)(m_powerSlider->value() / 10.0);
    double r   = (double)(m_radiusSlider->value() /10.0);
    double b   = (double)(m_brightnessSlider->value() / 100.0);
    double c   = (double)(m_contrastSlider->value() / 100.0) + (double)(1.00);    
    double g   = (double)(m_gammaSlider->value() / 100.0) + (double)(1.00);

    m_progressBar->setValue(0); 

    // Calc mask preview.    
    QImage preview(90, 90, 32);
    memset(preview.bits(), 255, preview.numBytes());
    antiVignetting((uint*)preview.bits(), preview.width(), preview.height(), d, p, r, 0, 0, false);
    QPixmap pix (preview);
    QPainter pt(&pix);
    pt.setPen( QPen::QPen(Qt::black, 1) ); 
    pt.drawRect( 0, 0, pix.width(), pix.height() );
    pt.end();
    m_maskPreviewLabel->setPixmap( pix );
    
    // Apply mask to image.
    antiVignetting(data, w, h, d, p, r, 0, 0);
    
    // Normalize colors for a best rendering.   
    Digikam::ImageFilters::normalizeImage(data, w, h);

    iface->putPreviewData(data);
           
    delete [] data;
          
    // Adjust Image BCG.
    iface->setPreviewBCG(b, c, g);

    m_progressBar->setValue(0); 
    m_previewWidget->update();
}

void ImageEffect_AntiVignetting::slotOk()
{
    m_densitySlider->setEnabled(false);
    m_densitySpinBox->setEnabled(false);
    m_powerSlider->setEnabled(false);
    m_powerSpinBox->setEnabled(false);
    m_radiusSlider->setEnabled(false);
    m_radiusSpinBox->setEnabled(false);
    m_brightnessSlider->setEnabled(false);
    m_brightnessSpinBox->setEnabled(false);
    m_contrastSlider->setEnabled(false);
    m_contrastSpinBox->setEnabled(false);
    m_gammaSlider->setEnabled(false);
    m_gammaSpinBox->setEnabled(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data = iface->getOriginalData();
    int w      = iface->originalWidth();
    int h      = iface->originalHeight();
    double d   = (double)(m_densitySlider->value() / 10.0);
    double p   = (double)(m_powerSlider->value() / 10.0);
    double r   = (double)(m_radiusSlider->value() / 10.0);
    double b   = (double)(m_brightnessSlider->value() / 100.0);
    double c   = (double)(m_contrastSlider->value() / 100.0) + (double)(1.00);    
    double g   = (double)(m_gammaSlider->value() / 100.0) + (double)(1.00);

    m_progressBar->setValue(0); 
        
    if (data) 
       {
       // Apply mask to image.
       antiVignetting(data, w, h, d, p, r, 0, 0);
           
       // Normalize colors for a best rendering.   
       Digikam::ImageFilters::normalizeImage(data, w, h);

       if ( !m_cancel ) 
          {
          iface->putOriginalData(i18n("Anti Vignetting"), data);
          
          // Adjust Image BCG.
          iface->setOriginalBCG(b, c, g);
          }
       }
    
    delete [] data;    
    
    m_parent->setCursor( KCursor::arrowCursor() );        
    accept();
}

// This method is inspired from John Walker 'pnmctrfilt' algorithm code.

void ImageEffect_AntiVignetting::antiVignetting(uint *data, int Width, int Height,
                                                double density, double power, double radius,
                                                int xshift, int yshift,
                                                bool progress)
{
    int     col, row, xd, td, yd, p;
    int     i, xsize, ysize, diagonal, erad, xctr, yctr;
    int     BitCount = 0;
    double *ldens;

    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);

    BitCount = LineWidth * Height;
    
    uchar* Bits    = (uchar*)data;
    uchar* NewBits = new uchar[BitCount];

    // Determine the radius of the filter.  This is the half diagonal
    // measure of the image multiplied by the command line radius factor. 
    
    xsize = (Height + 1) / 2;
    ysize = (Width + 1) / 2;
    erad = (int)((sqrt((xsize * xsize) + (ysize * ysize)) + 0.5) * radius);    
        
    // Build the in-memory table which maps distance from the
    // centre of the image (as adjusted by the X and Y offset,
    // if any) to the density of the filter at this remove.  This
    // table needs to be as large as the diagonal from the
    // (possibly offset) centre to the most distant corner
    // of the image. 

    xsize    = ((Height + 1) / 2) + abs(xshift);
    ysize    = ((Width + 1) / 2) + abs(yshift);
    diagonal = ((int) (sqrt((xsize * xsize) + (ysize * ysize)) + 0.5)) +  1;
    
    ldens = new double[diagonal];
    
    for (i = 0 ; !m_cancel && (i < diagonal) ; ++i)
        {
        if ( i >= erad )
           ldens[i] = 1;
        else 
           ldens[i] =  (1.0 + (density - 1) * pow(1.0 - (((double) i) / (erad - 1)), power));
        }

    xctr = ((Height + 1) / 2) + xshift;
    yctr = ((Width + 1) / 2) + yshift;
    
    for (row = 0 ; !m_cancel && (row < Width) ; ++row) 
        {
        yd = abs(yctr - row);

        for (col = 0 ; !m_cancel && (col < Height) ; ++col) 
            {
            p = col * LineWidth + 4 * row;         

            xd = abs(xctr - col);
            td = (int) (sqrt((xd * xd) + (yd * yd)) + 0.5);

            NewBits[p+2]  = (uint)((Bits[p+2]) / ldens[td]);
            NewBits[p+1]  = (uint)((Bits[p+1]) / ldens[td]);
            NewBits[ p ]  = (uint)((Bits[ p ]) / ldens[td]);
            }
        
        // Update progress bar in dialog.
        
        if (progress)
            {
            m_progressBar->setValue((int) (((double)row * 100.0) / Width));
            kapp->processEvents(); 
            }
        }

    if (!m_cancel) 
       memcpy (data, NewBits, BitCount);        
        
    delete [] ldens;        
    delete [] NewBits;
}

}  // NameSpace DigikamAntiVignettingImagesPlugin

#include "imageeffect_antivignetting.moc"
