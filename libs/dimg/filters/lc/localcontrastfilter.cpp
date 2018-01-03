/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : Enhance image with local contrasts (as human eye does).
 *               LDR ToneMapper <http://zynaddsubfx.sourceforge.net/other/tonemapping>
 *
 * Copyright (C) 2009      by Nasca Octavian Paul <zynaddsubfx at yahoo dot com>
 * Copyright (C) 2009      by Julien Pontabry <julien dot pontabry at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "localcontrastfilter.h"

// Qt includes

#include <QtMath>
#include <QtConcurrent>

// Local includes

#include "digikam_debug.h"
#include "randomnumbergenerator.h"

namespace Digikam
{

class LocalContrastFilter::Private
{
public:

    Private()
    {
        current_process_power_value = 20.0;
    }

    // preprocessed values
    float                  current_process_power_value;

    LocalContrastContainer par;

    RandomNumberGenerator  generator;
};

LocalContrastFilter::LocalContrastFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

LocalContrastFilter::LocalContrastFilter(DImg* const image, QObject* const parent, const LocalContrastContainer& par)
    : DImgThreadedFilter(image, parent, QLatin1String("LocalContrast")),
      d(new Private)
{
    d->par = par;
    d->generator.seedByTime();
    initFilter();
}

LocalContrastFilter::~LocalContrastFilter()
{
    cancelFilter();
    delete d;
}

void LocalContrastFilter::filterImage()
{
    if (!m_orgImage.isNull())
    {
        int size = m_orgImage.width() * m_orgImage.height() * 3;
        int i, j;

        d->generator.reseed();

        if (m_orgImage.sixteenBit())
        {
            // sixteen bit image
            QScopedArrayPointer<unsigned short> data(new unsigned short[size]);
            unsigned short* dataImg = reinterpret_cast<unsigned short*>(m_orgImage.bits());

            for (i = 0, j = 0; runningFlag() && (i < size); i += 3, j += 4)
            {
                data[i]     = dataImg[j];
                data[i + 1] = dataImg[j + 1];
                data[i + 2] = dataImg[j + 2];
            }

            postProgress(10);

            process16bitRgbImage(data.data(), m_orgImage.width(), m_orgImage.height());

            for (uint x = 0; runningFlag() && (x < m_orgImage.width()); ++x)
            {
                for (uint y = 0; runningFlag() && (y < m_orgImage.height()); ++y)
                {
                    i = (m_orgImage.width() * y + x) * 3;
                    m_destImage.setPixelColor(x, y, DColor((unsigned short)data[i + 2],
                                                           (unsigned short)data[i + 1],
                                                           (unsigned short)data[i],
                                                           65535, true));
                }
            }

            postProgress(90);
        }
        else
        {
            // eight bit image
            QScopedArrayPointer<uchar> data(new uchar[size]);

            for (i = 0, j = 0; runningFlag() && (i < size); i += 3, j += 4)
            {
                data[i]     = m_orgImage.bits()[j];
                data[i + 1] = m_orgImage.bits()[j + 1];
                data[i + 2] = m_orgImage.bits()[j + 2];
            }

            postProgress(10);

            process8bitRgbImage(data.data(), m_orgImage.width(), m_orgImage.height());

            for (uint x = 0; runningFlag() && (x < m_orgImage.width()); ++x)
            {
                for (uint y = 0; runningFlag() && (y < m_orgImage.height()); ++y)
                {
                    i = (m_orgImage.width() * y + x) * 3;
                    m_destImage.setPixelColor(x, y, DColor(data[i + 2], data[i + 1], data[i], 255, false));
                }
            }

            postProgress(90);
        }
    }

    postProgress(100);
}

void LocalContrastFilter::process8bitRgbImage(unsigned char* const img, int sizex, int sizey)
{
    int size = sizex * sizey;
    QScopedArrayPointer<float> tmpimage(new float[size * 3]);

    for (int i = 0 ; runningFlag() && (i < size * 3) ; ++i)
    {
        // convert to floating point
        tmpimage[i] = (float)(img[i] / 255.0);
    }

    postProgress(20);

    processRgbImage(tmpimage.data(), sizex, sizey);

    // convert back to 8 bits (with dithering)
    int pos = 0;

    for (int i = 0 ; runningFlag() && (i < size) ; ++i)
    {
        float dither = d->generator.number(0.0, 1.0);
        img[pos]     = (int)(tmpimage[pos]     * 255.0 + dither);
        img[pos + 1] = (int)(tmpimage[pos + 1] * 255.0 + dither);
        img[pos + 2] = (int)(tmpimage[pos + 2] * 255.0 + dither);
        pos += 3;
    }

    postProgress(80);
}

void LocalContrastFilter::process16bitRgbImage(unsigned short* const img, int sizex, int sizey)
{
    int size = sizex * sizey;
    QScopedArrayPointer<float> tmpimage(new float[size * 3]);

    for (int i = 0 ; runningFlag() && (i < size * 3) ; ++i)
    {
        // convert to floating point
        tmpimage[i] = (float)(img[i] / 65535.0);
    }

    postProgress(20);

    processRgbImage(tmpimage.data(), sizex, sizey);

    // convert back to 16 bits (with dithering)
    int pos = 0;

    for (int i = 0 ; runningFlag() && (i < size) ; ++i)
    {
        float dither = d->generator.number(0.0, 1.0);
        img[pos]     = (int)(tmpimage[pos]     * 65535.0 + dither);
        img[pos + 1] = (int)(tmpimage[pos + 1] * 65535.0 + dither);
        img[pos + 2] = (int)(tmpimage[pos + 2] * 65535.0 + dither);
        pos += 3;
    }

    postProgress(80);
}

float LocalContrastFilter::func(float x1, float x2)
{
    float result = 0.5;
    float p;

    switch (d->par.functionId)
    {
        case 0:  // power function
        {
            p = (float)(qPow((double)10.0, (double)qFabs((x2 * 2.0 - 1.0)) * d->current_process_power_value * 0.02));

            if (x2 >= 0.5)
            {
                result = qPow(x1, p);
            }
            else
            {
                result = (float)(1.0 - qPow((double)1.0 - x1, (double)p));
            }

            break;
        }

        case 1:  // linear function
        {
            p      = (float)(1.0 / (1 + qExp(-(x2 * 2.0 - 1.0) * d->current_process_power_value * 0.04)));
            result = (x1 < p) ? (float)(x1 * (1.0 - p) / p) : (float)((1.0 - p) + (x1 - p) * p / (1.0 - p));
            break;
        }
    }

    return result;
}

void LocalContrastFilter::blurMultithreaded(uint start, uint stop, float* const img, float* const blurimage)
{
    uint pos = start * 3;

    for (uint i = start ; runningFlag() && (i < stop) ; ++i)
    {
        float src_r  = img[pos];
        float src_g  = img[pos + 1];
        float src_b  = img[pos + 2];

        float blur   = blurimage[i];

        float dest_r = func(src_r, blur);
        float dest_g = func(src_g, blur);
        float dest_b = func(src_b, blur);

        img[pos]     = dest_r;
        img[pos + 1] = dest_g;
        img[pos + 2] = dest_b;

        pos += 3;
    }
}

void LocalContrastFilter::saturationMultithreaded(uint start, uint stop, float* const img, float* const srcimg)
{
    float src_h,  src_s,  src_v;
    float dest_h, dest_s, dest_v;
    float destSaturation, s1;

    uint pos                 = start * 3;
    int  highSaturationValue = 100 - d->par.highSaturation;
    int  lowSaturationValue  = 100 - d->par.lowSaturation;

    for (uint i = start ; runningFlag() && (i < stop) ; ++i)
    {
        rgb2hsv(srcimg[pos], srcimg[pos + 1], srcimg[pos + 2], src_h, src_s, src_v);
        rgb2hsv(img[pos], img[pos + 1], img[pos + 2], dest_h, dest_s, dest_v);

        destSaturation = (float)((src_s * highSaturationValue + dest_s * (100.0 - highSaturationValue)) * 0.01);

        if (dest_v > src_v)
        {
            s1             = (float)(destSaturation * src_v / (dest_v + 1.0 / 255.0));
            destSaturation = (float)((lowSaturationValue * s1 + d->par.lowSaturation * destSaturation) * 0.01);
        }

        hsv2rgb(dest_h, destSaturation, dest_v, img[pos], img[pos + 1], img[pos + 2]);

        pos += 3;
    }
}

void LocalContrastFilter::processRgbImage(float* const img, int sizex, int sizey)
{
    int size = sizex * sizey;
    QScopedArrayPointer<float> blurimage(new float[size]);
    QScopedArrayPointer<float> srcimg(new float[size * 3]);

    for (int i = 0 ; i < (size * 3) ; ++i)
    {
        srcimg[i] = img[i];
    }

    postProgress(30);

    if (d->par.stretchContrast)
    {
        stretchContrast(img, size * 3);
    }

    postProgress(40);

    QList<int> vals = multithreadedSteps(size);
    int         pos  = 0;

    for (int nstage = 0 ; runningFlag() && (nstage < TONEMAPPING_MAX_STAGES) ; ++nstage)
    {
        if (d->par.stage[nstage].enabled)
        {
            // compute the desatured image

            pos = 0;

            for (int i = 0 ; runningFlag() && (i < size) ; ++i)
            {
                blurimage[i] = (float)((img[pos] + img[pos + 1] + img[pos + 2]) / 3.0);
                pos += 3;
            }

            d->current_process_power_value = d->par.getPower(nstage);

            // blur

            inplaceBlur(blurimage.data(), sizex, sizey, d->par.getBlur(nstage));

            QList <QFuture<void> > tasks;

            for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
            {
                tasks.append(QtConcurrent::run(this,
                                               &LocalContrastFilter::blurMultithreaded,
                                               vals[j],
                                               vals[j+1],
                                               img,
                                               blurimage.data()
                                              ));
            }

            foreach(QFuture<void> t, tasks)
                t.waitForFinished();
        }

        postProgress(50 + nstage * 5);
    }

    if ((d->par.highSaturation != 100) || (d->par.lowSaturation != 100))
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "highSaturation : " << d->par.highSaturation;
        qCDebug(DIGIKAM_DIMG_LOG) << "lowSaturation : "  << d->par.lowSaturation;

        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            tasks.append(QtConcurrent::run(this,
                                           &LocalContrastFilter::saturationMultithreaded,
                                           vals[j],
                                           vals[j+1],
                                           img,
                                           srcimg.data()
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();
    }

    postProgress(70);
}

void LocalContrastFilter::inplaceBlurYMultithreaded(const Args& prm)
{
    for (uint y = prm.start ; runningFlag() && (y < prm.stop) ; ++y)
    {
        uint pos  = y * prm.sizex;
        float old = prm.data[pos];
        ++pos;

        for (int x = 1 ; runningFlag() && (x < prm.sizex) ; ++x)
        {
            old           = (prm.data[pos] * (1 - prm.a) + old * prm.a) + prm.denormal_remove;
            prm.data[pos] = old;
            ++pos;
        }

        pos = y * prm.sizex + prm.sizex - 1;

        for (int x = 1 ; runningFlag() && (x < prm.sizex) ; ++x)
        {
            old           = (prm.data[pos] * (1 - prm.a) + old * prm.a) + prm.denormal_remove;
            prm.data[pos] = old;
            pos--;
        }
    }
}

void LocalContrastFilter::inplaceBlurXMultithreaded(const Args& prm)
{
    for (uint x = prm.start ; runningFlag() && (x < prm.stop) ; ++x)
    {
        uint pos  = x;
        float old = prm.data[pos];

        for (int y = 1 ; runningFlag() && (y < prm.sizey) ; ++y)
        {
            old            = (prm.data[pos] * (1 - prm.a) + old * prm.a) + prm.denormal_remove;
            prm.data[pos]  = old;
            pos           += prm.sizex;
        }

        pos = x + prm.sizex * (prm.sizey - 1);

        for (int y = 1 ; runningFlag() && (y < prm.sizey) ; ++y)
        {
            old            = (prm.data[pos] * (1 - prm.a) + old * prm.a) + prm.denormal_remove;
            prm.data[pos]  = old;
            pos           -= prm.sizex;
        }
    }
}

void LocalContrastFilter::inplaceBlur(float* const data, int sizex, int sizey, float blur)
{
    if (blur < 0.3)
    {
        return;
    }

    Args prm;

    prm.a = (float)(qExp(log(0.25) / blur));

    if ((prm.a <= 0.0) || (prm.a >= 1.0))
    {
        return;
    }

    prm.a *= prm.a;
    prm.data            = data;
    prm.sizex           = sizex;
    prm.sizey           = sizey;
    prm.blur            = blur;
    prm.denormal_remove = (float)(1e-15);

    QList<int> valsx = multithreadedSteps(prm.sizex);
    QList<int> valsy = multithreadedSteps(prm.sizey);

    for (uint stage = 0 ; runningFlag() && (stage < 2) ; ++stage)
    {
        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < valsy.count()-1) ; ++j)
        {
            prm.start = valsy[j];
            prm.stop  = valsy[j+1];
            tasks.append(QtConcurrent::run(this,
                                           &LocalContrastFilter::inplaceBlurYMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        tasks.clear();

        for (int j = 0 ; runningFlag() && (j < valsx.count()-1) ; ++j)
        {
            prm.start = valsx[j];
            prm.stop  = valsx[j+1];
            tasks.append(QtConcurrent::run(this,
                                           &LocalContrastFilter::inplaceBlurXMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();
    }
}

void LocalContrastFilter::stretchContrast(float* const data, int datasize)
{
    // stretch the contrast
    const unsigned int histogram_size = 256;
    // first, we compute the histogram
    unsigned int histogram[histogram_size];

    for (unsigned int i = 0 ; i < histogram_size ; ++i)
    {
        histogram[i] = 0;
    }

    for (unsigned int i = 0 ; runningFlag() && (i < (unsigned int)datasize) ; ++i)
    {
        int m = (int)(data[i] * (histogram_size - 1));

        if (m < 0)
        {
            m = 0;
        }

        if (m > (int)(histogram_size - 1))
        {
            m = histogram_size - 1;
        }

        histogram[m]++;
    }

    // I want to strip the lowest and upper 0.1 procents (in the histogram) of the pixels
    int          min         = 0;
    int          max         = 255;
    unsigned int desired_sum = datasize / 1000;
    unsigned int sum_min     = 0;
    unsigned int sum_max     = 0;

    for (unsigned int i = 0 ; runningFlag() && (i < histogram_size) ; ++i)
    {
        sum_min += histogram[i];

        if (sum_min > desired_sum)
        {
            min = i;
            break;
        }
    }

    for (int i = histogram_size - 1 ; runningFlag() && (i >= 0) ; i--)
    {
        sum_max += histogram[i];

        if (sum_max > desired_sum)
        {
            max = i;
            break;
        }
    }

    if (min >= max)
    {
        min = 0;
        max = 255;
    }

    float min_src_val = (float)(min / 255.0);
    float max_src_val = (float)(max / 255.0);

    for (int i = 0 ; runningFlag() && (i < datasize) ; ++i)
    {
        // stretch the contrast
        float x = data[i];
        x       = (x - min_src_val) / (max_src_val - min_src_val);

        if (x < 0.0)
        {
            x = 0.0;
        }

        if (x > 1.0)
        {
            x = 1.0;
        }

        data[i] = x;
    }
}

void LocalContrastFilter::rgb2hsv(const float& r, const float& g, const float& b, float& h, float& s, float& v)
{
    float maxrg = (r > g) ? r : g;
    float max   = (maxrg > b) ? maxrg : b;
    float minrg = (r < g) ? r : g;
    float min   = (minrg < b) ? minrg : b;
    float delta = max - min;

    //hue
    if (min == max)
    {
        h = 0.0;
    }
    else
    {
        if (max == r)
        {
            h = (float)(fmod(60.0 * (g - b) / delta + 360.0, 360.0));
        }
        else
        {
            if (max == g)
            {
                h = (float)(60.0 * (b - r) / delta + 120.0);
            }
            else
            {
                //max==b
                h = (float)(60.0 * (r - g) / delta + 240.0);
            }
        }
    }

    //saturation
    if (max < 1e-6)
    {
        s = 0;
    }
    else
    {
        s = (float)(1.0 - min / max);
    }

    //value
    v = max;
}

void LocalContrastFilter::hsv2rgb(const float& h, const float& s, const float& v, float& r, float& g, float& b)
{
    float hfi = (float)(floor(h / 60.0));
    float f   = (float)((h / 60.0) - hfi);
    int hi    = ((int)hfi) % 6;
    float p   = (float)(v * (1.0 - s));
    float q   = (float)(v * (1.0 - f * s));
    float t   = (float)(v * (1.0 - (1.0 - f) * s));

    switch (hi)
    {
        case 0:
            r = v ;
            g = t ;
            b = p;
            break;

        case 1:
            r = q ;
            g = v ;
            b = p;
            break;

        case 2:
            r = p ;
            g = v ;
            b = t;
            break;

        case 3:
            r = p ;
            g = q ;
            b = v;
            break;

        case 4:
            r = t ;
            g = p;
            b = v;
            break;

        case 5:
            r = v ;
            g = p ;
            b = q;
            break;
    }
}

FilterAction LocalContrastFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("functionId"),      d->par.functionId);
    action.addParameter(QLatin1String("highSaturation"),  d->par.highSaturation);
    action.addParameter(QLatin1String("lowSaturation"),   d->par.lowSaturation);
    action.addParameter(QLatin1String("stretchContrast"), d->par.stretchContrast);

    for (int nstage = 0 ; nstage < TONEMAPPING_MAX_STAGES ; ++nstage)
    {
        QString stage = QString::fromLatin1("stage[%1]:").arg(nstage);
        action.addParameter(stage + QLatin1String("enabled"), d->par.stage[nstage].enabled);

        if (d->par.stage[nstage].enabled)
        {
            action.addParameter(stage + QLatin1String("power"), d->par.stage[nstage].power);
            action.addParameter(stage + QLatin1String("blur"),  d->par.stage[nstage].blur);
        }
    }

    action.addParameter(QLatin1String("randomSeed"), d->generator.currentSeed());

    return action;
}

void LocalContrastFilter::readParameters(const FilterAction& action)
{
    d->par.functionId      = action.parameter(QLatin1String("functionId")).toInt();
    d->par.highSaturation  = action.parameter(QLatin1String("highSaturation")).toInt();
    d->par.lowSaturation   = action.parameter(QLatin1String("lowSaturation")).toInt();
    d->par.stretchContrast = action.parameter(QLatin1String("stretchContrast")).toBool();

    for (int nstage = 0 ; nstage < TONEMAPPING_MAX_STAGES ; ++nstage)
    {
        QString stage                = QString::fromLatin1("stage[%1]:").arg(nstage);
        d->par.stage[nstage].enabled = action.parameter(stage + QLatin1String("enabled")).toBool();

        if (d->par.stage[nstage].enabled)
        {
            d->par.stage[nstage].power = action.parameter(stage + QLatin1String("power")).toFloat();
            d->par.stage[nstage].blur  = action.parameter(stage + QLatin1String("blur")).toFloat();
        }
    }

    d->generator.seed(action.parameter(QLatin1String("randomSeed")).toUInt());
}

} // namespace Digikam
