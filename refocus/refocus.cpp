/* ============================================================
 * File  : refocus.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-05-25
 * Description : Refocus threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
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

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "matrix.h"
#include "refocus.h"

namespace DigikamRefocusImagesPlugin
{

Refocus::Refocus(Digikam::DImg *orgImage, QObject *parent, int matrixSize, double radius, 
                 double gauss, double correlation, double noise)
       : Digikam::DImgThreadedFilter(orgImage, parent, "Refocus")
{ 
    m_matrixSize  = matrixSize;
    m_radius      = radius;
    m_gauss       = gauss;
    m_correlation = correlation;
    m_noise       = noise;
    initFilter();
}

void Refocus::filterImage(void)
{
    refocusImage(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(),
                 m_orgImage.sixteenBit(), m_matrixSize, m_radius, m_gauss, 
                 m_correlation, m_noise);
}

void Refocus::refocusImage(uchar* data, int width, int height, bool sixteenBit,
                           int matrixSize, double radius, double gauss,
                           double correlation, double noise)
{
    CMat *matrix=0;
    
    // Compute matrix
    kdDebug() << "Refocus::Compute matrix..." << endl;

    CMat circle, gaussian, convolution;
    
    RefocusMatrix::make_gaussian_convolution (gauss, &gaussian, matrixSize);
    RefocusMatrix::make_circle_convolution (radius, &circle, matrixSize);
    RefocusMatrix::init_c_mat (&convolution, matrixSize);
    RefocusMatrix::convolve_star_mat (&convolution, &gaussian, &circle);

    matrix = RefocusMatrix::compute_g_matrix (&convolution, matrixSize, correlation, noise, 0.0, true);
    
    RefocusMatrix::finish_c_mat (&convolution);
    RefocusMatrix::finish_c_mat (&gaussian);
    RefocusMatrix::finish_c_mat (&circle);

    // Apply deconvolution kernel to image.
    kdDebug() << "Refocus::Apply Matrix to image..." << endl;
    convolveImage(data, m_destImage.bits(), width, height, sixteenBit,
                  matrix->data, 2 * matrixSize + 1);
    
    // Clean up memory
    delete matrix;   
}

void Refocus::convolveImage(uchar *orgData, uchar *destData, int width, int height,
                            bool sixteenBit, const double *const mat, int mat_size)
{
    int progress;
    unsigned short *orgData16  = (unsigned short *)orgData;
    unsigned short *destData16 = (unsigned short *)destData;
    
    double matrix[mat_size][mat_size];
    double valRed, valGreen, valBlue;
    int    x1, y1, x2, y2, index1, index2;
    
    const int imageSize  = width*height;
    const int mat_offset = mat_size / 2;
    
    memcpy (&matrix, mat, mat_size* mat_size * sizeof(double));
    
    for (y1 = 0; !m_cancel && (y1 < height); y1++)
    {
        for (x1 = 0; !m_cancel && (x1 < width); x1++)
        {
            valRed = valGreen = valBlue = 0.0;

            if (!sixteenBit)        // 8 bits image.
            {
                uchar red, green, blue;
                uchar *ptr;  

                for (y2 = 0; !m_cancel && (y2 < mat_size); y2++)
                {
                    for (x2 = 0; !m_cancel && (x2 < mat_size); x2++)
                    {
                        index1 = width * (y1 + y2 - mat_offset) +
                                 x1 + x2 - mat_offset;
                            
                        if ( index1 >= 0 && index1 < imageSize )
                        {
                            ptr   = &orgData[index1*4];
                            blue  = ptr[0];
                            green = ptr[1];
                            red   = ptr[2];
                            valRed   += matrix[y2][x2] * red;
                            valGreen += matrix[y2][x2] * green;
                            valBlue  += matrix[y2][x2] * blue;
                        }
                    }
                }
                
                index2 = y1 * width + x1;
                            
                if (index2 >= 0 && index2 < imageSize)
                {
                    // To get Alpha channel value from original (unchanged)
                    memcpy (&destData[index2*4], &orgData[index2*4], 4);                    
                    ptr = &destData[index2*4];
                    
                    // Overwrite RGB values to destination.
                    ptr[0] = (uchar) CLAMP (valBlue,  0, 255);
                    ptr[1] = (uchar) CLAMP (valGreen, 0, 255);
                    ptr[2] = (uchar) CLAMP (valRed,   0, 255);
                }
            }
            else                 // 16 bits image.
            {
                unsigned short red, green, blue;
                unsigned short *ptr;  

                for (y2 = 0; !m_cancel && (y2 < mat_size); y2++)
                {
                    for (x2 = 0; !m_cancel && (x2 < mat_size); x2++)
                    {
                        index1 = width * (y1 + y2 - mat_offset) + 
                                 x1 + x2 - mat_offset;
                            
                        if ( index1 >= 0 && index1 < imageSize )
                        {
                            ptr   = &orgData16[index1*4];
                            blue  = ptr[0];
                            green = ptr[1];
                            red   = ptr[2];
                            valRed   += matrix[y2][x2] * red;
                            valGreen += matrix[y2][x2] * green;
                            valBlue  += matrix[y2][x2] * blue;
                        }
                    }
                }
                
                index2 = y1 * width + x1;
                            
                if (index2 >= 0 && index2 < imageSize)
                {
                    // To get Alpha channel value from original (unchanged)
                    memcpy (&destData16[index2*4], &orgData16[index2*4], 8);                    
                    ptr = &destData16[index2*4];
                    
                    // Overwrite RGB values to destination.
                    ptr[0] = (unsigned short) CLAMP (valBlue,  0, 65535);
                    ptr[1] = (unsigned short) CLAMP (valGreen, 0, 65535);
                    ptr[2] = (unsigned short) CLAMP (valRed,   0, 65535);
                }
            }
        }
            
        // Update the progress bar in dialog.
        progress = (int)(((double)y1 * 100.0) / height);
        if (progress%5 == 0)
            postProgress( progress );        
        }
}

}  // NameSpace DigikamRefocusImagesPlugin

