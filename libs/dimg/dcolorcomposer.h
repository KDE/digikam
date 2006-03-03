/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date  : 2006-03-02
 * Description : DColor methods for composing
 *
 * Copyright 2006 by Marcel Wiesweg
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

#ifndef DCOLORCOMPOSER_H
#define DCOLORCOMPOSER_H

// Local includes

#include "dcolor.h"

namespace Digikam
{

class DColorComposer
{
public:
    /** The available rules to combine src and destination color.

        For the Porter-Duff rules, the formula is
            component = (source * fs + destination * fd)
        where
            fs, fd according to the following table with
            sa = source alpha,
            da = destination alpha:

            None          fs: sa      fd: 1.0-sa
            Clear         fs: 0.0     fd: 0.0
            Src           fs: 1.0     fd: 0.0
            Src Over      fs: 1.0     fd: 1.0-sa
            Dst Over      fs: 1.0-da  fd: 1.0
            Src In        fs: da      fd: 0.0
            Dst In        fs: 0.0     fd: sa
            Src Out       fs: 1.0-da  fd: 0.0
            Dst Out       fs: 0.0     fd: 1.0-sa

            Src Atop      fs: da      fd: 1.0-sa
            Dst Atop      fs: 1.0-da  fd: sa
            Xor           fs: 1.0-da  fd: 1.0-sa

            None is the default, classical blending mode.
            Src is the painter's algorithm.
            The documentation of the java.awt.AlphaComposite class
            provides a good introduction and documentation on Porter Duff.

        Premultiply alpha premultiplies the dest color
        with the alpha from src.
        If src is dest, premultiplies src.
     */

    enum CompositingOperation
    {
        PorterDuffNone,
        PorterDuffClear,
        PorterDuffSrc,
        PorterDuffSrcOver,
        PorterDuffDstOver,
        PorterDuffSrcIn,
        PorterDuffDstIn,
        PorterDuffSrcOut,
        PorterDuffDstOut,
        PorterDuffSrcAtop,
        PorterDuffDstAtop,
        PorterDuffXor,
        PremultiplyAlpha
    };

    /**
        Retrieve a DColorComposer object for one of the predefined rules.
        The object needs to be deleted by the caller.
    */
    static DColorComposer *getComposer(CompositingOperation rule);

    /**
        Carry out the actual composition process.
        Src and Dest are composed and the result is written to dest.
        If you just pass the object to a DImg method, you do not need to call this.
        Call this function if you want to compose two colors.
        Implement this function if you create a custom DColorComposer.

        The bit depth of source and destination color must be identical.
    */
    virtual void compose(DColor &dest, DColor src) = 0;

    /*
       Operations before and after compose that are not (directly) supported:
        - premultiplication of source and color values when they are not yet premultiplied
        - extra alpha value that is applied to source alpha
        - demultiplication of destination when destination is not premultiplied
    */
};

}

#endif
