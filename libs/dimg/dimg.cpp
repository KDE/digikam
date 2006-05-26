/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2005-06-14
 * Description : digiKam 8/16 bits image management API
 * 
 * Copyright 2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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
#include <stdint.h>
}

// C++ includes.

#include <cstdio>

// Qt includes.

#include <qfile.h>
#include <qfileinfo.h>
#include <qmap.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "dcraw_parse.h"
#include "pngloader.h"
#include "jpegloader.h"
#include "tiffloader.h"
#include "ppmloader.h"
#include "rawloader.h"
#include "qimageloader.h"
#include "dimgprivate.h"
#include "dcolorcomposer.h"
#include "icctransform.h"
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

DImg::DImg(uint width, uint height, bool sixteenBit, bool alpha, uchar* data, bool copyData)
    : m_priv(new DImgPrivate)
{
    putImageData(width, height, sixteenBit, alpha, data, copyData);
}

DImg::DImg(const DImg &image, int w, int h)
    : m_priv(new DImgPrivate)
{
    // This private constructor creates a copy of everything except the data.
    // The image size is set to the given values and a buffer corresponding to these values is allocated.
    // This is used by copy and scale.
    copyImageData(image.m_priv);
    copyMetaData(image.m_priv);
    setImageDimension(w, h);
    allocateData();
}

DImg::~DImg()
{
    if (m_priv->deref())
        delete m_priv;
}


//---------------------------------------------------------------------------------------------------
// data management


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

bool DImg::operator==(const DImg& image) const
{
    return m_priv == image.m_priv;
}

void DImg::reset(void)
{
    if (m_priv->deref())
        delete m_priv;

    m_priv = new DImgPrivate;
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
    copyImageData(old);
    copyMetaData(old);

    if (old->data)
    {
        int size = allocateData();
        memcpy(m_priv->data, old->data, size);
    }

    old->deref();
}

void DImg::putImageData(uint width, uint height, bool sixteenBit, bool alpha, uchar *data, bool copyData)
{
    // set image data, metadata is untouched

    bool null = (width == 0) || (height == 0);
    // allocateData, or code below will set null to false
    setImageData(true, width, height, sixteenBit, alpha);

    // replace data
    delete [] m_priv->data;
    if (null)
    {
        // image is null - no data
        m_priv->data = 0;
    }
    else if (copyData)
    {
        int size = allocateData();
        if (data)
            memcpy(m_priv->data, data, size);
    }
    else
    {
        if (data)
        {
            m_priv->data = data;
            m_priv->null = false;
        }
        else
            allocateData();
    }
}

void DImg::putImageData(uchar *data, bool copyData)
{
    if (!data)
    {
        delete [] m_priv->data;
        m_priv->data = 0;
        m_priv->null = true;
    }
    else if (copyData)
    {
        memcpy(m_priv->data, data, numBytes());
    }
    else
    {
        m_priv->data = data;
    }
}

void DImg::resetMetaData()
{
    m_priv->attributes.clear();
    m_priv->embeddedText.clear();
    m_priv->cameraModel       = QString();
    m_priv->cameraConstructor = QString();
    m_priv->metaData.clear();
}

uchar *DImg::stripImageData()
{
    uchar *data  = m_priv->data;
    m_priv->data = 0;
    m_priv->null = true;
    return data;
}

void DImg::copyMetaData(const DImgPrivate *src)
{
    m_priv->isReadOnly        = src->isReadOnly;
    m_priv->attributes        = src->attributes;
    m_priv->embeddedText      = src->embeddedText;
    m_priv->cameraModel       = src->cameraModel;
    m_priv->cameraConstructor = src->cameraConstructor;

    // since qbytearrays are explicity shared, we need to make sure that they are
    // detached from any shared references

    for (QMap<int, QByteArray>::const_iterator it = src->metaData.begin();
         it != src->metaData.end(); ++it)
    {
        m_priv->metaData.insert(it.key(), it.data().copy());
    }
}

void DImg::copyImageData(const DImgPrivate *src)
{
    setImageData(src->null, src->width, src->height, src->sixteenBit, src->alpha);
}

int DImg::allocateData()
{
    int size = m_priv->width * m_priv->height * (m_priv->sixteenBit ? 8 : 4);
    m_priv->data = new uchar[size];
    m_priv->null = false;
    return size;
}

void DImg::setImageDimension(uint width, uint height)
{
    m_priv->width  = width;
    m_priv->height = height;
}

