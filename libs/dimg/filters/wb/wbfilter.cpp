/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-16-01
 * Description : white balance color correction.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008      by Guillaume Castagnino <casta at xwing dot info>
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

#include "wbfilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "imagehistogram.h"

namespace Digikam
{

class WBFilter::Private
{
public:

    Private()
    {
        // Obsolete in algorithm since over/under exposure indicators
        // are implemented directly with preview widget.
        WBind   = false;
        overExp = false;

        clipSat = true;
        mr      = 1.0;
        mg      = 1.0;
        mb      = 1.0;
        BP      = 0;
        WP      = 0;
        rgbMax  = 0;

        for (int i = 0; i < 65536; ++i)
        {
            curve[i] = 0.0;
        }
    }

    bool  clipSat;
    bool  overExp;
    bool  WBind;

    int   BP;
    int   WP;

    uint  rgbMax;

    float curve[65536];
    float mr;
    float mg;
    float mb;
};

WBFilter::WBFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

WBFilter::WBFilter(DImg* const orgImage, QObject* const parent, const WBContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("WBFilter")),
      d(new Private)
{
    m_settings = settings;
    initFilter();
}

WBFilter::WBFilter(const WBContainer& settings, DImgThreadedFilter* const master,
                   const DImg& orgImage, const DImg& destImage, int progressBegin, int progressEnd)
    : DImgThreadedFilter(master, orgImage, destImage, progressBegin, progressEnd, QLatin1String("WBFilter")),
      d(new Private)
{
    m_settings = settings;
    filterImage();
}

WBFilter::~WBFilter()
{
    cancelFilter();
    delete d;
}

void WBFilter::filterImage()
{
    d->WP      = m_orgImage.sixteenBit() ? 65536 : 256;
    d->rgbMax  = m_orgImage.sixteenBit() ? 65536 : 256;

    // Set final lut.
    setRGBmult();
    d->mr = d->mb = 1.0;

    if (d->clipSat)
    {
        d->mg = 1.0;
    }

    setLUTv();
    setRGBmult();

    // See bug #259223 : scaling down the rgb multipliers just enough to prevent clipping
    if (m_settings.maxr == -1 && m_settings.maxg == -1 && m_settings.maxb == -1)
    {
        findChanelsMax((const DImg*) &m_orgImage,
                       m_settings.maxr,
                       m_settings.maxg,
                       m_settings.maxb);
    }

    preventAutoExposure(m_settings.maxr, m_settings.maxg, m_settings.maxb);

    // Apply White balance adjustments.
    adjustWhiteBalance(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit());
    m_destImage = m_orgImage;
}

void WBFilter::autoWBAdjustementFromColor(const QColor& tc, double& temperature, double& green)
{
    // Calculate Temperature and Green component from color picked.

    float mr=0.0, mg=0.0, mb=0.0;

    qCDebug(DIGIKAM_DIMG_LOG) << "Sums:  R:" << tc.red() << " G:" << tc.green() << " B:" << tc.blue();

    /* This is a dichotomic search based on Blue and Red layers ratio
       to find the matching temperature
       adapted from ufraw (0.12.1) RGB_to_Temperature
    */
    double tmin  = 2000.0;
    double tmax  = 12000.0;
    double mBR   = (double)tc.blue() / (double)tc.red();
    green        = 1.0;

    for (temperature = (tmin + tmax) / 2; tmax - tmin > 10; temperature = (tmin + tmax) / 2)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Intermediate Temperature (K):" << temperature;
        setRGBmult(temperature, green, mr, mg, mb);

        if (mr / mb > mBR)
        {
            tmax = temperature;
        }
        else
        {
            tmin = temperature;
        }
    }

    // Calculate the green level to neutralize picture
    green = (mr / mg) / ((double)tc.green() / (double)tc.red());

    qCDebug(DIGIKAM_DIMG_LOG) << "Temperature (K):" << temperature;
    qCDebug(DIGIKAM_DIMG_LOG) << "Green component:" << green;
}

void WBFilter::autoExposureAdjustement(const DImg* const img, double& black, double& expo)
{
    // Create an histogram of original image.

    ImageHistogram* const histogram = new ImageHistogram(*img);
    histogram->calculate();

    // Calculate optimal exposition and black level

    int    i;
    double sum, stop;
    uint   rgbMax = img->sixteenBit() ? 65536 : 256;

    // Cutoff at 0.5% of the histogram.

    stop = img->width() * img->height() / 200;

    for (i = rgbMax, sum = 0; (i >= 0) && (sum < stop); --i)
    {
        sum += histogram->getValue(LuminosityChannel, i);
    }

    expo = -log((float)(i + 1) / rgbMax) / log(2);
    qCDebug(DIGIKAM_DIMG_LOG) << "White level at:" << i;

    for (i = 1, sum = 0; (i < (int)rgbMax) && (sum < stop); ++i)
    {
        sum += histogram->getValue(LuminosityChannel, i);
    }

    black = (double)i / rgbMax;
    black /= 2;

    qCDebug(DIGIKAM_DIMG_LOG) << "Black:" << black << "  Exposition:" << expo;

    delete histogram;
}

