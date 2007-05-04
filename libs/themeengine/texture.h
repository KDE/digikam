/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-26
 * Description : texture pixmap methods
 *
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * 
 * Adapted from fluxbox: Texture/TextureRender
 *
 * Texture.hh for Fluxbox Window Manager
 * Copyright (c) 2002-2003 Henrik Kinnunen <fluxbox@users.sourceforge.net>
 *
 * from Image.hh for Blackbox - an X11 Window manager
 * Copyright (c) 1997 - 2000 Brad Hughes (bhughes@tcac.net)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING                     
 * ============================================================ */

#ifndef TEXTURE_H
#define TEXTURE_H

// Qt includes.

#include <qcolor.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class TexturePriv;

class DIGIKAM_EXPORT Texture
{

public:

    Texture(int w, int h, const QColor& from, const QColor& to,
            Theme::Bevel bevel, Theme::Gradient gradient,
            bool border, const QColor& borderColor);
    ~Texture();

    QPixmap renderPixmap() const;
    
private:

    void doBevel();

    void doSolid();
    void doHgradient();
    void doVgradient();
    void doDgradient();

    void buildImage();

private:

    TexturePriv* d;
    
};

}  // NameSpace Digikam

#endif /* TEXTURE_H */
