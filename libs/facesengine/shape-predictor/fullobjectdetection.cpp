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

// Local includes

#include "fullobjectdetection.h"

namespace Digikam
{

FullObjectDetection::FullObjectDetection(const cv::Rect& rect_, const std::vector<std::vector<float> >& parts_)
    : rect(rect_),
      parts(parts_)
{
}

FullObjectDetection::FullObjectDetection()
{
}

FullObjectDetection::FullObjectDetection(const cv::Rect& rect_)
    : rect(rect_)
{
}

const cv::Rect& FullObjectDetection::get_rect() const
{
    return rect;
}

cv::Rect& FullObjectDetection::get_rect()
{
    return rect;
}

unsigned long FullObjectDetection::num_parts() const
{
    return parts.size();
}

const std::vector<float>& FullObjectDetection::part(unsigned long idx) const
{
    return parts[idx];
}

std::vector<float>& FullObjectDetection::part(unsigned long idx)
{
    return parts[idx];
}

// -------------------------------------------------------------------

std::vector<cv::Rect> geteyes(const FullObjectDetection& shape)
{
    std::vector<cv::Rect> eyes;

    for (int j = 0 ; j < 2 ; j++)
    {
        int start = j ? 36 : 42;
        int end   = j ? 41 : 47;
        int tlx, tly, brx, bry;

        // initializing
        std::vector<float> firstpoint = shape.part(start);
        tlx                           = firstpoint[0];
        brx                           = firstpoint[0];
        tly                           = firstpoint[1];
        bry                           = firstpoint[1];

        for(int i = start ; i <= end ; i++)
        {
            std::vector<float> x = shape.part(i);

            if (x[0] < tlx)
                tlx = x[0];
            else if (x[0] > brx)
                brx = x[0];
            if (x[1] < tly)
                tly = x[1];
            else if (x[1] > bry)
                bry = x[1];
        }

        eyes.push_back(cv::Rect(cv::Point(tlx,tly), cv::Point(brx,bry)));
    }

    return eyes;
}

}  // namespace Digikam