void WBFilter::setRGBmult(double& temperature, double& green, float& mr, float& mg, float& mb)
{
    float  mi;
    double xD, yD, X, Y, Z;

    if (temperature > 12000)
    {
        temperature = 12000.0;
    }

    /* Here starts the code picked from ufraw (0.12.1)
       to convert Temperature + green multiplier to RGB multipliers
    */

    /* Convert between Temperature and RGB.
     * Base on information from http://www.brucelindbloom.com/
     * The fit for D-illuminant between 4000K and 12000K are from CIE
     * The generalization to 2000K < T < 4000K and the blackbody fits
     * are my own and should be taken with a grain of salt.
     */
    const double XYZ_to_RGB[3][3] =
    {
        { 3.24071,  -0.969258,   0.0556352 },
        { -1.53726,  1.87599,    -0.203996 },
        { -0.498571, 0.0415557,  1.05707   }
    };

    // Fit for CIE Daylight illuminant
    if (temperature <= 4000)
    {
        xD = 0.27475e9   / (temperature * temperature * temperature)
             - 0.98598e6 / (temperature * temperature)
             + 1.17444e3 / temperature + 0.145986;
    }
    else if (temperature <= 7000)
    {
        xD = -4.6070e9   / (temperature * temperature * temperature)
             + 2.9678e6  / (temperature * temperature)
             + 0.09911e3 / temperature + 0.244063;
    }
    else
    {
        xD = -2.0064e9   / (temperature * temperature * temperature)
             + 1.9018e6  / (temperature * temperature)
             + 0.24748e3 / temperature + 0.237040;
    }

    yD = -3 * xD * xD + 2.87 * xD - 0.275;

    X  = xD / yD;
    Y  = 1;
    Z  = (1 - xD - yD) / yD;
    mr = X * XYZ_to_RGB[0][0] + Y * XYZ_to_RGB[1][0] + Z * XYZ_to_RGB[2][0];
    mg = X * XYZ_to_RGB[0][1] + Y * XYZ_to_RGB[1][1] + Z * XYZ_to_RGB[2][1];
    mb = X * XYZ_to_RGB[0][2] + Y * XYZ_to_RGB[1][2] + Z * XYZ_to_RGB[2][2];
    /* End of the code picked to ufraw
    */

    // Apply green multiplier
    mg  = mg / green;

    mr  = 1.0 / mr;
    mg  = 1.0 / mg;
    mb  = 1.0 / mb;

    // Normalize to at least 1.0, so we are not dimming colors only bumping.
    mi  = qMin(mb, qMin(mr, mg));
    mr /= mi;
    mg /= mi;
    mb /= mi;
}

void WBFilter::setRGBmult()
{
    setRGBmult(m_settings.temperature, m_settings.green, d->mr, d->mg, d->mb);
}

void WBFilter::findChanelsMax(const DImg* const img, int& maxr, int& maxg, int& maxb)
{
    const uchar* data = img->bits();
    int width         = img->width();
    int height        = img->height();
    bool sixteenBit   = img->sixteenBit();

    maxr              = 0;
    maxg              = 0;
    maxb              = 0;
    uint size         = (uint)(width * height);

    // Look for the maximum r, g, b values in the given image.

    if (!sixteenBit)        // 8 bits image.
    {
        for (uint j = 0 ; j < size; ++j)
        {
            if (maxb < data[0])
            {
                maxb = data[0];
            }

            if (maxg < data[1])
            {
                maxg = data[1];
            }

            if (maxr < data[2])
            {
                maxr = data[2];
            }

            data += 4;
        }
    }
    else                    // 16 bits image.
    {
        const unsigned short* ptr = reinterpret_cast<const unsigned short*>(data);

        for (uint j = 0 ; j < size; ++j)
        {
            if (maxb < ptr[0])
            {
                maxb = ptr[0];
            }

            if (maxg < ptr[1])
            {
                maxg = ptr[1];
            }

            if (maxr < ptr[2])
            {
                maxr = ptr[2];
            }

            ptr += 4;
        }
    }
}

