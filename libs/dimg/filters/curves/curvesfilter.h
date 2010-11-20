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

#ifndef CURVESFILTER_H
#define CURVESFILTER_H

// Qt includes

#include <QPolygon>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"
#include "imagecurves.h"

using namespace Digikam;

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT CurvesContainer
{

public:

    CurvesContainer(bool init=true)
    {
        curvesType = ImageCurves::CURVE_FREE;

        // Construct linear curves.
        lumCurveVals.resize(MAX_SEGMENT_16BIT+1);
        redCurveVals.resize(MAX_SEGMENT_16BIT+1);
        greenCurveVals.resize(MAX_SEGMENT_16BIT+1);
        blueCurveVals.resize(MAX_SEGMENT_16BIT+1);
        alphaCurveVals.resize(MAX_SEGMENT_16BIT+1);

        if (init)
        {
            for (int i = 0 ; i <= MAX_SEGMENT_16BIT ; ++i)
            {
                lumCurveVals.setPoint(i, i, i);
                redCurveVals.setPoint(i, i, i);
                greenCurveVals.setPoint(i, i, i);
                blueCurveVals.setPoint(i, i, i);
                alphaCurveVals.setPoint(i, i, i);
            }
        }
    };

    ~CurvesContainer(){};

public:

    ImageCurves::CurveType curvesType;      // Smooth : QPolygon have size of 18 points.
                                            // Free   : QPolygon have size of 255 or 65535 values.

    QPolygon               lumCurveVals;
    QPolygon               redCurveVals;
    QPolygon               greenCurveVals;
    QPolygon               blueCurveVals;
    QPolygon               alphaCurveVals;
};

// --------------------------------------------------------------------------------

class DIGIKAM_EXPORT CurvesFilter : public DImgThreadedFilter
{

public:

    explicit CurvesFilter(DImg* orgImage, QObject* parent=0, const CurvesContainer& settings=CurvesContainer());
    virtual ~CurvesFilter();

private:

    void filterImage();

private:

    CurvesContainer m_settings;
};

}  // namespace Digikam

#endif /* CURVESFILTER_H */
