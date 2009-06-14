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

// C++ includes

#include <ctime>

// Qt includes

#include <QImage>
#include <QBuffer>
#include <QFile>
#include <QIODevice>
#include <QByteArray>
#include <QDataStream>

// KDE includes

#include "kdebug.h"

// Local includes

#include "pgfutils.h"

using namespace Digikam;

int main(int /*argc*/, char** /*argv*/)
{
    clock_t    start, end;
    QImage     img;
    QByteArray pgfData, jpgData;

    // QImage => PGF conversion

    img.load("test.png");

    start = clock();

    if (!writePGFImageData(img, pgfData, 4))
    {
        kDebug(50003) << "writePGFImageData failed..." << endl;
        return -1;
    }
    end = clock();

    kDebug(50003) << "PGF Encoding time: " << double(end - start)/CLOCKS_PER_SEC << " s" << endl;

    // Write PGF file.

    QFile file("test.pgf");
    if ( !file.open(QIODevice::WriteOnly) )
    {
        kDebug(50003) << "Cannot open PGF file to write..." << endl;
        return -1;
    }
    QDataStream stream(&file);
    stream.writeRawData(pgfData.data(), pgfData.size());
    file.close();

    // PGF => QImage conversion

    start = clock();

    if (!readPGFImageData(pgfData, img))
    {
        kDebug(50003) << "readPGFImageData failed..." << endl;
        return -1;
    }
    end = clock();

    img.save("test2.png", "PNG");

    kDebug(50003) << "PGF Decoding time: " << double(end - start)/CLOCKS_PER_SEC << " s" << endl;

    // JPEG tests for comparisons.

    start = clock();

    QBuffer buffer(&jpgData);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "JPEG", 85);  // Here we will use JPEG quality = 85 to reduce artifacts.
    if (jpgData.isNull())
    {
        kDebug(50003) << "Save JPG image data to byte array failed..." << endl;
        return -1;
    }
    end = clock();

    kDebug(50003) << "JPG Encoding time: " << double(end - start)/CLOCKS_PER_SEC << " s" << endl;

    start = clock();

    buffer.open(QIODevice::ReadOnly);
    img.load(&buffer, "JPEG");
    if (jpgData.isNull())
    {
        kDebug(50003) << "Load JPG image data from byte array failed..." << endl;
        return -1;
    }
    end = clock();

    kDebug(50003) << "JPG Decoding time: " << double(end - start)/CLOCKS_PER_SEC << " s" << endl;

    return 0;
}
