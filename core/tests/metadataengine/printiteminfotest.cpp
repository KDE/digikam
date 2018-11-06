/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : An unit-test to print item info from file using DMetadata.
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

#include "printiteminfotest.h"

// Qt includes

#include <QTextStream>

QTEST_MAIN(PrintItemInfoTest)

void PrintItemInfoTest::testPrintItemInfo()
{
    //                                                Expected tags to  found in Exif,  Iptc,  Xmp
    printMetadata(m_originalImageFolder + QLatin1String("nikon-e2100.jpg"),      true,  true,  true);
    printMetadata(m_originalImageFolder + QLatin1String("_27A1417.CR2"),         true,  false, true);
    printMetadata(m_originalImageFolder + QLatin1String("20160821035715.jpg"),   true,  false, true);
    printMetadata(m_originalImageFolder + QLatin1String("2015-07-22_00001.JPG"), true,  false, false);
}

void PrintItemInfoTest::printItemInfo(const QString& filePath,
                                      bool com, bool ttl,            // Comments and titles
                                      bool cnt, bool loc, bool isb,  // Iptc
                                      bool pho, bool vid,            // Media
                                      bool key, bool xsb, bool cat   // Xmp
                                     )
{
    DMetadata meta;

    bool ret = meta.load(filePath);
    QVERIFY(ret);

    printComments(meta, com);
    printTitles(meta, iptc);
    printIptcContact(meta, xmp);
    printIptcLocation(meta,
    printIptcSubjects(meta,

    printPhotoInfo(meta,
    printVideoInfo(meta,

    printXmpKeywords(meta,
    printXmpSubjects(meta,
    printXmpSubCategories(meta,
}

void PrintItemInfoTest::printComments(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- Comments from %1 --").arg(meta.getFilePath());

    CaptionsMap map = meta.getItemComments();
    QCOMPARE(!map.isEmpty(), expected);

    qDebug() << map;
}

void PrintItemInfoTest::printTitles(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- Titles from %1 --").arg(meta.getFilePath());

    CaptionsMap map = meta.getImageTitles();
    QCOMPARE(!map.isEmpty(), expected);

    qDebug() << map;
}

void PrintItemInfoTest::printIptcContact(const DMetadata& meta, bool expected)
void PrintItemInfoTest::printIptcLocation(const DMetadata& meta, bool expected)
void PrintItemInfoTest::printIptcSubjects(const DMetadata& meta, bool expected)

void PrintItemInfoTest::printPhotoInfo(const DMetadata& meta, bool expected)
void PrintItemInfoTest::printVideoInfo(const DMetadata& meta, bool expected)

void PrintItemInfoTest::printXmpKeywords(const DMetadata& meta, bool expected)
void PrintItemInfoTest::printXmpSubjects(const DMetadata& meta, bool expected)
void PrintItemInfoTest::printXmpSubCategories(const DMetadata& meta, bool expected)


void PrintItemInfoTest::loadExif(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- Exif metadata from %1 --").arg(meta.getFilePath());

    DMetadata::MetaDataMap map = meta.getExifTagsDataList();
    QCOMPARE(!map.isEmpty(), expected);

    printMetadataMap(map);
}

void PrintItemInfoTest::loadIptc(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- Iptc metadata from %1 --").arg(meta.getFilePath());

    DMetadata::MetaDataMap map = meta.getIptcTagsDataList();
    QCOMPARE(!map.isEmpty(), expected);

    printMetadataMap(map);
}

void PrintItemInfoTest::loadXmp(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- Xmp metadata from %1 --").arg(meta.getFilePath());

    if (meta.supportXmp())
    {
        DMetadata::MetaDataMap map = meta.getXmpTagsDataList();
        QCOMPARE(!map.isEmpty(), expected);

        printMetadataMap(map);
    }
    else
    {
        QWARN("Exiv2 has no XMP support...");
    }
}
