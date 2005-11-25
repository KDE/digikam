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

extern "C"
{
    static void dimg_tiff_warning(const char* module, const char* fmt, va_list ap)
    {
        kdDebug() << k_funcinfo << module << "::" << fmt << "::" << ap << endl;
    }
  
    static void dimg_tiff_error(const char* module, const char* fmt, va_list ap)
    {
        kdDebug() << k_funcinfo << module << "::" << fmt << "::" << ap << endl;
    }
}

TIFFLoader::TIFFLoader(DImg* image)
          : DImgLoader(image)
{
    m_hasAlpha   = false;
    m_sixteenBit = false;
}

bool TIFFLoader::load(const QString& filePath)
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
        kdWarning() << k_funcinfo << "Cannot open image file." << endl;
        return false;
    }

    // Comment this line to increase loading speed.
    //TIFFPrintDirectory(tif, stdout, 0);
    
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
        kdWarning() << k_funcinfo << "Can't handle non-stripped images." << endl;
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
    {
        m_hasAlpha = true;
    }
    else 
    {
        m_hasAlpha = false;
    }
    
    // -------------------------------------------------------------------
    // Get image data.
    
    uchar* data   = 0;
    strip_size    = TIFFStripSize(tif);
    num_of_strips = TIFFNumberOfStrips(tif);
    
    if (bits_per_sample == 16)          // 16 bits image.
    {
        data           = new uchar[w*h*8];
        m_sixteenBit   = true;
        uchar* strip   = new uchar[strip_size];
        long offset    = 0;
        long bytesRead = 0;
    
        for (tstrip_t st=0; st < num_of_strips; st++)
        {
            bytesRead = TIFFReadEncodedStrip(tif, st, strip, strip_size);
    
            if (bytesRead == -1)
            {
                kdWarning() << k_funcinfo << "Failed to read strip" << endl;
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
        data         = new uchar[w*h*4];
        m_sixteenBit = false;
      	
      	// Read the image
      	
      	int ret = TIFFReadRGBAImageOriented( tif, w, h, (uint32*)data, ORIENTATION_TOPLEFT, 0 );

        if (ret != 1)
            {
                kdWarning() << k_funcinfo << "Failed to read image on RGBA mode" << endl;
                delete [] data;
                TIFFClose(tif);
                return false;
            }
        
        // Reverse red and blue
        
        uchar ptr[4];   // One pixel to swap
        
        for (uint p = 0; p < w*h*4; p+=4)
        {
            memcpy (&ptr[0], &data[p], 4);  // Current pixel 
                
            data[ p ] = ptr[2];  // Blue
            data[p+1] = ptr[1];  // Green
            data[p+2] = ptr[0];  // Red
            data[p+3] = ptr[3];  // Alpha
        }
    }

    // -------------------------------------------------------------------
    // Read image ICC profile
    
    uchar  *profile_data=NULL;
    uint32  profile_size;
    
    if (TIFFGetField (tif, TIFFTAG_ICCPROFILE, &profile_size, &profile_data))
    {
        kdDebug() << "Reading TIFF ICC Profil" << endl;
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

    imageWidth()  = w;
    imageHeight() = h;
    imageData()   = data;
    return true;
}

bool TIFFLoader::save(const QString& filePath)
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
        kdWarning() << k_funcinfo << "Cannot open target image file." << endl;
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
    QVariant compressAttr = imageAttribute("compress");
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
        kdWarning() << k_funcinfo << "Cannot allocate memory buffer." << endl;
        TIFFClose(tif);
        return false;
    }

    for (y = 0; y < h; y++)
    {
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
            kdWarning() << k_funcinfo << "Cannot write to target file." << endl;
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
        kdDebug() << "Writing TIFF ICC Profil" << endl;
        TIFFSetField (tif, TIFFTAG_ICCPROFILE, (uint32)profile_rawdata.size(), (uchar *)profile_rawdata.data());        
    }    
    
    // -------------------------------------------------------------------
        
    TIFFClose(tif);

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
