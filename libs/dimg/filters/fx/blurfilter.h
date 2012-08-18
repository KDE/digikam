/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Gaussian Blur threaded image filter.
 *
 * Copyright (C) 2005-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef BLURFILTER_H
#define BLURFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"

namespace Digikam
{

class DIGIKAM_EXPORT BlurFilter : public DImgThreadedFilter
{

public:

    explicit BlurFilter(QObject* const parent = 0);
    explicit BlurFilter(DImg* const orgImage, QObject* const parent=0, int radius=3);

    // Constructor for slave mode: execute immediately in current thread with specified master filter
    explicit BlurFilter(DImgThreadedFilter* const parentFilter, const DImg& orgImage, const DImg& destImage,
                        int progressBegin=0, int progressEnd=100, int radius=3);

    ~BlurFilter();

    static QString          FilterIdentifier()
    {
        return "digikam:BlurFilter";
    }

    static QString          DisplayableName()
    {
        return I18N_NOOP("Blur Filter");
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
    void cimgBlurImage(uchar* data, int width, int height, bool sixteenBit, double radius);
    void gaussianBlurImage(uchar* data, int width, int height, bool sixteenBit, int radius);

    // function to allocate a 2d array
    int** Alloc2DArray(int Columns, int Rows)
    {
        // First, we declare our future 2d array to be returned
        int** lpcArray = 0L;

        // Now, we alloc the main pointer with Columns
        lpcArray = new int*[Columns];

        for (int i = 0; i < Columns; ++i)
        {
            lpcArray[i] = new int[Rows];
        }

        return (lpcArray);
    };

    // Function to deallocates the 2d array previously created
    void Free2DArray(int** lpcArray, int Columns)
    {
        // loop to deallocate the columns
        for (int i = 0; i < Columns; ++i)
        {
            delete [] lpcArray[i];
        }

        // now, we delete the main pointer
        delete [] lpcArray;
    };

    inline bool IsInside(int Width, int Height, int X, int Y)
    {
        bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
        bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
        return (bIsWOk && bIsHOk);
    };

private:

    int m_radius;
};

}  // namespace Digikam

#endif /* BLURFILTER_H */
