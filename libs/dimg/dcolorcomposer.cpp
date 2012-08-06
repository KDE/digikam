/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-03-02
 * Description : DColor methods for composing
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

// Integer arithmetic inspired by DirectFB,
// src/gfx/generic/generic.c and src/display/idirectfbsurface.c:

/*
   (c) Copyright 2000-2002  convergence integrated media GmbH <curanz@convergence.de>
   (c) Copyright 2002-2005  convergence GmbH.

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrj <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

*/

#include "dcolorcomposer.h"

namespace Digikam
{

class DColorComposerPorterDuffNone : public DColorComposer
{
public:
    virtual void compose(DColor& dest, DColor src);
    virtual void compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags);
};

class DColorComposerPorterDuffClear : public DColorComposer
{
public:
    virtual void compose(DColor& dest, DColor src);
    virtual void compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags);
};

class DColorComposerPorterDuffSrc : public DColorComposer
{
public:
    virtual void compose(DColor& dest, DColor src);
    virtual void compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags);
};

class DColorComposerPorterDuffSrcOver : public DColorComposer
{
public:
    virtual void compose(DColor& dest, DColor src);
    virtual void compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags);
};

class DColorComposerPorterDuffDstOver : public DColorComposer
{
public:
    virtual void compose(DColor& dest, DColor src);
    virtual void compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags);
};

class DColorComposerPorterDuffSrcIn : public DColorComposer
{
public:
    virtual void compose(DColor& dest, DColor src);
    virtual void compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags);
};

class DColorComposerPorterDuffDstIn : public DColorComposer
{
public:
    virtual void compose(DColor& dest, DColor src);
    virtual void compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags);
};

class DColorComposerPorterDuffSrcOut : public DColorComposer
{
public:
    virtual void compose(DColor& dest, DColor src);
    virtual void compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags);
};

class DColorComposerPorterDuffDstOut : public DColorComposer
{
public:
    virtual void compose(DColor& dest, DColor src);
    virtual void compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags);
};

class DColorComposerPorterDuffSrcAtop : public DColorComposer
{
public:
    virtual void compose(DColor& dest, DColor src);
    virtual void compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags);
};

class DColorComposerPorterDuffDstAtop : public DColorComposer
{
public:
    virtual void compose(DColor& dest, DColor src);
    virtual void compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags);
};

class DColorComposerPorterDuffXor : public DColorComposer
{
public:
    virtual void compose(DColor& dest, DColor src);
    virtual void compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags);
};

