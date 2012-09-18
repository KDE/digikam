/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Wavelets Noise Reduction threaded image filter.
 *               This filter work in YCrCb color space.
 *
 * Copyright (C) 2005-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008      by Marco Rossini <marco dot rossini at gmx dot net>
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

#include "nrfilter.h"

// C++ includes

#include <cmath>

// Local includes

#include "dimg.h"
#include "dcolor.h"

namespace Digikam
{

class NRFilter::Private
{
public:

    Private()
    {
        for (int c = 0 ; c < 3; ++c)
        {
            fimg[c]   = 0;
            buffer[c] = 0;
        }
    }

    float*      fimg[3];
    float*      buffer[3];

    NRContainer settings;
};

NRFilter::NRFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

NRFilter::NRFilter(DImg* const orgImage, QObject* const parent, const NRContainer& settings)
    : DImgThreadedFilter(orgImage, parent, "NRFilter"),
      d(new Private)
{
    d->settings = settings;

    initFilter();
}

NRFilter::~NRFilter()
{
    cancelFilter();
    delete d;
}

void NRFilter::filterImage()
{
    DColor col;
    int    progress;

    int width  = m_orgImage.width();
    int height = m_orgImage.height();
    float clip = m_orgImage.sixteenBit() ? 65535.0 : 255.0;

    // Allocate buffers.

    for (int c = 0; c < 3; ++c)
    {
        d->fimg[c] = new float[width * height];
    }

    d->buffer[1] = new float[width * height];
    d->buffer[2] = new float[width * height];

    // Read the full image and convert pixel values to float [0,1].

    int j = 0;

    for (int y = 0; runningFlag() && (y < height); ++y)
    {
        for (int x = 0; runningFlag() && (x < width); ++x)
        {
            col           = m_orgImage.getPixelColor(x, y);
            d->fimg[0][j] = col.red()   / clip;
            d->fimg[1][j] = col.green() / clip;
            d->fimg[2][j] = col.blue()  / clip;
            ++j;
        }
    }

    postProgress(10);

    // do colour model conversion sRGB[0,1] -> YCrCb.

    if (runningFlag())
    {
        srgb2ycbcr(d->fimg, width * height);
    }

    postProgress(20);

    // denoise the channels individually

    for (int c = 0; runningFlag() && (c < 3); ++c)
    {
        d->buffer[0] = d->fimg[c];

        if (d->settings.thresholds[c] > 0.0)
        {
            waveletDenoise(d->buffer, width, height, d->settings.thresholds[c], d->settings.softness[c]);

            progress = (int)(30.0 + ((double)c * 60.0) / 4);

            if (progress % 5 == 0)
            {
                postProgress(progress);
            }
        }
    }

    // Retransform the image data to sRGB[0,1].

    if (runningFlag())
    {
        ycbcr2srgb(d->fimg, width * height);
    }

    postProgress(80);

    // clip the values

    for (int c = 0; runningFlag() && (c < 3); ++c)
    {
        for (int i = 0; i < width * height; ++i)
        {
            d->fimg[c][i] = qBound(0.0F, d->fimg[c][i] * clip, clip);
        }
    }

    postProgress(90);

    // Write back the full image and convert pixel values from float [0,1].

    j = 0;

    for (int y = 0; runningFlag() && (y < height); ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            col.setRed((int)(d->fimg[0][j] + 0.5));
            col.setGreen((int)(d->fimg[1][j] + 0.5));
            col.setBlue((int)(d->fimg[2][j] + 0.5));
            col.setAlpha(m_orgImage.getPixelColor(x, y).alpha());
            ++j;

            m_destImage.setPixelColor(x, y, col);
        }
    }

    postProgress(100);

    // Free buffers.

    for (int c = 0; c < 3; ++c)
    {
        delete [] d->fimg[c];
    }

    delete [] d->buffer[1];
    delete [] d->buffer[2];
}

// -- Wavelets denoise methods -----------------------------------------------------------

