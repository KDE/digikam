/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : PGF thumbnail interface.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbnailpgf.h"

// Qt includes

#include <QImage>
#include <QByteArray>

// KDE includes

#include <kdebug.h>

// LibPGF includes

#include "PGFimage.h"

namespace Digikam
{

bool ThumbnailPGF::readPGFImageData(const QByteArray& data, QImage& img)
{
    try
    {
        CPGFMemoryStream stream((UINT8*)data.data(), (size_t)data.size());
        CPGFImage pgfImg;
        pgfImg.Open(&stream);

        if (pgfImg.Channels() != 3)
        {
            kDebug(50003) << "PGF channels not supported" << endl;
            return false;
        }

        const PGFHeader* header = pgfImg.GetHeader();
        kDebug(50003) << "PGF width    = " << header->width    << endl;
        kDebug(50003) << "PGF height   = " << header->height   << endl;
        kDebug(50003) << "PGF bbp      = " << header->bpp      << endl;
        kDebug(50003) << "PGF channels = " << header->channels << endl;
        kDebug(50003) << "PGF quality  = " << header->quality  << endl;
        kDebug(50003) << "PGF mode     = " << header->mode     << endl;

        img = QImage(pgfImg.Width(), pgfImg.Height(), QImage::Format_RGB32);
        pgfImg.Read();
        pgfImg.GetBitmap(img.bytesPerLine(), (UINT8*)img.bits(), 8);
        img = img.rgbSwapped();
    }
    catch(IOException& e)
    {
        int err = e.error;

        if (err >= AppError) err -= AppError;
        kDebug(50003) << "Error running libpgf (" << err << ")!" << endl;
        return false;
    }

    return true;
}

bool ThumbnailPGF::writePGFImageData(const QImage& img, QByteArray& data, int quality)
{
    try
    {
        if (img.isNull())
        {
            kDebug(50003) << "Thumb image is null" << endl;
            return false;
        }

        // No need Alpha to optimize space on DB.
        if (img.format() != QImage::Format_ARGB32)
            img.convertToFormat(QImage::Format_ARGB32);

        CPGFImage pgfImg;

        PGFHeader header;
        header.width    = img.width();
        header.height   = img.height();
        header.bpp      = img.depth();
        header.channels = 3;
        header.quality  = quality;
        header.mode     = ImageModeRGBColor;
        header.background.rgbtBlue = header.background.rgbtGreen = header.background.rgbtRed = 0;
        pgfImg.SetHeader(header);

        int channelMap[] = { 3, 2, 1 };
        pgfImg.ImportBitmap(img.bytesPerLine(), (UINT8*)img.bits(), 8, channelMap);

        // TODO : optimize memory allocation...
        CPGFMemoryStream stream(256000);
        UINT32 nWrittenBytes = 0;
        pgfImg.Write(&stream, 0, NULL, &nWrittenBytes);

        data = QByteArray((const char*)stream.GetBuffer(), nWrittenBytes);
    }
    catch(IOException& e)
    {
        int err = e.error;

        if (err >= AppError) err -= AppError;
        kDebug(50003) << "Error running libpgf (" << err << ")!" << endl;
        return false;
    }

    return true;
}

}  // namespace Digikam
