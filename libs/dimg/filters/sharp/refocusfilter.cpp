/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Refocus threaded image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Matthias Welwarsky <matze at welwarsky dot de>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "refocusfilter.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QtConcurrent>

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "dcolor.h"
#include "matrix.h"

namespace Digikam
{

const int MAX_MATRIX_SIZE = 25;

class RefocusFilter::Private
{
public:

    Private() :
        matrixSize(5),
        radius(0.9),
        gauss(0.0),
        correlation(0.5),
        noise(0.01)
    {
    }

    DImg      preImage;

    int       matrixSize;

    double    radius;
    double    gauss;
    double    correlation;
    double    noise;
};

RefocusFilter::RefocusFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

RefocusFilter::RefocusFilter(DImg* const orgImage, QObject* const parent, int matrixSize, double radius,
                             double gauss, double correlation, double noise)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("Refocus")),
      d(new Private)
{
    d->matrixSize  = matrixSize;
    d->radius      = radius;
    d->gauss       = gauss;
    d->correlation = correlation;
    d->noise       = noise;

    // initialize filter
    initFilter();

    // initialize intermediate image
    d->preImage = DImg(orgImage->width()  + 4 * MAX_MATRIX_SIZE,
                       orgImage->height() + 4 * MAX_MATRIX_SIZE,
                       orgImage->sixteenBit(), orgImage->hasAlpha());
}

RefocusFilter::~RefocusFilter()
{
    cancelFilter();
    delete d;
}

void RefocusFilter::filterImage()
{
    bool sb = m_orgImage.sixteenBit();
    bool a  = m_orgImage.hasAlpha();
    int w   = m_orgImage.width();
    int h   = m_orgImage.height();

    DImg img(w + 4 * MAX_MATRIX_SIZE, h + 4 * MAX_MATRIX_SIZE, sb, a);
    DImg tmp;

    // copy the original
    img.bitBltImage(&m_orgImage, 2 * MAX_MATRIX_SIZE, 2 * MAX_MATRIX_SIZE);

    // Create dummy top border
    tmp = m_orgImage.copy(0, 0, w, 2 * MAX_MATRIX_SIZE);
    tmp.flip(DImg::VERTICAL);
    img.bitBltImage(&tmp, 2 * MAX_MATRIX_SIZE, 0);

    // Create dummy bottom border
    tmp = m_orgImage.copy(0, h - 2 * MAX_MATRIX_SIZE, w, 2 * MAX_MATRIX_SIZE);
    tmp.flip(DImg::VERTICAL);
    img.bitBltImage(&tmp, 2 * MAX_MATRIX_SIZE, 2 * MAX_MATRIX_SIZE + h);

    // Create dummy left border
    tmp = m_orgImage.copy(0, 0, 2 * MAX_MATRIX_SIZE, h);
    tmp.flip(DImg::HORIZONTAL);
    img.bitBltImage(&tmp, 0, 2 * MAX_MATRIX_SIZE);

    // Create dummy right border
    tmp = m_orgImage.copy(w - 2 * MAX_MATRIX_SIZE, 0, 2 * MAX_MATRIX_SIZE, h);
    tmp.flip(DImg::HORIZONTAL);
    img.bitBltImage(&tmp, w + 2 * MAX_MATRIX_SIZE, 2 * MAX_MATRIX_SIZE);

    // Create dummy top/left corner
    tmp = m_orgImage.copy(0, 0, 2 * MAX_MATRIX_SIZE, 2 * MAX_MATRIX_SIZE);
    tmp.flip(DImg::HORIZONTAL);
    tmp.flip(DImg::VERTICAL);
    img.bitBltImage(&tmp, 0, 0);

    // Create dummy top/right corner
    tmp = m_orgImage.copy(w - 2 * MAX_MATRIX_SIZE, 0, 2 * MAX_MATRIX_SIZE, 2 * MAX_MATRIX_SIZE);
    tmp.flip(DImg::HORIZONTAL);
    tmp.flip(DImg::VERTICAL);
    img.bitBltImage(&tmp, w + 2 * MAX_MATRIX_SIZE, 0);

    // Create dummy bottom/left corner
    tmp = m_orgImage.copy(0, h - 2 * MAX_MATRIX_SIZE, 2 * MAX_MATRIX_SIZE, 2 * MAX_MATRIX_SIZE);
    tmp.flip(DImg::HORIZONTAL);
    tmp.flip(DImg::VERTICAL);
    img.bitBltImage(&tmp, 0, h + 2 * MAX_MATRIX_SIZE);

    // Create dummy bottom/right corner
    tmp = m_orgImage.copy(w - 2 * MAX_MATRIX_SIZE, h - 2 * MAX_MATRIX_SIZE, 2 * MAX_MATRIX_SIZE, 2 * MAX_MATRIX_SIZE);
    tmp.flip(DImg::HORIZONTAL);
    tmp.flip(DImg::VERTICAL);
    img.bitBltImage(&tmp, w + 2 * MAX_MATRIX_SIZE, h + 2 * MAX_MATRIX_SIZE);

    // run filter algorithm on the prepared copy
    refocusImage(img.bits(), img.width(), img.height(),
                 img.sixteenBit(), d->matrixSize, d->radius, d->gauss,
                 d->correlation, d->noise);

    // copy the result from intermediate image to final image
    m_destImage.bitBltImage(&d->preImage, 2 * MAX_MATRIX_SIZE, 2 * MAX_MATRIX_SIZE, w, h, 0, 0);
}

