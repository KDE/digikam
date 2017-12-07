/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-17
 * Description : A TIFF IO file for DImg framework
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Specifications & references:
 * - TIFF 6.0  : http://partners.adobe.com/public/developer/en/tiff/TIFF6.pdf
 * - TIFF/EP   : http://www.map.tu.chiba-u.ac.jp/IEC/100/TA2/recdoc/N4378.pdf
 * - TIFF/Tags : http://www.awaresystems.be/imaging/tiff/tifftags.html
 * - DNG       : http://www.adobe.com/products/dng/pdfs/dng_spec.pdf
 *
 * Others Linux Tiff Loader implementation using libtiff:
 * - http://websvn.kde.org/trunk/koffice/filters/krita/tiff/kis_tiff_converter.cc
 * - http://artis.inrialpes.fr/Software/TiffIO/
 * - http://cvs.graphicsmagick.org/cgi-bin/cvsweb.cgi/GraphicsMagick/coders/tiff.c
 * - http://freeimage.cvs.sourceforge.net/freeimage/FreeImage/Source/FreeImage/PluginTIFF.cpp
 * - http://freeimage.cvs.sourceforge.net/freeimage/FreeImage/Source/Metadata/XTIFF.cpp
 * - https://subversion.imagemagick.org/subversion/ImageMagick/trunk/coders/tiff.c
 *
 * Test images repository:
 * - http://www.remotesensing.org/libtiff/images.html
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

// C ANSI includes
extern "C"
{
#include <tiffvers.h>
}

// C++ includes

#include <cstdio>

// Qt includes

#include <QFile>
#include <QByteArray>

// Local includes

#include "digikam_config.h"
#include "dimg.h"
#include "digikam_debug.h"
#include "dimgloaderobserver.h"
#include "dmetadata.h"
#include "tiffloader.h"     //krazy:exclude=includes

namespace Digikam
{

// To manage Errors/Warnings handling provide by libtiff

void TIFFLoader::dimg_tiff_warning(const char* module, const char* format, va_list warnings)
{
    if(DIGIKAM_DIMG_LOG_TIFF().isDebugEnabled()) {
        char message[4096];
        vsnprintf(message, 4096, format, warnings);
        qCDebug(DIGIKAM_DIMG_LOG_TIFF) << module <<  "::" <<  message;
    }
}

void TIFFLoader::dimg_tiff_error(const char* module, const char* format, va_list errors)
{
    if(DIGIKAM_DIMG_LOG_TIFF().isDebugEnabled()) {
        char message[4096];
        vsnprintf(message, 4096, format, errors);
        qCDebug(DIGIKAM_DIMG_LOG_TIFF) << module << "::" << message;
    }
}

TIFFLoader::TIFFLoader(DImg* const image)
    : DImgLoader(image)
{
    m_hasAlpha   = false;
    m_sixteenBit = false;
}

bool TIFFLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    readMetadata(filePath, DImg::TIFF);

    // -------------------------------------------------------------------
    // TIFF error handling. If an errors/warnings occurs during reading,
    // libtiff will call these methods

    TIFFSetWarningHandler(dimg_tiff_warning);
    TIFFSetErrorHandler(dimg_tiff_error);

    // -------------------------------------------------------------------
    // Open the file

    TIFF* tif = TIFFOpen(QFile::encodeName(filePath).constData(), "r");

    if (!tif)
    {
        qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Cannot open image file.";
        loadingFailed();
        return false;
    }

    if(DIGIKAM_DIMG_LOG_TIFF().isDebugEnabled()) {
        TIFFPrintDirectory(tif, stdout, 0);
    }

    // -------------------------------------------------------------------
    // Get image information.

    uint32    w, h;
    uint16    bits_per_sample;
    uint16    samples_per_pixel;
    uint16    photometric;
    uint16    planar_config;
    uint32    rows_per_strip;
    tsize_t   strip_size;
    tstrip_t  num_of_strips;

    TIFFGetFieldDefaulted(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetFieldDefaulted(tif, TIFFTAG_IMAGELENGTH, &h);

    TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel);
    TIFFGetFieldDefaulted(tif, TIFFTAG_PLANARCONFIG, &planar_config);

    if (TIFFGetFieldDefaulted(tif, TIFFTAG_ROWSPERSTRIP, &rows_per_strip) == 0 || rows_per_strip == 0)
    {
        qCWarning(DIGIKAM_DIMG_LOG_TIFF)  << "TIFF loader: Cannot handle non-stripped images. Loading file "
                   << filePath;
        TIFFClose(tif);
        loadingFailed();
        return false;
    }

