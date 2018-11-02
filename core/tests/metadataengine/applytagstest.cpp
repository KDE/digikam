/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-10-30
 * Description : An unit-test to read metadata and apply tag paths to item with DMetadata.
 *
 * Copyright (C) 2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "applytagstest.h"

// Qt includes

#include <QDebug>
#include <QTest>
#include <QFile>

// Local includes

#include "dmetadata.h"
#include "wstoolutils.h"

QTEST_MAIN(ApplyTagsTest)

QDir          s_tempDir;
QString       s_tempPath;
const QString s_originalImageFolder(QFINDTESTDATA("data/"));

void ApplyTagsTest::initTestCase()
{
    MetaEngine::initializeExiv2();
    qDebug() << "Using Exiv2 Version:" << MetaEngine::Exiv2Version();
    s_tempPath = QString::fromLatin1(QTest::currentAppName());
    s_tempPath.replace(QLatin1String("./"), QString());
}

void ApplyTagsTest::init()
{
    s_tempDir = WSToolUtils::makeTemporaryDir(s_tempPath.toLatin1().data());
}

void ApplyTagsTest::cleanup()
{
    WSToolUtils::removeTemporaryDir(s_tempPath.toLatin1().data());
}

void ApplyTagsTest::cleanupTestCase()
{
    MetaEngine::cleanupExiv2();
}

void ApplyTagsTest::testApplyTagsToMetadata()
{
    MetaEngineSettingsContainer settings;

    // For bug #400436

    settings.metadataWritingMode = DMetadata::WRITE_TO_IMAGE_ONLY;

    applyTags(s_originalImageFolder + QLatin1String("2015-07-22_00001.JPG"),
              QStringList() << QLatin1String("nature"),
              settings,
              true,
              true);

    // For bug #397189
    // The bug is known for Exiv2-0.26, only test the newer versions

    bool ok = true;

    if (MetaEngine::Exiv2Version().section(QLatin1Char('.'), 0, 1).toDouble(&ok) > 0.26 && ok)
    {
        settings.metadataWritingMode = DMetadata::WRITE_TO_IMAGE_ONLY;

        applyTags(s_originalImageFolder + QLatin1String("20160821035715.jpg"),
                  QStringList() << QLatin1String("test"),
                  settings,
                  true,       // NOTE: image is corrupted => no expected crash
                  false);
    }
}

void ApplyTagsTest::applyTags(const QString& file,
                              const QStringList& tags,
                              const MetaEngineSettingsContainer& settings,
                              bool  expectedRead,
                              bool  expectedWrite)
{
    qDebug() << "File to process:" << file;
    bool ret     = false;
    QString path = s_tempDir.filePath(QFileInfo(file).fileName().trimmed());

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
    QCOMPARE(ret, expectedRead);

    meta.setImageTagsPath(tags);
    ret = meta.applyChanges(true);
    QCOMPARE(ret, expectedWrite);

    if (expectedWrite)
    {
        DMetadata meta2;
        meta2.setSettings(settings);
        QStringList newTags;
        ret = meta2.load(path);
        QVERIFY(ret);

        ret = meta2.getImageTagsPath(newTags);
        QVERIFY(ret);

        foreach (const QString& tag, tags)
        {
            ret = newTags.contains(tag);
            QVERIFY(ret);
        }
    }
}
