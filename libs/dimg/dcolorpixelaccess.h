/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-02
 * Description : methods to access on pixels color
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DCOLORPIXELACCESS_H
#define DCOLORPIXELACCESS_H

namespace Digikam
{

/** These methods are used in quite a few image effects,
 * typically in loops iterating the data.
 * Providing them as inline methods allows the compiler to optimize better.
 */

inline void DColor::setColor(uchar* const data, bool sixteenBit)
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
        unsigned short* data16 = reinterpret_cast<unsigned short*>(data);
        setBlue (data16[0]);
        setGreen(data16[1]);
        setRed  (data16[2]);
        setAlpha(data16[3]);
    }
}

inline void DColor::setPixel(uchar* const data) const
{
    if (sixteenBit())       // 16 bits image.
    {
        unsigned short* data16 = reinterpret_cast<unsigned short*>(data);
        data16[0]              = (unsigned short)blue();
        data16[1]              = (unsigned short)green();
        data16[2]              = (unsigned short)red();
        data16[3]              = (unsigned short)alpha();
    }
    else                    // 8 bits image.
    {
        data[0] = (uchar)blue();
        data[1] = (uchar)green();
        data[2] = (uchar)red();
        data[3] = (uchar)alpha();
    }
}

}  // namespace Digikam

#endif  // DCOLORPIXELACCESS_H
