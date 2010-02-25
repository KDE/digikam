/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-25-02
 * Description : Curves image filter
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "imagecurves.h"

namespace Digikam
{

CurvesFilter::CurvesFilter(DImg* orgImage, QObject* parent, const CurvesContainer& settings)
            : DImgThreadedFilter(orgImage, parent, "CurvesFilter")
{
    m_settings = settings;
    initFilter();
}

CurvesFilter::~CurvesFilter()
{
}

void CurvesFilter::filterImage()
{
    postProgress(10);
    ImageCurves curves(m_orgImage.sixteenBit());
    curves.setCurvePoints(LuminosityChannel, m_settings.lumCurvePts);
    postProgress(20);

    curves.setCurvePoints(RedChannel, m_settings.redCurvePts);
    postProgress(30);

    curves.setCurvePoints(GreenChannel, m_settings.greenCurvePts);
    postProgress(40);

    curves.setCurvePoints(BlueChannel, m_settings.blueCurvePts);
    postProgress(50);
    
    curves.setCurvePoints(AlphaChannel, m_settings.alphaCurvePts);
    postProgress(60);
    
    m_destImage = DImg(m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit(), m_orgImage.hasAlpha());
    postProgress(70);
    
    // Process all channels curves
    curves.curvesLutSetup(AlphaChannel);
    postProgress(80);
    
    curves.curvesLutProcess(m_orgImage.bits(), m_destImage.bits(), m_orgImage.width(), m_orgImage.height());
    postProgress(90);
}

}  // namespace Digikam
