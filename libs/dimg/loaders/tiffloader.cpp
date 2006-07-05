/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date  : 2005-06-17
 * Description : A TIFF IO file for DImg framework
 * 
 * Copyright 2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006 by Gilles Caulier
 *
 * Specifications & references:
 * - TIFF 6.0  : http://partners.adobe.com/public/developer/en/tiff/TIFF6.pdf
 * - TIFF/EP   : http://www.map.tu.chiba-u.ac.jp/IEC/100/TA2/recdoc/N4378.pdf
 * - TIFF/Tags : http://www.awaresystems.be/imaging/tiff/tifftags.html
 * - DNG       : http://www.adobe.com/products/dng/pdfs/dng_spec.pdf
 *
 * Others Linux Tiff Loader implementation using libtiff:
 * - http://artis.inrialpes.fr/Software/TiffIO/
 * - http://cvs.graphicsmagick.org/cgi-bin/cvsweb.cgi/GraphicsMagick/coders/tiff.c
 * - http://freeimage.cvs.sourceforge.net/freeimage/FreeImage/Source/FreeImage/PluginTIFF.cpp
 * - http://freeimage.cvs.sourceforge.net/freeimage/FreeImage/Source/Metadata/XTIFF.cpp
 * - https://subversion.imagemagick.org/subversion/ImageMagick/trunk/coders/tiff.c
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
#include <tiffvers.h>
}

// C++ includes.

#include <cstdio>

// Qt includes.

#include <qfile.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "version.h"
#include "dimg.h"
#include "dimgloaderobserver.h"
#include "dmetadata.h"
#include "tiffloader.h"

namespace Digikam
{

// To manage Errors/Warnings handling provide by libtiff

void TIFFLoader::dimg_tiff_warning(const char* module, const char* fmt, va_list ap)
{
    kdDebug() << k_funcinfo << module << "::" << fmt << "::" << ap << endl;
}

void TIFFLoader::dimg_tiff_error(const char* module, const char* fmt, va_list ap)
{
    kdDebug() << k_funcinfo << module << "::" << fmt << "::" << ap << endl;
}

TIFFLoader::TIFFLoader(DImg* image)
          : DImgLoader(image)
{
    m_hasAlpha   = false;
    m_sixteenBit = false;
}

bool TIFFLoader::load(const QString& filePath, DImgLoaderObserver *observer)
{
    readMetadata(filePath, DImg::TIFF);

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

    // -------------------------------------------------------------------
    // Read image ICC profile
    
    QMap<int, QByteArray>& metaData = imageMetaData();

    uchar  *profile_data=NULL;
    uint32  profile_size;
    
    if (TIFFGetField (tif, TIFFTAG_ICCPROFILE, &profile_size, &profile_data))
    {
        QByteArray profile_rawdata(profile_size);
        memcpy(profile_rawdata.data(), profile_data, profile_size);
        metaData.insert(DImg::ICC, profile_rawdata);
    }
    else
    {
        // If ICC profile is null, check Exif metadata.
        checkExifWorkingColorSpace();
    }

    // -------------------------------------------------------------------
    // Get image data.

    if (observer)
        observer->progressInfo(m_image, 0.1);
    
    uchar* data   = 0;
    
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

    // -------------------------------------------------------------------
    
    TIFFClose(tif);

    if (observer)
        observer->progressInfo(m_image, 1.0);

    imageWidth()  = w;
    imageHeight() = h;
    imageData()   = data;
    imageSetAttribute("format", "TIFF");
    
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
    
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH,     w);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH,    h);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC,    PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG,   PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_ORIENTATION,    ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);
 
    // Image must be compressed using deflate algorithm ?
    QVariant compressAttr = imageGetAttribute("compress");
    bool compress = compressAttr.isValid() ? compressAttr.toBool() : false;
    
    if (compress)
    {
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_ADOBE_DEFLATE);
        TIFFSetField(tif, TIFFTAG_ZIPQUALITY,  9);
        // NOTE : this tag values aren't defined in libtiff 3.6.1. '2' is PREDICTOR_HORIZONTAL.
        //        Use horizontal differencing for images which are
        //        likely to be continuous tone. The TIFF spec says that this
        //        usually leads to better compression.
        //        See this url for more details:
        //        http://www.awaresystems.be/imaging/tiff/tifftags/predictor.html
        TIFFSetField(tif, TIFFTAG_PREDICTOR,   2); 
    }
    else
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

    // Image has an alpha channel ?
    if (imageHasAlpha())
    {
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
        TIFFSetField(tif, TIFFTAG_EXTRASAMPLES,    EXTRASAMPLE_ASSOCALPHA);
    }
    else
    {
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
    }

    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, imageBitsDepth());
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,  TIFFDefaultStripSize(tif, 0));

    // -------------------------------------------------------------------
    // Write meta-data Tags contents. 
    // TODO : - this code will be removed when Exiv2 will support TIFF in writting mode
        
    DMetadata metaData;
    metaData.setExif(m_image->getExif());
    metaData.setIptc(m_image->getIptc());

    // Standard IPTC tag (available with libtiff 3.6.1)    

    QByteArray ba = metaData.getIptc(true);
    if (!ba.isEmpty()) 
    {
#if defined(TIFFTAG_PHOTOSHOP)
        TIFFSetField (tif, TIFFTAG_PHOTOSHOP, (uint32)ba.size(), (uchar *)ba.data());
#endif
    }

    // Standard XMP tag (available with libtiff 3.6.1)    

