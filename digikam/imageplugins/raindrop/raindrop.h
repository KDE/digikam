/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Raindrop threaded image filter.
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
  
#ifndef RAINDROP_H
#define RAINDROP_H

// Digikam includes.

#include "dimgthreadedfilter.h"

class QRect;

namespace DigikamRainDropImagesPlugin
{

class RainDrop : public Digikam::DImgThreadedFilter
{

public:
    
    RainDrop(Digikam::DImg *orgImage, QObject *parent=0, int drop=80, 
             int amount=150, int coeff=30, QRect *selection=0L);
    
    ~RainDrop(){};

private:

    virtual void filterImage(void);

    void rainDropsImage(Digikam::DImg *orgImage, Digikam::DImg *destImage, int MinDropSize, int MaxDropSize,
                        int Amount, int Coeff, bool bLimitRange, int progressMin, int progressMax);

    bool CreateRainDrop(uchar *pBits, int Width, int Height, bool sixteenBit, int bytesDepth,
                        uchar *pResBits, uchar* pStatusBits,
                        int X, int Y, int DropSize, double Coeff, bool bLimitRange);

    bool CanBeDropped(int Width, int Height, uchar *pStatusBits, int X, int Y, int DropSize, bool bLimitRange);

    bool SetDropStatusBits (int Width, int Height, uchar *pStatusBits, int X, int Y, int DropSize);

    // A color is represented in RGB value (e.g. 0xFFFFFF is white color). 
    // But R, G and B values has 256 values to be used so, this function analize 
    // the value and limits to this range.
    inline int LimitValues8(int ColorValue)
    {
        if (ColorValue > 255) ColorValue = 255;
        if (ColorValue < 0) ColorValue = 0;
        return ColorValue;
    };

    inline int LimitValues16(int ColorValue)
    {
        if (ColorValue > 65535) ColorValue = 65535;
        if (ColorValue < 0) ColorValue = 0;
        return ColorValue;
    };

    inline bool IsInside (int Width, int Height, int X, int Y)
    {
        bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
        bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
        return (bIsWOk && bIsHOk);
    };

    inline int Offset(int Width, int X, int Y, int bytesDepth)
    {
        return (Y * Width * bytesDepth + X * bytesDepth);
    };
            
private:  

    int m_drop;
    int m_amount;
    int m_coeff;
    
    int m_selectedX;
    int m_selectedY;
    int m_selectedW;
    int m_selectedH;
};

}  // NameSpace DigikamRainDropImagesPlugin

#endif /* RAINDROP_H */
