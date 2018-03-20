/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : Hue/Saturation/Lightness image filter.
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
 * ============================================================ */

#ifndef HSLFILTER_H
#define HSLFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT HSLContainer
{

public:

    HSLContainer()
    {
        hue        = 0.0;
        saturation = 0.0;
        vibrance   = 0.0;
        lightness  = 0.0;
    };

    ~HSLContainer() {};

public:

    double hue;
    double saturation;
    double vibrance;
    double lightness;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT HSLFilter : public DImgThreadedFilter
{

public:

    explicit HSLFilter(QObject* const parent = 0);
    explicit HSLFilter(DImg* const orgImage, QObject* const parent=0, const HSLContainer& settings=HSLContainer());
    virtual ~HSLFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:HSLFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Hue / Saturation / Lightness Filter"));
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

    void reset();
    void setHue(double val);
    void setSaturation(double val);
    void setLightness(double val);
    void applyHSL(DImg& image);
    int  vibranceBias(double sat, double hue, double vib, bool sixteenbit);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // HSLFILTER_H
