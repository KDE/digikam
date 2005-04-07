/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2003-01-15
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju, Gilles Caulier
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

#define PI 3.14159265
 
// Lib Tiff includes.

extern "C" 
{
#include <tiffio.h>
}
 
// C++ includes.

#include <cmath>
#include <cstdio>
#include <cstdlib>

// Qt includes.

#include <qstring.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qapplication.h>
#include <qfile.h>
#include <qfileinfo.h>

// KDE includes.

#include <kdebug.h>
#include <libkexif/kexifdata.h>

// Imlib2 includes.

#include <X11/Xlib.h>
#include <Imlib2.h>

// Local includes.

#include "undomanager.h"
#include "undoaction.h"
#include "imlibinterface.h"

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
    
    int  width;
    int  height;
    int  origWidth;
    int  origHeight;
    int  selX;
    int  selY;
    int  selW;
    int  selH;
    double zoom;

    float gamma;
    float brightness;
    float contrast;

    bool  exifOrient;
    
    Imlib_Context        context;
    Imlib_Image          image;
    Imlib_Color_Modifier cmod;
    Imlib_Load_Error     errorRet;
    QString              filename;

    UndoManager*         undoMan;
};

ImlibInterface::ImlibInterface()
              : QObject()
{
    m_instance = this;
    
    d = new ImlibInterfacePrivate;

    d->undoMan = new UndoManager(this);
    
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
    
    d->cmod = imlib_create_color_modifier();
    imlib_context_set_color_modifier(d->cmod);

    imlib_context_pop();

    d->image  = 0;
    d->valid = false;
    d->width  = 0;
    d->height = 0;
    d->origWidth  = 0;
    d->origHeight = 0;
    d->selX = 0;
    d->selY = 0;
    d->selW = 0;
    d->selH = 0;
    d->zoom = 1.0;
    d->exifOrient = false;
        
    m_rotatedOrFlipped = false;
}

ImlibInterface::~ImlibInterface()
{
    imlib_context_free(d->context);
    delete d->undoMan;
    delete d;

    m_instance = 0;
}

bool ImlibInterface::load(const QString& filename)
{
    bool valRet;
    d->valid = false;

    d->filename = filename;
    
    imlib_context_push(d->context);
    if (d->image) {
        imlib_context_set_image(d->image);
        imlib_free_image();
        d->image = 0;
    }

    d->width      = 0;
    d->height     = 0;
    d->origWidth  = 0;
    d->origHeight = 0;
    d->selX       = 0;
    d->selY       = 0;
    d->selW       = 0;
    d->selH       = 0;
    d->gamma      = 1.0;
    d->contrast   = 1.0;
    d->brightness = 0.0;

    imlib_context_set_color_modifier(d->cmod);
    imlib_reset_color_modifier();

    d->undoMan->clear();
    
    Imlib_Load_Error errorReturn;
    d->image = imlib_load_image_with_error_return(QFile::encodeName(filename),
                                                  &errorReturn);

    // try to load with kde imageio
    if (!d->image)
    {
        QImage qtimage(filename);
        if (!qtimage.isNull())
        {
            if (!qtimage.depth() == 32)
                qtimage.convertDepth(32);

            d->image = imlib_create_image(qtimage.width(), qtimage.height());
            imlib_context_set_image(d->image);
            DATA32* data = imlib_image_get_data_for_reading_only();
            memcpy(data, qtimage.bits(), qtimage.numBytes());
            kdDebug() << "Loaded image with kde imageio resources: "
                      << filename
                      << ", width="  << qtimage.width()
                      << ", height=" << qtimage.height() << endl;
        }
    }

    if (d->image) {
        imlib_context_set_image(d->image);
        imlib_image_set_changes_on_disk();

        d->origWidth  = imlib_image_get_width();
        d->origHeight = imlib_image_get_height();
        d->valid  = true;

        d->width  = d->origWidth;
        d->height = d->origHeight;

        valRet = true;
    }
    else {
        kdWarning() << k_funcinfo << "Failed to load image: error number: "
                    << errorReturn << endl;
        valRet = false;
    }
        
    imlib_context_pop();

    if (d->exifOrient)
    {
        exifRotate(filename);
    }
    
    return (valRet);
}


bool ImlibInterface::exifRotated()
{
    return m_rotatedOrFlipped;
}


