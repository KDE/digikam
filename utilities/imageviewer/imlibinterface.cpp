//////////////////////////////////////////////////////////////////////////////
//
//    IMLIBINTERFACE.CPP
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

// Qt lib includes.

#include <qwidget.h>
#include <qstring.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qptrlist.h>

// X11 includes.

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Imlib.h>

// C Ansi include.

extern "C" 
{
#include <tiffio.h>
}

// Local includes.

#include "imlibinterface.h"


class ImImage 
{
public:

    ImImage(ImlibData *id, const QString& filename) 
        {
        w          = 0;
        h          = 0;
        ow         = 0;
        oh         = 0;
        pixmap     = 0;
        im         = 0;
        dirty      = true;
        changed    = false;
        valid      = false;

        file  = filename;
        idata = id;
        im = Imlib_load_image(idata, QFile::encodeName(file).data());
        
        if (im) 
            {
            valid = true;
            ow    = im->rgb_width;
            oh    = im->rgb_height;
            w     = ow;
            h     = oh;
            Imlib_get_image_modifier(idata, im, &mod);
            render();
            }

        }
    
    ~ImImage() 
        {
        if (im) 
            {
            if (changed)
                Imlib_kill_image(idata, im);
            else
                Imlib_destroy_image(idata, im);
            }
        if (pixmap) 
            {
            Imlib_free_pixmap(idata, pixmap);
            }
        }

    bool isValid() 
        {
        return valid;
        }

    QString filename() 
        {
        return file;
        }

    void restore() 
        {
        qDebug ("%i", changed);
        if (im && changed) 
            {
            if (pixmap)
                Imlib_free_pixmap(idata, pixmap);
            
            pixmap = 0;
            Imlib_kill_image(idata, im);
            im = Imlib_load_image(idata,
                                  QFile::encodeName(file).data());
            ow = im->rgb_width;
            oh = im->rgb_height;
            w  = ow;
            h  = oh;
            Imlib_get_image_modifier(idata, im, &mod);
            changed = false;
            dirty = true;
            }
        }

    void render() 
        {
        if (!im || !dirty) return;

        if (pixmap)
            Imlib_free_pixmap(idata, pixmap);
        pixmap = 0;

        Imlib_render(idata, im, w, h);
        pixmap = Imlib_move_image(idata, im);
        dirty  = false;
        }

    Pixmap x11Pixmap() 
        {
        if (dirty)
            render();
        return pixmap;       
        }

    int width() 
        {
        return w;
        }

    int origWidth() 
        {
        return ow;
        }

    int height() 
        {
        return h;
        }

    int origHeight() 
        {
        return oh;
        }
    
    void zoom(double zoom) 
        {
        if (!im) return;

        w  = int(ow  * zoom);
        h  = int(oh * zoom);
        dirty  = true;
        }

    void rotate90() 
        {
        if (!im) return;

        Imlib_rotate_image(idata, im, -1);
        Imlib_flip_image_horizontal(idata, im);
        ow  = im->rgb_width;
        oh = im->rgb_height;
        changed = true;
        dirty = true;
        }

    void rotate180() 
        {
        if (!im) return;

        Imlib_flip_image_horizontal(idata, im);
        Imlib_flip_image_vertical(idata, im);
        ow  = im->rgb_width;
        oh = im->rgb_height;
        changed = true;
        dirty = true;
        }

    void rotate270() 
        {
        if (!im) return;

        Imlib_rotate_image(idata, im, -1);
        Imlib_flip_image_vertical(idata, im);
        ow  = im->rgb_width;
        oh = im->rgb_height;
        changed = true;
        dirty = true;
        }

    void flipHorizontal() 
        {
        if (!im) return;

        Imlib_flip_image_horizontal(idata, im);
        ow  = im->rgb_width;
        oh = im->rgb_height;
        changed = true;
        dirty = true;
        }

    void flipVertical() 
        {
        if (!im) return;

        Imlib_flip_image_vertical(idata, im);
        ow  = im->rgb_width;
        oh = im->rgb_height;
        changed = true;
        dirty = true;
        }
    
    void crop(int x, int y, int w, int h) 
        {
        if (!im) return;

        Imlib_crop_image(idata, im, x, y, w, h);
        ow  = im->rgb_width;
        oh = im->rgb_height;
        changed = true;
        dirty = true;
        }
    