    if (rows_per_strip > h)
    {
        rows_per_strip = h;
    }

    if (bits_per_sample   == 0
        || samples_per_pixel == 0
        || rows_per_strip    == 0
        //        || rows_per_strip    >  h
       )
    {
        qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "TIFF loader: Encountered invalid value in image." << endl
                   << " bits_per_sample   : " << bits_per_sample  << endl
                   << " samples_per_pixel : " << samples_per_pixel << endl
                   << " rows_per_strip    : " << rows_per_strip << endl
                   << " h                 : " << h << endl
                   << " Loading file      : " << filePath;
        TIFFClose(tif);
        loadingFailed();
        return false;
    }

    // TODO: check others TIFF color-spaces here. Actually, only RGB, PALETTE and MINISBLACK
    // have been tested.
    // Complete description of TIFFTAG_PHOTOMETRIC tag can be found at this url:
    // http://www.awaresystems.be/imaging/tiff/tifftags/photometricinterpretation.html

    TIFFGetFieldDefaulted(tif, TIFFTAG_PHOTOMETRIC, &photometric);

    if (photometric != PHOTOMETRIC_RGB &&
        photometric != PHOTOMETRIC_PALETTE &&
        photometric != PHOTOMETRIC_MINISWHITE &&
        photometric != PHOTOMETRIC_MINISBLACK &&
        ((photometric != PHOTOMETRIC_YCBCR) | (bits_per_sample != 8)) &&
        ((photometric != PHOTOMETRIC_SEPARATED) | (bits_per_sample != 8)) &&
        (m_loadFlags & LoadImageData))
    {
        qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Can not handle image without RGB color-space: "
                   << photometric;
        TIFFClose(tif);
        loadingFailed();
        return false;
    }

    int colorModel = DImg::COLORMODELUNKNOWN;

    switch (photometric)
    {
        case PHOTOMETRIC_MINISWHITE:
        case PHOTOMETRIC_MINISBLACK:
            colorModel = DImg::GRAYSCALE;
            break;

        case PHOTOMETRIC_RGB:
            colorModel = DImg::RGB;
            break;

        case PHOTOMETRIC_PALETTE:
            colorModel = DImg::INDEXED;
            break;

        case PHOTOMETRIC_MASK:
            colorModel = DImg::MONOCHROME;
            break;

        case PHOTOMETRIC_SEPARATED:
            colorModel = DImg::CMYK;
            break;

        case PHOTOMETRIC_YCBCR:
            colorModel = DImg::YCBCR;
            break;

        case PHOTOMETRIC_CIELAB:
        case PHOTOMETRIC_ICCLAB:
        case PHOTOMETRIC_ITULAB:
            colorModel = DImg::CIELAB;
            break;

        case PHOTOMETRIC_LOGL:
        case PHOTOMETRIC_LOGLUV:
            colorModel = DImg::COLORMODELRAW;
            break;
    }

    if (samples_per_pixel == 4)
    {
        m_hasAlpha = true;
    }
    else
    {
        m_hasAlpha = false;
    }

    if (bits_per_sample == 16 || bits_per_sample == 32)
    {
        m_sixteenBit = true;
    }
    else
    {
        m_sixteenBit = false;
    }

    // -------------------------------------------------------------------
    // Read image ICC profile

    if (m_loadFlags & LoadICCData)
    {
        uchar*  profile_data = 0;
        uint32  profile_size;

        if (TIFFGetField(tif, TIFFTAG_ICCPROFILE, &profile_size, &profile_data))
        {
            QByteArray profile_rawdata;
            profile_rawdata.resize(profile_size);
            memcpy(profile_rawdata.data(), profile_data, profile_size);
            imageSetIccProfile(profile_rawdata);
        }
        else
        {
            // If ICC profile is null, check Exif metadata.
            checkExifWorkingColorSpace();
        }
    }

    // -------------------------------------------------------------------
    // Get image data.

    QScopedArrayPointer<uchar> data;

