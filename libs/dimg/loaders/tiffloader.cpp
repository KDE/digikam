/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2005-06-17
 * Description : A TIFF IO file for DImg framework
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

// This line must be commented to prevent any latency time
// when we use threaded image loader interface for each image
// files io. Uncomment this line only for debugging.
//#define ENABLE_DEBUG_MESSAGES 

// C ansi includes.

extern "C" 
{
#include <stdio.h>
}

// Qt includes.

#include <qfile.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "dimg.h"
#include "tiffloader.h"

namespace Digikam
{

// To manage Errors/Warnings handling provide by libtiff

void TIFFLoader::dimg_tiff_warning(const char* module, const char* fmt, va_list ap)
{
#ifdef ENABLE_DEBUG_MESSAGES    
    kdDebug() << k_funcinfo << module << "::" << fmt << "::" << ap << endl;
#endif
}

void TIFFLoader::dimg_tiff_error(const char* module, const char* fmt, va_list ap)
{
#ifdef ENABLE_DEBUG_MESSAGES    
    kdDebug() << k_funcinfo << module << "::" << fmt << "::" << ap << endl;
#endif
}

TIFFLoader::TIFFLoader(DImg* image)
          : DImgLoader(image)
{
    m_hasAlpha   = false;
    m_sixteenBit = false;
}

bool TIFFLoader::load(const QString& filePath, DImgLoaderObserver *observer, bool loadImageData)
{
    // -------------------------------------------------------------------
    // TIFF error handling. If an errors/warnings occurs during reading, 
    // libtiff will call these methods

    TIFFSetWarningHandler(dimg_tiff_warning);
    TIFFSetErrorHandler(dimg_tiff_error);

    // -------------------------------------------------------------------
    // Open the file
    
    TIFF* tif = TIFFOpen(QFile::encodeName(filePath), "r");
    if (!tif)
    {
        kdDebug() << k_funcinfo << "Cannot open image file." << endl;
        return false;
    }

#ifdef ENABLE_DEBUG_MESSAGES    
    TIFFPrintDirectory(tif, stdout, 0);
#endif
    
    // -------------------------------------------------------------------
    // Get image informations.
    
    uint32    w, h;
    uint16    bits_per_sample;
    uint16    samples_per_pixel;
    uint16    photometric;
    uint16    rows_per_strip;
    tsize_t   strip_size;
    tstrip_t  num_of_strips;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel);
    TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rows_per_strip);

    if (TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rows_per_strip) == 0)
    {
        kdDebug() << k_funcinfo << "Can't handle non-stripped images." << endl;
        TIFFClose(tif);
        return false;
    }
    
    TIFFGetFieldDefaulted(tif, TIFFTAG_PHOTOMETRIC, &photometric);