void RefocusFilter::refocusImage(uchar* const data, int width, int height, bool sixteenBit,
                                 int matrixSize, double radius, double gauss,
                                 double correlation, double noise)
{
    CMat* matrix = 0;

    // Compute matrix
    qCDebug(DIGIKAM_DIMG_LOG) << "RefocusFilter::Compute matrix...";

    CMat circle, gaussian, convolution;

    RefocusMatrix::make_gaussian_convolution(gauss, &gaussian, matrixSize);
    RefocusMatrix::make_circle_convolution(radius, &circle, matrixSize);
    RefocusMatrix::init_c_mat(&convolution, matrixSize);
    RefocusMatrix::convolve_star_mat(&convolution, &gaussian, &circle);

    matrix = RefocusMatrix::compute_g_matrix(&convolution, matrixSize, correlation, noise, 0.0, true);

    RefocusMatrix::finish_c_mat(&convolution);
    RefocusMatrix::finish_c_mat(&gaussian);
    RefocusMatrix::finish_c_mat(&circle);

    // Apply deconvolution kernel to image.
    qCDebug(DIGIKAM_DIMG_LOG) << "RefocusFilter::Apply Matrix to image...";

    Args prm;
    prm.orgData    = data;
    prm.destData   = d->preImage.bits();
    prm.width      = width;
    prm.height     = height;
    prm.sixteenBit = sixteenBit;
    prm.matrix     = matrix->data;
    prm.mat_size   = 2 * matrixSize + 1;

    convolveImage(prm);

    // Clean up memory
    delete matrix;
}

