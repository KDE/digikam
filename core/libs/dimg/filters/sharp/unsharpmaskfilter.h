/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Unsharp Mask threaded image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Matthias Welwarsky <matze at welwarsky dot de>
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

#ifndef UNSHARPMASKFILTER_H
#define UNSHARPMASKFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DIGIKAM_EXPORT UnsharpMaskFilter : public DImgThreadedFilter
{

public:

    explicit UnsharpMaskFilter(QObject* const parent = 0);
    explicit UnsharpMaskFilter(DImg* const orgImage, QObject* const parent=0, double radius=1.0,
                               double amount=1.0, double threshold=0.05, bool luma=false);

    // Constructor for slave mode: execute immediately in current thread with specified master filter
    // UnsharpMaskFilter(DImgThreadedFilter *parentFilter, const DImg& orgImage, const DImg& destImage,
    //            int progressBegin=0, int progressEnd=100, double radius=0.0, double sigma=1.0);

    virtual ~UnsharpMaskFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:UnsharpMaskFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Unsharp Mask Tool"));
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
    void unsharpMaskMultithreaded(uint start, uint stop, uint y);

private:

    double m_radius;

    double m_amount;
    double m_threshold;
    bool   m_luma;
};

} // namespace Digikam

#endif // UNSHARPMASKFILTER_H
