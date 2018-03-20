/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-03
 * Description : A PGF IO file for DImg framework
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "digikam_config.h"
#include "pgfloader.h"

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
#include <QVariant>
#include <QByteArray>
#include <QTextStream>
#include <QDataStream>

// Windows includes

#ifdef WIN32
#include <windows.h>
#endif

// Libpgf includes

#include <PGFimage.h>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "dimgloaderobserver.h"
#include "pgfutils.h"
#include "metaengine.h"

namespace Digikam
{

static bool CallbackForLibPGF(double percent, bool escapeAllowed, void* data)
{
    if (data)
    {
        PGFLoader* d = static_cast<PGFLoader*>(data);

        if (d)
        {
            return d->progressCallback(percent, escapeAllowed);
        }
    }

    return false;
}

PGFLoader::PGFLoader(DImg* const image)
    : DImgLoader(image)
{
    m_hasAlpha   = false;
    m_sixteenBit = false;
    m_observer   = 0;
}

bool PGFLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    m_observer = observer;
    readMetadata(filePath, DImg::PGF);

    FILE* file = fopen(QFile::encodeName(filePath).constData(), "rb");

    if (!file)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Error: Could not open source file.";
        loadingFailed();
        return false;
    }

    unsigned char header[3];

    if (fread(&header, 3, 1, file) != 1)
    {
        fclose(file);
        loadingFailed();
        return false;
    }

    unsigned char pgfID[3] = { 0x50, 0x47, 0x46 };

    if (memcmp(&header[0], &pgfID, 3) != 0)
    {
        // not a PGF file
        fclose(file);
        loadingFailed();
        return false;
    }

    fclose(file);

    // -------------------------------------------------------------------
    // Initialize PGF API.

#ifdef WIN32
#ifdef UNICODE
    HANDLE fd = CreateFile((LPCWSTR)(QFile::encodeName(filePath).constData()), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#else
    HANDLE fd = CreateFile(QFile::encodeName(filePath).constData(), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#endif

    if (fd == INVALID_HANDLE_VALUE)
    {
        loadingFailed();
        return false;
    }

#else

    int fd = open(QFile::encodeName(filePath).constData(), O_RDONLY);

    if (fd == -1)
    {
        loadingFailed();
        return false;
    }

#endif

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
                m_hasAlpha = false;
                colorModel = DImg::RGB;
                break;

            case ImageModeRGBA:
                m_hasAlpha = true;
                colorModel = DImg::RGB;
                break;

            default:
                qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Cannot load PGF image: color mode not supported (" << pgf.Mode() << ")";
                loadingFailed();
                return false;
                break;
        }

        switch (pgf.Channels())
        {
            case 3:
            case 4:
                break;

            default:
                qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Cannot load PGF image: color channels number not supported (" << pgf.Channels() << ")";
                loadingFailed();
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
                qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Cannot load PGF image: color bits depth not supported (" << bitDepth << ")";
                loadingFailed();
                return false;
                break;
        }

        if(DIGIKAM_DIMG_LOG_PGF().isDebugEnabled()) {
            const PGFHeader* header = pgf.GetHeader();
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF width    = " << header->width;
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF height   = " << header->height;
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF bbp      = " << header->bpp;
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF channels = " << header->channels;
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF quality  = " << header->quality;
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF mode     = " << header->mode;
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "Has Alpha    = " << m_hasAlpha;
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "Is 16 bits   = " << m_sixteenBit;
        }

        // NOTE: see bug #273765 : Loading PGF thumbs with OpenMP support through a separated thread do not work properlly with libppgf 6.11.24
        pgf.ConfigureDecoder(false);

        int width   = pgf.Width();
        int height  = pgf.Height();
        uchar* data = 0;

        QSize originalSize(width, height);

        if (m_loadFlags & LoadImageData)
        {
            // -------------------------------------------------------------------
            // Find out if we do the fast-track loading with reduced size. PGF specific.
            int level          = 0;
            QVariant attribute = imageGetAttribute(QLatin1String("scaledLoadingSize"));

            if (attribute.isValid() && pgf.Levels() > 0)
            {
                int scaledLoadingSize = attribute.toInt();
                int i, w, h;

                for (i = pgf.Levels() - 1 ; i >= 0 ; --i)
                {
                    w = pgf.Width(i);
                    h = pgf.Height(i);

                    if (qMin(w, h) >= scaledLoadingSize)
                    {
                        break;
                    }
                }

                if (i >= 0)
                {
                    width  = w;
                    height = h;
                    level  = i;
                    qCDebug(DIGIKAM_DIMG_LOG_PGF) << "Loading PGF scaled version at level " << i
                                      << " (" << w << " x " << h << ") for size "
                                      << scaledLoadingSize;
                }
            }

            if (m_sixteenBit)
            {
                data = new_failureTolerant(width, height, 8); // 16 bits/color/pixel
            }
            else
            {
                data = new_failureTolerant(width, height, 4); // 8 bits/color/pixel
            }

            // Fill all with 255 including alpha channel.
            memset(data, 0xFF, width * height * (m_sixteenBit ? 8 : 4));

            pgf.Read(level, CallbackForLibPGF, this);
            pgf.GetBitmap(m_sixteenBit ? width * 8 : width * 4,
                          (UINT8*)data,
                          m_sixteenBit ? 64 : 32,
                          NULL,
                          CallbackForLibPGF, this);

            if (observer)
            {
                observer->progressInfo(m_image, 1.0);
            }
        }

        // -------------------------------------------------------------------
        // Get ICC color profile.

        if (m_loadFlags & LoadICCData)
        {
            // TODO: Implement proper storage in PGF for color profiles
            checkExifWorkingColorSpace();
        }

        imageWidth()  = width;
        imageHeight() = height;
        imageData()   = data;
        imageSetAttribute(QLatin1String("format"),             QLatin1String("PGF"));
        imageSetAttribute(QLatin1String("originalColorModel"), colorModel);
        imageSetAttribute(QLatin1String("originalBitDepth"),   bitDepth);
        imageSetAttribute(QLatin1String("originalSize"),       originalSize);

#ifdef WIN32
        CloseHandle(fd);
#else
        close(fd);
#endif

        return true;
    }
    catch (IOException& e)
    {
        int err = e.error;

        if (err >= AppError)
        {
            err -= AppError;
        }

        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Error: Opening and reading PGF image failed (" << err << ")!";

#ifdef WIN32
        CloseHandle(fd);
#else
        close(fd);
#endif

        loadingFailed();
        return false;
    }
    catch (std::bad_alloc& e)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Failed to allocate memory for loading" << filePath << e.what();

