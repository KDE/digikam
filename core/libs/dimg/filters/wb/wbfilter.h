/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-16-01
 * Description : white balance color correction.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008      by Guillaume Castagnino <casta at xwing dot info>
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

#ifndef WBFILTER_H
#define WBFILTER_H

// Qt includes

#include <QColor>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"
#include "wbcontainer.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT WBFilter : public DImgThreadedFilter
{

public:

    explicit WBFilter(QObject* const parent = 0);
    explicit WBFilter(DImg* const orgImage, QObject* const parent=0, const WBContainer& settings=WBContainer());
    explicit WBFilter(const WBContainer& settings, DImgThreadedFilter* const master, const DImg& orgImage, const DImg& destImage,
                      int progressBegin=0, int progressEnd=100);
    virtual ~WBFilter();

    void                    readParameters(const FilterAction& action);

    static void             autoExposureAdjustement(const DImg* const img, double& black, double& expo);
    static void             findChanelsMax(const DImg* const img, int& maxr, int& maxg, int& maxb);
    static void             autoWBAdjustementFromColor(const QColor& tc, double& temperature, double& green);

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:WhiteBalanceFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("White Balance Tool"));
    }

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 2;
    }

    static int              CurrentVersion()
    {
        return 2;
    }

    virtual QString         filterIdentifier() const
    {
        return FilterIdentifier();
    }

    virtual FilterAction    filterAction();

protected:

    void filterImage();

protected:

    WBContainer m_settings;

private:

    void setRGBmult();
    void setLUTv();
    void adjustWhiteBalance(uchar* const data, int width, int height, bool sixteenBit);
    inline unsigned short pixelColor(int colorMult, int index, int value);

    static void setRGBmult(double& temperature, double& green, float& mr, float& mg, float& mb);

    /** This method have been introduced with bug #259223
     */
    void preventAutoExposure(int maxr, int maxg, int maxb);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // WBFILTER_H
