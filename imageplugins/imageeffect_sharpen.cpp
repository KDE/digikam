/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-09
 * Description : Sharpen image filter for ImageEditor
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * Sharpening filter copied from gimp 2.2
 * Copyright 1997-1998 Michael Sweet (mike@easysw.com)
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

// C++ include.

#include <cstring>

// Qt includes.

#include <qlayout.h>
#include <qframe.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtimer.h>

// KDE includes.

#include <knuminput.h>
#include <kcursor.h>
#include <klocale.h>

// Digikam includes.

#include <imageiface.h>
#include <imagepreviewwidget.h>

// Local includes.

#include "imageeffect_sharpen.h"

ImageEffect_Sharpen::ImageEffect_Sharpen(QWidget* parent)
                   : KDialogBase(Plain, i18n("Sharpen Image"),
                                 Help|Ok|Cancel, Ok,
                                 parent, 0, true, true),
                     m_parent(parent)
{
    setHelp("blursharpentool.anchor", "digikam");
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    m_imagePreviewWidget = new Digikam::ImagePreviewWidget(240, 160, 
                                                           i18n("Sharpen Image Preview Effect"), 
                                                           plainPage());
    hlay1->addWidget(m_imagePreviewWidget);

    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label = new QLabel(i18n("Sharpness:"), plainPage());
    
    m_radiusInput = new KIntNumInput(plainPage());
    m_radiusInput->setRange(0, 100, 1, true);
    QWhatsThis::add( m_radiusInput, i18n("<p>A sharpness of 0 has no effect, "
                                         "1 and above determine the sharpen matrix radius "
                                         "that determines how much to sharpen the image."));
    
    hlay2->addWidget(label, 1);
    hlay2->addWidget(m_radiusInput, 5);

    m_radiusInput->setValue(0);
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));
    
    QTimer::singleShot(0, this, SLOT(slotEffect()));
    adjustSize();
    disableResize();                  
}

ImageEffect_Sharpen::~ImageEffect_Sharpen()
{
}

void ImageEffect_Sharpen::slotEffect()
{
    enableButtonOK(m_radiusInput->value() > 0);

    QImage img = m_imagePreviewWidget->getOriginalClipImage();

    uint* data = (uint *)img.bits();
    int   w    = img.width();
    int   h    = img.height();
    int   r    = m_radiusInput->value();

    sharpen(data, w, h, r);

    m_imagePreviewWidget->setPreviewImageData(img);
}

void ImageEffect_Sharpen::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    int r      = m_radiusInput->value();
            
    sharpen(data, w, h, r);
           
    iface.putOriginalData(data);
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();
}

void ImageEffect_Sharpen::sharpen(uint* data, int w, int h, int r)
{
    // initialize the LUTs

    int fact = 100 - r;
    if (fact < 1)
        fact = 1;

    int           negLUT[256];
    int           posLUT[256];
    
    for (int i = 0; i < 256; i++)
    {
        posLUT[i] = 800 * i / fact;
        negLUT[i] = (4 + posLUT[i] - (i << 3)) >> 3;
    }

    unsigned int* dstData = new unsigned int[w*h];
    
    // work with four rows at one time

    unsigned char* src_rows[4];
    unsigned char* src_ptr;
    unsigned char* dst_row;
    int*           neg_rows[4]; 
    int*           neg_ptr;
    int            row;
    int            count;

    int  width = sizeof(unsigned int)*w;

    for (row = 0; row < 4; row ++)
    {
        src_rows[row] = new unsigned char[width];
        neg_rows[row] = new int[width];
    }       

    dst_row = new unsigned char[width];
    
    // Pre-load the first row for the filter...

    memcpy(src_rows[0], data, width); 

    int i;
    for ( i = width, src_ptr = src_rows[0], neg_ptr = neg_rows[0];
         i > 0;
         i --, src_ptr ++, neg_ptr ++)
        *neg_ptr = negLUT[*src_ptr]; 

    row   = 1;
    count = 1;                                    

    for (int y = 0; y < h; y ++)
    {
        // Load the next pixel row...

        if ((y + 1) < h)
        {
            
            // Check to see if our src_rows[] array is overflowing yet...

            if (count >= 3)
                count --;

            // Grab the next row...

            memcpy(src_rows[row], data + y*w, width); 
            for (i = width, src_ptr = src_rows[row], neg_ptr = neg_rows[row];
                 i > 0;
                 i --, src_ptr ++, neg_ptr ++)
                *neg_ptr = negLUT[*src_ptr];

            count ++;
            row = (row + 1) & 3;
        }

        else
        {
            // No more pixels at the bottom...  Drop the oldest samples...

            count --;
        }

        // Now sharpen pixels and save the results...

        if (count == 3)
        {

            uchar* src  = src_rows[(row + 2) & 3];
            uchar* dst  = dst_row;
            int*   neg0 = neg_rows[(row + 1) & 3] + 4;
            int*   neg1 = neg_rows[(row + 2) & 3] + 4;
            int*   neg2 = neg_rows[(row + 3) & 3] + 4;
            
#define CLAMP0255(a)  QMIN(QMAX(a,0), 255) 
    
            // New pixel value 
            int pixel;         

            int wm = w;
            
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            wm -= 2;

            while (wm > 0)
            {
                pixel = (posLUT[*src++] - neg0[-4] - neg0[0] - neg0[4] -
                         neg1[-4] - neg1[4] -
                         neg2[-4] - neg2[0] - neg2[4]);
                pixel = (pixel + 4) >> 3;
                *dst++ = CLAMP0255 (pixel);

                pixel = (posLUT[*src++] - neg0[-3] - neg0[1] - neg0[5] -
                         neg1[-3] - neg1[5] -
                         neg2[-3] - neg2[1] - neg2[5]);
                pixel = (pixel + 4) >> 3;
                *dst++ = CLAMP0255 (pixel);

                pixel = (posLUT[*src++] - neg0[-2] - neg0[2] - neg0[6] -
                         neg1[-2] - neg1[6] -
                         neg2[-2] - neg2[2] - neg2[6]);
                pixel = (pixel + 4) >> 3;
                *dst++ = CLAMP0255 (pixel);

                *dst++ = *src++;

                neg0 += 4;
                neg1 += 4;
                neg2 += 4;
                wm --;
            }

            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            
            
            // Set the row...
            memcpy(dstData + y*w, dst_row, width); 
        }
        else if (count == 2)
        {
            if (y == 0)
            {
                // first row 
                memcpy(dstData + y*w, src_rows[0], width);
            }
            else
            {
                // last row 
                memcpy(dstData + y*w, src_rows[(h-1) & 3], width);
            }
        }

    }

    memcpy(data, dstData, w*h*sizeof(uint));
    delete [] dstData;
}


#include "imageeffect_sharpen.moc"
