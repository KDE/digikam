/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-25-02
 * Description : Curves image filter
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

namespace Digikam
{

CurvesFilter::CurvesFilter(QObject* parent)
                 : DImgThreadedFilter(parent)
{
    initFilter();
}

CurvesFilter::CurvesFilter(DImg* orgImage, QObject* parent, const CurvesContainer& settings)
            : DImgThreadedFilter(orgImage, parent, "CurvesFilter")
{
    m_settings = settings;
    initFilter();
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

    postProgress(50);

    // Process all channels curves
    curves.curvesLutSetup(AlphaChannel);
    postProgress(75);

    curves.curvesLutProcess(m_orgImage.bits(), m_destImage.bits(), m_orgImage.width(), m_orgImage.height());
    postProgress(100);
}

bool CurvesFilter::isStoredLosslessly(const CurvesContainer& settings)
{
    return !settings.sixteenBit || !settings.curvesType == ImageCurves::CURVE_FREE;
}

void CurvesFilter::addCurvesParameters(FilterAction& action, const CurvesContainer& settings)
{
    ImageCurves curves(settings);
    // Convert to 8bit: 16 bits curves takes 85kb, 8 bits only 400 bytes.
    if (curves.isSixteenBits())
    {
        ImageCurves depthCurve(false);
        depthCurve.fillFromOtherCurves(&curves);
        curves = depthCurve;
    }

    action.addParameter("curveBitDepth", 8);

    for (int i=0; i<ColorChannels; i++)
        action.addParameter(QString("curveData[%1]").arg(i), curves.channelToBase64(i));
}

CurvesContainer CurvesFilter::readCurvesParameters(const FilterAction& action)
{
    ImageCurves curves(action.parameter("curveBitDepth").toInt() == 16);
    for (int i=0; i<ColorChannels; i++)
    {
        QByteArray base64 = action.parameter(QString("curveData[%1]").arg(i)).toByteArray();
        // check return value and set readParametersError?
        curves.setChannelFromBase64(i, base64);
    }
    return curves.getContainer();
}

FilterAction CurvesFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion(),
                        isStoredLosslessly(m_settings) ? FilterAction::ComplexFilter : FilterAction::ReproducibleFilter);
    action.setDisplayableName(DisplayableName());

    addCurvesParameters(action, m_settings);

    return action;
}

void CurvesFilter::readParameters(const FilterAction& action)
{
    m_settings = readCurvesParameters(action);
}

}  // namespace Digikam
