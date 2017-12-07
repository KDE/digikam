/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Oil Painting threaded image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * Original OilPaint algorithm copyrighted 2004 by
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
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

#include "oilpaintfilter.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QtConcurrent>
#include <QMutex>

// Local includes

#include "dimg.h"

namespace Digikam
{

class OilPaintFilter::Private
{
public:

    Private() :
        brushSize(1),
        smoothness(30),
        globalProgress(0)
    {
    }

    int    brushSize;
    int    smoothness;

    int    globalProgress;

    QMutex lock;
};

OilPaintFilter::OilPaintFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

OilPaintFilter::OilPaintFilter(DImg* const orgImage, QObject* const parent, int brushSize, int smoothness)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("OilPaintFilter")),
      d(new Private)
{
    d->brushSize  = brushSize;
    d->smoothness = smoothness;

    initFilter();
}

OilPaintFilter::~OilPaintFilter()
{
    cancelFilter();
    delete d;
}

/** Function to apply the OilPaintFilter effect.
 *  This method have been ported from Pieter Z. Voloshyn algorithm code.
 *
 *  Theory: Using MostFrequentColor function we take the main color in
 *          a matrix and simply write at the original position.
 */
void OilPaintFilter::oilPaintImageMultithreaded(uint start, uint stop)
{
    QScopedPointer<uchar> intensityCount(new uchar[d->smoothness + 1]);
    QScopedPointer<uint>  averageColorR(new  uint[d->smoothness + 1]);
    QScopedPointer<uint>  averageColorG(new  uint[d->smoothness + 1]);
    QScopedPointer<uint>  averageColorB(new  uint[d->smoothness + 1]);

    memset(intensityCount.data(), 0, sizeof(uchar)*(d->smoothness + 1));
    memset(averageColorR.data(),  0, sizeof(uint)*(d->smoothness + 1));
    memset(averageColorG.data(),  0, sizeof(uint)*(d->smoothness + 1));
    memset(averageColorB.data(),  0, sizeof(uint)*(d->smoothness + 1));

    int    oldProgress=0, progress=0;
    DColor mostFrequentColor;

    mostFrequentColor.setSixteenBit(m_orgImage.sixteenBit());
    uchar* dest    = m_destImage.bits();
    uchar* dptr    = 0;

    for (uint h2 = start; runningFlag() && (h2 < stop); ++h2)
    {
        for (uint w2 = 0; runningFlag() && (w2 < m_orgImage.width()); ++w2)
        {
            mostFrequentColor = MostFrequentColor(m_orgImage, w2, h2, d->brushSize, d->smoothness,
                                                  intensityCount.data(), averageColorR.data(), averageColorG.data(), averageColorB.data());
            dptr              = dest + w2 * m_orgImage.bytesDepth() + (m_orgImage.width() * h2 * m_orgImage.bytesDepth());
            mostFrequentColor.setPixel(dptr);
        }

        progress = (int)( ( (double)h2 * (100.0 / QThreadPool::globalInstance()->maxThreadCount()) ) / (stop-start));

        if ((progress % 5 == 0) && (progress > oldProgress))
        {
            d->lock.lock();
            oldProgress       = progress;
            d->globalProgress += 5;
            postProgress(d->globalProgress);
            d->lock.unlock();
        }
    }
}

void OilPaintFilter::filterImage()
{
    QList<int> vals = multithreadedSteps(m_orgImage.height());
    QList <QFuture<void> > tasks;

    for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
    {
        tasks.append(QtConcurrent::run(this,
                                       &OilPaintFilter::oilPaintImageMultithreaded,
                                       vals[j],
                                       vals[j+1]
                                      ));
    }

    foreach(QFuture<void> t, tasks)
        t.waitForFinished();
}

/** Function to determine the most frequent color in a matrix
 *
 * Bits             => Bits array
 * Width            => Image width
 * Height           => Image height
 * X                => Position horizontal
 * Y                => Position vertical
 * Radius           => Is the radius of the matrix to be analyzed
 * Intensity        => Intensity to calculate
 *
 * Theory           => This function creates a matrix with the analyzed pixel in
 *                     the center of this matrix and find the most frequently color
 */
DColor OilPaintFilter::MostFrequentColor(DImg& src, int X, int Y, int Radius, int Intensity,
                                         uchar* intensityCount, uint* averageColorR, uint* averageColorG, uint* averageColorB)
{
    int  i, w, h, I, Width, Height;
    uint red, green, blue;

    uchar* dest     = src.bits();
    int bytesDepth  = src.bytesDepth();
    uchar* sptr     = 0;
    bool sixteenBit = src.sixteenBit();

    DColor mostFrequentColor;

    double Scale = Intensity / (sixteenBit ? 65535.0 : 255.0);
    Width        = (int)src.width();
    Height       = (int)src.height();

    // Erase the array
    memset(intensityCount, 0, (Intensity + 1) * sizeof(uchar));

    for (w = X - Radius; w <= X + Radius; ++w)
    {
        for (h = Y - Radius; h <= Y + Radius; ++h)
        {
            // This condition helps to identify when a point doesn't exist

            if ((w >= 0) && (w < Width) && (h >= 0) && (h < Height))
            {
                sptr          = dest + w * bytesDepth + (Width * h * bytesDepth);
                DColor color(sptr, sixteenBit);
                red           = (uint)color.red();
                green         = (uint)color.green();
                blue          = (uint)color.blue();

                I = lround(GetIntensity(red, green, blue) * Scale);
                intensityCount[I]++;

                if (intensityCount[I] == 1)
                {
                    averageColorR[I] = red;
                    averageColorG[I] = green;
                    averageColorB[I] = blue;
                }
                else
                {
                    averageColorR[I] += red;
                    averageColorG[I] += green;
                    averageColorB[I] += blue;
                }
            }
        }
    }

    I               = 0;
    int MaxInstance = 1;

    for (i = 0 ; i <= Intensity ; ++i)
    {
        if (intensityCount[i] > MaxInstance)
        {
            I = i;
            MaxInstance = intensityCount[i];
        }
    }

    // get Alpha channel value from original (unchanged)
    mostFrequentColor = src.getPixelColor(X, Y);

    // Overwrite RGB values to destination.
    mostFrequentColor.setRed(averageColorR[I]   / MaxInstance);
    mostFrequentColor.setGreen(averageColorG[I] / MaxInstance);
    mostFrequentColor.setBlue(averageColorB[I]  / MaxInstance);

    return mostFrequentColor;
}

/** Function to calculate the color intensity and return the luminance (Y)
  * component of YIQ color model.
  */
double OilPaintFilter::GetIntensity(uint Red, uint Green, uint Blue)
{
    return Red * 0.3 + Green * 0.59 + Blue * 0.11;
}

FilterAction OilPaintFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("brushSize"),  d->brushSize);
    action.addParameter(QLatin1String("smoothness"), d->smoothness);

    return action;
}

void OilPaintFilter::readParameters(const Digikam::FilterAction& action)
{
    d->brushSize  = action.parameter(QLatin1String("brushSize")).toInt();
    d->smoothness = action.parameter(QLatin1String("smoothness")).toInt();
}

}  // namespace Digikam