void NRFilter::waveletDenoise(float* const fimg[3], unsigned int width, unsigned int height,
                              float threshold, double softness)
{
    float        thold;
    unsigned int i, lev, lpass = 0, hpass = 0, size, col, row;
    double       stdev[5];
    unsigned int samples[5];

    size  = width * height;
    QScopedArrayPointer<float> temp(new float[qMax(width, height)]);

    for (lev = 0; runningFlag() && (lev < 5); ++lev)
    {
        lpass = ((lev & 1) + 1);

        for (row = 0; runningFlag() && (row < height); ++row)
        {
            hatTransform(temp.data(), fimg[hpass] + row * width, 1, width, 1 << lev);

            for (col = 0; col < width; ++col)
            {
                fimg[lpass][row * width + col] = temp[col] * 0.25;
            }
        }

        for (col = 0; runningFlag() && (col < width); ++col)
        {
            hatTransform(temp.data(), fimg[lpass] + col, width, height, 1 << lev);

            for (row = 0; row < height; ++row)
            {
                fimg[lpass][row * width + col] = temp[row] * 0.25;
            }
        }

        thold = 5.0 / (1 << 6) * exp(-2.6 * sqrt(lev + 1.0)) * 0.8002 / exp(-2.6);

        // initialize stdev values for all intensities

        stdev[0]   = stdev[1]   = stdev[2]   = stdev[3]   = stdev[4]   = 0.0;
        samples[0] = samples[1] = samples[2] = samples[3] = samples[4] = 0;

        // calculate stdevs for all intensities

        for (i = 0; runningFlag() && (i < size); ++i)
        {
            fimg[hpass][i] -= fimg[lpass][i];

            if (fimg[hpass][i] < thold && fimg[hpass][i] > -thold)
            {
                if (fimg[lpass][i] > 0.8)
                {
                    stdev[4] += fimg[hpass][i] * fimg[hpass][i];
                    samples[4]++;
                }
                else if (fimg[lpass][i] > 0.6)
                {
                    stdev[3] += fimg[hpass][i] * fimg[hpass][i];
                    samples[3]++;
                }
                else if (fimg[lpass][i] > 0.4)
                {
                    stdev[2] += fimg[hpass][i] * fimg[hpass][i];
                    samples[2]++;
                }
                else if (fimg[lpass][i] > 0.2)
                {
                    stdev[1] += fimg[hpass][i] * fimg[hpass][i];
                    samples[1]++;
                }
                else
                {
                    stdev[0] += fimg[hpass][i] * fimg[hpass][i];
                    samples[0]++;
                }
            }
        }

        stdev[0] = sqrt(stdev[0] / (samples[0] + 1));
        stdev[1] = sqrt(stdev[1] / (samples[1] + 1));
        stdev[2] = sqrt(stdev[2] / (samples[2] + 1));
        stdev[3] = sqrt(stdev[3] / (samples[3] + 1));
        stdev[4] = sqrt(stdev[4] / (samples[4] + 1));

        // do thresholding

        for (i = 0; runningFlag() && (i < size); ++i)
        {
            if (fimg[lpass][i] > 0.8)
            {
                thold = threshold * stdev[4];
            }
            else if (fimg[lpass][i] > 0.6)
            {
                thold = threshold * stdev[3];
            }
            else if (fimg[lpass][i] > 0.4)
            {
                thold = threshold * stdev[2];
            }
            else if (fimg[lpass][i] > 0.2)
            {
                thold = threshold * stdev[1];
            }
            else
            {
                thold = threshold * stdev[0];
            }

            if (fimg[hpass][i] < -thold)
            {
                fimg[hpass][i] += thold - thold * softness;
            }
            else if (fimg[hpass][i] > thold)
            {
                fimg[hpass][i] -= thold - thold * softness;
            }
            else
            {
                fimg[hpass][i] *= softness;
            }

            if (hpass)
            {
                fimg[0][i] += fimg[hpass][i];
            }
        }

        hpass = lpass;
    }

    for (i = 0; runningFlag() && (i < size); ++i)
    {
        fimg[0][i] = fimg[0][i] + fimg[lpass][i];
    }
}

void NRFilter::hatTransform(float* const temp, float* const base, int st, int size, int sc)
{
    int i;

    for (i = 0; i < sc; ++i)
    {
        temp[i] = 2 * base[st * i] + base[st * (sc - i)] + base[st * (i + sc)];
    }

    for (; i + sc < size; ++i)
    {
        temp[i] = 2 * base[st * i] + base[st * (i - sc)] + base[st * (i + sc)];
    }

    for (; i < size; ++i)
    {
        temp[i] = 2 * base[st * i] + base[st * (i - sc)] + base[st * (2 * size - 2 - (i + sc))];
    }
}

