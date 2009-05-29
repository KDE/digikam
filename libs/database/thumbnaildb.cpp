/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : database thumbnail interface.
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "thumbnaildb.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QImage>
#include <QByteArray>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// LibPGF includes

#include "PGFimage.h"

// Local includes

#include "databasebackend.h"
#include "collectionmanager.h"
#include "collectionlocation.h"

namespace Digikam
{

class ThumbnailDBPriv
{

public:

    ThumbnailDBPriv()
    {
        db = 0;
    }

    DatabaseBackend *db;
};

ThumbnailDB::ThumbnailDB(DatabaseBackend *backend)
       : d(new ThumbnailDBPriv)
{
    d->db = backend;
}

ThumbnailDB::~ThumbnailDB()
{
    delete d;
}

void ThumbnailDB::setSetting(const QString& keyword,
                         const QString& value )
{
    d->db->execSql( QString("REPLACE into Settings VALUES (?,?);"),
                    keyword, value );
}

QString ThumbnailDB::getSetting(const QString& keyword)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT value FROM Settings "
                            "WHERE keyword=?;"),
                    keyword, &values );

    if (values.isEmpty())
        return QString();
    else
        return values.first().toString();
}


bool ThumbnailDB::readPGFImageData(const QByteArray& data, QImage& img)
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

bool ThumbnailDB::writePGFImageData(const QImage& img, QByteArray& data)
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

}  // namespace Digikam
