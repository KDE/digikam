/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Wavelets Noise Reduction threaded image filter.
 *               This filter work in YCrCb color space.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QtConcurrent>

// Local includes

#include "dimg.h"
#include "dcolor.h"

namespace Digikam
{

NRContainer::NRContainer()
{
    thresholds[0] = 1.2;     // Y
    thresholds[1] = 1.2;     // Cr
    thresholds[2] = 1.2;     // Cb
    softness[0]   = 0.9;     // Y
    softness[1]   = 0.9;     // Cr
    softness[2]   = 0.9;     // Cb
}

NRContainer::~NRContainer()
{
}

QDebug operator<<(QDebug dbg, const NRContainer& inf)
{
    dbg.nospace() << "Y Threshold: "
                  << inf.thresholds[0] << ", ";
    dbg.nospace() << "Y Softness: "
                  << inf.softness[0] << ", ";
    dbg.nospace() << "Cb Threshold: "
                  << inf.thresholds[1] << ", ";
    dbg.nospace() << "Cb Softness: "
                  << inf.softness[1] << ", ";
    dbg.nospace() << "Cr  Threshold: "
                  << inf.thresholds[2] << ", ";
    dbg.nospace() << "Cr Softness: "
                  << inf.softness[2];
    return dbg.space();
}

// ----------------------------------------------------------------------------------------------

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
    : DImgThreadedFilter(orgImage, parent, QLatin1String("NRFilter")),
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

FilterAction NRFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    for (int i = 0; i < 3; ++i)
    {
        action.addParameter(QString::fromLatin1("softness[%1]").arg(i),   d->settings.softness[i]);
        action.addParameter(QString::fromLatin1("thresholds[%1]").arg(i), d->settings.thresholds[i]);
    }

    return action;
}

void NRFilter::readParameters(const FilterAction& action)
{
    for (int i = 0; i < 3; ++i)
    {
        d->settings.softness[i]   = action.parameter(QString::fromLatin1("softness[%1]").arg(i)).toDouble();
        d->settings.thresholds[i] = action.parameter(QString::fromLatin1("thresholds[%1]").arg(i)).toDouble();
    }
}

QString NRFilter::filterIdentifier() const
{
    return FilterIdentifier();
}

QString NRFilter::FilterIdentifier()
{
    return QLatin1String("digikam:NoiseReductionFilter");
}

QString NRFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Noise Reduction Filter"));
}

QList<int> NRFilter::SupportedVersions()
{
    return QList<int>() << 1;
}

