/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Sharpen threaded image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Matthias Welwarsky <matze at welwarsky dot de>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "unsharpmaskfilter.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "dcolor.h"
#include "blurfilter.h"

namespace Digikam
{

UnsharpMaskFilter::UnsharpMaskFilter(QObject* parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

UnsharpMaskFilter::UnsharpMaskFilter(DImg* orgImage, QObject* parent, int radius,
                                     double amount, double threshold)
    : DImgThreadedFilter(orgImage, parent, "UnsharpMask")
{
    m_radius    = radius;
    m_amount    = amount;
    m_threshold = threshold;
    initFilter();
}

UnsharpMaskFilter::~UnsharpMaskFilter()
{
    cancelFilter();
}

void UnsharpMaskFilter::filterImage()
{
    int      progress;
    long int quantum;
    long int zero = 0;
    double   quantumThreshold;
    double   value;
    DColor   p;
    DColor   q;

    if (m_orgImage.isNull())
    {
        kWarning() << "No image data available!";
        return;
    }

    BlurFilter(this, m_orgImage, m_destImage, 0, 10, (int)(m_radius));

    quantum          = m_destImage.sixteenBit() ? 65535 : 255;
    quantumThreshold = quantum * m_threshold;

    for (uint y = 0 ; runningFlag() && (y < m_destImage.height()) ; ++y)
    {
        for (uint x = 0 ; runningFlag() && (x < m_destImage.width()) ; ++x)
        {
            p = m_orgImage.getPixelColor(x, y);
            q = m_destImage.getPixelColor(x, y);

            // Red channel.
            value = (double)(p.red()) - (double)(q.red());

            if (fabs(2.0 * value) < quantumThreshold)
            {
                value = (double)(p.red());
            }
            else
            {
                value = (double)(p.red()) + value * m_amount;
            }

            q.setRed(CLAMP(lround(value), zero, quantum));

            // Green Channel.
            value = (double)(p.green()) - (double)(q.green());

            if (fabs(2.0 * value) < quantumThreshold)
            {
                value = (double)(p.green());
            }
            else
            {
                value = (double)(p.green()) + value * m_amount;
            }

            q.setGreen(CLAMP(lround(value), zero, quantum));

            // Blue Channel.
            value = (double)(p.blue()) - (double)(q.blue());

            if (fabs(2.0 * value) < quantumThreshold)
            {
                value = (double)(p.blue());
            }
            else
            {
                value = (double)(p.blue()) + value * m_amount;
            }

            q.setBlue(CLAMP(lround(value), zero, quantum));

            // Alpha Channel.
            value = (double)(p.alpha()) - (double)(q.alpha());

            if (fabs(2.0 * value) < quantumThreshold)
            {
                value = (double)(p.alpha());
            }
            else
            {
                value = (double)(p.alpha()) + value * m_amount;
            }

            q.setAlpha(CLAMP(lround(value), zero, quantum));

            m_destImage.setPixelColor(x, y, q);
        }

        progress = (int)(10.0 + ((double)y * 90.0) / m_destImage.height());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

FilterAction UnsharpMaskFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter("amount", m_amount);
    action.addParameter("radius", m_radius);
    action.addParameter("threshold", m_threshold);

    return action;
}

void UnsharpMaskFilter::readParameters(const FilterAction& action)
{
    m_amount = action.parameter("amount").toDouble();
    m_radius = action.parameter("radius").toInt();
    m_threshold = action.parameter("threshold").toDouble();
}

} // namespace Digikam