    if (m_loadFlags & LoadImageData)
    {
        if (observer)
        {
            observer->progressInfo(m_image, 0.1F);
        }

        strip_size    = TIFFStripSize(tif);
        num_of_strips = TIFFNumberOfStrips(tif);

        if (bits_per_sample == 16)          // 16 bits image.
        {
            data.reset(new_failureTolerant(w, h, 8));
            QScopedArrayPointer<uchar> strip(new_failureTolerant(strip_size));

            if (!data || strip.isNull())
            {
                qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to allocate memory for TIFF image" << filePath;
                TIFFClose(tif);
                loadingFailed();
                return false;
            }

            long offset    = 0;
            long bytesRead = 0;

            uint checkpoint = 0;

            for (tstrip_t st = 0; st < num_of_strips; ++st)
            {
                if (observer && st == checkpoint)
                {
                    checkpoint += granularity(observer, num_of_strips, 0.8F);

                    if (!observer->continueQuery(m_image))
                    {
                        TIFFClose(tif);
                        loadingFailed();
                        return false;
                    }

                    observer->progressInfo(m_image, 0.1 + (0.8 * (((float)st) / ((float)num_of_strips))));
                }

                bytesRead = TIFFReadEncodedStrip(tif, st, strip.data(), strip_size);

                if (bytesRead == -1)
                {
                    qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to read strip";
                    TIFFClose(tif);
                    loadingFailed();
                    return false;
                }

                if ((planar_config == PLANARCONFIG_SEPARATE) &&
                    (st % (num_of_strips / samples_per_pixel)) == 0)
                {
                    offset = 0;
                }

                ushort* stripPtr = reinterpret_cast<ushort*>(strip.data());
                ushort* dataPtr  = reinterpret_cast<ushort*>(data.data() + offset);
                ushort* p;

                // tiff data is read as BGR or ABGR or Greyscale

                if (samples_per_pixel == 1)   // See bug #148400: Greyscale pictures only have _one_ sample per pixel
                {
                    for (int i = 0; i < bytesRead / 2; ++i)
                    {
                        // We have to read two bytes for one pixel
                        p = dataPtr;

                        p[0] = *stripPtr;      // RGB have to be set to the _same_ value
                        p[1] = *stripPtr;
                        p[2] = *stripPtr++;
                        p[3] = 0xFFFF;         // set alpha to 100%

                        dataPtr += 4;
                    }

                    offset += bytesRead * 4;       // The _byte_offset in the data array is, of course, four times bytesRead
                }
                else if ((samples_per_pixel == 3) && (planar_config == PLANARCONFIG_CONTIG))
                {
                    for (int i = 0; i < bytesRead / 6; ++i)
                    {
                        p = dataPtr;

                        p[2] = *stripPtr++;
                        p[1] = *stripPtr++;
                        p[0] = *stripPtr++;
                        p[3] = 0xFFFF;

                        dataPtr += 4;
                    }

                    offset += bytesRead / 6 * 8;
                }
                else if ((samples_per_pixel == 3) && (planar_config == PLANARCONFIG_SEPARATE))
                {
                    for (int i = 0; i < bytesRead / 2; ++i)
                    {
                        p = dataPtr;

                        switch ((st / (num_of_strips / samples_per_pixel)))
                        {
                            case 0:
                                p[2] = *stripPtr++;
                                p[3] = 0xFFFF;
                                break;

                            case 1:
                                p[1] = *stripPtr++;
                                break;

                            case 2:
                                p[0] = *stripPtr++;
                                break;
                        }

                        dataPtr += 4;
                    }

                    offset += bytesRead / 2 * 8;
                }
                else if ((samples_per_pixel == 4) && (planar_config == PLANARCONFIG_CONTIG))
                {
                    for (int i = 0; i < bytesRead / 8; ++i)
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
                else if ((samples_per_pixel == 4) && (planar_config == PLANARCONFIG_SEPARATE))
                {
                    for (int i = 0; i < bytesRead / 2; ++i)
                    {
                        p = dataPtr;

                        switch ((st / (num_of_strips / samples_per_pixel)))
                        {
                            case 0:
                                p[2] = *stripPtr++;
                                break;

                            case 1:
                                p[1] = *stripPtr++;
                                break;

                            case 2:
                                p[0] = *stripPtr++;
                                break;

                            case 3:
                                p[3] = *stripPtr++;
                                break;
                        }

                        dataPtr += 4;
                    }

                    offset += bytesRead / 2 * 8;
                }
            }
        }
        else if (bits_per_sample == 32)          // 32 bits image.
        {
            data.reset(new_failureTolerant(w, h, 8));
            QScopedArrayPointer<uchar> strip(new_failureTolerant(strip_size));

            if (!data || strip.isNull())
            {
                qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to allocate memory for TIFF image" << filePath;
                TIFFClose(tif);
                loadingFailed();
                return false;
            }

            long  offset     = 0;
            long  bytesRead  = 0;

            uint  checkpoint = 0;
            float maxValue   = 0.0;

            for (tstrip_t st = 0; st < num_of_strips; ++st)
            {
                if (observer && !observer->continueQuery(m_image))
                {
                    TIFFClose(tif);
                    loadingFailed();
                    return false;
                }

                bytesRead = TIFFReadEncodedStrip(tif, st, strip.data(), strip_size);

                if (bytesRead == -1)
                {
                    qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to read strip";
                    TIFFClose(tif);
                    loadingFailed();
                    return false;
                }

                float* stripPtr = reinterpret_cast<float*>(strip.data());

                for (int i = 0; i < bytesRead / 4; ++i)
                {
                    maxValue = qMax(maxValue, *stripPtr++);
                }
            }

            double factor = (maxValue > 10.0) ? log10(maxValue) * 1.5 : 1.0;
            double scale  = (factor > 1.0) ? 0.75 : 1.0;

            if (factor > 1.0)
            {
                qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "TIFF image cannot be converted lossless from 32 to 16 bits" << filePath;
            }

            for (tstrip_t st = 0; st < num_of_strips; ++st)
            {
                if (observer && st == checkpoint)
                {
                    checkpoint += granularity(observer, num_of_strips, 0.8F);

                    if (!observer->continueQuery(m_image))
                    {
                        TIFFClose(tif);
                        loadingFailed();
                        return false;
                    }

                    observer->progressInfo(m_image, 0.1 + (0.8 * (((float)st) / ((float)num_of_strips))));
                }

                bytesRead = TIFFReadEncodedStrip(tif, st, strip.data(), strip_size);

                if (bytesRead == -1)
                {
                    qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to read strip";
                    TIFFClose(tif);
                    loadingFailed();
                    return false;
                }

                if ((planar_config == PLANARCONFIG_SEPARATE) &&
                    (st % (num_of_strips / samples_per_pixel)) == 0)
                {
                    offset = 0;
                }

                float*  stripPtr = reinterpret_cast<float*>(strip.data());
                ushort* dataPtr  = reinterpret_cast<ushort*>(data.data() + offset);
                ushort* p;

                if ((samples_per_pixel == 3) && (planar_config == PLANARCONFIG_CONTIG))
                {
                    for (int i = 0; i < bytesRead / 12; ++i)
                    {
                        p = dataPtr;

                        p[2] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                        p[1] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                        p[0] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                        p[3] = 0xFFFF;

                        dataPtr += 4;
                    }

                    offset += bytesRead / 12 * 8;
                }
                else if ((samples_per_pixel == 3) && (planar_config == PLANARCONFIG_SEPARATE))
                {
                    for (int i = 0; i < bytesRead / 4; ++i)
                    {
                        p = dataPtr;

                        switch ((st / (num_of_strips / samples_per_pixel)))
                        {
                            case 0:
                                p[2] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                                p[3] = 0xFFFF;
                                break;

                            case 1:
                                p[1] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                                break;

                            case 2:
                                p[0] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                                break;
                        }

                        dataPtr += 4;
                    }

                    offset += bytesRead / 4 * 8;
                }
                else if ((samples_per_pixel == 4) && (planar_config == PLANARCONFIG_CONTIG))
                {
                    for (int i = 0; i < bytesRead / 16; ++i)
                    {
                        p = dataPtr;

                        p[2] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                        p[1] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                        p[0] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                        p[3] = (ushort)qBound(0.0, (double)*stripPtr++ * 65535.0, 65535.0);

                        dataPtr += 4;
                    }

                    offset += bytesRead / 16 * 8;
                }
                else if ((samples_per_pixel == 4) && (planar_config == PLANARCONFIG_SEPARATE))
                {
                    for (int i = 0; i < bytesRead / 4; ++i)
                    {
                        p = dataPtr;

                        switch ((st / (num_of_strips / samples_per_pixel)))
                        {
                            case 0:
                                p[2] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                                break;

                            case 1:
                                p[1] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                                break;

                            case 2:
                                p[0] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                                break;

                            case 3:
                                p[3] = (ushort)qBound(0.0, (double)*stripPtr++ * 65535.0, 65535.0);
                                break;
                        }

                        dataPtr += 4;
                    }

                    offset += bytesRead / 4 * 8;
                }
            }
        }
        else       // Non 16 or 32 bits images ==> get it on BGRA 8 bits.
        {
            data.reset(new_failureTolerant(w, h, 4));
            QScopedArrayPointer<uchar> strip(new_failureTolerant(w, rows_per_strip, 4));

            if (!data || strip.isNull())
            {
                qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to allocate memory for TIFF image" << filePath;
                TIFFClose(tif);
                loadingFailed();
                return false;
            }

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
                qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to set up RGBA reading of image, filename "
                         << TIFFFileName(tif) <<  " error message from Libtiff: " << emsg;
                TIFFClose(tif);
                loadingFailed();
                return false;
            }

            // libtiff cannot handle all possible orientations, it give weird results.
            // We rotate ourselves. (Bug 274865)
            img.req_orientation = img.orientation;

            // read strips from image: read rows_per_strip, so always start at beginning of a strip
            for (uint row = 0; row < h; row += rows_per_strip)
            {
                if (observer && row >= checkpoint)
                {
                    checkpoint += granularity(observer, h, 0.8F);

                    if (!observer->continueQuery(m_image))
                    {
                        TIFFClose(tif);
                        loadingFailed();
                        return false;
                    }

                    observer->progressInfo(m_image, 0.1 + (0.8 * (((float)row) / ((float)h))));
                }

                img.row_offset  = row;
                img.col_offset  = 0;

                if (row + rows_per_strip > img.height)
                {
                    rows_to_read = img.height - row;
                }
                else
                {
                    rows_to_read = rows_per_strip;
                }

                // Read data

                if (TIFFRGBAImageGet(&img, reinterpret_cast<uint32*>(strip.data()), img.width, rows_to_read) == -1)
                {
                    qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to read image data";
                    TIFFClose(tif);
                    loadingFailed();
                    return false;
                }

                pixelsRead = rows_to_read * img.width;

                uchar* stripPtr = (uchar*)(strip.data());
                uchar* dataPtr  = (uchar*)(data.data() + offset);
                uchar* p;

                // Reverse red and blue

                for (int i = 0; i < pixelsRead; ++i)
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
        }
    }

    // -------------------------------------------------------------------

    TIFFClose(tif);

    if (observer)
    {
        observer->progressInfo(m_image, 1.0);
    }

    imageWidth()  = w;
    imageHeight() = h;
    imageData()   = data.take();
    imageSetAttribute(QLatin1String("format"),             QLatin1String("TIFF"));
    imageSetAttribute(QLatin1String("originalColorModel"), colorModel);
    imageSetAttribute(QLatin1String("originalBitDepth"),   bits_per_sample);
    imageSetAttribute(QLatin1String("originalSize"),       QSize(w, h));

    return true;
}

bool TIFFLoader::save(const QString& filePath, DImgLoaderObserver* const observer)
{
    uint32 w     = imageWidth();
    uint32 h     = imageHeight();
    uchar*  data = imageData();

    // -------------------------------------------------------------------
    // TIFF error handling. If an errors/warnings occurs during reading,
    // libtiff will call these methods

    TIFFSetWarningHandler(dimg_tiff_warning);
    TIFFSetErrorHandler(dimg_tiff_error);

    // -------------------------------------------------------------------
    // Open the file

    TIFF* tif = TIFFOpen(QFile::encodeName(filePath).constData(), "w");

    if (!tif)
    {
        qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Cannot open target image file.";
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
    QVariant compressAttr = imageGetAttribute(QLatin1String("compress"));
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
    {
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    }

    uint16 sampleinfo[1];

    if (imageHasAlpha())
    {
        sampleinfo[0] = EXTRASAMPLE_ASSOCALPHA;
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
        TIFFSetField(tif, TIFFTAG_EXTRASAMPLES,    1, sampleinfo);
    }
    else
    {
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
    }

    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, (uint16)imageBitsDepth());
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,  TIFFDefaultStripSize(tif, 0));

