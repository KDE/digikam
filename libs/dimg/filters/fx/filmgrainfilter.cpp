/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : filter to add Film Grain to image.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010      by Julien Narboux <julien at narboux dot fr>
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

#include "filmgrainfilter.h"

// C++ includes

#include <cstdlib>
#include <cmath>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "globals.h"

namespace Digikam
{

class FilmGrainFilterPriv
{
public:

    FilmGrainFilterPriv(){}

    enum YUVChannel
    {
        Luma = 0,
        ChromaBlue,
        ChromaRed
    };

    FilmGrainContainer settings;
};

FilmGrainFilter::FilmGrainFilter(DImg* orgImage, QObject* parent, const FilmGrainContainer& settings)
               : DImgThreadedFilter(orgImage, parent, "FilmGrain"),
                 d(new FilmGrainFilterPriv)
{
    d->settings = settings;
    initFilter();
}

FilmGrainFilter::FilmGrainFilter(DImgThreadedFilter* parentFilter,
                                 const DImg& orgImage, const DImg& destImage,
                                 int progressBegin, int progressEnd,
                                 const FilmGrainContainer& settings)
               : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                                    parentFilter->filterName() + ": FilmGrain"),
                 d(new FilmGrainFilterPriv)
{
    d->settings = settings;
    filterImage();
}

FilmGrainFilter::~FilmGrainFilter()
{
    delete d;
}

/** This method have been implemented following this report in bugzilla :
    https://bugs.kde.org/show_bug.cgi?id=148540
    We use YCbCr color space to perform noise addition. Please follow this url for
    details about this color space :
    http://en.allexperts.com/e/y/yc/ycbcr.htm
 */
void FilmGrainFilter::filterImage()
{
    if (d->settings.lumaIntensity <= 0)       return;
    if (d->settings.chromaBlueIntensity <= 0) return;
    if (d->settings.chromaRedIntensity <= 0)  return;

    DColor refCol, color;
    int    progress, posX, posY;
    double local_luma_noise, local_luma_range;
    double local_chroma_blue_noise, local_chroma_blue_range;
    double local_chroma_red_noise, local_chroma_red_range;

    int    width             = m_orgImage.width();
    int    height            = m_orgImage.height();
    bool   sb                = m_orgImage.sixteenBit();
    int    luma_noise        = d->settings.lumaIntensity       * (sb ? 256.0 : 1.0);
    double chroma_blue_noise = d->settings.chromaBlueIntensity * (sb ? 256.0 : 1.0);
    double chroma_red_noise  = d->settings.chromaRedIntensity  * (sb ? 256.0 : 1.0);

    qsrand(1); // noise will always be the same

    for (int x = 0; !m_cancel && x < width; x += d->settings.grainSize)
    {
        for (int y = 0; !m_cancel && y < height; y += d->settings.grainSize)
        {
            refCol = m_orgImage.getPixelColor(x, y);

            if (d->settings.addLuminanceNoise)
            {
                local_luma_range = interpolate(d->settings.lumaShadows, d->settings.lumaMidtones, 
                                               d->settings.lumaHighlights, refCol) * luma_noise + 1.0;
                local_luma_noise = randomizeUniform(local_luma_range, sb);
            }

            if (d->settings.addChrominanceBlueNoise)
            {
                local_chroma_blue_range = interpolate(d->settings.chromaBlueShadows, d->settings.chromaBlueMidtones, 
                                                      d->settings.chromaBlueHighlights, refCol) * chroma_blue_noise + 1.0;
                local_chroma_blue_noise = randomizeUniform(local_chroma_blue_range, sb);
            }

            if (d->settings.addChrominanceRedNoise)
            {
                local_chroma_red_range = interpolate(d->settings.chromaRedShadows, d->settings.chromaRedMidtones, 
                                                     d->settings.chromaRedHighlights, refCol) * chroma_red_noise + 1.0;
                local_chroma_red_noise = randomizeUniform(local_chroma_red_range, sb);
            }

            for (int zx = 0; !m_cancel && zx < d->settings.grainSize; ++zx)
            {
                for (int zy = 0; !m_cancel && zy < d->settings.grainSize; ++zy)
                {
                    posX  = x + zx;
                    posY  = y + zy;
                    if (posX < width && posY < height)
                    {
                        color = m_orgImage.getPixelColor(posX, posY);

                        if (d->settings.addLuminanceNoise)
                            adjustYCbCr(color, local_luma_range, local_luma_noise, 
                                        FilmGrainFilterPriv::Luma);

                        if (d->settings.addChrominanceBlueNoise)
                            adjustYCbCr(color, local_chroma_blue_range, local_chroma_blue_noise,
                                        FilmGrainFilterPriv::ChromaBlue);

                        if (d->settings.addChrominanceRedNoise)
                            adjustYCbCr(color, local_chroma_red_range, local_chroma_red_noise,
                                        FilmGrainFilterPriv::ChromaRed);

                        m_destImage.setPixelColor(posX, posY, color);
                    }
                }
            }
        }

        // Update progress bar in dialog.
        progress = (int) (((double)x * 100.0) / width);

        if (progress%5 == 0)
            postProgress( progress );
    }
}

