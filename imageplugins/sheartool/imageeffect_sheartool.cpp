/* ============================================================
 * File  : imageeffect_sheartool.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-23
 * Description : a Digikam image editor plugin for process 
 *               shearing image.
 * 
 * Copyright 2004 by Gilles Caulier
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

// Imlib2 include.

#define X_DISPLAY_MISSING 1
#include <Imlib2.h>

// C++ includes.

#include <cmath>
#include <cstdio>
#include <cstdlib>
 
// Qt includes. 
 
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>

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

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imageeffect_sheartool.h"

namespace DigikamShearToolImagesPlugin
{

ImageEffect_ShearTool::ImageEffect_ShearTool(QWidget* parent)
                     : KDialogBase(Plain, i18n("Shear Tool"),
                                   Help|User1|Ok|Cancel, Ok,
                                   parent, 0, true, true,
                                   i18n("&Reset Values")),
                        m_parent(parent)
{
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all parameters to the default values.") );
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Shear Tool"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to shear an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Shear Tool Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Shear Tool"), headerFrame, "labelTitle" );
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
    m_previewWidget = new Digikam::ImageGuideWidget(480, 320, frame);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the shearing image operation preview. "
                                           "If you move the mouse cursor on this preview, "
                                           "a vertical and horizontal dashed line will be drawn "
                                           "to guide you in adjusting the shearing correction. "
                                           "Press the left mouse button to freeze the dashed "
                                           "line's position."));
                                           
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    topLayout->addWidget(gbox);
    
    // -------------------------------------------------------------

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    m_ratioW = (float)(iface->previewWidth()) / (float)(iface->originalWidth());
    m_ratioH = (float)(iface->previewHeight()) / (float)(iface->originalHeight());
    
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label = new QLabel(i18n("Magnitude X:"), plainPage());
    m_magnitudeX = new KIntNumInput(plainPage());
    m_magnitudeX->setRange((int)((-2.0/3.0)*iface->originalWidth()), (int)((2.0/3.0)*iface->originalWidth()), 1, true);
    m_magnitudeX->setValue(0);
    QWhatsThis::add( m_magnitudeX, i18n("<p>The X-shearing magnitude, in pixels."));
    hlay->addWidget(label, 1);
    hlay->addWidget(m_magnitudeX, 5);
        
    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label2 = new QLabel(i18n("Magnitude Y:"), plainPage());
    m_magnitudeY = new KIntNumInput(plainPage());
    m_magnitudeY->setRange((int)((-2.0/3.0)*iface->originalHeight()), (int)((2.0/3.0)*iface->originalHeight()), 1, true);
    m_magnitudeY->setValue(0);
    QWhatsThis::add( m_magnitudeY, i18n("<p>The Y-shearing magnitude, in pixels."));
    hlay2->addWidget(label2, 1);
    hlay2->addWidget(m_magnitudeY, 5);
    
    adjustSize();
    disableResize();

    // -------------------------------------------------------------
    
    connect(m_magnitudeX, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));
    
    connect(m_magnitudeY, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));
}

ImageEffect_ShearTool::~ImageEffect_ShearTool()
{
}

void ImageEffect_ShearTool::slotUser1()
{
    blockSignals(true);
                   
    m_magnitudeX->setValue(0);
    m_magnitudeY->setValue(0);
    
    blockSignals(false);
    slotEffect();  
} 

void ImageEffect_ShearTool::slotHelp()
{
    KApplication::kApplication()->invokeHelp("sheartool",
                                             "digikamimageplugins");
}

void ImageEffect_ShearTool::slotEffect()
{
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data = iface->getPreviewData();
    int   w    = iface->previewWidth();
    int   h    = iface->previewHeight();
    int   mX   = (int)(m_magnitudeX->value() * m_ratioW);
    int   mY   = (int)(m_magnitudeY->value() * m_ratioH);
   
    Imlib_Context context = imlib_context_new();
    imlib_context_push(context);

    Imlib_Image imTop = imlib_create_image_using_copied_data(w, h, data);
    
    int X1=1, Y2=1;
    
    if (abs(mY) < w) 
       X1 = (int)(sqrt(w*w - mY*mY));
     
    if (abs(mX) < h) 
       Y2 = (int)(sqrt(h*h - mX*mX));
    
    Imlib_Image im = imlib_create_image(X1 + abs(mX), Y2 + abs(mY));
    imlib_context_set_image(im);
    
    imlib_blend_image_onto_image_skewed(imTop, 1, 
                                        0, 0, w, h, 
                                        mX < 0 ? abs(mX) : 0, 
                                        mY < 0 ? abs(mY) : 0,
                                        X1, mY,
                                        mX, Y2);
                                            
    QSize sz(imlib_image_get_width(), imlib_image_get_height());
    sz.scale(w, h, QSize::ScaleMin);
    
    // Scale image to the target preview size.
    Imlib_Image im2 = imlib_create_cropped_scaled_image(0, 0, imlib_image_get_width(), 
                                                        imlib_image_get_height(), 
                                                        sz.width(), sz.height());

    // Create empty target image with the preview dim.                    
                                        
    Imlib_Image im3 = imlib_create_image(w, h);
    imlib_context_set_image(im3);

    // Blend sheared image to target image     
    imlib_blend_image_onto_image(im2, 1, 0, 0, sz.width(), sz.height(), 
                                 (int)((float)(w - sz.width())/2.0), 
                                 (int)((float)(h - sz.height())/2.0),
                                 sz.width(), sz.height());
                                                                      
    uint* ptr = imlib_image_get_data_for_reading_only();
    memcpy(data, ptr, w * h * sizeof(unsigned int));
    
    imlib_context_set_image(imTop);
    imlib_free_image_and_decache();

    imlib_context_set_image(im);
    imlib_free_image_and_decache();

    imlib_context_set_image(im2);
    imlib_free_image_and_decache();
    
    imlib_context_set_image(im3);
    imlib_free_image_and_decache();

    imlib_context_pop();
    imlib_context_free(context);
    
    iface->putPreviewData(data);

    delete [] data;
    
    m_previewWidget->update();
}

void ImageEffect_ShearTool::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
        
    uint* data = iface.getOriginalData();
    int   w    = iface.originalWidth();
    int   h    = iface.originalHeight();
    int   mX   = m_magnitudeX->value();
    int   mY   = m_magnitudeY->value();

    Imlib_Context context = imlib_context_new();
    imlib_context_push(context);

    Imlib_Image imTop = imlib_create_image_using_copied_data(w, h, data);
    imlib_context_set_image(imTop);

    int X1=1, Y2=1;
    
    if (abs(mY) < w) 
       X1 = (int)(sqrt(w*w - mY*mY));
     
    if (abs(mX) < h) 
       Y2 = (int)(sqrt(h*h - mX*mX));
    
    Imlib_Image im = imlib_create_image(X1 + abs(mX), Y2 + abs(mY));
    imlib_context_set_image(im);
    
    imlib_blend_image_onto_image_skewed(imTop, 1, 
                                        0, 0, w, h, 
                                        mX < 0 ? abs(mX) : 0, 
                                        mY < 0 ? abs(mY) : 0,
                                        X1, mY,
                                        mX, Y2);
    
    uint* ptr  = imlib_image_get_data_for_reading_only();
    int   newW = imlib_image_get_width();
    int   newH = imlib_image_get_height();

    iface.putOriginalData(ptr, newW, newH);   
    
    imlib_context_set_image(imTop);
    imlib_free_image_and_decache();

    imlib_context_set_image(im);
    imlib_free_image_and_decache();

    imlib_context_pop();
    imlib_context_free(context);
    
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

}  // NameSpace DigikamShearToolImagesPlugin

#include "imageeffect_sheartool.moc"
