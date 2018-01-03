/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : a Brightness/Contrast/Gamma image filter.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "bcgfilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// Local includes

#include "dimg.h"

namespace Digikam
{

BCGContainer::BCGContainer()
{
    channel    = LuminosityChannel;
    brightness = 0.0;
    contrast   = 0.0;
    gamma      = 1.0;
}

bool BCGContainer::isDefault() const
{
    return (*this == BCGContainer());
}

bool BCGContainer::operator==(const BCGContainer& other) const
{
    return (channel    == other.channel    &&
            brightness == other.brightness &&
            contrast   == other.contrast   &&
            gamma      == other.gamma);
}

void BCGContainer::writeToFilterAction(FilterAction& action, const QString& prefix) const
{
    action.addParameter(prefix + QLatin1String("channel"),    channel);
    action.addParameter(prefix + QLatin1String("brightness"), brightness);
    action.addParameter(prefix + QLatin1String("contrast"),   contrast);
    action.addParameter(prefix + QLatin1String("gamma"),      gamma);
}

BCGContainer BCGContainer::fromFilterAction(const FilterAction& action, const QString& prefix)
{
    BCGContainer settings;
    settings.channel    = action.parameter(prefix + QLatin1String("channel"),    settings.channel);
    settings.brightness = action.parameter(prefix + QLatin1String("brightness"), settings.brightness);
    settings.contrast   = action.parameter(prefix + QLatin1String("contrast"),   settings.contrast);
    settings.gamma      = action.parameter(prefix + QLatin1String("gamma"),      settings.gamma);
    return settings;
}

// -----------------------------------------------------------------------------------------------

class BCGFilter::Private
{
public:

    Private()
    {
        memset(map,   0, sizeof(map));
        memset(map16, 0, sizeof(map16));
    }

    int          map[256];
    int          map16[65536];

    BCGContainer settings;
};

BCGFilter::BCGFilter(QObject* const parent)
    : DImgThreadedFilter(parent, QLatin1String("BCGFilter")),
      d(new Private)
{
    reset();
    initFilter();
}

BCGFilter::BCGFilter(DImg* const orgImage, QObject* const parent, const BCGContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("BCGFilter")),
      d(new Private)
{
    d->settings = settings;
    reset();
    initFilter();
}

BCGFilter::BCGFilter(const BCGContainer& settings, DImgThreadedFilter* const master,
                     const DImg& orgImage, const DImg& destImage, int progressBegin, int progressEnd)
    : DImgThreadedFilter(master, orgImage, destImage, progressBegin, progressEnd, QLatin1String("WBFilter")),
      d(new Private)
{
    d->settings = settings;
    reset();
    filterImage();
}

BCGFilter::~BCGFilter()
{
    cancelFilter();
    delete d;
}

FilterAction BCGFilter::filterAction()
{
    DefaultFilterAction<BCGFilter> action;
    d->settings.writeToFilterAction(action);
    return action;
}

void BCGFilter::readParameters(const FilterAction& action)
{
    d->settings = BCGContainer::fromFilterAction(action);
}

void BCGFilter::filterImage()
{
    setGamma(d->settings.gamma);
    setBrightness(d->settings.brightness);
    setContrast(d->settings.contrast);
    applyBCG(m_orgImage);
    m_destImage = m_orgImage;
}

void BCGFilter::setGamma(double val)
{
    val = (val < 0.01) ? 0.01 : val;

    for (int i = 0; i < 65536; ++i)
    {
        d->map16[i] = lround(pow(((double)d->map16[i] / 65535.0), (1.0 / val)) * 65535.0);
    }

    for (int i = 0; i < 256; ++i)
    {
        d->map[i] = lround(pow(((double)d->map[i] / 255.0), (1.0 / val)) * 255.0);
    }
}

void BCGFilter::setBrightness(double val)
{
    int val1 = lround(val * 65535);

    for (int i = 0; i < 65536; ++i)
    {
        d->map16[i] = d->map16[i] + val1;
    }

    val1 = lround(val * 255);

    for (int i = 0; i < 256; ++i)
    {
        d->map[i] = d->map[i] + val1;
    }
}

void BCGFilter::setContrast(double val)
{
    for (int i = 0; i < 65536; ++i)
    {
        d->map16[i] = lround((d->map16[i] - 32767) * val) + 32767;
    }

    for (int i = 0; i < 256; ++i)
    {
        d->map[i] = lround((d->map[i] - 127) * val) + 127;
    }
}

void BCGFilter::reset()
{
    // initialize to linear mapping

    for (int i = 0; i < 65536; ++i)
    {
        d->map16[i] = i;
    }

    for (int i = 0; i < 256; ++i)
    {
        d->map[i] = i;
    }
}

void BCGFilter::applyBCG(DImg& image)
{
    if (image.isNull())
    {
        return;
    }

    applyBCG(image.bits(), image.width(), image.height(), image.sixteenBit());
}

void BCGFilter::applyBCG(uchar* const bits, uint width, uint height, bool sixteenBits)
{
    if (!bits)
    {
        return;
    }

    uint size = width * height;
    int  progress;

    if (!sixteenBits)                    // 8 bits image.
    {
        uchar* data = bits;

        for (uint i = 0; runningFlag() && (i < size); ++i)
        {
            switch (d->settings.channel)
            {
                case BlueChannel:
                    data[0] = CLAMP0255(d->map[data[0]]);
                    break;

                case GreenChannel:
                    data[1] = CLAMP0255(d->map[data[1]]);
                    break;

                case RedChannel:
                    data[2] = CLAMP0255(d->map[data[2]]);
                    break;

                default:      // all channels
                    data[0] = CLAMP0255(d->map[data[0]]);
                    data[1] = CLAMP0255(d->map[data[1]]);
                    data[2] = CLAMP0255(d->map[data[2]]);
                    break;
            }

            data += 4;

            progress = (int)(((double)i * 100.0) / size);

            if (progress % 5 == 0)
            {
                postProgress(progress);
            }
        }
    }
    else                                        // 16 bits image.
    {
        ushort* data = reinterpret_cast<ushort*>(bits);

        for (uint i = 0; runningFlag() && (i < size); ++i)
        {
            switch (d->settings.channel)
            {
                case BlueChannel:
                    data[0] = CLAMP065535(d->map16[data[0]]);
                    break;

                case GreenChannel:
                    data[1] = CLAMP065535(d->map16[data[1]]);
                    break;

                case RedChannel:
                    data[2] = CLAMP065535(d->map16[data[2]]);
                    break;

                default:      // all channels
                    data[0] = CLAMP065535(d->map16[data[0]]);
                    data[1] = CLAMP065535(d->map16[data[1]]);
                    data[2] = CLAMP065535(d->map16[data[2]]);
                    break;
            }

            data += 4;

            progress = (int)(((double)i * 100.0) / size);

            if (progress % 5 == 0)
            {
                postProgress(progress);
            }
        }
    }

}

}  // namespace Digikam
