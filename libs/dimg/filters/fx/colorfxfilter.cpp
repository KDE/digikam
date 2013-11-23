/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : Color FX threaded image filter.
 *
 * Copyright 2005-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

// Qt includes

#include <qmath.h>

// Local includes

#include "dimg.h"
#include "mixerfilter.h"
#include "curvesfilter.h"

namespace Digikam
{

ColorFXFilter::ColorFXFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

ColorFXFilter::ColorFXFilter(DImg* const orgImage, QObject* const parent, const ColorFXContainer& settings)
    : DImgThreadedFilter(orgImage, parent, "ColorFX")
{
    m_settings = settings;

    initFilter();
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

FilterAction ColorFXFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter("type", m_settings.colorFXType);
    action.addParameter("iteration", m_settings.iterations);
    action.addParameter("level", m_settings.level);

    return action;
}

void ColorFXFilter::readParameters(const FilterAction& action)
{
    m_settings.colorFXType = action.parameter("type").toInt();
    m_settings.iterations  = action.parameter("iteration").toInt();
    m_settings.level       = action.parameter("level").toInt();
}

}  // namespace Digikam