void ImlibInterface::exifRotate(const QString& filename)
{
    // Rotate image based on EXIF rotate tag
    KExifData exifData;

    if(!exifData.readFromFile(filename))
        return;

    KExifData::ImageOrientation orientation = exifData.getImageOrientation();

    imlib_context_push(d->context);
    imlib_context_set_image(d->image);

    if(orientation != KExifData::NORMAL) {

        switch (orientation) {
            case KExifData::NORMAL:
            case KExifData::UNSPECIFIED:
                break;

            case KExifData::HFLIP:
                imlib_image_flip_horizontal();
                break;

            case KExifData::ROT_180:
                rotate180();
                break;

            case KExifData::VFLIP:
                imlib_image_flip_vertical();
                break;

            case KExifData::ROT_90_HFLIP:
                rotate90();
                imlib_image_flip_horizontal();
                break;

            case KExifData::ROT_90:
                rotate90();
                break;

            case KExifData::ROT_90_VFLIP:
                rotate90();
                imlib_image_flip_vertical();
                break;

            case KExifData::ROT_270:
                rotate270();
                break;
        }

        m_rotatedOrFlipped = true;
    }

    imlib_context_pop();
}


void ImlibInterface::preload(const QString& filename)
{
    imlib_context_push(d->context);
    Imlib_Image im = imlib_load_image(QFile::encodeName(filename));
    if (im)
    {
        imlib_context_set_image(im);
        imlib_free_image();
    }
    imlib_context_pop();
}

void ImlibInterface::setExifOrient(bool exifOrient)
{
    d->exifOrient = exifOrient;    
}

void ImlibInterface::undo()
{
    if (!d->undoMan->anyMoreUndo())
    {
        emit signalModified(false, d->undoMan->anyMoreRedo());
        return;
    }

    d->undoMan->undo();
    emit signalModified(d->undoMan->anyMoreUndo(), true);
}

void ImlibInterface::redo()
{
    if (!d->undoMan->anyMoreRedo())
    {
        emit signalModified(d->undoMan->anyMoreUndo(), false);
        return;
    }

    d->undoMan->redo();
    emit signalModified(true, d->undoMan->anyMoreRedo());
}

void ImlibInterface::restore()
{
    d->undoMan->clear();
    
    load(d->filename);
    emit signalModified(false, false);
}

bool ImlibInterface::save(const QString& file, int JPEGcompression, 
                          int PNGcompression, bool TIFFcompression)
{
    imlib_context_push(d->context);
    imlib_context_set_image(d->image);

    imlib_context_set_color_modifier(d->cmod);
    imlib_reset_color_modifier();
    
    imlib_modify_color_modifier_brightness(d->brightness);
    imlib_modify_color_modifier_contrast(d->contrast);
    imlib_modify_color_modifier_gamma(d->gamma);
    imlib_apply_color_modifier();
    
    QString currentMimeType(imlib_image_format());

    bool result = saveAction(file, JPEGcompression, PNGcompression, 
                             TIFFcompression, currentMimeType);

    imlib_context_pop();

    if (result)
    {
        d->undoMan->clear();
        emit signalModified(false, false);
    }
    
    return result;
}

bool ImlibInterface::saveAs(const QString& file, int JPEGcompression, 
                            int PNGcompression, bool TIFFcompression, 
                            const QString& mimeType)
{
    bool result;
    imlib_context_push(d->context);
    imlib_context_set_image(d->image);

    imlib_context_set_color_modifier(d->cmod);
    imlib_reset_color_modifier();

    imlib_modify_color_modifier_brightness(d->brightness);
    imlib_modify_color_modifier_contrast(d->contrast);
    imlib_modify_color_modifier_gamma(d->gamma);
    imlib_apply_color_modifier();

    if (mimeType.isEmpty())
        result = saveAction(file, JPEGcompression, PNGcompression, 
                            TIFFcompression, imlib_image_format());
    else
        result = saveAction(file, JPEGcompression, PNGcompression, 
                           TIFFcompression, mimeType);

    imlib_context_pop();

    return result;
}

