/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 16/08/2016
 * Description : A Red-Eye automatic detection and correction filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2016      by Omar Amin <Omar dot moh dot amin at gmail dot com>
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

#ifndef REDEYECORRECTIONFILTER_H
#define REDEYECORRECTIONFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"
#include "libopencv.h"
#include "redeyecorrectioncontainer.h"

namespace Digikam
{

class DIGIKAM_EXPORT RedEyeCorrectionFilter : public DImgThreadedFilter
{

public:

    explicit RedEyeCorrectionFilter(QObject* const parent = 0);
    explicit RedEyeCorrectionFilter(DImg* const orgImage, QObject* const parent=0,
                                    const RedEyeCorrectionContainer& settings=RedEyeCorrectionContainer());

    // Constructor for slave mode: execute immediately in current thread with specified master filter
    explicit RedEyeCorrectionFilter(const RedEyeCorrectionContainer& settings,
                                    DImgThreadedFilter* const parentFilter, const DImg& orgImage, const DImg& destImage,
                                    int progressBegin=0, int progressEnd=100);

    ~RedEyeCorrectionFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:RedEyeCorrectionFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("RedEye Correction Filter"));
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

private:

    void filterImage();
    void readParameters(const FilterAction&);

    void correctRedEye(uchar* data, int type, cv::Rect eyerect, cv::Rect imgRect);
    void QRectFtocvRect(const QList<QRect>& faces, std::vector<cv::Rect>& result);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // REDEYECORRECTIONFILTER_H
