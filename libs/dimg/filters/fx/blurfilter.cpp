/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Gaussian Blur threaded image filter.
 *
 * Copyright (C) 2005-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "blurfilter.h"

// Qt includes

#include <qmath.h>
#include <QtConcurrentRun>
#include <QMutex>

// KDE includes

#include <kdebug.h>

namespace Digikam
{

class BlurFilter::Private
{
public:

    Private()
    {
        globalProgress = 0;
        radius         = 3;
    }

    int    radius;
    int    globalProgress;

    QMutex lock;
};

BlurFilter::BlurFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

BlurFilter::BlurFilter(DImg* const orgImage, QObject* const parent, int radius)
    : DImgThreadedFilter(orgImage, parent, "GaussianBlur"),
      d(new Private)
{
    d->radius = radius;
    initFilter();
}

BlurFilter::BlurFilter(DImgThreadedFilter* const parentFilter,
                       const DImg& orgImage, const DImg& destImage,
                       int progressBegin, int progressEnd, int radius)
    : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                         parentFilter->filterName() + ": GaussianBlur"),
      d(new Private)
{
    d->radius = radius;
    filterImage();
}

BlurFilter::~BlurFilter()
{
    cancelFilter();
    delete d;
}

void BlurFilter::blurMultithreaded(uint start, uint stop)
{
    int  oldProgress=0, progress=0;
    int  a, r, g, b;
    int  mx;
    int  my;
    int  mw;
    int  mh;
    int  mt;
    int* as = new int[m_orgImage.width()];
    int* rs = new int[m_orgImage.width()];
    int* gs = new int[m_orgImage.width()];
    int* bs = new int[m_orgImage.width()];

    for (uint y = start ; runningFlag() && (y < stop) ; ++y)
    {
        my = y - d->radius;
        mh = (d->radius << 1) + 1;

        if (my < 0)
        {
            mh += my;
            my = 0;
        }

        if ((my + mh) > (int)m_orgImage.height())
            mh = m_orgImage.height() - my;

        uchar* pDst8           = m_destImage.scanLine(y);
        unsigned short* pDst16 = reinterpret_cast<unsigned short*>(m_destImage.scanLine(y));

        memset(as, 0, m_orgImage.width() * sizeof(int));
        memset(rs, 0, m_orgImage.width() * sizeof(int));
        memset(gs, 0, m_orgImage.width() * sizeof(int));
        memset(bs, 0, m_orgImage.width() * sizeof(int));

        for (int yy = 0; yy < mh; yy++)
        {
            uchar* pSrc8           = m_orgImage.scanLine(yy + my);
            unsigned short* pSrc16 = reinterpret_cast<unsigned short*>(m_orgImage.scanLine(yy + my));

            for (int x = 0; x < (int)m_orgImage.width(); x++)
            {
                if (m_orgImage.sixteenBit())
                {  
                    bs[x]  += pSrc16[0];
                    gs[x]  += pSrc16[1];
                    rs[x]  += pSrc16[2];
                    as[x]  += pSrc16[3];
                    pSrc16 += 4;
                }
                else
                {  
                    bs[x] += pSrc8[0];
                    gs[x] += pSrc8[1];
                    rs[x] += pSrc8[2];
                    as[x] += pSrc8[3];
                    pSrc8 += 4;
                }
            }
        }

        if ((int)m_orgImage.width() > ((d->radius << 1) + 1))
        {
            for (int x = 0; x < (int)m_orgImage.width(); x++)
            {
                a  = r = g = b = 0;
                mx = x - d->radius;
                mw = (d->radius << 1) + 1;

                if (mx < 0)
                {
                    mw += mx;
                    mx = 0;
                }

                if ((mx + mw) > (int)m_orgImage.width())
                    mw = m_orgImage.width() - mx;

                mt = mw * mh;

                for (int xx = mx; xx < (mw + mx); xx++)
                {
                    a += as[xx];
                    r += rs[xx];
                    g += gs[xx];
                    b += bs[xx];
                }

                a = a / mt;
                r = r / mt;
                g = g / mt;
                b = b / mt;

                if (m_orgImage.sixteenBit())
                {
                    pDst16[0] = b;
                    pDst16[1] = g;
                    pDst16[2] = r;
                    pDst16[3] = a;
                    pDst16   += 4;
                }
                else
                {
                    pDst8[0] = b;
                    pDst8[1] = g;
                    pDst8[2] = r;
                    pDst8[3] = a;
                    pDst8   += 4;
                }
            }
        }
        else
        {
            kDebug() << "Radius too small..."; 
        }

        progress = (int)( ( (double)y * (100.0 / QThreadPool::globalInstance()->maxThreadCount()) ) / (stop-start));

        if ((progress % 5 == 0) && (progress > oldProgress))
        {
            d->lock.lock();
            oldProgress       = progress;
            d->globalProgress += 5;
            postProgress(d->globalProgress);
            d->lock.unlock();
        }
    }

    delete [] as;
    delete [] rs;
    delete [] gs;
    delete [] bs;
}

void BlurFilter::filterImage()
{
    if (d->radius < 1)
    {
        kDebug() << "Radius out of range..."; 
        return;
    }

    QList<int> vals = multithreadedSteps(m_orgImage.height());
    QList <QFuture<void> > tasks;

    for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
    {
        tasks.append(QtConcurrent::run(this,
                                       &BlurFilter::blurMultithreaded,
                                       vals[j],
                                       vals[j+1]
                                      ));
    }

    foreach(QFuture<void> t, tasks)
        t.waitForFinished();
}

FilterAction BlurFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter("radius", d->radius);

    return action;
}

void BlurFilter::readParameters(const FilterAction& action)
{
    d->radius = action.parameter("radius").toInt();
}

}  // namespace Digikam
