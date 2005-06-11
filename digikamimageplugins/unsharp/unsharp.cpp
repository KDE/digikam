/* ============================================================
 * File  : unsharp.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Unsharp Mask threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Unsharp Mask algorithm come from plug-ins/common/unsharp.c 
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
  
// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// Local includes.

#include "unsharp.h"

namespace DigikamUnsharpMaskImagesPlugin
{

UnsharpMask::UnsharpMask(QImage *orgImage, QObject *parent, double radius, 
                         double amount, int threshold)
           : Digikam::ThreadedFilter(orgImage, parent, "UnsharpMask")
{ 
    m_radius    = radius;
    m_amount    = amount;
    m_threshold = threshold;
    initFilter();
}

void UnsharpMask::filterImage(void)
{
    unsharpImage((uint*)m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), 
                 m_radius, m_amount, m_threshold);
}

void UnsharpMask::unsharpImage(uint* data, int w, int h, double radius, double amount, int threshold)
{
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

    uint* newData = (uint*)m_destImage.bits();
    
    // these are counters for loops 
    int row, col;  

    // these are used for the merging step 
    int diff;
    int u, v;

    // find height and width of subregion to act on 
    x = x2 - x1;
    y = y2 - y1;

    // generate convolution matrix and make sure it's smaller than each dimension 
    cmatrix_length = gen_convolve_matrix(radius, &cmatrix);
  
    // generate lookup table 
    ctable = gen_lookup_table(cmatrix, cmatrix_length);

    // allocate row buffers 
    cur_row  = new uchar[x * bytes];
    dest_row = new uchar[x * bytes];

    // find height and width of subregion to act on 
    x = x2 - x1;
    y = y2 - y1;

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

      // update progress bar in dialog every five columns.
      
      if (row%5 == 0)
         postProgress( (int)(100.0*((double)row/(3*y))) );
      }

    // allocate column buffers 
    cur_col  = new uchar[y * bytes];
    dest_col = new uchar[y * bytes];

    // blur the cols 
  
    for (col = 0 ; !m_cancel && (col < x) ; col++)
      {
      for (int n = 0 ; n < y ; n++)
          memcpy(cur_col + (n*bytes), newData + x1+col+w*(n+y1), bytes);
            
      for (int n = 0 ; n < y ; n++)
          memcpy(dest_col + (n*bytes), newData + x1+col+w*(n+y1), bytes);
      
      blur_line(ctable, cmatrix, cmatrix_length, cur_col, dest_col, y, bytes);
      
      for (int n = 0 ; n < y ; n++)
          memcpy(newData + x1+col+w*(n+y1), dest_col + (n*bytes), bytes);
          
      // update progress bar in dialog every five columns.
      
      if (col%5 == 0)
         postProgress( (int)(100.0*((double)col/(3*x) + 0.33)) );
      }

    // merge the source and destination (which currently contains the blurred version) images 
  
    for (row = 0 ; !m_cancel && (row < y) ; row++)
      {
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
            if (abs (2 * diff) < threshold) diff = 0;

            dest_row[u*bytes+v] = (uchar) CLAMP (cur_row[u*bytes+v] + amount * diff, 0, 255);
            }
         }
      
      // update progress bar in dialog every five rows.
      
      if (row%5 == 0)
         postProgress( (int)(100.0*((double)row/(3*y) + 0.67)) );      
         
      memcpy(newData + x1 + (y1+row)*w, dest_row, x*bytes); 
      }

    // free the memory we took 
    delete [] cur_row;
    delete [] dest_row;
    delete [] cur_col;
    delete [] dest_col;
    delete [] cmatrix;
    delete [] ctable;
}

/*
 this function is written as if it is blurring a column at a time,
 even though it can operate on rows, too.  There is no difference
 in the processing of the lines, at least to the blur_line function. 
 */
   
void UnsharpMask::blur_line (double *ctable, double *cmatrix, int cmatrix_length,
                             uchar *cur_col, uchar *dest_col, int y, long bytes)
{
    double  scale;
    double  sum;
    int     i=0, j=0;
    int     row;
    int     cmatrix_middle = cmatrix_length/2;

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
 
int UnsharpMask::gen_convolve_matrix (double radius, double **cmatrix_p)
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
 
    radius  = fabs(radius) + 1.0;

    std_dev = radius;
    radius  = std_dev * 2;

    // Go out 'radius' in each direction 
    matrix_length = (int)(2 * ceil(radius-0.5) + 1);
  
    if (matrix_length <= 0) matrix_length = 1;
  
    matrix_midpoint = matrix_length/2 + 1;
    *cmatrix_p      = new double[matrix_length];
    cmatrix         = *cmatrix_p;

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
double* UnsharpMask::gen_lookup_table (double *cmatrix, int cmatrix_length)
{
    int     i, j;
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

}  // NameSpace DigikamUnsharpMaskImagesPlugin
