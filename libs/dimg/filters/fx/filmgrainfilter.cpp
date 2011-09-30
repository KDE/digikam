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
#include "randomnumbergenerator.h"

namespace Digikam
{

class FilmGrainFilterPriv
{
public:

    FilmGrainFilterPriv() :
        div(0.0),
        leadLumaNoise(1.0),
        leadChromaBlueNoise(1.0),
        leadChromaRedNoise(1.0)
    {
    }

    enum YUVChannel
    {
        Luma = 0,
        ChromaBlue,
        ChromaRed
    };

    double div;
    double leadLumaNoise;
    double leadChromaBlueNoise;
    double leadChromaRedNoise;

    FilmGrainContainer settings;

    RandomNumberGenerator generator;
};

FilmGrainFilter::FilmGrainFilter(QObject* parent)
    : DImgThreadedFilter(parent),
      d(new FilmGrainFilterPriv)
{
    initFilter();
}

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
    cancelFilter();
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
    if (d->settings.lumaIntensity <= 0       ||
        d->settings.chromaBlueIntensity <= 0 ||
        d->settings.chromaRedIntensity <= 0  ||
        !d->settings.isDirty())
    {
        m_destImage = m_orgImage;
        return;
    }

    // To emulate grain size we use a matrix [grainSize x grainSize].
    // We will parse whole image using grainSize step. Color from a reference point located
    // on the top left corner of matrix will be used to apply noise on whole matrix.
    // In first, for each matrix processed over the image, we compute the lead noise value
    // using Uniform noise generator.
    // In second time, all others points from the matrix are process to add suplemental noise
    // generated with Gaussian or Poisson noise generator.

    DColor refCol, matCol;
    int    progress, posX, posY;

    // Reference point noise adjustements.
    double refLumaNoise=0.0,       refLumaRange=0.0;
    double refChromaBlueNoise=0.0, refChromaBlueRange=0.0;
    double refChromaRedNoise=0.0,  refChromaRedRange=0.0;

    // Current matrix point noise adjustements.
    double matLumaNoise=0.0,       matLumaRange=0.0;
    double matChromaBlueNoise=0.0, matChromaBlueRange=0.0;
    double matChromaRedNoise=0.0,  matChromaRedRange=0.0;

    int    width           = m_orgImage.width();
    int    height          = m_orgImage.height();
    d->div                 = m_orgImage.sixteenBit() ? 65535.0 : 255.0;
    d->leadLumaNoise       = d->settings.lumaIntensity       * (m_orgImage.sixteenBit() ? 256.0 : 1.0);
    d->leadChromaBlueNoise = d->settings.chromaBlueIntensity * (m_orgImage.sixteenBit() ? 256.0 : 1.0);
    d->leadChromaRedNoise  = d->settings.chromaRedIntensity  * (m_orgImage.sixteenBit() ? 256.0 : 1.0);

    d->generator.seed(1); // noise will always be the same

