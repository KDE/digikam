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
#include <QVariant>
#include <QByteArray>
#include <QTextStream>
#include <QDataStream>

// KDE includes

#include <kdebug.h>
#include <ktemporaryfile.h>

// Windows includes

#ifdef WIN32
#include <windows.h>
#endif

// Local includes

#include "PGFimage.h"
#include "dmetadata.h"
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
#ifdef Q_CC_MSVC
    HANDLE fd = CreateFile((LPCWSTR)(QFile::encodeName(filePath).constData()), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#else /* MinGW compiler. */
    HANDLE fd = CreateFile(QFile::encodeName(filePath), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#endif
    if (fd == INVALID_HANDLE_VALUE)
        return false;
#else
    int fd = open(QFile::encodeName(filePath), O_RDONLY);
    if (fd == -1)
        return false;
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
                kDebug(50003) << "Cannot load PGF image: color mode not supported (" << pgf.Mode() << ")" << endl;
                return false;
                break;
        }

        switch (pgf.Channels())
        {
            case 3:
            case 4:
                break;
            default:
                kDebug(50003) << "Cannot load PGF image: color channels number not supported (" << pgf.Channels() << ")" << endl;
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
                kDebug(50003) << "Cannot load PGF image: color bits depth not supported (" << bitDepth << ")" << endl;
                return false;
                break;
        }

#ifdef ENABLE_DEBUG_MESSAGES
        const PGFHeader* header = pgf.GetHeader();
        kDebug(50003) << "PGF width    = " << header->width    << endl;
        kDebug(50003) << "PGF height   = " << header->height   << endl;
        kDebug(50003) << "PGF bbp      = " << header->bpp      << endl;
        kDebug(50003) << "PGF channels = " << header->channels << endl;
        kDebug(50003) << "PGF quality  = " << header->quality  << endl;
        kDebug(50003) << "PGF mode     = " << header->mode     << endl;
        kDebug(50003) << "Has Alpha    = " << m_hasAlpha       << endl;
        kDebug(50003) << "Is 16 bits   = " << m_sixteenBit     << endl;
#endif

        UINT32 size;
        const UINT8* udata = pgf.GetUserData(size);
        if (udata)
        {
            QByteArray data((const char*)udata, size);
            DMetadata meta;
            if (meta.load(data))
            {
                kDebug(50003) << "Load PGF metadata (" << data.size() << ")" << endl;
                QMap<int, QByteArray>& imageMetadata = imageMetaData();
                imageMetadata.clear();

                if (!meta.getComments().isNull())
                {
                    kDebug(50003) << "PGF as Comments" << endl;
                    imageMetadata.insert(DImg::COM, meta.getComments());
                }
                if (!meta.getExif().isNull())
                {
                    kDebug(50003) << "PGF as Exif" << endl;
                    imageMetadata.insert(DImg::EXIF, meta.getExif());
                }
                if (!meta.getIptc().isNull())
                {
                    kDebug(50003) << "PGF as Iptc" << endl;
                    imageMetadata.insert(DImg::IPTC, meta.getIptc());
                }
                if (!meta.getXmp().isNull())
                {
                    kDebug(50003) << "PGF as Xmp" << endl;
                    imageMetadata.insert(DImg::XMP, meta.getXmp());
                }
            }
        }

        int width   = pgf.Width();
        int height  = pgf.Height();
        uchar *data = 0;

        if (m_loadFlags & LoadImageData)
        {
            // -------------------------------------------------------------------
            // Find out if we do the fast-track loading with reduced size. PGF specific.
            int scaledLoadingSize = 0;
            int level             = 0;
            QVariant attribute = imageGetAttribute("pgfScaledLoadingSize");
            if (attribute.isValid() && pgf.Levels() > 0)
            {
                scaledLoadingSize = attribute.toInt();
                int i, w, h;
                for (i=pgf.Levels()-1 ; i>=0 ; --i)
                {
                    w = pgf.Width(i);
                    h = pgf.Height(i);
                    if (qMin(w, h) >= scaledLoadingSize)
                        break;
                }

                if (i >= 0)
                {
                    width  = w;
                    height = h;
                    level  = i;
                    kDebug(50003) << "Loading PGF scaled version at level " << i
                                  << " (" << w << " x " << h << ") for size "
                                  << scaledLoadingSize << endl;
                }
            }

            if (m_sixteenBit)
                data = new uchar[width*height*8];  // 16 bits/color/pixel
            else
                data = new uchar[width*height*4];  // 8 bits/color/pixel

            // Fill all with 255 including alpha channel.
            memset(data, sizeof(data), 0xFF);

            pgf.Read(level, CallbackForLibPGF, this);
            pgf.GetBitmap(m_sixteenBit ? width*8 : width*4,
                          (UINT8*)data,
                          m_sixteenBit ? 64 : 32,
                          NULL,
                          CallbackForLibPGF, this);

            if (observer)
                observer->progressInfo(m_image, 1.0);
        }

        imageWidth()  = width;
        imageHeight() = height;
        imageData()   = data;
        imageSetAttribute("format", "PGF");
        imageSetAttribute("originalColorModel", colorModel);
        imageSetAttribute("originalBitDepth", bitDepth);

#ifdef WIN32
        CloseHandle(fd);
#else
        close(fd);
#endif

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
#ifdef Q_CC_MSVC
    HANDLE fd = CreateFile((LPCWSTR)(QFile::encodeName(filePath).constData()), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#else /* MinGW compiler. */
    HANDLE fd = CreateFile(QFile::encodeName(filePath), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#endif
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

    try
    {
        QVariant qualityAttr = imageGetAttribute("quality");
        int quality          = qualityAttr.isValid() ? qualityAttr.toInt() : 3;

        kDebug(50003) << "PGF quality: " << quality << endl;

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

        header.background.rgbtBlue = header.background.rgbtGreen = header.background.rgbtRed = 0;

        // Host Exif/IPTC/XMP metadata to PGF header. We use a small thumb JPEG image to embed all data
        // and pass it to PGF header as user data byte-array.
        QImage thumb = m_image->smoothScale(256, 256, Qt::KeepAspectRatio).copyQImage();
        KTemporaryFile thumbTmp;
        thumbTmp.setPrefix(filePath);
        thumbTmp.setSuffix("-thumb.jpg");
        thumbTmp.setAutoRemove(true);
        if ( !thumbTmp.open() )
        {
            kDebug(50003) << "Cannot open tmp file to save PGF metadata" << endl;
            pgf.SetHeader(header);
        }
        else
        {
            thumb.save(thumbTmp.fileName(), "JPEG");
            kDebug(50003) << "Created PGF metadata tmp file" << endl;

            DMetadata meta(thumbTmp.fileName());
            QMap<int, QByteArray>& metaData = imageMetaData();
            meta.setExif(metaData[DImg::EXIF]);
            meta.setIptc(metaData[DImg::IPTC]);
            meta.setXmp(metaData[DImg::XMP]);
            meta.setComments(metaData[DImg::COM]);
            meta.removeIptcTag("Iptc.Application2.Preview");
            meta.removeIptcTag("Iptc.Application2.PreviewFormat");
            meta.removeIptcTag("Iptc.Application2.PreviewVersion");
            meta.applyChanges();

            QByteArray data;
            data.resize(thumbTmp.size());
            QDataStream stream( &thumbTmp );
            stream.readRawData(data.data(), data.size());
            thumbTmp.close();

            kDebug(50003) << "Save PGF metadata (" << data.size() << ")"  << endl;
            pgf.SetHeader(header, 0, (UINT8*)data.constData(), data.size());
        }

        pgf.ImportBitmap(4 * imageWidth() * (imageSixteenBit() ? 2 : 1),
                         (UINT8*)imageData(),
                         imageBitsDepth() * 4,
                         NULL,
                         CallbackForLibPGF, this);

        UINT32 nWrittenBytes = 0;
        pgf.Write(&stream, 0, CallbackForLibPGF, &nWrittenBytes, this);

#ifdef ENABLE_DEBUG_MESSAGES
        kDebug(50003) << "PGF width     = " << header.width    << endl;
        kDebug(50003) << "PGF height    = " << header.height   << endl;
        kDebug(50003) << "PGF bbp       = " << header.bpp      << endl;
        kDebug(50003) << "PGF channels  = " << header.channels << endl;
        kDebug(50003) << "PGF quality   = " << header.quality  << endl;
        kDebug(50003) << "PGF mode      = " << header.mode     << endl;
        kDebug(50003) << "Bytes Written = " << nWrittenBytes   << endl;
#endif

#ifdef WIN32
        CloseHandle(fd);
#else
        close(fd);
#endif

        if (observer)
            observer->progressInfo(m_image, 1.0);

        imageSetAttribute("savedformat", "PGF");
        saveMetadata(filePath);

        return true;
    }
    catch(IOException& e)
    {
        int err = e.error;
        if (err >= AppError) err -= AppError;
        kDebug(50003) << "Error: Opening and saving PGF image failed (" << err << ")!" << endl;

#ifdef WIN32
        CloseHandle(fd);
#else
        close(fd);
#endif

        return false;
    }
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
