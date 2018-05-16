/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : stretch contrast image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_STRETCH_FILTER_H
#define DIGIKAM_STRETCH_FILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT StretchFilter : public DImgThreadedFilter
{

public:

    explicit StretchFilter(QObject* const parent = 0);
    StretchFilter(DImg* const orgImage, const DImg* const refImage, QObject* const parent=0);
    virtual ~StretchFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:StretchFilter");
    }

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int              CurrentVersion()
    {
        return 1;
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Stretch Contrast"));
    }

    virtual QString         filterIdentifier() const
    {
        return FilterIdentifier();
    }

    virtual FilterAction    filterAction();

    void                    readParameters(const FilterAction& action);

private:

    void filterImage();
    void stretchContrastImage();

private:

    struct double_packet
    {
        double_packet()
            : red(0.0),
              green(0.0),
              blue(0.0),
              alpha(0.0)
        {
        }

        double red;
        double green;
        double blue;
        double alpha;
    };

    struct int_packet
    {
       int_packet()
            : red(0),
              green(0),
              blue(0),
              alpha(0)
        {
        }

        unsigned int red;
        unsigned int green;
        unsigned int blue;
        unsigned int alpha;
    };

    DImg m_refImage;
};

} // namespace Digikam

#endif // DIGIKAM_STRETCH_FILTER_H
