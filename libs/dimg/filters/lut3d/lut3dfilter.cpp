/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-10-10
 * Description : Lut3D color adjustment tool.
 *
 * Copyright (C) 2015 by Andrej Krutak <dev at andree dot sk>
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

#include "lut3dfilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// Qt includes

#include <QFileInfo>
#include <QImage>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"

namespace Digikam
{

Lut3DFilter::Lut3DFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

Lut3DFilter::Lut3DFilter(DImg* const orgImage,
                         const Lut3DContainer& par,
                         QObject* const parent)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("Lut3DFilter")),
      m_intensity(par.intensity)
{
    initFilter();

    loadLut3D(par.path);
}

Lut3DFilter::~Lut3DFilter()
{
    if (m_lutTable != NULL)
    {
        delete [] m_lutTable;
    }

    cancelFilter();
}

void Lut3DFilter::filterImage()
{
    applyLut3D();
    m_destImage = m_orgImage;
}

#define Lut3DSetPixel(table, w, x, y, p) \
    table[((y) * (w) + (x)) * 4 + 0] = qRed(p)   * 65535 / 255; \
    table[((y) * (w) + (x)) * 4 + 1] = qGreen(p) * 65535 / 255; \
    table[((y) * (w) + (x)) * 4 + 2] = qBlue(p)  * 65535 / 255;

void Lut3DFilter::loadLut3D(const QString& path)
{
    QFileInfo fi(path);

    m_lutTable = NULL;

    if (fi.suffix().toLower() == QLatin1String("cube"))
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Can't yet process Cube LUTs";
        // TODO: Adobe Cube LUT http://wwwimages.adobe.com/content/dam/Adobe/en/products/speedgrade/cc/pdfs/cube-lut-specification-1.0.pdf
    }
    else
    {
        QImage img(path);

        if (img.isNull())
        {
            qCDebug(DIGIKAM_DIMG_LOG) << "Format of " << path << " unknown!";
            return;
        }

        if (img.width() == img.height())
        {
            // HALD LUT (Like imagemagick creates)
            int w = img.width();

            m_lutTableSize = int(pow(pow(w, 1 / 3.0), 2) + 0.1);

            if ((w / m_lutTableSize) * m_lutTableSize != w)
            {
                qCDebug(DIGIKAM_DIMG_LOG) << "Format of " << path << " unknown (bad dimensions)!";
                return;
            }

            /* Rearrange the square-table to SxS rectangles, arranged alongside:
             *
             * Input:
             * 11 12 13 14 15 16 17 18
             * 21 22 23 24 25 26 27 28
             * 31 32 33 34 ...
             *
             * Output:
             * 11 12 13 14|31 32 33 34|...
             * 15 16 17 18|...
             * 21 22 23 24|
             * 25 26 27 28|
             */

            m_lutTable = new quint16[(m_lutTableSize * m_lutTableSize) * m_lutTableSize * 4];

            int x;
            int xOff;
            int oz, y1, y2;
            int iRow        = 0, iCol;
            const int zSize = w / m_lutTableSize;

            for (oz = 0; oz < m_lutTableSize; oz++)
            {
                xOff = oz * m_lutTableSize;

                for (y1 = 0; y1 < zSize; y1++)
                {
                    iCol = 0;

                    for (y2 = 0; y2 < zSize; y2++)
                    {
                        for (x = 0; x < m_lutTableSize; x++, iCol++)
                        {
                            QRgb p = img.pixel(iCol, iRow);
                            Lut3DSetPixel(m_lutTable,
                                          m_lutTableSize * m_lutTableSize,
                                          xOff + x,
                                          y1 * zSize + y2,
                                          p);
                        }
                    }

                    iRow++;
                }
            }
        }
        else if (img.width() / img.height() == img.height())
        {
            int x, y, w;
            // LUT (like Android's Gallery2 uses)
            m_lutTableSize = img.height();
            m_lutTable     = new quint16[img.width() * img.height() * 4];
            w              = img.width();

            for (y = 0; y < m_lutTableSize; y++)
            {
                for (x = 0; x < w; x++)
                {
                    QRgb p = img.pixel(x, y);
                    Lut3DSetPixel(m_lutTable, w, x, y, p);
                }
            }
        }
        else
        {
            qCDebug(DIGIKAM_DIMG_LOG) << "Format of " << path << " unknown (bad dimensions)!";
            return;
        }
    }
}

