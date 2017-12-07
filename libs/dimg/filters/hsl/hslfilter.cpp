/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : Hue/Saturation/Lightness image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Julien Narboux <julien at narboux dot fr>
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

#include "hslfilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// Local includes

#include "dimg.h"

namespace Digikam
{

class HSLFilter::Private
{
public:

    Private()
    {
        memset(&htransfer,   0, sizeof(htransfer));
        memset(&ltransfer,   0, sizeof(ltransfer));
        memset(&stransfer,   0, sizeof(stransfer));
        memset(&htransfer16, 0, sizeof(htransfer16));
        memset(&ltransfer16, 0, sizeof(ltransfer16));
        memset(&stransfer16, 0, sizeof(stransfer16));
    }

    int          htransfer[256];
    int          ltransfer[256];
    int          stransfer[256];

    int          htransfer16[65536];
    int          ltransfer16[65536];
    int          stransfer16[65536];

    HSLContainer settings;
};

HSLFilter::HSLFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    reset();
    initFilter();
}

HSLFilter::HSLFilter(DImg* const orgImage, QObject* const parent, const HSLContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("HSLFilter")),
      d(new Private)
{
    d->settings = settings;
    reset();
    initFilter();
}

HSLFilter::~HSLFilter()
{
    cancelFilter();
    delete d;
}

void HSLFilter::filterImage()
{
    setHue(d->settings.hue);
    setSaturation(d->settings.saturation);
    setLightness(d->settings.lightness);
    applyHSL(m_orgImage);
    m_destImage = m_orgImage;
}

void HSLFilter::reset()
{
    // initialize to linear mapping

    for (int i = 0; i < 65536; ++i)
    {
        d->htransfer16[i] = i;
        d->ltransfer16[i] = i;
        d->stransfer16[i] = i;
    }

    for (int i = 0; i < 256; ++i)
    {
        d->htransfer[i] = i;
        d->ltransfer[i] = i;
        d->stransfer[i] = i;
    }
}

void HSLFilter::setHue(double val)
{
    int value;

    for (int i = 0; i < 65536; ++i)
    {
        value = lround(val * 65535.0 / 360.0);

        if ((i + value) < 0)
        {
            d->htransfer16[i] = 65535 + (i + value);
        }
        else if ((i + value) > 65535)
        {
            d->htransfer16[i] = i + value - 65535;
        }
        else
        {
            d->htransfer16[i] = i + value;
        }
    }

    for (int i = 0; i < 256; ++i)
    {
        value = lround(val * 255.0 / 360.0);

        if ((i + value) < 0)
        {
            d->htransfer[i] = 255 + (i + value);
        }
        else if ((i + value) > 255)
        {
            d->htransfer[i] = i + value - 255;
        }
        else
        {
            d->htransfer[i] = i + value;
        }
    }
}

void HSLFilter::setSaturation(double val)
{
    val = CLAMP(val, -100.0, 100.0);
    int value;

    for (int i = 0; i < 65536; ++i)
    {
        value = lround((i * (100.0 + val)) / 100.0);
        d->stransfer16[i] = CLAMP065535(value);
    }

    for (int i = 0; i < 256; ++i)
    {
        value = lround((i * (100.0 + val)) / 100.0);
        d->stransfer[i]  = CLAMP0255(value);
    }
}

void HSLFilter::setLightness(double val)
{
    // val needs to be in that range so that the result is in the range 0..65535
    val = CLAMP(val, -100.0, 100.0);

    if (val < 0)
    {
        for (int i = 0; i < 65536; ++i)
        {
            d->ltransfer16[i] = lround((i * (val + 100.0)) / 100.0);
        }

        for (int i = 0; i < 256; ++i)
        {
            d->ltransfer[i] = lround((i * (val + 100.0)) / 100.0);
        }
    }
    else
    {
        for (int i = 0; i < 65536; ++i)
        {
            d->ltransfer16[i] = lround(i * (1.0 - val / 100.0)  + 65535.0 / 100.0 * val);
        }

        for (int i = 0; i < 256; ++i)
        {
            d->ltransfer[i] = lround(i * (1.0 - val / 100.0)  + 255.0 / 100.0 * val);
        }
    }
}

int HSLFilter::vibranceBias(double sat, double hue, double vib, bool sixteenbit)
{
    double ratio;
    int    localsat;
    double normalized_hue = hue / (sixteenbit ? 65535.0 : 255.0);

    if (normalized_hue > 0.85 || normalized_hue < 0.2)
    {
        ratio = 0.3;
    }
    else
    {
        ratio = 1.0;
    }

    localsat = lround((sat * (100.0 + vib * ratio)) / 100.0);

    if (sixteenbit)
    {
        return(CLAMP065535(localsat));
    }
    else
    {
        return(CLAMP0255(localsat));
    }
}

void HSLFilter::applyHSL(DImg& image)
{
    if (image.isNull())
    {
        return;
    }

    bool   sixteenBit     = image.sixteenBit();
    uint   numberOfPixels = image.numPixels();
    int    progress;
    int    hue, sat, lig;
    double vib = d->settings.vibrance;
    DColor color;

    if (sixteenBit)                   // 16 bits image.
    {
        unsigned short* data = reinterpret_cast<unsigned short*>(image.bits());

        for (uint i = 0; runningFlag() && (i < numberOfPixels); ++i)
        {
            color = DColor(data[2], data[1], data[0], 0, sixteenBit);

            // convert RGB to HSL
            color.getHSL(&hue, &sat, &lig);

            // convert HSL to RGB
            color.setHSL(d->htransfer16[hue], vibranceBias(d->stransfer16[sat], hue, vib, sixteenBit), d->ltransfer16[lig], sixteenBit);

            data[2] = color.red();
            data[1] = color.green();
            data[0] = color.blue();

            data += 4;

            progress = (int)(((double)i * 100.0) / numberOfPixels);

            if (progress % 5 == 0)
            {
                postProgress(progress);
            }
        }
    }
    else                                      // 8 bits image.
    {
        uchar* data = image.bits();

        for (uint i = 0; runningFlag() && (i < numberOfPixels); ++i)
        {
            color = DColor(data[2], data[1], data[0], 0, sixteenBit);

            // convert RGB to HSL
            color.getHSL(&hue, &sat, &lig);

            // convert HSL to RGB
            color.setHSL(d->htransfer[hue], vibranceBias(d->stransfer[sat], hue, vib, sixteenBit), d->ltransfer[lig], sixteenBit);

            data[2] = color.red();
            data[1] = color.green();
            data[0] = color.blue();

            data += 4;

            progress = (int)(((double)i * 100.0) / numberOfPixels);

            if (progress % 5 == 0)
            {
                postProgress(progress);
            }
        }
    }
}

FilterAction HSLFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("hue"),        d->settings.hue);
    action.addParameter(QLatin1String("lightness"),  d->settings.lightness);
    action.addParameter(QLatin1String("saturation"), d->settings.saturation);
    action.addParameter(QLatin1String("vibrance"),   d->settings.vibrance);

    return action;
}

void HSLFilter::readParameters(const Digikam::FilterAction& action)
{
    d->settings.hue        = action.parameter(QLatin1String("hue")).toDouble();
    d->settings.lightness  = action.parameter(QLatin1String("lightness")).toDouble();
    d->settings.saturation = action.parameter(QLatin1String("saturation")).toDouble();
    d->settings.vibrance   = action.parameter(QLatin1String("vibrance")).toDouble();
}

}  // namespace Digikam