// Porter-Duff None
// component = (source * sa + destination * (1-sa))
// Src blending function Src Alpha
// Dst blending function Inv Src Alpha
void DColorComposerPorterDuffNone::compose(DColor& dest, DColor src)
{
    // preserve src alpha value for dest blending,
    // src.alpha() will be changed after blending src
    int sa = src.alpha();

    if (dest.sixteenBit())
    {
        src.blendAlpha16(sa);
        dest.blendInvAlpha16(sa);
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendAlpha8(sa);
        dest.blendInvAlpha8(sa);
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

void DColorComposerPorterDuffNone::compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1
    DColorComposer::compose(dest, src, multiplicationFlags);
}

// Porter-Duff Clear
// component = (source * 0 + destination * 0)
// Src blending function Zero
// Dst blending function Zero
void DColorComposerPorterDuffClear::compose(DColor& dest, DColor src)
{
    src.blendZero();
    dest.blendZero();
    dest.blendAdd(src);
}

void DColorComposerPorterDuffClear::compose(DColor& dest, DColor src, MultiplicationFlags)
{
    // skip pre- and demultiplication
    compose(dest, src);
}

// Porter-Duff Src
// Normal Painter's algorithm
// component = (source * 1 + destination * 0)
// Src blending function One
// Dst blending function Zero
void DColorComposerPorterDuffSrc::compose(DColor& dest, DColor src)
{
    // src: no-op
    dest.blendZero();
    dest.blendAdd(src);
}

void DColorComposerPorterDuffSrc::compose(DColor& dest, DColor src, MultiplicationFlags)
{
    // skip pre- and demultiplication
    compose(dest, src);
}

// Porter-Duff Src Over
// component = (source * 1 + destination * (1-sa))
// Src blending function One
// Dst blending function Inv Src Alpha
void DColorComposerPorterDuffSrcOver::compose(DColor& dest, DColor src)
{
    if (dest.sixteenBit())
    {
        // src: no-op
        dest.blendInvAlpha16(src.alpha());
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        // src: no-op
        dest.blendInvAlpha8(src.alpha());
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

void DColorComposerPorterDuffSrcOver::compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1
    DColorComposer::compose(dest, src, multiplicationFlags);
}

// Porter-Duff Dst over
// component = (source * (1.0-da) + destination * 1)
// Src blending function Inv Dst Alpha
// Dst blending function One
void DColorComposerPorterDuffDstOver::compose(DColor& dest, DColor src)
{
    if (dest.sixteenBit())
    {
        src.blendInvAlpha16(dest.alpha());
        // dest: no-op
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendInvAlpha8(dest.alpha());
        // dest: no-op
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

void DColorComposerPorterDuffDstOver::compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1
    DColorComposer::compose(dest, src, multiplicationFlags);
}

// Porter-Duff Src In
// component = (source * da + destination * 0)
// Src blending function Dst Alpha
// Dst blending function Zero
void DColorComposerPorterDuffSrcIn::compose(DColor& dest, DColor src)
{
    if (dest.sixteenBit())
    {
        src.blendAlpha16(dest.alpha());
        dest.blendZero();
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendAlpha8(dest.alpha());
        dest.blendZero();
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

void DColorComposerPorterDuffSrcIn::compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1
    DColorComposer::compose(dest, src, multiplicationFlags);
}

// Porter-Duff Dst In
// component = (source * 0 + destination * sa)
// Src blending function Zero
// Dst blending function Src Alpha
void DColorComposerPorterDuffDstIn::compose(DColor& dest, DColor src)
{
    int sa = src.alpha();

    if (dest.sixteenBit())
    {
        src.blendZero();
        dest.blendAlpha16(sa);
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendZero();
        dest.blendAlpha8(sa);
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

void DColorComposerPorterDuffDstIn::compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1
    DColorComposer::compose(dest, src, multiplicationFlags);
}

// Porter-Duff Src Out
// component = (source * (1-da) + destination * 0)
// Src blending function Inv Dst Alpha
// Dst blending function Zero
void DColorComposerPorterDuffSrcOut::compose(DColor& dest, DColor src)
{
    if (dest.sixteenBit())
    {
        src.blendInvAlpha16(dest.alpha());
        dest.blendZero();
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendInvAlpha8(dest.alpha());
        dest.blendZero();
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

void DColorComposerPorterDuffSrcOut::compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1
    DColorComposer::compose(dest, src, multiplicationFlags);
}

// Porter-Duff Dst Out
// component = (source * 0 + destination * (1-sa))
// Src blending function Zero
// Dst blending function Inv Src Alpha
void DColorComposerPorterDuffDstOut::compose(DColor& dest, DColor src)
{
    int sa = src.alpha();

    if (dest.sixteenBit())
    {
        src.blendZero();
        dest.blendInvAlpha16(sa);
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendZero();
        dest.blendInvAlpha8(sa);
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

void DColorComposerPorterDuffDstOut::compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1
    DColorComposer::compose(dest, src, multiplicationFlags);
}

// Porter-Duff Src Atop
// component = (source * da + destination * (1-sa))
// Src blending function Dst Alpha
// Dst blending function Inv Src Alpha
void DColorComposerPorterDuffSrcAtop::compose(DColor& dest, DColor src)
{
    int sa = src.alpha();

    if (dest.sixteenBit())
    {
        src.blendAlpha16(dest.alpha());
        dest.blendInvAlpha16(sa);
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendAlpha8(dest.alpha());
        dest.blendInvAlpha8(sa);
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

void DColorComposerPorterDuffSrcAtop::compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1
    DColorComposer::compose(dest, src, multiplicationFlags);
}

// Porter-Duff Dst Atop
// component = (source * (1-da) + destination * sa)
// Src blending function Inv Dest Alpha
// Dst blending function Src Alpha
void DColorComposerPorterDuffDstAtop::compose(DColor& dest, DColor src)
{
    int sa = src.alpha();

    if (dest.sixteenBit())
    {
        src.blendInvAlpha16(dest.alpha());
        dest.blendAlpha16(sa);
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendInvAlpha8(dest.alpha());
        dest.blendInvAlpha8(sa);
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

void DColorComposerPorterDuffDstAtop::compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1
    DColorComposer::compose(dest, src, multiplicationFlags);
}

// Porter-Duff Xor
// component = (source * (1-da) + destination * (1-sa))
// Src blending function Inv Dst Alpha
// Dst blending function Inv Src Alpha
void DColorComposerPorterDuffXor::compose(DColor& dest, DColor src)
{
    int sa = src.alpha();

    if (dest.sixteenBit())
    {
        src.blendInvAlpha16(dest.alpha());
        dest.blendInvAlpha16(sa);
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendInvAlpha8(dest.alpha());
        dest.blendInvAlpha8(sa);
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

void DColorComposerPorterDuffXor::compose(DColor& dest, DColor src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1
    DColorComposer::compose(dest, src, multiplicationFlags);
}

// ----------------------------------------------------------

void DColorComposer::compose(DColor& dest, DColor src, DColorComposer::MultiplicationFlags multiplicationFlags)
{
    if (multiplicationFlags & PremultiplySrc)
    {
        src.premultiply();
    }

    if (multiplicationFlags & PremultiplyDst)
    {
        dest.premultiply();
    }

    compose(dest, src);

    if (multiplicationFlags & DemultiplyDst)
    {
        dest.demultiply();
    }
}

DColorComposer* DColorComposer::getComposer(DColorComposer::CompositingOperation rule)
{
    switch (rule)
    {
        case PorterDuffNone:
            return new DColorComposerPorterDuffNone;
        case PorterDuffClear:
            return new DColorComposerPorterDuffClear;
        case PorterDuffSrc:
            return new DColorComposerPorterDuffSrc;
        case PorterDuffSrcOver:
            return new DColorComposerPorterDuffSrcOver;
        case PorterDuffDstOver:
            return new DColorComposerPorterDuffDstOver;
        case PorterDuffSrcIn:
            return new DColorComposerPorterDuffSrcIn;
        case PorterDuffDstIn:
            return new DColorComposerPorterDuffDstIn;
        case PorterDuffSrcOut:
            return new DColorComposerPorterDuffSrcOut;
        case PorterDuffDstOut:
            return new DColorComposerPorterDuffDstOut;
        case PorterDuffSrcAtop:
            return new DColorComposerPorterDuffDstOut;
        case PorterDuffDstAtop:
            return new DColorComposerPorterDuffDstOut;
        case PorterDuffXor:
            return new DColorComposerPorterDuffDstOut;
    }

    return 0;
}

}  // namespace Digikam