    void changeGamma(int val)  
        {
        if (!im) return;

        int nval = mod.gamma + val;
        if (nval <= 512 && nval >= 0) 
            {
            mod.gamma = nval;
            Imlib_set_image_modifier(idata, im, &mod);
            changed = true;
            dirty = true;
            }
        }

    void changeBrightness(int val)  
        {
        if (!im) return;

        int nval = mod.brightness + val;
        if (nval <= 512 && nval >= 0) 
            {
            mod.brightness = nval;
            Imlib_set_image_modifier(idata, im, &mod);
            changed = true;
            dirty = true;
            }
        }

    void changeContrast(int val)  
        {
        if (!im) return;

        int nval = mod.contrast + val;
        if (nval <= 512 && nval >= 0) 
            {
            mod.contrast = nval;
            Imlib_set_image_modifier(idata, im, &mod);
            changed = true;
            dirty = true;
            }
        }

    int save(const QString& saveFile) 
        {
        if (!im) return 0;

        int result = saveAction(saveFile);
        changed = false;
        dirty   = true;

        return result;
        }

    int saveAs(const QString& saveFile) 
        {
        if (!im) return 0;

        int result = saveAction(saveFile);
        changed = true;
        dirty   = true;

        return result;
        }
            
private:

    int saveAction(const QString& saveFile) 
        {
        // Apply the modifiers to the image
        Imlib_apply_modifiers_to_rgb(idata, im);

        QFileInfo fileInfo(saveFile);
        QString ext = fileInfo.extension(false);
        int result;
        
        if (ext.upper() == QString("TIFF") || ext.upper() == QString("TIF")) 
            {
            // Imlib uses LZW compression (not available
            // in most distributions) which results
            // in corrupted tiff files. Here we use
            // a different compression algorithm
            result = saveTIFF(saveFile, true);
            }
        else 
            {
            // Always save at 100 % quality for jpeg files
            // TODO: maybe change this later
            ImlibSaveInfo saveInfo;
            saveInfo.quality = 256;
        
            result =
                Imlib_save_image(idata, im,
                                 QFile::encodeName(saveFile).data(),
                                 &saveInfo);
            }

        // Now kill the image and re-read it from the saved file
        
        if (pixmap)
            Imlib_free_pixmap(idata, pixmap);
        
        pixmap = 0;
        Imlib_kill_image(idata, im);
        im = Imlib_load_image(idata,
                              QFile::encodeName(saveFile).data());
        ow = im->rgb_width;
        oh = im->rgb_height;
        w  = ow;
        h  = oh;
        Imlib_get_image_modifier(idata, im, &mod);

        return result;
        }

    int saveTIFF(const QString& saveFile, bool compress=false) 
        {
        TIFF               *tif;
        unsigned char      *data;
        int                 y;
        int                 w;

        tif = TIFFOpen(QFile::encodeName(saveFile).data(), "w");
        if (tif)
            {
            TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, im->rgb_width);
            TIFFSetField(tif, TIFFTAG_IMAGELENGTH, im->rgb_height);
            TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
            TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
            TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
            
            if (compress)
                TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
            else
                TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
                    {
                    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
                    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
                    w = TIFFScanlineSize(tif);
                    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,
                                 TIFFDefaultStripSize(tif, 0));
                
                    for (y = 0; y < im->rgb_height; y++)
                        {
                        data = im->rgb_data + (y * im->rgb_width * 3);
                        TIFFWriteScanline(tif, data, y, 0);
                        }
                    }
            
            TIFFClose(tif);
            return 1;
            }
        return 0;  
        }
    
    ImlibData          *idata;
    ImlibImage         *im;
    ImlibColorModifier  mod;
    Pixmap              pixmap;
    int                 w;
    int                 h;
    int                 ow;
    int                 oh;
    bool                changed;
    bool                dirty;
    bool                valid;
    QString             file;
    
};

// ----------------------------------------------------------------

class ImCache 
{
public:

    ImCache(ImlibData* id, unsigned int size) 
        {
        cache.setAutoDelete(true);
        idata = id;
        cacheSize = size;
        current = 0;
        }

    ~ImCache() 
        {
        cache.clear();
        }

    ImImage* add(const QString& file) 
        {
        if (cache.count() + 1 > cacheSize) 
            {
            if (current != cache.first())
                cache.removeFirst();
            }
            
        ImImage *im = new ImImage(idata, file);

        cache.append(im);

        return im;
        }

