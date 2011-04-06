/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-26
 * Description : texture pixmap methods
 *
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Adapted from fluxbox: Texture/TextureRender
 *
 * Texture.hh for Fluxbox Window Manager
 * Copyright (C) 2002-2003 Henrik Kinnunen <fluxbox@users.sourceforge.net>
 *
 * from Image.hh for Blackbox - an X11 Window manager
 * Copyright (C) 1997 - 2000 Brad Hughes (bhughes@tcac.net)
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

#ifndef TEXTURE_H
#define TEXTURE_H

// Qt includes

#include <QPixmap>

// Local includes

#include "digikam_export.h"
#include "theme.h"

namespace Digikam
{

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

    class TexturePriv;
    TexturePriv* const d;
};

}  // namespace Digikam

#endif /* TEXTURE_H */
