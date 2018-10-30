/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-10-30
 * Description : Apply tag paths to item with DMetadata.
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
#include "previewloadthread.h"

QTEST_MAIN(ApplyTagsTest)

using namespace Digikam;

const QString originalImageFolder(QFINDTESTDATA("data/"));

void ApplyTagsTest::initTestCase()
{
    MetaEngine::initializeExiv2();
}

void ApplyTagsTest::testApplyTagsToMetadata()
{
    applyTags(originalImageFolder + QLatin1String("2015-07-22_00001.JPG"), QStringList() << QLatin1String("nature")); // See bug #400436
}

void ApplyTagsTest::cleanupTestCase()
{
    MetaEngine::cleanupExiv2();
}

void ApplyTagsTest::applyTags(const QString& file, const QStringList& tags)
{
    qDebug() << "File to process:" << file;
    bool ret     = false;

    QString path = WSToolUtils::makeTemporaryDir("applytagstest").filePath(QFileInfo(file)
                                                 .baseName().trimmed() + QLatin1String(".jpg"));

    qDebug() << "Temporary target file:" << path;

    ret = !path.isNull();
    QVERIFY(ret);

    QFile target(file);
    ret = target.copy(path);
    QVERIFY(ret);

    DMetadata meta;
    ret = meta.load(path);
    QVERIFY(ret);

    meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
    meta.setImageTagsPath(tags);
    ret = meta.applyChanges(true);
    QVERIFY(ret);

    DMetadata meta2;
    QStringList newTags;
    ret = meta2.load(path);
    QVERIFY(ret);

    meta2.getImageTagsPath(newTags);

    foreach (const QString& tag, tags)
    {
        ret = newTags.contains(tag);
        QVERIFY(ret);
    }

    WSToolUtils::removeTemporaryDir("applytagstest");
}