    ImImage* find(const QString& file) 
        {
        for (ImImage* im = cache.first(); im;
             im = cache.next()) 
            {
            if (im->filename() == file)
                return im;
            }

        return 0;
        }

     ImImage* image(const QString& file) 
        {
        if (current)
            current->restore();

        ImImage *im = find(file);
        
        if (im) 
            {
            current = im;
            return im;
            }

        current = add(file);
        return current;
        }

    ImImage* currentImage() 
        {
        return current;
        }
    
private:

    QPtrList<ImImage>  cache;
    unsigned int       cacheSize;
    ImlibData         *idata;
    ImImage           *current;
    
};

// ----------------------------------------------------------------

class ImlibInterfacePrivate 
{
public:
    
    ImlibData *idata;
    Display   *display;
    Window     win;
    GC         gc;
    QString    file;
    ImCache   *cache;

};

ImlibInterface::ImlibInterface(QWidget *parent)
{
    d = new ImlibInterfacePrivate;

    d->display = parent->x11Display();
    d->win     = parent->handle();
    d->gc = XCreateGC(parent->x11Display(),
                      RootWindow(parent->x11Display(),
                                 parent->x11Screen()),
                      0, 0);

    // Fix this parameters for now
    // TODO: Provide options for changing these settings
    ImlibInitParams par;
    par.flags = ( PARAMS_REMAP |
                  PARAMS_FASTRENDER | PARAMS_HIQUALITY |
                  PARAMS_DITHER |
                  PARAMS_IMAGECACHESIZE | PARAMS_PIXMAPCACHESIZE );
    par.remap           = 1;
    par.fastrender      = 1;
    par.hiquality       = 1;
    par.dither          = 1;
    uint maxcache       = 10240;
    par.imagecachesize  = maxcache * 1024;
    par.pixmapcachesize = maxcache * 1024;
    d->idata = Imlib_init_with_params(d->display, &par );
    
    d->cache = new ImCache(d->idata, 4);
}


ImlibInterface::~ImlibInterface()
{
    XFreeGC(d->display, d->gc);
    delete d->cache;
    delete d;
}


void ImlibInterface::load(const QString& file)
{
    d->file = file;

    if (!d->cache->find(d->file))
        d->cache->add(file);

    // Sets this item as the current
    d->cache->image(d->file);
}


void ImlibInterface::preload(const QString& file)
{
    if (!d->cache->find(file))
        d->cache->add(file);
}


void ImlibInterface::paint(int dx, int dy, int dw, int dh,
                           int sx, int sy)
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) im = d->cache->image(d->file);
    
    if (im) 
        {
        XSetGraphicsExposures(d->display, d->gc, False);
        XCopyArea(d->display,
                  im->x11Pixmap(),
                  d->win,
                  d->gc, dx, dy, dw, dh, sx, sy);
        }
}


int ImlibInterface::width()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);

    if (im)
        return im->width();
    else
        return 0;
}


int ImlibInterface::height()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im)
        return im->height();
    else
        return 0;
}


int ImlibInterface::origWidth()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);

    if (im)
        return im->origWidth();
    else
        return 0;
}


int ImlibInterface::origHeight()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);

    if (im)
        return im->origHeight();
    else
        return 0;
}


void ImlibInterface::zoom(double val)
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im)
        im->zoom(val);
}


void ImlibInterface::rotate90()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im)
        im->rotate90();
}


void ImlibInterface::rotate180()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im)
        im->rotate180();
}


void ImlibInterface::rotate270()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im)
        im->rotate270();
}


void ImlibInterface::flipHorizontal()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im)
        im->flipHorizontal();
}


void ImlibInterface::flipVertical()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im)
        im->flipVertical();
}


void ImlibInterface::crop(int x, int y, int w, int h)
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im)
        im->crop(x, y, w, h);
}


void ImlibInterface::changeGamma(int val)
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im)
        im->changeGamma(val);
}


void ImlibInterface::changeBrightness(int val)
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im)
        im->changeBrightness(val);
}


void ImlibInterface::changeContrast(int val)
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im)
        im->changeContrast(val);
}


int ImlibInterface::save(const QString& file)
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im)
        return im->save(file);
    else
        return 0;
}


int ImlibInterface::saveAs(const QString& file)
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im)
        return im->saveAs(file);
    else
        return 0;
}


void ImlibInterface::restore()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im) im->restore();
}