/* 
    if (photometric != PHOTOMETRIC_RGB)
    {
        kdWarning() << k_funcinfo << "Can't handle image without RGB photometric: " << photometric << endl;
        TIFFClose(tif);
        return false;
    }
  */  
    
    if (samples_per_pixel == 4)
        m_hasAlpha = true;
    else
        m_hasAlpha = false;

    if (bits_per_sample == 16)
        m_sixteenBit = true;
    else
        m_sixteenBit = false;

    if (observer)
        observer->progressInfo(m_image, 0.1);

    // -------------------------------------------------------------------
    // Get image data.
    
    uchar* data   = 0;
    
    if (loadImageData)
    {
        strip_size    = TIFFStripSize(tif);
        num_of_strips = TIFFNumberOfStrips(tif);
        
        if (bits_per_sample == 16)          // 16 bits image.
        {
            data           = new uchar[w*h*8];
            uchar* strip   = new uchar[strip_size];
            long offset    = 0;
            long bytesRead = 0;
    
            uint checkpoint = 0;
    
            for (tstrip_t st=0; st < num_of_strips; st++)
            {
    
                if (observer && st == checkpoint)
                {
                    checkpoint += granularity(observer, num_of_strips, 0.8);
                    if (!observer->continueQuery(m_image))
                    {
                        delete [] data;
                        delete [] strip;
                        TIFFClose(tif);
                        return false;
                    }
                    observer->progressInfo(m_image, 0.1 + (0.8 * ( ((float)st)/((float)num_of_strips) )));
                }
    
                bytesRead = TIFFReadEncodedStrip(tif, st, strip, strip_size);
        
                if (bytesRead == -1)
                {
                    kdDebug() << k_funcinfo << "Failed to read strip" << endl;
                    delete [] data;
                    TIFFClose(tif);
                    return false;
                }
                
                ushort *stripPtr = (ushort*)(strip);
                ushort *dataPtr  = (ushort*)(data + offset);
                ushort *p;
        
                // tiff data is read as BGR or ABGR
                
                if (samples_per_pixel == 3)
                {
                    for (int i=0; i < bytesRead/6; i++)
                    {
                        p = dataPtr;
                        
                        p[2] = *stripPtr++;
                        p[1] = *stripPtr++;
                        p[0] = *stripPtr++;
                        p[3] = 0xFFFF;
                        
                        dataPtr += 4;
                    }
                    
                    offset += bytesRead/6 * 8;
                }
                else
                {
                    for (int i=0; i < bytesRead/8; i++)
                    {
                        p = dataPtr;
                        
                        p[2] = *stripPtr++;
                        p[1] = *stripPtr++;
                        p[0] = *stripPtr++;
                        p[3] = *stripPtr++;
                        
                        dataPtr += 4;
                    }
                    
                    offset += bytesRead;
                }
            
            }
            
            delete [] strip;
        }
        else       // Non 16 bits images ==> get it on BGRA 8 bits.
        {
            data            = new uchar[w*h*4];
            uchar* strip    = new uchar[w*rows_per_strip*4];
            long offset     = 0;
            long pixelsRead = 0;
    
            // this is inspired by TIFFReadRGBAStrip, tif_getimage.c
            char          emsg[1024] = "";
            TIFFRGBAImage img;
            uint32        rows_to_read;
    
            uint checkpoint = 0;
    
            // test whether libtiff can read format and initiate reading
    
            if (!TIFFRGBAImageOK(tif, emsg) || !TIFFRGBAImageBegin(&img, tif, 0, emsg))
            {
                kdDebug() << k_funcinfo << "Failed to set up RGBA reading of image, filename " 
                        << TIFFFileName(tif) <<  " error message from Libtiff: " << emsg << endl;
                delete [] data;
                delete [] strip;
                TIFFClose(tif);
                return false;
            }
    
            img.req_orientation = ORIENTATION_TOPLEFT;
    
            // read strips from image: read rows_per_strip, so always start at beginning of a strip
            for (uint row = 0; row < h; row += rows_per_strip)
            {
    
                if (observer && row >= checkpoint)
                {
                    checkpoint += granularity(observer, h, 0.8);
                    if (!observer->continueQuery(m_image))
                    {
                        delete [] data;
                        delete [] strip;
                        TIFFClose(tif);
                        return false;
                    }
                    observer->progressInfo(m_image, 0.1 + (0.8 * ( ((float)row)/((float)h) )));
                }
    
                img.row_offset  = row;
                img.col_offset  = 0;
    
                if( row + rows_per_strip > img.height )
                    rows_to_read = img.height - row;
                else
                    rows_to_read = rows_per_strip;
    
                // Read data
    
                if (TIFFRGBAImageGet(&img, (uint32*)strip, img.width, rows_to_read ) == -1)
                {
                    kdDebug() << k_funcinfo << "Failed to read image data" << endl;
                    delete [] data;
                    delete [] strip;
                    TIFFClose(tif);
                    return false;
                }
    
                pixelsRead = rows_to_read * img.width;
    
                uchar *stripPtr = (uchar*)(strip);
                uchar *dataPtr  = (uchar*)(data + offset);
                uchar *p;
    
                // Reverse red and blue
    
                for (int i=0; i < pixelsRead; i++)
                {
                    p = dataPtr;
    
                    p[2] = *stripPtr++;
                    p[1] = *stripPtr++;
                    p[0] = *stripPtr++;
                    p[3] = *stripPtr++;
    
                    dataPtr += 4;
                }
    
                offset += pixelsRead * 4;
            }
    
            TIFFRGBAImageEnd(&img);
            delete [] strip;
    
        }
    }

    // -------------------------------------------------------------------
    // Read image ICC profile
    
    uchar  *profile_data=NULL;
    uint32  profile_size;
    
    if (TIFFGetField (tif, TIFFTAG_ICCPROFILE, &profile_size, &profile_data))
    {
        QByteArray profile_rawdata = imageICCProfil();
        profile_rawdata.resize(profile_size);
        memcpy(profile_rawdata.data(), profile_data, profile_size);
    }
    
    // -------------------------------------------------------------------
    // Get meta-data Tags contents.
    
    imageMetaData().clear();
    
    for (int t = DImg::TIF_TAG_ARTIST; t <= DImg::TIF_TAG_TARGETPRINTER; t++) 
        getTiffTextTag(tif, t);
    
    // -------------------------------------------------------------------
    
    TIFFClose(tif);

    if (observer)
        observer->progressInfo(m_image, 1.0);

    imageWidth()  = w;
    imageHeight() = h;
    imageSetAttribute("format", "TIFF");
        
    if (loadImageData)
        imageData()   = data;

    return true;
}

