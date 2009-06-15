/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Refocus threaded image filter.
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Matthias Welwarsky <matze at welwarsky dot de>
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

#include "dimgrefocus.h"

// C++ includes

#include <cmath>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "dcolor.h"
#include "dimgimagefilters.h"
#include "matrix.h"

#define MAX_MATRIX_SIZE 25

namespace Digikam
{

int DImgRefocus::maxMatrixSize()
{
    return MAX_MATRIX_SIZE;
}

DImgRefocus::DImgRefocus(DImg *orgImage, QObject *parent, int matrixSize, double radius,
                         double gauss, double correlation, double noise)
           : DImgThreadedFilter(orgImage, parent, "Refocus")
{
    m_matrixSize  = matrixSize;
    m_radius      = radius;
    m_gauss       = gauss;
    m_correlation = correlation;
    m_noise       = noise;
    
    // initialize filter
    initFilter();

    // initialize intermediate image
    m_preImage = DImg(orgImage->width()+4*MAX_MATRIX_SIZE, 
            orgImage->height()+4*MAX_MATRIX_SIZE, 
            orgImage->sixteenBit(), orgImage->hasAlpha());    
}

void DImgRefocus::filterImage(void)
{
    bool sb = m_orgImage.sixteenBit();
    bool a  = m_orgImage.hasAlpha();
    int w = m_orgImage.width();
    int h = m_orgImage.height();
    
    DImg img(w + 4*MAX_MATRIX_SIZE, h + 4*MAX_MATRIX_SIZE, sb, a);
    DImg tmp;

    // copy the original
    img.bitBltImage(&m_orgImage, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    
    // Create dummy top border
    tmp = m_orgImage.copy(0, 0, w, 2*MAX_MATRIX_SIZE);
    tmp.flip(DImg::VERTICAL);
    img.bitBltImage(&tmp, 2*MAX_MATRIX_SIZE, 0);

    // Create dummy bottom border
    tmp = m_orgImage.copy(0, h-2*MAX_MATRIX_SIZE, w, 2*MAX_MATRIX_SIZE);
    tmp.flip(DImg::VERTICAL);
    img.bitBltImage(&tmp, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE+h);

    // Create dummy left border
    tmp = m_orgImage.copy(0, 0, 2*MAX_MATRIX_SIZE, h);
    tmp.flip(DImg::HORIZONTAL);
    img.bitBltImage(&tmp, 0, 2*MAX_MATRIX_SIZE);

    // Create dummy right border
    tmp = m_orgImage.copy(w-2*MAX_MATRIX_SIZE, 0, 2*MAX_MATRIX_SIZE, h);
    tmp.flip(DImg::HORIZONTAL);
    img.bitBltImage(&tmp, w+2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);

    // Create dummy top/left corner
    tmp = m_orgImage.copy(0, 0, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(DImg::HORIZONTAL);
    tmp.flip(DImg::VERTICAL);
    img.bitBltImage(&tmp, 0, 0);

    // Create dummy top/right corner
    tmp = m_orgImage.copy(w-2*MAX_MATRIX_SIZE, 0, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(DImg::HORIZONTAL);
    tmp.flip(DImg::VERTICAL);
    img.bitBltImage(&tmp, w+2*MAX_MATRIX_SIZE, 0);

    // Create dummy bottom/left corner
    tmp = m_orgImage.copy(0, h-2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(DImg::HORIZONTAL);
    tmp.flip(DImg::VERTICAL);
    img.bitBltImage(&tmp, 0, h+2*MAX_MATRIX_SIZE);

    // Create dummy bottom/right corner
    tmp = m_orgImage.copy(w-2*MAX_MATRIX_SIZE, h-2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(DImg::HORIZONTAL);
    tmp.flip(DImg::VERTICAL);
    img.bitBltImage(&tmp, w+2*MAX_MATRIX_SIZE, h+2*MAX_MATRIX_SIZE);

    // run filter algorithm on the prepared copy
    refocusImage(img.bits(), img.width(), img.height(),
                 img.sixteenBit(), m_matrixSize, m_radius, m_gauss,
                 m_correlation, m_noise);
    
    // copy the result from intermediate image to final image
    m_destImage.bitBltImage(&m_preImage, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE, w, h, 0, 0);
}

void DImgRefocus::refocusImage(uchar* data, int width, int height, bool sixteenBit,
                               int matrixSize, double radius, double gauss,
                               double correlation, double noise)
{
    CMat *matrix=0;

    // Compute matrix
    kDebug(50006) << "DImgRefocus::Compute matrix...";

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
    kDebug(50006) << "DImgRefocus::Apply Matrix to image...";
    convolveImage(data, m_preImage.bits(), width, height, sixteenBit,
                  matrix->data, 2 * matrixSize + 1);

    // Clean up memory
    delete matrix;
}

void DImgRefocus::convolveImage(uchar *orgData, uchar *destData, int width, int height,
                                bool sixteenBit, const double *const matrix, int mat_size)
{
    int progress;
    unsigned short *orgData16  = (unsigned short *)orgData;
    unsigned short *destData16 = (unsigned short *)destData;

    double valRed, valGreen, valBlue;
    int    x1, y1, x2, y2, index1, index2;

    const int imageSize  = width*height;
    const int mat_offset = mat_size / 2;

    for (y1 = 0; !m_cancel && (y1 < height); ++y1)
    {
        for (x1 = 0; !m_cancel && (x1 < width); ++x1)
        {
            valRed = valGreen = valBlue = 0.0;

            if (!sixteenBit)        // 8 bits image.
            {
                uchar red, green, blue;
                uchar *ptr;

                for (y2 = 0; !m_cancel && (y2 < mat_size); ++y2)
                {
                    for (x2 = 0; !m_cancel && (x2 < mat_size); ++x2)
                    {
                        index1 = width * (y1 + y2 - mat_offset) +
                                 x1 + x2 - mat_offset;

                        if ( index1 >= 0 && index1 < imageSize )
                        {
                            ptr   = &orgData[index1*4];
                            blue  = ptr[0];
                            green = ptr[1];
                            red   = ptr[2];
                            const double matrixValue = matrix[y2 * mat_size + x2];
                            valRed   += matrixValue * red;
                            valGreen += matrixValue * green;
                            valBlue  += matrixValue * blue;
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

                for (y2 = 0; !m_cancel && (y2 < mat_size); ++y2)
                {
                    for (x2 = 0; !m_cancel && (x2 < mat_size); ++x2)
                    {
                        index1 = width * (y1 + y2 - mat_offset) +
                                 x1 + x2 - mat_offset;

                        if ( index1 >= 0 && index1 < imageSize )
                        {
                            ptr   = &orgData16[index1*4];
                            blue  = ptr[0];
                            green = ptr[1];
                            red   = ptr[2];
                            const double matrixValue = matrix[y2 * mat_size + x2];
                            valRed   += matrixValue * red;
                            valGreen += matrixValue * green;
                            valBlue  += matrixValue * blue;
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

}  // namespace DigikamImagesPluginCore
