/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Parser - private container
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
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

#ifndef DIGIKAM_IMAGE_QUALITY_PARSER_P_H
#define DIGIKAM_IMAGE_QUALITY_PARSER_P_H

#include "imagequalityparser.h"

// C++ includes

#include <cmath>
#include <cfloat>
#include <cstdio>

// Qt includes

#include <QTextStream>
#include <QFile>
#include <QImage>

// Local includes

#include "digikam_opencv.h"
#include "digikam_debug.h"
#include "mixerfilter.h"
#include "nrfilter.h"
#include "nrestimate.h"
#include "exposurecontainer.h"

// To switch on/off log trace file.
// #define TRACE 1

using namespace cv;

namespace Digikam
{

class Q_DECL_HIDDEN ImageQualityParser::Private
{
public:

    explicit Private()
      : clusterCount(30),                   // used for k-means clustering algorithm in noise detection
        size(512)
    {
        for (int c = 0 ; c < 3 ; ++c)
        {
            fimg[c] = 0;
        }

        // Setting the default values

        edgeThresh        = 1;
        lowThreshold      = 0.4;
        ratio             = 3;
        kernel_size       = 3;
        blurrejected      = 0.0;
        blur              = 0.0;
        acceptedThreshold = 0.0;
        pendingThreshold  = 0.0;
        rejectedThreshold = 0.0;
        label             = 0;
        running           = true;
    }

    float*                fimg[3];
    const uint            clusterCount;
    const uint            size;              // Size of squared original image.

    Mat                   src_gray;          // Matrix of the grayscaled source image
    Mat                   detected_edges;    // Matrix containing only edges in the image

    int                   edgeThresh;        // threshold above which we say that edges are present at a point
    int                   ratio;             // lower:upper threshold for canny edge detector algorithm
    int                   kernel_size;       // kernel size for the Sobel operations to be performed internally by the edge detector

    double                lowThreshold;

    DImg                  image;             // original image
    DImg                  neimage;           // noise estimation image[for color]
    DImg                  img8;              // compression detector image on 8 bits

    ImageQualityContainer imq;

    double                blurrejected;
    double                blur;

    double                acceptedThreshold;
    double                pendingThreshold;
    double                rejectedThreshold;

    QString               path;              // Path to host result file

    PickLabel*            label;

    volatile bool         running;
};

} // namespace Digikam

#endif // DIGIKAM_IMAGE_QUALITY_PARSER_P_H
