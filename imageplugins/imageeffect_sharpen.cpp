/* ============================================================
 * File  : imageeffect_sharpen.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-09
 * Description : Sharpen image filter for ImageEditor
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

// Qt includes.

#include <qlayout.h>
#include <qframe.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>

// KDE includes.

#include <knuminput.h>
#include <klocale.h>

// Digikam includes.

#include <imageiface.h>
#include <imagewidget.h>

// Local includes.

#include "imageeffect_sharpen.h"

ImageEffect_Sharpen::ImageEffect_Sharpen(QWidget* parent)
                : KDialogBase(Plain, i18n("Sharpen image"),
                  Ok|Cancel, Ok,
                  parent, 0, true, true)
{
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(),
                                              0, spacingHint());

    QHBoxLayout *hlay  = 0;
    QLabel      *label = 0;

    hlay          = new QHBoxLayout(topLayout);
    label         = new QLabel(i18n("Radius :"), plainPage());
    
    m_radiusInput = new KIntNumInput(plainPage());
    m_radiusInput->setRange(0, 10, 1, true);
    QWhatsThis::add( m_radiusInput, i18n("<p>A radius of 0 has no effect, "
                                         "1 and above determine the sharpen matrix radius "
                                         "that determine how much to sharp the image."));
    
    hlay->addWidget(label,1);
    hlay->addWidget(m_radiusInput, 5);

    m_radiusInput->setValue(1);
    
    adjustSize();
}

ImageEffect_Sharpen::~ImageEffect_Sharpen()
{
}

void ImageEffect_Sharpen::slotOk()
{
    Digikam::ImageIface iface(0, 0);

    bool selection = true;
    
    // Try to work on a selection.
    uint* data = iface.getSelectedData();
    int   w    = iface.selectedWidth();
    int   h    = iface.selectedHeight();
    
    if (!data || !w || !h)
        {
        // If no selection, try to work on the full image.
        data  = iface.getOriginalData();
        w     = iface.originalWidth();
        h     = iface.originalHeight();
        
        if (!data || !w || !h)
           return;
        
        selection = false;           
        }

    uint* newData = new uint[w*h];
    memcpy(newData, data, w*h*sizeof(unsigned int));
    
    Imlib_Context context = imlib_context_new();
    imlib_context_push(context);

    Imlib_Image imTop = imlib_create_image_using_copied_data(w, h, newData);
    imlib_context_set_image(imTop);
    
    imlib_image_sharpen( m_radiusInput->value() );
    
    uint* ptr = imlib_image_get_data_for_reading_only();
    memcpy(data, ptr, w*h*sizeof(unsigned int));
    
    imlib_context_set_image(imTop);
    imlib_free_image_and_decache();
    
    imlib_context_pop();
    imlib_context_free(context);
           
    if ( selection == true )
       iface.putSelectedData(data);
    else 
       iface.putOriginalData(data);
    
    delete [] newData;
    accept();
}

#include "imageeffect_sharpen.moc"
