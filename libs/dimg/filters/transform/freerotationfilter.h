/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-18
 * Description : Free rotation threaded image filter.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef FREE_ROTATION_FILTER_H
#define FREE_ROTATION_FILTER_H

// Qt includes

#include <QSize>
#include <QColor>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"

namespace Digikam
{

class DIGIKAM_EXPORT FreeRotationFilter : public DImgThreadedFilter
{

public:

    explicit FreeRotationFilter(DImg* orgImage, QObject* parent=0, double angle=0.0,
                                bool antialiasing=true, int autoCrop=NoAutoCrop, 
                                const QColor& backgroundColor=Qt::black,
                                int orgW=0, int orgH=0);

    ~FreeRotationFilter(){};

    QSize getNewSize(){ return m_newSize; };

    static double calculateAngle(int x1, int y1, int x2, int y2);
    static double calculateAngle(const QPoint& p1, const QPoint& p2);

public:

    enum AutoCropTypes
    {
        NoAutoCrop = 0,
        WidestArea,
        LargestArea
    };

private:

    void filterImage();

    inline int setPosition (int Width, int X, int Y)
    {
        return (Y *Width*4 + 4*X);
    };

    inline bool isInside (int Width, int Height, int X, int Y)
    {
        bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
        bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
        return (bIsWOk && bIsHOk);
    };

private:

    bool   m_antiAlias;

    int    m_autoCrop;
    int    m_orgW;
    int    m_orgH;

    double m_angle;

    QSize  m_newSize;

    QColor m_backgroundColor;
};

}  // namespace Digikam

#endif /* FREE_ROTATION_FILTER_H */