void RefocusFilter::convolveImageMultithreaded(uint start, uint stop, uint y1, const Args& prm)
{
    ushort* orgData16  = reinterpret_cast<ushort*>(prm.orgData);
    ushort* destData16 = reinterpret_cast<ushort*>(prm.destData);

    double valRed, valGreen, valBlue;
    uint   x1, x2, y2;
    int    index1, index2;

    const int imageSize  = prm.width * prm.height;
    const int mat_offset = prm.mat_size / 2;

    for (x1 = start; runningFlag() && (x1 < stop); ++x1)
    {
        valRed = valGreen = valBlue = 0.0;

        if (!prm.sixteenBit)        // 8 bits image.
        {
            uchar red, green, blue;
            uchar* ptr = 0;

            for (y2 = 0; runningFlag() && (y2 < prm.mat_size); ++y2)
            {
                int y2_matsize = y2 * prm.mat_size;

                for (x2 = 0; runningFlag() && (x2 < prm.mat_size); ++x2)
                {
                    index1 = prm.width * (y1 + y2 - mat_offset) + x1 + x2 - mat_offset;

                    if (index1 >= 0 && index1 < imageSize)
                    {
                        ptr                      =  &prm.orgData[index1 * 4];
                        blue                     =  ptr[0];
                        green                    =  ptr[1];
                        red                      =  ptr[2];
                        const double matrixValue =  prm.matrix[y2_matsize + x2];
                        valRed                   += matrixValue * red;
                        valGreen                 += matrixValue * green;
                        valBlue                  += matrixValue * blue;
                    }
                }
            }

            index2 = y1 * prm.width + x1;

            if (index2 >= 0 && index2 < imageSize)
            {
                // To get Alpha channel value from original (unchanged)
                memcpy(&prm.destData[index2 * 4], &prm.orgData[index2 * 4], 4);
                ptr = &prm.destData[index2 * 4];

                // Overwrite RGB values to destination.
                ptr[0] = (uchar) CLAMP(valBlue,  0.0, 255.0);
                ptr[1] = (uchar) CLAMP(valGreen, 0.0, 255.0);
                ptr[2] = (uchar) CLAMP(valRed,   0.0, 255.0);
            }
        }
        else                 // 16 bits image.
        {
            ushort red, green, blue;
            ushort* ptr = 0;

            for (y2 = 0; runningFlag() && (y2 < prm.mat_size); ++y2)
            {
                int y2_matsize = y2 * prm.mat_size;

                for (x2 = 0; runningFlag() && (x2 < prm.mat_size); ++x2)
                {
                    index1 = prm.width * (y1 + y2 - mat_offset) + x1 + x2 - mat_offset;

                    if (index1 >= 0 && index1 < imageSize)
                    {
                        ptr                      =  &orgData16[index1 * 4];
                        blue                     =  ptr[0];
                        green                    =  ptr[1];
                        red                      =  ptr[2];
                        const double matrixValue =  prm.matrix[y2_matsize + x2];
                        valRed                   += matrixValue * red;
                        valGreen                 += matrixValue * green;
                        valBlue                  += matrixValue * blue;
                    }
                }
            }

            index2 = y1 * prm.width + x1;

            if (index2 >= 0 && index2 < imageSize)
            {
                // To get Alpha channel value from original (unchanged)
                memcpy(&destData16[index2 * 4], &orgData16[index2 * 4], 8);
                ptr = &destData16[index2 * 4];

                // Overwrite RGB values to destination.
                ptr[0] = (ushort) CLAMP(valBlue,  0.0, 65535.0);
                ptr[1] = (ushort) CLAMP(valGreen, 0.0, 65535.0);
                ptr[2] = (ushort) CLAMP(valRed,   0.0, 65535.0);
            }
        }
    }
}

void RefocusFilter::convolveImage(const Args& prm)
{
    int progress;

    QList<int> vals = multithreadedSteps(prm.width);

    for (int y1 = 0; runningFlag() && (y1 < prm.height); ++y1)
    {
        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            tasks.append(QtConcurrent::run(this,
                                           &RefocusFilter::convolveImageMultithreaded,
                                           vals[j],
                                           vals[j+1],
                                           y1,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(((double)y1 * 100.0) / prm.height);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

FilterAction RefocusFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("correlation"), d->correlation);
    action.addParameter(QLatin1String("gauss"),       d->gauss);
    action.addParameter(QLatin1String("matrixSize"),  d->matrixSize);
    action.addParameter(QLatin1String("noise"),       d->noise);
    action.addParameter(QLatin1String("radius"),      d->radius);

    return action;
}

void RefocusFilter::readParameters(const Digikam::FilterAction& action)
{
    d->correlation = action.parameter(QLatin1String("correlation")).toDouble();
    d->gauss       = action.parameter(QLatin1String("gauss")).toDouble();
    d->matrixSize  = action.parameter(QLatin1String("matrixSize")).toInt();
    d->noise       = action.parameter(QLatin1String("noise")).toDouble();
    d->radius      = action.parameter(QLatin1String("radius")).toDouble();
}

int RefocusFilter::maxMatrixSize()
{
    return MAX_MATRIX_SIZE;
}

}  // namespace Digikam