#if defined(TIFFTAG_XMLPACKET)
    tiffSetExifDataTag(tif, TIFFTAG_XMLPACKET,                &metaData, "Exif.Image.XMLPacket");
#endif

    // Standard Exif Ascii tags (available with libtiff 3.6.1)    

    tiffSetExifAsciiTag(tif, TIFFTAG_DOCUMENTNAME,            &metaData, "Exif.Image.DocumentName");
    tiffSetExifAsciiTag(tif, TIFFTAG_IMAGEDESCRIPTION,        &metaData, "Exif.Image.ImageDescription");
    tiffSetExifAsciiTag(tif, TIFFTAG_MAKE,                    &metaData, "Exif.Image.Make");
    tiffSetExifAsciiTag(tif, TIFFTAG_MODEL,                   &metaData, "Exif.Image.Model");
    tiffSetExifAsciiTag(tif, TIFFTAG_DATETIME,                &metaData, "Exif.Image.DateTime");
    tiffSetExifAsciiTag(tif, TIFFTAG_ARTIST,                  &metaData, "Exif.Image.Artist");
    tiffSetExifAsciiTag(tif, TIFFTAG_COPYRIGHT,               &metaData, "Exif.Image.Copyright");

    QString soft = metaData.getExifTagString("Exif.Image.Software");
    QString libtiffver(TIFFLIB_VERSION_STR);
    libtiffver.replace('\n', ' ');
    soft.append(QString(" ( %1 )").arg(libtiffver));
    TIFFSetField(tif, TIFFTAG_SOFTWARE, (const char*)soft.ascii());

    // Standard Exif.Photo tags (available with libtiff 3.8.2).  
