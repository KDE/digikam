/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : An unit test to load metadata from byte array
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

#include "loadfrombatest.h"

// Qt includes

#include <QFile>
#include <QDataStream>
#include <QImage>
#include <QByteArray>

QTEST_MAIN(LoadFromBATest)

void LoadFromBATest::testLoadFromByteArray()
{
    loadFromByteArray(m_originalImageFolder + QLatin1String("nikon-e2100.jpg"));
}

void LoadFromBATest::loadFromByteArray(const QString& file)
{
    qDebug() << "File to process:" << file;
    bool ret     = false;
    QString path = m_tempDir.filePath(QFileInfo(file).fileName().trimmed());

    qDebug() << "Temporary target file:" << path;

    ret = !path.isNull();
    QVERIFY(ret);

    QFile target(file);
    ret = target.copy(path);
    QVERIFY(ret);

    QString baFile(path + QLatin1String("ba.dat"));

    QImage image(file);
    ret = image.save(baFile, "PNG");
    QVERIFY(ret);

    QFile baf(baFile);
    ret = baf.open(QIODevice::ReadOnly);
    QVERIFY(ret);

    QByteArray data;
    data.resize(baf.size());
    QDataStream stream(&baf);
    ret = stream.readRawData(data.data(), data.size());
    baf.close();
    QVERIFY(ret);

    DMetadata meta;
    ret = meta.loadFromData(data);
    QVERIFY(ret);
}
