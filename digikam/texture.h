/* ============================================================
 * File  : texture.h
 * Date  : 2004-07-26
 * Description : 
 * 
 * Adapted from fluxbox: Texture/TextureRender
 *
 * Texture.hh for Fluxbox Window Manager
 * Copyright (c) 2002-2003 Henrik Kinnunen (fluxbox<at>users.sourceforge.net)
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

#include <qpixmap.h>
#include <qimage.h>
#include <qcolor.h>

class Texture
{
public:

    enum Bevel {
        FLAT =     0x00002,
        SUNKEN =   0x00004,
        RAISED =   0x00008
    };
    
    enum Textures {
        NONE =     0x00000,
        SOLID =    0x00010,
        GRADIENT = 0x00020
    };
    
    enum Gradients {
        HORIZONTAL =  0x00040,
        VERTICAL =    0x00080,
        DIAGONAL =    0x00100
    };

    enum {
        BEVEL1 =         0x04000,
        BEVEL2 =         0x08000
    };

    Texture(int w, int h, const QColor& from, const QColor& to);

    QPixmap renderPixmap();
    
private:

    void bevel();
    void vgradient();
    void buildImage();

    QImage        m_image;
    QColor        m_color0;
    QColor        m_color1;
    unsigned long m_type;
    int           m_width;
    int           m_height;

    unsigned char* m_red;
    unsigned char* m_green;
    unsigned char* m_blue;
};

#endif /* TEXTURE_H */
