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
#include <cstdio>

// Kde include

#include <kdebug.h>

// Local includes

#include "nrestimate.h"
#include "libopencv.h"
#include "mixerfilter.h"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

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

    Mat       src;
    Mat       src_gray;
    Mat       dst;
    Mat       detected_edges;

    int const max_lowThreshold;

    int       edgeThresh;
    int       ratio;
    int       kernel_size;

    double    lowThreshold;

    DImg      image;
};

ImgQSort::ImgQSort()
    : d(new Private)
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

PickLabel ImgQSort::analyseQuality(const DImg& img)
{
    d->image = img;

    kDebug() << "Amount of Blur present in image is ";
    kDebug() << blurdetector();
    kDebug() << "Amount of Noise present in image is ";
    kDebug() << noisedetector();

    // FIXME
    return NoPickLabel;
}

void ImgQSort::readImage()
{
    MixerContainer settings;
    settings.bMonochrome = true;

    MixerFilter mixer(&d->image, 0L, settings);
    mixer.startFilterDirectly();

    d->image.putImageData(mixer.getTargetImage().bits());
}

/**
 * @function CannyThreshold
 * @brief Trackbar callback - Canny thresholds input with a ratio 1:3
 */
void ImgQSort::CannyThreshold(int, void*) const
{
    // Reduce noise with a kernel 3x3
    blur(d->src_gray, d->detected_edges, Size(3,3) );

    // Canny detector
    Canny(d->detected_edges, d->detected_edges, d->lowThreshold, d->lowThreshold*d->ratio,d-> kernel_size );

    // Using Canny's output as a mask, we display our result
    d->dst = Scalar::all(0);

    d->src.copyTo(d-> dst, d->detected_edges);
}

double ImgQSort::blurdetector() const
{
    d->lowThreshold   = 0.4;
    double average    = 0.0;
    double maxval     = 0.0;
    double blurresult = 0.0;

    // Create a matrix of the same type and size as src (for dst)
    d->dst.create( d->src.size(), d->src.type() );

    // Convert the image to grayscale
    cvtColor( d->src, d->src_gray, CV_BGR2GRAY );

    ImgQSort::CannyThreshold(0, 0);

    average           = mean(d->detected_edges)[0];
    int* const maxIdx = (int* )malloc(sizeof(d->detected_edges));
    minMaxIdx(d->detected_edges, 0, &maxval, 0, maxIdx);

    blurresult=average/maxval;
    kDebug() << "The average of the edge intensity is ";
    kDebug() << average;
    kDebug() << "The maximum of the edge intensity is ";
    kDebug() << maxval;
    kDebug() << "The result of the edge intensity is ";
    kDebug() << blurresult;

    return blurresult;
}

double ImgQSort::noisedetector() const
{

    d->lowThreshold    = 0.035;   //given in research paper for noise. Variable parameter
    double noiseresult = 0.0;
    double average     = 0.0;
    double maxval      = 0.0;

    if ( !d->src.data )
    {
        return -1;
    }

    // Create a matrix of the same type and size as src (for dst)
    d->dst.create(d-> src.size(), d->src.type() );

    // Convert the image to grayscale
    cvtColor(d-> src, d->src_gray, CV_BGR2GRAY );

    // Apply Canny Edge Detector to get the edges
    CannyThreshold(0, 0);

    average     = mean(d->detected_edges)[0];
    int* maxIdx = (int* )malloc(sizeof(d->detected_edges));

    // To find the maximum edge intensity value

    minMaxIdx(d->detected_edges, 0, &maxval, 0, maxIdx);

    noiseresult = average/maxval;

    kDebug() << "The average of the edge intensity is ";
    kDebug() << average;
    kDebug() << "The maximum of the edge intensity is ";
    kDebug() << maxval;
    kDebug() << "The result of the edge intensity is ";
    kDebug() << noiseresult;

    return noiseresult;
}

}  // namespace Digikam
