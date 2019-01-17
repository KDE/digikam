/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - faces helpers
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * Copyright (C) 2011      by Leif Huhn <leif at dkstat dot com>
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

#include "dmetadata.h"

// Qt includes

#include <QLocale>

// Local includes

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

bool DMetadata::getItemFacesMap(QMultiMap<QString,QVariant>& faces) const
{
    faces.clear();

    // The example code for Exiv2 says:
    // > There are no specialized values for structures, qualifiers and nested
    // > types. However, these can be added by using an XmpTextValue and a path as
    // > the key.

    // I think that means I have to iterate over the WLPG face tags in the clunky
    // way below (guess numbers and look them up as strings). (Leif)

    const QString personPathTemplate = QLatin1String("Xmp.MP.RegionInfo/MPRI:Regions[%1]/MPReg:PersonDisplayName");
    const QString rectPathTemplate   = QLatin1String("Xmp.MP.RegionInfo/MPRI:Regions[%1]/MPReg:Rectangle");

    for (int i = 1 ; ; ++i)
    {
        QString person = getXmpTagString(personPathTemplate.arg(i).toLatin1().constData(), false);

        if (person.isEmpty())
            break;

        // The WLPG tags have the format X.XX, Y.YY, W.WW, H.HH
        // That is, four decimal numbers ranging from 0-1.
        // The top left position is indicated by X.XX, Y.YY (as a
        // percentage of the width/height of the entire image).
        // Similarly the width and height of the face's box are
        // indicated by W.WW and H.HH.
        QString rectString = getXmpTagString(rectPathTemplate.arg(i).toLatin1().constData(), false);
        QStringList list   = rectString.split(QLatin1Char(','));

        if (list.size() < 4)
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Cannot parse WLPG rectangle string" << rectString;
            continue;
        }

        QRectF rect(list.at(0).toFloat(),
                    list.at(1).toFloat(),
                    list.at(2).toFloat(),
                    list.at(3).toFloat());

        faces.insertMulti(person, rect);
    }

    /** Read face tags only if Exiv2 can write them, otherwise
     *  garbage tags will be generated on image transformation
     */

    // Read face tags as saved by Picasa
    // http://www.exiv2.org/tags-xmp-mwg-rs.html
    const QString mwg_personPathTemplate  = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Name");
    const QString mwg_rect_x_PathTemplate = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Area/stArea:x");
    const QString mwg_rect_y_PathTemplate = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Area/stArea:y");
    const QString mwg_rect_w_PathTemplate = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Area/stArea:w");
    const QString mwg_rect_h_PathTemplate = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Area/stArea:h");

    for (int i = 1 ; ; ++i)
    {
        QString person = getXmpTagString(mwg_personPathTemplate.arg(i).toLatin1().constData(), false);

        if (person.isEmpty())
            break;

        // x and y is the center point
        float x = getXmpTagString(mwg_rect_x_PathTemplate.arg(i).toLatin1().constData(), false).toFloat();
        float y = getXmpTagString(mwg_rect_y_PathTemplate.arg(i).toLatin1().constData(), false).toFloat();
        float w = getXmpTagString(mwg_rect_w_PathTemplate.arg(i).toLatin1().constData(), false).toFloat();
        float h = getXmpTagString(mwg_rect_h_PathTemplate.arg(i).toLatin1().constData(), false).toFloat();
        QRectF rect(x - w/2,
                    y - h/2,
                    w,
                    h);

        faces.insertMulti(person, rect);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Found new rect " << person << " "<< rect;
    }

    return !faces.isEmpty();
}

