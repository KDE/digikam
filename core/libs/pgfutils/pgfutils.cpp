/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : PGF utils.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "pgfutils.h"

// C Ansi includes

extern "C"
{
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
}

// Qt includes

#include <QImage>
#include <QByteArray>
#include <QFile>

// Windows includes

#ifdef WIN32
#include <windows.h>
#endif

// LibPGF includes

#include <PGFimage.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

namespace PGFUtils
{

// Private method
bool writePGFImageDataToStream(const QImage& image, CPGFStream& stream, int quality, UINT32& nWrittenBytes, bool verbose);

bool readPGFImageData(const QByteArray& data, QImage& img, bool verbose)
{
    try
    {
        if (data.isEmpty())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: PGF image data to decode : size is null";
            return false;
        }

        CPGFMemoryStream stream((UINT8*)data.data(), (size_t)data.size());
        if (verbose) qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: image data stream size is : " << stream.GetSize();

        CPGFImage pgfImg;
        // NOTE: see bug #273765 : Loading PGF thumbs with OpenMP support through a separated thread do not work properlly with libppgf 6.11.24
        pgfImg.ConfigureDecoder(false);

        pgfImg.Open(&stream);

        if (verbose) qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: PGF image is open";

        if (pgfImg.Channels() != 4)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: PGF channels not supported";
            return false;
        }

        img = QImage(pgfImg.Width(), pgfImg.Height(), QImage::Format_ARGB32);
        pgfImg.Read();

        if (verbose) qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: PGF image is read";

        if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
        {
            int map[] = {3, 2, 1, 0};
            pgfImg.GetBitmap(img.bytesPerLine(), (UINT8*)img.bits(), img.depth(), map);
        }
        else
        {
            int map[] = {0, 1, 2, 3};
            pgfImg.GetBitmap(img.bytesPerLine(), (UINT8*)img.bits(), img.depth(), map);
        }

        if (verbose) qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: PGF image is decoded";
    }
    catch (IOException& e)
    {
        int err = e.error;

        if (err >= AppError)
        {
            err -= AppError;
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: Error running libpgf (" << err << ")!";
        return false;
    }

    return true;
}

bool writePGFImageFile(const QImage& image, const QString& filePath, int quality, bool verbose)
{
#ifdef WIN32
#ifdef UNICODE
    HANDLE fd = CreateFile((LPCWSTR)(QFile::encodeName(filePath).constData()), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#else
    HANDLE fd = CreateFile(QFile::encodeName(filePath).constData(), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#endif

    if (fd == INVALID_HANDLE_VALUE)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: Error: Could not open destination file.";
        return false;
    }

#elif defined(__POSIX__)
    int fd = open(QFile::encodeName(filePath).constData(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd == -1)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: Error: Could not open destination file.";
        return false;
    }
#endif

    CPGFFileStream stream(fd);
    UINT32 nWrittenBytes = 0;
    bool ret             = writePGFImageDataToStream(image, stream, quality, nWrittenBytes, verbose);

    if (!nWrittenBytes)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: Written PGF file : data size is null";
        ret = false;
    }
    else
    {
        if (verbose) qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: file size written : " << nWrittenBytes;
    }

#ifdef WIN32
    CloseHandle(fd);
#else
    close(fd);
#endif

    return ret;
}

bool writePGFImageData(const QImage& image, QByteArray& data, int quality, bool verbose)
{
    try
    {
        // We will use uncompressed image bytes size to allocate PGF stream in memory. In all case, due to PGF compression ratio,
        // PGF data will be so far lesser than image raw size.
        int rawSize          = image.byteCount();
        CPGFMemoryStream stream(rawSize);

        if (verbose)
            qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: PGF stream memory allocation in bytes: " << rawSize;

        UINT32 nWrittenBytes = 0;
        bool ret             = writePGFImageDataToStream(image, stream, quality, nWrittenBytes, verbose);
        int pgfsize          =
#ifdef PGFCodecVersionID
#   if PGFCodecVersionID == 0x061224
                               // Wrap around libpgf 6.12.24 about CPGFMemoryStream bytes size generated to make PGF file data.
                               // It miss 16 bytes at end. This solution fix the problem. Problem have been fixed in 6.12.27.
                               nWrittenBytes + 16;
#   else
                               nWrittenBytes;
#   endif
#else
                               nWrittenBytes;
#endif

        data                 = QByteArray((const char*)stream.GetBuffer(), pgfsize);

        if (!pgfsize)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: Encoded PGF image : data size is null";
            ret = false;
        }
        else
        {
            if (verbose)
                qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: data size written : " << pgfsize;
        }

        return ret;
    }
    catch (IOException& e)
    {
        int err = e.error;

        if (err >= AppError)
        {
            err -= AppError;
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: Error running libpgf (" << err << ")!";
        return false;
    }
}

