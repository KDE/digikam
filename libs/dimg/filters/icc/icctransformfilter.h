/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-19
 * Description : ICC Transform threaded image filter.
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ICCTRANSFORMFILTER_H
#define ICCTRANSFORMFILTER_H

// Local includes

#include "dimgloaderobserver.h"
#include "dimgthreadedfilter.h"
#include "icctransform.h"

namespace Digikam
{

class DIGIKAM_EXPORT IccTransformFilter : public DImgThreadedFilter,
                                          public DImgLoaderObserver
{

public:

    explicit IccTransformFilter(QObject* const parent = 0);
    explicit IccTransformFilter(DImg* const orgImage, QObject* const parent, const IccTransform& transform);
    ~IccTransformFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:IccTransformFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Color Profile Conversion"));
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
    bool                    parametersSuccessfullyRead() const;
    QString                 readParametersError(const FilterAction& actionThatFailed) const;

protected:

    virtual void progressInfo(const DImg* const, float progress);
    virtual void filterImage();

private:

    IccTransform m_transform;
};

}  // namespace Digikam

#endif /* DIMGREFOCUS_H */
