/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Blur FX threaded image filter.
 *
 * Copyright 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
  
#ifndef BLURFX_H
#define BLURFX_H

// Digikam includes.

#include "dimgthreadedfilter.h"

namespace DigikamBlurFXImagesPlugin
{

class BlurFX : public Digikam::DImgThreadedFilter
{

public:

    BlurFX(Digikam::DImg *orgImage, QObject *parent=0, int blurFXType=ZoomBlur,
           int distance=100, int level=45);

    ~BlurFX(){};

public:

    enum BlurFXTypes 
    {
    ZoomBlur=0,
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

private:  // BlurFX filter data.

    int m_blurFXType;
    int m_distance;
    int m_level;

private:  // BlurFX filter methods.

    virtual void filterImage(void);

    // Backported from ImageProcessing version 1
    void softenerBlur(Digikam::DImg *orgImage, Digikam::DImg *destImage);
    void shakeBlur(Digikam::DImg *orgImage, Digikam::DImg *destImage, int Distance);
    void frostGlass(Digikam::DImg *orgImage, Digikam::DImg *destImage, int Frost);

    // Backported from ImageProcessing version 2
    void zoomBlur(Digikam::DImg *orgImage, Digikam::DImg *destImage,
                  int X, int Y, int Distance, QRect pArea=QRect());
    void radialBlur(Digikam::DImg *orgImage, Digikam::DImg *destImage,
                    int X, int Y, int Distance, QRect pArea=QRect());
    void focusBlur(Digikam::DImg *orgImage, Digikam::DImg *destImage,
                   int X, int Y, int BlurRadius, int BlendRadius,
                   bool bInversed=false, QRect pArea=QRect());
    void farBlur(Digikam::DImg *orgImage, Digikam::DImg *destImage, int Distance);
    void motionBlur(Digikam::DImg *orgImage, Digikam::DImg *destImage, int Distance, double Angle=0.0);
    void smartBlur(Digikam::DImg *orgImage, Digikam::DImg *destImage, int Radius, int Strenght);
    void mosaic(Digikam::DImg *orgImage, Digikam::DImg *destImage, int SizeW, int SizeH);

private:  // Internal filter methods.

    void MakeConvolution(Digikam::DImg *orgImage, Digikam::DImg *destImage, int Radius, int Kernel[]);

    Digikam::DColor RandomColor(uchar *Bits, int Width, int Height, bool sixteenBit, int bytesDepth,
                                int X, int Y, int Radius,
                                int alpha, uint *randomSeed, int range, uchar *IntensityCount,
                                uint *AverageColorR, uint *AverageColorG, uint *AverageColorB);

    // Return the limit defined the max and min values.
    inline int Lim_Max(int Now, int Up, int Max) 
    {
        --Max; 
        while (Now > Max - Up) --Up; 
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

        for (int i = 0; i < Columns; i++)
            lpcArray[i] = new int[Rows];

        return (lpcArray);
    }

    // Function to deallocates the 2d array previously created
    inline void Free2DArray (int** lpcArray, int Columns)
    {
       // loop to dealocate the columns
        for (int i = 0; i < Columns; i++)
            delete [] lpcArray[i];

       // now, we delete the main pointer
        delete [] lpcArray;
    }

    inline bool IsInside (int Width, int Height, int X, int Y)
    {
        bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
        bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
        return (bIsWOk && bIsHOk);
    };

    inline uchar LimitValues8(int ColorValue)
    {
        if (ColorValue > 255) ColorValue = 255;
        if (ColorValue < 0) ColorValue = 0;
        return ((uchar) ColorValue);
    };


    inline int LimitValues16(int ColorValue)
    {
        if (ColorValue > 65535) ColorValue = 65535;
        if (ColorValue < 0) ColorValue = 0;
        return ColorValue;
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
            if ((nG >= cG - Range) && (nG <= cG + Range))
                if ((nB >= cB - Range) && (nB <= cB + Range))
                    return (true);

        return (false);
    };

};

}  // NameSpace DigikamBlurFXImagesPlugin

#endif /* BLURFX_H */
