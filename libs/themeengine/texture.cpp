/* ============================================================
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

#include <qpainter.h>
#include <qimage.h>

#include <cstring>
#include <cstdio>

#include "texture.h"


Texture::Texture(int w, int h, const QColor& from, const QColor& to,
                 Theme::Bevel bevel, Theme::Gradient gradient,
                 bool border, const QColor& borderColor)
{
    m_bevel       = bevel;
    m_gradient    = gradient;
    m_red         = 0;
    m_green       = 0;
    m_blue        = 0;
    m_border      = border;
    m_borderColor = borderColor;

    if (!border)
    {
        m_width  = w;
        m_height = h;
    }
    else
    {
        m_width  = w-2;
        m_height = h-2;
    }

    if (m_width <= 0 || m_height <= 0)
        return;
    
    
    if (bevel & Theme::SUNKEN)
    {
        m_color0 = to;
        m_color1 = from;
    }
    else
    {
        m_color0 = from;
        m_color1 = to;
    }

    if (gradient == Theme::SOLID)
    {
        doSolid();
    }
    else
    {
        m_red   = new unsigned char[w*h];
        m_green = new unsigned char[w*h];
        m_blue  = new unsigned char[w*h];

        if (gradient == Theme::HORIZONTAL)
            doHgradient();
        else if (gradient == Theme::VERTICAL)
            doVgradient();
        else if (gradient == Theme::DIAGONAL)
            doDgradient();
        
        if (bevel & Theme::RAISED || bevel & Theme::SUNKEN)
            doBevel();
        
        buildImage();
    }
}

Texture::~Texture()
{
    if (m_red)
        delete [] m_red;
    if (m_green)
        delete [] m_green;
    if (m_blue)
        delete [] m_blue;
}

QPixmap Texture::renderPixmap() const
{
    if (m_width <= 0 || m_height <= 0)
        return QPixmap();

    if (!m_border)
        return m_pixmap;

    QPixmap pix(m_width+2, m_height+2);
    bitBlt(&pix, 1, 1, &m_pixmap, 0, 0);
    QPainter p(&pix);
    p.setPen(m_borderColor);
    p.drawRect(0, 0, m_width+2, m_height+2);
    p.end();

    return pix;
}

void Texture::doSolid()
{
    m_pixmap.resize(m_width, m_height);
    QPainter p(&m_pixmap);
    p.fillRect(0, 0, m_width, m_height, m_color0);
    if (m_bevel == Theme::RAISED)
    {
        p.setPen(m_color0.light(120));
        p.drawLine(0, 0, m_width-1, 0);  // top
        p.drawLine(0, 0, 0, m_height-1); // left
        p.setPen(m_color0.dark(120));
        p.drawLine(0, m_height-1, m_width-1, m_height-1); // bottom
        p.drawLine(m_width-1, 0, m_width-1, m_height-1);  // right
    }
    else if (m_bevel == Theme::SUNKEN)
    {
        p.setPen(m_color0.dark(120));
        p.drawLine(0, 0, m_width-1, 0);  // top
        p.drawLine(0, 0, 0, m_height-1); // left
        p.setPen(m_color0.light(120));
        p.drawLine(0, m_height-1, m_width-1, m_height-1); // bottom
        p.drawLine(m_width-1, 0, m_width-1, m_height-1);  // right
    }
    p.end();
}

void Texture::doHgradient()
{
    float drx, dgx, dbx,
        xr = (float) m_color0.red(),
        xg = (float) m_color0.green(),
        xb = (float) m_color0.blue();
    unsigned char *pr = m_red, *pg = m_green, *pb = m_blue;

    register int x, y;

    drx = (float) (m_color1.red()   - m_color0.red());
    dgx = (float) (m_color1.green() - m_color0.green());
    dbx = (float) (m_color1.blue()  - m_color0.blue());

    drx /= m_width;
    dgx /= m_width;
    dbx /= m_width;

    for (x = 0; x < m_width; x++) {
        *(pr++) = (unsigned char) (xr);
        *(pg++) = (unsigned char) (xg);
        *(pb++) = (unsigned char) (xb);

        xr += drx;
        xg += dgx;
        xb += dbx;
    }

    for (y = 1; y < m_height; y++, pr += m_width, pg += m_width, pb += m_width) {
        memcpy(pr, m_red, m_width);
        memcpy(pg, m_green, m_width);
        memcpy(pb, m_blue, m_width);
    }
}

void Texture::doVgradient()
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


void Texture::doDgradient()
{
    unsigned int* xtable = new unsigned int[m_width*3]; 
    unsigned int* ytable = new unsigned int[m_height*3];
 
    float drx, dgx, dbx, dry, dgy, dby, yr = 0.0, yg = 0.0, yb = 0.0,
                                        xr = (float) m_color0.red(),
                                        xg = (float) m_color0.green(),
                                        xb = (float) m_color0.blue();
    unsigned char *pr = m_red, *pg = m_green, *pb = m_blue;
    unsigned int w = m_width * 2, h = m_height * 2;
    unsigned int *xt = xtable; 
    unsigned int *yt = ytable; 


    register int x, y;

    dry = drx = (float) (m_color1.red()   - m_color0.red());
    dgy = dgx = (float) (m_color1.green() - m_color0.green());
    dby = dbx = (float) (m_color1.blue()  - m_color0.blue());

    // Create X table
    drx /= w;
    dgx /= w;
    dbx /= w;

    for (x = 0; x < m_width; x++) {
        *(xt++) = (unsigned char) (xr);
        *(xt++) = (unsigned char) (xg);
        *(xt++) = (unsigned char) (xb);

        xr += drx;
        xg += dgx;
        xb += dbx;
    }

    // Create Y table
    dry /= h;
    dgy /= h;
    dby /= h;

    for (y = 0; y < m_height; y++) {
        *(yt++) = ((unsigned char) yr);
        *(yt++) = ((unsigned char) yg);
        *(yt++) = ((unsigned char) yb);

        yr += dry;
        yg += dgy;
        yb += dby;
    }

    // Combine tables to create gradient

    for (yt = ytable, y = 0; y < m_height; y++, yt += 3)
    {
        for (xt = xtable, x = 0; x < m_width; x++)
        {
            *(pr++) = *(xt++) + *(yt);
            *(pg++) = *(xt++) + *(yt + 1);
            *(pb++) = *(xt++) + *(yt + 2);
        }
    }

    delete [] xtable;
    delete [] ytable;
}

void Texture::doBevel()
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

void Texture::buildImage()
{
    unsigned char *pr = m_red, *pg = m_green, *pb = m_blue;

    QImage image(m_width, m_height, 32);

    unsigned int* bits = (unsigned int*) image.bits();
    
    register int p;
    for (p =0; p < m_width*m_height; p++)
    {
        *bits = 0xff << 24 | *pr << 16 | *pg << 8 | *pb;
        bits++;
        pr++;
        pg++;
        pb++;
    }

    m_pixmap = QPixmap(image);
}