void WBFilter::preventAutoExposure(int maxr, int maxg, int maxb)
{
    // White balance them and find the brightest one.
    maxr     *= d->mr;
    maxg     *= d->mg;
    maxb     *= d->mb;
    uint max  = qMax(maxr, qMax(maxb, maxg));

    // Scale wb coefficients down if one of them is overexposed.
    if (max > d->rgbMax - 1)
    {
        double adjust  = (double)(d->rgbMax - 1) / max;
        d->mb         *= adjust;
        d->mg         *= adjust;
        d->mr         *= adjust;
    }
}

void WBFilter::setLUTv()
{
    double b = d->mg * pow(2, m_settings.expositionMain + m_settings.expositionFine);
    d->BP    = (uint)(d->rgbMax * m_settings.black);
    d->WP    = (uint)(d->rgbMax / b);

    if (d->WP - d->BP < 1)
    {
        d->WP = d->BP + 1;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "T(K): " << m_settings.temperature
             << " => R:" << d->mr
             << " G:   " << d->mg
             << " B:   " << d->mb
             << " BP:  " << d->BP
             << " WP:  " << d->WP;

    d->curve[0] = 0;

    // We will try to reproduce the same Gamma effect here than BCG tool.
    double gamma;

    if (m_settings.gamma >= 1.0)
    {
        gamma = 0.335 * (2.0 - m_settings.gamma) + 0.665;
    }
    else
    {
        gamma = 1.8 * (2.0 - m_settings.gamma) - 0.8;
    }

    for (int i = 1; i < (int)d->rgbMax; ++i)
    {
        float x      = (float)(i - d->BP) / (d->WP - d->BP);
        d->curve[i]  = (i < d->BP) ? 0 : (d->rgbMax - 1) * pow((double)x, gamma);
        d->curve[i] *= (1 - m_settings.dark * exp(-x * x / 0.002));
        d->curve[i] /= (float)i;
    }
}

void WBFilter::adjustWhiteBalance(uchar* const data, int width, int height, bool sixteenBit)
{
    uint size = (uint)(width * height);
    uint i, j;
    int  progress;

    if (!sixteenBit)        // 8 bits image.
    {
        uchar  red, green, blue;
        uchar* ptr = data;

        for (j = 0 ; runningFlag() && (j < size) ; ++j)
        {
            int v, rv[3];

            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];

            rv[0] = (int)(blue  * d->mb);
            rv[1] = (int)(green * d->mg);
            rv[2] = (int)(red   * d->mr);
            v     = qMax(rv[0], rv[1]);
            v     = qMax(v, rv[2]);

            if (d->clipSat)
            {
                v = qMin(v, (int)d->rgbMax - 1);
            }

            i = v;

            ptr[0] = (uchar)pixelColor(rv[0], i, v);
            ptr[1] = (uchar)pixelColor(rv[1], i, v);
            ptr[2] = (uchar)pixelColor(rv[2], i, v);
            ptr    += 4;

            progress = (int)(((double)j * 100.0) / size);

            if (progress % 5 == 0)
            {
                postProgress(progress);
            }
        }
    }
    else               // 16 bits image.
    {
        unsigned short  red, green, blue;
        unsigned short* ptr = reinterpret_cast<unsigned short*>(data);

        for (j = 0 ; runningFlag() && (j < size) ; ++j)
        {
            int v, rv[3];

            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];

            rv[0] = (int)(blue  * d->mb);
            rv[1] = (int)(green * d->mg);
            rv[2] = (int)(red   * d->mr);
            v     = qMax(rv[0], rv[1]);
            v     = qMax(v, rv[2]);

            if (d->clipSat)
            {
                v = qMin(v, (int)d->rgbMax - 1);
            }

            i = v;

            ptr[0] = pixelColor(rv[0], i, v);
            ptr[1] = pixelColor(rv[1], i, v);
            ptr[2] = pixelColor(rv[2], i, v);
            ptr    += 4;

            progress = (int)(((double)j * 100.0) / size);

            if (progress % 5 == 0)
            {
                postProgress(progress);
            }
        }
    }
}

unsigned short WBFilter::pixelColor(int colorMult, int index, int value)
{
    int r = (d->clipSat && colorMult > (int)d->rgbMax) ? d->rgbMax : colorMult;

    if (value > d->BP && d->overExp && value > d->WP)
    {
        if (d->WBind)
        {
            r = (colorMult > d->WP) ? 0 : r;
        }
        else
        {
            r = 0;
        }
    }

    return((unsigned short)CLAMP((int)((index - m_settings.saturation * (index - r)) * d->curve[index]), 0, (int)(d->rgbMax - 1)));
}

FilterAction WBFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    m_settings.writeToFilterAction(action);

    return action;
}

void WBFilter::readParameters(const FilterAction& action)
{
    m_settings = WBContainer::fromFilterAction(action);
}

}  // namespace Digikam
