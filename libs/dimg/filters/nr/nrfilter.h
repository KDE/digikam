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

// Qt includes

#include <QMetaType>
#include <QDebug>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT NRContainer
{

public:

    NRContainer();
    ~NRContainer();

public:

    /** Separated values per chanel
     */
    double thresholds[3];    // Y, Cb, Cr thresholds.
    double softness[3];      // Y, Cb, Cr softness.
};

//! qDebug() stream operator. Writes property @a inf to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const NRContainer& inf);

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

    static void srgb2ycbcr(float** const fimg, int size);

private:

    struct Args
    {
        uint    start;
        uint    stop;
        float*  thold;
        uint*   lpass;
        uint*   hpass;
        double* stdev;
        uint*   samples;
        float** fimg;
        float   threshold;
        double  softness;
    };

private:

    void filterImage();

    void waveletDenoise(float* fimg[3], unsigned int width, unsigned int height,
                        float threshold, double softness);
    inline void hatTransform(float* const temp, float* const base, int st, int size, int sc);

    void ycbcr2srgb(float** const fimg, int size);

    void calculteStdevMultithreaded(const Args& prm);
    void thresholdingMultithreaded(const Args& prm);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* NRFILTER_H */
