/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-10-25
 * Description : Extract preview and patch with DMetadata.
 *               This stage is used by Export tools.
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

#include "patchpreviewtest.h"

// Qt includes

#include <QDebug>
#include <QTest>
#include <QImage>

// Local includes

#include "dmetadata.h"
#include "wstoolutils.h"
#include "previewloadthread.h"

QTEST_MAIN(PatchPreviewTest)

using namespace Digikam;

const QString originalImageFolder(QFINDTESTDATA("data/"));

void PatchPreviewTest::initTestCase()
{
    MetaEngine::initializeExiv2();
}

void PatchPreviewTest::testExtractPreviewAndFixMetadata()
{
    patchPreview(originalImageFolder + QLatin1String("IMG_2520.CR2"), true, 1024, 100);
}

void PatchPreviewTest::cleanupTestCase()
{
    MetaEngine::cleanupExiv2();
}

void PatchPreviewTest::patchPreview(const QString& file, bool rescale, int maxDim, int imageQuality)
{
    qDebug() << "File to process:" << file;
    bool ret     = false;
    QImage image = PreviewLoadThread::loadHighQualitySynchronously(file).copyQImage();

    if (image.isNull())
    {
        image.load(file);
    }

    ret = image.isNull();
    QVERIFY(!ret);

    QString path = WSToolUtils::makeTemporaryDir("patchpreviewtest").filePath(QFileInfo(file)
                                                 .baseName().trimmed() + QLatin1String(".jpg"));

    ret = path.isNull();
    QVERIFY(!ret);

    int imgQualityToApply = 100;

    if (rescale)
    {
        if (image.width() > maxDim || image.height() > maxDim)
            image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        imgQualityToApply = imageQuality;
    }

    ret = image.save(path, "JPEG", imgQualityToApply);
    QVERIFY(ret);

    DMetadata meta;
    ret = meta.load(file);
    QVERIFY(ret);

    meta.setImageDimensions(image.size());
    meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);
    meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
    ret = meta.save(path, true);
    QVERIFY(ret);
}
