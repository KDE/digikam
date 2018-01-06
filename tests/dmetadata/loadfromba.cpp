/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : a command line tool to load metadata from byte array
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

// Qt includes

#include <QDataStream>
#include <QImage>
#include <QString>
#include <QFile>
#include <QByteArray>
#include <QDebug>

// Local includes

#include "dmetadata.h"

using namespace Digikam;

int main (int argc, char **argv)
{
    if (argc != 2)
    {
        qDebug() << "loadfromba - test to load metadata from image as byte array";
        qDebug() << "Usage: <image>";
        return -1;
    }

    QString filePath = QString::fromLocal8Bit(argv[1]);
    QString baFile(QLatin1String("ba.dat"));

    QImage image(filePath);
    image.save(baFile, "PNG");

    QFile file(baFile);

    if ( !file.open(QIODevice::ReadOnly) )
        return false;

    QByteArray data;
    data.resize(file.size());
    QDataStream stream( &file );
    stream.readRawData(data.data(), data.size());
    file.close();

    DMetadata meta;
    meta.loadFromData(data);

    return 0;
}
