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
#include <Imlib2.h>

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

    ImImage(const QString& filename) 
        {
        w          = 0;
        h          = 0;
        ow         = 0;
        oh         = 0;
        
        gamma      = 1.0;
        brightness = 0.0;
        contrast   = 1.0;
        
        pixmap     = 0;
        im         = 0;
        
        dirty      = true;
        changed    = false;
        valid      = false;
        
        file       = filename;
        
        im = imlib_load_image_with_error_return(QFile::encodeName(file).data(), &errorRet);
        
        if (im) 
            {
            valid = true;
            imlib_context_set_image(im);
            
            ow = imlib_image_get_width();
            oh = imlib_image_get_height();
            w     = ow;
            h     = oh;
            
            mod = imlib_create_color_modifier();
            imlib_context_set_color_modifier(mod);
            
            if (mod == NULL) qDebug ("color modifier is null");
            
            render();
            }
        else
            qDebug("error loading image '%s': %i", QFile::encodeName(file).data(),(int)errorRet);
        }
    
    ~ImImage() 
        {
        if (im) 
            {
            imlib_context_set_image(im);
            imlib_free_image();
            }
            
        if (pixmap) 
            {
            imlib_context_set_mask(pixmap);
            imlib_free_pixmap_and_mask(pixmap); 
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
        if (im && changed) 
            {
            if (pixmap)
                {
                imlib_context_set_mask(pixmap);
                imlib_free_pixmap_and_mask(pixmap); 
                }
            
            pixmap = 0;
            
            imlib_context_set_image(im);
            imlib_free_image();
            
            im = imlib_load_image_with_error_return(QFile::encodeName(file).data(), &errorRet);
            imlib_context_set_image(im);
            
            ow = imlib_image_get_width();
            oh = imlib_image_get_height();
            w  = ow;
            h  = oh;
            
            mod = imlib_create_color_modifier();
            imlib_context_set_color_modifier(mod);
            
            if (mod == NULL) qDebug ("color modifier is null");
            
            changed = false;
            dirty   = true;
            }
        }

    void render() 
        {
        if (!im || !dirty) return;

        if (pixmap)
            {
            imlib_context_set_mask(pixmap);
            imlib_free_pixmap_and_mask(pixmap); 
            }
            
        pixmap = 0;
        
        imlib_context_set_image(im);
        imlib_render_pixmaps_for_whole_image_at_size(&pixmap, &mask, w, h);
        
        dirty  = false;
        }

    Pixmap x11Pixmap() 
        {
        if (dirty) render();
        
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

        w  = int(ow * zoom);
        h  = int(oh * zoom);
        dirty  = true;
        }

    void rotate90() 
        {
        if (!im) return;

        imlib_image_orientate(1);
        
        ow = imlib_image_get_width();
        oh = imlib_image_get_height();
        
        changed = true;
        dirty = true;
        }

    void rotate180() 
        {
        if (!im) return;

        imlib_image_orientate(2);

        ow = imlib_image_get_width();
        oh = imlib_image_get_height();
        
        changed = true;
        dirty = true;
        }

    void rotate270() 
        {
        if (!im) return;

        imlib_image_orientate(3);

        ow = imlib_image_get_width();
        oh = imlib_image_get_height();
                
        changed = true;
        dirty = true;
        }

    void flipHorizontal() 
        {
        if (!im) return;

        imlib_image_flip_horizontal();
        
        ow = imlib_image_get_width();
        oh = imlib_image_get_height();

        changed = true;
        dirty = true;
        }

    void flipVertical() 
        {
        if (!im) return;

        imlib_image_flip_vertical();
        
        ow = imlib_image_get_width();
        oh = imlib_image_get_height();
        
        changed = true;
        dirty = true;
        }
    
    void crop(int x, int y, int w, int h) 
        {
        if (!im) return;

        im = imlib_create_cropped_image(x, y, w, h);
        
        imlib_context_set_image(im);        
        ow = imlib_image_get_width();
        oh = imlib_image_get_height();
        
        changed = true;
        dirty = true;
        }
    
    void changeGamma(double val)  
        {
        if (!im) return;
        
        double nval = gamma + val;
        
        if ( nval <= 5.0 && nval >= 0.0 ) 
            {            
            imlib_modify_color_modifier_gamma(nval);
            imlib_apply_color_modifier();
            gamma   = nval;
            changed = true;
            dirty   = true;
            qDebug("gamma:%f", (float)nval);
            }
        }

    void changeBrightness(double val)  
        {
        if (!im) return;
        
        double nval = brightness + val;
        
        if ( nval <= 1.0 && nval >= -1.0 ) 
            {
            imlib_modify_color_modifier_brightness(nval);
            imlib_apply_color_modifier();
            brightness = nval;
            changed    = true;
            dirty      = true;
            qDebug("brightness:%f", (float)nval);
            }
        }

    void changeContrast(double val)  
        {
        if (!im) return;

        double nval = contrast + val;
        
        if ( nval <= 10.0 && nval >= -10.0 ) 
            {
            imlib_modify_color_modifier_contrast(nval);
            imlib_apply_color_modifier();
            contrast = nval;
            changed  = true;
            dirty    = true;
            qDebug("contrast:%f", (float)nval);
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
        
        imlib_context_set_image(im);
        imlib_apply_color_modifier(); 
        
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
            
            //ImlibSaveInfo saveInfo;
            //saveInfo.quality = 256;
            
            imlib_save_image_with_error_return(QFile::encodeName(saveFile).data(), &errorRet);
            result = (int)errorRet;
            }

        // Now kill the image and re-read it from the saved file
        
        if (pixmap)
            {
            imlib_context_set_mask(pixmap);
            imlib_free_pixmap_and_mask(pixmap); 
            }
        
        pixmap = 0;

        imlib_context_set_image(im);
        imlib_free_image();
        
        im = imlib_load_image_with_error_return(QFile::encodeName(saveFile).data(), &errorRet);
        imlib_context_set_image(im);
        
        ow = imlib_image_get_width();
        oh = imlib_image_get_height();
        w  = ow;
        h  = oh;

        mod = imlib_context_get_color_modifier();

        return result;
        }

    int saveTIFF(const QString& saveFile, bool compress=false) 
        {
        TIFF               *tif;
        DATA32             *data;
        int                 y;
        int                 w;

        tif = TIFFOpen(QFile::encodeName(saveFile).data(), "w");
        
        if (tif)
            {
            TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, imlib_image_get_width());
            TIFFSetField(tif, TIFFTAG_IMAGELENGTH, imlib_image_get_height());
            TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
            TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
            TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
            
            if (compress)
                TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
            else
                TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
                    {
                    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
                    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
                    w = TIFFScanlineSize(tif);
                    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,
                                 TIFFDefaultStripSize(tif, 0));
                
                    for (y = 0 ; y < imlib_image_get_height() ; ++y)
                        {
                        data = imlib_image_get_data() + (DATA32)( y * imlib_image_get_width() );
                        TIFFWriteScanline(tif, data, y, 0);
                        }
                    }
            
            TIFFClose(tif);
            return 1;
            }
        return 0;  
        }
    
    Imlib_Image           im;
    Imlib_Color_Modifier  mod;
    Pixmap                pixmap;
    Pixmap                mask;
    Imlib_Load_Error      errorRet;
    
    int                   w;
    int                   h;
    int                   ow;
    int                   oh;
    
    double                gamma;
    double                brightness;
    double                contrast;
        
    bool                  changed;
    bool                  dirty;
    bool                  valid;
    
    QString               file;
};