    for (int x = 0; runningFlag() && x < width; x += d->settings.grainSize)
    {
        for (int y = 0; runningFlag() && y < height; y += d->settings.grainSize)
        {
            refCol = m_orgImage.getPixelColor(x, y);
            computeNoiseSettings(refCol,
                                 refLumaRange,       refLumaNoise,
                                 refChromaBlueRange, refChromaBlueNoise,
                                 refChromaRedRange,  refChromaRedNoise);

            // Grain size matrix processing.

            for (int zx = 0; runningFlag() && zx < d->settings.grainSize; ++zx)
            {
                for (int zy = 0; runningFlag() && zy < d->settings.grainSize; ++zy)
                {
                    posX = x + zx;
                    posY = y + zy;

                    if (posX < width && posY < height)
                    {
                        matCol = m_orgImage.getPixelColor(posX, posY);

                        computeNoiseSettings(matCol,
                                             matLumaRange,       matLumaNoise,
                                             matChromaBlueRange, matChromaBlueNoise,
                                             matChromaRedRange,  matChromaRedNoise);

                        if (d->settings.addLuminanceNoise)
                        {
                            if (((refLumaRange - matLumaRange) / refLumaRange) > 0.1)
                            {
                                adjustYCbCr(matCol, matLumaRange, matLumaNoise, FilmGrainFilterPriv::Luma);
                            }
                            else
                            {
                                adjustYCbCr(matCol, refLumaRange, refLumaNoise, FilmGrainFilterPriv::Luma);
                            }
                        }

                        if (d->settings.addChrominanceBlueNoise)
                        {
                            if (((refChromaBlueRange - matChromaBlueRange) / refChromaBlueRange) > 0.1)
                            {
                                adjustYCbCr(matCol, matChromaBlueRange, matChromaBlueNoise, FilmGrainFilterPriv::ChromaBlue);
                            }
                            else
                            {
                                adjustYCbCr(matCol, refChromaBlueRange, refChromaBlueNoise, FilmGrainFilterPriv::ChromaBlue);
                            }
                        }

                        if (d->settings.addChrominanceRedNoise)
                        {
                            if (((refChromaRedRange - matChromaRedRange) / refChromaRedRange) > 0.1)
                            {
                                adjustYCbCr(matCol, matChromaRedRange, matChromaRedNoise, FilmGrainFilterPriv::ChromaBlue);
                            }
                            else
                            {
                                adjustYCbCr(matCol, refChromaRedRange, refChromaRedNoise, FilmGrainFilterPriv::ChromaRed);
                            }
                        }

                        m_destImage.setPixelColor(posX, posY, matCol);
                    }
                }
            }
        }

        // Update progress bar in dialog.
        progress = (int) (((double)x * 100.0) / width);

        if (progress%5 == 0)
        {
            postProgress( progress );
        }
    }
}

/** This method compute lead noise of reference matrix point used to similate graininess size
 */
void FilmGrainFilter::computeNoiseSettings(const DColor& col,
        double& luRange, double& luNoise,
        double& cbRange, double& cbNoise,
        double& crRange, double& crNoise)
{
    if (d->settings.addLuminanceNoise)
    {
        luRange = interpolate(d->settings.lumaShadows, d->settings.lumaMidtones,
                              d->settings.lumaHighlights, col) * d->leadLumaNoise + 1.0;
        luNoise = randomizeUniform(luRange);
    }

    if (d->settings.addChrominanceBlueNoise)
    {
        cbRange = interpolate(d->settings.chromaBlueShadows, d->settings.chromaBlueMidtones,
                              d->settings.chromaBlueHighlights, col) * d->leadChromaBlueNoise + 1.0;
        cbNoise = randomizeUniform(cbRange);
    }

    if (d->settings.addChrominanceRedNoise)
    {
        crRange = interpolate(d->settings.chromaRedShadows, d->settings.chromaRedMidtones,
                              d->settings.chromaRedHighlights, col) * d->leadChromaRedNoise + 1.0;
        crNoise = randomizeUniform(crRange);
    }
}

/** This method apply grain adjustement on a pixel color channel from YCrCb color space.
    NRand is the lead uniform noise set from matrix used to scan whole image step by step.
    Additionally noise is applied on pixel using Poisson or Gausian distribution.
 */
void FilmGrainFilter::adjustYCbCr(DColor& col, double range, double nRand, int channel)
{
    double y, cb, cr, n2;
    col.getYCbCr(&y, &cb, &cr);

    if (d->settings.photoDistribution)
    {
        n2 = randomizePoisson((d->settings.grainSize/2.0)*(range/1.414));
    }
    else
    {
        n2 = randomizeGauss((d->settings.grainSize/2.0)*(range/1.414));
    }

    switch (channel)
    {
        case FilmGrainFilterPriv::Luma:
            y  = CLAMP(y  + (nRand + n2)/ d->div, 0.0, 1.0);
            break;
        case FilmGrainFilterPriv::ChromaBlue:
            cb = CLAMP(cb + (nRand + n2)/ d->div, 0.0, 1.0);
            break;
        default:       // ChromaRed
            cr = CLAMP(cr + (nRand + n2)/ d->div, 0.0, 1.0);
            break;
    }

    col.setYCbCr(y, cb, cr, col.sixteenBit());
}

/** This method compute uniform noise value used to randomize matrix reference point.
    This value is lead noise apply to image.
 */
double FilmGrainFilter::randomizeUniform(double range)
{
    return d->generator.number(- range/2, range/2);
}

