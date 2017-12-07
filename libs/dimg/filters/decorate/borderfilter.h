/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : border threaded image filter.
 *
 * Copyright 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef BORDERFILTER_H
#define BORDERFILTER_H

// Qt includes

#include <QColor>
#include <QImage>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DIGIKAM_EXPORT BorderContainer
{

public:

    enum BorderTypes
    {
        SolidBorder = 0,
        NiepceBorder,
        BeveledBorder,
        PineBorder,
        WoodBorder,
        PaperBorder,
        ParqueBorder,
        IceBorder,
        LeafBorder,
        MarbleBorder,
        RainBorder,
        CratersBorder,
        DriedBorder,
        PinkBorder,
        StoneBorder,
        ChalkBorder,
        GraniteBorder,
        RockBorder,
        WallBorder
    };

public:

    BorderContainer()
    {
        preserveAspectRatio   = true;
        orgWidth              = 0;
        orgHeight             = 0;
        borderType            = 0;
        borderPercent         = 0.1;
        borderWidth1          = 0;
        borderWidth2          = 0;
        borderWidth3          = 0;
        borderWidth4          = 0;
        solidColor            = QColor(0, 0, 0);
        niepceBorderColor     = QColor(255, 255, 255);
        niepceLineColor       = QColor(0, 0, 0);
        bevelUpperLeftColor   = QColor(192, 192, 192);
        bevelLowerRightColor  = QColor(128, 128, 128);
        decorativeFirstColor  = QColor(0, 0, 0);
        decorativeSecondColor = QColor(0, 0, 0);
    };

    ~BorderContainer() {};

public:

    bool    preserveAspectRatio;

    int     orgWidth;
    int     orgHeight;

    int     borderType;

    int     borderWidth1;
    int     borderWidth2;
    int     borderWidth3;
    int     borderWidth4;

    double  borderPercent;

    QString borderPath;

    QColor  solidColor;
    QColor  niepceBorderColor;
    QColor  niepceLineColor;
    QColor  bevelUpperLeftColor;
    QColor  bevelLowerRightColor;
    QColor  decorativeFirstColor;
    QColor  decorativeSecondColor;
};

class DIGIKAM_EXPORT BorderFilter : public DImgThreadedFilter
{

public:

    /** Constructor using settings to preserve aspect ratio of image.
     */
    explicit BorderFilter(QObject* parent = 0);
    explicit BorderFilter(DImg* orgImage, QObject* parent=0, const BorderContainer& settings = BorderContainer());
    virtual ~BorderFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:BorderFilter");
    }
    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Border Tool"));
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

    /** Methods to preserve aspect ratio of image. */
    void solid(DImg& src, DImg& dest, const DColor& fg, int borderWidth);
    void niepce(DImg& src, DImg& dest, const DColor& fg, int borderWidth,
                const DColor& bg, int lineWidth);
    void bevel(DImg& src, DImg& dest, const DColor& topColor,
               const DColor& btmColor, int borderWidth);
    void pattern(DImg& src, DImg& dest, int borderWidth, const DColor& firstColor,
                 const DColor& secondColor, int firstWidth, int secondWidth);

    /** Methods to not-preserve aspect ratio of image. */
    void solid2(DImg& src, DImg& dest, const DColor& fg, int borderWidth);
    void niepce2(DImg& src, DImg& dest, const DColor& fg, int borderWidth,
                 const DColor& bg, int lineWidth);
    void bevel2(DImg& src, DImg& dest, const DColor& topColor,
                const DColor& btmColor, int borderWidth);
    void pattern2(DImg& src, DImg& dest, int borderWidth, const DColor& firstColor,
                  const DColor& secondColor, int firstWidth, int secondWidth);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* BORDERFILTER_H */
