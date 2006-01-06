/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2005-06-14
 * Description : main DImg framework implementation
 * 
 * Copyright 2005 by Renchi Raju, Gilles Caulier
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

// C ANSI includes.

extern "C"
{
#include <stdio.h>
#include <stdint.h>
}

// Qt includes.

#include <qfile.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "dcraw_parse.h"
#include "jpegloader.h"
#include "tiffloader.h"
#include "pngloader.h"
#include "ppmloader.h"
#include "rawloader.h"
#include "qimageloader.h"
#include "dimgprivate.h"
#include "dimg.h"

typedef uint64_t ullong;
typedef int64_t  llong;

namespace Digikam
{

DImg::DImg()
    : m_priv(new DImgPrivate)
{
}

DImg::DImg(const QString& filePath, DImgLoaderObserver *observer,
           RawDecodingSettings rawDecodingSettings)
    : m_priv(new DImgPrivate)
{
    load(filePath, observer, rawDecodingSettings);
}

DImg::DImg(const DImg& image)
{
    m_priv = image.m_priv;
    m_priv->ref();
}

DImg::DImg(uint width, uint height, bool sixteenBit, bool alpha, uchar* data)
    : m_priv(new DImgPrivate)
{
    m_priv->null       = (width == 0) || (height == 0);
    m_priv->width      = width;
    m_priv->height     = height;
    m_priv->sixteenBit = sixteenBit;
    m_priv->alpha      = alpha;

    if (sixteenBit)
    {
        m_priv->data = new uchar[width*height*8];
        if (!m_priv->data)
            return;
        if (data)
            memcpy(m_priv->data, data, width*height*8);
    }
    else
    {
        m_priv->data = new uchar[width*height*4];
        if (!m_priv->data)
            return;
        if (data)
            memcpy(m_priv->data, data, width*height*4); 
    }
}

DImg::~DImg()
{
    if (m_priv->deref())
        delete m_priv;
}

DImg& DImg::operator=(const DImg& image)
{
    if (m_priv == image.m_priv)
        return *this;

    if (m_priv->deref())
    {
        delete m_priv;
        m_priv = 0;
    }

    m_priv = image.m_priv;
    m_priv->ref();

    return *this;
}

void DImg::reset(void)
{
    if (m_priv->deref())
        delete m_priv;

    m_priv = new DImgPrivate;
}

bool DImg::load(const QString& filePath, DImgLoaderObserver *observer,
                RawDecodingSettings rawDecodingSettings)

{
    FORMAT format = fileFormat(filePath);

    switch (format)
    {
        case(NONE):
        {
            kdDebug() << filePath << " : Unknown image format !!!" << endl;
            return false;
            break;
        }
        case(JPEG):
        {
            kdDebug() << filePath << " : JPEG file identified" << endl;
            JPEGLoader loader(this);
            if (loader.load(filePath, observer))
            {
                m_priv->null       = false;
                m_priv->alpha      = loader.hasAlpha();
                m_priv->sixteenBit = loader.sixteenBit();
                m_priv->isReadOnly = loader.isReadOnly();
                return true;
            }
            break;
        }
        case(TIFF):
        {
            kdDebug() << filePath << " : TIFF file identified" << endl;
            TIFFLoader loader(this);
            if (loader.load(filePath, observer))
            {
                m_priv->null       = false;
                m_priv->alpha      = loader.hasAlpha();
                m_priv->sixteenBit = loader.sixteenBit();
                m_priv->isReadOnly = loader.isReadOnly();
                return true;
            }
            break;
        }
        case(PNG):
        {
            kdDebug() << filePath << " : PNG file identified" << endl;
            PNGLoader loader(this);
            if (loader.load(filePath, observer))
            {
                m_priv->null       = false;
                m_priv->alpha      = loader.hasAlpha();
                m_priv->sixteenBit = loader.sixteenBit();
                m_priv->isReadOnly = loader.isReadOnly();
                return true;
            }
            break;
        }
        case(PPM):
        {
            kdDebug() << filePath << " : PPM file identified" << endl;
            PPMLoader loader(this);
            if (loader.load(filePath, observer))
            {
                m_priv->null       = false;
                m_priv->alpha      = loader.hasAlpha();
                m_priv->sixteenBit = loader.sixteenBit();
                m_priv->isReadOnly = loader.isReadOnly();
                return true;
            }
            break;
        }
        case(RAW):
        {
            kdDebug() << filePath << " : RAW file identified" << endl;
            RAWLoader loader(this, rawDecodingSettings);
            if (loader.load(filePath, observer))
            {
                m_priv->null       = false;
                m_priv->alpha      = loader.hasAlpha();
                m_priv->sixteenBit = loader.sixteenBit();
                m_priv->isReadOnly = loader.isReadOnly();
                return true;
            }
            break;
        }
        default:
        {
            kdDebug() << filePath << " : QIMAGE file identified" << endl;
            QImageLoader loader(this);
            if (loader.load(filePath, observer))
            {
                m_priv->null       = false;
                m_priv->alpha      = loader.hasAlpha();
                m_priv->sixteenBit = loader.sixteenBit();
                m_priv->isReadOnly = loader.isReadOnly();
                return true;
            }
            break;
        }
    }

    return false;
}

bool DImg::save(const QString& filePath, const char* format, DImgLoaderObserver *observer)
{
    if (isNull())
        return false;
    
    QString frm = QString::fromLatin1(format);
    frm = frm.upper();
    
    if (frm == "JPEG" || frm == "JPG")
    {
        JPEGLoader loader(this);
        return loader.save(filePath, observer);
    }
    else if (frm == "PNG")
    {
        PNGLoader loader(this);
        return loader.save(filePath, observer);
    }
    else if (frm == "TIFF" || frm == "TIF")
    {
        TIFFLoader loader(this);
        return loader.save(filePath, observer);
    }
    else if (frm == "PPM")
    {
        PPMLoader loader(this);
        return loader.save(filePath, observer);
    }
    else
    {
        setAttribute("format", format);
        QImageLoader loader(this);
        return loader.save(filePath, observer);
    }

    return false;
}

DImg::FORMAT DImg::fileFormat(const QString& filePath)
{
    if ( filePath == QString::null )
        return NONE;

    FILE* f = fopen(QFile::encodeName(filePath), "rb");
    if (!f)
    {
        kdDebug() << k_funcinfo << "Failed to open file" << endl;
        return NONE;
    }

    const int headerLen = 8;
    unsigned char header[headerLen];

    if (fread(&header, 8, 1, f) != 1)
    {
        kdDebug() << k_funcinfo << "Failed to read header" << endl;
        fclose(f);
        return NONE;
    }

    fclose(f);

    DcrawParse     rawFileParser;
    unsigned short jpegID    = 0xD8FF;
    unsigned short tiffBigID = 0x4d4d;
    unsigned short tiffLilID = 0x4949;
    unsigned char  pngID[8]  = {'\211', 'P', 'N', 'G', '\r', '\n', '\032', '\n'};

    if (memcmp(&header, &jpegID, 2) == 0)            // JPEG file ?
    {
        return JPEG;
    }
    else if (memcmp(&header, &pngID, 8) == 0)        // PNG file ?
    {
        return PNG;
    }
    else if (memcmp(&header[0], "P", 1)  == 0 &&
             memcmp(&header[2], "\n", 1) == 0)       // PPM 16 bits file ?
    {
        int width, height, rgbmax;
        char nl;

        FILE *file = fopen(QFile::encodeName(filePath), "rb");
        if (fscanf (file, "P6 %d %d %d%c", &width, &height, &rgbmax, &nl) == 4) 
        {
            if (rgbmax > 255)
            {
                pclose (file);
                return PPM;
            }
        }
        pclose (file);
    }
    else if (rawFileParser.getCameraModel( QFile::encodeName(filePath), NULL, NULL) == 0)
    {
        // RAW File test using dcraw.  
        // Need to test it before TIFF because any RAW file 
        // formats using TIFF header.
        return RAW;
    }
    else if (memcmp(&header, &tiffBigID, 2) == 0 ||  // TIFF file ?
             memcmp(&header, &tiffLilID, 2) == 0)
    {
        return TIFF;
    }

    // In others cases, QImage will be used to try to open file.
    return QIMAGE;
}

bool DImg::isNull() const
{
    return m_priv->null;
}

uint DImg::width() const
{
    return m_priv->width;
}

uint DImg::height() const
{
    return m_priv->height;
}

uchar* DImg::bits() const
{
    return m_priv->data;
}

bool DImg::hasAlpha() const
{
    return m_priv->alpha;
}

bool DImg::sixteenBit() const
{
    return m_priv->sixteenBit;
}

bool DImg::isReadOnly() const
{
    return m_priv->isReadOnly;
}

QByteArray DImg::getICCProfil() const
{
    return m_priv->ICCProfil;
}

uint DImg::numBytes() const
{
    return (width() * height() * bytesDepth());
}

int DImg::bytesDepth() const
{
    if (sixteenBit())
       return 8;

    return 4;
}

int DImg::bitsDepth() const
{
    if (sixteenBit())
       return 16;

    return 8;
}

void DImg::setAttribute(const QString& key, const QVariant& value)
{
    m_priv->attributes.insert(key, value);
}

QVariant DImg::attribute(const QString& key)
{
    if (m_priv->attributes.contains(key))
        return m_priv->attributes[key];

    return QVariant();
}

void DImg::setEmbeddedText(const QString& key, const QString& text)
{
    m_priv->embeddedText.insert(key, text);
}

QString DImg::embeddedText(const QString& key)
{
    if (m_priv->embeddedText.contains(key))
        return m_priv->embeddedText[key];

    return QString();
}

void DImg::setCameraModel(QString model)
{
    m_priv->cameraModel = model;
}

QString DImg::cameraModel()
{
    return ( m_priv->cameraModel );
}

void DImg::setCameraConstructor(QString constructor)
{
    m_priv->cameraConstructor = constructor;
}

QString DImg::cameraConstructor()
{
    return ( m_priv->cameraConstructor );
}

DImg DImg::copy()
{
    DImg img(*this);
    img.detach();
    return img;
}

DImg DImg::copy(QRect rect)
{
    return copy(rect.x(), rect.y(), rect.width(), rect.height());
}

DImg DImg::copy(uint x, uint y, uint w, uint h)
{
    if ( (x+w) > width() )
        w = width() - x;

    if ( (y+h) > height() )
        h = height() - y;

    if ( w <= 0 || h <= 0)
    {
        kdDebug() << k_funcinfo << " : return null image!" << endl;
        return DImg();
    }

    DImg image(w, h, sixteenBit());

    uchar* sptr      = 0;
    uchar* dptr      = image.bits();
    uchar* origData  = bits();

    for (uint j = y ; j < (y + h) ; j++)
    {
        sptr = origData + (j * width() + x) * bytesDepth();

        for (uint i=0 ; i < w*bytesDepth() ; i++, *dptr++, *sptr++)
        {
            *dptr = *sptr;
        }
    }

    image.m_priv->alpha = hasAlpha();

    return image;
}

void DImg::bitBltImage(DImg* src, int dx, int dy)
{
    int sw = src->width();
    int sh = src->height();
    
    if (isNull() || src->sixteenBit() != sixteenBit() ||
        sw <= 0 || sh <= 0)
       return;

    // Normalize

    if (dx < 0)
    {
       sw = sw - dx;
       dx = 0;
    }

    if (dy < 0)
    {
       sh = sh - dy;
       dy = 0;
    }

    if (sw > (int)width())
    {
       sw = width();
    }

    if (sh > (int)height())
    {
       sh = height();
    }

    uchar *pptr;
    uchar *ptr  = bits();
    uchar *dptr = src->bits();

    for (int j = dy ; j < (dy + sh) ; j++) 
    {
        pptr  = &ptr[ j * width() * bytesDepth() ] + dx * bytesDepth();

        for (int i = 0; i < sw * bytesDepth() ; i++, *pptr++, *dptr++)
        {
            *pptr = *dptr;
        }
    }
}

// This method come from imlib2 and do not yet work properly with red eyes correction tool.

void DImg::bitBlend(DImg& region, int x, int y, int w, int h)
{
    if (isNull() || region.sixteenBit() != sixteenBit() ||
        w <= 0 || h <= 0)
       return;

    // Normalize

    if (x < 0)
    {
       w = w - x;
       x = 0;
    }

    if (y < 0)
    {
       h = h - y;
       y = 0;
    }

    if (w > (int)width())
    {
       w = width();
    }

    if (h > (int)height())
    {
       w = height();
    }

    uchar pow_lut[256][256];
    int i, j;

    for (i = 0; i < 256; i++)
    {
        for (j = 0; j < 256; j++)
        {
            int divisor;

            divisor = (i + (j * (255 - i)) / 255);

            if (divisor > 0)
                pow_lut[i][j] = (i * 255) / divisor;
            else
                pow_lut[i][j] = 0;
        }
    }

    int tmp;

#define B_VAL(p) ((uchar *)(p))[0]
#define G_VAL(p) ((uchar *)(p))[1]
#define R_VAL(p) ((uchar *)(p))[2]
#define A_VAL(p) ((uchar *)(p))[3]

#define BLEND_COLOR(a, nc, c, cc) \
tmp = ((c) - (cc)) * (a); \
nc = (cc) + ((tmp + (tmp >> 8) + 0x80) >> 8);

#define BLEND(r1, g1, b1, a1, dest) \
BLEND_COLOR(a1, R_VAL(dest), r1, R_VAL(dest)); \
BLEND_COLOR(a1, G_VAL(dest), g1, G_VAL(dest)); \
BLEND_COLOR(a1, B_VAL(dest), b1, B_VAL(dest));

    uchar  a, aa;
    uchar *pptr;
    uchar *src = bits();
    uchar *dst = region.bits();

    for (int j = y; j < (y + h); j++) 
    {
        pptr  = &src[ j * width() * bytesDepth() ] + x * bytesDepth();

        for (int i = 0; i < w * bytesDepth() ; i++) 
        {
            aa = A_VAL(pptr);

            switch (aa)
            {
                case 0:
                    break;

                case 255:
                    *pptr = *dst;
                    break;

                default:
                    a = pow_lut[aa][A_VAL(dst)];
                    BLEND_COLOR(aa, A_VAL(dst), 255, A_VAL(dst));
                    BLEND(R_VAL(pptr), G_VAL(pptr), B_VAL(pptr), a, dst);
                    break;
            }

            pptr++;
            dst++;
        }
    }
}

QImage DImg::copyQImage()
{
    if (isNull())
        return QImage();

    if (sixteenBit())
    {
        DImg img(*this);
        img.detach();
        img.convertDepth(32);
        return img.copyQImage();
    }

    QImage img(width(), height(), 32);

    uchar* sptr = bits();
    uint*  dptr = (uint*)img.bits();

    for (uint i=0; i < width()*height(); i++)
    {
        *dptr++ = qRgba(sptr[2], sptr[1], sptr[0], sptr[3]);
        sptr += 4;
    }

    if (hasAlpha())
    {
        img.setAlphaBuffer(true);
    }

    return img;
}

QImage DImg::copyQImage(QRect rect)
{
    return (copyQImage(rect.x(), rect.y(), rect.width(), rect.height()));
}

QImage DImg::copyQImage(uint x, uint y, uint w, uint h)
{
    if (isNull())
        return QImage();

    DImg img = copy(x, y, w, h);

    if (img.sixteenBit())
        img.convertDepth(32);

    return img.copyQImage();
}

QPixmap DImg::convertToPixmap()
{
    if (isNull())
        return QPixmap();

    if (sixteenBit())
    {
        // make fastaaaa..
        return QPixmap(copyQImage(0, 0, width(), height()));
    }

    if (QImage::systemByteOrder() == QImage::BigEndian)
    {
        QImage img(width(), height(), 32);

        uchar* sptr = bits();
        uint*  dptr = (uint*)img.bits();

        for (uint i=0; i<width()*height(); i++)
        {
            *dptr++ = qRgba(sptr[2], sptr[1], sptr[0], sptr[3]);
            sptr += 4;
        }

        if (hasAlpha())
        {
            img.setAlphaBuffer(true);
        }

        return QPixmap(img);
    }
    else
    {
        QImage img(bits(), width(), height(), 32, 0, 0, QImage::IgnoreEndian);

        if (hasAlpha())
        {
            img.setAlphaBuffer(true);
        }

        return QPixmap(img);
    }
}

void DImg::detach()
{
    // are we being shared?
    if (m_priv->count <= 1)
    {
        return;
    }

    DImgPrivate* old = m_priv;

    m_priv = new DImgPrivate;
    m_priv->null              = old->null;
    m_priv->alpha             = old->alpha;
    m_priv->sixteenBit        = old->sixteenBit;
    m_priv->isReadOnly        = old->isReadOnly;
    m_priv->width             = old->width;
    m_priv->height            = old->height;
    m_priv->attributes        = old->attributes;
    m_priv->embeddedText      = old->embeddedText;
    m_priv->ICCProfil         = old->ICCProfil;
    m_priv->cameraModel       = old->cameraModel;
    m_priv->cameraConstructor = old->cameraConstructor;

    // since qbytearrays are explicity shared, we need to make sure that they are
    // detached from any shared references
    for (QMap<int, QByteArray>::const_iterator it = old->metaData.begin();
         it != old->metaData.end(); ++it)
    {
        m_priv->metaData.insert(it.key(), it.data().copy());
    }

    if (old->data)
    {
        int size = m_priv->width * m_priv->height * (m_priv->sixteenBit ? 8 : 4);
        m_priv->data = new uchar[size];
        memcpy(m_priv->data, old->data, size);
    }
}

void DImg::crop(QRect rect)
{
    crop(rect.x(), rect.y(), rect.width(), rect.height());
}

void DImg::crop(int x, int y, int w, int h)
{
    if ( w <= 0 || h <= 0)
        return;

    uchar *newData;
    
    DImg image = copy(x, y, w, h);
    int width  = image.width();
    int height = image.height();
    
    if (sixteenBit())
    {
        newData = new uchar[width*height*8];
        memcpy (newData, image.bits(), width*height*8);
    }
    else
    {
        newData = new uchar[width*height*4];    
        memcpy (newData, image.bits(), width*height*4);
    }
    
    m_priv->width  = image.width();
    m_priv->height = image.height();
    
    delete [] m_priv->data;
    m_priv->data = newData;
}    

void DImg::resize(int w, int h)
{
    if ( w <= 0 || h <= 0)
        return;

    DImg image = smoothScale(w, h);

    int width  = image.width();
    int height = image.height();
    m_priv->width  = width;
    m_priv->height = height;

    delete [] m_priv->data;
    
    if (sixteenBit())
    {
        m_priv->data = new uchar[width*height*8];
        memcpy (m_priv->data, image.bits(), width*height*8);
    }
    else
    {
        m_priv->data = new uchar[width*height*4];    
        memcpy (m_priv->data, image.bits(), width*height*4);
    }
}    
            
void DImg::rotate(ANGLE angle)
{
    if (isNull())
        return;
    
    switch (angle)
    {
    case(ROT90):
    {
        uint w  = height();
        uint h  = width();

        if (sixteenBit())
        {
            ullong* newData = new ullong[w*h];
        
            ullong *from = (ullong*) m_priv->data;
            ullong *to;
        
            for (int y = w-1; y >=0; y--)
            {
                to = newData + y;
                
                for (uint x=0; x < h; x++)
                {
                    *to = *from++;
                    to += w;
                }
            }

            m_priv->width  = w;
            m_priv->height = h;

            delete [] m_priv->data;
            m_priv->data = (uchar*)newData;
        }
        else
        {
            uint* newData = new uint[w*h];
        
            uint *from = (uint*) m_priv->data;
            uint *to;
        
            for (int y = w-1; y >=0; y--)
            {
                to = newData + y;
                
                for (uint x=0; x < h; x++)
                {
                    *to = *from++;
                    to += w;
                }
            }

            m_priv->width  = w;
            m_priv->height = h;

            delete [] m_priv->data;
            m_priv->data = (uchar*)newData;
        }
        
        break;
    }
    case(ROT180):
    {
        uint w  = height();
        uint h  = width();

        if (sixteenBit())
        {
            ullong *line1;
            ullong *line2;

            ullong* data = (ullong*) bits();
            ullong  tmp;
        
            // can be done inplace
            for (uint y = 0; y < h/2; y++)
            {
                line1 = data + y * w;
                line2 = data + (h-y) * w;
                for (uint x=0; x < w; x++)
                {
                    tmp    = *line1;
                    *line1 = *line2;
                    *line2 = tmp;

                    line1++;
                    line2--;
                }
            }
        }
        else
        {
            uint *line1;
            uint *line2;

            uint* data = (uint*) bits();
            uint  tmp;
        
            // can be done inplace
            for (uint y = 0; y < h/2; y++)
            {
                line1 = data + y * w;
                line2 = data + (h-y) * w;
                
                for (uint x=0; x < w; x++)
                {
                    tmp    = *line1;
                    *line1 = *line2;
                    *line2 = tmp;

                    line1++;
                    line2--;
                }
            }
        }
        
        break;
    }
    case(ROT270):
    {
        uint w  = height();
        uint h  = width();

        if (sixteenBit())
        {
            ullong* newData = new ullong[w*h];
        
            ullong *from = (ullong*) m_priv->data;
            ullong *to;
        
            for (uint y = 0; y < w; y++)
            {
                to = newData + y + w*(h-1);
                
                for (uint x=0; x < h; x++)
                {
                    *to = *from++;
                    to -= w;
                }
            }

            m_priv->width  = w;
            m_priv->height = h;

            delete [] m_priv->data;
            m_priv->data = (uchar*)newData;
        }
        else
        {
            uint* newData = new uint[w*h];
        
            uint *from = (uint*) m_priv->data;
            uint *to;
        
            for (uint y = 0; y < w; y++)
            {
                to = newData + y + w*(h-1);
                
                for (uint x=0; x < h; x++)
                {
                    *to = *from++;
                    to -= w;
                }
            }

            m_priv->width  = w;
            m_priv->height = h;

            delete [] m_priv->data;
            m_priv->data = (uchar*)newData;
        }
        
        break;
    }
    default:
        break;
    }
}

// This method have been tested with valgring by Gilles.

void DImg::flip(FLIP direction)
{
    if (isNull())
        return;
    
    switch (direction)
    {
        case(HORIZONTAL):
        {
            uint w  = width();
            uint h  = height();
    
            if (sixteenBit())
            {
                unsigned short  tmp[4];
                unsigned short *beg;
                unsigned short *end;
    
                unsigned short * data = (unsigned short *)bits();
    
                // can be done inplace
                for (uint y = 0 ; y < h ; y++)
                {
                    beg = data + y * w * 4;
                    end = beg  + (w-1) * 4;
    
                    for (uint x=0 ; x < (w/2) ; x++)
                    {
                        memcpy(&tmp, beg, 8);
                        memcpy(beg, end, 8);
                        memcpy(end, &tmp, 8);
    
                        beg+=4;
                        end-=4;
                    }
                }
            }
            else
            {
                uchar  tmp[4];
                uchar *beg;
                uchar *end;
    
                uchar* data = bits();
    
                // can be done inplace
                for (uint y = 0 ; y < h ; y++)
                {
                    beg = data + y * w * 4;
                    end = beg  + (w-1) * 4;
    
                    for (uint x=0 ; x < (w/2) ; x++)
                    {
                        memcpy(&tmp, beg, 4);
                        memcpy(beg, end, 4);
                        memcpy(end, &tmp, 4);

                        beg+=4;
                        end-=4;
                    }
                }
            }
    
            break;
        }
        case(VERTICAL):
        {
            uint w  = width();
            uint h  = height();
    
            if (sixteenBit())
            {
                unsigned short  tmp[4];
                unsigned short *line1;
                unsigned short *line2;
    
                unsigned short* data = (unsigned short*) bits();
            
                // can be done inplace
                for (uint y = 0 ; y < (h/2) ; y++)
                {
                    line1 = data + y * w * 4;
                    line2 = data + (h-y-1) * w * 4;
                    
                    for (uint x=0 ; x < w ; x++)
                    {
                        memcpy(&tmp, line1, 8);
                        memcpy(line1, line2, 8);
                        memcpy(line2, &tmp, 8);

                        line1+=4;
                        line2+=4;
                    }
                }
            }
            else
            {
                uchar  tmp[4];
                uchar *line1;
                uchar *line2;
    
                uchar* data = bits();
            
                // can be done inplace
                for (uint y = 0 ; y < (h/2) ; y++)
                {
                    line1 = data + y * w * 4;
                    line2 = data + (h-y-1) * w * 4;
                    
                    for (uint x=0 ; x < w ; x++)
                    {
                        memcpy(&tmp, line1, 4);
                        memcpy(line1, line2, 4);
                        memcpy(line2, &tmp, 4);

                        line1+=4;
                        line2+=4;
                    }
                }
            }
            
            break;
        }
        default:
            break;
    }
}

void DImg::convertDepth(int depth)
{
    if (isNull())
        return;

    if (((depth == 32) && !sixteenBit()) ||
        ((depth == 64) && sixteenBit()))
        return;

    if (depth == 32)
    {
        // downgrading from 16 bit to 8 bit

        uchar*  data = new uchar[width()*height()*4];
        uchar*  dptr = data;
        ushort* sptr = (ushort*)bits();

        for (uint i=0; i<width()*height()*4; i++)
        {
            *dptr++ = (*sptr++ * 255UL)/65535UL;
        }

        delete [] m_priv->data;
        m_priv->data = data;
        m_priv->sixteenBit = false;
    }
    else if (depth == 64)
    {
        // upgrading from 8 bit to 16 bit

        uchar*  data = new uchar[width()*height()*8];
        ushort* dptr = (ushort*)data;
        uchar*  sptr = bits();

        for (uint i=0; i<width()*height()*4; i++)
        {
            *dptr++ = (*sptr++ * 65535ULL)/255ULL;
        }

        delete [] m_priv->data;
        m_priv->data = data;
        m_priv->sixteenBit = true;
    }
}

}  // NameSpace Digikam
