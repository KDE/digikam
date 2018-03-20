/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Sharpen threaded image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * Original Sharpen algorithm copyright 2002
 * by Daniel M. Duley <mosfet@kde.org> from KImageEffect API.
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

#include "sharpenfilter.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QtConcurrent>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

SharpenFilter::SharpenFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    m_radius = 0.0;
    m_sigma  = 1.0;

    initFilter();
}

SharpenFilter::SharpenFilter(DImg* const orgImage, QObject* const parent, double radius, double sigma)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("Sharpen"))
{
    m_radius = radius;
    m_sigma  = sigma;
    initFilter();
}

SharpenFilter::SharpenFilter(DImgThreadedFilter* const parentFilter,
                             const DImg& orgImage, const DImg& destImage,
                             int progressBegin, int progressEnd, double radius, double sigma)
    : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                         parentFilter->filterName() + QLatin1String(": Sharpen"))
{
    m_radius = radius;
    m_sigma  = sigma;

    // We need to provide support for orgImage == destImage.
    // The algorithm does not support this out of the box, so use a temporary.
    if (orgImage.bits() == destImage.bits())
    {
        m_destImage = DImg(destImage.width(), destImage.height(), destImage.sixteenBit());
    }

    filterImage();

    if (orgImage.bits() == destImage.bits())
    {
        memcpy(destImage.bits(), m_destImage.bits(), m_destImage.numBytes());
    }
}

SharpenFilter::~SharpenFilter()
{
    cancelFilter();
}

void SharpenFilter::filterImage()
{
    sharpenImage(m_radius, m_sigma);
}

/** Function to apply the sharpen filter on an image*/

void SharpenFilter::sharpenImage(double radius, double sigma)
{
    if (m_orgImage.isNull())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "No image data available!";
        return;
    }

    if (radius <= 0.0)
    {
        m_destImage = m_orgImage;
        return;
    }

    double        alpha, normalize = 0.0;
   long i = 0, u, v;

    int kernelWidth     = getOptimalKernelWidth(radius, sigma);
    int halfKernelWidth = kernelWidth / 2;

    if ((int)m_orgImage.width() < kernelWidth)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Image is smaller than radius!";
        return;
    }

    QScopedArrayPointer<double> kernel(new double[kernelWidth * kernelWidth]);

    if (kernel.isNull())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Unable to allocate memory!";
        return;
    }

    for (v = -halfKernelWidth; v <= halfKernelWidth; ++v)
    {
        for (u = -halfKernelWidth; u <= halfKernelWidth; ++u)
        {
            alpha      = exp(-((double) u * u + v * v) / (2.0 * sigma * sigma));
            kernel[i]  = alpha / (2.0 * M_PI * sigma * sigma);
            normalize += kernel[i];
            ++i;
        }
    }

    kernel[i / 2] = (-2.0) * normalize;
    convolveImage(kernelWidth, kernel.data());
}

void SharpenFilter::convolveImageMultithreaded(const Args& prm)
{
    double  maxClamp = m_destImage.sixteenBit() ? 16777215.0 : 65535.0;
    double* k        = 0;
    double  red, green, blue, alpha;
    int     mx, my, sx, sy, mcx, mcy;
    DColor  color;

    for (uint x = prm.start ; runningFlag() && (x < prm.stop) ; ++x)
    {
        k   = prm.normal_kernel;
        red = green = blue = alpha = 0;
        sy  = prm.y - prm.halfKernelWidth;

        for (mcy = 0 ; runningFlag() && (mcy < prm.kernelWidth) ; ++mcy, ++sy)
        {
            my = sy < 0 ? 0 : sy > (int)m_destImage.height() - 1 ? m_destImage.height() - 1 : sy;
            sx = x + (-prm.halfKernelWidth);

            for (mcx = 0 ; runningFlag() && (mcx < prm.kernelWidth) ; ++mcx, ++sx)
            {
                mx     = sx < 0 ? 0 : sx > (int)m_destImage.width() - 1 ? m_destImage.width() - 1 : sx;
                color  = m_orgImage.getPixelColor(mx, my);
                red   += (*k) * (color.red()   * 257.0);
                green += (*k) * (color.green() * 257.0);
                blue  += (*k) * (color.blue()  * 257.0);
                alpha += (*k) * (color.alpha() * 257.0);
                ++k;
            }
        }

        red   =   red < 0.0 ? 0.0 :   red > maxClamp ? maxClamp :   red + 0.5;
        green = green < 0.0 ? 0.0 : green > maxClamp ? maxClamp : green + 0.5;
        blue  =  blue < 0.0 ? 0.0 :  blue > maxClamp ? maxClamp :  blue + 0.5;
        alpha = alpha < 0.0 ? 0.0 : alpha > maxClamp ? maxClamp : alpha + 0.5;

        m_destImage.setPixelColor(x, prm.y, DColor((int)(red  / 257UL), (int)(green / 257UL),
                                                   (int)(blue / 257UL), (int)(alpha / 257UL),
                                                   m_destImage.sixteenBit()));
    }
}

bool SharpenFilter::convolveImage(const unsigned int order, const double* const kernel)
{
    uint    y;
    int     progress;
    long    i;
    double  normalize = 0.0;

    Args prm;
    prm.kernelWidth     = order;
    prm.halfKernelWidth = prm.kernelWidth / 2;;

    if ((prm.kernelWidth % 2) == 0)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Kernel width must be an odd number!";
        return false;
    }

    QScopedArrayPointer<double> normal_kernel(new double[prm.kernelWidth * prm.kernelWidth]);

    if (normal_kernel.isNull())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Unable to allocate memory!";
        return false;
    }

    for (i = 0 ; i < (prm.kernelWidth * prm.kernelWidth) ; ++i)
    {
        normalize += kernel[i];
    }

    if (fabs(normalize) <= Epsilon)
    {
        normalize = 1.0;
    }

    normalize = 1.0 / normalize;

    for (i = 0 ; i < (prm.kernelWidth * prm.kernelWidth) ; ++i)
    {
        normal_kernel[i] = normalize * kernel[i];
    }

    prm.normal_kernel = normal_kernel.data();
    QList<int> vals = multithreadedSteps(m_destImage.width());

    for (y = 0 ; runningFlag() && (y < m_destImage.height()) ; ++y)
    {
        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.y     = y;

            tasks.append(QtConcurrent::run(this,
                                           &SharpenFilter::convolveImageMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        progress = (int)(((double)y * 100.0) / m_destImage.height());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }

    return true;
}

int SharpenFilter::getOptimalKernelWidth(double radius, double sigma)
{
    double        normalize, value;
    long          kernelWidth;
   long u;

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

    return ((int)kernelWidth - 2);
}

FilterAction SharpenFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("radius"), m_radius);
    action.addParameter(QLatin1String("sigma"),  m_sigma);

    return action;
}

void SharpenFilter::readParameters(const Digikam::FilterAction& action)
{
    m_radius = action.parameter(QLatin1String("radius")).toDouble();
    m_sigma  = action.parameter(QLatin1String("sigma")).toDouble();
}

}  // namespace Digikam
