/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-12
 * Description : An unit-test to print all available metadata tags provided by Exiv2.
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

#include "printtagslisttest.h"

QTEST_MAIN(PrintTagsListTest)

void PrintTagsListTest::testPrintAllAvailableStdExifTags()
{
    DMetadata meta;

    qDebug() << "-- Standard Exif Tags -------------------------------------------------------------";

    DMetadata::TagsMap exiftags = meta.getStdExifTagsList();
    QVERIFY(!exiftags.isEmpty());

    qDebug() << "Found" << exiftags.size() << "tags:";

    for (DMetadata::TagsMap::const_iterator it = exiftags.constBegin(); it != exiftags.constEnd(); ++it )
    {
        QString     key    = it.key();
        QStringList values = it.value();
        QString     name   = values[0];
        QString     title  = values[1];
        QString     desc   = values[2];
        qDebug() << key << " :: " << name << " :: " << title << " :: " << desc;
    }
}

void PrintTagsListTest::testPrintAllAvailableMakernotesTags()
{
    DMetadata meta;
    
    qDebug() << "-- Makernote Tags -----------------------------------------------------------------";
    
    DMetadata::TagsMap mntags = meta.getMakernoteTagsList();

    QVERIFY(!mntags.isEmpty());

    qDebug() << "Found" << mntags.size() << "tags:";

    for (DMetadata::TagsMap::const_iterator it = mntags.constBegin(); it != mntags.constEnd(); ++it )
    {
        QString     key    = it.key();
        QStringList values = it.value();
        QString     name   = values[0];
        QString     title  = values[1];
        QString     desc   = values[2];
        qDebug() << key << " :: " << name << " :: " << title << " :: " << desc;
    }
}

void PrintTagsListTest::testPrintAllAvailableIptcTags()
{
    DMetadata meta;
    
    qDebug() << "-- Standard Iptc Tags -----------------------------------------------------------------";

    DMetadata::TagsMap iptctags = meta.getIptcTagsList();

    QVERIFY(!iptctags.isEmpty());

    qDebug() << "Found" << iptctags.size() << "tags:";
    
    for (DMetadata::TagsMap::const_iterator it = iptctags.constBegin(); it != iptctags.constEnd(); ++it )
    {
        QString     key    = it.key();
        QStringList values = it.value();
        QString     name   = values[0];
        QString     title  = values[1];
        QString     desc   = values[2];
        qDebug() << key << " :: " << name << " :: " << title << " :: " << desc;
    }
}

void PrintTagsListTest::testPrintAllAvailableXmpTags()
{
    DMetadata meta;
    
    qDebug() << "-- Standard Xmp Tags -----------------------------------------------------------------";

    DMetadata::TagsMap xmptags = meta.getXmpTagsList();

    if (meta.supportXmp())
    {
        QVERIFY(!xmptags.isEmpty());

        qDebug() << "Found" << xmptags.size() << "tags:";

        for (DMetadata::TagsMap::const_iterator it = xmptags.constBegin(); it != xmptags.constEnd(); ++it )
        {
            QString     key    = it.key();
            QStringList values = it.value();
            QString     name   = values[0];
            QString     title  = values[1];
            QString     desc   = values[2];
            qDebug() << key << " :: " << name << " :: " << title << " :: " << desc;
        }
    }
    else
    {
        QWARN("Exiv2 has no XMP support...");
    }
}