/*
#if defined(TIFFTAG_EXIFIFD)
    long sub_offset=0;
    TIFFSetField(tif, TIFFTAG_SUBIFD, 1, &sub_offset);
#endif  

#if defined(EXIFTAG_EXPOSURETIME)
    tiffSetExifDataTag(tif, EXIFTAG_EXPOSURETIME,             &metaData, "Exif.Photo.ExposureTime");
#endif  
#if defined(EXIFTAG_FNUMBER)
    tiffSetExifDataTag(tif, EXIFTAG_FNUMBER,                  &metaData, "Exif.Photo.FNumber");
#endif  
#if defined(EXIFTAG_EXPOSUREPROGRAM)
    tiffSetExifDataTag(tif, EXIFTAG_EXPOSUREPROGRAM,          &metaData, "Exif.Photo.ExposureProgram");
#endif  
#if defined(EXIFTAG_SPECTRALSENSITIVITY)
    tiffSetExifAsciiTag(tif, EXIFTAG_SPECTRALSENSITIVITY,     &metaData, "Exif.Photo.SpectralSensitivity");
#endif
#if defined(EXIFTAG_ISOSPEEDRATINGS)
    tiffSetExifDataTag(tif, EXIFTAG_ISOSPEEDRATINGS,          &metaData, "Exif.Photo.ISOSpeedRatings");
#endif  
#if defined(EXIFTAG_OECF)
    tiffSetExifDataTag(tif, EXIFTAG_OECF,                     &metaData, "Exif.Photo.OECF");
#endif  
#if defined(EXIFTAG_EXIFVERSION)
    tiffSetExifDataTag(tif, EXIFTAG_EXIFVERSION,              &metaData, "Exif.Photo.ExifVersion");
#endif  
#if defined(EXIFTAG_DATETIMEORIGINAL)
    tiffSetExifAsciiTag(tif, EXIFTAG_DATETIMEORIGINAL,        &metaData, "Exif.Photo.DateTimeOriginal");
#endif
#if defined(EXIFTAG_DATETIMEDIGITIZED)
    tiffSetExifAsciiTag(tif, EXIFTAG_DATETIMEDIGITIZED,       &metaData, "Exif.Photo.DateTimeDigitized");
#endif
#if defined(EXIFTAG_COMPONENTSCONFIGURATION)
    tiffSetExifDataTag(tif, EXIFTAG_COMPONENTSCONFIGURATION,  &metaData, "Exif.Photo.ComponentsConfiguration");
#endif  
#if defined(EXIFTAG_COMPRESSEDBITSPERPIXEL)
    tiffSetExifDataTag(tif, EXIFTAG_COMPRESSEDBITSPERPIXEL,   &metaData, "Exif.Photo.CompressedBitsPerPixel");
#endif  
#if defined(EXIFTAG_SHUTTERSPEEDVALUE)
    tiffSetExifDataTag(tif, EXIFTAG_SHUTTERSPEEDVALUE,        &metaData, "Exif.Photo.ShutterSpeedValue");
#endif  
#if defined(EXIFTAG_APERTUREVALUE)
    tiffSetExifDataTag(tif, EXIFTAG_APERTUREVALUE,            &metaData, "Exif.Photo.ApertureValue");
#endif
#if defined(EXIFTAG_BRIGHTNESSVALUE)
    tiffSetExifDataTag(tif, EXIFTAG_BRIGHTNESSVALUE,          &metaData, "Exif.Photo.BrightnessValue");
#endif
#if defined(EXIFTAG_EXPOSUREBIASVALUE)
    tiffSetExifDataTag(tif, EXIFTAG_EXPOSUREBIASVALUE,        &metaData, "Exif.Photo.ExposureBiasValue");
#endif
#if defined(EXIFTAG_MAXAPERTUREVALUE)
    tiffSetExifDataTag(tif, EXIFTAG_MAXAPERTUREVALUE,         &metaData, "Exif.Photo.MaxApertureValue");
#endif
#if defined(EXIFTAG_SUBJECTDISTANCE)
    tiffSetExifDataTag(tif, EXIFTAG_SUBJECTDISTANCE,          &metaData, "Exif.Photo.SubjectDistance");
#endif
#if defined(EXIFTAG_METERINGMODE)
    tiffSetExifDataTag(tif, EXIFTAG_METERINGMODE,             &metaData, "Exif.Photo.MeteringMode");
#endif
#if defined(EXIFTAG_LIGHTSOURCE)
    tiffSetExifDataTag(tif, EXIFTAG_LIGHTSOURCE,              &metaData, "Exif.Photo.LightSource");
#endif
#if defined(EXIFTAG_FLASH)
    tiffSetExifDataTag(tif, EXIFTAG_FLASH,                    &metaData, "Exif.Photo.Flash");
#endif
#if defined(EXIFTAG_FOCALLENGTH)
    tiffSetExifDataTag(tif, EXIFTAG_FOCALLENGTH,              &metaData, "Exif.Photo.FocalLength");
#endif
#if defined(EXIFTAG_SUBJECTAREA)
    tiffSetExifDataTag(tif, EXIFTAG_SUBJECTAREA,              &metaData, "Exif.Photo.SubjectArea");
#endif
#if defined(EXIFTAG_MAKERNOTE)
    tiffSetExifDataTag(tif, EXIFTAG_MAKERNOTE,                &metaData, "Exif.Photo.MakerNote");
#endif
#if defined(EXIFTAG_USERCOMMENT)   
    tiffSetExifDataTag(tif, EXIFTAG_USERCOMMENT,              &metaData, "Exif.Photo.UserComment");
#endif
#if defined(EXIFTAG_SUBSECTIME)
    tiffSetExifAsciiTag(tif, EXIFTAG_SUBSECTIME,              &metaData, "Exif.Photo.SubSecTime");
#endif
#if defined(EXIFTAG_SUBSECTIMEORIGINAL)
    tiffSetExifAsciiTag(tif, EXIFTAG_SUBSECTIMEORIGINAL,      &metaData, "Exif.Photo.SubSecTimeOriginal");
#endif
#if defined(EXIFTAG_SUBSECTIMEDIGITIZED)
    tiffSetExifAsciiTag(tif, EXIFTAG_SUBSECTIMEDIGITIZED,     &metaData, "Exif.Photo.SubSecTimeDigitized");
#endif
#if defined(EXIFTAG_FLASHPIXVERSION)
    tiffSetExifDataTag(tif, EXIFTAG_FLASHPIXVERSION,          &metaData, "Exif.Photo.FlashpixVersion");
#endif
#if defined(EXIFTAG_COLORSPACE)
    tiffSetExifDataTag(tif, EXIFTAG_COLORSPACE,               &metaData, "Exif.Photo.ColorSpace");
#endif
#if defined(EXIFTAG_PIXELXDIMENSION)
    tiffSetExifDataTag(tif, EXIFTAG_PIXELXDIMENSION,          &metaData, "Exif.Photo.PixelXDimension");
#endif
#if defined(EXIFTAG_PIXELYDIMENSION)
    tiffSetExifDataTag(tif, EXIFTAG_PIXELYDIMENSION,          &metaData, "Exif.Photo.PixelYDimension");
#endif
#if defined(EXIFTAG_RELATEDSOUNDFILE)
    tiffSetExifAsciiTag(tif, EXIFTAG_RELATEDSOUNDFILE,        &metaData, "Exif.Photo.RelatedSoundFile");
#endif
#if defined(EXIFTAG_FLASHENERGY)
    tiffSetExifDataTag(tif, EXIFTAG_FLASHENERGY,              &metaData, "Exif.Photo.FlashEnergy");
#endif
#if defined(EXIFTAG_SPATIALFREQUENCYRESPONSE)
    tiffSetExifDataTag(tif, EXIFTAG_SPATIALFREQUENCYRESPONSE, &metaData, "Exif.Photo.SpatialFrequencyResponse");
#endif
#if defined(EXIFTAG_FOCALPLANEXRESOLUTION)
    tiffSetExifDataTag(tif, EXIFTAG_FOCALPLANEXRESOLUTION,    &metaData, "Exif.Photo.FocalPlaneXResolution");
#endif
#if defined(EXIFTAG_FOCALPLANEYRESOLUTION)
    tiffSetExifDataTag(tif, EXIFTAG_FOCALPLANEYRESOLUTION,    &metaData, "Exif.Photo.FocalPlaneYResolution");
#endif
#if defined(EXIFTAG_FOCALPLANERESOLUTIONUNIT)
    tiffSetExifDataTag(tif, EXIFTAG_FOCALPLANERESOLUTIONUNIT, &metaData, "Exif.Photo.FocalPlaneResolutionUnit");
#endif
#if defined(EXIFTAG_SUBJECTLOCATION)
    tiffSetExifDataTag(tif, EXIFTAG_SUBJECTLOCATION,          &metaData, "Exif.Photo.SubjectLocation");
#endif
#if defined(EXIFTAG_EXPOSUREINDEX)
    tiffSetExifDataTag(tif, EXIFTAG_EXPOSUREINDEX,            &metaData, "Exif.Photo.ExposureIndex");
#endif
#if defined(EXIFTAG_SENSINGMETHOD)
    tiffSetExifDataTag(tif, EXIFTAG_SENSINGMETHOD,            &metaData, "Exif.Photo.SensingMethod");
#endif
#if defined(EXIFTAG_FILESOURCE)
    tiffSetExifDataTag(tif, EXIFTAG_FILESOURCE,               &metaData, "Exif.Photo.FileSource");
#endif
#if defined(EXIFTAG_SCENETYPE)
    tiffSetExifDataTag(tif, EXIFTAG_SCENETYPE,                &metaData, "Exif.Photo.SceneType");
#endif
#if defined(EXIFTAG_CFAPATTERN)
    tiffSetExifDataTag(tif, EXIFTAG_CFAPATTERN,               &metaData, "Exif.Photo.CFAPattern");
#endif
#if defined(EXIFTAG_CUSTOMRENDERED)
    tiffSetExifDataTag(tif, EXIFTAG_CUSTOMRENDERED,           &metaData, "Exif.Photo.CustomRendered");
#endif
#if defined(EXIFTAG_EXPOSUREMODE)
    tiffSetExifDataTag(tif, EXIFTAG_EXPOSUREMODE,             &metaData, "Exif.Photo.ExposureMode");
#endif
#if defined(EXIFTAG_WHITEBALANCE)
    tiffSetExifDataTag(tif, EXIFTAG_WHITEBALANCE,             &metaData, "Exif.Photo.WhiteBalance");
#endif
#if defined(EXIFTAG_DIGITALZOOMRATIO)
    tiffSetExifDataTag(tif, EXIFTAG_DIGITALZOOMRATIO,         &metaData, "Exif.Photo.DigitalZoomRatio");
#endif
#if defined(EXIFTAG_FOCALLENGTHIN35MMFILM)
    tiffSetExifDataTag(tif, EXIFTAG_FOCALLENGTHIN35MMFILM,    &metaData, "Exif.Photo.FocalLengthIn35mmFilm");
#endif
#if defined(EXIFTAG_SCENECAPTURETYPE)
    tiffSetExifDataTag(tif, EXIFTAG_SCENECAPTURETYPE,         &metaData, "Exif.Photo.SceneCaptureType");
#endif
#if defined(EXIFTAG_GAINCONTROL)
    tiffSetExifDataTag(tif, EXIFTAG_GAINCONTROL,              &metaData, "Exif.Photo.GainControl");
#endif
#if defined(EXIFTAG_CONTRAST)
    tiffSetExifDataTag(tif, EXIFTAG_CONTRAST,                 &metaData, "Exif.Photo.Contrast");
#endif
#if defined(EXIFTAG_SATURATION)
    tiffSetExifDataTag(tif, EXIFTAG_SATURATION,               &metaData, "Exif.Photo.Saturation");
#endif
#if defined(EXIFTAG_SHARPNESS)
    tiffSetExifDataTag(tif, EXIFTAG_SHARPNESS,                &metaData, "Exif.Photo.Sharpness");
#endif 
#if defined(EXIFTAG_DEVICESETTINGDESCRIPTION)
    tiffSetExifDataTag(tif, EXIFTAG_DEVICESETTINGDESCRIPTION, &metaData, "Exif.Photo.DeviceSettingDescription");
#endif 
#if defined(EXIFTAG_SUBJECTDISTANCERANGE)
    tiffSetExifDataTag(tif, EXIFTAG_SUBJECTDISTANCERANGE,     &metaData, "Exif.Photo.SubjectDistanceRange");
#endif 
#if defined(EXIFTAG_IMAGEUNIQUEID)
    tiffSetExifAsciiTag(tif, EXIFTAG_IMAGEUNIQUEID,           &metaData, "Exif.Photo.ImageUniqueID");
#endif

#if defined(TIFFTAG_EXIFIFD)
    TIFFSetField(tif, TIFFTAG_EXIFIFD, sub_offset);
#endif
*/
    
    // -------------------------------------------------------------------
    // Write ICC profil.
    
    QByteArray profile_rawdata = m_image->getICCProfil();
    
    if (!profile_rawdata.isEmpty())
    {
#if defined(TIFFTAG_ICCPROFILE)    
        TIFFSetField(tif, TIFFTAG_ICCPROFILE, (uint32)profile_rawdata.size(), (uchar *)profile_rawdata.data());
#endif      
    }    

    // -------------------------------------------------------------------
    // Write full image data in tiff directory IFD0

    if (observer)
        observer->progressInfo(m_image, 0.1);
    
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
        kdDebug() << k_funcinfo << "Cannot allocate memory buffer for main image." << endl;
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
            kdDebug() << k_funcinfo << "Cannot write main image to target file." << endl;
            _TIFFfree(buf);
            TIFFClose(tif);
            return false;
        }
    }

    _TIFFfree(buf);
    TIFFWriteDirectory(tif);

    // -------------------------------------------------------------------
    // Write thumbnail in tiff directory IFD1

    QImage thumb = m_image->smoothScale(160, 120, QSize::ScaleMin).copyQImage();

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH,      thumb.width());
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH,     thumb.height());
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC,     PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG,    PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_ORIENTATION,     ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT,  RESUNIT_NONE);
    TIFFSetField(tif, TIFFTAG_COMPRESSION,     COMPRESSION_NONE);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,   8);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,    TIFFDefaultStripSize(tif, 0));

    uchar *pixelThumb;
    uchar *dataThumb = thumb.bits();
    uint8 *bufThumb  = (uint8 *) _TIFFmalloc(TIFFScanlineSize(tif));

    if (!bufThumb)
    {
        kdDebug() << k_funcinfo << "Cannot allocate memory buffer for thumbnail." << endl;
        TIFFClose(tif);
        return false;
    }

    for (y = 0 ; y < uint32(thumb.height()) ; y++)
    {
        i = 0;
        
        for (x = 0 ; x < uint32(thumb.width()) ; x++)
        {
            pixelThumb = &dataThumb[((y * thumb.width()) + x) * 4];
            
            // This might be endian dependent 
            bufThumb[i++] = (uint8)pixelThumb[2];
            bufThumb[i++] = (uint8)pixelThumb[1];
            bufThumb[i++] = (uint8)pixelThumb[0];
        }

        if (!TIFFWriteScanline(tif, bufThumb, y, 0))
        {
            kdDebug() << k_funcinfo << "Cannot write thumbnail to target file." << endl;
            _TIFFfree(bufThumb);
            TIFFClose(tif);
            return false;
        }
    }

    _TIFFfree(bufThumb);
    TIFFClose(tif);

    // -------------------------------------------------------------------

    if (observer)
        observer->progressInfo(m_image, 1.0);

    imageSetAttribute("savedformat", "TIFF");
        
    // TODO : enable this line when Exiv2 will support TIFF in writting mode.
    //saveMetadata(filePath);
    
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

void TIFFLoader::tiffSetExifAsciiTag(TIFF* tif, ttag_t tiffTag, 
                                     DMetadata *metaData, const char* exifTagName)
{
    QByteArray tag = metaData->getExifTagData(exifTagName);
    if (!tag.isEmpty()) 
    {
        QCString str(tag.data(), tag.size());
        TIFFSetField(tif, tiffTag, (const char*)str);
    }
}

void TIFFLoader::tiffSetExifDataTag(TIFF* tif, ttag_t tiffTag, 
                                    DMetadata *metaData, const char* exifTagName)
{
    QByteArray tag = metaData->getExifTagData(exifTagName);
    if (!tag.isEmpty()) 
    {
        TIFFSetField (tif, tiffTag, (uint16)tag.size(), (char *)tag.data());
    }
}

}  // NameSpace Digikam
