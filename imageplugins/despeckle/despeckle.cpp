/* ============================================================
 * File  : despeckle.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-24
 * Description : Despeckle image filter for ImageEditor
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * Despeckle algorithm come from plug-ins/common/despeckle.c 
 * Gimp 2.0 source file and copyrighted 
 * 1997-1998 by Michael Sweet (mike at easysw.com)
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


#define TILE_HEIGHT       64 
 
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
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qimage.h>

// KDE includes.

#include <knuminput.h>
#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kimageeffect.h>
#include <kprogress.h>
#include <kdebug.h>

// Digikam includes.

#include <imageiface.h>
#include <imagepreviewwidget.h>

// Local includes.

#include "despeckle.h"

namespace DigikamDespeckleFilterImagesPlugin
{

DespeckleDialog::DespeckleDialog(QWidget* parent)
               : KDialogBase(Plain, i18n("Noise reduction"), Help|User1|Ok|Cancel, Ok,
                             parent, 0, true, true, i18n("&Reset values")),
                 m_parent(parent)
{
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Noise reduction"), 
                                       "0.7.0-cvs",
                                       I18N_NOOP("A despeckle image filter plugin for Digikam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Gilles Caulier", 
                                       0,
                                       "http://digikam.sourceforge.net");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Michael Sweet", I18N_NOOP("Despeckle algorithm author from Gimp"),
                     "mike at easysw.com");
                         
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Noise reduction handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePreviewWidget(240, 160, 
                                                           i18n("Noise reduction image preview"),
                                                           plainPage());
    hlay1->addWidget(m_imagePreviewWidget);
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label1 = new QLabel(i18n("Radius :"), plainPage());
    m_radiusInput = new KIntNumInput(plainPage());
    m_radiusInput->setRange(1, 20, 1, true);
    QWhatsThis::add( m_radiusInput, i18n("<p>A radius of 0 has no effect, "
                                         "1 and above determine the blur matrix radius "
                                         "that determines how much to blur the image."));
    hlay2->addWidget(label1, 1);
    hlay2->addWidget(m_radiusInput, 5);
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay3 = new QHBoxLayout(topLayout);
    QLabel *label2 = new QLabel(i18n("Black level :"), plainPage());
    m_blackLevelInput = new KIntNumInput(plainPage());
    m_blackLevelInput->setRange(0, 255, 1, true);
    QWhatsThis::add( m_blackLevelInput, i18n("<p>This value controls adjust the black "
                                             "levels used by the adaptive filter to "
                                             "adjust the filter radius."));
    hlay3->addWidget(label2, 1);
    hlay3->addWidget(m_blackLevelInput, 5);

    // -------------------------------------------------------------

    QHBoxLayout *hlay4 = new QHBoxLayout(topLayout);
    QLabel *label3 = new QLabel(i18n("White level :"), plainPage());
    m_whiteLevelInput = new KIntNumInput(plainPage());
    m_whiteLevelInput->setRange(0, 255, 1, true);
    QWhatsThis::add( m_whiteLevelInput, i18n("<p>This value controls adjust the white "
                                             "levels used by the adaptive filter to "
                                             "adjust the filter radius."));
    hlay4->addWidget(label3, 1);
    hlay4->addWidget(m_whiteLevelInput, 5);

    // -------------------------------------------------------------
    
    QHBoxLayout *hlay5 = new QHBoxLayout(topLayout);
    m_useAdaptativeMethod = new QCheckBox( i18n("Adaptative"), plainPage());
    m_useAdaptativeMethod->setChecked( true );
    QWhatsThis::add( m_useAdaptativeMethod, i18n("<p>This option use an adaptive median filter type."));
    m_useRecursiveMethod = new QCheckBox( i18n("Recursive"), plainPage());
    m_useRecursiveMethod->setChecked( false );
    QWhatsThis::add( m_useRecursiveMethod, i18n("<p>This option use a recursive media filter type."));           
    hlay5->addWidget(m_useAdaptativeMethod, 1);
    hlay5->addWidget(m_useRecursiveMethod, 1);

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
    
    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));

    connect(m_blackLevelInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));

    connect(m_whiteLevelInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));                                                
            
    connect(m_useAdaptativeMethod, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));             
    
    connect(m_useRecursiveMethod, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));             
}

DespeckleDialog::~DespeckleDialog()
{
}

void DespeckleDialog::slotHelp()
{
    KApplication::kApplication()->invokeHelp("despeckle",
                                             "digikamimageplugins");
}

void DespeckleDialog::closeEvent(QCloseEvent *e)
{
    e->accept();    
}

void DespeckleDialog::slotUser1()
{
    blockSignals(true);
    m_radiusInput->setValue(3);
    m_blackLevelInput->setValue(7);
    m_whiteLevelInput->setValue(248);
    blockSignals(false);
    
    slotEffect();
} 

void DespeckleDialog::slotEffect()
{
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    QImage img = m_imagePreviewWidget->getOriginalClipImage();
   
    uint* data = (uint *)img.bits();
    int   w    = img.width();
    int   h    = img.height();
    int   r    = m_radiusInput->value();
    int   bl   = m_blackLevelInput->value();
    int   wl   = m_whiteLevelInput->value();
    bool  af   = m_useAdaptativeMethod->isChecked();
    bool  rf   = m_useRecursiveMethod->isChecked();
    
    m_progressBar->setValue(0);              
    despeckle(data, w, h, r, bl, wl, af, rf);   
    m_progressBar->setValue(0);  
    
    memcpy(img.bits(), (uchar *)data, img.numBytes());
    m_imagePreviewWidget->setPreviewImageData(img);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
}

void DespeckleDialog::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
    
    uint* data = iface.getOriginalData();
    int   w     = iface.originalWidth();
    int   h     = iface.originalHeight();
    int   r     = m_radiusInput->value();
    int   bl    = m_blackLevelInput->value();
    int   wl    = m_whiteLevelInput->value();
    bool  af    = m_useAdaptativeMethod->isChecked();
    bool  rf    = m_useRecursiveMethod->isChecked();

    m_progressBar->setValue(0);               
    despeckle(data, w, h, r, bl, wl, af, rf);   
           
    iface.putOriginalData(data);
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();
}

/*
 * Despeckle an image using a median filter.
 *
 * A median filter basically collects pixel values in a region around the
 * target pixel, sorts them, and uses the median value. This code uses a
 * circular row buffer to improve performance.
 *
 * The adaptive filter is based on the median filter but analizes the histogram
 * of the region around the target pixel and adjusts the despeckle radius
 * accordingly.
 */

