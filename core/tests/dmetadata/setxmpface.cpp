/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-21
 * Description : a command line tool to set faces in Picassa format
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

// Qt includes

#include <QString>
#include <QFile>
#include <QDebug>

// Local includes

#include "dmetadata.h"

using namespace Digikam;

bool setFaceTags(DMetadata& meta, const char* xmpTagName, const QMap<QString,QRectF>& faces)
{
        meta.setXmpTagString(xmpTagName,QString(),DMetadata::XmpTagType(1));

        QString qxmpTagName(QString::fromLatin1(xmpTagName));
        QString nameTagKey     = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Name");
        QString typeTagKey     = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Type");
        QString areaTagKey     = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Area");
        QString areaxTagKey    = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Area/stArea:x");
        QString areayTagKey    = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Area/stArea:y");
        QString areawTagKey    = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Area/stArea:w");
        QString areahTagKey    = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Area/stArea:h");
        QString areanormTagKey = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Area/stArea:unit");

        QMap<QString,QRectF>::const_iterator it = faces.constBegin();
        int i =1;

        while(it != faces.constEnd())
        {
            qreal x,y,w,h;
            it.value().getRect(&x,&y,&w,&h);
            /** Set tag name **/
            meta.setXmpTagString(nameTagKey.arg(i).toLatin1().constData(),it.key(),
                                 DMetadata::XmpTagType(0));
            /** Set tag type as Face **/
            meta.setXmpTagString(typeTagKey.arg(i).toLatin1().constData(),QString::fromLatin1("Face"),
                                 DMetadata::XmpTagType(0));
            /** Set tag Area, with xmp type struct **/
            meta.setXmpTagString(areaTagKey.arg(i).toLatin1().constData(),QString(),
                                 DMetadata::XmpTagType(2));
            /** Set stArea:x inside Area structure **/
            meta.setXmpTagString(areaxTagKey.arg(i).toLatin1().constData(),QString::number(x),
                                 DMetadata::XmpTagType(0));
            /** Set stArea:y inside Area structure **/
            meta.setXmpTagString(areayTagKey.arg(i).toLatin1().constData(),QString::number(y),
                                 DMetadata::XmpTagType(0));
            /** Set stArea:w inside Area structure **/
            meta.setXmpTagString(areawTagKey.arg(i).toLatin1().constData(),QString::number(w),
                                 DMetadata::XmpTagType(0));
            /** Set stArea:h inside Area structure **/
            meta.setXmpTagString(areahTagKey.arg(i).toLatin1().constData(),QString::number(h),
                                 DMetadata::XmpTagType(0));
            /** Set stArea:unit inside Area structure  as normalized **/
            meta.setXmpTagString(areanormTagKey.arg(i).toLatin1().constData(),QString::fromLatin1("normalized"),
                                 DMetadata::XmpTagType(0));

            ++it;
            ++i;
        }

    return true;
}

void removeFaceTags(DMetadata& meta,const char* xmpTagName)
{
    QString qxmpTagName(QString::fromLatin1(xmpTagName));
    QString regionTagKey   = qxmpTagName + QString::fromLatin1("[%1]");
    QString nameTagKey     = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Name");
    QString typeTagKey     = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Type");
    QString areaTagKey     = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Area");
    QString areaxTagKey    = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Area/stArea:x");
    QString areayTagKey    = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Area/stArea:y");
    QString areawTagKey    = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Area/stArea:w");
    QString areahTagKey    = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Area/stArea:h");
    QString areanormTagKey = qxmpTagName + QString::fromLatin1("[%1]/mwg-rs:Area/stArea:unit");

    meta.removeXmpTag(xmpTagName);
    bool dirty = true;
    int i      =1;

    while (dirty)
    {
        dirty = false;
        dirty |=meta.removeXmpTag(regionTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(nameTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(typeTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(areaTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(areaxTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(areayTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(areawTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(areahTagKey.arg(i).toLatin1().constData());
        dirty |=meta.removeXmpTag(areanormTagKey.arg(i).toLatin1().constData());
        i++;
    }
}

int main (int argc, char **argv)
{
    if (argc != 3)
    {
        qDebug() << "Adding a face rectangle to image";
        qDebug() << "Usage: <add/remove> <image>";
        return -1;
    }

    QString filePath(QString::fromLatin1(argv[2]));

    DMetadata::initializeExiv2();
    DMetadata meta;
    meta.load(filePath);
    meta.setWriteRawFiles(true);

    /** Add a random rectangle with facetag Bob **/
    QString name = QString::fromLatin1("Bob Marley");
    float x      = 0.5;
    float y      = 0.5;
    float w      = 60;
    float h      = 60;

    QRectF rect(x,y,w,h);

    QMap<QString, QRectF> faces;

    faces[name] = rect;

    QString name2 = QString::fromLatin1("Hello Kitty!");
    QRectF rect2(0.4, 0.4, 30,30);

    faces[name2] = rect2;

    bool g = meta.supportXmp();

    qDebug() << "Image support XMP" << g;

    const QString bag = QString::fromLatin1("Xmp.mwg-rs.Regions/mwg-rs:RegionList");

    QString op(QString::fromLatin1(argv[1]));

    if (op == QString::fromLatin1("add"))
        setFaceTags(meta, bag.toLatin1().constData(), faces);
    else
        removeFaceTags(meta, bag.toLatin1().constData());

    meta.applyChanges();

    QString recoverName = QString::fromLatin1("Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Name");

    DMetadata meta2;
    meta2.load(filePath);
    meta2.setWriteRawFiles(true);

    QString nameR = meta2.getXmpTagString(recoverName.toLatin1().constData(), false);

    qDebug() << "Saved name is:" << nameR;

    DMetadata::cleanupExiv2();
    return 0;
}