bool TIFFLoader::save(const QString& filePath, DImgLoaderObserver *observer)
{
    TIFF   *tif;
    uchar  *data;
    uint32  w, h;
    
    w    = imageWidth();
    h    = imageHeight();
    data = imageData();
    
    // -------------------------------------------------------------------
    // TIFF error handling. If an errors/warnings occurs during reading, 
    // libtiff will call these methods

    TIFFSetWarningHandler(dimg_tiff_warning);
    TIFFSetErrorHandler(dimg_tiff_error);

    // -------------------------------------------------------------------
    // Open the file
    
    tif = TIFFOpen(QFile::encodeName(filePath), "w");
        
    if (!tif)
    {
        kdDebug() << k_funcinfo << "Cannot open target image file." << endl;
        return false;
    }

    // -------------------------------------------------------------------
    // Set image properties
    
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH,  w);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, h);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);
 
    // Image must be compressed using deflate algorithm ?
    QVariant compressAttr = imageGetAttribute("compress");
    bool compress = compressAttr.isValid() ? compressAttr.toBool() : false;
    
    if (compress)
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
    else
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

    // Image has an alpha channel ?
    if (imageHasAlpha())
    {
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
        TIFFSetField(tif, TIFFTAG_EXTRASAMPLES, EXTRASAMPLE_ASSOCALPHA);
    }
    else
    {
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
    }

    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, imageBitsDepth());
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, 0));

    if (observer)
        observer->progressInfo(m_image, 0.1);

    // -------------------------------------------------------------------
    // Write image data
    
    uint8  *buf=0;
    uchar  *pixel;
    double  alpha_factor;
    uint32  x, y;
    uint8   r8, g8, b8, a8=0;
    uint16  r16, g16, b16, a16=0;
    int     i=0;

    buf = (uint8 *) _TIFFmalloc(TIFFScanlineSize(tif));

    if (!buf)
    {
        kdDebug() << k_funcinfo << "Cannot allocate memory buffer." << endl;
        TIFFClose(tif);
        return false;
    }

    uint checkpoint = 0;

    for (y = 0; y < h; y++)
    {

        if (observer && y == checkpoint)
        {
            checkpoint += granularity(observer, h, 0.8);
            if (!observer->continueQuery(m_image))
            {
                _TIFFfree(buf);
                TIFFClose(tif);
                return false;
            }
            observer->progressInfo(m_image, 0.1 + (0.8 * ( ((float)y)/((float)h) )));
        }

        i = 0;
        
        for (x = 0; x < w; x++)
        {
            pixel = &data[((y * w) + x) * imageBytesDepth()];
            
            if ( imageSixteenBit() )        // 16 bits image.
            {
                b16 = (uint16)(pixel[0]+256*pixel[1]);
                g16 = (uint16)(pixel[2]+256*pixel[3]);
                r16 = (uint16)(pixel[4]+256*pixel[5]);
                
                if (imageHasAlpha())
                {
                    // TIFF makes you pre-mutiply the rgb components by alpha 
                    
                    a16          = (uint16)(pixel[6]+256*pixel[7]);
                    alpha_factor = ((double)a16 / 65535.0);
                    r16          = (uint16)(r16*alpha_factor);
                    g16          = (uint16)(g16*alpha_factor);
                    b16          = (uint16)(b16*alpha_factor);
                }
    
                // This might be endian dependent 
                
                buf[i++] = (uint8)(r16);
                buf[i++] = (uint8)(r16 >> 8);
                buf[i++] = (uint8)(g16);
                buf[i++] = (uint8)(g16 >> 8);
                buf[i++] = (uint8)(b16);
                buf[i++] = (uint8)(b16 >> 8);
                
                if (imageHasAlpha())
                {
                    buf[i++] = (uint8)(a16) ;
                    buf[i++] = (uint8)(a16 >> 8) ;
                }
            }
            else                            // 8 bits image.
            {
                b8 = (uint8)pixel[0];
                g8 = (uint8)pixel[1];
                r8 = (uint8)pixel[2];
                
                if (imageHasAlpha())
                {
                    // TIFF makes you pre-mutiply the rgb components by alpha 
                    
                    a8           = (uint8)(pixel[3]);
                    alpha_factor = ((double)a8 / 255.0);
                    r8           = (uint8)(r8*alpha_factor);
                    g8           = (uint8)(g8*alpha_factor);
                    b8           = (uint8)(b8*alpha_factor);
                }
    
                // This might be endian dependent 
                
                buf[i++] = r8;
                buf[i++] = g8;
                buf[i++] = b8;
                
                if (imageHasAlpha())
                    buf[i++] = a8;
            }
        }

        if (!TIFFWriteScanline(tif, buf, y, 0))
        {
            kdDebug() << k_funcinfo << "Cannot write to target file." << endl;
            _TIFFfree(buf);
            TIFFClose(tif);
            return false;
        }

    }

    _TIFFfree(buf);
    
    // -------------------------------------------------------------------
    // Write meta-data Tags contents.
        
    for (int t = DImg::TIF_TAG_ARTIST; t <= DImg::TIF_TAG_TARGETPRINTER; t++) 
        setTiffTextTag(tif, t);
            
    // -------------------------------------------------------------------
    // Write ICC profil.
    
    QByteArray profile_rawdata = imageICCProfil();
    
    if (profile_rawdata.data() != 0)
    {
        TIFFSetField (tif, TIFFTAG_ICCPROFILE, (uint32)profile_rawdata.size(), (uchar *)profile_rawdata.data());
    }    
    
    // -------------------------------------------------------------------
        
    TIFFClose(tif);

    if (observer)
        observer->progressInfo(m_image, 1.0);

    return true;
}