/* TODO: using liblcms would be fancier... */
/* Tetrahedral interpolation, taken from AOSP Gallery2 app */
static __inline__ int interp(const quint16* src, int p, int* off ,float dr, float dg, float db)
{
    float fr00 = (src[p+off[0]])*(1-dr)+(src[p+off[1]])*dr;
    float fr01 = (src[p+off[2]])*(1-dr)+(src[p+off[3]])*dr;
    float fr10 = (src[p+off[4]])*(1-dr)+(src[p+off[5]])*dr;
    float fr11 = (src[p+off[6]])*(1-dr)+(src[p+off[7]])*dr;
    float frb0 = fr00 * (1-db)+fr01*db;
    float frb1 = fr10 * (1-db)+fr11*db;
    float frbg = frb0 * (1-dg)+frb1*dg;

    return (int)frbg;
}

#define unlikely(x)     __builtin_expect(!!(x), 0)
static __inline__ int clamp(int from, int maxVal)
{
    if (unlikely(from < 0))
        from = 0;
    else if (unlikely(from > 65535))
        from = 65535;

    if (maxVal == 65535)
        return from;
    else
        return from * maxVal / 65535;
}

template<typename T>
static void ImageFilterFx(const quint16* lutrgb, int lutTableSize,
                          T* rgb, uint start, uint end, int maxVal, int intensity)
{
    int lutdim_r  = lutTableSize;
    int lutdim_g  = lutTableSize;
    int lutdim_b  = lutTableSize;
    int STEP      = 4;
    const int RED = 2, GREEN = 1, BLUE = 0;

    int off[8] =
    {
            0,
            STEP*1,
            STEP*lutdim_r,
            STEP*(lutdim_r + 1),
            STEP*(lutdim_r*lutdim_b),
            STEP*(lutdim_r*lutdim_b+1),
            STEP*(lutdim_r*lutdim_b+lutdim_r),
            STEP*(lutdim_r*lutdim_b+lutdim_r + 1)
    };

    float scale_R = (lutdim_r-1.f)/(maxVal + 1);
    float scale_G = (lutdim_g-1.f)/(maxVal + 1);
    float scale_B = (lutdim_b-1.f)/(maxVal + 1);

    uint i;
    rgb += 4 * start;

    for (i = start; i < end; i++)
    {
        int r      = rgb[RED], rn;
        int g      = rgb[GREEN], gn;
        int b      = rgb[BLUE], bn;

        float fb   = b*scale_B;
        float fg   = g*scale_G;
        float fr   = r*scale_R;
        int lut_b  = (int)fb;
        int lut_g  = (int)fg;
        int lut_r  = (int)fr;
        int p      = lut_r+lut_b*lutdim_r+lut_g*lutdim_r*lutdim_b;
        p         *= STEP;
        float dr   = fr-lut_r;
        float dg   = fg-lut_g;
        float db   = fb-lut_b;
        rn         = clamp(interp(lutrgb,p  ,off,dr,dg,db), maxVal);
        gn         = clamp(interp(lutrgb,p+1,off,dr,dg,db), maxVal);
        bn         = clamp(interp(lutrgb,p+2,off,dr,dg,db), maxVal);

        rgb[RED]   = (T)(((100-intensity) * r + intensity * rn) / 100);
        rgb[GREEN] = (T)(((100-intensity) * g + intensity * gn) / 100);
        rgb[BLUE]  = (T)(((100-intensity) * b + intensity * bn) / 100);

        rgb       += 4;
    }
}

#define min(a, b) ((a) < (b) ? (a) : (b))
void Lut3DFilter::applyLut3D()
{
    uint i, stepI, maxI;
    int progress;
    const int steps = 10;

    if (!m_lutTable)
        return;

    maxI  = m_orgImage.width() * m_orgImage.height();
    stepI = maxI / steps;

    for (progress = 0, i = 0;
         runningFlag() && (i < maxI);
         i += stepI, progress += (100 / steps))
    {
        if (!m_orgImage.sixteenBit())
        {
            ImageFilterFx(m_lutTable, m_lutTableSize, m_orgImage.bits(),
                          i, min(i + stepI, maxI),
                          255, m_intensity);
        }
        else
        {
            ImageFilterFx(m_lutTable, m_lutTableSize, reinterpret_cast<unsigned short*>(m_orgImage.bits()),
                          i, min(i + stepI, maxI),
                          65535, m_intensity);
        }

        postProgress(progress);
    }
}

FilterAction Lut3DFilter::filterAction()
{
    return DefaultFilterAction<Lut3DFilter>();
}

void Lut3DFilter::readParameters(const FilterAction& /*action*/)
{
    return;
}

}  // namespace Digikam
