/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Sharpen threaded image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Matthias Welwarsky <matze at welwarsky dot de>
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

#include "unsharpmaskfilter.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QtConcurrent>

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "dcolor.h"
#include "blurfilter.h"

namespace Digikam
{

UnsharpMaskFilter::UnsharpMaskFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    m_radius    = 1;
    m_amount    = 1.0;
    m_threshold = 0.05;
    m_luma      = false;

    initFilter();
}

UnsharpMaskFilter::UnsharpMaskFilter(DImg* const orgImage, QObject* const parent, double radius,
                                     double amount, double threshold, bool luma)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("UnsharpMask"))
{
    m_radius    = radius;
    m_amount    = amount;
    m_threshold = threshold;
    m_luma      = luma;
    initFilter();
}

UnsharpMaskFilter::~UnsharpMaskFilter()
{
    cancelFilter();
}

void UnsharpMaskFilter::unsharpMaskMultithreaded(uint start, uint stop, uint y)
{
    long int zero  = 0;
    double   value = 0.0;
    DColor   p;
    DColor   q;

    long int quantum        = m_destImage.sixteenBit() ? 65535 : 255;
    double quantumThreshold = quantum * m_threshold;
    int hp = 0, sp = 0, lp = 0, hq = 0, sq = 0, lq = 0;

    for (uint x = start ; runningFlag() && (x < stop) ; ++x)
    {
        p = m_orgImage.getPixelColor(x, y);
        q = m_destImage.getPixelColor(x, y);

        if (m_luma)
        {
            p.getHSL(&hp, &sp, &lp);
            q.getHSL(&hq, &sq, &lq);

            //luma channel
            value = (double)(lp) - (double)(lq);

            if (fabs(2.0 * value) < quantumThreshold)
            {
                value = (double)(lp);
            }
            else
            {
                value = (double)(lp) + value * m_amount;
            }

            q.setHSL(hp, sp, CLAMP(lround(value), zero, quantum), m_destImage.sixteenBit());
            q.setAlpha(p.alpha());

        }
        else
        {
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
        }

        m_destImage.setPixelColor(x, y, q);
    }
}

void UnsharpMaskFilter::filterImage()
{
    int progress;

    if (m_orgImage.isNull())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "No image data available!";
        return;
    }

    BlurFilter(this, m_orgImage, m_destImage, 0, 10, (int)(m_radius*10.0));

    QList<int> vals = multithreadedSteps(m_destImage.width());

    for (uint y = 0 ; runningFlag() && (y < m_destImage.height()) ; ++y)
    {
        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            tasks.append(QtConcurrent::run(this,
                                           &UnsharpMaskFilter::unsharpMaskMultithreaded,
                                           vals[j],
                                           vals[j+1],
                                           y));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

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

    action.addParameter(QLatin1String("amount"),    m_amount);
    action.addParameter(QLatin1String("radius"),    m_radius);
    action.addParameter(QLatin1String("threshold"), m_threshold);
    action.addParameter(QLatin1String("luma"),      m_luma);

    return action;
}

void UnsharpMaskFilter::readParameters(const FilterAction& action)
{
    m_amount    = action.parameter(QLatin1String("amount")).toDouble();
    m_radius    = action.parameter(QLatin1String("radius")).toDouble();
    m_threshold = action.parameter(QLatin1String("threshold")).toDouble();
    m_luma      = action.parameter(QLatin1String("luma")).toBool();
}

} // namespace Digikam
