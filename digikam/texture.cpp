/* ============================================================
 * File  : texture.cpp
 * Date  : 2004-07-26
 * Description : 
 * 
 * Adapted from fluxbox: Texture/TextureRender
 *
 * Texture.cc for Fluxbox Window Manager
 * Copyright (c) 2002-2003 Henrik Kinnunen (fluxbox<at>users.sourceforge.net)
 *
 * from Image.cc for Blackbox - an X11 Window manager
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
 * 
 * ============================================================ */

#include <cstring>
#include <cstdio>

#include "texture.h"

Texture::Texture(int w, int h, const QColor& from, const QColor& to)
    : m_width(w), m_height(h)
{
    m_type = RAISED | GRADIENT | DIAGONAL | BEVEL1;
    m_image = QImage(w, h, 32);

    m_color0 = from;
    m_color1 = to;

    m_red   = new unsigned char[w*h];
    m_green = new unsigned char[w*h];
    m_blue  = new unsigned char[w*h];
    
    vgradient();
    bevel();
    buildImage();
}

QPixmap Texture::renderPixmap()
{
    return QPixmap(m_image);
}

void Texture::bevel()
{
    unsigned char *pr = m_red, *pg = m_green, *pb = m_blue;

    register unsigned char r, g, b, rr ,gg ,bb;
    register unsigned int w = m_width, h = m_height - 1, wh = w * h;

    while (--w) {
        r = *pr;
        rr = r + (r >> 1);
        if (rr < r) rr = ~0;
        g = *pg;
        gg = g + (g >> 1);
        if (gg < g) gg = ~0;
        b = *pb;
        bb = b + (b >> 1);
        if (bb < b) bb = ~0;

        *pr = rr;
        *pg = gg;
        *pb = bb;

        r = *(pr + wh);
        rr = (r >> 2) + (r >> 1);
        if (rr > r) rr = 0;
        g = *(pg + wh);
        gg = (g >> 2) + (g >> 1);
        if (gg > g) gg = 0;
        b = *(pb + wh);
        bb = (b >> 2) + (b >> 1);
        if (bb > b) bb = 0;

        *((pr++) + wh) = rr;
        *((pg++) + wh) = gg;
        *((pb++) + wh) = bb;
    }

    r = *pr;
    rr = r + (r >> 1);
    if (rr < r) rr = ~0;
    g = *pg;
    gg = g + (g >> 1);
    if (gg < g) gg = ~0;
    b = *pb;
    bb = b + (b >> 1);
    if (bb < b) bb = ~0;

    *pr = rr;
    *pg = gg;
    *pb = bb;

    r = *(pr + wh);
    rr = (r >> 2) + (r >> 1);
    if (rr > r) rr = 0;
    g = *(pg + wh);
    gg = (g >> 2) + (g >> 1);
    if (gg > g) gg = 0;
    b = *(pb + wh);
    bb = (b >> 2) + (b >> 1);
    if (bb > b) bb = 0;

    *(pr + wh) = rr;
    *(pg + wh) = gg;
    *(pb + wh) = bb;

    pr = m_red   + m_width;
    pg = m_green + m_width;
    pb = m_blue  + m_width;

    while (--h) {
        r = *pr;
        rr = r + (r >> 1);
        if (rr < r) rr = ~0;
        g = *pg;
        gg = g + (g >> 1);
        if (gg < g) gg = ~0;
        b = *pb;
        bb = b + (b >> 1);
        if (bb < b) bb = ~0;

        *pr = rr;
        *pg = gg;
        *pb = bb;

        pr += m_width - 1;
        pg += m_width - 1;
        pb += m_width - 1;

        r = *pr;
        rr = (r >> 2) + (r >> 1);
        if (rr > r) rr = 0;
        g = *pg;
        gg = (g >> 2) + (g >> 1);
        if (gg > g) gg = 0;
        b = *pb;
        bb = (b >> 2) + (b >> 1);
        if (bb > b) bb = 0;

        *(pr++) = rr;
        *(pg++) = gg;
        *(pb++) = bb;
    }

    r = *pr;
    rr = r + (r >> 1);
    if (rr < r) rr = ~0;
    g = *pg;
    gg = g + (g >> 1);
    if (gg < g) gg = ~0;
    b = *pb;
    bb = b + (b >> 1);
    if (bb < b) bb = ~0;

    *pr = rr;
    *pg = gg;
    *pb = bb;

    pr += m_width - 1;
    pg += m_width - 1;
    pb += m_width - 1;

    r = *pr;
    rr = (r >> 2) + (r >> 1);
    if (rr > r) rr = 0;
    g = *pg;
    gg = (g >> 2) + (g >> 1);
    if (gg > g) gg = 0;
    b = *pb;
    bb = (b >> 2) + (b >> 1);
    if (bb > b) bb = 0;

    *pr = rr;
    *pg = gg;
    *pb = bb;
}

void Texture::vgradient()
{
    float dry, dgy, dby,
        yr = (float) m_color0.red(),
        yg = (float) m_color0.green(),
        yb = (float) m_color0.blue();

    dry = (float) (m_color1.red()   - m_color0.red());
    dgy = (float) (m_color1.green() - m_color0.green());
    dby = (float) (m_color1.blue()  - m_color0.blue());

    dry /= m_height;
    dgy /= m_height;
    dby /= m_height;

    unsigned char *pr = m_red, *pg = m_green, *pb = m_blue;
    register int y;
    
    for (y = 0; y < m_height; y++, pr += m_width, pg += m_width, pb += m_width) {
        memset(pr, (unsigned char) yr, m_width);
        memset(pg, (unsigned char) yg, m_width);
        memset(pb, (unsigned char) yb, m_width);

        yr += dry;
        yg += dgy;
        yb += dby;
    }
}

void Texture::buildImage()
{
    unsigned char *pr = m_red, *pg = m_green, *pb = m_blue;

    unsigned int* bits = (unsigned int*) m_image.bits();
    
    register int p;
    for (p =0; p < m_width*m_height; p++)
    {
        *bits = 0xff << 24 | *pr << 16 | *pg << 8 | *pb;
        bits++;
        pr++;
        pg++;
        pb++;
    }
}