// ----------------------------------------------------------------

class ImCache 
{
public:

    ImCache(unsigned int size) 
        {
        cache.setAutoDelete(true);
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
            
        ImImage *im = new ImImage(file);

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
    ImImage           *current;    
};

// ----------------------------------------------------------------

class ImlibInterfacePrivate 
{
public:
    
    Display   *display;
    Window     win;
    GC         gc;
    Visual    *vis;
    Colormap   cm;
    
    QString    file;
    ImCache   *cache;
};

ImlibInterface::ImlibInterface(QWidget *parent)
{
    d = new ImlibInterfacePrivate;

    d->display = parent->x11Display();
    d->win     = parent->handle();
    d->gc      = XCreateGC(parent->x11Display(),
                           RootWindow(parent->x11Display(),
                           parent->x11Screen()),
                           0, 0);

    // Fix this parameters for now
    // TODO: Provide options for changing these settings
    
    uint maxcache       = 10240;
    
    d->vis   = DefaultVisual(d->display, DefaultScreen(d->display));
    d->cm    = DefaultColormap(d->display, DefaultScreen(d->display));
    
    /* ImlibInitParams par;
    par.flags = ( PARAMS_REMAP |
                  PARAMS_FASTRENDER | PARAMS_HIQUALITY |
                  PARAMS_DITHER |
                  PARAMS_IMAGECACHESIZE | PARAMS_PIXMAPCACHESIZE );
    par.remap           = 1;
    par.fastrender      = 1;
    par.hiquality       = 1;
    par.dither          = 1;
    par.imagecachesize  = maxcache * 1024;
    par.pixmapcachesize = maxcache * 1024;
    d->idata = Imlib_init_with_params(d->display, &par );*/
   
    /* set our cache to maxcache (Mb) so it doesn't have to go hit the disk as long as */
    /* the images we use use less than maxcache of RAM (that is uncompressed) */ 
    imlib_set_cache_size(maxcache * 1024);
    /* set the maximum number of colors to allocate for 8bpp and less to 128 */
    imlib_set_color_usage(128);
    /* dither for depths < 24bpp */
    imlib_context_set_dither(1);
    /* set the display , visual, colormap and drawable we are using */
    imlib_context_set_display(d->display);
    imlib_context_set_visual(d->vis);
    imlib_context_set_colormap(d->cm);
    imlib_context_set_drawable(d->win);
   
    d->cache = new ImCache(4);
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
    if ( !d->cache->find(file) )
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
    
    if (im) im->zoom(val);
}


void ImlibInterface::rotate90()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im) im->rotate90();
}


void ImlibInterface::rotate180()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im) im->rotate180();
}


void ImlibInterface::rotate270()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im) im->rotate270();
}


void ImlibInterface::flipHorizontal()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im) im->flipHorizontal();
}


void ImlibInterface::flipVertical()
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im) im->flipVertical();
}


void ImlibInterface::crop(int x, int y, int w, int h)
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im) im->crop(x, y, w, h);
}


void ImlibInterface::changeGamma(double val)
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im) im->changeGamma(val);
}


void ImlibInterface::changeBrightness(double val)
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im) im->changeBrightness(val);
}


void ImlibInterface::changeContrast(double val)
{
    ImImage *im = d->cache->currentImage();
    
    if (!im) d->cache->image(d->file);
    
    if (im) im->changeContrast(val);
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

#include "imlibinterface.moc"
