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

    ~NRContainer()
    {
    };

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

    explicit NRFilter(QObject* const parent = 0);
    NRFilter(DImg* const orgImage, QObject* const parent, const NRContainer& settings);
    ~NRFilter();

    void readParameters(const FilterAction& action);

    virtual FilterAction    filterAction();
    virtual QString         filterIdentifier() const;

    static QString          FilterIdentifier();
    static QString          DisplayableName();
    static QList<int>       SupportedVersions();
    static int              CurrentVersion();

private:

    void filterImage();

    void waveletDenoise(float* const fimg[3], unsigned int width, unsigned int height,
                        float threshold, double softness);
    inline void hatTransform(float* const temp, float* const base, int st, int size, int sc);

    void srgb2ycbcr(float** const fimg, int size);
    void ycbcr2srgb(float** const fimg, int size);

    // Methods not used.
/*
    void srgb2lab(float** const fimg, int size);
    void lab2srgb(float** const fimg, int size);
    void srgb2xyz(float** const fimg, int size);
    void xyz2srgb(float** const fimg, int size);
*/
private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* NRFILTER_H */
