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

QTEST_MAIN(SetXmpFaceTest)

void SetXmpFaceTest::testSetXmpFace()
{
    setXmpFace(m_originalImageFolder + QLatin1String("2015-07-22_00001.JPG"));
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
    meta.setWriteRawFiles(true);
    ret = meta.load(filePath);
    QVERIFY(ret);

    bool g       = meta.supportXmp();
    qDebug() << "Exiv2 XMP support" << g;
    
    // Add random rectangles with facetags

    QString name = QLatin1String("Bob Marley");
    float x      = 0.5;
    float y      = 0.5;
    float w      = 60;
    float h      = 60;

    QRectF rect(x, y, w, h);
    QMap<QString, QRectF> faces;

    faces[name]       = rect;

    QString name2     = QLatin1String("Hello Kitty!");
    QRectF rect2(0.4, 0.4, 30,30);

    faces[name2]      = rect2;

    const QString bag = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList");

    ret = setFaceTags(meta, bag.toLatin1().constData(), faces);
    QVERIFY(ret);

    ret = meta.applyChanges();
    QVERIFY(ret);

    // Check back face tags assigned.

    QString recoverName = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Name");

    DMetadata meta2;
    meta2.setWriteRawFiles(true);
    ret = meta2.load(filePath);
    QVERIFY(ret);

    QString nameR = meta2.getXmpTagString(recoverName.toLatin1().constData(), false);
    QVERIFY(!nameR.isEmpty());

    qDebug() << "Saved name is:" << nameR;

    // Now clear factags and check if well removed.

    DMetadata meta3;
    meta3.setWriteRawFiles(true);
    ret = meta3.load(filePath);
    QVERIFY(ret);
    
    ret = removeFaceTags(meta3, bag.toLatin1().constData());
    QVERIFY(ret);

    ret = meta3.applyChanges();
    QVERIFY(ret);

    DMetadata meta4;
    meta4.setWriteRawFiles(true);
    ret = meta4.load(filePath);
    QVERIFY(ret);

    QString nameR2 = meta4.getXmpTagString(recoverName.toLatin1().constData(), false);
    QVERIFY(nameR2.isEmpty());
}

bool SetXmpFaceTest::setFaceTags(DMetadata& meta, const char* xmpTagName, const QMap<QString,QRectF>& faces)
{
    if (!meta.setXmpTagString(xmpTagName, QString(), DMetadata::XmpTagType(1)))
        return false;

    QString qxmpTagName(QString::fromLatin1(xmpTagName));
    QString nameTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Name");
    QString typeTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Type");
    QString areaTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area");
    QString areaxTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:x");
    QString areayTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:y");
    QString areawTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:w");
    QString areahTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:h");
    QString areanormTagKey = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:unit");

    QMap<QString, QRectF>::const_iterator it = faces.constBegin();
    int i                                    = 1;

    while (it != faces.constEnd())
    {
        qreal x, y, w, h;
        it.value().getRect(&x, &y, &w, &h);

        /** Set tag name **/
        if (!meta.setXmpTagString(nameTagKey.arg(i).toLatin1().constData(), it.key(),
                                  DMetadata::XmpTagType(0)))
            return false;

        /** Set tag type as Face **/
        if (!meta.setXmpTagString(typeTagKey.arg(i).toLatin1().constData(), QLatin1String("Face"),
                                  DMetadata::XmpTagType(0)))
            return false;

        /** Set tag Area, with xmp type struct **/
        if (!meta.setXmpTagString(areaTagKey.arg(i).toLatin1().constData(), QString(),
                                  DMetadata::XmpTagType(2)))
            return false;

        /** Set stArea:x inside Area structure **/
        if (!meta.setXmpTagString(areaxTagKey.arg(i).toLatin1().constData(), QString::number(x),
                                  DMetadata::XmpTagType(0)))
            return false;

        /** Set stArea:y inside Area structure **/
        if (!meta.setXmpTagString(areayTagKey.arg(i).toLatin1().constData(), QString::number(y),
                                  DMetadata::XmpTagType(0)))
            return false;

        /** Set stArea:w inside Area structure **/
        if (!meta.setXmpTagString(areawTagKey.arg(i).toLatin1().constData(), QString::number(w),
                                  DMetadata::XmpTagType(0)))
            return false;

        /** Set stArea:h inside Area structure **/
        if (!meta.setXmpTagString(areahTagKey.arg(i).toLatin1().constData(),QString::number(h),
                                  DMetadata::XmpTagType(0)))
            return false;

        /** Set stArea:unit inside Area structure  as normalized **/
        if (!meta.setXmpTagString(areanormTagKey.arg(i).toLatin1().constData(), QLatin1String("normalized"),
                                  DMetadata::XmpTagType(0)))
            return false;

        ++it;
        ++i;
    }

    return true;
}

bool SetXmpFaceTest::removeFaceTags(DMetadata& meta, const char* xmpTagName)
{
    QString qxmpTagName(QString::fromLatin1(xmpTagName));
    QString regionTagKey   = qxmpTagName + QLatin1String("[%1]");
    QString nameTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Name");
    QString typeTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Type");
    QString areaTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area");
    QString areaxTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:x");
    QString areayTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:y");
    QString areawTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:w");
    QString areahTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:h");
    QString areanormTagKey = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:unit");

    if (!meta.removeXmpTag(xmpTagName))
        return false;

    bool dirty = true;
    int i      = 1;

    while (dirty)
    {
        dirty  = false;
        dirty |=meta.removeXmpTag(regionTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(nameTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(typeTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(areaTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(areaxTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(areayTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(areawTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(areahTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(areanormTagKey.arg(i).toLatin1().constData());
        ++i;
    }

    return true;
}