void DImg::setImageData(bool null, uint width, uint height, bool sixteenBit, bool alpha)
{
    m_priv->null       = null;
    m_priv->width      = width;
    m_priv->height     = height;
    m_priv->alpha      = alpha;
    m_priv->sixteenBit = sixteenBit;
}


//---------------------------------------------------------------------------------------------------
// load and save


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

bool DImg::save(const QString& filePath, const QString& format, DImgLoaderObserver *observer)
{
    if (isNull())
        return false;

    if (format.isEmpty())
        return false;

    QString frm = format.upper();

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

    // In first we trying to check the file extension. This is mandatory because
    // some tiff files are detected like RAW files by dcraw::parse method.

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists())
    {
        kdDebug() << k_funcinfo << "Failed to open file" << endl;
        return NONE;
    }
    
    QString ext = fileInfo.extension().upper();

    if (ext == QString("JPEG") || ext == QString("JPG"))
        return JPEG;
    else if (ext == QString("PNG"))
        return PNG;
    else if (ext == QString("TIFF") || ext == QString("TIF"))
        return TIFF;

    // In second, we trying to parse file header.

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
    
    DcrawParse rawFileParser;
    uchar jpegID[2]    = { 0xFF, 0xD8 };   
    uchar tiffBigID[2] = { 0x4D, 0x4D };
    uchar tiffLilID[2] = { 0x49, 0x49 };
    uchar pngID[8]     = {'\211', 'P', 'N', 'G', '\r', '\n', '\032', '\n'};
    
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


//---------------------------------------------------------------------------------------------------
// accessing properties


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

QSize DImg::size() const
{
    return QSize(m_priv->width, m_priv->height);
}

uchar* DImg::bits() const
{
    return m_priv->data;
}

