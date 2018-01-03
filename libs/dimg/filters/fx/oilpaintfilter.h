/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Oil Painting threaded image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef OILPAINTFILTER_H
#define OILPAINTFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DIGIKAM_EXPORT OilPaintFilter : public DImgThreadedFilter
{

public:

    explicit OilPaintFilter(QObject* const parent = 0);
    explicit OilPaintFilter(DImg* const orgImage, QObject* const parent=0, int brushSize=1, int smoothness=30);
    ~OilPaintFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:OilPaintFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Oil Painter Effect"));
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
    void oilPaintImageMultithreaded(uint start, uint stop);
    DColor MostFrequentColor(DImg& src, int X, int Y, int Radius, int Intensity,
                             uchar* intensityCount, uint* averageColorR, uint* averageColorG, uint* averageColorB);
    inline double GetIntensity(uint Red, uint Green, uint Blue);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* OILPAINTFILTER_H */
