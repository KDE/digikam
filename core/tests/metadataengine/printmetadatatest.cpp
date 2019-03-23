/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : An unit-test to print metadata tags from file using DMetadata.
 *
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "printmetadatatest.h"

// Qt includes

#include <QTextStream>

QTEST_MAIN(PrintMetadataTest)

void PrintMetadataTest::printMetadataMap(const DMetadata::MetaDataMap& map)
{
    QString output;
    QTextStream stream(&output);
    stream << endl;

    qDebug() << "Found" << map.size() << "tags:" << endl;

    for (DMetadata::MetaDataMap::const_iterator it = map.constBegin() ;
         it != map.constEnd() ; ++it)
    {
        QString key   = it.key();
        QString value = it.value();

        // None of these strings can be null, event if strings are translated.
        QVERIFY(!key.isNull());
        QVERIFY(!value.isNull());

        QString tagName = key.simplified();
        tagName.append(QString().fill(QLatin1Char(' '), 48 - tagName.length()));

        QString tagVal  = value.simplified();

        if (tagVal.length() > 48)
        {
            tagVal.truncate(48);
            tagVal.append(QString::fromLatin1("... (%1 bytes)").arg(value.length()));
        }

        stream << tagName << " : " << tagVal << endl;
    }

    qDebug().noquote() << output;
}

void PrintMetadataTest::testPrintMetadata()
{
    //                                                   Expected tags to found in Exif,  Iptc,  Xmp,   expectedRead
    printMetadata(m_originalImageFolder + QLatin1String("nikon-e2100.jpg"),        true,  true,  true,  true);
    printMetadata(m_originalImageFolder + QLatin1String("_27A1417.CR2"),           true,  false, true,  true);
    printMetadata(m_originalImageFolder + QLatin1String("2008-05_DSC_0294.JPG"),   true,  true,  true,  true);

    // The file cannot be loaded with Exiv2-0.26, only test the newer versions

    bool ok = true;

    if ((MetaEngine::Exiv2Version().section(QLatin1Char('.'), 0, 1).toDouble(&ok) > 0.26) && ok)
    {
        printMetadata(m_originalImageFolder + QLatin1String("20160821035715.jpg"), false, false, false, false);
    }
}

void PrintMetadataTest::printMetadata(const QString& filePath, bool exif, bool iptc, bool xmp, bool expectedRead)
{
    DMetadata meta;

    bool ret = meta.load(filePath);
    QCOMPARE(ret, expectedRead);

    loadExif(meta, exif);
    loadIptc(meta, iptc);
    loadXmp(meta,  xmp);
}

void PrintMetadataTest::loadExif(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- Exif metadata from %1 --").arg(meta.getFilePath());

    DMetadata::MetaDataMap map = meta.getExifTagsDataList();
    QCOMPARE(!map.isEmpty(), expected);

    printMetadataMap(map);
}

void PrintMetadataTest::loadIptc(const DMetadata& meta, bool expected)
{
    qDebug() << QString::fromUtf8("-- Iptc metadata from %1 --").arg(meta.getFilePath());

    DMetadata::MetaDataMap map = meta.getIptcTagsDataList();
    QCOMPARE(!map.isEmpty(), expected);

    printMetadataMap(map);
}

void PrintMetadataTest::loadXmp(const DMetadata& meta, bool expected)
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
