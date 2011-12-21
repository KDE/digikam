/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Charcoal threaded image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#define SQ2PI   2.50662827463100024161235523934010416269302368164062
#define Epsilon 1.0e-12

#include "charcoalfilter.h"

// C++ includes

#include <cmath>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "blurfilter.h"
#include "stretchfilter.h"
#include "mixerfilter.h"
#include "invertfilter.h"

namespace Digikam
{

CharcoalFilter::CharcoalFilter(QObject* parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

CharcoalFilter::CharcoalFilter(DImg* orgImage, QObject* parent, double pencil, double smooth)
    : DImgThreadedFilter(orgImage, parent, "Charcoal")
{
    m_pencil = pencil;
    m_smooth = smooth;

    initFilter();
}

CharcoalFilter::~CharcoalFilter()
{
    cancelFilter();
}

void CharcoalFilter::filterImage()
{
    if (m_orgImage.isNull())
    {
        kWarning() << "No image data available!";
        return;
    }

    if (m_pencil <= 0.0)
    {
        m_destImage = m_orgImage;
        return;
    }

    // -- Applying Edge effect -----------------------------------------------

    register long i = 0;
    int kernelWidth = getOptimalKernelWidth(m_pencil, m_smooth);

    if ((int)m_orgImage.width() < kernelWidth)
    {
        kWarning() << "Image is smaller than radius!";
        return;
    }

    QScopedArrayPointer<double> kernel(new double[kernelWidth * kernelWidth]);

    if (kernel.isNull())
    {
        kWarning() << "Unable to allocate memory!";
        return;
    }

    for (i = 0 ; i < (kernelWidth * kernelWidth) ; ++i)
    {
        kernel[i] = (-1.0);
    }

    kernel[i / 2] = kernelWidth * kernelWidth - 1.0;
    convolveImage(kernelWidth, kernel.data());

    // -- Applying Gaussian blur effect ---------------------------------------

    BlurFilter(this, m_destImage, m_destImage, 80, 85, (int)(m_smooth / 10.0));

    if (!runningFlag())
    {
        return;
    }

    // -- Applying stretch contrast color effect -------------------------------

    StretchFilter stretch(&m_destImage, &m_destImage);
    stretch.startFilterDirectly();
    m_destImage.putImageData(stretch.getTargetImage().bits());

    postProgress(90);

    if (!runningFlag())
    {
        return;
    }

    // -- Inverting image color -----------------------------------------------

    InvertFilter invert(&m_destImage);
    invert.startFilterDirectly();
    m_destImage.putImageData(invert.getTargetImage().bits());

    postProgress(95);

    if (!runningFlag())
    {
        return;
    }

    // -- Convert to neutral black & white ------------------------------------

    MixerContainer settings;
    settings.bMonochrome    = true;
    settings.blackRedGain   = 0.3;
    settings.blackGreenGain = 0.59;
    settings.blackBlueGain  = 0.11;
    MixerFilter mixer(&m_destImage, 0L, settings);
    mixer.startFilterDirectly();
    m_destImage.putImageData(mixer.getTargetImage().bits());

    postProgress(100);

    if (!runningFlag())
    {
        return;
    }
}

bool CharcoalFilter::convolveImage(const unsigned int order, const double* kernel)
{
    long kernelWidth = order;

    if ((kernelWidth % 2) == 0)
    {
        kWarning() << "Kernel width must be an odd number!";
        return false;
    }

    uint    x, y;
    long    i;
    int     mx, my, sx, sy, mcx, mcy, progress;
    double  red, green, blue, alpha, normalize = 0.0;
    double* k = 0;

    QScopedArrayPointer<double> normal_kernel(new double[kernelWidth * kernelWidth]);

    if (!normal_kernel)
    {
        kWarning() << "Unable to allocate memory!";
        return false;
    }

    for (i = 0; i < (kernelWidth * kernelWidth); ++i)
    {
        normalize += kernel[i];
    }

    if (fabs(normalize) <= Epsilon)
    {
        normalize = 1.0;
    }

    normalize = 1.0 / normalize;

    for (i = 0; i < (kernelWidth * kernelWidth); ++i)
    {
        normal_kernel[i] = normalize * kernel[i];
    }

    // --------------------------------------------------------

    // caching
    uint height     = m_destImage.height();
    uint width      = m_destImage.width();
    bool sixteenBit = m_destImage.sixteenBit();
    uchar* ddata    = m_destImage.bits();
    int ddepth      = m_destImage.bytesDepth();

    uchar* sdata    = m_orgImage.bits();
    int sdepth      = m_orgImage.bytesDepth();

    double maxClamp = m_destImage.sixteenBit() ? 16777215.0 : 65535.0;

    // --------------------------------------------------------

    for (y = 0; runningFlag() && (y < height); ++y)
    {
        sy = y - (kernelWidth / 2);

        for (x = 0; runningFlag() && (x < width); ++x)
        {
            k = normal_kernel.data();
            red = green = blue = alpha = 0;
            sy = y - (kernelWidth / 2);

            for (mcy = 0; runningFlag() && (mcy < kernelWidth); ++mcy, ++sy)
            {
                my = sy < 0 ? 0 : sy > (int) height - 1 ? height - 1 : sy;
                sx = x + (-kernelWidth / 2);

                for (mcx = 0; runningFlag() && (mcx < kernelWidth); ++mcx, ++sx)
                {
                    mx = sx < 0 ? 0 : sx > (int) width - 1 ? width - 1 : sx;
                    DColor color(sdata + mx * sdepth + (width * my * sdepth), sixteenBit);
                    red += (*k) * (color.red() * 257.0);
                    green += (*k) * (color.green() * 257.0);
                    blue += (*k) * (color.blue() * 257.0);
                    alpha += (*k) * (color.alpha() * 257.0);
                    ++k;
                }
            }

            red   =   red < 0.0 ? 0.0 :   red > maxClamp ? maxClamp :   red + 0.5;
            green = green < 0.0 ? 0.0 : green > maxClamp ? maxClamp : green + 0.5;
            blue  =  blue < 0.0 ? 0.0 :  blue > maxClamp ? maxClamp :  blue + 0.5;
            alpha = alpha < 0.0 ? 0.0 : alpha > maxClamp ? maxClamp : alpha + 0.5;

            DColor color((int)(red / 257UL), (int)(green / 257UL),
                         (int)(blue / 257UL), (int)(alpha / 257UL), sixteenBit);
            color.setPixel((ddata + x * ddepth + (width * y * ddepth)));
        }

        progress = (int)(((double) y * 80.0) / height);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
    return true;
}

int CharcoalFilter::getOptimalKernelWidth(double radius, double sigma)
{
    double        normalize, value;
    long          kernelWidth;
    register long u;

    if (radius > 0.0)
    {
        return((int)(2.0 * ceil(radius) + 1.0));
    }

    for (kernelWidth = 5; ;)
    {
        normalize = 0.0;

        for (u = (-kernelWidth / 2) ; u <= (kernelWidth / 2) ; ++u)
        {
            normalize += exp(-((double) u * u) / (2.0 * sigma * sigma)) / (SQ2PI * sigma);
        }

        u     = kernelWidth / 2;
        value = exp(-((double) u * u) / (2.0 * sigma * sigma)) / (SQ2PI * sigma) / normalize;

        if ((long)(65535 * value) <= 0)
        {
            break;
        }

        kernelWidth += 2;
    }

    return((int)kernelWidth - 2);
}

FilterAction CharcoalFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter("pencil", m_pencil);
    action.addParameter("smooth", m_smooth);

    return action;
}

void CharcoalFilter::readParameters(const Digikam::FilterAction& action)
{
    m_pencil = action.parameter("pencil").toDouble();
    m_smooth = action.parameter("smooth").toDouble();
}


}  // namespace Digikam
