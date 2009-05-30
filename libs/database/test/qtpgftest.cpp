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
#include <QFile>
#include <QIODevice>
#include <QByteArray>
#include <QDataStream>

// KDE includes.

#include "kdebug.h"

// LibPGF includes

#include "PGFimage.h"

// Local includes.

#include "thumbnailpgf.h"

using namespace Digikam;

int main(int /*argc*/, char** /*argv*/)
{
    QImage     img;
    QByteArray data;

    // QImage => PGF conversion


    img.load("test.png");
    if (!ThumbnailPGF::writePGFImageData(img, data, 3))
    {
        kDebug(50003) << "writePGFImageData failed..." << endl;
        return -1;
    }

    // Write PGF file.

    QFile file("result.pgf");
    if ( !file.open(QIODevice::WriteOnly) )
    {
        kDebug(50003) << "Cannot open PGF file to write..." << endl;
        return -1;
    }
    QDataStream stream(&file);
    stream.writeRawData(data.data(), data.size());
    file.close();

    // PGF => QImage conversion

    if (!ThumbnailPGF::readPGFImageData(data, img))
    {
        kDebug(50003) << "readPGFImageData failed..." << endl;
        return -1;
    }
    img.save("result.png", "PNG");

    return 0;
}