// -- Color Space conversion methods --------------------------------------------------

void NRFilter::srgb2ycbcr(float** const fimg, int size)
{
    float y, cb, cr;

    for (int i = 0; i < size; ++i)
    {
        y          =  0.2990 * fimg[0][i] + 0.5870 * fimg[1][i] + 0.1140 * fimg[2][i];
        cb         = -0.1687 * fimg[0][i] - 0.3313 * fimg[1][i] + 0.5000 * fimg[2][i] + 0.5;
        cr         =  0.5000 * fimg[0][i] - 0.4187 * fimg[1][i] - 0.0813 * fimg[2][i] + 0.5;
        fimg[0][i] = y;
        fimg[1][i] = cb;
        fimg[2][i] = cr;
    }
}

void NRFilter::ycbcr2srgb(float** const fimg, int size)
{
    float r, g, b;

    for (int i = 0; i < size; ++i)
    {
        r          = fimg[0][i] + 1.40200 * (fimg[2][i] - 0.5);
        g          = fimg[0][i] - 0.34414 * (fimg[1][i] - 0.5) - 0.71414 * (fimg[2][i] - 0.5);
        b          = fimg[0][i] + 1.77200 * (fimg[1][i] - 0.5);
        fimg[0][i] = r;
        fimg[1][i] = g;
        fimg[2][i] = b;
    }
}

void NRFilter::srgb2xyz(float** const fimg, int size)
{
    // fimg in [0:1], sRGB
    float x, y, z;

    for (int i = 0; i < size; ++i)
    {
        // scaling and gamma correction (approximate)
        fimg[0][i] = pow(fimg[0][i], (float)2.2);
        fimg[1][i] = pow(fimg[1][i], (float)2.2);
        fimg[2][i] = pow(fimg[2][i], (float)2.2);

        // matrix RGB -> XYZ, with D65 reference white (www.brucelindbloom.com)
        x = 0.412424  * fimg[0][i] + 0.357579 * fimg[1][i] + 0.180464  * fimg[2][i];
        y = 0.212656  * fimg[0][i] + 0.715158 * fimg[1][i] + 0.0721856 * fimg[2][i];
        z = 0.0193324 * fimg[0][i] + 0.119193 * fimg[1][i] + 0.950444  * fimg[2][i];

/*
        x = 0.412424 * fimg[0][i] + 0.212656  * fimg[1][i] + 0.0193324 * fimg[2][i];
        y = 0.357579 * fimg[0][i] + 0.715158  * fimg[1][i] + 0.119193  * fimg[2][i];
        z = 0.180464 * fimg[0][i] + 0.0721856 * fimg[1][i] + 0.950444  * fimg[2][i];
*/

        fimg[0][i] = x;
        fimg[1][i] = y;
        fimg[2][i] = z;
    }
}

void NRFilter::xyz2srgb(float** const fimg, int size)
{
    float r, g, b;

    for (int i = 0; i < size; ++i)
    {
        // matrix RGB -> XYZ, with D65 reference white (www.brucelindbloom.com)
        r = 3.24071   * fimg[0][i] - 1.53726  * fimg[1][i] - 0.498571  * fimg[2][i];
        g = -0.969258 * fimg[0][i] + 1.87599  * fimg[1][i] + 0.0415557 * fimg[2][i];
        b = 0.0556352 * fimg[0][i] - 0.203996 * fimg[1][i] + 1.05707   * fimg[2][i];

/*
        r =  3.24071  * fimg[0][i] - 0.969258  * fimg[1][i]
          + 0.0556352 * fimg[2][i];
        g = -1.53726  * fimg[0][i] + 1.87599   * fimg[1][i]
          - 0.203996  * fimg[2][i];
        b = -0.498571 * fimg[0][i] + 0.0415557 * fimg[1][i]
          + 1.05707   * fimg[2][i];
*/

        // scaling and gamma correction (approximate)
        r = r < 0 ? 0 : pow(r, (float)(1.0 / 2.2));
        g = g < 0 ? 0 : pow(g, (float)(1.0 / 2.2));
        b = b < 0 ? 0 : pow(b, (float)(1.0 / 2.2));

        fimg[0][i] = r;
        fimg[1][i] = g;
        fimg[2][i] = b;
    }
}

