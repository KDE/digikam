/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Unsharp Mask threaded image filter.
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

#ifndef DIMGUNSHARPMASK_H_
#define DIMGUNSHARPMASK_H_

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT DImgUnsharpMask : public DImgThreadedFilter
{

public:

    explicit DImgUnsharpMask(DImg *orgImage, QObject *parent=0, int radius=1,
                             double amount=1.0, double threshold=0.05);

    // Constructor for slave mode: execute immediately in current thread with specified master filter
    // DImgUnsharpMask(DImgThreadedFilter *parentFilter, const DImg& orgImage, const DImg& destImage,
    //            int progressBegin=0, int progressEnd=100, double radius=0.0, double sigma=1.0);

    ~DImgUnsharpMask(){};

private:

    virtual void filterImage();

private:

    int    m_radius;

    double m_amount;
    double m_threshold;
};

} // namespace Digikam

#endif /* DIMGUNSHARPMASK_H_ */
