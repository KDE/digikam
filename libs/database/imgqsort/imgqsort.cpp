/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Sorter
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013 by Gowtham Ashok <gwty93 at gmail dot com>
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

#include "imgqsort.h"

// C++ includes

#include <cmath>
#include <cfloat>
#include <iostream>
#include <cstdio>

// Qt includes.

#include <QTextStream>
#include <QFile>

// Kde include

#include <kdebug.h>

// Local includes

#include "nrestimate.h"
#include "nrfilter.h"
#include "libopencv.h"
#include "mixerfilter.h"

namespace Digikam
{

class ImgQSort::Private
{
public:

    Private() :
        max_lowThreshold(100)
    {
        edgeThresh   = 1;
        lowThreshold = 0.4;   // given in research paper
        ratio        = 3;
        kernel_size  = 3;
    };

    CvMat     src;
    CvMat     src_gray;
    CvMat     dst;
    CvMat     detected_edges;

    int const max_lowThreshold;

    int       edgeThresh;
    int       ratio;
    int       kernel_size;

    double    lowThreshold;
};

ImgQSort::ImgQSort(DImg* const img, QObject* const parent)
    : DImgThreadedAnalyser(img, parent, "ImgQSort"), d(new Private)
{
    //TODO: Using full image at present. Uncomment to set the window size.
    //int w = (img->width()  > d->size) ? d->size : img->width();
    //int h = (img->height() > d->size) ? d->size : img->height();
    //setOriginalImage(img->copy(0, 0, w, h));
}

ImgQSort::~ImgQSort()
{
    delete d;
}

void ImgQSort::readImage()
{
    MixerContainer settings;
    settings.bMonochrome = true;

    MixerFilter mixer(&m_orgImage, 0L, settings);
    mixer.startFilterDirectly();

    m_orgImage.putImageData(mixer.getTargetImage().bits());
}

/**
 * @function CannyThreshold
 * @brief Trackbar callback - Canny thresholds input with a ratio 1:3
 */
void ImgQSort::CannyThreshold(int, void*)
{
/** FIXME
    // Reduce noise with a kernel 3x3
    blur( src_gray, detected_edges, Size(3,3) );

    // Canny detector
    Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );

    // Using Canny's output as a mask, we display our result
    dst = Scalar::all(0);

    src.copyTo( dst, detected_edges);
*/
}

double ImgQSort::blurdetector() const
{
/* FIXME
    // Load an image
    src = imread( argv[1] );

    // Create a matrix of the same type and size as src (for dst)
    dst.create( src.size(), src.type() );

    // Convert the image to grayscale
    cvtColor( src, src_gray, CV_BGR2GRAY );

    ImgQSort::CannyThreshold(0, 0);

    double average    = mean(detected_edges)[0];
    double maxval     = 0;
    int* const maxIdx = (int* )malloc(sizeof(detected_edges));
    minMaxIdx(detected_edges, 0, &maxval, 0, maxIdx);

    double blurresult=average/maxval;
    KDebug() << "The average of the edge intensity is " << average;
    KDebug() << "The maximum of the edge intensity is " << maxval;
    KDebug() << "The result of the edge intensity is "  << blurresult;

    return blurresult;
*/
    return 0.0;
}

void ImgQSort::startAnalyse()
{
    double amount_of_blur = blurdetector();
}

}  // namespace Digikam
