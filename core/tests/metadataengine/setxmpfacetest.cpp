/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-21
 * Description : an unit-test to set and clear faces in Picassa format with DMetadata
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "setxmpfacetest.h"

// Qt includes

#include <QFile>
#include <QMultiMap>
#include <QRectF>

QTEST_MAIN(SetXmpFaceTest)

void SetXmpFaceTest::testSetXmpFace()
{
    setXmpFace(m_originalImageFolder + QLatin1String("nikon-e2100.jpg"));
}

void SetXmpFaceTest::setXmpFace(const QString& file)
{
    qDebug() << "File to process:          " << file;

    QString filePath = m_tempDir.filePath(QFileInfo(file).fileName().trimmed());

    qDebug() << "Temporary target file:    " << filePath;

    bool ret = !filePath.isNull();
    QVERIFY(ret);

    // Copy image file in temporary dir.
    QFile::remove(filePath);
    QFile target(file);
    ret = target.copy(filePath);
    QVERIFY(ret);

    DMetadata meta;
    ret = meta.load(filePath);
    QVERIFY(ret);

    qDebug() << "Add region with face tags in file...";

    QMultiMap<QString, QVariant> faces;

    QString name  = QLatin1String("Bob Marley");
    QRectF rect(10, 100, 60, 80);
    faces.insert(name, QVariant(rect));

    QString name2 = QLatin1String("Alice in wonderland");
    QRectF rect2(20, 40, 90, 70);
    faces.insert(name2, QVariant(rect2));

    ret = meta.setItemFacesMap(faces, true);
    QVERIFY(ret);

    ret = meta.applyChanges();
    QVERIFY(ret);

    qDebug() << "Check if face tags are well assigned in file...";

    DMetadata meta2;
    ret = meta2.load(filePath);
    QVERIFY(ret);

    QMultiMap<QString, QVariant> faces2;
    ret = meta2.getItemFacesMap(faces2);
    QVERIFY(ret);

    QVERIFY(!faces2.isEmpty());
    QVERIFY(faces2.contains(name));
    QVERIFY(faces2.contains(name2));
    QVERIFY(faces2.value(name)  == rect);
    QVERIFY(faces2.value(name2) == rect2);
    ret = meta2.applyChanges();
    QVERIFY(ret);

    qDebug() << "Clear face tags from file...";

    DMetadata meta3;
    ret = meta3.load(filePath);
    QVERIFY(ret);

    meta3.removeItemFacesMap();
    ret = meta3.applyChanges();
    QVERIFY(ret);

    qDebug() << "Check if face tags are well removed from file...";

    DMetadata meta4;
    ret = meta4.load(filePath);
    QVERIFY(ret);

    QMultiMap<QString, QVariant> faces4;
    ret = meta4.getItemFacesMap(faces4);
    QVERIFY(!ret);   // Empty map must be returned
}