/** This method compute Guaussian noise value used to randomize all matrix points.
    This value is added to lead noise value.
 */
double FilmGrainFilter::randomizeGauss(double sigma)
{
    double u = - d->generator.number(-1.0, 0.0); // exclude 0
    double v = d->generator.number(0.0, 1.0);
    return (sigma * sqrt(-2 * log (u)) * cos(2 * M_PI * v)) ;
}

/** This method compute Poisson noise value used to randomize all matrix points.
    This value is added to lead noise value.
    Poisson noise is more realist to simulate photon noise apply on analog film.
    NOTE: see approximation of Poisson noise using Gauss algorithm from noise.c code take from :
          http://registry.gimp.org/node/13016
          This method is very fast compared to real Poisson noise generator.
 */
double FilmGrainFilter::randomizePoisson(double lambda)
{
    return (randomizeGauss( sqrt( lambda * d->settings.grainSize * d->settings.grainSize) ));
}

/** This method interpolate gain adjustements to apply grain on shadows, midtones and highlights colors.
    The output value is a coefficient computed between 0.0 and 1.0.
 */
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

FilterAction FilmGrainFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter("grainSize", d->settings.grainSize);
    action.addParameter("photoDistribution", d->settings.photoDistribution);

    action.addParameter("addLuminanceNoise", d->settings.addLuminanceNoise);
    action.addParameter("lumaIntensity", d->settings.lumaIntensity);
    action.addParameter("lumaShadows", d->settings.lumaShadows);
    action.addParameter("lumaMidtones", d->settings.lumaMidtones);
    action.addParameter("lumaHighlights", d->settings.lumaHighlights);

    action.addParameter("addChrominanceBlueNoise", d->settings.addChrominanceBlueNoise);
    action.addParameter("chromaBlueIntensity", d->settings.chromaBlueIntensity);
    action.addParameter("chromaBlueShadows", d->settings.chromaBlueShadows);
    action.addParameter("chromaBlueMidtones", d->settings.chromaBlueMidtones);
    action.addParameter("chromaBlueHighlights", d->settings.chromaBlueHighlights);

    action.addParameter("addChrominanceRedNoise", d->settings.addChrominanceRedNoise);
    action.addParameter("chromaRedIntensity", d->settings.chromaRedIntensity);
    action.addParameter("chromaRedShadows", d->settings.chromaRedShadows);
    action.addParameter("chromaRedMidtones", d->settings.chromaRedMidtones);
    action.addParameter("chromaRedHighlights", d->settings.chromaRedHighlights);

    return action;
}

void FilmGrainFilter::readParameters(const Digikam::FilterAction& action)
{
    d->settings.grainSize               = action.parameter("grainSize").toInt();
    d->settings.photoDistribution       = action.parameter("photoDistribution").toBool();

    d->settings.addLuminanceNoise       = action.parameter("addLuminanceNoise").toBool();
    d->settings.lumaIntensity           = action.parameter("lumaIntensity").toInt();
    d->settings.lumaShadows             = action.parameter("lumaShadows").toInt();
    d->settings.lumaMidtones            = action.parameter("lumaMidtones").toInt();
    d->settings.lumaHighlights          = action.parameter("lumaHighlights").toInt();

    d->settings.addChrominanceBlueNoise = action.parameter("addChrominanceBlueNoise").toBool();
    d->settings.chromaBlueIntensity     = action.parameter("chromaBlueIntensity").toInt();
    d->settings.chromaBlueShadows       = action.parameter("chromaBlueShadows").toInt();
    d->settings.chromaBlueMidtones      = action.parameter("chromaBlueMidtones").toInt();
    d->settings.chromaBlueHighlights    = action.parameter("chromaBlueHighlights").toInt();

    d->settings.addChrominanceRedNoise  = action.parameter("addChrominanceRedNoise").toBool();
    d->settings.chromaRedIntensity      = action.parameter("chromaRedIntensity").toInt();
    d->settings.chromaRedShadows        = action.parameter("chromaRedShadows").toInt();
    d->settings.chromaRedMidtones       = action.parameter("chromaRedMidtones").toInt();
    d->settings.chromaRedHighlights     = action.parameter("chromaRedHighlights").toInt();
}



}  // namespace Digikam
