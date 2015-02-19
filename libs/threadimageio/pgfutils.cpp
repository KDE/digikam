/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : PGF utils.
 *
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "config-digikam.h"
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

// KDE includes

#include <kdebug.h>

// Windows includes

#ifdef WIN32
#include <windows.h>
#endif

// LibPGF includes

#include <PGFimage.h>

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
            kDebug() << "PGF image data to decode : size is null";
            return false;
        }

        CPGFMemoryStream stream((UINT8*)data.data(), (size_t)data.size());
        if (verbose) kDebug() << "image data stream size is : " << stream.GetSize();

        CPGFImage pgfImg;
        // NOTE: see bug #273765 : Loading PGF thumbs with OpenMP support through a separated thread do not work properlly with libppgf 6.11.24
        pgfImg.ConfigureDecoder(false);

        pgfImg.Open(&stream);

        if (verbose) kDebug() << "PGF image is open";

        if (pgfImg.Channels() != 4)
        {
            kDebug() << "PGF channels not supported";
            return false;
        }

        img = QImage(pgfImg.Width(), pgfImg.Height(), QImage::Format_ARGB32);
        pgfImg.Read();

        if (verbose) kDebug() << "PGF image is read";

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

        if (verbose) kDebug() << "PGF image is decoded";
    }
    catch (IOException& e)
    {
        int err = e.error;

        if (err >= AppError)
        {
            err -= AppError;
        }

        kDebug() << "Error running libpgf (" << err << ")!";
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
    HANDLE fd = CreateFile(QFile::encodeName(filePath), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#endif

    if (fd == INVALID_HANDLE_VALUE)
    {
        kDebug() << "Error: Could not open destination file.";
        return false;
    }

#elif defined(__POSIX__)
    int fd = open(QFile::encodeName(filePath), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd == -1)
    {
        kDebug() << "Error: Could not open destination file.";
        return false;
    }
#endif

    CPGFFileStream stream(fd);
    UINT32 nWrittenBytes = 0;
    bool ret             = writePGFImageDataToStream(image, stream, quality, nWrittenBytes, verbose);

    if (!nWrittenBytes)
    {
        kDebug() << "Written PGF file : data size is null";
        ret = false;
    }
    else
    {
        if (verbose) kDebug() << "file size written : " << nWrittenBytes;
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
            kDebug() << "PGF stream memory allocation in bytes: " << rawSize;

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
            kDebug() << "Encoded PGF image : data size is null";
            ret = false;
        }
        else
        {
            if (verbose)
                kDebug() << "data size written : " << pgfsize;
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

        kDebug() << "Error running libpgf (" << err << ")!";
        return false;
    }
}

bool writePGFImageDataToStream(const QImage& image, CPGFStream& stream, int quality, UINT32& nWrittenBytes, bool verbose)
{
    try
    {
        if (image.isNull())
        {
            kDebug() << "Thumb image is null";
            return false;
        }

        QImage img;

        // Convert image with Alpha channel.
        if (image.format() != QImage::Format_ARGB32)
        {
            img = image.convertToFormat(QImage::Format_ARGB32);
            if (verbose) kDebug() << "RGB => ARGB";
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
            kDebug() << "PGF image settings:";
            kDebug() << "   width: "              << header.width;
            kDebug() << "   height: "             << header.height;
            kDebug() << "   nLevels: "            << header.nLevels;
            kDebug() << "   quality: "            << header.quality;
            kDebug() << "   bpp: "                << header.bpp;
            kDebug() << "   channels: "           << header.channels;
            kDebug() << "   mode: "               << header.mode;
            kDebug() << "   usedBitsPerChannel: " << header.usedBitsPerChannel;
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

        kDebug() << "Error running libpgf (" << err << ")!";
        return false;
    }

    return true;
}

bool loadPGFScaled(QImage& img, const QString& path, int maximumSize)
{
    FILE* const file = fopen(QFile::encodeName(path), "rb");

    if (!file)
    {
        kDebug() << "Error: Could not open source file.";
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
    HANDLE fd = CreateFile(QFile::encodeName(path), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#endif

    if (fd == INVALID_HANDLE_VALUE)
    {
        return false;
    }

#else
    int fd = open(QFile::encodeName(path), O_RDONLY);

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
        kDebug() << "PGF width    = " << header->width;
        kDebug() << "PGF height   = " << header->height;
        kDebug() << "PGF bbp      = " << header->bpp;
        kDebug() << "PGF channels = " << header->channels;
        kDebug() << "PGF quality  = " << header->quality;
        kDebug() << "PGF mode     = " << header->mode;
        kDebug() << "PGF levels   = " << header->nLevels;
        kDebug() << "Level (w x h)= " << i << "(" << pgf.Width(i)
                                      << " x " << pgf.Height(i) << ")";
        kDebug() << "QImage depth = " << img.depth();
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

        kDebug() << "Error running libpgf (" << err << ")!";
        return false;
    }

    return true;
}

QString libPGFVersion()
{
    return (QString(PGFCodecVersion));
}

} // namespace PGFUtils

} // namespace Digikam
