/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Wavelets Noise Reduction threaded image filter.
 *               This filter work in YCrCb color space.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef NRFILTER_H
#define NRFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT NRContainer
{

public:

    NRContainer()
    {
        thresholds[0] = 1.2;     // Y
        thresholds[1] = 1.2;     // Cr
        thresholds[2] = 1.2;     // Cb
        softness[0]   = 0.9;     // Y
        softness[1]   = 0.9;     // Cr
        softness[2]   = 0.9;     // Cb
    };

    ~NRContainer() {};

public:

    /** Separated values per chanel
     */
    double thresholds[3];    // Y, Cr, Cb thresholds.
    double softness[3];      // Y, Cr, Cb softness.
};

// --------------------------------------------------------------------------

class DIGIKAM_EXPORT NRFilter : public DImgThreadedFilter
{

public:

    explicit NRFilter(QObject* parent = 0);
    NRFilter(DImg* orgImage, QObject* parent, const NRContainer& settings);
    ~NRFilter();

    static QString          FilterIdentifier()
    {
        return "digikam:NoiseReductionFilter";
    }
    static QString          DisplayableName()
    {
        return I18N_NOOP("Noise Reduction Filter");
    }
    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 1;
    }
    static int              CurrentVersion()
    {
        return 1;
    }

    virtual QString         filterIdentifier() const
    {
        return FilterIdentifier();
    }
    virtual FilterAction    filterAction();
    void                    readParameters(const FilterAction& action);

private:

    void filterImage();

    void waveletDenoise(float* fimg[3], unsigned int width, unsigned int height,
                        float threshold, double softness);
    inline void hatTransform(float* temp, float* base, int st, int size, int sc);

    void srgb2ycbcr(float** fimg, int size);
    void ycbcr2srgb(float** fimg, int size);

    // Methods not used.
    void srgb2lab(float** fimg, int size);
    void lab2srgb(float** fimg, int size);
    void srgb2xyz(float** fimg, int size);
    void xyz2srgb(float** fimg, int size);

private:

    class NRFilterPriv;
    NRFilterPriv* const d;
};

}  // namespace Digikam

#endif /* NRFILTER_H */
