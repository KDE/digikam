/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-12
 * Description : a command line tool to print all tags list supported by Exiv2
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

#include <QStringList>
#include <QDebug>

// Local includes

#include "dmetadata.h"

using namespace Digikam;

int main (int /*argc*/, char** /*argv*/)
{
    DMetadata meta;

    qDebug() << "-- Standard Exif Tags -------------------------------------------------------------";
    DMetadata::TagsMap exiftags = meta.getStdExifTagsList();

    for (DMetadata::TagsMap::const_iterator it = exiftags.constBegin(); it != exiftags.constEnd(); ++it )
    {
        QString     key    = it.key();
        QStringList values = it.value();
        QString     name   = values[0];
        QString     title  = values[1];
        QString     desc   = values[2];
        qDebug() << key << " :: " << name << " :: " << title << " :: " << desc;
    }

    qDebug() << "-- Makernote Tags -----------------------------------------------------------------";
    DMetadata::TagsMap mntags = meta.getMakernoteTagsList();

    for (DMetadata::TagsMap::const_iterator it = mntags.constBegin(); it != mntags.constEnd(); ++it )
    {
        QString     key    = it.key();
        QStringList values = it.value();
        QString     name   = values[0];
        QString     title  = values[1];
        QString     desc   = values[2];
        qDebug() << key << " :: " << name << " :: " << title << " :: " << desc;
    }

    qDebug() << "-- Standard Iptc Tags -----------------------------------------------------------------";
    DMetadata::TagsMap iptctags = meta.getIptcTagsList();

    for (DMetadata::TagsMap::const_iterator it = iptctags.constBegin(); it != iptctags.constEnd(); ++it )
    {
        QString     key    = it.key();
        QStringList values = it.value();
        QString     name   = values[0];
        QString     title  = values[1];
        QString     desc   = values[2];
        qDebug() << key << " :: " << name << " :: " << title << " :: " << desc;
    }

    qDebug() << "-- Standard Xmp Tags -----------------------------------------------------------------";
    DMetadata::TagsMap xmptags = meta.getXmpTagsList();

    for (DMetadata::TagsMap::const_iterator it = xmptags.constBegin(); it != xmptags.constEnd(); ++it )
    {
        QString     key    = it.key();
        QStringList values = it.value();
        QString     name   = values[0];
        QString     title  = values[1];
        QString     desc   = values[2];
        qDebug() << key << " :: " << name << " :: " << title << " :: " << desc;
    }

    return 0;
}
