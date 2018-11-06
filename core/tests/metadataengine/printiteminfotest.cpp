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
#include <QStringList>

QTEST_MAIN(PrintItemInfoTest)

void PrintItemInfoTest::testPrintItemInfo()
{
    //                                                 Expected tags to found in Comments,    Titles,
    //                                                                           IptcContact, IptcLocation, IptcSubjects,
    //                                                                           PhotoInfo,   VideoInfo,
    //                                                                           XmpKeywords, XmpSubjects,  XmpSubCategories
    printItemInfo(m_originalImageFolder + QLatin1String("nikon-e2100.jpg"),      false,       false,
                                                                                 false,       false,        false,
                                                                                 true,        true,
                                                                                 true,        false,        false);

    printItemInfo(m_originalImageFolder + QLatin1String("_27A1417.CR2"),         false,       false,
                                                                                 false,       false,        false,
                                                                                 true,        true,
                                                                                 false,       false,        false);

    printItemInfo(m_originalImageFolder + QLatin1String("20160821035715.jpg"),   false,       false,
                                                                                 false,       false,        false,
                                                                                 false,       true,
                                                                                 false,       false,        false);

    printItemInfo(m_originalImageFolder + QLatin1String("2015-07-22_00001.JPG"), false,       false,
                                                                                 false,       false,        false,
                                                                                 true,        false,
                                                                                 false,       false,        false);

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

    // Comments and titles
    printComments(meta,         com);
    printTitles(meta,           ttl);

    // Iptc
    printIptcContact(meta,      cnt);
    printIptcLocation(meta,     loc);
    printIptcSubjects(meta,     isb),

    // Media
    printPhotoInfo(meta,        pho);
    printVideoInfo(meta,        vid);

    // Xmp
    printXmpKeywords(meta,      key);
    printXmpSubjects(meta,      xsb);
    printXmpSubCategories(meta, cat);
}

void PrintItemInfoTest::printComments(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- Comments from %1 --------------------------").arg(meta.getFilePath());

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
{
    qDebug() << QString::fromUtf8("-- IptcContact from %1 --").arg(meta.getFilePath());

    IptcCoreContactInfo map = meta.getCreatorContactInfo();
    QCOMPARE(!map.isEmpty(), expected);

    qDebug() << map;
}

void PrintItemInfoTest::printIptcLocation(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- IptcLocation from %1 --").arg(meta.getFilePath());

    IptcCoreLocationInfo map = meta.getIptcCoreLocation();
    QCOMPARE(!map.isEmpty(), expected);

    qDebug() << map;
}

void PrintItemInfoTest::printIptcSubjects(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- IptcSubjects from %1 --").arg(meta.getFilePath());

    QStringList map = meta.getIptcCoreSubjects();
    QCOMPARE(!map.isEmpty(), expected);

    qDebug() << map;
}

void PrintItemInfoTest::printPhotoInfo(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- PhotoInfo from %1 --").arg(meta.getFilePath());

    PhotoInfoContainer map = meta.getPhotographInformation();
    QCOMPARE(!map.isEmpty(), expected);

    qDebug() << map;
}

void PrintItemInfoTest::printVideoInfo(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- VideoInfo from %1 --").arg(meta.getFilePath());

    VideoInfoContainer map = meta.getVideoInformation();
    QCOMPARE(!map.isEmpty(), expected);

    qDebug() << map;
}

void PrintItemInfoTest::printXmpKeywords(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- XmpKeywords from %1 --").arg(meta.getFilePath());

    if (meta.supportXmp())
    {
        QStringList map = meta.getXmpKeywords();
        QCOMPARE(!map.isEmpty(), expected);

        qDebug() << map;
    }
    else
    {
        QWARN("Exiv2 has no XMP support...");
    }
}

void PrintItemInfoTest::printXmpSubjects(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- XmpSubjects from %1 --").arg(meta.getFilePath());

    if (meta.supportXmp())
    {
        QStringList map = meta.getXmpSubjects();
        QCOMPARE(!map.isEmpty(), expected);

        qDebug() << map;
    }
    else
    {
        QWARN("Exiv2 has no XMP support...");
    }
}

void PrintItemInfoTest::printXmpSubCategories(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- XmpSubCategories from %1 --").arg(meta.getFilePath());

    if (meta.supportXmp())
    {
        QStringList map = meta.getXmpSubCategories();
        QCOMPARE(!map.isEmpty(), expected);

        qDebug() << map;
    }
    else
    {
        QWARN("Exiv2 has no XMP support...");
    }
}