    // -------------------------------------------------------------------
    // Write meta-data Tags contents.

    DMetadata metaData(m_image->getMetadata());

    // Standard IPTC tag (available with libtiff 3.6.1)

    QByteArray ba = metaData.getIptc(true);

    if (!ba.isEmpty())
    {
#if defined(TIFFTAG_PHOTOSHOP)
        TIFFSetField(tif, TIFFTAG_PHOTOSHOP, (uint32)ba.size(), (uchar*)ba.data());
#endif
    }

    // Standard XMP tag (available with libtiff 3.6.1)

    if (metaData.hasXmp())
    {
#if defined(TIFFTAG_XMLPACKET)
        tiffSetExifDataTag(tif, TIFFTAG_XMLPACKET,            metaData, "Exif.Image.XMLPacket");
#endif
    }

    // Standard Exif ASCII tags (available with libtiff 3.6.1)

    tiffSetExifAsciiTag(tif, TIFFTAG_DOCUMENTNAME,            metaData, "Exif.Image.DocumentName");
    tiffSetExifAsciiTag(tif, TIFFTAG_IMAGEDESCRIPTION,        metaData, "Exif.Image.ImageDescription");
    tiffSetExifAsciiTag(tif, TIFFTAG_MAKE,                    metaData, "Exif.Image.Make");
    tiffSetExifAsciiTag(tif, TIFFTAG_MODEL,                   metaData, "Exif.Image.Model");
    tiffSetExifAsciiTag(tif, TIFFTAG_DATETIME,                metaData, "Exif.Image.DateTime");
    tiffSetExifAsciiTag(tif, TIFFTAG_ARTIST,                  metaData, "Exif.Image.Artist");
    tiffSetExifAsciiTag(tif, TIFFTAG_COPYRIGHT,               metaData, "Exif.Image.Copyright");

