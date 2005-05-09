/* ============================================================
 * File  : unsharp.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-27
 * Description : Unsharp mask image filter for ImageEditor
 * 
 * Copyright 2004-2005 by Gilles Caulier
 *
 * Unsharp mask algorithm come from plug-ins/common/unsharp.c 
 * Gimp 2.0 source file and copyrighted 
 * 1999 by Winston Chang (winstonc at cs.wisc.edu)
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
#include <cstdio>
#include <cstdlib>

// Qt includes.

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qimage.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qstring.h>
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
#include "unsharp.h"

namespace DigikamUnsharpFilterImagesPlugin
{

UnsharpDialog::UnsharpDialog(QWidget* parent)
             : KDialogBase(Plain, i18n("Unsharp Mask"), Help|User1|Ok|Cancel, Ok,
                           parent, 0, true, true, i18n("&Reset Values")),
               m_parent(parent)
{
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    m_cancel = false;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Unsharp Mask"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("An unsharp mask image filter plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Winston Chang", I18N_NOOP("Unsharp mask algorithm author from Gimp"),
                     "winstonc at cs.wisc.edu");
                         
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Unsharp Mask Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Unsharp Mask"), headerFrame, "labelTitle" );
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
    
    m_radiusSlider = new QSlider(1, 1200, 1, 50, Qt::Horizontal, plainPage(), "m_radiusSlider");
    m_radiusSlider->setTickmarks(QSlider::Below);
    m_radiusSlider->setTickInterval(50);
    m_radiusSlider->setTracking ( false );
    
    m_radiusInput = new QSpinBox(1, 1200, 1, plainPage(), "m_radiusInput");
    m_radiusInput->setValue(50);
    
    whatsThis = i18n("<p>A radius of 0 has no effect, "
                     "10 and above determine the blur matrix radius "
                     "that determines how much to blur the image.");
    
    QWhatsThis::add( m_radiusInput, whatsThis);
    QWhatsThis::add( m_radiusSlider, whatsThis);

    hlay2->addWidget(label1, 1);
    hlay2->addWidget(m_radiusSlider, 3);
    hlay2->addWidget(m_radiusInput, 1);
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay3 = new QHBoxLayout(topLayout);
    QLabel *label2 = new QLabel(i18n("Amount:"), plainPage());
    
    m_amountSlider = new QSlider(0, 50, 1, 5, Qt::Horizontal, plainPage(), "m_amountSlider");
    m_amountSlider->setTickmarks(QSlider::Below);
    m_amountSlider->setTickInterval(5);
    m_amountSlider->setTracking ( false );
    
    m_amountInput = new QSpinBox(0, 50, 1, plainPage(), "m_amountInput");
    m_amountInput->setValue(5);
            
    whatsThis = i18n("<p>The value of the difference between the "
                     "original and the blur image that is added back "      
                     "into the original.");
    
    QWhatsThis::add( m_amountSlider, whatsThis);
    QWhatsThis::add( m_amountInput, whatsThis);
                 
    hlay3->addWidget(label2, 1);
    hlay3->addWidget(m_amountSlider, 3);
    hlay3->addWidget(m_amountInput, 1);

    // -------------------------------------------------------------

    QHBoxLayout *hlay4 = new QHBoxLayout(topLayout);
    QLabel *label3 = new QLabel(i18n("Threshold:"), plainPage());
    
    m_thresholdSlider = new QSlider(0, 255, 1, 0, Qt::Horizontal, plainPage(), "m_thresholdSlider");
    m_thresholdSlider->setTickmarks(QSlider::Below);
    m_thresholdSlider->setTickInterval(20);
    m_thresholdSlider->setTracking ( false );  
    
    m_thresholdInput = new QSpinBox(0, 255, 1, plainPage(), "m_thresholdInput");
    m_thresholdInput->setValue(0);
        
    whatsThis = i18n("<p>The threshold, as a fraction of the maximum luminosity value, "
                     "needed to apply the difference amount.");
    
    QWhatsThis::add( m_thresholdInput, whatsThis);
    QWhatsThis::add( m_thresholdSlider, whatsThis);                    
    
    hlay4->addWidget(label3, 1);
    hlay4->addWidget(m_thresholdSlider, 3);
    hlay4->addWidget(m_thresholdInput, 1);

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

    connect(m_amountSlider, SIGNAL(valueChanged(int)),
            m_amountInput, SLOT(setValue(int)));
    connect(m_amountInput, SIGNAL(valueChanged(int)),
            m_amountSlider, SLOT(setValue(int)));
    connect(m_amountInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));                                                            
            
    connect(m_thresholdSlider, SIGNAL(valueChanged(int)),
            m_thresholdInput, SLOT(setValue(int)));
    connect(m_thresholdInput, SIGNAL(valueChanged(int)),
            m_thresholdSlider, SLOT(setValue(int)));
    connect(m_thresholdInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));                                                
}

UnsharpDialog::~UnsharpDialog()
{
}

void UnsharpDialog::slotHelp()
{
    KApplication::kApplication()->invokeHelp("unsharp", "digikamimageplugins");
}

void UnsharpDialog::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void UnsharpDialog::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void UnsharpDialog::slotUser1()
{
    m_radiusInput->blockSignals(true);
    m_radiusSlider->blockSignals(true);
    m_amountInput->blockSignals(true);
    m_amountSlider->blockSignals(true);
    m_thresholdInput->blockSignals(true);
    m_thresholdSlider->blockSignals(true);
                
    m_radiusInput->setValue(50);
    m_radiusSlider->setValue(50);
    m_amountInput->setValue(5);
    m_amountSlider->setValue(5);
    m_thresholdInput->setValue(0);
    m_thresholdSlider->setValue(0);
                                     
    m_radiusInput->blockSignals(false);
    m_radiusSlider->blockSignals(false);
    m_amountInput->blockSignals(false);
    m_amountSlider->blockSignals(false);
    m_thresholdInput->blockSignals(false);
    m_thresholdSlider->blockSignals(false);
    slotEffect();
} 

void UnsharpDialog::slotEffect()
{
    m_radiusInput->setEnabled(false);
    m_radiusSlider->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_amountSlider->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    m_thresholdSlider->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    QImage img = m_imagePreviewWidget->getOriginalClipImage();
   
    uint*  data = (uint *)img.bits();
    int    w    = img.width();
    int    h    = img.height();
    int    r    = m_radiusSlider->value();
    int    a    = m_amountSlider->value();
    int    th   = m_thresholdSlider->value();
    
    m_imagePreviewWidget->setProgress(0);
    unsharp(data, w, h, r, a, th);   
    
    if (m_cancel) return;
    
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageData(img);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_radiusInput->setEnabled(true);
    m_radiusSlider->setEnabled(true);
    m_amountInput->setEnabled(true);
    m_amountSlider->setEnabled(true);
    m_thresholdInput->setEnabled(true);
    m_thresholdSlider->setEnabled(true);
    m_imagePreviewWidget->setEnable(true);   
}

void UnsharpDialog::slotOk()
{
    m_radiusInput->setEnabled(false);
    m_radiusSlider->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_amountSlider->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    m_thresholdSlider->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
    
    uint*  data = iface.getOriginalData();
    int    w     = iface.originalWidth();
    int    h     = iface.originalHeight();
    int    r     = m_radiusSlider->value();
    int    a     = m_amountSlider->value();
    int    th    = m_thresholdSlider->value();
    
    m_imagePreviewWidget->setProgress(0);       
    unsharp(data, w, h, r, a, th);   
           
    if ( !m_cancel )
       iface.putOriginalData(i18n("Unsharpen"), data);
    
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();
}

void UnsharpDialog::unsharp(uint* data, int w, int h, int r, int a, int threshold)
{
    double radius = r / 10.0;
    double amount = a / 10.0;
    int     bytes = 4;      // bpp in image.
    int     x1 = 0;         // Full image used.
    int     x2 = w;
    int     y1 = 0;
    int     y2 = h;
  
    uchar  *cur_col;
    uchar  *dest_col;
    uchar  *cur_row;
    uchar  *dest_row;
    int     x;
    int     y;
    double *cmatrix = NULL;
    int     cmatrix_length;
    double *ctable;

    uint* newData = new uint[w*h];
    
    // these are counters for loops 
    int row, col;  

    // these are used for the merging step 
    int diff;
    int value;
    int u,v;

    // find height and width of subregion to act on 
    x = x2-x1;
    y = y2-y1;

    // generate convolution matrix and make sure it's smaller than each dimension 
    cmatrix_length = gen_convolve_matrix(radius, &cmatrix);
  
    // generate lookup table 
    ctable = gen_lookup_table(cmatrix, cmatrix_length);

    //  allocate row buffers 
    cur_row  = new uchar[x * bytes];
    dest_row = new uchar[x * bytes];

    // find height and width of subregion to act on 
    x = x2-x1;
    y = y2-y1;

    // blank out a region of the destination memory area, I think 
    
    for (row = 0 ; !m_cancel && (row < y) ; row++)
      {
      memcpy(dest_row, newData + x1 + (y1+row)*w, (x2-x1)*bytes); 
      memset(dest_row, 0, x*bytes);
      memcpy(newData + x1 + (y1+row)*w, dest_row, (x2-x1)*bytes); 
      }

    // blur the rows 
    
    for (row = 0 ; !m_cancel && (row < y) ; row++)
      {
      memcpy(cur_row, data + x1 + (y1+row)*w, x*bytes); 
      memcpy(dest_row, newData + x1 + (y1+row)*w, x*bytes); 
      blur_line(ctable, cmatrix, cmatrix_length, cur_row, dest_row, x, bytes);
      memcpy(newData + x1 + (y1+row)*w, dest_row, x*bytes); 
      
      if (row%5 == 0)
          {
          m_imagePreviewWidget->setProgress((int)(100.0*((double)row/(3*y))));
          kapp->processEvents();
          }
      }

    // allocate column buffers 
    cur_col  = new uchar[y * bytes];
    dest_col = new uchar[y * bytes];

    // blur the cols 
  
    for (col = 0 ; !m_cancel && (col < x) ; col++)
      {
      for (int n = 0 ; n < y ; ++n)
          memcpy(cur_col + (n*bytes), newData + x1+col+w*(n+y1), bytes);
            
      for (int n = 0 ; n < y ; ++n)
          memcpy(dest_col + (n*bytes), newData + x1+col+w*(n+y1), bytes);
      
      blur_line(ctable, cmatrix, cmatrix_length, cur_col, dest_col, y, bytes);
      
      for (int n = 0 ; n < y ; ++n)
          memcpy(newData + x1+col+w*(n+y1), dest_col + (n*bytes), bytes);
          
      if (col%5 == 0)
         {
         m_imagePreviewWidget->setProgress((int)(100.0*((double)col/(3*x) + 0.33)));          
         kapp->processEvents();
         }
      }

    // merge the source and destination (which currently contains the blurred version) images 
  
    for (row = 0 ; !m_cancel && (row < y) ; row++)
      {
      value = 0;
      // get source row 
      memcpy(cur_row, data + x1 + (y1+row)*w, x*bytes); 
      
      // get dest row 

      memcpy(dest_row, newData + x1 + (y1+row)*w, x*bytes); 
      
      // combine the two 
      
      for (u = 0; u < x; u++)
         {
         for (v = 0; v < bytes; v++)
            {
            diff = (cur_row[u*bytes+v] - dest_row[u*bytes+v]);
            // do tresholding 
          
            if (abs (2 * diff) < threshold)
               diff = 0;

            value = (int)(cur_row[u*bytes+v] + amount * diff);

            if (value < 0) dest_row[u*bytes+v] = 0;
            else if (value > 255) dest_row[u*bytes+v] = 255;
            else  dest_row[u*bytes+v] = value;
            }
         }
      
      // update progress bar every five rows 
      
      if (row%5 == 0)
         {
         m_imagePreviewWidget->setProgress((int)(100.0*((double)row/(3*y) + 0.67)));
         kapp->processEvents();
         }
         
      memcpy(newData + x1 + (y1+row)*w, dest_row, x*bytes); 
      }

    memcpy(data, newData, w*h*bytes);  
    
    // free the memory we took 
    delete [] cur_row;
    delete [] dest_row;
    delete [] cur_col;
    delete [] dest_col;
    delete [] cmatrix;
    delete [] ctable;
    delete [] newData;
}

/*
 this function is written as if it is blurring a column at a time,
 even though it can operate on rows, too.  There is no difference
 in the processing of the lines, at least to the blur_line function. 
 */
   
