/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-27
 * Description : a command line tool to test XMP sidecar functionality
 *
 * Copyright (C) 2010 by Jakob Malm <jakob dot malm at gmail dot com>
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

// Qt includes

#include <QString>
#include <QFile>
#include <QDebug>

// Local includes

#include "dmetadata.h"

using namespace Digikam;

int main (int argc, char **argv)
{
    if(argc != 2)
    {
        qDebug() << "readimagewritecmpsidecar - read metadata from image and write to XMP sidecar";
        qDebug() << "Usage: <image>";
        return -1;
    }

    QString filePath = QString::fromLocal8Bit(argv[1]);

    DMetadata meta;

    // Read metadata from the image
    meta.load(filePath);

    // Write metadata to XMP sidecar
    meta.setMetadataWritingMode(DMetadata::WRITETOSIDECARONLY);
    meta.save(filePath);

    return 0;
}
