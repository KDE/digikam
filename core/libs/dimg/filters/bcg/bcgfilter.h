/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : a Brightness/Contrast/Gamma image filter.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
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

#ifndef BCGFILTER_H
#define BCGFILTER_H

// Local includes

#include "bcgcontainer.h"
#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT BCGFilter : public DImgThreadedFilter
{

public:

    explicit BCGFilter(QObject* const parent=0);
    explicit BCGFilter(DImg* const orgImage, QObject* const parent=0, const BCGContainer& settings=BCGContainer());
    explicit BCGFilter(const BCGContainer& settings, DImgThreadedFilter* const master,
                       const DImg& orgImage, const DImg& destImage, int progressBegin=0, int progressEnd=100);
    virtual ~BCGFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:BCGFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Brightness / Contrast / Gamma Filter"));
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
    void setGamma(double val);
    void setBrightness(double val);
    void setContrast(double val);
    void applyBCG(DImg& image);
    void applyBCG(uchar* const bits, uint width, uint height, bool sixteenBits);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* BCGFILTER_H */
