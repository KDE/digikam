/* ============================================================
 * File  : despeckle.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Despeckle threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
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
 
// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// Local includes.

#include "despeckle.h"

namespace DigikamNoiseReductionImagesPlugin
{

Despeckle::Despeckle(QImage *orgImage, QObject *parent, int radius, int black_level, int white_level, 
                     bool adaptativeFilter, bool recursiveFilter)
         : Digikam::ThreadedFilter(orgImage, parent, "Despeckle")
{ 
    m_radius           = radius;
    m_black_level      = black_level;
    m_white_level      = white_level;
    m_adaptativeFilter = adaptativeFilter;
    m_recursiveFilter  = recursiveFilter;
    initFilter();
}

void Despeckle::filterImage(void)
{
    despeckleImage((uint*)m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), 
                   m_radius, m_black_level, m_white_level, 
                   m_adaptativeFilter, m_recursiveFilter);
}

void Despeckle::despeckleImage(uint* data, int w, int h, int despeckle_radius, 
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
                 
     int         sel_width  = w; // Selection width 
     int         sel_height = h; // Selection height                  
     int         img_bpp    = 4; // Bytes-per-pixel in image

     QImage      image, region;                 
     
     uint* newData = (uint*)m_destImage.bits();                 
                 
     // Setup for filter...

     image.create( w, h, 32 );
     memcpy(image.bits(), data, image.numBytes());

     size        = despeckle_radius * 2 + 1;
     max_row     = 2 * TILE_HEIGHT;
     width       = w * img_bpp;
     src_rows    = new uchar*[max_row];
     src_rows[0] = new uchar[max_row * width];
     dst_row     = new uchar[width],
     sort        = new uchar[size * size];

     for (row = 1 ; row < max_row ; row ++)
         src_rows[row] = src_rows[0] + row * width;

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
 
     for (y = sel_y1 ; !m_cancel && (y < sel_y2) ; y++)
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
           }

        // Now find the median pixels and save the results...
      
        radius = despeckle_radius;

        memcpy (dst_row, src_rows[(row + y - lasty + max_row) % max_row], width);

        if (y >= sel_y1 && y < sel_y2)
           {
           for (x = 0 ; !m_cancel && (x < width) ; x++)
              {
              hist0   = 0;
              hist255 = 0;
              xmin    = x - radius * img_bpp;
              xmax    = x + (radius + 1) * img_bpp;

              if (xmin < 0) xmin = x % img_bpp;

              if (xmax > width) xmax = width;

              startrow = (row + y - lasty - radius + max_row) % max_row;
              endrow   = (row + y - lasty + radius + 1 + max_row) % max_row;

              for (sort_ptr = sort, trow = startrow ;
                   trow != endrow ; trow = (trow + 1) % max_row)
                 {
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
                    }
                 }

              // Shell sort the color values...
           
              sort_count = sort_ptr - sort;

              if (sort_count > 1)
                 {
                 for (d = sort_count / 2; d > 0; d = d / 2)
                    {
                    for (i = d; i < sort_count; i++)
                       {
                       for (j = i - d, sort_ptr = sort + j;
                            j >= 0 && sort_ptr[0] > sort_ptr[d];
                            j -= d, sort_ptr -= d)
                          {
                          t           = sort_ptr[0];
                          sort_ptr[0] = sort_ptr[d];
                          sort_ptr[d] = t;
                          }
                       }
                    }

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
                 }

              // Check the histogram and adjust the radius in according...

              if (adaptativeFilter)
                 {
                 if (hist0 >= radius || hist255 >= radius)
                    {
                    if (radius < despeckle_radius)
                       radius++;
                    }
                 else if (radius > 1)
                    radius--;
                 }
              }
           }
         
        memcpy (newData + (w * y), dst_row, width);
        
        if ( y%5 == 0)
           postProgress( (int)(100.0*(double) (y - sel_y1) / (double) sel_height) );   
        }

     // OK, we're done.  Free all memory used...
   
     delete [] src_rows; 
     delete [] dst_row;
     delete [] sort;
}

}  // NameSpace DigikamNoiseReductionImagesPlugin
