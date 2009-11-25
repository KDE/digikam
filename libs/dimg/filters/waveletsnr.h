/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Wavelets Noise Reduction threaded image filter.
 *               This filter work in YCrCb color space.
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef WAVELETS_NR_H
#define WAVELETS_NR_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class WaveletsNRPriv;

class DIGIKAM_EXPORT WaveletsNRContainer
{

public:

    WaveletsNRContainer()
    {
        leadThreshold = 1.2;
        leadSoftness  = 0.9;
        advanced      = false;
        thresholds[0] = 1.2;     // Y
        thresholds[1] = 1.2;     // Cr
        thresholds[2] = 1.2;     // Cb
        softness[0]   = 0.9;     // Y
        softness[1]   = 0.9;     // Cr
        softness[2]   = 0.9;     // Cb
    };

    ~WaveletsNRContainer(){};

public:

    /** If false thresholds and softness values are the same for Y, Cr, and Cb
        else, each chanel has a dedicated value.
     */
    bool   advanced;

    /** General settings if advanced is false
     */
    double leadThreshold;
    double leadSoftness;

    /** Separated values per chanel
     */
    double thresholds[3];    // Y, Cr, Cb thresholds.
    double softness[3];      // Y, Cr, Cb softness.
};

// --------------------------------------------------------------------------

class DIGIKAM_EXPORT WaveletsNR : public DImgThreadedFilter
{

public:

    WaveletsNR(DImg* orgImage, QObject* parent, const WaveletsNRContainer& settings);
    ~WaveletsNR();

private:

    void filterImage();

    void waveletDenoise(float* fimg[3], unsigned int width, unsigned int height,
                        float threshold, double softness);
    inline void hatTransform(float* temp, float *base, int st, int size, int sc);

    void srgb2ycbcr(float** fimg, int size);
    void ycbcr2srgb(float** fimg, int size);

    // Methods not used.
    void srgb2lab(float** fimg, int size);
    void lab2srgb(float** fimg, int size);
    void srgb2xyz(float** fimg, int size);
    void xyz2srgb(float** fimg, int size);

private:

    WaveletsNRPriv* const d;
};

}  // namespace Digikam

#endif /* WAVELETS_NR_H */
