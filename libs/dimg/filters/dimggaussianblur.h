/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Gaussian Blur threaded image filter.
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef DIMGGAUSSIAN_BLUR_H
#define DIMGGAUSSIAN_BLUR_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT DImgGaussianBlur : public DImgThreadedFilter
{

public:

    explicit DImgGaussianBlur(DImg *orgImage, QObject *parent=0, double radius=3.0);

    // Constructor for slave mode: execute immediately in current thread with specified master filter
    explicit DImgGaussianBlur(DImgThreadedFilter *parentFilter, const DImg& orgImage, const DImg& destImage,
                              int progressBegin=0, int progressEnd=100, double radius=3.0);

    ~DImgGaussianBlur(){};

private:  // Gaussian blur filter data.

    double m_radius;

private:  // Gaussian blur filter methods.

    virtual void filterImage();

    void gaussianBlurImage(uchar *data, int width, int height, bool sixteenBit, double radius);
};

}  // namespace Digikam

#endif /* DIMGGAUSSIAN_BLUR_H */