    QString soft = metaData.getExifTagString("Exif.Image.Software");
    QString libtiffver = QLatin1String(TIFFLIB_VERSION_STR);
    libtiffver.replace(QLatin1Char('\n'), QLatin1Char(' '));
    soft.append(QString::fromLatin1(" ( %1 )").arg(libtiffver));
    TIFFSetField(tif, TIFFTAG_SOFTWARE, (const char*)soft.toLatin1().constData());

    // NOTE: All others Exif tags will be written by Exiv2 (<= 0.18)

    // -------------------------------------------------------------------
    // Write ICC profile.

    QByteArray profile_rawdata = m_image->getIccProfile().data();

    if (!profile_rawdata.isEmpty())
    {
#if defined(TIFFTAG_ICCPROFILE)
        purgeExifWorkingColorSpace();
        TIFFSetField(tif, TIFFTAG_ICCPROFILE, (uint32)profile_rawdata.size(), (uchar*)profile_rawdata.data());
#endif
    }

    // -------------------------------------------------------------------
    // Write full image data in tiff directory IFD0

    if (observer)
    {
        observer->progressInfo(m_image, 0.1F);
    }

    uchar*  pixel;
    uint16* pixel16;
    double  alpha_factor;
    uint32  x, y;
    uint8   r8, g8, b8, a8 = 0;
    uint16  r16, g16, b16, a16 = 0;
    int     i = 0;

