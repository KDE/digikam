/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Blur FX threaded image filter.
 *
 * Copyright 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * Original Blur algorithms copyrighted 2004 by
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
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

#ifndef BLURFXFILTER_H
#define BLURFXFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class RandomNumberGenerator;

class DIGIKAM_EXPORT BlurFXFilter : public DImgThreadedFilter
{

public:

    explicit BlurFXFilter(QObject* const parent = 0);
    explicit BlurFXFilter(DImg* const orgImage, QObject* const parent=0, int blurFXType=ZoomBlur,
                          int distance=100, int level=45);
    ~BlurFXFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:BlurFXFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Blur FX Filter"));
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

public:

    enum BlurFXFilterTypes
    {
        ZoomBlur = 0,
        RadialBlur,
        FarBlur,
        MotionBlur,
        SoftenerBlur,
        ShakeBlur,
        FocusBlur,
        SmartBlur,
        FrostGlass,
        Mosaic
    };

private:

    struct Args
    {
        uint   start;
        uint   stop;
        uint   h;
        uint   w;
        DImg*  orgImage;
        DImg*  destImage;
        int    X;
        int    Y;
        int    Distance;

        int    nCount;
        int*   lpXArray;
        int*   lpYArray;

        int    BlendRadius;
        bool   bInversed;

        uchar* layer1;
        uchar* layer2;
        uchar* layer3;
        uchar* layer4;

        int    SizeW;
        int    SizeH;

        int    StrengthRange;
        int    Radius;
        int*   Kernel;
        int**  arrMult;
        uchar* pBlur;
    };

private:

    void filterImage();

    // Backported from ImageProcessing version 1
    void softenerBlur(DImg* const orgImage, DImg* const destImage);
    void softenerBlurMultithreaded(const Args& prm);

    void shakeBlur(DImg* const orgImage, DImg* const destImage, int Distance);
    void shakeBlurStage1Multithreaded(const Args& prm);
    void shakeBlurStage2Multithreaded(const Args& prm);

    void frostGlass(DImg* const orgImage, DImg* const destImage, int Frost);

    // Backported from ImageProcessing version 2
    void zoomBlur(DImg* const orgImage, DImg* const destImage, int X, int Y, int Distance, const QRect& pArea=QRect());
    void zoomBlurMultithreaded(const Args& prm);

    void radialBlur(DImg* const orgImage, DImg* const destImage, int X, int Y, int Distance, const QRect& pArea=QRect());
    void radialBlurMultithreaded(const Args& prm);

    void focusBlur(DImg* const orgImage, DImg* const destImage, int X, int Y, int BlurRadius, int BlendRadius,
                   bool bInversed=false, const QRect& pArea=QRect());
    void focusBlurMultithreaded(const Args& prm);

    void farBlur(DImg* const orgImage, DImg* const destImage, int Distance);

    void motionBlur(DImg* const orgImage, DImg* const destImage, int Distance, double Angle=0.0);
    void motionBlurMultithreaded(const Args& prm);

    void smartBlur(DImg* const orgImage, DImg* const destImage, int Radius, int Strength);
    void smartBlurStage1Multithreaded(const Args& prm);
    void smartBlurStage2Multithreaded(const Args& prm);

    void mosaic(DImg* const orgImage, DImg* const destImage, int SizeW, int SizeH);
    void mosaicMultithreaded(const Args& prm);

private:

    void MakeConvolution(DImg* const orgImage, DImg* const destImage, int Radius, int Kernel[]);
    void MakeConvolutionStage1Multithreaded(const Args& prm);
    void MakeConvolutionStage2Multithreaded(const Args& prm);

    DColor RandomColor(uchar* const Bits, int Width, int Height, bool sixteenBit, int bytesDepth,
                       int X, int Y, int Radius,
                       int alpha, RandomNumberGenerator& generator, int range, uchar* const IntensityCount,
                       uint* const AverageColorR, uint* const AverageColorG, uint* const AverageColorB);

    // Return the limit defined the max and min values.
    inline int Lim_Max(int Now, int Up, int Max)
    {
        --Max;

        while (Now > Max - Up)
        {
            --Up;
        }

        return (Up);
    };

    // Return the luminance (Y) component of YIQ color model.
    inline int GetIntensity (int R, int G, int B)
    {
        return (int)(R * 0.3 + G * 0.59 + B * 0.11);
    };

    // function to allocate a 2d array
    inline int** Alloc2DArray (int Columns, int Rows)
    {
        // First, we declare our future 2d array to be returned
        int** lpcArray = NULL;

        // Now, we alloc the main pointer with Columns
        lpcArray = new int*[Columns];

        for (int i = 0; i < Columns; ++i)
        {
            lpcArray[i] = new int[Rows];
        }

        return (lpcArray);
    }

    // Function to deallocates the 2d array previously created
    inline void Free2DArray (int** lpcArray, int Columns)
    {
        // loop to deallocate the columns
        for (int i = 0; i < Columns; ++i)
        {
            delete [] lpcArray[i];
        }

        // now, we delete the main pointer
        delete [] lpcArray;
    }

    inline bool IsInside (int Width, int Height, int X, int Y)
    {
        bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
        bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
        return (bIsWOk && bIsHOk);
    };

    inline int GetOffset(int Width, int X, int Y, int bytesDepth)
    {
        return (Y * Width * bytesDepth) + (X * bytesDepth);
    };

    inline int GetOffsetAdjusted(int Width, int Height, int X, int Y, int bytesDepth)
    {
        X = (X < 0) ?  0  :  ((X >= Width ) ? (Width  - 1) : X);
        Y = (Y < 0) ?  0  :  ((Y >= Height) ? (Height - 1) : Y);
        return GetOffset(Width, X, Y, bytesDepth);
    };

    inline bool IsColorInsideTheRange (int cR, int cG, int cB,
                                       int nR, int nG, int nB,
                                       int Range)
    {
        if ((nR >= cR - Range) && (nR <= cR + Range))
        {
            if ((nG >= cG - Range) && (nG <= cG + Range))
            {
                if ((nB >= cB - Range) && (nB <= cB + Range))
                {
                    return true;
                }
            }
        }

        return false;
    };

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* BLURFXFILTER_H */
