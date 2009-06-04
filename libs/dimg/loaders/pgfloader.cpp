/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-03
 * Description : A PGF IO file for DImg framework
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This implementation use LibPGF API <http://www.libpgf.org>
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

// C Ansi includes

extern "C"
{
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
}

// C++ includes

#include <iostream>
#include <cmath>
#include <cstdio>

// Qt includes

#include <QFile>
#include <QByteArray>
#include <QTextStream>

// KDE includes

#include <kdebug.h>

// Windows includes

#ifdef WIN32
#include <windows.h>
#endif

// Local includes

#include "PGFimage.h"
#include "dimg.h"
#include "dimgloaderobserver.h"
#include "pgfloader.h"

namespace Digikam
{

static bool CallbackForLibPGF(double percent, bool escapeAllowed, void *data)
{
    if (data)
    {
        PGFLoader *d = static_cast<PGFLoader*>(data);
        if (d) return d->progressCallback(percent, escapeAllowed);
    }
    return false;
}

PGFLoader::PGFLoader(DImg* image)
         : DImgLoader(image)
{
    m_hasAlpha   = false;
    m_sixteenBit = false;
    m_observer   = 0;
}

bool PGFLoader::load(const QString& filePath, DImgLoaderObserver *observer)
{
    m_observer = observer;
    readMetadata(filePath, DImg::PGF);

    FILE *file = fopen(QFile::encodeName(filePath), "rb");
    if (!file)
    {
        kDebug(50003) << "Error: Could not open source file." << endl;
        return false;
    }

    unsigned char header[3];

    if (fread(&header, 3, 1, file) != 1)
    {
        fclose(file);
        return false;
    }

    unsigned char pgfID[3] = { 0x50, 0x47, 0x46 };

    if (memcmp(&header[0], &pgfID, 3) != 0)
    {
        // not a PGF file
        fclose(file);
        return false;
    }

    fclose(file);

    // -------------------------------------------------------------------
    // Initialize PGF API.

#ifdef WIN32
    HANDLE fd = CreateFile(QFile::encodeName(filePath), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
    if (fd == INVALID_HANDLE_VALUE)
        return false;
#else
    int fd = open(QFile::encodeName(filePath), O_RDONLY);
    if (fd == -1)
        return false;
#endif

    // Create stream object and image
    CPGFFileStream stream(fd);
    CPGFImage      pgf;
    int            colorModel = DImg::COLORMODELUNKNOWN;

    try
    {
        // open pgf image
        pgf.Open(&stream);

        switch (pgf.Mode())
        {
            case ImageModeRGBColor:
            case ImageModeRGB48:
            case ImageModeRGBA:
                colorModel = DImg::RGB;
                break;
            default:
                kDebug(50003) << "Cannot load PGF image: color mode not supported..." << endl;
                return false;
                break;
        }

        switch (pgf.Channels())
        {
            case 3:
                m_hasAlpha = false;
            case 4:
                m_hasAlpha = true;
                break;
            default:
                kDebug(50003) << "Cannot load PGF image: color channels number not supported..." << endl;
                return false;
                break;
        }

        int bitDepth = pgf.BPP();

        switch (bitDepth)
        {
            case 24:    // RGB 8 bits.
            case 32:    // RGBA 8 bits.
                m_sixteenBit = false;
                break;
            case 48:    // RGB 16 bits.
            case 64:    // RGBA 16 bits.
                m_sixteenBit = true;
                break;
            default:
                kDebug(50003) << "Cannot load PGF image: color bits depth not supported..." << endl;
                return false;
                break;
        }

        int width        = pgf.Width();
        int height       = pgf.Height();;
        uchar *data      = 0;
        int channelMap[] = { 0, 1, 2, 3 };

        if (m_sixteenBit)
            data = new uchar[width*height*8];  // 16 bits/color/pixel
        else
            data = new uchar[width*height*4];  // 8 bits/color/pixel

        pgf.Read(0, CallbackForLibPGF, this);
        pgf.GetBitmap(m_sixteenBit ? width*8 : width*4, (UINT8*)data, m_sixteenBit ? 64 : 32, channelMap, CallbackForLibPGF, this);

        if (observer)
            observer->progressInfo(m_image, 1.0);

        imageWidth()  = width;
        imageHeight() = height;
        imageData()   = data;
        imageSetAttribute("format", "PGF");
        imageSetAttribute("originalColorModel", colorModel);
        imageSetAttribute("originalBitDepth", bitDepth);

        return true;
    }
    catch(IOException& e)
    {
        int err = e.error;
        if (err >= AppError) err -= AppError;
        kDebug(50003) << "Error: Opening and reading PGF image failed (" << err << ")!" << endl;

#ifdef WIN32
        CloseHandle(fd);
#else
        close(fd);
#endif

        return false;
    }
}

bool PGFLoader::save(const QString& filePath, DImgLoaderObserver *observer)
{
    m_observer = observer;

#ifdef WIN32
    HANDLE fd = CreateFile(QFile::encodeName(filePath), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (fd == INVALID_HANDLE_VALUE)
    {
        kDebug(50003) << "Error: Could not open destination file." << endl;
        return false;
    }

#elif defined(__POSIX__)
    int fd = open(QFile::encodeName(filePath), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1)
    {
        kDebug(50003) << "Error: Could not open destination file." << endl;
        return false;
    }
#endif

    // create stream object
    CPGFFileStream stream(fd);

    CPGFImage pgf;
/*
    // -------------------------------------------------------------------
    // Initialize JPEG 2000 API.

    register long  i, x, y;
    unsigned long  number_components;

    jas_image_t          *jp2_image   = 0;
    jas_stream_t         *jp2_stream  = 0;
    jas_matrix_t         *pixels[4];
    jas_image_cmptparm_t  component_info[4];

    int init = jas_init();
    if (init != 0)
    {
        kDebug(50003) << "Unable to init JPEG2000 decoder" << endl;
        return false;
    }

    jp2_stream = jas_stream_fopen(QFile::encodeName(filePath), "wb");
    if (jp2_stream == 0)
    {
        kDebug(50003) << "Unable to open JPEG2000 stream" << endl;
        return false;
    }

    number_components = imageHasAlpha() ? 4 : 3;

    for (i = 0 ; i < (long)number_components ; ++i)
    {
        component_info[i].tlx    = 0;
        component_info[i].tly    = 0;
        component_info[i].hstep  = 1;
        component_info[i].vstep  = 1;
        component_info[i].width  = imageWidth();
        component_info[i].height = imageHeight();
        component_info[i].prec   = imageBitsDepth();
        component_info[i].sgnd   = false;
    }

    jp2_image = jas_image_create(number_components, component_info, JAS_CLRSPC_UNKNOWN);
    if (jp2_image == 0)
    {
        jas_stream_close(jp2_stream);
        kDebug(50003) << "Unable to create JPEG2000 image" << endl;
        return false;
    }

    if (observer)
        observer->progressInfo(m_image, 0.1F);

    // -------------------------------------------------------------------
    // Check color space.

    if (number_components >= 3 )    // RGB & RGBA
    {
        // Alpha Channel
        if (number_components == 4 )
            jas_image_setcmpttype(jp2_image, 3, JAS_IMAGE_CT_OPACITY);

        jas_image_setclrspc(jp2_image, JAS_CLRSPC_SRGB);
        jas_image_setcmpttype(jp2_image, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
        jas_image_setcmpttype(jp2_image, 1, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
        jas_image_setcmpttype(jp2_image, 2, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));
    }

    // -------------------------------------------------------------------
    // Set ICC color profile.

    // FIXME : doesn't work yet!

    jas_cmprof_t  *cm_profile  = 0;
    jas_iccprof_t *icc_profile = 0;

    QByteArray profile_rawdata = m_image->getICCProfil();

    icc_profile = jas_iccprof_createfrombuf((uchar*)profile_rawdata.data(), profile_rawdata.size());
    if (icc_profile != 0)
    {
        cm_profile = jas_cmprof_createfromiccprof(icc_profile);
        if (cm_profile != 0)
        {
            jas_image_setcmprof(jp2_image, cm_profile);
        }
    }

    // -------------------------------------------------------------------
    // Convert to JPEG 2000 pixels.

    for (i = 0 ; i < (long)number_components ; ++i)
    {
        pixels[i] = jas_matrix_create(1, (unsigned int)imageWidth());
        if (pixels[i] == 0)
        {
            for (x = 0 ; x < i ; ++x)
            jas_matrix_destroy(pixels[x]);

            jas_image_destroy(jp2_image);
            kDebug(50003) << "Error encoding JPEG2000 image data : Memory Allocation Failed" << endl;
            return false;
        }
    }

    unsigned char* data = imageData();
    unsigned char* pixel;
    unsigned short r, g, b, a=0;
    uint           checkpoint = 0;

    for (y = 0 ; y < (long)imageHeight() ; ++y)
    {
        if (observer && y == (long)checkpoint)
        {
            checkpoint += granularity(observer, imageHeight(), 0.8F);
            if (!observer->continueQuery(m_image))
            {
                jas_image_destroy(jp2_image);
                for (i = 0 ; i < (long)number_components ; ++i)
                    jas_matrix_destroy(pixels[i]);

                jas_cleanup();

                return false;
            }
            observer->progressInfo(m_image, 0.1 + (0.8 * ( ((float)y)/((float)imageHeight()) )));
        }

        for (x = 0 ; x < (long)imageWidth() ; ++x)
        {
            pixel = &data[((y * imageWidth()) + x) * imageBytesDepth()];

            if ( imageSixteenBit() )        // 16 bits image.
            {
                b = (unsigned short)(pixel[0]+256*pixel[1]);
                g = (unsigned short)(pixel[2]+256*pixel[3]);
                r = (unsigned short)(pixel[4]+256*pixel[5]);

                if (imageHasAlpha())
                    a = (unsigned short)(pixel[6]+256*pixel[7]);
            }
            else                            // 8 bits image.
            {
                b = (unsigned short)pixel[0];
                g = (unsigned short)pixel[1];
                r = (unsigned short)pixel[2];

                if (imageHasAlpha())
                    a = (unsigned short)(pixel[3]);
            }

            jas_matrix_setv(pixels[0], x, r);
            jas_matrix_setv(pixels[1], x, g);
            jas_matrix_setv(pixels[2], x, b);

            if (number_components > 3)
                jas_matrix_setv(pixels[3], x, a);
        }

        for (i = 0 ; i < (long)number_components ; ++i)
        {
            int ret = jas_image_writecmpt(jp2_image, (short) i, 0, (unsigned int)y,
                                          (unsigned int)imageWidth(), 1, pixels[i]);
            if (ret != 0)
            {
                kDebug(50003) << "Error encoding JPEG2000 image data" << endl;

                jas_image_destroy(jp2_image);
                for (i = 0 ; i < (long)number_components ; ++i)
                    jas_matrix_destroy(pixels[i]);

                jas_cleanup();
                return false;
            }
        }
    }

    QVariant qualityAttr = imageGetAttribute("quality");
    int quality          = qualityAttr.isValid() ? qualityAttr.toInt() : 90;

    if (quality < 0)
        quality = 90;
    if (quality > 100)
        quality = 100;

    QString     rate;
    QTextStream ts( &rate, QIODevice::WriteOnly );

    // NOTE: to have a lossless compression use quality=100.
    // jp2_encode()::optstr:
    // - rate=#B => the resulting file size is about # bytes
    // - rate=0.0 .. 1.0 => the resulting file size is about the factor times
    //                      the uncompressed size
    ts << "rate=" << ( quality / 100.0F );

    kDebug(50003) << "JPEG2000 quality: " << quality << endl;
    kDebug(50003) << "JPEG2000 " << rate << endl;

    int ret = jp2_encode(jp2_image, jp2_stream, rate.toUtf8().data());
    if (ret != 0)
    {
        kDebug(50003) << "Unable to encode JPEG2000 image" << endl;

        jas_image_destroy(jp2_image);
        jas_stream_close(jp2_stream);
        for (i = 0 ; i < (long)number_components ; ++i)
            jas_matrix_destroy(pixels[i]);

        jas_cleanup();

        return false;
    }
*/
    if (observer)
        observer->progressInfo(m_image, 1.0);

    imageSetAttribute("savedformat", "PGF");
    saveMetadata(filePath);

    return false;
}

bool PGFLoader::hasAlpha() const
{
    return m_hasAlpha;
}

bool PGFLoader::sixteenBit() const
{
    return m_sixteenBit;
}

bool PGFLoader::progressCallback(double percent, bool escapeAllowed)
{
    if (m_observer)
    {
        m_observer->progressInfo(m_image, percent);

    if (escapeAllowed)
        return (!m_observer->continueQuery(m_image));
    }

    return false;
}

}  // namespace Digikam