void NRFilter::lab2srgb(float** const fimg, int size)
{
    float x, y, z;

    for (int i = 0; i < size; ++i)
    {
        // convert back to normal LAB
        fimg[0][i] = (fimg[0][i] - 0 * 16 * 27 / 24389.0) * 116;
        fimg[1][i] = (fimg[1][i] - 0.5) * 500 * 2;
        fimg[2][i] = (fimg[2][i] - 0.5) * 200 * 2.2;

        // matrix
        y = (fimg[0][i] + 16) / 116;
        z = y - fimg[2][i] / 200.0;
        x = fimg[1][i] / 500.0 + y;

        // scale
        if (x * x * x > 216 / 24389.0)
        {
            x = x * x * x;
        }
        else
        {
            x = (116 * x - 16) * 27 / 24389.0;
        }

        if (fimg[0][i] > 216 / 27.0)
        {
            y = y * y * y;
        }
        else
        {
             //y = fimg[0][i] * 27 / 24389.0;*/
            y = (116 * y - 16) * 27 / 24389.0;
        }

        if (z * z * z > 216 / 24389.0)
        {
            z = z * z * z;
        }
        else
        {
            z = (116 * z - 16) * 27 / 24389.0;
        }

        // white reference
        fimg[0][i] = x * 0.95047;
        fimg[1][i] = y;
        fimg[2][i] = z * 1.08883;
    }

    xyz2srgb(fimg, size);
}

void NRFilter::srgb2lab(float** const fimg, int size)
{
    float l, a, b;

    srgb2xyz(fimg, size);

    for (int i = 0; i < size; ++i)
    {
        // reference white
        fimg[0][i] /= 0.95047F;
/*
        fimg[1][i] /= 1.00000;          // (just for completeness)
*/
        fimg[2][i] /= 1.08883F;

        // scale
        if (fimg[0][i] > 216.0 / 24389.0)
        {
            fimg[0][i] = pow(fimg[0][i], (float)(1.0 / 3.0));
        }
        else
        {
            fimg[0][i] = (24389.0 * fimg[0][i] / 27.0 + 16.0) / 116.0;
        }

        if (fimg[1][i] > 216.0 / 24389.0)
        {
            fimg[1][i] = pow(fimg[1][i], (float)(1.0 / 3.0));
        }
        else
        {
            fimg[1][i] = (24389 * fimg[1][i] / 27.0 + 16.0) / 116.0;
        }

        if (fimg[2][i] > 216.0 / 24389.0)
        {
            fimg[2][i] = (float)pow(fimg[2][i], (float)(1.0 / 3.0));
        }
        else
        {
            fimg[2][i] = (24389.0 * fimg[2][i] / 27.0 + 16.0) / 116.0;
        }

        l          = 116 * fimg[1][i]  - 16;
        a          = 500 * (fimg[0][i] - fimg[1][i]);
        b          = 200 * (fimg[1][i] - fimg[2][i]);
        fimg[0][i] = l / 116.0; // + 16 * 27 / 24389.0;
        fimg[1][i] = a / 500.0 / 2.0 + 0.5;
        fimg[2][i] = b / 200.0 / 2.2 + 0.5;

        if (fimg[0][i] < 0)
        {
            fimg[0][i] = 0;
        }
    }
}

FilterAction NRFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    for (int i = 0; i < 3; ++i)
    {
        action.addParameter(QString("softness[%1]").arg(i),   d->settings.softness[i]);
        action.addParameter(QString("thresholds[%1]").arg(i), d->settings.thresholds[i]);
    }

    return action;
}

void NRFilter::readParameters(const FilterAction& action)
{
    for (int i = 0; i < 3; ++i)
    {
        d->settings.softness[i]   = action.parameter(QString("softness[%1]").arg(i)).toDouble();
        d->settings.thresholds[i] = action.parameter(QString("thresholds[%1]").arg(i)).toDouble();
    }
}

QString NRFilter::filterIdentifier() const
{
    return FilterIdentifier();
}

QString NRFilter::FilterIdentifier()
{
    return "digikam:NoiseReductionFilter";
}

QString NRFilter::DisplayableName()
{
    return I18N_NOOP("Noise Reduction Filter");
}

QList<int> NRFilter::SupportedVersions()
{
    return QList<int>() << 1;
}

int NRFilter::CurrentVersion()
{
    return 1;
}

}  // namespace Digikam