bool TIFFLoader::hasAlpha() const
{
    return m_hasAlpha;    
}

bool TIFFLoader::sixteenBit() const
{
    return m_sixteenBit;    
}

void TIFFLoader::getTiffTextTag(TIFF* tif, int tag)
{
    imageMetaData().clear();
    char   *text;
    ttag_t  tiffTag;
    
    switch (tag)
    {
        case DImg::TIF_TAG_ARTIST:
            tiffTag = TIFFTAG_ARTIST;
            break;
        case DImg::TIF_TAG_COPYRIGHT:
            tiffTag = TIFFTAG_COPYRIGHT;
            break;
        case DImg::TIF_TAG_DATETIME:
            tiffTag = TIFFTAG_DATETIME;
            break;
        case DImg::TIF_TAG_DOCUMENTNAME:
            tiffTag = TIFFTAG_DOCUMENTNAME;
            break;
        case DImg::TIF_TAG_HOSTCOMPUTER:
            tiffTag = TIFFTAG_HOSTCOMPUTER;
            break;
        case DImg::TIF_TAG_IMAGEDESCRIPTION:
            tiffTag = TIFFTAG_IMAGEDESCRIPTION;
            break;
        case DImg::TIF_TAG_INKNAMES:
            tiffTag = TIFFTAG_INKNAMES;
            break;
        case DImg::TIF_TAG_MAKE:
            tiffTag = TIFFTAG_MAKE;
            break;
        case DImg::TIF_TAG_MODEL:
            tiffTag = TIFFTAG_MODEL;
            break;
        case DImg::TIF_TAG_PAGENAME:
            tiffTag = TIFFTAG_PAGENAME;
            break;
        case DImg::TIF_TAG_SOFTWARE:
            tiffTag = TIFFTAG_SOFTWARE;
            break;
        case DImg::TIF_TAG_TARGETPRINTER:
            tiffTag = TIFFTAG_TARGETPRINTER;
            break;
        default:
            return;
    }
    
    if (TIFFGetField(tif, tiffTag, &text) == 1)        
    {
        QByteArray ba(strlen(text));
        memcpy(ba.data(), text, strlen(text));
        imageMetaData().insert(tag, ba);

        if (tiffTag == TIFFTAG_MODEL)
           imageSetCameraModel(QString::QString(text));

        if (tiffTag == TIFFTAG_MAKE)
           imageSetCameraConstructor(QString::QString(text));
    }
}

