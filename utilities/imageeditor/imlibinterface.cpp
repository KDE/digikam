/* ============================================================
 * File  : imlibinterface.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-15
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <qwidget.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>

#include <X11/Xlib.h>
#include <Imlib2.h>

#include <iostream>
#include "imlibinterface.h"

using namespace std;

namespace Digikam
{

#define MaxRGB 255L

class ImlibInterfacePrivate {

public:

    Display *display;
    Visual  *vis;
    GC       gc;
    Colormap cm;
    int      depth;

    bool valid;
    bool dirty;
    
    int  width;
    int  height;
    int  origWidth;
    int  origHeight;
    int  selX;
    int  selY;
    int  selW;
    int  selH;

    float gamma;
    float brightness;
    float contrast;

    QPixmap qpix;
    QBitmap qmask;
    QPixmap qcheck;
    
    Imlib_Context context;
    Imlib_Image   image;
    QString       filename;
};

ImlibInterface::ImlibInterface()
    : QObject()
{
    m_instance = this;
    
    d = new ImlibInterfacePrivate;

    d->display = QPaintDevice::x11AppDisplay();

    d->vis   = DefaultVisual(d->display, DefaultScreen(d->display));
    d->depth = DefaultDepth(d->display, DefaultScreen(d->display));
    d->cm    = DefaultColormap(d->display, DefaultScreen(d->display));

    d->context = imlib_context_new();
    imlib_context_push(d->context);
    
    // 60 MB of cache
    imlib_set_cache_size(60 * 1024 * 1024);

    // set the maximum number of colors to allocate for
    // 8bpp and less to 128 
    imlib_set_color_usage(128);
    // dither for depths < 24bpp 
    imlib_context_set_dither(1);
    // set the display , visual, colormap and drawable we are using 
    imlib_context_set_display(d->display);
    imlib_context_set_visual(d->vis);
    imlib_context_set_colormap(d->cm);
    
    Imlib_Color_Modifier mod = imlib_create_color_modifier();
    imlib_context_set_color_modifier(mod);

    imlib_context_pop();

    d->image  = 0;
    d->valid = false;
    d->dirty = true;
    d->width  = 0;
    d->height = 0;
    d->origWidth  = 0;
    d->origHeight = 0;
    d->selX = 0;
    d->selY = 0;
    d->selW = 0;
    d->selH = 0;

    d->qpix.setMask(d->qmask);

    d->qcheck.resize(16, 16);

    QPainter p;
    p.begin(&d->qcheck);
    p.fillRect(0, 0, 8, 8, QColor(144,144,144));
    p.fillRect(8, 8, 8, 8, QColor(144,144,144));
    p.fillRect(0, 8, 8, 8, QColor(100,100,100));
    p.fillRect(8, 0, 8, 8, QColor(100,100,100));
    p.end();
}

ImlibInterface::~ImlibInterface()
{
    delete d;

    m_instance = 0;
}

void ImlibInterface::load(const QString& filename)
{
    d->valid = false;

    d->filename = filename;
    
    imlib_context_push(d->context);
    if (d->image) {
        imlib_context_set_image(d->image);
        imlib_free_image();
        d->image = 0;
    }

    d->width  = 0;
    d->height = 0;
    d->origWidth  = 0;
    d->origHeight = 0;
    d->selX = 0;
    d->selY = 0;
    d->selW = 0;
    d->selH = 0;

    d->image = imlib_load_image(filename.latin1());

    if (d->image) {
        imlib_context_set_image(d->image);

        d->origWidth  = imlib_image_get_width();
        d->origHeight = imlib_image_get_height();
        d->valid  = true;
        d->dirty  = true;

        d->width  = d->origWidth;
        d->height = d->origHeight;

        d->gamma      = 1.0;
        d->contrast   = 1.0;
        d->brightness = 0.0;
    }
    imlib_context_pop();
}

int ImlibInterface::width()
{
    return d->width;    
}

int ImlibInterface::height()
{
    return d->height;
}

int ImlibInterface::origWidth()
{
    return d->origWidth;    
}

int ImlibInterface::origHeight()
{
    return d->origHeight;
}

void ImlibInterface::setSelectedArea(int x, int y, int w, int h)
{
    d->selX = x;
    d->selY = y;
    d->selW = w;
    d->selH = h;
}

void ImlibInterface::getSelectedArea(int& x, int& y, int& w, int& h)
{
    x = d->selX;
    y = d->selY;
    w = d->selW;
    h = d->selH;
}

bool ImlibInterface::hasAlpha()
{
    imlib_context_push(d->context);
    imlib_context_set_image(d->image);

    bool alpha = imlib_image_has_alpha();
    imlib_context_pop();

    return alpha;
}

void ImlibInterface::paint(QPaintDevice *w, int dx, int dy, int dw, int dh,
                           int sx, int sy)
{
    if (d->dirty)
        render();

    bitBlt(w, sx, sy, &d->qpix, dx, dy, dw, dh, Qt::CopyROP, false);
}

void ImlibInterface::render()
{
    d->qpix.resize(d->width, d->height);
    d->qmask.resize(d->width, d->height);

    imlib_context_push(d->context);
    imlib_context_set_image(d->image);

    if (imlib_image_has_alpha()) {
        QPainter p;
        p.begin(&d->qpix);
        p.drawTiledPixmap(0,0,d->width,d->height,d->qcheck);
        p.end();
    }    
        
    imlib_context_set_drawable(d->qpix.handle());
    imlib_context_set_mask(d->qmask.handle());
    imlib_render_image_on_drawable_at_size(0,0,d->width,d->height);
    imlib_context_pop();

    d->dirty = false;
}

void ImlibInterface::zoom(double val)
{
    d->width  = (int)(d->origWidth  * val);
    d->height = (int)(d->origHeight * val);
    d->dirty  = true;
}

void ImlibInterface::rotate90()
{
    imlib_context_push(d->context);
    imlib_image_orientate(1);
    d->dirty = true;

    d->origWidth = imlib_image_get_width();
    d->origHeight = imlib_image_get_height();
    imlib_context_pop();
}

void ImlibInterface::rotate180()
{
    imlib_context_push(d->context);
    imlib_image_orientate(2);
    d->dirty = true;

    d->origWidth = imlib_image_get_width();
    d->origHeight = imlib_image_get_height();
    imlib_context_pop();
}

void ImlibInterface::rotate270()
{
    imlib_context_push(d->context);
    imlib_image_orientate(3);
    d->dirty = true;

    d->origWidth = imlib_image_get_width();
    d->origHeight = imlib_image_get_height();
    imlib_context_pop();
}

void ImlibInterface::flipHoriz()
{
    imlib_context_push(d->context);
    imlib_image_flip_horizontal();
    d->dirty = true;

    d->origWidth = imlib_image_get_width();
    d->origHeight = imlib_image_get_height();
    imlib_context_pop();
    
}

void ImlibInterface::flipVert()
{
    imlib_context_push(d->context);
    imlib_image_flip_vertical();
    d->dirty = true;

    d->origWidth = imlib_image_get_width();
    d->origHeight = imlib_image_get_height();
    imlib_context_pop();
}

void ImlibInterface::crop(int x, int y, int w, int h)
{
    imlib_context_push(d->context);

    imlib_context_set_image(d->image);
    Imlib_Image im = imlib_create_cropped_image(x, y, w, h);
    imlib_free_image();
    d->image = im;
    imlib_context_set_image(d->image);
    
    d->origWidth = imlib_image_get_width();
    d->origHeight = imlib_image_get_height();
    imlib_context_pop();

    d->dirty = true;
}

void ImlibInterface::resize(int w, int h)
{
    imlib_context_push(d->context);

    imlib_context_set_image(d->image);
    Imlib_Image im =
        imlib_create_cropped_scaled_image(0, 0, d->origWidth, d->origHeight,
                                          w, h);
    imlib_free_image();
    d->image = im;
    imlib_context_set_image(d->image);
    
    d->origWidth = imlib_image_get_width();
    d->origHeight = imlib_image_get_height();
    imlib_context_pop();

    d->dirty = true;
}

void ImlibInterface::restore()
{
    load(d->filename);
}

void ImlibInterface::save(const QString&)
{
}

void ImlibInterface::saveAs(const QString&)
{
    
}

void ImlibInterface::changeGamma(double gamma)
{
    imlib_context_push(d->context);
    imlib_reset_color_modifier();
    
    d->gamma += gamma/10.0;

    imlib_modify_color_modifier_gamma(d->gamma);
    imlib_modify_color_modifier_brightness(d->brightness);
    imlib_modify_color_modifier_contrast(d->contrast);
    
    d->dirty = true;
    imlib_context_pop();
}

void ImlibInterface::changeBrightness(double brightness)
{
    imlib_context_push(d->context);
    imlib_reset_color_modifier();

    d->brightness += brightness/100.0;

    imlib_modify_color_modifier_gamma(d->gamma);
    imlib_modify_color_modifier_brightness(d->brightness);
    imlib_modify_color_modifier_contrast(d->contrast);

    d->dirty = true;

    imlib_context_pop();
}

void ImlibInterface::changeContrast(double contrast)
{
    imlib_context_push(d->context);
    imlib_reset_color_modifier();
    
    d->contrast += contrast/100.0;

    imlib_modify_color_modifier_gamma(d->gamma);
    imlib_modify_color_modifier_brightness(d->brightness);
    imlib_modify_color_modifier_contrast(d->contrast);

    d->dirty = true;

    imlib_context_pop();
}

void ImlibInterface::setBCG(double brightness, double contrast, double gamma)
{
    imlib_context_push(d->context);

    bool alpha = imlib_image_has_alpha();
    
    imlib_reset_color_modifier();
    
    imlib_modify_color_modifier_brightness(brightness);
    imlib_modify_color_modifier_contrast(contrast);
    imlib_modify_color_modifier_gamma(gamma);
    imlib_apply_color_modifier();

    imlib_reset_color_modifier();

    // restore image's alpha setting.
    // seems applying brightness/contrast/alpha will cause alpha lookup table to
    // change which causes applying the color modifier to make an image have an
    // alpha channel even thought it didn't have one earlier.
    imlib_image_set_has_alpha(alpha ? 1:0);

    imlib_context_pop();

    d->dirty = true;
    emit signalRequestUpdate();
}

uint* ImlibInterface::getData()
{
    if (d->image) {
        imlib_context_push(d->context);
        imlib_context_set_image(d->image);
        DATA32 *ptr = imlib_image_get_data_for_reading_only();
        imlib_context_pop();
        return ptr;
    }
    else
        return 0;
}

void ImlibInterface::putData(uint* data)
{
    imlib_context_push(d->context);
    
    DATA32* ptr = imlib_image_get_data();

    memcpy(ptr, data, d->origWidth*d->origHeight*sizeof(DATA32));
    
    imlib_image_put_back_data(ptr);

    imlib_context_pop();

    d->dirty = true;

    emit signalRequestUpdate();
}

uint* ImlibInterface::getSelectedData()
{
    if (!d->selW || !d->selH)
        return 0;
    
    if (d->image) {
        imlib_context_push(d->context);
        imlib_context_set_image(d->image);

        DATA32 *ptr = imlib_image_get_data_for_reading_only();
        DATA32 *pptr;
        
        DATA32 *data = new DATA32[d->selW*d->selH];
        DATA32 *dptr  = data;

        for (int j = d->selY; j < (d->selY + d->selH); j++) {
            pptr  = &ptr[j*d->origWidth] + d->selX;
            for (int i = 0; i < d->selW; i++) {
                *(dptr++) = *(pptr++);
            }
        }

        imlib_context_pop();

        return data;
    }
    else
        return 0;
}

void ImlibInterface::putSelectedData(uint* data)
{
    if (!data || !d->image)
        return;
    
    imlib_context_push(d->context);
    imlib_context_set_image(d->image);

    DATA32 *ptr = imlib_image_get_data();
    DATA32 *pptr;
        
    DATA32 *dptr  = data;
        
    for (int j = d->selY; j < (d->selY + d->selH); j++) {
        pptr  = &ptr[j*d->origWidth] + d->selX;
        for (int i = 0; i < d->selW; i++) {
            *(pptr++) = *(dptr++);
        }
    }

    imlib_context_pop();

    d->dirty = true;

    emit signalRequestUpdate();
}

ImlibInterface* ImlibInterface::instance()
{
    if (!m_instance) {
        new ImlibInterface();
    }
    
    return m_instance;    
}

ImlibInterface* ImlibInterface::m_instance = 0;

}


#include "imlibinterface.moc"
