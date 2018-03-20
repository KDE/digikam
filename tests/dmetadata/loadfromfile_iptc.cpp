/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : a command line tool to load all IPTC metadata from file
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

#include <QString>
#include <QDebug>
#include <QApplication>

// Local includes

#include "dmetadata.h"

using namespace Digikam;

int main (int argc, char** argv)
{
    QApplication app(argc, argv);

    if (argc != 2)
    {
        qDebug() << "loadfromfile_iptc - load and print all iptc metadata from file";
        qDebug() << "Usage: <file>";
        return -1;
    }

    QString filePath = QString::fromLocal8Bit(argv[1]);

    DMetadata meta;
    meta.load(filePath);

    DMetadata::MetaDataMap map = meta.getIptcTagsDataList();

    if (map.isEmpty())
    {
        qDebug() << "No metadata to show...";
    }
    else
    {
        for (DMetadata::MetaDataMap::const_iterator it = map.constBegin();
             it != map.constEnd() ; ++it)
        {
            qDebug() << it.key() << "::" << it.value();
        }
    }

    return 0;
}