#ifdef WIN32
        CloseHandle(fd);
#else
        close(fd);
#endif

        loadingFailed();
        return false;
    }
    return true;
}

bool PGFLoader::save(const QString& filePath, DImgLoaderObserver* const observer)
{
    m_observer = observer;

#ifdef WIN32
#ifdef UNICODE
    HANDLE fd = CreateFile((LPCWSTR)(QFile::encodeName(filePath).constData()), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#else
    HANDLE fd = CreateFile(QFile::encodeName(filePath).constData(), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#endif

    if (fd == INVALID_HANDLE_VALUE)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Error: Could not open destination file.";
        return false;
    }

#elif defined(__POSIX__)
    int fd = open(QFile::encodeName(filePath).constData(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd == -1)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Error: Could not open destination file.";
        return false;
    }

#endif

    try
    {
        QVariant qualityAttr = imageGetAttribute(QLatin1String("quality"));
        int quality          = qualityAttr.isValid() ? qualityAttr.toInt() : 3;

        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF quality: " << quality;

        CPGFFileStream stream(fd);
        CPGFImage      pgf;
        PGFHeader      header;
        header.width   = imageWidth();
        header.height  = imageHeight();
        header.quality = quality;

        if (imageHasAlpha())
        {
            if (imageSixteenBit())
            {
                // NOTE : there is no PGF color mode in 16 bits with alpha.
                header.channels = 3;
                header.bpp      = 48;
                header.mode     = ImageModeRGB48;
            }
            else
            {
                header.channels = 4;
                header.bpp      = 32;
                header.mode     = ImageModeRGBA;
            }
        }
        else
        {
            if (imageSixteenBit())
            {
                header.channels = 3;
                header.bpp      = 48;
                header.mode     = ImageModeRGB48;
            }
            else
            {
                header.channels = 3;
                header.bpp      = 24;
                header.mode     = ImageModeRGBColor;
            }
        }

#ifdef PGFCodecVersionID
#   if PGFCodecVersionID < 0x061142
        header.background.rgbtBlue  = 0;
        header.background.rgbtGreen = 0;
        header.background.rgbtRed   = 0;
#   endif
#endif

        pgf.SetHeader(header);

        // NOTE: see bug #273765 : Loading PGF thumbs with OpenMP support through a separated thread do not work properlly with libppgf 6.11.24
        pgf.ConfigureEncoder(false);

        pgf.ImportBitmap(4 * imageWidth() * (imageSixteenBit() ? 2 : 1),
                         (UINT8*)imageData(),
                         imageBitsDepth() * 4,
                         NULL,
                         CallbackForLibPGF, this);

        UINT32 nWrittenBytes = 0;

#ifdef PGFCodecVersionID
#   if PGFCodecVersionID >= 0x061124
        pgf.Write(&stream, &nWrittenBytes, CallbackForLibPGF, this);
#   endif
#else
        pgf.Write(&stream, 0, CallbackForLibPGF, &nWrittenBytes, this);
#endif

        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF width     = " << header.width;
        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF height    = " << header.height;
        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF bbp       = " << header.bpp;
        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF channels  = " << header.channels;
        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF quality   = " << header.quality;
        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF mode      = " << header.mode;
        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "Bytes Written = " << nWrittenBytes;

#ifdef WIN32
        CloseHandle(fd);
#else
        close(fd);
#endif
        // TODO: Store ICC profile in an appropriate place in the image
        storeColorProfileInMetadata();

        if (observer)
        {
            observer->progressInfo(m_image, 1.0);
        }

        imageSetAttribute(QLatin1String("savedformat"), QLatin1String("PGF"));
        saveMetadata(filePath);

        return true;
    }
    catch (IOException& e)
    {
        int err = e.error;

        if (err >= AppError)
        {
            err -= AppError;
        }

        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Error: Opening and saving PGF image failed (" << err << ")!";

#ifdef WIN32
        CloseHandle(fd);
#else
        close(fd);
#endif

        return false;
    }
    return true;
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
        {
            return (!m_observer->continueQuery(m_image));
        }
    }

    return false;
}

bool PGFLoader::isReadOnly() const
{
    return false;
}

}  // namespace Digikam