uchar* DImg::scanLine(uint i) const
{
    if ( i >= height() )
        return 0;

    uchar *data = bits() + (width() * bytesDepth() * i);
    return data;
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

bool DImg::getICCProfilFromFile(const QString& filePath)
{
    QFile file(filePath);
    if ( !file.open(IO_ReadOnly) ) 
        return false;
    
    QByteArray data(file.size());
    QDataStream stream( &file );
    stream.readRawBytes(data.data(), data.size());
    setICCProfil(data);
    file.close();
    return true;
}

bool DImg::setICCProfilToFile(const QString& filePath)
{
    QFile file(filePath);
    if ( !file.open(IO_WriteOnly) ) 
        return false;
    
    QByteArray data(getICCProfil());
    QDataStream stream( &file );
    stream.writeRawBytes(data.data(), data.size());
    file.close();
    return true;
}

QByteArray DImg::getComments() const
{
    return metadata(COM);
}

QByteArray DImg::getExif() const
{
    return metadata(EXIF);
}

QByteArray DImg::getIptc() const
{
    return metadata(IPTC);
}

QByteArray DImg::getICCProfil() const
{
    return metadata(ICC);
}

void DImg::setComments(const QByteArray& commentsData)
{
    m_priv->metaData.replace(COM, commentsData);
}

void DImg::setExif(const QByteArray& exifData)
{
    m_priv->metaData.replace(EXIF, exifData);
}

void DImg::setIptc(const QByteArray& iptcData)
{
    m_priv->metaData.replace(IPTC, iptcData);
}

void DImg::setICCProfil(const QByteArray& profile)
{
    m_priv->metaData.replace(ICC, profile);
}

QByteArray DImg::metadata(DImg::METADATA key) const
{
    typedef QMap<int, QByteArray> MetaDataMap;
    
    for (MetaDataMap::iterator it = m_priv->metaData.begin(); it != m_priv->metaData.end(); ++it)
    {
        if (it.key() == key)
            return it.data();
    }

    return QByteArray();
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

QVariant DImg::attribute(const QString& key) const
{
    if (m_priv->attributes.contains(key))
        return m_priv->attributes[key];

    return QVariant();
}

void DImg::setEmbeddedText(const QString& key, const QString& text)
{
    m_priv->embeddedText.insert(key, text);
}

QString DImg::embeddedText(const QString& key) const
{
    if (m_priv->embeddedText.contains(key))
        return m_priv->embeddedText[key];

    return QString();
}

void DImg::setCameraModel(QString model)
{
    m_priv->cameraModel = model;
}

QString DImg::cameraModel() const
{
    return ( m_priv->cameraModel );
}

void DImg::setCameraConstructor(QString constructor)
{
    m_priv->cameraConstructor = constructor;
}

QString DImg::cameraConstructor() const
{
    return ( m_priv->cameraConstructor );
}

DColor DImg::getPixelColor(uint x, uint y) const
{
    if (isNull() || x > width() || y > height())
    {
        kdDebug() << k_funcinfo << " : wrong pixel position!" << endl;
        return DColor();
    }

    uchar *data = bits() + x*bytesDepth() + (width()*y*bytesDepth());

    return( DColor(data, sixteenBit()) );
}

void DImg::setPixelColor(uint x, uint y, DColor color)
{
    if (isNull() || x > width() || y > height())
    {
        kdDebug() << k_funcinfo << " : wrong pixel position!" << endl;
        return;
    }

    if (color.sixteenBit() != sixteenBit())
    {
        kdDebug() << k_funcinfo << " : wrong color depth!" << endl;
        return;
    }

    uchar *data = bits() + x*bytesDepth() + (width()*y*bytesDepth());
    color.setPixel(data);
}


//---------------------------------------------------------------------------------------------------
// copying operations


DImg DImg::copy()
{
    DImg img(*this);
    img.detach();
    return img;
}

DImg DImg::copyImageData()
{
    DImg img(width(), height(), sixteenBit(), hasAlpha(), bits(), true);
    return img;
}

DImg DImg::copyMetaData()
{
    DImg img;
    // copy width, height, alpha, sixteenBit, null
    img.copyImageData(m_priv);
    // deeply copy metadata
    img.copyMetaData(m_priv);
    // set image to null
    img.m_priv->null = true;
    return img;
}

DImg DImg::copy(QRect rect)
{
    return copy(rect.x(), rect.y(), rect.width(), rect.height());
}

DImg DImg::copy(int x, int y, int w, int h)
{
    if ( isNull() || w <= 0 || h <= 0)
    {
        kdDebug() << k_funcinfo << " : return null image!" << endl;
        return DImg();
    }

    DImg image(*this, w, h);
    image.bitBltImage(this, x, y, w, h, 0, 0);

    return image;
}


//---------------------------------------------------------------------------------------------------
// bitwise operations


void DImg::bitBltImage(const DImg* src, int dx, int dy)
{
    bitBltImage(src, 0, 0, src->width(), src->height(), dx, dy);
}

void DImg::bitBltImage(const DImg* src, int sx, int sy, int dx, int dy)
{
    bitBltImage(src, sx, sy, src->width() - sx, src->height() - sy, dx, dy);
}

void DImg::bitBltImage(const DImg* src, int sx, int sy, int w, int h, int dx, int dy)
{
    if (isNull())
       return;

    if (src->sixteenBit() != sixteenBit())
    {
        kdWarning() << "Blitting from 8-bit to 16-bit or vice versa is not supported" << endl;
        return;
    }

    if (w == -1 && h == -1)
    {
        w = src->width();
        h = src->height();
    }

    bitBlt(src->bits(), bits(), sx, sy, w, h, dx, dy,
           src->width(), src->height(), width(), height(), sixteenBit(), src->bytesDepth(), bytesDepth());
}

void DImg::bitBltImage(const uchar* src, int sx, int sy, int w, int h, int dx, int dy,
                       uint swidth, uint sheight, int sdepth)
{
    if (isNull())
        return;

    if (bytesDepth() != sdepth)
    {
        kdWarning() << "Blitting from 8-bit to 16-bit or vice versa is not supported" << endl;
        return;
    }

    if (w == -1 && h == -1)
    {
        w = swidth;
        h = sheight;
    }

    bitBlt(src, bits(), sx, sy, w, h, dx, dy, swidth, sheight, width(), height(), sixteenBit(), sdepth, bytesDepth());
}

bool DImg::normalizeRegionArguments(int &sx, int &sy, int &w, int &h, int &dx, int &dy,
                                    uint swidth, uint sheight, uint dwidth, uint dheight)
{
    if (sx < 0)
    {
        // sx is negative, so + is - and - is +
        dx -= sx;
        w  += sx;
        sx = 0;
    }

    if (sy < 0)
    {
        dy -= sy;
        h  += sy;
        sy = 0;
    }

    if (dx < 0)
    {
        sx -= dx;
        w  += dx;
        dx = 0;
    }

    if (dy < 0)
    {
        sy -= dy;
        h  += dy;
        dy = 0;
    }

    if (sx + w > (int)swidth)
    {
        w = swidth - sx;
    }

    if (sy + h > (int)sheight)
    {
        h = sheight - sy;
    }

    if (dx + w > (int)dwidth)
    {
        w = dwidth - dx;
    }

    if (dy + h > (int)dheight)
    {
        h = dheight - dy;
    }

    // Nothing left to copy
    if (w <= 0 || h <= 0)
        return false;

    return true;
}

void DImg::bitBlt (const uchar *src, uchar *dest,
                         int sx, int sy, int w, int h, int dx, int dy,
                         uint swidth, uint sheight, uint dwidth, uint dheight,
                         bool /*sixteenBit*/, int sdepth, int ddepth)
{
    // Normalize
    if (!normalizeRegionArguments(sx, sy, w, h, dx, dy, swidth, sheight, dwidth, dheight))
        return;

    // Same pixels
    if (src == dest && dx==sx && dy==sy)
        return;

    const uchar *sptr;
    uchar *dptr;
    uint   slinelength = swidth * sdepth;
    uint   dlinelength = dwidth * ddepth;

    int scurY = sy;
    int dcurY = dy;
    for (int j = 0 ; j < h ; j++, scurY++, dcurY++) 
    {
        sptr  = &src [ scurY * slinelength ] + sx * sdepth;
        dptr  = &dest[ dcurY * dlinelength ] + dx * ddepth;

            // plain and simple bitBlt
        for (int i = 0; i < w * sdepth ; i++, sptr++, dptr++)
        {
            *dptr = *sptr;
        }
    }
}

void DImg::bitBlendImage(DColorComposer *composer, const DImg* src,
                         int sx, int sy, int w, int h, int dx, int dy,
                         DColorComposer::MultiplicationFlags multiplicationFlags)
{
    if (isNull())
        return;

    if (src->sixteenBit() != sixteenBit())
    {
        kdWarning() << "Blending from 8-bit to 16-bit or vice versa is not supported" << endl;
        return;
    }

    bitBlend(composer, src->bits(), bits(), sx, sy, w, h, dx, dy,
             src->width(), src->height(), width(), height(), sixteenBit(),
             src->bytesDepth(), bytesDepth(), multiplicationFlags);
}

void DImg::bitBlend (DColorComposer *composer, const uchar *src, uchar *dest,
                     int sx, int sy, int w, int h, int dx, int dy,
                     uint swidth, uint sheight, uint dwidth, uint dheight,
                     bool sixteenBit, int sdepth, int ddepth,
                     DColorComposer::MultiplicationFlags multiplicationFlags)
{
    // Normalize
    if (!normalizeRegionArguments(sx, sy, w, h, dx, dy, swidth, sheight, dwidth, dheight))
        return;

    const uchar *sptr;
    uchar *dptr;
    uint   slinelength = swidth * sdepth;
    uint   dlinelength = dwidth * ddepth;

    int scurY = sy;
    int dcurY = dy;
    for (int j = 0 ; j < h ; j++, scurY++, dcurY++) 
    {
        sptr  = &src [ scurY * slinelength ] + sx * sdepth;
        dptr  = &dest[ dcurY * dlinelength ] + dx * ddepth;

        // blend src and destination
        for (int i = 0 ; i < w ; i++, sptr+=sdepth, dptr+=ddepth)
        {
            DColor src(sptr, sixteenBit);
            DColor dst(dptr, sixteenBit);

            // blend colors
            composer->compose(dst, src, multiplicationFlags);

            dst.setPixel(dptr);
        }
    }
}


// This method is inspired from imlib2::__imlib_BlendRGBAToRGB() (blend.c)

void DImg::bitBlend_RGBA2RGB(DImg& region, int x, int y, int w, int h)
{
    int tmp;

#define BLEND_COLOR(a, nc, c, cc) \
    tmp = ((c) - (cc)) * (a); \
    nc = (cc) + ((tmp + (tmp >> bitsDepth()) + (128*(sixteenBit() ? 256 : 1)) >> bitsDepth()));

#define BLEND(r1, g1, b1, a1, dest) \
    BLEND_COLOR(a1, dest[2], r1, dest[2]); \
    BLEND_COLOR(a1, dest[1], g1, dest[1]); \
    BLEND_COLOR(a1, dest[0], b1, dest[0]);

    if (isNull() || region.sixteenBit() != sixteenBit() ||
        w <= 0 || h <= 0)
       {
           kdDebug() << k_funcinfo << " : invalid blending arguments!" << endl;
           return;
       }

    // Normalize

    if (x < 0)
    {
       // x is negative, so + is -
       w = w + x;
       x = 0;
    }

    if (y < 0)
    {
       h = h + y;
       y = 0;
    }

    if (w > (int)width())
       w = width();

    if (h > (int)height())
       w = height();

    if (!sixteenBit())         // 8 bits image.
    {
        uchar  alpha;
        uchar *pptr;
        uchar *dst = bits();
        uchar *src = region.bits();
    
        for (int j = y ; j < (y + h) ; j++) 
        {
            pptr = &src[ j * width() * 4 ] + x * 4;
    
            for (int i = 0 ; i < w ; i++)
            {
                alpha = pptr[3];
    
                switch (alpha)
                {
                    case 0:
                        break;
        
                    default:
                        BLEND(pptr[2], pptr[1], pptr[0], alpha, dst);
                        break;
                }
                
                pptr+=4;
                dst+=4;
            }
        }
    }
    else                        // 16 bits image.
    {
        unsigned short  alpha;
        unsigned short *pptr;
        unsigned short *dst = (unsigned short *)bits();
        unsigned short *src = (unsigned short *)region.bits();
    
        for (int j = y ; j < (y + h) ; j++) 
        {
            pptr = &src[ j * width() * 4 ] + x * 4;
    
            for (int i = 0 ; i < w ; i++)
            {
                alpha = pptr[3];
    
                switch (alpha)
                {
                    case 0:
                        break;
        
                    default:
                        BLEND(pptr[2], pptr[1], pptr[0], alpha, dst);
                        break;
                }
                
                pptr+=4;
                dst+=4;
            }
        }
    }
}


//---------------------------------------------------------------------------------------------------
// QImage / QPixmap access


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

QImage DImg::copyQImage(int x, int y, int w, int h)
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

QPixmap DImg::convertToPixmap(IccTransform *monitorICCtrans)
{
    if (isNull())
        return QPixmap();

    if (!monitorICCtrans->hasOutputProfile())
    {
        kdDebug() << k_funcinfo << " : no monitor ICC profile available!" << endl;
        return convertToPixmap();
    }
    
    DImg img = copy();

    // Without embedded profile
    if (img.getICCProfil().isNull())
    {
        monitorICCtrans->apply( img );
    }
    // With embedded profile.
    else
    {
        monitorICCtrans->getEmbeddedProfile( img );
        monitorICCtrans->apply( img );
    }

    return (img.convertToPixmap());
}


//---------------------------------------------------------------------------------------------------
// basic imaging operations


void DImg::crop(QRect rect)
{
    crop(rect.x(), rect.y(), rect.width(), rect.height());
}

void DImg::crop(int x, int y, int w, int h)
{
    if ( isNull() || w <= 0 || h <= 0)
        return;

    uint  oldw = width();
    uint  oldh = height();
    uchar *old = stripImageData();

    // set new image data, bits(), width(), height() change
    setImageDimension(w, h);
    allocateData();

    // copy image region (x|y), wxh, from old data to point (0|0) of new data
    bitBlt(old, bits(), x, y, w, h, 0, 0, oldw, oldh, width(), height(), sixteenBit(), bytesDepth(), bytesDepth());
    delete [] old;
}

void DImg::resize(int w, int h)
{
    if ( w <= 0 || h <= 0)
        return;

    DImg image = smoothScale(w, h);

    delete [] m_priv->data;
    m_priv->data = image.stripImageData();
    setImageDimension(w, h);
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

            setImageDimension(w, h);

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

            setImageDimension(w, h);

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

            setImageDimension(w, h);

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

            setImageDimension(w, h);

            delete [] m_priv->data;
            m_priv->data = (uchar*)newData;
        }
        
        break;
    }
    default:
        break;
    }
}

