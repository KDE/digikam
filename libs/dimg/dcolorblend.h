/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-03-01
 * Description : DColor methods for blending
 *
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

// Inspired by DirectFB, src/gfx/generic/generic.c:

/*
   (c) Copyright 2000-2002  convergence integrated media GmbH <curanz@convergence.de>
   (c) Copyright 2002-2005  convergence GmbH.

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

*/

#ifndef DCOLORBLEND_H
#define DCOLORBLEND_H

namespace Digikam
{

inline void DColor::premultiply()
{
    if (sixteenBit())
        premultiply16(alpha());
    else
        premultiply8(alpha());
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

    setRed  ((Sa * (uint)Qt::red()) >> 16);
    setGreen((Sa * (uint)Qt::green()) >> 16);
    setBlue ((Sa * (uint)Qt::blue()) >> 16);
    setAlpha((Sa * (uint)alpha()) >> 16);
}

inline void DColor::blendAlpha8(int alphaValue)
{
    uint Sa = alphaValue + 1;

    setRed  ((Sa * Qt::red()) >> 8);
    setGreen((Sa * Qt::green()) >> 8);
    setBlue ((Sa * Qt::blue()) >> 8);
    setAlpha((Sa * alpha()) >> 8);
}

inline void DColor::blendInvAlpha16(int alphaValue)
{
    uint Sa = 65536 - alphaValue;

    setRed  ((Sa * (uint)Qt::red()) >> 16);
    setGreen((Sa * (uint)Qt::green()) >> 16);
    setBlue ((Sa * (uint)Qt::blue()) >> 16);
    setAlpha((Sa * (uint)alpha()) >> 16);
}

inline void DColor::blendInvAlpha8(int alphaValue)
{
    uint Sa = 256 - alphaValue;

    setRed  ((Sa * Qt::red()) >> 8);
    setGreen((Sa * Qt::green()) >> 8);
    setBlue ((Sa * Qt::blue()) >> 8);
    setAlpha((Sa * alpha()) >> 8);
}

inline void DColor::premultiply16(int alphaValue)
{
    uint Da = alphaValue + 1;

    setRed  ((Da * (uint)Qt::red()) >> 16);
    setGreen((Da * (uint)Qt::green()) >> 16);
    setBlue ((Da * (uint)Qt::blue()) >> 16);
}

inline void DColor::premultiply8(int alphaValue)
{
    uint Da = alphaValue + 1;

    setRed  ((Da * Qt::red()) >> 8);
    setGreen((Da * Qt::green()) >> 8);
    setBlue ((Da * Qt::blue()) >> 8);
}

inline void DColor::demultiply16(int alphaValue)
{
    uint Da = alphaValue + 1;

    setRed  (((uint)Qt::red()   << 16) / Da);
    setGreen(((uint)Qt::green() << 16) / Da);
    setBlue (((uint)Qt::blue()  << 16) / Da);
}

inline void DColor::demultiply8(int alphaValue)
{
    uint Da = alphaValue + 1;

    setRed  ((Qt::red()   << 8) / Da);
    setGreen((Qt::green() << 8) / Da);
    setBlue ((Qt::blue()  << 8) / Da);
}

inline void DColor::blendAdd(const DColor &src)
{
    setRed  (Qt::red()   + src.Qt::red());
    setGreen(Qt::green() + src.Qt::green());
    setBlue (Qt::blue()  + src.Qt::blue());
    setAlpha(alpha() + src.alpha());
}

inline void DColor::blendClamp16()
{
    if (0xFFFF0000 & Qt::red())   setRed(65535);
    if (0xFFFF0000 & Qt::green()) setGreen(65535);
    if (0xFFFF0000 & Qt::blue())  setBlue(65535);
    if (0xFFFF0000 & alpha()) setAlpha(65535);
}

inline void DColor::blendClamp8()
{
    if (0xFF00 & Qt::red())   setRed(255);
    if (0xFF00 & Qt::green()) setGreen(255);
    if (0xFF00 & Qt::blue())  setBlue(255);
    if (0xFF00 & alpha()) setAlpha(255);
}

} // namespace Digikam

#endif  // DCOLORBLEND_H