void FilmGrainFilter::adjustYCbCr(DColor& col, double range, double nRand, int channel)
{
    double y, cb, cr, n2;
    col.getYCbCr(&y, &cb, &cr);

    if (d->settings.photoDistribution)
        n2 = randomizePoisson((d->settings.grainSize/2.0)*(range/1.414), d->settings.grainSize, col.sixteenBit());
    else 
        n2 = randomizeGauss((d->settings.grainSize/2.0)*(range/1.414), col.sixteenBit());

    switch (channel)
    {
        case FilmGrainFilterPriv::Luma:
            y  = CLAMP(y  + nRand + n2, 0.0, 1.0);
            break;
        case FilmGrainFilterPriv::ChromaBlue:
            cb = CLAMP(cb + nRand + n2, 0.0, 1.0);
            break;
        default:       // ChromaRed
            cr = CLAMP(cr + nRand + n2, 0.0, 1.0);
            break;
    }
    col.setYCbCr(y, cb, cr, col.sixteenBit());
}

double FilmGrainFilter::randomizeUniform(double range, bool sixteenbits)
{
    return ((double)(qrand() % (int)range) - range/2.0) / (sixteenbits ? 65535.0 : 255.0);
}

double FilmGrainFilter::randomizeGauss(double sigma, bool sixteenbits)
{
  double u, v;
  while ((u = qrand () / (double) RAND_MAX) == 0.0);
  v = qrand () / (double) RAND_MAX;
  return (sigma * sqrt(-2 * log (u)) * cos(2 * M_PI * v)) / (sixteenbits ? 65535.0 : 255.0);
}

double FilmGrainFilter::randomizePoisson(double lambda, int photonNumber, bool sixteenbits)
{
    if (photonNumber == 0)
        return 0.0;

    uint   r = (sixteenbits ? 65535 : 255);
    double L = exp (-lambda);
    uint   k = 0;
    double p = 1.0;
    do
    {
        k++;
        p = p * qrand () / (double)RAND_MAX;
    }
    while (p >= L && k <= r);
    return (((double)k - 1.0 + randomizePoisson(lambda, photonNumber-1, sixteenbits)) / (double)r);
}

double FilmGrainFilter::interpolate(int shadows, int midtones, int highlights, const DColor& col)
{
    double s = (shadows    + 100.0) / 200.0;
    double m = (midtones   + 100.0) / 200.0; 
    double h = (highlights + 100.0) / 200.0;

    double y, cb, cr;
    col.getYCbCr(&y, &cb, &cr);

    if (y >= 0.0 && y <= 0.5)
    {
        return (s+2*(m-s)*y);
    }
    else if (y >= 0.5 && y <= 1.0)
    {
        return (2*(h-m)*y+2*m-h);
    }
    else
    {
        return 1.0;
    }
}

}  // namespace Digikam