void DespeckleDialog::despeckle(uint* data, int w, int h, int despeckle_radius, 
                                int black_level, int white_level, 
                                bool adaptativeFilter, bool recursiveFilter)
{
    uchar      **src_rows,       // Source pixel rows 
                *dst_row,        // Destination pixel row 
                *src_ptr,        // Source pixel pointer 
                *sort,           // Pixel value sort array 
                *sort_ptr;       // Current sort value 
    
    int          sort_count,     // Number of soft values 
                 i, j, t, d,     // Looping vars 
                 x, y,           // Current location in image 
                 row,            // Current row in src_rows 
                 rowcount,       // Number of rows loaded 
                 lasty,          // Last row loaded in src_rows 
                 trow,           // Looping var 
                 startrow,       // Starting row for loop 
                 endrow,         // Ending row for loop 
                 max_row,        // Maximum number of filled src_rows 
                 size,           // Width/height of the filter box 
                 width,          // Byte width of the image 
                 xmin, xmax, tx, // Looping vars 
                 radius,         // Current radius 
                 hist0,          // Histogram count for 0 values 
                 hist255;        // Histogram count for 255 values 

     int         sel_x1 = 0,     // Selection bounds : always the full image data !
                 sel_y1 = 0,
                 sel_y2 = h;
                 
     int         sel_width = w;  // Selection width 
     int         sel_height = h; // Selection height                  
     int         img_bpp = 4;    // Bytes-per-pixel in image
                 
     QImage      image, region;                 
                 
     // Setup for filter...

     image.create( w, h, 32 );
     image.setAlphaBuffer(true) ;
     memcpy(image.bits(), data, image.numBytes());

     size        = despeckle_radius * 2 + 1;
     
     max_row     = 2 * TILE_HEIGHT;
     
     width       = w * img_bpp;

     src_rows    = new uchar*[max_row];
     src_rows[0] = new uchar[max_row * width];

     for (row = 1 ; row < max_row ; row ++)
         src_rows[row] = src_rows[0] + row * width;

     dst_row = new uchar[width],
     sort    = new uchar[size * size];

     // Pre-load the first "size" rows for the filter...

     if ( h < TILE_HEIGHT )
        rowcount = h;
     else
        rowcount = TILE_HEIGHT;

     region = image.copy(sel_x1, sel_y1, sel_width, rowcount);
     memcpy(src_rows[0], region.bits(), region.numBytes());
     
     row   = rowcount;
     lasty = sel_y1 + rowcount;

     // Despeckle...
 
     for (y = sel_y1 ; y < sel_y2; y ++)
        {
        if ((y + despeckle_radius) >= lasty && lasty < sel_y2)
           {
           // Load the next block of rows...
       
           rowcount -= TILE_HEIGHT;
           
           if ((i = sel_y2 - lasty) > TILE_HEIGHT)
              i = TILE_HEIGHT;

           region = image.copy(sel_x1, lasty, sel_width, i);
           memcpy(src_rows[row], region.bits(), region.numBytes());
     
           rowcount += i;
           lasty    += i;
           row      = (row + i) % max_row;
           };

        // Now find the median pixels and save the results...
      
        radius = despeckle_radius;

        memcpy (dst_row, src_rows[(row + y - lasty + max_row) % max_row], width);

        if (y >= (sel_y1 + radius) && y < (sel_y2 - radius))
           {
           for (x = 0; x < width; x ++)
              {
              hist0   = 0;
              hist255 = 0;
              xmin    = x - radius * img_bpp;
              xmax    = x + (radius + 1) * img_bpp;

              if (xmin < 0)
                 xmin = x % img_bpp;

              if (xmax > width)
                 xmax = width;

              startrow = (row + y - lasty - radius + max_row) % max_row;
              endrow   = (row + y - lasty + radius + 1 + max_row) % max_row;

              for (sort_ptr = sort, trow = startrow ;
                   trow != endrow ; trow = (trow + 1) % max_row)
                 for (tx = xmin, src_ptr = src_rows[trow] + xmin;
                      tx < xmax;
                      tx += img_bpp, src_ptr += img_bpp)
                    {
                    if ((*sort_ptr = *src_ptr) <= black_level)
                       hist0 ++;
                    else if (*sort_ptr >= white_level)
                       hist255 ++;

                    if (*sort_ptr < white_level && *sort_ptr > black_level)
                       sort_ptr ++;
                    };

              // Shell sort the color values...
           
              sort_count = sort_ptr - sort;

              if (sort_count > 1)
                 {
                 for (d = sort_count / 2; d > 0; d = d / 2)
                    for (i = d; i < sort_count; i ++)
                       for (j = i - d, sort_ptr = sort + j;
                            j >= 0 && sort_ptr[0] > sort_ptr[d];
                            j -= d, sort_ptr -= d)
                          {
                          t           = sort_ptr[0];
                          sort_ptr[0] = sort_ptr[d];
                          sort_ptr[d] = t;
                          };

                 // Assign the median value...
           
                 t = sort_count / 2;

                 if (sort_count & 1)
                    dst_row[x] = (sort[t] + sort[t + 1]) / 2;
                 else
                    dst_row[x] = sort[t];

                 // Save the change to the source image too if the user
                 // wants the recursive method...

                 if (recursiveFilter)
                    src_rows[(row + y - lasty + max_row) % max_row][x] = dst_row[x];
                 };

              // Check the histogram and adjust the radius accordingly...

              if (adaptativeFilter)
                 {
                 if (hist0 >= radius || hist255 >= radius)
                    {
                    if (radius < despeckle_radius)
                       radius ++;
                    }
                 else if (radius > 1)
                    radius --;
                 };
              };
           };
         
        memcpy (data + (w * y), dst_row, width);
        
        if ((y & 15) == 0)
           {
           m_progressBar->setValue((int)(100.0*(double) (y - sel_y1) / (double) sel_height));
           kapp->processEvents();
           }
        };

     // OK, we're done.  Free all memory used...
   
     delete [] src_rows; 
     delete [] dst_row;
     delete [] sort;
}

}  // NameSpace DigikamDespeckleFilterImagesPlugin

#include "despeckle.moc"
