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

#include "icctransformfilter.h"

// C++ includes

#include <cmath>

// Local includes

#include "dimg.h"

namespace Digikam
{

IccTransformFilter::IccTransformFilter(DImg *orgImage, QObject *parent, const IccTransform& transform)
                  : DImgThreadedFilter(orgImage, parent, "ICC Transform")
{
    m_transform = transform;
    // initialize filter
    initFilter();
}

void IccTransformFilter::filterImage()
{
    m_destImage = m_orgImage;
    m_transform.apply(m_destImage, this);
    m_destImage.setIccProfile(m_transform.outputProfile());
}

void IccTransformFilter::progressInfo(const DImg*, float progress)
{
    postProgress(lround(progress * 100));
}

}  // namespace DigikamImagesPluginCore
