/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-06-11
 * @brief  a command line tool to tag from photo
 *
 * @author Copyright (C) 2009-2017 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
        qDebug() << "erasetag - erase tag from image";
        qDebug() << "Usage: <image>";
        return -1;
    }

    QString filePath = QString::fromLocal8Bit(argv[1]);

    DMetadata meta;
    meta.load(filePath);
    meta.setWriteRawFiles(true);
    bool b = meta.removeExifTag("Exif.OlympusIp.BlackLevel", false);
    qDebug() << "Exif.OlympusIp.BlackLevel found = " << b;

    QByteArray ba = meta.getExifTagData("Exif.OlympusIp.BlackLevel");
    qDebug() << "Exif.OlympusIp.BlackLevel removed = " << ba.isEmpty();

    if (b)
    {
        meta.applyChanges();
    }

    return 0;
}