void ImlibInterface::setModified(bool val)
{
    if (val)
    {
        emit signalModified(true, true);
    }
    else 
    {
        d->undoMan->clear();
        emit signalModified(false, false);
    }
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

void ImlibInterface::paintOnDevice(QPaintDevice* p,
                                   int sx, int sy, int sw, int sh,
                                   int dx, int dy, int dw, int dh,
                                   int antialias)
{
    if (!d->image)
        return;

    imlib_context_push(d->context);
    imlib_context_set_image(d->image);
    imlib_context_set_drawable(p->handle());
    imlib_context_set_anti_alias(antialias);

    imlib_context_set_color_modifier(d->cmod);

    if (d->zoom == 1.0)
    {
        // this hack is needed because imlib2 renders image incorrectly
        // if zoom == 1 and an unmodified colot modifier is applied
        Imlib_Image tmp =
            imlib_create_cropped_scaled_image(sx, sy, sw, sh, dw, dh);
        if (tmp)
        {        
            imlib_context_set_image(tmp);
            imlib_render_image_on_drawable(dx, dy);
            imlib_free_image();
        }
    }
    else
    {
        imlib_render_image_part_on_drawable_at_size(sx, sy, sw, sh,
                                                    dx, dy, dw, dh);
    }

    imlib_context_pop();
}

void ImlibInterface::paintOnDevice(QPaintDevice* p,
                                   int sx, int sy, int sw, int sh,
                                   int dx, int dy, int dw, int dh,
                                   int mx, int my, int mw, int mh,
                                   int antialias)
{
    if (!d->image)
        return;

    imlib_context_push(d->context);
    imlib_context_set_image(d->image);
    imlib_context_set_drawable(p->handle());
    imlib_context_set_anti_alias(antialias);

    imlib_context_set_color_modifier(0);

    Imlib_Image bot = imlib_create_cropped_scaled_image(sx, sy, sw, sh,
                                                        dw, dh);

    // create the mask -----------------------------------

    Imlib_Image top = imlib_create_image(dw, dh);
    imlib_context_set_image(top);
    DATA32* data = imlib_image_get_data();
    DATA32* ptr = data;

    for (int j=0; j<dh; j++) {
        for (int i=0; i<dw; i++) {
            if (i >= (mx-dx) && i <= (mx-dx+mw-1) &&
                j >= (my-dy) && j <= (my-dy+mh-1))
                *(ptr++) = 0x00000000;
            else
                *(ptr++) = 0xBBAAAAAA;
        }
    }
    imlib_image_put_back_data(data);
    imlib_image_set_has_alpha(1);

    // blend the mask -----------------------------------
    imlib_context_set_image(bot);
    imlib_context_set_blend(1);
    imlib_blend_image_onto_image(top, 0,
                                 0, 0, dw, dh,
                                 0, 0, dw, dh);

    imlib_context_set_color_modifier(d->cmod);
    imlib_render_image_on_drawable(dx, dy);

    imlib_context_set_image(bot);
    imlib_free_image();
    imlib_context_set_image(top);
    imlib_free_image();

    imlib_context_pop();
}

void ImlibInterface::zoom(double val)
{
    d->zoom   = val;
    d->width  = (int)(d->origWidth  * val);
    d->height = (int)(d->origHeight * val);
}

void ImlibInterface::rotate90(bool saveUndo)
{
    if (saveUndo)
    {
        d->undoMan->addAction(new UndoActionRotate(this, UndoActionRotate::R90));    
    }
    
    imlib_context_push(d->context);
    imlib_context_set_image(d->image);

    imlib_image_orientate(1);

    d->origWidth = imlib_image_get_width();
    d->origHeight = imlib_image_get_height();    
    imlib_context_pop();

    emit signalModified(true, d->undoMan->anyMoreRedo());
}

void ImlibInterface::rotate180(bool saveUndo)
{
    if (saveUndo)
    {
        d->undoMan->addAction(new UndoActionRotate(this, UndoActionRotate::R180));    
    }

    imlib_context_push(d->context);
    imlib_context_set_image(d->image);

    imlib_image_orientate(2);

    d->origWidth = imlib_image_get_width();
    d->origHeight = imlib_image_get_height();
    imlib_context_pop();

    emit signalModified(true, d->undoMan->anyMoreRedo());
}

void ImlibInterface::rotate270(bool saveUndo)
{
    if (saveUndo)
    {
        d->undoMan->addAction(new UndoActionRotate(this, UndoActionRotate::R270));    
    }

    imlib_context_push(d->context);
    imlib_context_set_image(d->image);
    
    imlib_image_orientate(3);

    d->origWidth = imlib_image_get_width();
    d->origHeight = imlib_image_get_height();
    imlib_context_pop();

    emit signalModified(true, d->undoMan->anyMoreRedo());
}

void ImlibInterface::flipHoriz(bool saveUndo)
{
    if (saveUndo)
    {
        d->undoMan->addAction(new UndoActionFlip(this, UndoActionFlip::Horizontal));    
    }

    imlib_context_push(d->context);
    imlib_context_set_image(d->image);

    imlib_image_flip_horizontal();

    d->origWidth = imlib_image_get_width();
    d->origHeight = imlib_image_get_height();
    imlib_context_pop();
    
    emit signalModified(true, d->undoMan->anyMoreRedo());
}

void ImlibInterface::flipVert(bool saveUndo)
{
    if (saveUndo)
    {
        d->undoMan->addAction(new UndoActionFlip(this, UndoActionFlip::Vertical));
    }

    imlib_context_push(d->context);
    imlib_context_set_image(d->image);
    
    imlib_image_flip_vertical();

    d->origWidth = imlib_image_get_width();
    d->origHeight = imlib_image_get_height();
    imlib_context_pop();

    emit signalModified(true, d->undoMan->anyMoreRedo());
}

void ImlibInterface::crop(int x, int y, int w, int h)
{
    d->undoMan->addAction(new UndoActionIrreversible(this, "Crop"));

    imlib_context_push(d->context);
    imlib_context_set_image(d->image);
    
    QString format(imlib_image_format());
    Imlib_Image im = imlib_create_cropped_image(x, y, w, h);
    imlib_free_image();
    d->image = im;
    imlib_context_set_image(d->image);
    imlib_image_set_format(format.ascii());
    
    d->origWidth = imlib_image_get_width();
    d->origHeight = imlib_image_get_height();
    imlib_context_pop();

    emit signalModified(true, d->undoMan->anyMoreRedo());
}

void ImlibInterface::resize(int w, int h)
{
    d->undoMan->addAction(new UndoActionIrreversible(this, "Resize"));

    imlib_context_push(d->context);
    imlib_context_set_image(d->image);

    QString format(imlib_image_format());
    Imlib_Image im =
        imlib_create_cropped_scaled_image(0, 0, d->origWidth, d->origHeight,
                                          w, h);
    imlib_free_image();
    d->image = im;

    imlib_context_set_image(d->image);
    imlib_image_set_format(format.ascii());
    
    d->origWidth = imlib_image_get_width();
    d->origHeight = imlib_image_get_height();
    imlib_context_pop();

    emit signalModified(true, d->undoMan->anyMoreRedo());
}

void ImlibInterface::changeGamma(double gamma)
{
    d->undoMan->addAction(new UndoActionBCG(this, d->gamma, d->brightness,
                                            d->contrast, gamma, d->brightness,
                                            d->contrast));

    imlib_context_push(d->context);
    imlib_context_set_color_modifier(d->cmod);
    imlib_reset_color_modifier();
    
    d->gamma += gamma/10.0;

    imlib_modify_color_modifier_gamma(d->gamma);
    imlib_modify_color_modifier_brightness(d->brightness);
    imlib_modify_color_modifier_contrast(d->contrast);

    imlib_context_set_color_modifier(0);
    
    imlib_context_pop();

    emit signalModified(true, d->undoMan->anyMoreRedo());
}

void ImlibInterface::changeBrightness(double brightness)
{
    d->undoMan->addAction(new UndoActionBCG(this, d->gamma, d->brightness,
                                            d->contrast, d->gamma, brightness,
                                            d->contrast));

    imlib_context_push(d->context);
    imlib_context_set_color_modifier(d->cmod);
    imlib_reset_color_modifier();

    d->brightness += brightness/100.0;

    imlib_modify_color_modifier_gamma(d->gamma);
    imlib_modify_color_modifier_brightness(d->brightness);
    imlib_modify_color_modifier_contrast(d->contrast);

    imlib_context_set_color_modifier(0);

    imlib_context_pop();

    emit signalModified(true, d->undoMan->anyMoreRedo());
}

void ImlibInterface::changeContrast(double contrast)
{
    d->undoMan->addAction(new UndoActionBCG(this, d->gamma, d->brightness,
                                            d->contrast, d->gamma, d->brightness,
                                            contrast));

    imlib_context_push(d->context);
    imlib_context_set_color_modifier(d->cmod);
    imlib_reset_color_modifier();
    
    d->contrast += contrast/100.0;

    imlib_modify_color_modifier_gamma(d->gamma);
    imlib_modify_color_modifier_brightness(d->brightness);
    imlib_modify_color_modifier_contrast(d->contrast);

    imlib_context_set_color_modifier(0);

    imlib_context_pop();

    emit signalModified(true, d->undoMan->anyMoreRedo());
}

void ImlibInterface::changeBCG(double gamma, double brightness, double contrast)
{
    imlib_context_push(d->context);
    imlib_context_set_color_modifier(d->cmod);
    imlib_reset_color_modifier();
    
    d->gamma      = gamma;
    d->brightness = brightness;
    d->contrast   = contrast;

    imlib_modify_color_modifier_gamma(d->gamma);
    imlib_modify_color_modifier_brightness(d->brightness);
    imlib_modify_color_modifier_contrast(d->contrast);

    imlib_context_set_color_modifier(0);

    imlib_context_pop();

    emit signalModified(true, d->undoMan->anyMoreRedo());
}


void ImlibInterface::setBCG(double brightness, double contrast, double gamma)
{
    d->undoMan->addAction(new UndoActionIrreversible(this, "Brithness, Contrast, Gamma"));

    imlib_context_push(d->context);

    imlib_context_set_image(d->image);

    bool alpha = imlib_image_has_alpha();
    imlib_context_set_color_modifier(d->cmod);
    imlib_reset_color_modifier();
    
    imlib_modify_color_modifier_brightness(brightness);
    imlib_modify_color_modifier_contrast(contrast);
    imlib_modify_color_modifier_gamma(gamma);
    imlib_apply_color_modifier();

    imlib_reset_color_modifier();
    imlib_context_set_color_modifier(0);
    d->gamma      = 1.0;
    d->contrast   = 1.0;
    d->brightness = 0.0;

    // restore image's alpha setting.
    // seems applying brightness/contrast/alpha will cause alpha lookup table to
    // change which causes applying the color modifier to make an image have an
    // alpha channel even thought it didn't have one earlier.
    imlib_image_set_has_alpha(alpha ? 1:0);

    imlib_context_pop();

    emit signalModified(true, d->undoMan->anyMoreRedo());
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


void ImlibInterface::putData(const QString &caller, uint* data, int w, int h)
{
    d->undoMan->addAction(new UndoActionIrreversible(this, caller));
    putData(data, w, h);
}


void ImlibInterface::putData(uint* data, int w, int h)
{
    imlib_context_push(d->context);
    imlib_context_set_image(d->image);

    QString format(imlib_image_format());
    
    if (w != -1 && h != -1) {
    
        // New image size !
            
        Imlib_Image im = imlib_create_image_using_copied_data( w, h, data );
        imlib_free_image();
        
        imlib_context_set_image(im);
        d->image = im;

        imlib_context_set_image(d->image);
        imlib_image_set_format(format.ascii());
        
        d->origWidth = imlib_image_get_width();
        d->origHeight = imlib_image_get_height();
    }
    else {
        
        // New image data size = original data size !
        
        DATA32* ptr = imlib_image_get_data();

        memcpy(ptr, data, d->origWidth*d->origHeight*sizeof(DATA32));
    
        imlib_image_put_back_data(ptr);
    }
    
    imlib_context_pop();

    emit signalModified(true, d->undoMan->anyMoreRedo());
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

void ImlibInterface::putSelectedData(uint* data, bool saveUndo)
{
    if (!data || !d->image)
        return;

    if (saveUndo)
    {
        d->undoMan->addAction(new UndoActionIrreversible(this));
    }
    
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

    emit signalModified(true, d->undoMan->anyMoreRedo());
}

bool ImlibInterface::saveAction(const QString& saveFile, int JPEGcompression,
                                int PNGcompression, bool TIFFcompression,
                                const QString& mimeType) 
{
    kdDebug() << "Saving to :" << QFile::encodeName(saveFile).data() << " (" 
              << mimeType.ascii() << ")" << endl;
    
    if ( mimeType.upper() == QString("TIFF") || mimeType.upper() == QString("TIF") ) 
    {
        // Imlib2 uses LZW compression (not available
        // in most distributions) which results
        // in corrupted tiff files. Here we use
        // a different compression algorithm
       
        // This function is temporarily used until we move dependency to imlib2 > 1.1
        return ( saveTIFF(saveFile, TIFFcompression) );        
    }
    
    if ( !mimeType.isEmpty() )
        imlib_image_set_format(mimeType.ascii()); 
    
    // Always save JPEG files with 'JPEGcompression' compression ratio (in %).
            
    if ( mimeType.upper() == QString("JPG") || mimeType.upper() == QString("JPEG") ) 
        imlib_image_attach_data_value ("quality", NULL, JPEGcompression,
                                       (void (*)(void*, void*))NULL);
            
    // Always saving PNG files with 'PNGcompression' compression ratio (in %).
              
    if ( mimeType.upper() == QString("PNG") ) 
        imlib_image_attach_data_value ("quality", NULL, PNGcompression,
                                       (void (*)(void*, void*))NULL);

    imlib_save_image_with_error_return(QFile::encodeName(saveFile).data(), &d->errorRet);

    if( d->errorRet != IMLIB_LOAD_ERROR_NONE ) 
    {
        kdWarning() << "error saving image '" << QFile::encodeName(saveFile).data() << "', " 
                    << (int)d->errorRet << endl;
        return false;  // Do not reload the file if saving failed !
    }

    return true;
}


// This function is temporarily used until we move dependency to imlib2 > 1.1
// backported from imlib2 1.1.x
bool ImlibInterface::saveTIFF(const QString& saveFile, bool compress) 
{
    TIFF   *tif;
    DATA32 *data;
    uint32  w, h;
    bool    has_alpha;

    w = imlib_image_get_width();
    h = imlib_image_get_height();
    data = imlib_image_get_data();
    has_alpha = imlib_image_has_alpha();

    if (!data || !w || !h)
        return false;

    tif = TIFFOpen(QFile::encodeName(saveFile).data(), "w");
        
    if (tif)
    {
        TIFFSetField(tif, TIFFTAG_IMAGEWIDTH,  w);
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH, h);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);

        if (compress)
            TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
        else
            TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

        if (has_alpha)
        {
            TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
            TIFFSetField(tif, TIFFTAG_EXTRASAMPLES, EXTRASAMPLE_ASSOCALPHA);
        }
        else
        {
            TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
        }

        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, 0));
        
        uint8*              buf = 0;
        DATA32              pixel;
        double              alpha_factor;
        uint32              x, y;
        uint8               r, g, b, a = 0;
        int                 i = 0;

        buf = (uint8 *) _TIFFmalloc(TIFFScanlineSize(tif));

        if (!buf)
        {
            TIFFClose(tif);
            return false;
        }

        for (y = 0; y < h; y++)
        {
            i = 0;
            for (x = 0; x < w; x++)
            {
                pixel = data[(y * w) + x];

                r = (pixel >> 16) & 0xff;
                g = (pixel >> 8) & 0xff;
                b = pixel & 0xff;
                if (has_alpha)
                {
                    /* TIFF makes you pre-mutiply the rgb components by alpha */
                    a = (pixel >> 24) & 0xff;
                    alpha_factor =  ((double)a / 255.0);
                    r = uint8(r*alpha_factor);
                    g = uint8(g*alpha_factor);
                    b = uint8(b*alpha_factor);
                }

                /* This might be endian dependent */
                buf[i++] = r;
                buf[i++] = g;
                buf[i++] = b;
                if (has_alpha)
                    buf[i++] = a;
            }

            if (!TIFFWriteScanline(tif, buf, y, 0))
            {
                _TIFFfree(buf);
                TIFFClose(tif);
                return false;
            }

        }

        _TIFFfree(buf);
        TIFFClose(tif);

        return true;
    }

    return false;
}

void ImlibInterface::getUndoHistory(QStringList &titles)
{
    d->undoMan->getUndoHistory(titles);
}

void ImlibInterface::getRedoHistory(QStringList &titles)
{
    d->undoMan->getRedoHistory(titles);
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
