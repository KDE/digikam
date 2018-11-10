/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-27
 * Description : an unit-test to test XMP sidecar creation with DMetadata
 *
 * Copyright (C) 2010      by Jakob Malm <jakob dot malm at gmail dot com>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "createxmpsidecartest.h"

// Qt includes

#include <QFile>

QTEST_MAIN(CreateXmpSidecarTest)

void CreateXmpSidecarTest::testCreateXmpSidecar()
{
    MetaEngineSettingsContainer settings;
    settings.metadataWritingMode = DMetadata::WRITE_TO_SIDECAR_ONLY;

    createXmpSidecar(m_originalImageFolder + QLatin1String("2015-07-22_00001.JPG"), settings);

    createXmpSidecar(m_originalImageFolder + QLatin1String("IMG_2520.CR2"),         settings);
}

void CreateXmpSidecarTest::createXmpSidecar(const QString& file,
                                            const MetaEngineSettingsContainer& settings)
{
    qDebug() << "File to process:          " << file;

    QString path    = m_tempDir.filePath(QFileInfo(file).fileName().trimmed());
    QString pathXmp = path + QLatin1String(".xmp");

    qDebug() << "Temporary target file:    " << path;

    bool ret = !path.isNull();
    QVERIFY(ret);

    // Copy image file in temporary dir.
    QFile::remove(path);
    QFile target(file);
    ret = target.copy(path);
    QVERIFY(ret);

    // Check if no xmp sidecar relevant is present.

    qDebug() << "Temporary XMP target file to create:" << pathXmp;

    ret = !pathXmp.isNull();
    QVERIFY(ret);

    QFile::remove(pathXmp);

    // Export metadata from image to a fresh xmp sidecar.
    DMetadata meta;
    meta.setSettings(settings);
    ret = meta.load(path);
    QVERIFY(ret);

    ret = meta.save(path);
    QVERIFY(ret);

    QFile sidecar(pathXmp);
    ret = sidecar.exists();
    QVERIFY(ret);
    
    qDebug() << "Sidecar" << pathXmp << "size :" << sidecar.size();

    // Check if xmp sidecar are created and can be loaded
    DMetadata meta2;
    ret = meta2.load(pathXmp);
    QVERIFY(ret);
}
