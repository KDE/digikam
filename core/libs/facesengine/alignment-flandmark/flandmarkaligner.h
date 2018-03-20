/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2013-06-14
 * Description : Alignment by Image Fland Mark Aligner.
 *               Funneling for complex, realistic images
 *               using sequence of distribution fields learned from congealReal
 *               Gary B. Huang, Vidit Jain, and Erik Learned-Miller.
 *               Unsupervised joint alignment of complex images.
 *               International Conference on Computer Vision (ICCV), 2007.
 *
 * Copyright (C) 2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2007 by Gary B. Huang, UMass-Amherst
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

#ifndef FACESENGINE_ALIGNMENT_FLANDMARK_ALIGNER
#define FACESENGINE_ALIGNMENT_FLANDMARK_ALIGNER

// OpenCV includes

#include "libopencv.h"

namespace Digikam
{

class FlandmarkAligner
{

public:

    FlandmarkAligner();
    ~FlandmarkAligner();

    cv::Mat align(const cv::Mat& inputImage);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // FACESENGINE_ALIGNMENT_CONGEALING_FUNNELREAL_H
