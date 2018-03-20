/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : Change tonality image filter
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "tonalityfilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// Local includes

#include "dimg.h"
#include "dcolor.h"

namespace Digikam
{

TonalityFilter::TonalityFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

TonalityFilter::TonalityFilter(DImg* const orgImage, QObject* const parent, const TonalityContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("TonalityFilter"))
{
    m_settings = settings;
    initFilter();
}

TonalityFilter::~TonalityFilter()
{
    cancelFilter();
}

/** Change color tonality of an image for applying a RGB color mask.*/
void TonalityFilter::filterImage()
{
    m_destImage.putImageData(m_orgImage.bits());

    uchar* bits       = m_destImage.bits();
    uint   width      = m_destImage.width();
    uint   height     = m_destImage.height();
    bool   sixteenBit = m_destImage.sixteenBit();
    uint   size       = width * height;
    int    progress;
    int    hue, sat, lig;

    DColor mask(m_settings.redMask, m_settings.greenMask, m_settings.blueMask, 0, sixteenBit);
    mask.getHSL(&hue, &sat, &lig);

    if (!sixteenBit)        // 8 bits image.
    {
        uchar* ptr = bits;

        for (uint i = 0 ; i < size ; ++i)
        {
            // Convert to grayscale using tonal mask

            lig = lround(0.3 * ptr[2] + 0.59 * ptr[1] + 0.11 * ptr[0]);

            mask.setHSL(hue, sat, lig, sixteenBit);

            ptr[0] = (uchar)mask.blue();
            ptr[1] = (uchar)mask.green();
            ptr[2] = (uchar)mask.red();
            ptr += 4;

            progress = (int)(((double)i * 100.0) / size);

            if (progress % 5 == 0)
            {
                postProgress(progress);
            }
        }
    }
    else               // 16 bits image.
    {
        unsigned short* ptr = reinterpret_cast<unsigned short*>(bits);

        for (uint i = 0 ; i < size ; ++i)
        {
            // Convert to grayscale using tonal mask

            lig = lround(0.3 * ptr[2] + 0.59 * ptr[1] + 0.11 * ptr[0]);

            mask.setHSL(hue, sat, lig, sixteenBit);

            ptr[0] = (unsigned short)mask.blue();
            ptr[1] = (unsigned short)mask.green();
            ptr[2] = (unsigned short)mask.red();
            ptr += 4;

            progress = (int)(((double)i * 100.0) / size);

            if (progress % 5 == 0)
            {
                postProgress(progress);
            }
        }
    }
}

FilterAction TonalityFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("blueMask"), m_settings.blueMask);
    action.addParameter(QLatin1String("greenMask"), m_settings.greenMask);
    action.addParameter(QLatin1String("redMask"), m_settings.redMask);

    return action;
}

void TonalityFilter::readParameters(const Digikam::FilterAction& action)
{
    m_settings.blueMask  = action.parameter(QLatin1String("blueMask")).toInt();
    m_settings.greenMask = action.parameter(QLatin1String("greenMask")).toInt();
    m_settings.redMask   = action.parameter(QLatin1String("redMask")).toInt();
}

}  // namespace Digikam
