/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-16-01
 * Description : white balance color correction.
 * 
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Guillaume Castagnino <casta at xwing dot info>
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
 
#ifndef WHITEBALANCE_H
#define WHITEBALANCE_H

// Digikam includes.

#include "digikam_export.h"

class QColor;

namespace Digikam
{

class WhiteBalancePriv;

class DIGIKAM_EXPORT WhiteBalance
{

public:
    
    WhiteBalance(bool sixteenBit);
    ~WhiteBalance();

    void whiteBalance(uchar *data, int width, int height, bool sixteenBit, 
                      double black=0.0, double exposition=0.0,
                      double temperature=6500.0, double green=1.0, double dark=0.5, 
                      double gamma=1.0, double saturation=1.0);

    static void autoExposureAdjustement(uchar* data, int width, int height, bool sb,
                                        double &black, double &expo); 

    static void autoWBAdjustementFromColor(const QColor &tc, double &temperature, double &green); 

private:

    void setRGBmult();
    static void setRGBmult(double &temperature, double &green, float &mr, float &mg, float &mb);
    void setLUTv();
    void adjustWhiteBalance(uchar *data, int width, int height, bool sixteenBit);
    inline unsigned short pixelColor(int colorMult, int index, int value);

private:

    WhiteBalancePriv* d;
};

}  // NameSpace Digikam

#endif /* WHITEBALANCE_H */
