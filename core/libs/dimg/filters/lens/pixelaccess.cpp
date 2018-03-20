/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-27
 * Description : access pixels method for lens distortion algorithm.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "pixelaccess.h"

// C++ includes

#include <cstring>
#include <cmath>
#include <cstdlib>

namespace Digikam
{

PixelAccess::PixelAccess(DImg* srcImage)
{
    m_image       = srcImage;

    m_width       = PixelAccessWidth;
    m_height      = PixelAccessHeight;

    m_depth       = m_image->bytesDepth();
    m_imageWidth  = m_image->width();
    m_imageHeight = m_image->height();
    m_sixteenBit  = m_image->sixteenBit();

    for (int i = 0 ; i < PixelAccessRegions ; ++i)
    {
        m_buffer[i] = new DImg(m_image->copy(0, 0, m_width, m_height));

        m_tileMinX[i] = 1;
        m_tileMaxX[i] = m_width - 2;
        m_tileMinY[i] = 1;
        m_tileMaxY[i] = m_height - 2;
    }
}

PixelAccess::~PixelAccess()
{
    for (int i = 0 ; i < PixelAccessRegions ; ++i)
    {
        delete m_buffer[i];
    }
}

uchar* PixelAccess::pixelAccessAddress(int i, int j)
{
    return m_buffer[0]->bits() + m_depth * (m_width * (j + 1 - m_tileMinY[0]) + (i + 1 - m_tileMinX[0]));
}

// Swap region[n] with region[0].
void PixelAccess::pixelAccessSelectRegion(int n)
{
    DImg* temp;
    int    a, b, c, d;
    int    i;

    temp = m_buffer[n];
    a    = m_tileMinX[n];
    b    = m_tileMaxX[n];
    c    = m_tileMinY[n];
    d    = m_tileMaxY[n];

    for (i = n ; i > 0 ; --i)
    {
        m_buffer[i]   = m_buffer[i - 1];
        m_tileMinX[i] = m_tileMinX[i - 1];
        m_tileMaxX[i] = m_tileMaxX[i - 1];
        m_tileMinY[i] = m_tileMinY[i - 1];
        m_tileMaxY[i] = m_tileMaxY[i - 1];
    }

    m_buffer[0]   = temp;
    m_tileMinX[0] = a;
    m_tileMaxX[0] = b;
    m_tileMinY[0] = c;
    m_tileMaxY[0] = d;
}

// Buffer[0] is cleared, should start at [i, j], fill rows that overlap image.
void PixelAccess::pixelAccessDoEdge(int i, int j)
{
    int    lineStart, lineEnd;
    int    rowStart, rowEnd;
    int    lineWidth;
    uchar* line;

    lineStart = i;

    if (lineStart < 0)
    {
        lineStart = 0;
    }

    lineEnd = i + m_width;

    if (lineEnd > m_imageWidth)
    {
        lineEnd = m_imageWidth;
    }

    lineWidth = lineEnd - lineStart;

    if (lineStart >= lineEnd)
    {
        return;
    }

    rowStart = j;

    if (rowStart < 0)
    {
        rowStart = 0;
    }

    rowEnd = j + m_height;

    if (rowEnd > m_imageHeight)
    {
        rowEnd = m_imageHeight;
    }

    for (int y = rowStart ; y < rowEnd ; ++y)
    {
        line = pixelAccessAddress(lineStart, y);
        memcpy(line, m_image->scanLine(y) + lineStart * m_depth, lineWidth * m_depth);
    }
}

// Moves buffer[0] so that [x, y] is inside it.
void PixelAccess::pixelAccessReposition(int xInt, int yInt)
{
    int newStartX = xInt - PixelAccessXOffset;
    int newStartY = yInt - PixelAccessYOffset;

    m_tileMinX[0] = newStartX + 1;
    m_tileMaxX[0] = newStartX + m_width - 2;
    m_tileMinY[0] = newStartY + 1;
    m_tileMaxY[0] = newStartY + m_height - 2;


    if ((newStartX < 0) || ((newStartX + m_width) >= m_imageWidth) ||
        (newStartY < 0) || ((newStartY + m_height) >= m_imageHeight))
    {
        // some data is off edge of image

        m_buffer[0]->fill(DColor(0, 0, 0, 0, m_sixteenBit));

        // This could probably be done by bitBltImage but I did not figure out how,
        // so leave the working code here. And no, it is not this:
        //m_buffer[0]->bitBltImage(m_image, newStartX, newStartY, m_width, m_height, 0, 0);

        if (((newStartX + m_width) < 0) || (newStartX >= m_imageWidth) ||
            ((newStartY + m_height) < 0) || (newStartY >= m_imageHeight))
        {
            // totally outside, just leave it.
        }
        else
        {
            pixelAccessDoEdge(newStartX, newStartY);
        }
    }
    else
    {
        m_buffer[0]->bitBltImage(m_image, newStartX, newStartY, m_width, m_height, 0, 0);
    }
}

void PixelAccess::pixelAccessGetCubic(double srcX, double srcY, double brighten, uchar* dst)
{
    int    xInt, yInt;
    double dx, dy;
    uchar* corner;

    xInt = (int)floor(srcX);
    dx   = srcX - xInt;
    yInt = (int)floor(srcY);
    dy   = srcY - yInt;

    // We need 4x4 pixels, xInt-1 to xInt+2 horz, yInt-1 to yInt+2 vert
    // they're probably in the last place we looked...

    if ((xInt >= m_tileMinX[0]) && (xInt < m_tileMaxX[0]) &&
        (yInt >= m_tileMinY[0]) && (yInt < m_tileMaxY[0]))
    {
        corner = pixelAccessAddress(xInt - 1, yInt - 1);
        cubicInterpolate(corner, m_depth * m_width, dst, m_sixteenBit, dx, dy, brighten);
        return;
    }

    // Or maybe it was a while back...

    for (int i = 1 ; i < PixelAccessRegions ; ++i)
    {
        if ((xInt >= m_tileMinX[i]) && (xInt < m_tileMaxX[i]) &&
            (yInt >= m_tileMinY[i]) && (yInt < m_tileMaxY[i]))
        {
            // Check here first next time

            pixelAccessSelectRegion(i);
            corner = pixelAccessAddress(xInt - 1, yInt - 1);
            cubicInterpolate(corner, m_depth * m_width, dst, m_sixteenBit, dx, dy, brighten);
            return;
        }
    }

    // Nope, recycle an old region.

    pixelAccessSelectRegion(PixelAccessRegions - 1);
    pixelAccessReposition(xInt, yInt);

    corner = pixelAccessAddress(xInt - 1, yInt - 1);
    cubicInterpolate(corner, m_depth * m_width, dst, m_sixteenBit, dx, dy, brighten);
}

/*
 * Catmull-Rom cubic interpolation
 *
 * equally spaced points p0, p1, p2, p3
 * interpolate 0 <= u < 1 between p1 and p2
 *
 * (1 u u^2 u^3) (  0.0  1.0  0.0  0.0 ) (p0)
 *               ( -0.5  0.0  0.5  0.0 ) (p1)
 *               (  1.0 -2.5  2.0 -0.5 ) (p2)
 *               ( -0.5  1.5 -1.5  0.5 ) (p3)
 *
 */
void PixelAccess::cubicInterpolate(uchar* src, int rowStride, uchar* dst,
                                   bool sixteenBit, double dx, double dy, double brighten)
{
    float um1, u, up1, up2;
    float vm1, v, vp1, vp2;
    int   c;
    const int numberOfComponents = 4;
    float verts[4 * numberOfComponents];

    um1 = ((-0.5 * dx + 1.0) * dx - 0.5) * dx;
    u   = (1.5 * dx - 2.5) * dx * dx + 1.0;
    up1 = ((-1.5 * dx + 2.0) * dx + 0.5) * dx;
    up2 = (0.5 * dx - 0.5) * dx * dx;

    vm1 = ((-0.5 * dy + 1.0) * dy - 0.5) * dy;
    v   = (1.5 * dy - 2.5) * dy * dy + 1.0;
    vp1 = ((-1.5 * dy + 2.0) * dy + 0.5) * dy;
    vp2 = (0.5 * dy - 0.5) * dy * dy;

    if (sixteenBit)
    {
        unsigned short* src16 = reinterpret_cast<unsigned short*>(src);
        unsigned short* dst16 = reinterpret_cast<unsigned short*>(dst);

        // for each component, read the values of 4 pixels into array

        for (c = 0 ; c < 4 * numberOfComponents ; ++c)
        {
            verts[c] = vm1 * src16[c] + v * src16[c + rowStride] + vp1 * src16[c + rowStride * 2] + vp2 * src16[c + rowStride * 3];
        }

        // for each component, compute resulting value from array

        for (c = 0 ; c < numberOfComponents ; ++c)
        {
            float result;
            result = um1 * verts[c] + u * verts[c + numberOfComponents]
                     + up1 * verts[c + numberOfComponents * 2] + up2 * verts[c + numberOfComponents * 3];
            result *= brighten;

            if (result < 0.0)
            {
                dst16[c] = 0;
            }
            else if (result > 65535.0)
            {
                dst16[c] = 65535;
            }
            else
            {
                dst16[c] = (uint)result;
            }
        }
    }
    else
    {
        for (c = 0 ; c < 4 * numberOfComponents ; ++c)
        {
            verts[c] = vm1 * src[c] + v * src[c + rowStride] + vp1 * src[c + rowStride * 2] + vp2 * src[c + rowStride * 3];
        }

        for (c = 0 ; c < numberOfComponents ; ++c)
        {
            float result;
            result = um1 * verts[c] + u * verts[c + numberOfComponents]
                     + up1 * verts[c + numberOfComponents * 2] + up2 * verts[c + numberOfComponents * 3];
            result *= brighten;

            if (result < 0.0)
            {
                dst[c] = 0;
            }
            else if (result > 255.0)
            {
                dst[c] = 255;
            }
            else
            {
                dst[c] = (uint)result;
            }
        }
    }
}

}  // namespace Digikam
