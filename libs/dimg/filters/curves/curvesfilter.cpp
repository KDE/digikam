/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-25-02
 * Description : Curves image filter
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "curvesfilter.h"

// Local includes

#include "dimg.h"
#include "digikam_debug.h"

namespace Digikam
{

CurvesFilter::CurvesFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

CurvesFilter::CurvesFilter(DImg* const orgImage, QObject* const parent, const CurvesContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("CurvesFilter"))
{
    m_settings = settings;
    initFilter();
}

CurvesFilter::CurvesFilter(const CurvesContainer& settings, DImgThreadedFilter* const master,
                           const DImg& orgImage, DImg& destImage, int progressBegin, int progressEnd)
    : DImgThreadedFilter(master, orgImage, destImage, progressBegin, progressEnd, QLatin1String("CurvesFilter"))
{
    m_settings = settings;

    // cannot operate in-place, so allocate dest image
    initFilter();
    destImage = m_destImage;
}

CurvesFilter::~CurvesFilter()
{
    cancelFilter();
}

void CurvesFilter::filterImage()
{
    postProgress(0);

    ImageCurves curves(m_settings);

    if (m_orgImage.sixteenBit() != m_settings.sixteenBit)
    {
        ImageCurves depthCurve(m_orgImage.sixteenBit());
        depthCurve.fillFromOtherCurves(&curves);
        curves = depthCurve;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Image 16 bits: " << m_orgImage.sixteenBit();
    qCDebug(DIGIKAM_DIMG_LOG) << "Curve 16 bits: " << curves.isSixteenBits();

    postProgress(50);

    // Process all channels curves
    curves.curvesLutSetup(AlphaChannel);
    postProgress(75);

    curves.curvesLutProcess(m_orgImage.bits(), m_destImage.bits(), m_orgImage.width(), m_orgImage.height());
    postProgress(100);
}

FilterAction CurvesFilter::filterAction()
{
    DefaultFilterAction<CurvesFilter> action(m_settings.isStoredLosslessly());

    m_settings.writeToFilterAction(action);

    return action;
}

void CurvesFilter::readParameters(const FilterAction& action)
{
    m_settings = CurvesContainer::fromFilterAction(action);
}

}  // namespace Digikam