int NRFilter::CurrentVersion()
{
    return 1;
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

void NRFilter::calculteStdevMultithreaded(const Args& prm)
{
    for (uint i = prm.start; runningFlag() && (i < prm.stop); ++i)
    {
        prm.fimg[*prm.hpass][i] -= prm.fimg[*prm.lpass][i];

        if (prm.fimg[*prm.hpass][i] < *prm.thold && prm.fimg[*prm.hpass][i] > -*prm.thold)
        {
            if (prm.fimg[*prm.lpass][i] > 0.8)
            {
                prm.stdev[4] += prm.fimg[*prm.hpass][i] * prm.fimg[*prm.hpass][i];
                prm.samples[4]++;
            }
            else if (prm.fimg[*prm.lpass][i] > 0.6)
            {
                prm.stdev[3] += prm.fimg[*prm.hpass][i] * prm.fimg[*prm.hpass][i];
                prm.samples[3]++;
            }
            else if (prm.fimg[*prm.lpass][i] > 0.4)
            {
                prm.stdev[2] += prm.fimg[*prm.hpass][i] * prm.fimg[*prm.hpass][i];
                prm.samples[2]++;
            }
            else if (prm.fimg[*prm.lpass][i] > 0.2)
            {
                prm.stdev[1] += prm.fimg[*prm.hpass][i] * prm.fimg[*prm.hpass][i];
                prm.samples[1]++;
            }
            else
            {
                prm.stdev[0] += prm.fimg[*prm.hpass][i] * prm.fimg[*prm.hpass][i];
                prm.samples[0]++;
            }
        }
    }
}

void NRFilter::thresholdingMultithreaded(const Args& prm)
{
    for (uint i = prm.start; runningFlag() && (i < prm.stop); ++i)
    {
        if (prm.fimg[*prm.lpass][i] > 0.8)
        {
            *prm.thold = prm.threshold * prm.stdev[4];
        }
        else if (prm.fimg[*prm.lpass][i] > 0.6)
        {
            *prm.thold = prm.threshold * prm.stdev[3];
        }
        else if (prm.fimg[*prm.lpass][i] > 0.4)
        {
            *prm.thold = prm.threshold * prm.stdev[2];
        }
        else if (prm.fimg[*prm.lpass][i] > 0.2)
        {
            *prm.thold = prm.threshold * prm.stdev[1];
        }
        else
        {
            *prm.thold = prm.threshold * prm.stdev[0];
        }

        if (prm.fimg[*prm.hpass][i] < -*prm.thold)
        {
            prm.fimg[*prm.hpass][i] += *prm.thold - *prm.thold * prm.softness;
        }
        else if (prm.fimg[*prm.hpass][i] > *prm.thold)
        {
            prm.fimg[*prm.hpass][i] -= *prm.thold - *prm.thold * prm.softness;
        }
        else
        {
            prm.fimg[*prm.hpass][i] *= prm.softness;
        }

        if (*prm.hpass)
        {
            prm.fimg[0][i] += prm.fimg[*prm.hpass][i];
        }
    }
}

void NRFilter::waveletDenoise(float* fimg[3], unsigned int width, unsigned int height,
                              float threshold, double softness)
{
    float  thold;
    uint   lpass = 0, hpass = 0;
    double stdev[5];
    uint   samples[5];
    uint   size  = width * height;

    QScopedArrayPointer<float> temp(new float[qMax(width, height)]);

    QList<int> vals = multithreadedSteps(size);
    QList <QFuture<void> > tasks;

    Args prm;
    prm.thold     = &thold;
    prm.lpass     = &lpass;
    prm.hpass     = &hpass;
    prm.threshold = threshold;
    prm.softness  = softness;
    prm.stdev     = &stdev[0];
    prm.samples   = &samples[0];
    prm.fimg      = fimg;

    for (uint lev = 0; runningFlag() && (lev < 5); ++lev)
    {
        lpass = ((lev & 1) + 1);

        for (uint row = 0; runningFlag() && (row < height); ++row)
        {
            hatTransform(temp.data(), fimg[hpass] + row * width, 1, width, 1 << lev);

            for (uint col = 0; col < width; ++col)
            {
                fimg[lpass][row * width + col] = temp[col] * 0.25;
            }
        }

        for (uint col = 0; runningFlag() && (col < width); ++col)
        {
            hatTransform(temp.data(), fimg[lpass] + col, width, height, 1 << lev);

            for (uint row = 0; row < height; ++row)
            {
                fimg[lpass][row * width + col] = temp[row] * 0.25;
            }
        }

        thold = 5.0 / (1 << 6) * exp(-2.6 * sqrt(lev + 1.0)) * 0.8002 / exp(-2.6);

        // initialize stdev values for all intensities

        stdev[0]   = stdev[1]   = stdev[2]   = stdev[3]   = stdev[4]   = 0.0;
        samples[0] = samples[1] = samples[2] = samples[3] = samples[4] = 0;

        // calculate stdevs for all intensities

        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            tasks.append(QtConcurrent::run(this,
                                        &NRFilter::calculteStdevMultithreaded,
                                        prm
                                        ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        stdev[0] = sqrt(stdev[0] / (samples[0] + 1));
        stdev[1] = sqrt(stdev[1] / (samples[1] + 1));
        stdev[2] = sqrt(stdev[2] / (samples[2] + 1));
        stdev[3] = sqrt(stdev[3] / (samples[3] + 1));
        stdev[4] = sqrt(stdev[4] / (samples[4] + 1));

        // do thresholding

        tasks.clear();

        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            tasks.append(QtConcurrent::run(this,
                                        &NRFilter::thresholdingMultithreaded,
                                        prm
                                        ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        hpass = lpass;
    }

    for (uint i = 0; runningFlag() && (i < size); ++i)
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

}  // namespace Digikam
