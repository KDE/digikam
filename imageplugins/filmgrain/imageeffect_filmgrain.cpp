/* ============================================================
 * File  : imageeffect_filmgrain.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a Digikam image editor plugin for to add film 
 *               grain on an image.
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

// Imlib2 include.

#define X_DISPLAY_MISSING 1
#include <Imlib2.h>

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
#include <qdatetime.h> 

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

// Digikam includes.

#include <digikam/imageiface.h>
#include <digikam/imagepreviewwidget.h>

// Local includes.

#include "version.h"
#include "imageeffect_filmgrain.h"

namespace DigikamFilmGrainImagesPlugin
{

ImageEffect_FilmGrain::ImageEffect_FilmGrain(QWidget* parent)
                     : KDialogBase(Plain, i18n("Film Grain"),
                                Help|User1|Ok|Cancel, Ok,
                                parent, 0, true, true, i18n("&Reset values")),
                       m_parent(parent)
{
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    m_cancel = false;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Film Grain"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A Digikam image plugin for add film grain to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins.php");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Film Grain handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Add Film Grain To Image"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addWidget(headerFrame);
    
    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left", "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePreviewWidget(240, 160, 
                                                           i18n("Film Grain Preview"),
                                                           plainPage());
    hlay1->addWidget(m_imagePreviewWidget);
    
    // -------------------------------------------------------------
    
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label1 = new QLabel(i18n("Film sensibility (ASA):"), plainPage());
    
    m_sensibilitySlider = new QSlider(0, 14, 1, 2, Qt::Horizontal, plainPage(), "m_sensibilitySlider");
    m_sensibilitySlider->setTracking ( false );
    m_sensibilitySlider->setTickInterval(1);
    m_sensibilitySlider->setTickmarks(QSlider::Below);
    
    m_sensibilityLCDValue = new QLCDNumber (4, plainPage(), "m_sensibilityLCDValue");
    m_sensibilityLCDValue->setSegmentStyle ( QLCDNumber::Flat );
    m_sensibilityLCDValue->display( QString::number(800) );
    whatsThis = i18n("<p>Set here the film sensibility in ASA to use for simulate the film graininess.");
        
    QWhatsThis::add( m_sensibilityLCDValue, whatsThis);
    QWhatsThis::add( m_sensibilitySlider, whatsThis);

    hlay->addWidget(label1, 1);
    hlay->addWidget(m_sensibilitySlider, 3);
    hlay->addWidget(m_sensibilityLCDValue, 1);
    
    // -------------------------------------------------------------
        
    QHBoxLayout *hlay6 = new QHBoxLayout(topLayout);
    m_progressBar = new KProgress(100, plainPage(), "progressbar");
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    hlay6->addWidget(m_progressBar, 1);

    // -------------------------------------------------------------
    
    adjustSize();
    slotUser1();    // Reset all parameters to the default values.
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect( m_sensibilitySlider, SIGNAL(valueChanged(int)),
             this, SLOT(slotSensibilityChanged(int)) ); 
                       
}

ImageEffect_FilmGrain::~ImageEffect_FilmGrain()
{
}

void ImageEffect_FilmGrain::slotUser1()
{
    blockSignals(true);
    m_sensibilitySlider->setValue(2);
    blockSignals(false);
} 

void ImageEffect_FilmGrain::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_FilmGrain::slotHelp()
{
    KApplication::kApplication()->invokeHelp("filmgrain",
                                             "digikamimageplugins");
}

void ImageEffect_FilmGrain::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_FilmGrain::slotSensibilityChanged(int v)
{
    m_sensibilityLCDValue->display( QString::number(400+200*v) );
    slotEffect();
}

void ImageEffect_FilmGrain::slotEffect()
{
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    uint* data   = (uint *)image.bits();
    int   w      = image.width();
    int   h      = image.height();
    int   s      = 400 + 200*m_sensibilitySlider->value();
            
    m_progressBar->setValue(0); 
    FilmGrain(data, w, h, s);
    
    if (m_cancel) return;
    
    m_progressBar->setValue(0);  
    memcpy(image.bits(), (uchar *)data, image.numBytes());
    m_imagePreviewWidget->setPreviewImageData(image);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
}

void ImageEffect_FilmGrain::slotOk()
{
    enableButton(Ok, false);
    enableButton(User1, false);
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
        
    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    int s      = 400 + 200*m_sensibilitySlider->value();
    
    m_progressBar->setValue(0);
    FilmGrain(data, w, h, s);
        
    if ( !m_cancel )
       iface.putOriginalData(data);
       
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

void ImageEffect_FilmGrain::FilmGrain(uint* data, int Width, int Height, int Sensibility)
{
    int Graininess = (int)(Sensibility / 100.0);
    
    int LineWidth = Width * 4;
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);

    uchar *Bits = (uchar*) data;
    
    int RandValue;

    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    
    srand ((uint) dt.secsTo(Y2000));

    int i = 0, h, w;            
    
    for (h = 0 ; !m_cancel && (h < Height) ; ++h)
       {
       for (w = 0 ; !m_cancel && (w < Width) ; ++w)
          {
          i = h * LineWidth + 4 * w;
                        
          RandValue = (rand() % Graininess);
                    
          if (RandValue % 2)
             RandValue = 0;
                    
          if (((Bits[i+2] + Bits[i+1] + Bits[i]) / 3) > 127)
             RandValue = -RandValue;
                    
          Bits[i+2] = LimitValues (Bits[i+2] + RandValue);
          Bits[i+1] = LimitValues (Bits[i+1] + RandValue);
          Bits[ i ] = LimitValues (Bits[ i ] + RandValue);
          }
       
       // Update de progress bar in dialog.
       m_progressBar->setValue((int) (((double)h * 100.0) / Height));
       kapp->processEvents(); 
       }
}
       
uchar ImageEffect_FilmGrain::LimitValues (int ColorValue)
{
    if (ColorValue > 255)        // MAX = 255
        ColorValue = 255;        
    if (ColorValue < 0)          // MIN = 0
        ColorValue = 0;
    return ((uchar) ColorValue);
}
    
}  // NameSpace DigikamFilmGrainImagesPlugin

#include "imageeffect_filmgrain.moc"