bool writePGFImageDataToStream(const QImage& image, CPGFStream& stream, int quality, UINT32& nWrittenBytes, bool verbose)
{
    try
    {
        if (image.isNull())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: Thumb image is null";
            return false;
        }

        QImage img;

        // Convert image with Alpha channel.
        if (image.format() != QImage::Format_ARGB32)
        {
            img = image.convertToFormat(QImage::Format_ARGB32);
            if (verbose) qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: RGB => ARGB";
        }
        else
        {
            img = image;
        }

        CPGFImage pgfImg;
        PGFHeader header;
        header.width                = img.width();
        header.height               = img.height();
        header.nLevels              = 0;             // Auto.
        header.quality              = quality;
        header.bpp                  = img.depth();
        header.channels             = 4;
        header.mode                 = ImageModeRGBA;
        header.usedBitsPerChannel   = 0;             // Auto

#ifdef PGFCodecVersionID
#   if PGFCodecVersionID < 0x061142
        header.background.rgbtBlue  = 0;
        header.background.rgbtGreen = 0;
        header.background.rgbtRed   = 0;
#   endif
#endif
        pgfImg.SetHeader(header);

        // NOTE: see bug #273765 : Loading PGF thumbs with OpenMP support through a separated thread do not work properlly with libppgf 6.11.24
        pgfImg.ConfigureEncoder(false);

        if (verbose)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: PGF image settings:";
            qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils:    width: "              << header.width;
            qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils:    height: "             << header.height;
            qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils:    nLevels: "            << header.nLevels;
            qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils:    quality: "            << header.quality;
            qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils:    bpp: "                << header.bpp;
            qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils:    channels: "           << header.channels;
            qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils:    mode: "               << header.mode;
            qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils:    usedBitsPerChannel: " << header.usedBitsPerChannel;
        }

        if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
        {
            int map[] = {3, 2, 1, 0};
            pgfImg.ImportBitmap(img.bytesPerLine(), (UINT8*)img.bits(), img.depth(), map);
        }
        else
        {
            int map[] = {0, 1, 2, 3};
            pgfImg.ImportBitmap(img.bytesPerLine(), (UINT8*)img.bits(), img.depth(), map);
        }

        nWrittenBytes = 0;

#ifdef PGFCodecVersionID
#   if PGFCodecVersionID >= 0x061124
        pgfImg.Write(&stream, &nWrittenBytes);
#   else
        pgfImg.Write(&stream, 0, 0, &nWrittenBytes);
#   endif
#else
        pgfImg.Write(&stream, 0, 0, &nWrittenBytes);
#endif

    }
    catch (IOException& e)
    {
        int err = e.error;

        if (err >= AppError)
        {
            err -= AppError;
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: Error running libpgf (" << err << ")!";
        return false;
    }

    return true;
}

bool loadPGFScaled(QImage& img, const QString& path, int maximumSize)
{
    FILE* const file = fopen(QFile::encodeName(path).constData(), "rb");

    if (!file)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: Error: Could not open source file.";
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
#ifdef UNICODE
    HANDLE fd = CreateFile((LPCWSTR)(QFile::encodeName(path).constData()), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#else
    HANDLE fd = CreateFile(QFile::encodeName(path).constData(), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#endif

    if (fd == INVALID_HANDLE_VALUE)
    {
        return false;
    }

#else
    int fd = open(QFile::encodeName(path).constData(), O_RDONLY);

    if (fd == -1)
    {
        return false;
    }

#endif

    try
    {
        CPGFFileStream stream(fd);
        CPGFImage      pgf;
        pgf.Open(&stream);

        // Try to find the right PGF level to get reduced image accordingly
        // with preview size wanted.
        int i = 0;

        if (pgf.Levels() > 0)
        {
            for (i=pgf.Levels()-1 ; i>=0 ; --i)
            {
                if (qMin((int)pgf.Width(i), (int)pgf.Height(i)) >= maximumSize)
                {
                    break;
                }
            }
        }

        if (i < 0)
        {
            i = 0;
        }

        pgf.Read(i);  // Read PGF image at reduced level i.
        img = QImage(pgf.Width(i), pgf.Height(i), QImage::Format_RGB32);

/*
        const PGFHeader* header = pgf.GetHeader();
        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: PGF width    = " << header->width;
        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: PGF height   = " << header->height;
        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: PGF bbp      = " << header->bpp;
        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: PGF channels = " << header->channels;
        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: PGF quality  = " << header->quality;
        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: PGF mode     = " << header->mode;
        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: PGF levels   = " << header->nLevels;
        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: Level (w x h)= " << i << "(" << pgf.Width(i)
                                      << " x " << pgf.Height(i) << ")";
        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: QImage depth = " << img.depth();
*/

        if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
        {
            int map[] = {3, 2, 1, 0};
            pgf.GetBitmap(img.bytesPerLine(), (UINT8*)img.bits(), img.depth(), map);
        }
        else
        {
            int map[] = {0, 1, 2, 3};
            pgf.GetBitmap(img.bytesPerLine(), (UINT8*)img.bits(), img.depth(), map);
        }
    }
    catch (IOException& e)
    {
        int err = e.error;

        if (err >= AppError)
        {
            err -= AppError;
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "PGFUtils: Error running libpgf (" << err << ")!";
        return false;
    }

    return true;
}

QString libPGFVersion()
{
    return (QLatin1String(PGFCodecVersion));
}

} // namespace PGFUtils

} // namespace Digikam
