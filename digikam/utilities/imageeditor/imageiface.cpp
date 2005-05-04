/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-14
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <cstdio>

#include <qwidget.h>
#include <qsize.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>

#include <X11/Xlib.h>
#include <Imlib2.h>

#include "imlibinterface.h"
#include "imageiface.h"

namespace Digikam
{

class ImageIfacePriv
{
public:

    Display *display;
    Window   win;
    Visual  *vis;
    Colormap cm;
    int      depth;

    Imlib_Context context;
    Imlib_Image   image;

    int         constrainWidth;
    int         constrainHeight;
    int         previewWidth;
    int         previewHeight;
    int         originalWidth;
    int         originalHeight;

    QPixmap     qcheck;
    QPixmap     qpix;
    QBitmap     qmask;
    
    DATA32*     previewData;
};

ImageIface::ImageIface(int w, int h)
{
    d = new ImageIfacePriv;
    d->display = d->qcheck.x11Display();
    d->win     = d->qcheck.handle();

    d->vis   = DefaultVisual(d->display, DefaultScreen(d->display));
    d->depth = DefaultDepth(d->display, DefaultScreen(d->display));
    d->cm    = DefaultColormap(d->display, DefaultScreen(d->display));

    d->constrainWidth  = w;
    d->constrainHeight = h;

    d->image          = 0;
    d->originalWidth  = 0;
    d->originalHeight = 0;
    d->originalHeight = 0;
    d->previewWidth   = 0;
    d->previewHeight  = 0;
    d->previewData    = 0;
    
    d->context = imlib_context_new();
    imlib_context_push(d->context);

    imlib_context_set_display(d->display);
    imlib_context_set_visual(d->vis);
    imlib_context_set_colormap(d->cm);
    imlib_context_set_drawable(d->win);

    imlib_context_pop();

    d->originalWidth  = ImlibInterface::instance()->origWidth();
    d->originalHeight = ImlibInterface::instance()->origHeight();

    d->qpix.setMask(d->qmask);
    d->qcheck.resize(8, 8);

    QPainter p;
    p.begin(&d->qcheck);
    p.fillRect(0, 0, 4, 4, QColor(144,144,144));
    p.fillRect(4, 4, 4, 4, QColor(144,144,144));
    p.fillRect(0, 4, 4, 4, QColor(100,100,100));
    p.fillRect(4, 0, 4, 4, QColor(100,100,100));
    p.end();
}

ImageIface::~ImageIface()
{
    if (d->image) {
        imlib_context_push(d->context);
        imlib_context_set_image(d->image);
        imlib_free_image_and_decache();
        imlib_context_pop();
    }

    if (d->previewData) {
        delete [] d->previewData;
    }
    
    imlib_context_free(d->context);    

    delete d;
}

uint* ImageIface::getPreviewData()
{
    if (!d->previewData) {
    
        DATA32 *ptr   = ImlibInterface::instance()->getData();
        int      w    = ImlibInterface::instance()->origWidth();
        int      h    = ImlibInterface::instance()->origHeight();
        bool hasAlpha = ImlibInterface::instance()->hasAlpha();

        if (!ptr || !w || !h) {
            return 0;
        }

        DATA32 *origData = new DATA32[w*h];
        memcpy(origData, ptr, w*h*sizeof(DATA32));

        imlib_context_push(d->context);

        Imlib_Image im = imlib_create_image_using_copied_data(w, h, origData);
        delete [] origData;


        imlib_context_set_image(im);
        if (hasAlpha)
            imlib_image_set_has_alpha(1);
        else
            imlib_image_set_has_alpha(0);

        QSize sz(w, h);
        sz.scale(d->constrainWidth, d->constrainHeight, QSize::ScaleMin);

        d->image = imlib_create_cropped_scaled_image(0,0,w,h,
                                                     sz.width(),sz.height());

        imlib_context_set_image(im);
        imlib_free_image_and_decache();

        imlib_context_set_image(d->image);
        d->previewWidth  = imlib_image_get_width();
        d->previewHeight = imlib_image_get_height();

        if (hasAlpha)
            imlib_image_set_has_alpha(1);
        else
            imlib_image_set_has_alpha(0);

        ptr = imlib_image_get_data_for_reading_only();

        d->previewData = new DATA32[d->previewWidth*d->previewHeight];

        memcpy(d->previewData, ptr,
               d->previewWidth*d->previewHeight*sizeof(DATA32));

        Imlib_Color_Modifier mod = imlib_create_color_modifier();
        imlib_context_set_color_modifier(mod);

        imlib_context_pop();    

        d->qmask.resize(d->previewWidth, d->previewHeight);
        d->qpix.resize(d->previewWidth, d->previewHeight);
    }

    int size = d->previewWidth*d->previewHeight;
    
    DATA32* data = new DATA32[size];
    memcpy(data, d->previewData, size*sizeof(DATA32));
    
    return data;
}

uint* ImageIface::getOriginalData()
{
    DATA32 *ptr = ImlibInterface::instance()->getData();
    int      w  = ImlibInterface::instance()->origWidth();
    int      h  = ImlibInterface::instance()->origHeight();

    if (!ptr || !w || !h) {
        return 0;
    }
        
    DATA32 *origData = new DATA32[w*h];
    memcpy(origData, ptr, w*h*sizeof(DATA32));

    return origData;
}

uint* ImageIface::getSelectedData()
{
    return ImlibInterface::instance()->getSelectedData();    
}

void ImageIface::putPreviewData(uint* data)
{
    if (!data)
        return;
    
    imlib_context_push(d->context);

    imlib_context_set_image(d->image);
    uint* origData = imlib_image_get_data();
    int   w = imlib_image_get_width();
    int   h = imlib_image_get_height();

    memcpy(origData, data, w*h*sizeof(DATA32));
    
    imlib_image_put_back_data(origData);
    
    imlib_context_pop();    
}

void ImageIface::putOriginalData(const QString &caller, uint* data, int w, int h)
{
    if (!data)
        return;

    ImlibInterface::instance()->putData(caller, data, w, h);
}

void ImageIface::putSelectedData(uint* data)
{
    if (!data)
        return;

    ImlibInterface::instance()->putSelectedData(data);
}

int ImageIface::previewWidth()
{
    return d->previewWidth;    
}

int ImageIface::previewHeight()
{
    return d->previewHeight;    
}

int ImageIface::originalWidth()
{
    return ImlibInterface::instance()->origWidth();
}

int ImageIface::originalHeight()
{
    return ImlibInterface::instance()->origHeight();
}

int ImageIface::selectedWidth()
{
    int x, y, w, h;
    ImlibInterface::instance()->getSelectedArea(x,y,w,h);
    return w;
}

int ImageIface::selectedHeight()
{
    int x, y, w, h;
    ImlibInterface::instance()->getSelectedArea(x,y,w,h);
    return h;
}

int ImageIface::selectedXOrg()
{
    int x, y, w, h;
    ImlibInterface::instance()->getSelectedArea(x,y,w,h);
    return x;
}

int ImageIface::selectedYOrg()
{
    int x, y, w, h;
    ImlibInterface::instance()->getSelectedArea(x,y,w,h);
    return y;
}

void ImageIface::setPreviewBCG(double brightness, double contrast, double gamma)
{
    imlib_context_push(d->context);
    imlib_reset_color_modifier();
    
    imlib_modify_color_modifier_brightness(brightness);
    imlib_modify_color_modifier_contrast(contrast);
    imlib_modify_color_modifier_gamma(gamma);

    imlib_context_pop();
}

void ImageIface::setOriginalBCG(double brightness, double contrast, double gamma)
{
    ImlibInterface::instance()->setBCG(brightness, contrast, gamma);    
}

void ImageIface::paint(QPaintDevice* device, int x, int y, int w, int h)
{
    imlib_context_push(d->context);

    if (d->image) {

        imlib_context_set_image(d->image);
        
        if (imlib_image_has_alpha()) {
            QPainter p(&d->qpix);
            p.drawTiledPixmap(0,0,d->qpix.width(),d->qpix.height(),
                              d->qcheck);
            p.end();
        }

        imlib_context_set_drawable(d->qpix.handle());
        imlib_context_set_mask(d->qmask.handle());
        imlib_render_image_on_drawable_at_size(0,0,w,h);
    }

    imlib_context_pop();

    bitBlt(device, x, y, &d->qpix, 0, 0, -1, -1, Qt::CopyROP, false);
}

}
