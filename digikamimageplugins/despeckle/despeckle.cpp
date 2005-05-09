/* ============================================================
 * File  : despeckle.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-24
 * Description : noise reduction image filter for Digikam 
 *               image editor.
 * 
 * Copyright 2004-2005 by Gilles Caulier
 *
 * Despeckle algorithm come from plug-ins/common/despeckle.c 
 * Gimp 2.0 source file and copyrighted 
 * 1997-1998 by Michael Sweet (mike at easysw.com)
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

#define TILE_HEIGHT       64 
 
// C++ include.

#include <cstring>

// Qt includes.

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qimage.h>
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
#include <kdebug.h>
#include <kstandarddirs.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "despeckle.h"

namespace DigikamDespeckleFilterImagesPlugin
{

DespeckleDialog::DespeckleDialog(QWidget* parent)
               : KDialogBase(Plain, i18n("Noise Reduction"), Help|User1|Ok|Cancel, Ok,
                             parent, 0, true, true, i18n("&Reset Values")),
                 m_parent(parent)
{
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    m_cancel = false;
        
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Noise Reduction"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A despeckle image filter plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Michael Sweet", I18N_NOOP("Despeckle algorithm author from Gimp"),
                     "mike at easysw.com");
                         
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Noise Reduction Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Noise Reduction"), headerFrame, "labelTitle" );
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

    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label1 = new QLabel(i18n("Radius:"), plainPage());
    
    m_radiusSlider = new QSlider(1, 20, 1, 3, Qt::Horizontal, plainPage(), "m_radiusSlider");
    m_radiusSlider->setTickmarks(QSlider::Below);
    m_radiusSlider->setTickInterval(1);
    m_radiusSlider->setTracking ( false );
    
    m_radiusInput = new QSpinBox(1, 20, 1, plainPage(), "m_radiusInput");
    m_radiusInput->setValue(3);
    whatsThis = i18n("<p>A radius of 0 has no effect, "
                     "1 and above determine the blur matrix radius "
                     "that determines how much to blur the image.");
    QWhatsThis::add( m_radiusInput, whatsThis);
    QWhatsThis::add( m_radiusSlider, whatsThis);
    
    hlay2->addWidget(label1, 1);
    hlay2->addWidget(m_radiusSlider, 3);
    hlay2->addWidget(m_radiusInput, 1);
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay3 = new QHBoxLayout(topLayout);
    QLabel *label2 = new QLabel(i18n("Black level:"), plainPage());
    
    m_blackLevelSlider = new QSlider(0, 255, 1, 7, Qt::Horizontal, plainPage(), "m_blackLevelSlider");
    m_blackLevelSlider->setTickmarks(QSlider::Below);
    m_blackLevelSlider->setTickInterval(20);
    m_blackLevelSlider->setTracking ( false );  
    
    m_blackLevelInput = new QSpinBox(0, 255, 1, plainPage(), "m_blackLevelInput");
    m_blackLevelInput->setValue(7);    
    whatsThis = i18n("<p>This value controls adjust the black "
                     "levels used by the adaptive filter to "
                     "adjust the filter radius.");
    QWhatsThis::add( m_blackLevelInput, whatsThis);
    QWhatsThis::add( m_blackLevelSlider, whatsThis);                     
    
    hlay3->addWidget(label2, 1);
    hlay3->addWidget(m_blackLevelSlider, 3);
    hlay3->addWidget(m_blackLevelInput, 1);

    // -------------------------------------------------------------

    QHBoxLayout *hlay4 = new QHBoxLayout(topLayout);
    QLabel *label3 = new QLabel(i18n("White level:"), plainPage());
    
    m_whiteLevelSlider = new QSlider(0, 255, 1, 248, Qt::Horizontal, plainPage(), "m_whiteLevelSlider");
    m_whiteLevelSlider->setTickmarks(QSlider::Below);
    m_whiteLevelSlider->setTickInterval(20);
    m_whiteLevelSlider->setTracking ( false );  
    
    m_whiteLevelInput = new QSpinBox(0, 255, 1, plainPage(), "m_whiteLevelInput");
    m_whiteLevelInput->setValue(248);    
    whatsThis = i18n("<p>This value controls adjust the white "
                     "levels used by the adaptive filter to "
                     "adjust the filter radius.");
    
    QWhatsThis::add( m_whiteLevelInput, whatsThis);
    QWhatsThis::add( m_whiteLevelSlider, whatsThis);                         
    
    hlay4->addWidget(label3, 1);
    hlay4->addWidget(m_whiteLevelSlider, 3);
    hlay4->addWidget(m_whiteLevelInput, 1);

    // -------------------------------------------------------------
    
    QHBoxLayout *hlay5 = new QHBoxLayout(topLayout);
    m_useAdaptativeMethod = new QCheckBox( i18n("Adaptive"), plainPage());
    m_useAdaptativeMethod->setChecked( true );
    QWhatsThis::add( m_useAdaptativeMethod, i18n("<p>This option use an adaptive median filter type."));
    m_useRecursiveMethod = new QCheckBox( i18n("Recursive"), plainPage());
    m_useRecursiveMethod->setChecked( false );
    QWhatsThis::add( m_useRecursiveMethod, i18n("<p>This option use a recursive median filter type."));           
    hlay5->addWidget(m_useAdaptativeMethod, 1);
    hlay5->addWidget(m_useRecursiveMethod, 1);

    // -------------------------------------------------------------
            
    adjustSize();
    disableResize(); 
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
    
    // -------------------------------------------------------------
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_radiusSlider, SIGNAL(valueChanged(int)),
            m_radiusInput, SLOT(setValue(int)));
    connect(m_radiusInput, SIGNAL(valueChanged(int)),
            m_radiusSlider, SLOT(setValue(int)));            
    connect(m_radiusInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));            
            
    connect(m_blackLevelSlider, SIGNAL(valueChanged(int)),
            m_blackLevelInput, SLOT(setValue(int)));
    connect(m_blackLevelInput, SIGNAL(valueChanged(int)),
            m_blackLevelSlider, SLOT(setValue(int)));   
    connect(m_blackLevelInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));

    connect(m_whiteLevelSlider, SIGNAL(valueChanged(int)),
            m_whiteLevelInput, SLOT(setValue(int)));
    connect(m_whiteLevelInput, SIGNAL(valueChanged(int)),
            m_whiteLevelSlider, SLOT(setValue(int)));
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
    KApplication::kApplication()->invokeHelp("despeckle", "digikamimageplugins");
}

void DespeckleDialog::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void DespeckleDialog::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void DespeckleDialog::slotUser1()
{
    m_radiusInput->blockSignals(true);
    m_radiusSlider->blockSignals(true);
    m_blackLevelInput->blockSignals(true);
    m_blackLevelSlider->blockSignals(true);
    m_whiteLevelInput->blockSignals(true);
    m_whiteLevelSlider->blockSignals(true);
            
    m_radiusInput->setValue(3);
    m_radiusSlider->setValue(3);
    m_blackLevelInput->setValue(7);
    m_blackLevelSlider->setValue(7);
    m_whiteLevelInput->setValue(248);
    m_whiteLevelSlider->setValue(248);

    m_radiusInput->blockSignals(false);
    m_radiusSlider->blockSignals(false);
    m_blackLevelInput->blockSignals(false);
    m_blackLevelSlider->blockSignals(false);
    m_whiteLevelInput->blockSignals(false);
    m_whiteLevelSlider->blockSignals(false);
    slotEffect();
} 

void DespeckleDialog::slotEffect()
{
    m_radiusInput->setEnabled(false);
    m_radiusSlider->setEnabled(false);
    m_blackLevelInput->setEnabled(false);
    m_blackLevelSlider->setEnabled(false);
    m_whiteLevelInput->setEnabled(false);
    m_whiteLevelSlider->setEnabled(false);
    m_useAdaptativeMethod->setEnabled(false);
    m_useRecursiveMethod->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);

    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    QImage img = m_imagePreviewWidget->getOriginalClipImage();
   
    uint* data = (uint *)img.bits();
    int   w    = img.width();
    int   h    = img.height();
    int   r    = m_radiusSlider->value();
    int   bl   = m_blackLevelSlider->value();
    int   wl   = m_whiteLevelSlider->value();
    bool  af   = m_useAdaptativeMethod->isChecked();
    bool  rf   = m_useRecursiveMethod->isChecked();
    
    m_imagePreviewWidget->setProgress(0);          
    despeckle(data, w, h, r, bl, wl, af, rf);   
    
    if (m_cancel) return;
    
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageData(img);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_radiusInput->setEnabled(true);
    m_radiusSlider->setEnabled(true);
    m_blackLevelInput->setEnabled(true);
    m_blackLevelSlider->setEnabled(true);
    m_whiteLevelInput->setEnabled(true);
    m_whiteLevelSlider->setEnabled(true);
    m_useAdaptativeMethod->setEnabled(true);
    m_useRecursiveMethod->setEnabled(true);
    m_imagePreviewWidget->setEnable(true);
}

void DespeckleDialog::slotOk()
{
    m_radiusInput->setEnabled(false);
    m_radiusSlider->setEnabled(false);
    m_blackLevelInput->setEnabled(false);
    m_blackLevelSlider->setEnabled(false);
    m_whiteLevelInput->setEnabled(false);
    m_whiteLevelSlider->setEnabled(false);
    m_useAdaptativeMethod->setEnabled(false);
    m_useRecursiveMethod->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
    
    uint* data = iface.getOriginalData();
    int   w     = iface.originalWidth();
    int   h     = iface.originalHeight();
    int   r     = m_radiusSlider->value();
    int   bl    = m_blackLevelSlider->value();
    int   wl    = m_whiteLevelSlider->value();
    bool  af    = m_useAdaptativeMethod->isChecked();
    bool  rf    = m_useRecursiveMethod->isChecked();

    m_imagePreviewWidget->setProgress(0);             
    despeckle(data, w, h, r, bl, wl, af, rf);   
    
    if ( !m_cancel )
       iface.putOriginalData(i18n("Despeckle"), data);
       
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
 
     for (y = sel_y1 ; !m_cancel && (y < sel_y2) ; y ++)
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
           for (x = 0 ; !m_cancel && (x < width) ; x ++)
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
           m_imagePreviewWidget->setProgress((int)(100.0*(double) (y - sel_y1) / (double) sel_height));
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
