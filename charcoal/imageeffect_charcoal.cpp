/* ============================================================
 * File  : imageeffect_charcoal.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a digikam image editor plugin for to
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
#include <kimageeffect.h>
#include <knuminput.h>
#include <kstandarddirs.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imageeffect_charcoal.h"

namespace DigikamCharcoalImagesPlugin
{

ImageEffect_Charcoal::ImageEffect_Charcoal(QWidget* parent)
                    : KDialogBase(Plain, i18n("Charcoal Drawing"),
                                  Help|User1|Ok|Cancel, Ok,
                                  parent, 0, true, true, i18n("&Reset Values")),
                      m_parent(parent)
{
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    m_cancel = false;
    
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
    
    m_imagePreviewWidget = new Digikam::ImagePreviewWidget(240, 160, i18n("Preview"), plainPage(), true);
    hlay1->addWidget(m_imagePreviewWidget);
    
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setProgressWhatsThis(i18n("<p>This is the current percentage of the task completed."));
    
    // -------------------------------------------------------------
    
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label1 = new QLabel(i18n("Pencil size:"), plainPage());
    
    m_pencilSlider = new QSlider(1, 100, 1, 30, Qt::Horizontal, plainPage(), "m_pencilSlider");
    m_pencilSlider->setTickmarks(QSlider::Below);
    m_pencilSlider->setTickInterval(10);
    m_pencilSlider->setTracking ( false );
    
    m_pencilInput = new QSpinBox(1, 100, 1, plainPage(), "m_pencilInput");
    m_pencilInput->setValue(30);
    whatsThis = i18n("<p>Set here the charcoal pencil size used to simulate the drawing.");
        
    QWhatsThis::add( m_pencilInput, whatsThis);
    QWhatsThis::add( m_pencilSlider, whatsThis);

    hlay->addWidget(label1, 1);
    hlay->addWidget(m_pencilSlider, 3);
    hlay->addWidget(m_pencilInput, 1);
    
    // -------------------------------------------------------------
    
    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label2 = new QLabel(i18n("Smooth:"), plainPage());
    
    m_smoothSlider = new QSlider(1, 100, 1, 10, Qt::Horizontal, plainPage(), "m_smoothSlider");
    m_smoothSlider->setTickmarks(QSlider::Below);
    m_smoothSlider->setTickInterval(10);
    m_smoothSlider->setTracking ( false );
    
    m_smoothInput = new QSpinBox(1, 100, 1, plainPage(), "m_smoothInput");
    m_smoothInput->setValue(10);    
    whatsThis = i18n("<p>This value controls the smoothing effect of the pencil under the canvas.");
    
    QWhatsThis::add( m_smoothSlider, whatsThis);
    QWhatsThis::add( m_smoothInput, whatsThis);
                 
    hlay2->addWidget(label2, 1);
    hlay2->addWidget(m_smoothSlider, 3);
    hlay2->addWidget(m_smoothInput, 1);

    // -------------------------------------------------------------
    
    adjustSize();
    disableResize();        
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
    
    // -------------------------------------------------------------
        
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_pencilSlider, SIGNAL(valueChanged(int)),
            m_pencilInput, SLOT(setValue(int)));
    connect(m_pencilInput, SIGNAL(valueChanged(int)),
            m_pencilSlider, SLOT(setValue(int)));            
    connect(m_pencilInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));  
                
    connect(m_smoothSlider, SIGNAL(valueChanged(int)),
            m_smoothInput, SLOT(setValue(int)));
    connect(m_smoothInput, SIGNAL(valueChanged(int)),
            m_smoothSlider, SLOT(setValue(int)));            
    connect(m_smoothInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));  
}

ImageEffect_Charcoal::~ImageEffect_Charcoal()
{
}

void ImageEffect_Charcoal::slotUser1()
{
    m_pencilInput->blockSignals(true);
    m_pencilInput->blockSignals(true);
    m_smoothInput->blockSignals(true);
    m_smoothSlider->blockSignals(true);
            
    m_pencilInput->setValue(30);
    m_pencilSlider->setValue(30);
    m_smoothInput->setValue(10);
    m_smoothSlider->setValue(10);
    
    m_pencilInput->blockSignals(false);
    m_pencilInput->blockSignals(false);
    m_smoothInput->blockSignals(false);
    m_smoothSlider->blockSignals(false);
    slotEffect();
} 

void ImageEffect_Charcoal::slotHelp()
{
    KApplication::kApplication()->invokeHelp("charcoal", "digikamimageplugins");
}

void ImageEffect_Charcoal::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_Charcoal::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_Charcoal::slotEffect()
{
    m_imagePreviewWidget->setEnable(false);
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    m_pencilInput->setEnabled(false);
    m_pencilSlider->setEnabled(false);
    m_smoothInput->setEnabled(false);
    m_smoothSlider->setEnabled(false);
    int pencil = m_pencilSlider->value();
    int smooth  = m_smoothSlider->value();
    m_imagePreviewWidget->setProgress(0);
    
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    QImage newImage = charcoal(image, (double)pencil/10.0, (double)smooth/10.0);

    if (m_cancel) return;
    
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageData(newImage);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_pencilInput->setEnabled(true);
    m_pencilSlider->setEnabled(true);
    m_smoothInput->setEnabled(true);
    m_smoothSlider->setEnabled(true);
    m_imagePreviewWidget->setEnable(true);
}

void ImageEffect_Charcoal::slotOk()
{
    m_pencilInput->setEnabled(false);
    m_pencilSlider->setEnabled(false);
    m_smoothInput->setEnabled(false);
    m_smoothSlider->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
    
    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    int pencil = m_pencilSlider->value();
    int smooth  = m_smoothSlider->value();

    m_imagePreviewWidget->setProgress(0);
        
    if (data) 
        {
        QImage image( w, h, 32 );
        memcpy(image.bits(), data, image.numBytes());

        QImage newImage = charcoal(image, (double)pencil/10.0, (double)smooth/10.0);
        memcpy(data, newImage.bits(), newImage.numBytes());
        
        if ( !m_cancel )
           iface.putOriginalData(i18n("Charcoal"), data);
        
        delete [] data;
        }
    
    m_parent->setCursor( KCursor::arrowCursor() );        
    accept();
}

QImage ImageEffect_Charcoal::charcoal(QImage &src, double pencil, double smooth)
{
    if (m_cancel) return src;
    m_imagePreviewWidget->setProgress(0);
    kapp->processEvents();    
    
    // Detects edges in the image using pixel neighborhoods and an edge
    // detection mask.
    QImage img(KImageEffect::edge(src, pencil));
    m_imagePreviewWidget->setProgress(10);
    kapp->processEvents();    
           
    if (m_cancel) return src;
    m_imagePreviewWidget->setProgress(20);
    kapp->processEvents();    
    
    // Blurs the image by convolving pixel neighborhoods.
#if KDE_VERSION >= 0x30200
    img = KImageEffect::blur(img, pencil, smooth);
#else
    img = KImageEffect::blur(img, pencil);
#endif
    m_imagePreviewWidget->setProgress(30);
    kapp->processEvents();    
    
    if (m_cancel) return src;
    m_imagePreviewWidget->setProgress(40);
    kapp->processEvents();    
    
    // Normalises the pixel values to span the full range of color values.
    // This is a contrast enhancement technique.
    KImageEffect::normalize(img);
    m_imagePreviewWidget->setProgress(50);
    kapp->processEvents();    
    
    if (m_cancel) return src;
    m_imagePreviewWidget->setProgress(60);
    kapp->processEvents();    
    
    // Invert the pixels values.
    img.invertPixels(false);
    m_imagePreviewWidget->setProgress(70);
    kapp->processEvents();    
    
    if (m_cancel) return src;
    m_imagePreviewWidget->setProgress(80);
    kapp->processEvents();    
    
    // Convert image to grayscale.
    KImageEffect::toGray(img);
    m_imagePreviewWidget->setProgress(90);
    kapp->processEvents();    
    
    if (m_cancel) return src;
    m_imagePreviewWidget->setProgress(100);
    kapp->processEvents();    
    
    return(img);
}

}  // NameSpace DigikamCharcoalImagesPlugin

#include "imageeffect_charcoal.moc"
