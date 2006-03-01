/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-12-02
 * Description : 16 bits color management class
 *
 * Copyright 2005-2006 by Gilles Caulier
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

#ifndef DCOLOR_H
#define DCOLOR_H

// QT includes.

#include <qcolor.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DColorPriv;

class DIGIKAM_EXPORT DColor
{
public:

    /** Initialize with default value, fully transparent eight bit black */
    DColor()
        : m_red(0), m_green(0), m_blue(0), m_alpha(0), m_sixteenBit(false)
        {};

    /** Read value from data. Equivalent to setColor() */
    DColor(uchar *data, bool sixteenBit)
        { setColor(data, sixteenBit); }

    /** Initialize with given RGBA values */
    DColor(int red, int green, int blue, int alpha, bool sixteenBit)
        : m_red(red), m_green(green), m_blue(blue), m_alpha(alpha), m_sixteenBit(sixteenBit)
        {};

    /** Copy constructor */
    DColor(const DColor& color);

    /** Read values from QColor, convert to sixteenBit of sixteenBit is true */
    DColor(const QColor& color, bool sixteenBit=false);

    DColor& operator=(const DColor& color);
    ~DColor() {};

    /** Read color values as RGBA from the given memory location.
        If sixteenBit is false, 4 bytes are read.
        If sixteenBit is true, 8 bytes are read.
    */
    void DColor::setColor(uchar *data, bool sixteenBit = false);

    /** Write the values of this color to the given memory location.
        If sixteenBit is false, 4 bytes are written.
        If sixteenBit is true, 8 bytes are written.
    */
    void DColor::setPixel(uchar *data);

    int  red  () const { return m_red; }
    int  green() const { return m_green; }
    int  blue () const { return m_blue; }
    int  alpha() const { return m_alpha; }
    bool sixteenBit() const { return m_sixteenBit; }

    void setRed  (int red)   { m_red = red; }
    void setGreen(int green) { m_green = green; }
    void setBlue (int blue)  { m_blue = blue; }
    void setAlpha(int alpha) { m_alpha = alpha; }
    void setSixteenBit(bool sixteenBit) { m_sixteenBit = sixteenBit; }

    QColor getQColor() const;

    /** Convert the color values of this color to and from sixteen bit
        and set the sixteenBit value accordingly
    */
    void convertToSixteenBit();
    void convertToEightBit();

    /** Return the current RGB color values of this color
        in the HSL color space.
        Alpha is ignored for the conversion.
    */
    void getHSL(int* h, int* s, int* l) const;

    /** Set the RGB color values of this color
        to the given HSL values converted to RGB.
        Alpha is set to be fully opaque.
        sixteenBit determines both how the HSL values are interpreted
        and the sixteenBit value of this color after this operation.
    */
    void setRGB(int h, int s, int l, bool sixteenBit);

private:

    int  m_red;
    int  m_green;
    int  m_blue;
    int  m_alpha;

    bool m_sixteenBit;
};


// These methods are used in quite a few image effects,
// typically in loops iterating the data.
// Providing them as inline methods allows the compiler to optimize better.

inline void DColor::setColor(uchar *data, bool sixteenBit)
{
    m_sixteenBit = sixteenBit;

    if (!sixteenBit)          // 8 bits image
    {
        setBlue (data[0]);
        setGreen(data[1]);
        setRed  (data[2]);
        setAlpha(data[3]);
    }
    else                      // 16 bits image
    {
        unsigned short* data16 = (unsigned short*)data;
        setBlue (data16[0]);
        setGreen(data16[1]);
        setRed  (data16[2]);
        setAlpha(data16[3]);
    }
}

inline void DColor::setPixel(uchar *data)
{
    if (sixteenBit())       // 16 bits image.
    {
        unsigned short *data16 = (unsigned short *)data;
        data16[0] = (unsigned short)blue();
        data16[1] = (unsigned short)green();
        data16[2] = (unsigned short)red();
        data16[3] = (unsigned short)alpha();
    }
    else                    // 8 bits image.
    {
        data[0] = (uchar)blue();
        data[1] = (uchar)green();
        data[2] = (uchar)red();
        data[3] = (uchar)alpha();
    }
}

}  // NameSpace Digikam

#endif /* DCOLOR_H */