void UnsharpDialog::blur_line (double *ctable, double *cmatrix, int cmatrix_length,
                               uchar *cur_col, uchar *dest_col, int y, long bytes)
{
    double scale;
    double sum;
    int i=0, j=0;
    int row;
    int cmatrix_middle = cmatrix_length/2;

    double *cmatrix_p;
    uchar  *cur_col_p;
    uchar  *cur_col_p1;
    uchar  *dest_col_p;
    double *ctable_p;

    // this first block is the same as the non-optimized version --
    // it is only used for very small pictures, so speed isn't a
    // big concern.

    if (cmatrix_length > y)
       {
       for (row = 0; row < y ; row++)
          {
          scale=0;
      
          // find the scale factor 
          
          for (j = 0; j < y ; j++)
             {
             // if the index is in bounds, add it to the scale counter 
       
             if ((j + cmatrix_length/2 - row >= 0) && (j + cmatrix_length/2 - row < cmatrix_length))
                scale += cmatrix[j + cmatrix_length/2 - row];
             }
      
          for (i = 0; i<bytes; i++)
             {
             sum = 0;
       
             for (j = 0; j < y; j++)
                {
                if ((j >= row - cmatrix_length/2) && (j <= row + cmatrix_length/2))
                   sum += cur_col[j*bytes + i] * cmatrix[j];
                }
       
             dest_col[row*bytes + i] = (uchar) ROUND (sum / scale);
             }
          }
       }
    else
      {
      // for the edge condition, we only use available info and scale to one 
      
      for (row = 0; row < cmatrix_middle; row++)
         {
         // find scale factor 
         scale=0;
      
         for (j = cmatrix_middle - row; j<cmatrix_length; j++)
            scale += cmatrix[j];
         
         for (i = 0; i<bytes; i++)
            {
            sum = 0;
            
            for (j = cmatrix_middle - row; j<cmatrix_length; j++)
               {
               sum += cur_col[(row + j-cmatrix_middle)*bytes + i] * cmatrix[j];
               }
            
            dest_col[row*bytes + i] = (uchar) ROUND (sum / scale);
            }
         }
         
         // go through each pixel in each col 
         dest_col_p = dest_col + row*bytes;
         
         for (; row < y-cmatrix_middle; row++)
            {
            cur_col_p = (row - cmatrix_middle) * bytes + cur_col;
            
            for (i = 0; i<bytes; i++)
               {
               sum = 0;
               cmatrix_p = cmatrix;
               cur_col_p1 = cur_col_p;
               ctable_p = ctable;
               
               for (j = cmatrix_length; j>0; j--)
                  {
                  sum += *(ctable_p + *cur_col_p1);
                  cur_col_p1 += bytes;
                  ctable_p += 256;
                  }
          
               cur_col_p++;
               *(dest_col_p++) = ROUND (sum);
               }
            }

      // for the edge condition , we only use available info, and scale to one 
      
      for (; row < y; row++)
         {
         // find scale factor 
         scale=0;
      
         for (j = 0; j< y-row + cmatrix_middle; j++)
             scale += cmatrix[j];
         
         for (i = 0; i<bytes; i++)
            {
            sum = 0;
            
            for (j = 0; j<y-row + cmatrix_middle; j++)
               {
               sum += cur_col[(row + j-cmatrix_middle)*bytes + i] * cmatrix[j];
               }
          
            dest_col[row*bytes + i] = (uchar) ROUND (sum / scale);
            }
         }
      }
}

