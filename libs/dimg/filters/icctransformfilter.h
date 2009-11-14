/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-19
 * Description : ICC Transform threaded image filter.
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ICCTRANSFORMFILTER_H
#define ICCTRANSFORMFILTER_H

// Local includes

#include "dimgloaderobserver.h"
#include "dimgthreadedfilter.h"
#include "icctransform.h"

namespace Digikam
{

class DIGIKAM_EXPORT IccTransformFilter : public DImgThreadedFilter, public DImgLoaderObserver
{

public:

    IccTransformFilter(DImg *orgImage, QObject *parent, const IccTransform& transform);

protected:

    virtual void progressInfo(const DImg *, float progress);
    virtual void filterImage();

private:

    IccTransform m_transform;
};

}  // namespace Digikam

#endif /* DIMGREFOCUS_H */
