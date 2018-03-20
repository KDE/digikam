/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Charcoal threaded image filter.
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

#ifndef CHARCOALFILTER_H
#define CHARCOALFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DIGIKAM_EXPORT CharcoalFilter : public DImgThreadedFilter
{

public:

    explicit CharcoalFilter(QObject* const parent = 0);
    explicit CharcoalFilter(DImg* const orgImage, QObject* const parent=0, double pencil=5.0, double smooth=10.0);
    ~CharcoalFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:CharcoalFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Charcoal Effect"));
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
    bool convolveImage(const unsigned int order, const double* kernel);
    int  getOptimalKernelWidth(double radius, double sigma);
    void convolveImageMultithreaded(uint start, uint stop, double* normal_kernel, double kernelWidth);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* CHARCOALFILTER_H */
