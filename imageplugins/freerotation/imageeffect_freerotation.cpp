/* ============================================================
 * File  : imageeffect_freerotation.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-11-28
 * Description : a Digikam image editor plugin for process image 
 *               free rotation.
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

#define RAD2DEGCONST 0.0174532925

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
#include "freerotationwidget.h"
#include "imageeffect_freerotation.h"

namespace DigikamFreeRotationImagesPlugin
{

ImageEffect_FreeRotation::ImageEffect_FreeRotation(QWidget* parent)
                        : KDialogBase(Plain, i18n("Free Rotation"),
                                      Help|User1|Ok|Cancel, Ok,
                                      parent, 0, true, true,
                                      i18n("&Reset Values")),
                          m_parent(parent)
{
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Free Rotation"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to process image free "
                                       "rotation."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Free Rotation Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Free Rotation"), headerFrame, "labelTitle" );
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
    
    QVGroupBox *gbox = new QVGroupBox(i18n("Free Rotation Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new FreeRotationWidget(480, 320, frame);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the free rotation operation preview."
                                           "If you move the mouse cursor on this preview, "
                                           "a vertical and horizontal dashed line will be draw "
                                           "for guide you to adjust the rotation angle. "
                                           "Press the mouse left button to freeze the dashed "
                                           "lines position."));
                                           
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    topLayout->addWidget(gbox);
    
    // -------------------------------------------------------------
    
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label = new QLabel(i18n("Angle:"), plainPage());
    m_angleInput = new KDoubleNumInput(plainPage());
    m_angleInput->setPrecision(1);
    m_angleInput->setRange(-180.0, 180.0, 0.1, true);
    m_angleInput->setValue(0.0);
    QWhatsThis::add( m_angleInput, i18n("<p>An angle in degrees by which to rotate the image. "
                                        "A positive angle rotates the image clockwise; "
                                        "a negative angle rotates it counter-clockwise."));
    hlay->addWidget(label, 1);
    hlay->addWidget(m_angleInput, 5);
    
    adjustSize();
    disableResize();

    // -------------------------------------------------------------
    
    connect(m_angleInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));
}

ImageEffect_FreeRotation::~ImageEffect_FreeRotation()
{
}

void ImageEffect_FreeRotation::slotUser1()
{
    blockSignals(true);
                   
    m_angleInput->setValue(0.0);
    
    blockSignals(false);
    slotEffect();  
} 

void ImageEffect_FreeRotation::slotHelp()
{
    KApplication::kApplication()->invokeHelp("freerotation",
                                             "digikamimageplugins");
}

void ImageEffect_FreeRotation::slotEffect()
{
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint*  data  = iface->getPreviewData();
    int    w     = iface->previewWidth();
    int    h     = iface->previewHeight();
    double angle = m_angleInput->value();
    
    Imlib_Context context = imlib_context_new();
    imlib_context_push(context);

    Imlib_Image imTop = imlib_create_image_using_copied_data(w, h, data);
    imlib_context_set_image(imTop);

    // Imlib2 use an angle in Radian, not in Degrees. We must convert that !
    Imlib_Image im = imlib_create_rotated_image( angle * RAD2DEGCONST );
    
    // Calc distance for each angles to use for rotation.
    int d1 = abs((int)((double)(h)*sin( angle * RAD2DEGCONST ) ));
    int d2 = abs((int)((double)(w)*cos( angle * RAD2DEGCONST ) ));
    int d3 = abs((int)((double)(w)*sin( angle * RAD2DEGCONST ) ));
    int d4 = abs((int)((double)(h)*cos( angle * RAD2DEGCONST ) ));
    
    imlib_free_image();
    imlib_context_set_image(im);

    // Get the original image center.
    int center_x = (int)((float)(imlib_image_get_width()) / 2.0);
    int center_y = (int)((float)(imlib_image_get_height()) / 2.0);
    
    // Cropped operation to remove empty data around image generated by imlib2 rotate method.        
    Imlib_Image im2 = imlib_create_cropped_image(center_x - (int)((float)(d1 + d2) / 2.0),
                                                 center_y - (int)((float)(d3 + d4) / 2.0),
                                                 d1 + d2, d3 + d4);

    imlib_context_set_image(im2);

    //imlib_image_draw_rectangle(d1, d3, d4-d1, d2-d3);
    QSize sz(imlib_image_get_width(), imlib_image_get_height());
    sz.scale(w, h, QSize::ScaleMin);
    
    // Scale image to the target preview size.
    Imlib_Image im3 = imlib_create_cropped_scaled_image(0, 0, imlib_image_get_width(), 
                                                        imlib_image_get_height(), 
                                                        sz.width(), sz.height());

    // Create empty target image with the preview dim.                                                        
    Imlib_Image im4 = imlib_create_image(w, h);
    imlib_context_set_image(im4);

    // Blend rotated image to target image     
    imlib_blend_image_onto_image(im3, 1, 0, 0, sz.width(), sz.height(), 
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
    
    imlib_context_set_image(im4);
    imlib_free_image_and_decache();

    imlib_context_pop();
    imlib_context_free(context);
    
    iface->putPreviewData(data);

    delete [] data;
    
    m_previewWidget->update();
}

void ImageEffect_FreeRotation::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
        
    uint*  data  = iface.getOriginalData();
    int    w     = iface.originalWidth();
    int    h     = iface.originalHeight();
    double angle = m_angleInput->value();

    Imlib_Context context = imlib_context_new();
    imlib_context_push(context);

    Imlib_Image imTop = imlib_create_image_using_copied_data(w, h, data);
    imlib_context_set_image(imTop);

    // Imlib2 use an angle in Radian, not in Degrees. We must convert that !
    Imlib_Image im = imlib_create_rotated_image( angle * RAD2DEGCONST );
    
    // Calc distance for each angles to use for rotation.
    int d1 = abs((int)((double)(h)*sin( angle * RAD2DEGCONST ) ));
    int d2 = abs((int)((double)(w)*cos( angle * RAD2DEGCONST ) ));
    int d3 = abs((int)((double)(w)*sin( angle * RAD2DEGCONST ) ));
    int d4 = abs((int)((double)(h)*cos( angle * RAD2DEGCONST ) ));
    
    imlib_context_set_image(im);
    
    // Get the original image center.
    int center_x = (int)((float)(imlib_image_get_width()) / 2.0);
    int center_y = (int)((float)(imlib_image_get_height()) / 2.0);
    
    // Cropped operation to remove empty data around image generated by imlib2 rotate method.        
    Imlib_Image im2 = imlib_create_cropped_image(center_x - (int)((float)(d1 + d2) / 2.0),
                                                 center_y - (int)((float)(d3 + d4) / 2.0),
                                                 d1 + d2, d3 + d4);

    imlib_context_set_image(im2);
    
    uint* ptr  = imlib_image_get_data_for_reading_only();
    int   newW = imlib_image_get_width();
    int   newH = imlib_image_get_height();

    iface.putOriginalData(ptr, newW, newH);   
    
    imlib_context_set_image(imTop);
    imlib_free_image_and_decache();

    imlib_context_set_image(im);
    imlib_free_image_and_decache();

    imlib_context_set_image(im2);
    imlib_free_image_and_decache();
    
    imlib_context_pop();
    imlib_context_free(context);
    
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

}  // NameSpace DigikamFreeRotationImagesPlugin

#include "imageeffect_freerotation.moc"