    uint8* buf = (uint8*)_TIFFmalloc(TIFFScanlineSize(tif));
    uint16* buf16;

    if (!buf)
    {
        qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Cannot allocate memory buffer for main image.";
        TIFFClose(tif);
        return false;
    }

    uint checkpoint = 0;

    for (y = 0; y < h; ++y)
    {

        if (observer && y == checkpoint)
        {
            checkpoint += granularity(observer, h, 0.8F);

            if (!observer->continueQuery(m_image))
            {
                _TIFFfree(buf);
                TIFFClose(tif);
                return false;
            }

            observer->progressInfo(m_image, 0.1 + (0.8 * (((float)y) / ((float)h))));
        }

        i = 0;

        for (x = 0; x < w; ++x)
        {
            pixel = &data[((y * w) + x) * imageBytesDepth()];

            if (imageSixteenBit())          // 16 bits image.
            {
                pixel16 = reinterpret_cast<ushort*>(pixel);
                b16 = pixel16[0];
                g16 = pixel16[1];
                r16 = pixel16[2];

                if (imageHasAlpha())
                {
                    // TIFF makes you pre-multiply the RGB components by alpha

                    a16          = pixel16[3];
                    alpha_factor = ((double)a16 / 65535.0);
                    r16          = (uint16)(r16 * alpha_factor);
                    g16          = (uint16)(g16 * alpha_factor);
                    b16          = (uint16)(b16 * alpha_factor);
                }

                // This might be endian dependent

                buf16    = reinterpret_cast<ushort*>(buf+i);
                *buf16++ = r16;
                *buf16++ = g16;
                *buf16++ = b16;
                i+= 6;

                if (imageHasAlpha())
                {
                    *buf16++ = a16;
                    i += 2;
                }
            }
            else                            // 8 bits image.
            {
                b8 = (uint8)pixel[0];
                g8 = (uint8)pixel[1];
                r8 = (uint8)pixel[2];

                if (imageHasAlpha())
                {
                    // TIFF makes you pre-multiply the RGB components by alpha

                    a8           = (uint8)(pixel[3]);
                    alpha_factor = ((double)a8 / 255.0);
                    r8           = (uint8)(r8 * alpha_factor);
                    g8           = (uint8)(g8 * alpha_factor);
                    b8           = (uint8)(b8 * alpha_factor);
                }

                // This might be endian dependent

                buf[i++] = r8;
                buf[i++] = g8;
                buf[i++] = b8;

                if (imageHasAlpha())
                {
                    buf[i++] = a8;
                }
            }
        }

        if (!TIFFWriteScanline(tif, buf, y, 0))
        {
            qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Cannot write main image to target file.";
            _TIFFfree(buf);
            TIFFClose(tif);
            return false;
        }
    }

