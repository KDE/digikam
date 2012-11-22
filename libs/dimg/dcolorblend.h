/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-03-01
 * Description : DColor methods for blending
 *
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

/*
   Inspired by DirectFB, src/gfx/generic/generic.c:

   (c) Copyright 2000-2002  convergence integrated media GmbH <curanz@convergence.de>
   (c) Copyright 2002-2005  convergence GmbH.

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjala <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.
*/

#ifndef DCOLORBLEND_H
#define DCOLORBLEND_H

namespace Digikam
{

inline void DColor::premultiply()
{
    if (sixteenBit())
    {
        premultiply16(alpha());
    }
    else
    {
        premultiply8(alpha());
    }
}

inline void DColor::demultiply()
{
    if (sixteenBit())
    {
        demultiply16(alpha());
        blendClamp16();
    }
    else
    {
        demultiply8(alpha());
        blendClamp8();
    }
}

inline void DColor::blendZero()
{
    setAlpha(0);
    setRed(0);
    setGreen(0);
    setBlue(0);
}

inline void DColor::blendAlpha16(int alphaValue)
{
    uint Sa = alphaValue + 1;

    setRed  ((Sa * (uint)red())   >> 16);
    setGreen((Sa * (uint)green()) >> 16);
    setBlue ((Sa * (uint)blue())  >> 16);
    setAlpha((Sa * (uint)alpha()) >> 16);
}

inline void DColor::blendAlpha8(int alphaValue)
{
    uint Sa = alphaValue + 1;

    setRed  ((Sa * red())   >> 8);
    setGreen((Sa * green()) >> 8);
    setBlue ((Sa * blue())  >> 8);
    setAlpha((Sa * alpha()) >> 8);
}

inline void DColor::blendInvAlpha16(int alphaValue)
{
    uint Sa = 65536 - alphaValue;

    setRed  ((Sa * (uint)red())   >> 16);
    setGreen((Sa * (uint)green()) >> 16);
    setBlue ((Sa * (uint)blue())  >> 16);
    setAlpha((Sa * (uint)alpha()) >> 16);
}

inline void DColor::blendInvAlpha8(int alphaValue)
{
    uint Sa = 256 - alphaValue;

    setRed  ((Sa * red())   >> 8);
    setGreen((Sa * green()) >> 8);
    setBlue ((Sa * blue())  >> 8);
    setAlpha((Sa * alpha()) >> 8);
}

inline void DColor::premultiply16(int alphaValue)
{
    uint Da = alphaValue + 1;

    setRed  ((Da * (uint)red())   >> 16);
    setGreen((Da * (uint)green()) >> 16);
    setBlue ((Da * (uint)blue())  >> 16);
}

inline void DColor::premultiply8(int alphaValue)
{
    uint Da = alphaValue + 1;

    setRed  ((Da * red())   >> 8);
    setGreen((Da * green()) >> 8);
    setBlue ((Da * blue())  >> 8);
}

inline void DColor::demultiply16(int alphaValue)
{
    uint Da = alphaValue + 1;

    setRed  (((uint)red()   << 16) / Da);
    setGreen(((uint)green() << 16) / Da);
    setBlue (((uint)blue()  << 16) / Da);
}

inline void DColor::demultiply8(int alphaValue)
{
    uint Da = alphaValue + 1;

    setRed  ((red()   << 8) / Da);
    setGreen((green() << 8) / Da);
    setBlue ((blue()  << 8) / Da);
}

inline void DColor::blendAdd(const DColor& src)
{
    setRed  (red()   + src.red());
    setGreen(green() + src.green());
    setBlue (blue()  + src.blue());
    setAlpha(alpha() + src.alpha());
}

inline void DColor::blendClamp16()
{
    if (0xFFFF0000 & red())
    {
        setRed(65535);
    }

    if (0xFFFF0000 & green())
    {
        setGreen(65535);
    }

    if (0xFFFF0000 & blue())
    {
        setBlue(65535);
    }

    if (0xFFFF0000 & alpha())
    {
        setAlpha(65535);
    }
}

inline void DColor::blendClamp8()
{
    if (0xFFFFFF00 & red())
    {
        setRed(255);
    }

    if (0xFFFFFF00 & green())
    {
        setGreen(255);
    }

    if (0xFFFFFF00 & blue())
    {
        setBlue(255);
    }

    if (0xFFFFFF00 & alpha())
    {
        setAlpha(255);
    }
}

inline void DColor::multiply(float factor)
{
    setRed(lround( red()     * factor ));
    setGreen(lround( green() * factor ));
    setBlue(lround( blue()   * factor ));
    setAlpha(lround( alpha() * factor ));
}

} // namespace Digikam

#endif  // DCOLORBLEND_H
