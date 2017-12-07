/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : lens distortion algorithm.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2001-2003 by David Hodson <hodsond@acm.org>
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

#ifndef LENS_DISTORTION_FILTER_H
#define LENS_DISTORTION_FILTER_H

// Local includes

#include "dimgthreadedfilter.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT LensDistortionFilter : public DImgThreadedFilter
{

public:

    explicit LensDistortionFilter(QObject* const parent = 0);
    explicit LensDistortionFilter(DImg* const orgImage, QObject* const parent=0, double main=0.0,
                                  double edge=0.0, double rescale=0.0, double brighten=0.0,
                                  int center_x=0, int center_y=0);

    ~LensDistortionFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:LensDistortionFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Lens Distortion Tool"));
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

private:

    int    m_centre_x;
    int    m_centre_y;

    double m_main;
    double m_edge;
    double m_rescale;
    double m_brighten;
};

}  // namespace Digikam

#endif /* LENS_DISTORTION_FILTER_H */
