/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-25-02
 * Description : Curves image filter
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CURVESFILTER_H
#define CURVESFILTER_H

// Qt includes

#include <QPolygon>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"
#include "imagecurves.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT CurvesFilter : public DImgThreadedFilter
{

public:

    explicit CurvesFilter(QObject* const parent = 0);
    explicit CurvesFilter(DImg* const orgImage, QObject* const parent=0, const CurvesContainer& settings=CurvesContainer());
    explicit CurvesFilter(const CurvesContainer& settings, DImgThreadedFilter* const master,
                          const DImg& orgImage, DImg& destImage, int progressBegin=0, int progressEnd=100);
    virtual ~CurvesFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:CurvesFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Adjust Curves"));
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

    CurvesContainer m_settings;
};

} // namespace Digikam

#endif // CURVESFILTER_H