    _TIFFfree(buf);
    TIFFWriteDirectory(tif);

    // -------------------------------------------------------------------
    // Write thumbnail in tiff directory IFD1

    QImage thumb = m_image->smoothScale(160, 120, Qt::KeepAspectRatio).copyQImage();

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, (uint32)thumb.width());
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, (uint32)thumb.height());
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC,     PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG,    PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_ORIENTATION,     ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT,  RESUNIT_NONE);
    TIFFSetField(tif, TIFFTAG_COMPRESSION,     COMPRESSION_NONE);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,   8);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,    TIFFDefaultStripSize(tif, 0));

    uchar* pixelThumb;
    uchar* dataThumb = thumb.bits();
    uint8* bufThumb  = (uint8*) _TIFFmalloc(TIFFScanlineSize(tif));

    if (!bufThumb)
    {
        qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Cannot allocate memory buffer for thumbnail.";
        TIFFClose(tif);
        return false;
    }

    for (y = 0 ; y < uint32(thumb.height()) ; ++y)
    {
        i = 0;

        for (x = 0 ; x < uint32(thumb.width()) ; ++x)
        {
            pixelThumb = &dataThumb[((y * thumb.width()) + x) * 4];

            // This might be endian dependent
            bufThumb[i++] = (uint8)pixelThumb[2];
            bufThumb[i++] = (uint8)pixelThumb[1];
            bufThumb[i++] = (uint8)pixelThumb[0];
        }

        if (!TIFFWriteScanline(tif, bufThumb, y, 0))
        {
            qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Cannot write thumbnail to target file.";
            _TIFFfree(bufThumb);
            TIFFClose(tif);
            return false;
        }
    }

    _TIFFfree(bufThumb);
    TIFFClose(tif);

    // -------------------------------------------------------------------

    if (observer)
    {
        observer->progressInfo(m_image, 1.0);
    }

    imageSetAttribute(QLatin1String("savedformat"), QLatin1String("TIFF"));

    // Save metadata

    DMetadata metaDataToFile(filePath);
    metaDataToFile.setData(m_image->getMetadata());
    // see bug #211758 for these special steps needed when writing a TIFF
    metaDataToFile.removeExifThumbnail();
    metaDataToFile.removeExifTag("Exif.Image.ProcessingSoftware");
    metaDataToFile.applyChanges();

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

void TIFFLoader::tiffSetExifAsciiTag(TIFF* const tif, ttag_t tiffTag,
                                     const DMetadata& metaData, const char* const exifTagName)
{
    QByteArray tag = metaData.getExifTagData(exifTagName);

    if (!tag.isEmpty())
    {
        QByteArray str(tag.data(), tag.size());
        TIFFSetField(tif, tiffTag, (const char*)str.constData());
    }
}

void TIFFLoader::tiffSetExifDataTag(TIFF* const tif, ttag_t tiffTag,
                                    const DMetadata& metaData, const char* const exifTagName)
{
    QByteArray tag = metaData.getExifTagData(exifTagName);

    if (!tag.isEmpty())
    {
        TIFFSetField(tif, tiffTag, (uint32)tag.size(), (char*)tag.data());
    }
}

bool TIFFLoader::isReadOnly() const
{
    return false;
}

}  // namespace Digikam