// 15-11-2005: This method have been tested indeep with valgrind by Gilles.

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

void DImg::convertToSixteenBit()
{
    convertDepth(64);
}

void DImg::convertToEightBit()
{
    convertDepth(32);
}

void DImg::convertToDepthOfImage(const DImg *otherImage)
{
    if (otherImage->sixteenBit())
        convertToSixteenBit();
    else
        convertToEightBit();
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

void DImg::fill(DColor color)
{
    if (sixteenBit())
    {
        unsigned short *imgData16 = (unsigned short *)m_priv->data;

        for (uint i = 0 ; i < width()*height()*4 ; i+=4)
        {
            imgData16[ i ] = (unsigned short)color.blue();
            imgData16[i+1] = (unsigned short)color.green();
            imgData16[i+2] = (unsigned short)color.red();
            imgData16[i+3] = (unsigned short)color.alpha();
        }
    }
    else
    {
        uchar *imgData = m_priv->data;
        
        for (uint i = 0 ; i < width()*height()*4 ; i+=4)
        {
            imgData[ i ] = (uchar)color.blue();
            imgData[i+1] = (uchar)color.green();
            imgData[i+2] = (uchar)color.red();
            imgData[i+3] = (uchar)color.alpha();
        }
    }
}

}  // NameSpace Digikam
