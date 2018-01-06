/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Sharpen threaded image filter.
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

#ifndef SHARPENFILTER_H
#define SHARPENFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DIGIKAM_EXPORT SharpenFilter : public DImgThreadedFilter
{

public:

    explicit SharpenFilter(QObject* const parent = 0);
    explicit SharpenFilter(DImg* const orgImage, QObject* const parent=0, double radius=0.0, double sigma=1.0);

    // Constructor for slave mode: execute immediately in current thread with specified master filter
    SharpenFilter(DImgThreadedFilter* const parentFilter, const DImg& orgImage, const DImg& destImage,
                  int progressBegin=0, int progressEnd=100, double radius=0.0, double sigma=1.0);

    ~SharpenFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:SharpenFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Sharpen"));
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

    struct Args
    {
        uint    start;
        uint    stop;
        uint    y;
        long    kernelWidth;
        double* normal_kernel;
        long    halfKernelWidth;
    };

private:

    void filterImage();

    void sharpenImage(double radius, double sigma);

    bool convolveImage(const unsigned int order, const double* const kernel);

    void convolveImageMultithreaded(const Args& prm);

    int  getOptimalKernelWidth(double radius, double sigma);

private:

    double m_radius;
    double m_sigma;
};

}  // namespace Digikam

#endif /* SHARPENFILTER_H */
