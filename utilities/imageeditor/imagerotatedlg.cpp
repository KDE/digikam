/* ============================================================
 * File  : imagerotatedlg.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-16
 * Description : a dialog for free resizing image operations.
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
#include <qlayout.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qsize.h>
#include <qtimer.h>

// KDE includes.

#include <klocale.h>
#include <knuminput.h>

// Local includes.

#include "imageiface.h"
#include "imagetoolswidget.h"
#include "imagerotatedlg.h"

ImageRotateDlg::ImageRotateDlg(QWidget *parent, double *angle)
              : KDialogBase(Plain, i18n("Rotate Image"), Help|User1|Ok|Cancel, Ok,
                            parent, 0, true, true, i18n("&Reset Values"))
{
    setHelp("freerotationtool.anchor", "digikam");
    
    m_angle = angle;

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());
    
    // -------------------------------------------------------------
    
    QVGroupBox *gbox = new QVGroupBox(i18n("Free Rotation Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageToolsWidget(480, 320, frame);
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
    
    // -------------------------------------------------------------
    
    connect(m_angleInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));
            
    adjustSize();
    disableResize();
}

ImageRotateDlg::~ImageRotateDlg()
{
}

void ImageRotateDlg::slotUser1()
{
    blockSignals(true);
    disconnect(m_angleInput, SIGNAL(valueChanged (double)),
               this, SLOT(slotEffect()));
                   
    m_angleInput->setValue(0.0);
    
    connect(m_angleInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));
            
    blockSignals(false);
    slotEffect();    
} 

void ImageRotateDlg::slotEffect()
{
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint * data = iface->getPreviewData();
    int w       = iface->previewWidth();
    int h       = iface->previewHeight();

    double angle = m_angleInput->value();

    freerotation(angle, data, w, h);

    iface->putPreviewData(data);

    delete [] data;

    m_previewWidget->update();
}

void ImageRotateDlg::slotOk()
{
    *m_angle = m_angleInput->value();
    accept();
}

void ImageRotateDlg::freerotation(double angle, uint *data, int w, int h)
{
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
                                 (int)((float)(w-sz.width())/2.0), 
                                 (int)((float)(h-sz.height())/2.0),
                                 sz.width(), sz.height());
                                 
    uint* ptr = imlib_image_get_data_for_reading_only();
    memcpy(data, ptr, w*h*sizeof(unsigned int));
    
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
}

#include "imagerotatedlg.moc"
