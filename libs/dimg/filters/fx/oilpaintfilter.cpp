/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Oil Painting threaded image filter.
 *
 * Copyright (C) 2005-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "dimg.h"

namespace Digikam
{

OilPaintFilter::OilPaintFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    m_brushSize      = 1;
    m_smoothness     = 30;
    m_intensityCount = 0;
    m_averageColorR  = 0;
    m_averageColorG  = 0;
    m_averageColorB  = 0;

    initFilter();
}

OilPaintFilter::OilPaintFilter(DImg* const orgImage, QObject* const parent, int brushSize, int smoothness)
    : DImgThreadedFilter(orgImage, parent, "OilPaintFilter")
{
    m_brushSize      = brushSize;
    m_smoothness     = smoothness;
    m_intensityCount = 0;
    m_averageColorR  = 0;
    m_averageColorG  = 0;
    m_averageColorB  = 0;

    initFilter();
}

OilPaintFilter::~OilPaintFilter()
{
    cancelFilter();
}

/** Function to apply the OilPaintFilter effect.
 *  This method have been ported from Pieter Z. Voloshyn algorithm code.
 *
 *  Theory           => Using MostFrequentColor function we take the main color in
 *                      a matrix and simply write at the original position.
 */
void OilPaintFilter::filterImage()
{
    int    progress;
    DColor mostFrequentColor;
    int    w, h;

    mostFrequentColor.setSixteenBit(m_orgImage.sixteenBit());
    w              = (int)m_orgImage.width();
    h              = (int)m_orgImage.height();
    uchar* dest    = m_destImage.bits();
    int bytesDepth = m_orgImage.bytesDepth();
    uchar* dptr    = 0;

    // Allocate some arrays to be used.
    // Do this here once for all to save a few million new / delete operations
    m_intensityCount = new uchar[m_smoothness + 1];
    m_averageColorR  = new uint[m_smoothness + 1];
    m_averageColorG  = new uint[m_smoothness + 1];
    m_averageColorB  = new uint[m_smoothness + 1];

    for (int h2 = 0; runningFlag() && (h2 < h); ++h2)
    {
        for (int w2 = 0; runningFlag() && (w2 < w); ++w2)
        {
            mostFrequentColor = MostFrequentColor(m_orgImage, w2, h2, m_brushSize, m_smoothness);
            dptr              = dest + w2 * bytesDepth + (w * h2 * bytesDepth);
            mostFrequentColor.setPixel(dptr);
        }

        progress = (int)(((double)h2 * 100.0) / h);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }

    // free all the arrays
    delete [] m_intensityCount;
    delete [] m_averageColorR;
    delete [] m_averageColorG;
    delete [] m_averageColorB;
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
DColor OilPaintFilter::MostFrequentColor(DImg& src, int X, int Y, int Radius, int Intensity)
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
    memset(m_intensityCount, 0, (Intensity + 1) * sizeof(uchar));

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
                m_intensityCount[I]++;

                if (m_intensityCount[I] == 1)
                {
                    m_averageColorR[I] = red;
                    m_averageColorG[I] = green;
                    m_averageColorB[I] = blue;
                }
                else
                {
                    m_averageColorR[I] += red;
                    m_averageColorG[I] += green;
                    m_averageColorB[I] += blue;
                }
            }
        }
    }

    I = 0;
    int MaxInstance = 0;

    for (i = 0 ; i <= Intensity ; ++i)
    {
        if (m_intensityCount[i] > MaxInstance)
        {
            I = i;
            MaxInstance = m_intensityCount[i];
        }
    }

    // get Alpha channel value from original (unchanged)
    mostFrequentColor = src.getPixelColor(X, Y);

    // Overwrite RGB values to destination.
    mostFrequentColor.setRed(m_averageColorR[I]   / MaxInstance);
    mostFrequentColor.setGreen(m_averageColorG[I] / MaxInstance);
    mostFrequentColor.setBlue(m_averageColorB[I]  / MaxInstance);

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

    action.addParameter("brushSize", m_brushSize);
    action.addParameter("smoothness", m_smoothness);

    return action;
}

void OilPaintFilter::readParameters(const Digikam::FilterAction& action)
{
    m_brushSize  = action.parameter("brushSize").toInt();
    m_smoothness = action.parameter("smoothness").toInt();
}

}  // namespace Digikam
