/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : an unit-test tool to erase metadata tag from photo with DMetadata
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

#include "erasemetadatatagtest.h"

// Qt includes

#include <QFile>

QTEST_MAIN(EraseMetadataTagTest)

void EraseMetadataTagTest::testEraseMetadataTag()
{
    MetaEngineSettingsContainer settings;

    settings.metadataWritingMode = DMetadata::WRITE_TO_IMAGE_ONLY;
    eraseMetadataTag(m_originalImageFolder + QLatin1String("2008-05_DSC_0294.JPG"),
                     QLatin1String("Exif.Nikon3.RetouchHistory"),
                     settings);
}

void EraseMetadataTagTest::eraseMetadataTag(const QString& file,
                                            const QString& metadataTag,
                                            const MetaEngineSettingsContainer& settings)
{
    qDebug() << "File to process:" << file;
    bool ret     = false;
    QString path = m_tempDir.filePath(QFileInfo(file).fileName().trimmed());

    qDebug() << "Temporary target file:" << path;

    ret = !path.isNull();
    QVERIFY(ret);

    QFile::remove(path);
    QFile target(file);
    ret = target.copy(path);
    QVERIFY(ret);

    DMetadata meta;
    meta.setSettings(settings);
    ret = meta.load(path);
    QVERIFY(ret);

    // Check if tag contents exits and drop it.
    QByteArray ba;

    if (metadataTag.startsWith(QLatin1String("Exif")))
    {
        ba = meta.getExifTagData(metadataTag.toLatin1().data());
        QVERIFY(!ba.isNull());
        qDebug() << metadataTag << "found (" << ba.size() << "bytes)";

        ret = meta.removeExifTag(metadataTag.toLatin1().data());
        QVERIFY(ret);
    }
    else if (metadataTag.startsWith(QLatin1String("Iptc")))
    {
        ba = meta.getIptcTagData(metadataTag.toLatin1().data());
        QVERIFY(!ba.isNull());
        qDebug() << metadataTag << "found (" << ba.size() << "bytes)";

        ret = meta.removeIptcTag(metadataTag.toLatin1().data());
        QVERIFY(ret);
    }
    else if (metadataTag.startsWith(QLatin1String("Xmp")))
    {
        ba = meta.getXmpTagString(metadataTag.toLatin1().data()).toLatin1();
        QVERIFY(!ba.isNull());
        qDebug() << metadataTag << "found (" << ba.size() << "bytes)";

        ret = meta.removeXmpTag(metadataTag.toLatin1().data());
        QVERIFY(ret);
    }
    else
    {
        QFAIL(QString::fromLatin1("Metadata Tag not supported (%1)").arg(metadataTag).toLatin1().data());
    }

    ret = meta.applyChanges();
    QVERIFY(ret);

    // Re-read and check if really null

    DMetadata meta2;
    meta2.setSettings(settings);
    ret = meta2.load(path);
    QVERIFY(ret);

    if (metadataTag.startsWith(QLatin1String("Exif")))
    {
        ba = meta2.getExifTagData(metadataTag.toLatin1().data());
        QVERIFY(ba.isNull());
    }
    else if (metadataTag.startsWith(QLatin1String("Iptc")))
    {
        ba = meta2.getIptcTagData(metadataTag.toLatin1().data());
        QVERIFY(ba.isNull());
    }
    else if (metadataTag.startsWith(QLatin1String("Xmp")))
    {
        ba = meta2.getXmpTagString(metadataTag.toLatin1().data()).toLatin1();
        QVERIFY(ba.isNull());
    }
    else
    {
        QFAIL(QString::fromLatin1("Metadata Tag not supported (%1)").arg(metadataTag).toLatin1().data());
    }
}
