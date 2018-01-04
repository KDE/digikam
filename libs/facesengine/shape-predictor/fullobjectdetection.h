/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 16/08/2016
 * Description : Full object detection class representing the output of the
 *               shape predictor class, containing 64 facial point including
 *               eye, nose, and mouth.
 *
 * Copyright (C) 2016 by Omar Amin <Omar dot moh dot amin at gmail dot com>
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

#ifndef FULL_OBJECT_DETECTION_H
#define FULL_OBJECT_DETECTION_H

// C++ includes

#include <vector>

// Local includes

#include "libopencv.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT FullObjectDetection
{
public:

    FullObjectDetection(const cv::Rect& rect_, const std::vector<std::vector<float> >& parts_);
    FullObjectDetection();
    FullObjectDetection(const cv::Rect& rect_);

    const cv::Rect& get_rect() const;

    cv::Rect& get_rect();

    unsigned long num_parts() const;

    const std::vector<float>& part(unsigned long idx) const;

    std::vector<float>& part(unsigned long idx);

private:

    cv::Rect                         rect;
    std::vector<std::vector<float> > parts;
};

// -------------------------------------------------------------------

std::vector<cv::Rect> geteyes(const FullObjectDetection& shape);

}  // namespace Digikam

#endif // FULL_OBJECT_DETECTION_H