void TIFFLoader::setTiffTextTag(TIFF* tif, int tag)
{
    ttag_t  tiffTag;
    
    switch (tag)
    {
        case DImg::TIF_TAG_ARTIST:
            tiffTag = TIFFTAG_ARTIST;
            break;
        case DImg::TIF_TAG_COPYRIGHT:
            tiffTag = TIFFTAG_COPYRIGHT;
            break;
        case DImg::TIF_TAG_DATETIME:
            tiffTag = TIFFTAG_DATETIME;
            break;
        case DImg::TIF_TAG_DOCUMENTNAME:
            tiffTag = TIFFTAG_DOCUMENTNAME;
            break;
        case DImg::TIF_TAG_HOSTCOMPUTER:
            tiffTag = TIFFTAG_HOSTCOMPUTER;
            break;
        case DImg::TIF_TAG_IMAGEDESCRIPTION:
            tiffTag = TIFFTAG_IMAGEDESCRIPTION;
            break;
        case DImg::TIF_TAG_INKNAMES:
            tiffTag = TIFFTAG_INKNAMES;
            break;
        case DImg::TIF_TAG_MAKE:
            tiffTag = TIFFTAG_MAKE;
            break;
        case DImg::TIF_TAG_MODEL:
            tiffTag = TIFFTAG_MODEL;
            break;
        case DImg::TIF_TAG_PAGENAME:
            tiffTag = TIFFTAG_PAGENAME;
            break;
        case DImg::TIF_TAG_SOFTWARE:
            tiffTag = TIFFTAG_SOFTWARE;
            break;
        case DImg::TIF_TAG_TARGETPRINTER:
            tiffTag = TIFFTAG_TARGETPRINTER;
            break;
        default:
            return;
    }
    
    typedef QMap<int, QByteArray> MetaDataMap;
    MetaDataMap map = imageMetaData();
    
    if (map.contains(tag))
    {
        QByteArray ba = map[tag];
        TIFFSetField (tif, tiffTag, (uint32)ba.size(), (uchar *)ba.data());        
    }
}

}  // NameSpace Digikam
