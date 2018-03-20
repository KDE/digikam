/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image curves manipulation methods.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "curvescontainer.h"
#include "imagecurves.h"
#include "filteraction.h"

namespace Digikam
{

CurvesContainer::CurvesContainer()
    : curvesType(ImageCurves::CURVE_SMOOTH), sixteenBit(false)
{
}

CurvesContainer::CurvesContainer(int type, bool sixteenBit)
    : curvesType((ImageCurves::CurveType)type), sixteenBit(sixteenBit)
{
}

bool CurvesContainer::isEmpty() const
{
    for (int i = 0; i < ColorChannels; ++i)
    {
        if (!values[i].isEmpty())
        {
            return false;
        }
    }

    return true;
}

bool CurvesContainer::operator==(const CurvesContainer& other) const
{
    if (isEmpty() && other.isEmpty())
    {
        return true;
    }

    if (sixteenBit != other.sixteenBit || curvesType != other.curvesType)
    {
        return false;
    }

    for (int i = 0; i < ColorChannels; ++i)
    {
        if (values[i] != other.values[i])
        {
            return false;
        }
    }

    return true;
}

void CurvesContainer::initialize()
{
    int segmentMax = sixteenBit ? MAX_SEGMENT_16BIT : MAX_SEGMENT_8BIT;

    // Construct linear curves.

    if (curvesType == ImageCurves::CURVE_FREE)
    {
        for (int i = 0; i < ColorChannels; ++i)
        {
            values[i].resize(segmentMax + 1);

            for (int j = 0 ; j <= segmentMax ; ++j)
            {
                values[i].setPoint(j, j, j);
            }
        }
    }
    else // SMOOTH
    {
        for (int i = 0; i < ColorChannels; ++i)
        {
            values[i].resize(ImageCurves::NUM_POINTS);

            for (int j = 1 ; j < ImageCurves::NUM_POINTS - 1 ; ++j)
            {
                values[i].setPoint(j, -1, -1);
            }

            // First and last points init.
            values[i].setPoint(0, 0, 0);
            values[i].setPoint(ImageCurves::NUM_POINTS - 1, segmentMax, segmentMax);
        }
    }
}

bool CurvesContainer::isStoredLosslessly() const
{
    return !(sixteenBit && curvesType == ImageCurves::CURVE_FREE);
}

void CurvesContainer::writeToFilterAction(FilterAction& action, const QString& prefix) const
{
    if (isEmpty())
    {
        return;
    }

    ImageCurves curves(*this);

    if (curves.isLinear())
    {
        return;
    }

    // Convert to 8bit: 16 bits curves takes 85kb, 8 bits only 400 bytes.
    if (curves.isSixteenBits())
    {
        ImageCurves depthCurve(false);
        depthCurve.fillFromOtherCurves(&curves);
        curves = depthCurve;
    }

    action.addParameter(prefix + QLatin1String("curveBitDepth"), 8);

    for (int i = 0; i < ColorChannels; ++i)
    {
        action.addParameter(prefix + QString::fromLatin1("curveData[%1]").arg(i), curves.channelToBinary(i).toBase64());
    }
}

CurvesContainer CurvesContainer::fromFilterAction(const FilterAction& action, const QString& prefix)
{
    if (!action.hasParameter(prefix + QLatin1String("curveBitDepth")))
    {
        return CurvesContainer();
    }

    ImageCurves curves(action.parameter(prefix + QLatin1String("curveBitDepth"), 8) == 16);

    for (int i = 0; i < ColorChannels; ++i)
    {
        QByteArray base64 = action.parameter(prefix + QString::fromLatin1("curveData[%1]").arg(i), QByteArray());
        // check return value and set readParametersError?
        curves.setChannelFromBinary(i, QByteArray::fromBase64(base64));
    }

    // We don't need to call curves.curvesCalculateAllCurves() here ???
    return curves.getContainer();
}

}  // namespace Digikam