/*
 generates a 1-D convolution matrix to be used for each pass of
 a two-pass gaussian blur.  Returns the length of the matrix.
 */
 
int UnsharpDialog::gen_convolve_matrix (double radius, double **cmatrix_p)
{
    int     matrix_length;
    int     matrix_midpoint;
    double* cmatrix;
    int     i,j;
    double  std_dev;
    double  sum;

    // we want to generate a matrix that goes out a certain radius
    // from the center, so we have to go out ceil(rad-0.5) pixels,
    // inlcuding the center pixel.  Of course, that's only in one direction,
    // so we have to go the same amount in the other direction, but not count
    // the center pixel again.  So we double the previous result and subtract
    // one.
    // The radius parameter that is passed to this function is used as
    // the standard deviation, and the radius of effect is the
    // standard deviation * 2.  It's a little confusing.
 
    radius = fabs(radius) + 1.0;

    std_dev = radius;
    radius = std_dev * 2;

    // Go out 'radius' in each direction 
    matrix_length = (int)(2 * ceil(radius-0.5) + 1);
  
    if (matrix_length <= 0) matrix_length = 1;
  
    matrix_midpoint = matrix_length/2 + 1;
    *cmatrix_p = new double[matrix_length];
    cmatrix = *cmatrix_p;

    // Now we fill the matrix by doing a numeric integration approximation
    // from -2*std_dev to 2*std_dev, sampling 50 points per pixel.
    // We do the bottom half, mirror it to the top half, then compute the
    // center point.  Otherwise asymmetric quantization errors will occur.
    // The formula to integrate is e^-(x^2/2s^2).
  
    // first we do the top (right) half of matrix 
  
    for (i = matrix_length/2 + 1; i < matrix_length; i++)
      {
      double base_x = i - floor(matrix_length/2) - 0.5;
      sum = 0;
      
      for (j = 1; j <= 50; j++)
          {
          if ( base_x+0.02*j <= radius )
             sum += exp (-(base_x+0.02*j)*(base_x+0.02*j) / (2*std_dev*std_dev));
          }
      
      cmatrix[i] = sum/50;
      }

    // mirror the thing to the bottom half 
    
    for (i=0; i<=matrix_length/2; i++) 
       {
       cmatrix[i] = cmatrix[matrix_length-1-i];
       }

    // find center val -- calculate an odd number of quanta to make it symmetric,
    // * even if the center point is weighted slightly higher than others. 
    sum = 0;
  
    for (j=0; j<=50; j++)
       {
       sum += exp (-(0.5+0.02*j)*(0.5+0.02*j) / (2*std_dev*std_dev));
       }
  
    cmatrix[matrix_length/2] = sum/51;

    // normalize the distribution by scaling the total sum to one 
    sum=0;
    for (i=0; i<matrix_length; i++) sum += cmatrix[i];
    for (i=0; i<matrix_length; i++) cmatrix[i] = cmatrix[i] / sum;

    return matrix_length;
}


/*
 Generates a lookup table for every possible product of 0-255 and
 each value in the convolution matrix.  The returned array is
 indexed first by matrix position, then by input multiplicand (?)
 value.
 */
double* UnsharpDialog::gen_lookup_table (double *cmatrix, int cmatrix_length)
{
    int i, j;
    double* lookup_table   = new double[cmatrix_length * 256];
    double* lookup_table_p = lookup_table;
    double* cmatrix_p      = cmatrix;

    for (i=0 ; i<cmatrix_length ; i++)
      {
      for (j=0 ; j<256 ; j++)
         {
         *(lookup_table_p++) = *cmatrix_p * (double)j;
         }
         
      cmatrix_p++;
      }

    return lookup_table;
}

}  // NameSpace DigikamUnsharpFilterImagesPlugin

#include "unsharp.moc"
