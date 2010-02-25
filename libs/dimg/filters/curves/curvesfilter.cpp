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

CurvesFilter::CurvesFilter(DImg* orgImage, QObject* parent, const QPolygon& curvesPts)
            : DImgThreadedFilter(orgImage, parent, "CurvesFilter")
{
    m_curvesPts = curvesPts;
    initFilter();
}

CurvesFilter::~CurvesFilter()
{
}

void CurvesFilter::filterImage()
{
    postProgress(10);    
    m_destImage.putImageData(m_orgImage.bits());
    
    postProgress(20);

    if (!m_curvesPts.isEmpty())
    {
        postProgress(30);
        ImageCurves curves(m_destImage.sixteenBit());

        postProgress(40);
        curves.setCurvePoints(AlphaChannel, m_curvesPts);
        
        postProgress(50);
        uchar* targetData = new uchar[m_destImage.numBytes()];

        postProgress(60);
        curves.curvesLutSetup(AlphaChannel);

        postProgress(70);
        curves.curvesLutProcess(m_destImage.bits(), targetData, m_destImage.width(), m_destImage.height());

        postProgress(80);
        m_destImage.putImageData(targetData);
    }
    
    postProgress(90);
}

}  // namespace Digikam
