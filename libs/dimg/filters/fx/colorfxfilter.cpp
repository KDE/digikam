/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : Color FX threaded image filter.
 *
 * Copyright 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 * Copyright 2015      by Andrej Krutak <dev at andree dot sk>
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

#include "colorfxfilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// Qt includes

#include <QFileInfo>
#include <QImage>
#include <QtMath>

// Local includes

#include "digikam_debug.h"
#include "curvesfilter.h"
#include "mixerfilter.h"
#include "dimg.h"

namespace Digikam
{

ColorFXFilter::ColorFXFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      m_lutTable(0),
      m_lutTableSize(0)
{
    initFilter();
}

ColorFXFilter::ColorFXFilter(DImg* const orgImage, QObject* const parent, const ColorFXContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("ColorFX")),
      m_lutTable(0),
      m_lutTableSize(0)
{
    m_settings = settings;

    loadLut3D(m_settings.path);

    initFilter();
}

ColorFXFilter::~ColorFXFilter()
{
    cancelFilter();

    if (m_lutTable)
    {
        delete [] m_lutTable;
    }
}

void ColorFXFilter::filterImage()
{
    switch (m_settings.colorFXType)
    {
        case Solarize:
            solarize(&m_orgImage, &m_destImage, m_settings.level);
            break;

        case Vivid:
            vivid(&m_orgImage, &m_destImage, m_settings.level);
            break;

        case Neon:
            neon(&m_orgImage, &m_destImage, m_settings.level, m_settings.iterations);
            break;

        case FindEdges:
            findEdges(&m_orgImage, &m_destImage, m_settings.level, m_settings.iterations);
            break;

        case Lut3D:
            applyLut3D();
            m_destImage = m_orgImage;
            break;
    }
}

void ColorFXFilter::solarize(DImg* const orgImage, DImg* const destImage, int factor)
{
    bool stretch = true;

    int w             = orgImage->width();
    int h             = orgImage->height();
    const uchar* data = orgImage->bits();
    bool sb           = orgImage->sixteenBit();
    uchar* pResBits   = destImage->bits();

    if (!sb)        // 8 bits image.
    {
        uint threshold   = (uint)((100 - factor) * (255 + 1) / 100);
        threshold        = qMax((uint)1, threshold);
        const uchar* ptr = data;
        uchar* dst       = pResBits;
        uchar  a, r, g, b;

        for (int x = 0 ; x < w * h ; ++x)
        {
            b = ptr[0];
            g = ptr[1];
            r = ptr[2];
            a = ptr[3];

            if (stretch)
            {
                r = (r > threshold) ? (255 - r) * 255 / (255 - threshold) : r * 255 / threshold;
                g = (g > threshold) ? (255 - g) * 255 / (255 - threshold) : g * 255 / threshold;
                b = (b > threshold) ? (255 - b) * 255 / (255 - threshold) : b * 255 / threshold;
            }
            else
            {
                if (r > threshold)
                {
                    r = (255 - r);
                }

                if (g > threshold)
                {
                    g = (255 - g);
                }

                if (b > threshold)
                {
                    b = (255 - b);
                }
            }

            dst[0] = b;
            dst[1] = g;
            dst[2] = r;
            dst[3] = a;

            ptr += 4;
            dst += 4;
        }
    }
    else                            // 16 bits image.
    {
        uint threshold            = (uint)((100 - factor) * (65535 + 1) / 100);
        threshold                 = qMax((uint)1, threshold);
        const unsigned short* ptr = reinterpret_cast<const unsigned short*>(data);
        unsigned short* dst       = reinterpret_cast<unsigned short*>(pResBits);
        unsigned short  a, r, g, b;

        for (int x = 0 ; x < w * h ; ++x)
        {
            b = ptr[0];
            g = ptr[1];
            r = ptr[2];
            a = ptr[3];

            if (stretch)
            {
                r = (r > threshold) ? (65535 - r) * 65535 / (65535 - threshold) : r * 65535 / threshold;
                g = (g > threshold) ? (65535 - g) * 65535 / (65535 - threshold) : g * 65535 / threshold;
                b = (b > threshold) ? (65535 - b) * 65535 / (65535 - threshold) : b * 65535 / threshold;
            }
            else
            {
                if (r > threshold)
                {
                    r = (65535 - r);
                }

                if (g > threshold)
                {
                    g = (65535 - g);
                }

                if (b > threshold)
                {
                    b = (65535 - b);
                }
            }

            dst[0] = b;
            dst[1] = g;
            dst[2] = r;
            dst[3] = a;

            ptr += 4;
            dst += 4;
        }
    }
}

void ColorFXFilter::vivid(DImg* const orgImage, DImg* const destImage, int factor)
{
    float amount = factor / 100.0;

    // Apply Channel Mixer adjustments.

    MixerContainer settings;
    settings.redRedGain     = 1.0 + amount + amount;
    settings.redGreenGain   = (-1.0) * amount;
    settings.redBlueGain    = (-1.0) * amount;
    settings.greenRedGain   = (-1.0) * amount;
    settings.greenGreenGain = 1.0 + amount + amount;
    settings.greenBlueGain  = (-1.0) * amount;
    settings.blueRedGain    = (-1.0) * amount;
    settings.blueGreenGain  = (-1.0) * amount;
    settings.blueBlueGain   = 1.0 + amount + amount;

    MixerFilter mixer(orgImage, 0L, settings);
    mixer.startFilterDirectly();
    DImg mixed = mixer.getTargetImage();

    // And now apply the curve correction.

    CurvesContainer prm(ImageCurves::CURVE_SMOOTH, orgImage->sixteenBit());
    prm.initialize();

    if (!orgImage->sixteenBit())        // 8 bits image.
    {
        prm.values[LuminosityChannel].setPoint(0,  QPoint(0,   0));
        prm.values[LuminosityChannel].setPoint(5,  QPoint(63,  60));
        prm.values[LuminosityChannel].setPoint(10, QPoint(191, 194));
        prm.values[LuminosityChannel].setPoint(16, QPoint(255, 255));
    }
    else            // 16 bits image.
    {
        prm.values[LuminosityChannel].setPoint(0,  QPoint(0,     0));
        prm.values[LuminosityChannel].setPoint(5,  QPoint(16128, 15360));
        prm.values[LuminosityChannel].setPoint(10, QPoint(48896, 49664));
        prm.values[LuminosityChannel].setPoint(16, QPoint(65535, 65535));
    }

    CurvesFilter curves(&mixed, 0L, prm);
    curves.startFilterDirectly();
    *destImage = curves.getTargetImage();
}

/* Function to apply the Neon effect
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Intensity        => Intensity value
 * BW               => Border Width
 *
 * Theory           => Wow, this is a great effect, you've never seen a Neon effect
 *                     like this on PSC. Is very similar to Growing Edges (photoshop)
 *                     Some pictures will be very interesting
 */
void ColorFXFilter::neon(DImg* const orgImage, DImg* const destImage, int Intensity, int BW)
{
    neonFindEdges(orgImage, destImage, true, Intensity, BW);
}

/* Function to apply the Find Edges effect
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Intensity        => Intensity value
 * BW               => Border Width
 *
 * Theory           => Wow, another Photoshop filter (FindEdges). Do you understand
 *                     Neon effect ? This is the same engine, but is inversed with
 *                     255 - color.
 */
void ColorFXFilter::findEdges(DImg* const orgImage, DImg* const destImage, int Intensity, int BW)
{
    neonFindEdges(orgImage, destImage, false, Intensity, BW);
}

static inline int getOffset(int Width, int X, int Y, int bytesDepth)
{
    return (Y * Width * bytesDepth) + (X * bytesDepth);
}

static inline int Lim_Max(int Now, int Up, int Max)
{
    --Max;

    while (Now > Max - Up)
    {
        --Up;
    }

    return (Up);
}

// Implementation of neon and FindEdges. They share 99% of their code.
void ColorFXFilter::neonFindEdges(DImg* const orgImage, DImg* const destImage, bool neon, int Intensity, int BW)
{
    int Width         = orgImage->width();
    int Height        = orgImage->height();
    const uchar* data = orgImage->bits();
    bool sixteenBit   = orgImage->sixteenBit();
    int bytesDepth    = orgImage->bytesDepth();
    uchar* pResBits   = destImage->bits();

    Intensity = (Intensity < 0) ? 0 : (Intensity > 5) ? 5 : Intensity;
    BW        = (BW < 1) ? 1 : (BW > 5) ? 5 : BW;

    uchar* ptr, *ptr1, *ptr2;

    // these must be uint, we need full 2^32 range for 16 bit
    uint color_1, color_2, colorPoint, colorOther1, colorOther2;

    // initial copy
    memcpy(pResBits, data, Width * Height * bytesDepth);

    double intensityFactor = qSqrt(1 << Intensity);

    for (int h = 0; h < Height; ++h)
    {
        for (int w = 0; w < Width; ++w)
        {
            ptr  = pResBits + getOffset(Width, w, h, bytesDepth);
            ptr1 = pResBits + getOffset(Width, w + Lim_Max(w, BW, Width), h, bytesDepth);
            ptr2 = pResBits + getOffset(Width, w, h + Lim_Max(h, BW, Height), bytesDepth);

            if (sixteenBit)
            {
                for (int k = 0; k <= 2; ++k)
                {
                    colorPoint  = reinterpret_cast<unsigned short*>(ptr)[k];
                    colorOther1 = reinterpret_cast<unsigned short*>(ptr1)[k];
                    colorOther2 = reinterpret_cast<unsigned short*>(ptr2)[k];
                    color_1     = (colorPoint - colorOther1) * (colorPoint - colorOther1);
                    color_2     = (colorPoint - colorOther2) * (colorPoint - colorOther2);

                    // old algorithm was
                    // sqrt ((color_1 + color_2) << Intensity)
                    // As (a << I) = a * (1 << I) = a * (2^I), and we can split the square root

                    if (neon)
                    {
                        reinterpret_cast<unsigned short*>(ptr)[k] = CLAMP065535((int)(sqrt((double)color_1 + color_2) * intensityFactor));
                    }
                    else
                    {
                        reinterpret_cast<unsigned short*>(ptr)[k] = 65535 - CLAMP065535((int)(sqrt((double)color_1 + color_2) * intensityFactor));
                    }
                }
            }
            else
            {
                for (int k = 0; k <= 2; ++k)
                {
                    colorPoint  = ptr[k];
                    colorOther1 = ptr1[k];
                    colorOther2 = ptr2[k];
                    color_1     = (colorPoint - colorOther1) * (colorPoint - colorOther1);
                    color_2     = (colorPoint - colorOther2) * (colorPoint - colorOther2);

                    if (neon)
                    {
                        ptr[k] = CLAMP0255((int)(qSqrt((double)color_1 + color_2) * intensityFactor));
                    }
                    else
                    {
                        ptr[k] = 255 - CLAMP0255((int)(qSqrt((double)color_1 + color_2) * intensityFactor));
                    }
                }
            }
        }
    }
}

#define Lut3DSetPixel(table, w, x, y, p) \
    table[((y) * (w) + (x)) * 4 + 0] = qRed(p)   * 65535 / 255; \
    table[((y) * (w) + (x)) * 4 + 1] = qGreen(p) * 65535 / 255; \
    table[((y) * (w) + (x)) * 4 + 2] = qBlue(p)  * 65535 / 255;

void ColorFXFilter::loadLut3D(const QString& path)
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
void ColorFXFilter::applyLut3D()
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
                          255, m_settings.intensity);
        }
        else
        {
            ImageFilterFx(m_lutTable, m_lutTableSize, reinterpret_cast<unsigned short*>(m_orgImage.bits()),
                          i, min(i + stepI, maxI),
                          65535, m_settings.intensity);
        }

        postProgress(progress);
    }
}

FilterAction ColorFXFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("type"),      m_settings.colorFXType);
    action.addParameter(QLatin1String("iteration"), m_settings.iterations);
    action.addParameter(QLatin1String("level"),     m_settings.level);
    action.addParameter(QLatin1String("path"),      m_settings.path);
    action.addParameter(QLatin1String("intensity"), m_settings.intensity);

    return action;
}

void ColorFXFilter::readParameters(const FilterAction& action)
{
    m_settings.colorFXType = action.parameter(QLatin1String("type")).toInt();
    m_settings.iterations  = action.parameter(QLatin1String("iteration")).toInt();
    m_settings.level       = action.parameter(QLatin1String("level")).toInt();
    m_settings.path        = action.parameter(QLatin1String("path")).toString();
    m_settings.intensity   = action.parameter(QLatin1String("intensity")).toInt();
}

}  // namespace Digikam