bool DMetadata::setItemFacesMap(QMultiMap<QString, QVariant>& facesPath, bool write) const
{
    QString qxmpTagName    = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList");
    QString nameTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Name");
    QString typeTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Type");
    QString areaTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area");
    QString areaxTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:x");
    QString areayTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:y");
    QString areawTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:w");
    QString areahTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:h");
    QString areanormTagKey = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:unit");

    QString winQxmpTagName = QLatin1String("Xmp.MP.RegionInfo/MPRI:Regions");
    QString winRectTagKey  = winQxmpTagName + QLatin1String("[%1]/MPReg:Rectangle");
    QString winNameTagKey  = winQxmpTagName + QLatin1String("[%1]/MPReg:PersonDisplayName");

    if (!write)
    {
        QString check = getXmpTagString(nameTagKey.arg(1).toLatin1().constData());

        if (check.isEmpty())
            return true;
    }

    setXmpTagString(qxmpTagName.toLatin1().constData(),
                    QString(),
                    MetaEngine::ArrayBagTag);

    setXmpTagString(winQxmpTagName.toLatin1().constData(),
                    QString(),
                    MetaEngine::ArrayBagTag);

    QMap<QString, QVariant>::const_iterator it = facesPath.constBegin();
    int i                                      = 1;
    bool ok                                    = true;

    while (it != facesPath.constEnd())
    {
        qreal x, y, w, h;
        it.value().toRectF().getRect(&x, &y, &w, &h);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Set face region:" << x << y << w << h;

        // Write face tags in Windows Live Photo format

        QString rectString;

        rectString.append(QString::number(x) + QLatin1String(", "));
        rectString.append(QString::number(y) + QLatin1String(", "));
        rectString.append(QString::number(w) + QLatin1String(", "));
        rectString.append(QString::number(h));

        // Set tag rect
        setXmpTagString(winRectTagKey.arg(i).toLatin1().constData(),
                        rectString,
                        MetaEngine::NormalTag);

        // Set tag name
        setXmpTagString(winNameTagKey.arg(i).toLatin1().constData(),
                        it.key(),
                        MetaEngine::NormalTag);

        // Writing rectangle in Metadata Group format
        x += w / 2;
        y += h / 2;

        // Set tag name
        ok &= setXmpTagString(nameTagKey.arg(i).toLatin1().constData(),
                              it.key(),
                              MetaEngine::NormalTag);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set tag name:" << ok;

        // Set tag type as Face
        ok &= setXmpTagString(typeTagKey.arg(i).toLatin1().constData(),
                              QLatin1String("Face"),
                              MetaEngine::NormalTag);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set tag type:" << ok;

        // Set tag Area, with xmp type struct
        ok &= setXmpTagString(areaTagKey.arg(i).toLatin1().constData(),
                              QString(),
                              MetaEngine::StructureTag);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set area struct:" << ok;

        // Set stArea:x inside Area structure
        ok &= setXmpTagString(areaxTagKey.arg(i).toLatin1().constData(),
                              QString::number(x),
                              MetaEngine::NormalTag);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set xpos:" << ok;

        // Set stArea:y inside Area structure
        ok &= setXmpTagString(areayTagKey.arg(i).toLatin1().constData(),
                              QString::number(y),
                              MetaEngine::NormalTag);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set ypos:" << ok;

        // Set stArea:w inside Area structure
        ok &= setXmpTagString(areawTagKey.arg(i).toLatin1().constData(),
                              QString::number(w),
                              MetaEngine::NormalTag);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set width:" << ok;

        // Set stArea:h inside Area structure
        ok &= setXmpTagString(areahTagKey.arg(i).toLatin1().constData(),
                              QString::number(h),
                              MetaEngine::NormalTag);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set heigh:" << ok;

        // Set stArea:unit inside Area structure  as normalized
        ok &= setXmpTagString(areanormTagKey.arg(i).toLatin1().constData(),
                              QLatin1String("normalized"),
                              MetaEngine::NormalTag);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set unit:" << ok;

        ++it;
        ++i;
    }

    return ok;
}

void DMetadata::removeItemFacesMap()
{
    QString qxmpStructName    = QLatin1String("Xmp.mwg-rs.Regions");
    QString qxmpTagName       = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList");

    QString winQxmpStructName = QLatin1String("Xmp.MP.RegionInfo");
    QString winQxmpTagName    = QLatin1String("Xmp.MP.RegionInfo/MPRI:Regions");

    // Remove mwg-rs tags

    setXmpTagString(qxmpTagName.toLatin1().constData(),
                    QString(),
                    MetaEngine::ArrayBagTag);

    setXmpTagString(qxmpStructName.toLatin1().constData(),
                    QString(),
                    MetaEngine::StructureTag);

    // Remove MP tags

    setXmpTagString(winQxmpTagName.toLatin1().constData(),
                    QString(),
                    MetaEngine::ArrayBagTag);

    setXmpTagString(winQxmpStructName.toLatin1().constData(),
                    QString(),
                    MetaEngine::StructureTag);
}

} // namespace Digikam
