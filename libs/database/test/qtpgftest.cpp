/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2009-02-04
 * Description : a command line tool to test qt PGF interface
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com> 
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes.

#include <QImage>
#include <QString>
#include <QByteArray>

// KDE includes.

#include "kdebug.h"

// LibPGF includes

#include "PGFimage.h"

/** PGF image data to QImage */
static bool readPGFImageData(const QByteArray& data, QImage& img);
/** QImage to PGF image data */
static bool writePGFImageData(const QImage& img, QByteArray& data);

int main(int /*argc*/, char** /*argv*/)
{
    QImage     img;
    QByteArray data;

    img.load("test.png");
    if (!writePGFImageData(img, data))
    {
        kDebug(50003) << "writePGFImageData failed..." << endl;
        return -1;
    }

    if (!readPGFImageData(data, img))
    {
        kDebug(50003) << "readPGFImageData failed..." << endl;
        return -1;
    }
    img.save("result.png", "PNG");

    return 0;
}

bool readPGFImageData(const QByteArray& data, QImage& img)
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

bool writePGFImageData(const QImage& img, QByteArray& data)
{
    try
    {
        if (img.isNull())
        {
            kDebug(50003) << "Thumb image is null" << endl;
            return false;
        }

        // No need Alpha to optimize space on DB.
        img.convertToFormat(QImage::Format_RGB32);

        CPGFImage pgfImg;

        PGFHeader header;
        header.width    = img.width();
        header.height   = img.height();
        header.bpp      = 8;
        header.channels = 3;
        header.quality  = 10;
        header.mode     = ImageModeRGBColor;
        header.background.rgbtBlue = header.background.rgbtGreen = header.background.rgbtRed = 0;
        pgfImg.SetHeader(header);

        pgfImg.ImportBitmap(img.bytesPerLine(), (UINT8*)img.bits(), 8);

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
