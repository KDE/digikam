/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Listing information from database - TAlbum helpers.
 *
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2007-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Mohamed_Anwer  <m_dot_anwer at gmx dot com>
 * Copyright (C) 2018      by Mario Frank    <mario dot frank at uni minus potsdam dot de>
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

#include "itemlister_p.h"

namespace Digikam
{


void ItemLister::listTag(ItemListerReceiver* const receiver,
                         const QList<int>& tagIds)
{
    QSet<ItemListerRecord> records;
    QList<int>::const_iterator it;

    for (it = tagIds.constBegin() ; it != tagIds.constEnd() ; ++it)
    {
        QList<QVariant>         values;
        QMap<QString, QVariant> parameters;
        parameters.insert(QLatin1String(":tagPID"), *it);
        parameters.insert(QLatin1String(":tagID"),  *it);

        CoreDbAccess access;

        if (d->recursive)
        {
            access.backend()->execDBAction(access.backend()->getDBAction(QLatin1String("listTagRecursive")), parameters, &values);
        }
        else
        {
            access.backend()->execDBAction(access.backend()->getDBAction(QLatin1String("listTag")), parameters, &values);
        }

        QSet<int> albumRoots = albumRootsToList();

        int width, height;

        for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ;)
        {
            ItemListerRecord record;

            record.imageID           = (*it).toLongLong();
            ++it;
            record.name              = (*it).toString();
            ++it;
            record.albumID           = (*it).toInt();
            ++it;
            record.albumRootID       = (*it).toInt();
            ++it;
            record.rating            = (*it).toInt();
            ++it;
            record.category          = (DatabaseItem::Category)(*it).toInt();
            ++it;
            record.format            = (*it).toString();
            ++it;
            record.creationDate      = (*it).toDateTime();
            ++it;
            record.modificationDate  = (*it).toDateTime();
            ++it;
            record.fileSize          = d->toInt32BitSafe(it);
            ++it;
            width                    = (*it).toInt();
            ++it;
            height                   = (*it).toInt();
            ++it;

            if (d->listOnlyAvailableImages && !albumRoots.contains(record.albumRootID))
            {
                continue;
            }

            record.imageSize         = QSize(width, height);

            records.insert(record);
        }
    }

    for (QSet<ItemListerRecord>::iterator it = records.begin() ; it != records.end() ; ++it)
    {
        receiver->receive(*it);
    }
}

void ItemLister::listImageTagPropertySearch(ItemListerReceiver* const receiver, const QString& xml)
{
    if (xml.isEmpty())
    {
        return;
    }

    QList<QVariant> boundValues;
    QList<QVariant> values;
    QString sqlQuery;
    QString errMsg;

    // Currently, for optimization, this does not allow a general-purpose search,
    // ImageMetadata and ImagePositions are not joined and hooks are ignored.

    // query head
    sqlQuery = QString::fromUtf8(
               "SELECT DISTINCT Images.id, Images.name, Images.album, "
               "       Albums.albumRoot, "
               "       ImageInformation.rating, Images.category, "
               "       ImageInformation.format, ImageInformation.creationDate, "
               "       Images.modificationDate, Images.fileSize, "
               "       ImageInformation.width,  ImageInformation.height, "
               "       ImageTagProperties.value, ImageTagProperties.property, ImageTagProperties.tagid "
               " FROM Images "
               "       INNER JOIN ImageTagProperties ON ImageTagProperties.imageid=Images.id "
               "       LEFT JOIN ImageInformation ON Images.id=ImageInformation.imageid "
               "       INNER JOIN Albums           ON Albums.id=Images.album "
               "WHERE Images.status=1 AND ( ");

    // query body
    ItemQueryBuilder builder;
    ItemQueryPostHooks hooks;
    builder.setImageTagPropertiesJoined(true); // ImageTagProperties added by INNER JOIN
    sqlQuery += builder.buildQuery(xml, &boundValues, &hooks);
    sqlQuery += QString::fromUtf8(" );");

    qCDebug(DIGIKAM_DATABASE_LOG) << "Search query:\n" << sqlQuery << "\n" << boundValues;

    bool executionSuccess;
    {
        CoreDbAccess access;
        executionSuccess = access.backend()->execSql(sqlQuery, boundValues, &values);

        if (!executionSuccess)
        {
            errMsg = access.backend()->lastError();
        }
    }

    if (!executionSuccess)
    {
        receiver->error(errMsg);
        return;
    }

    qCDebug(DIGIKAM_DATABASE_LOG) << "Search result:" << values.size() / 15;

    QSet<int> albumRoots = albumRootsToList();

    int width, height;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ;)
    {
        ItemListerRecord record(d->allowExtraValues ? ItemListerRecord::ExtraValueFormat : ItemListerRecord::TraditionalFormat);

        record.imageID           = (*it).toLongLong();
        ++it;
        record.name              = (*it).toString();
        ++it;
        record.albumID           = (*it).toInt();
        ++it;
        record.albumRootID       = (*it).toInt();
        ++it;
        record.rating            = (*it).toInt();
        ++it;
        record.category          = (DatabaseItem::Category)(*it).toInt();
        ++it;
        record.format            = (*it).toString();
        ++it;
        record.creationDate      = (*it).toDateTime();
        ++it;
        record.modificationDate  = (*it).toDateTime();
        ++it;
        record.fileSize          = d->toInt32BitSafe(it);
        ++it;
        width                    = (*it).toInt();
        ++it;
        height                   = (*it).toInt();
        ++it;
        // sync the following order with the places where it's read, e.g., FaceTagsIface
        QVariant value           = (*it);
        ++it;
        QVariant property        = (*it);
        ++it;
        QVariant tagId           = (*it);
        ++it;

        // If the property is the autodetected person, get the original image tag properties
        if (property.toString().compare(ImageTagPropertyName::autodetectedPerson()) == 0)
        {
            // If we split the value by ',' we must have the segments tagId, property, region
            // Set the values.
            QStringList values = value.toString().split(QLatin1Char(','));

            if (values.size() == 3)
            {
                value    = values.at(2);
                property = values.at(1);
                tagId    = values.at(0);
            }
        }

        record.extraValues << value;
        record.extraValues << property;
        record.extraValues << tagId;

        if (d->listOnlyAvailableImages && !albumRoots.contains(record.albumRootID))
        {
            continue;
        }

        record.imageSize         = QSize(width, height);

        receiver->receive(record);
    }
}

QString ItemLister::tagSearchXml(int tagId,
                                 const QString& type,
                                 bool includeChildTags) const
{
    if (type == QLatin1String("faces"))
    {
        SearchXmlWriter writer;

        writer.writeGroup();
        writer.setDefaultFieldOperator(SearchXml::Or);

        QStringList properties;
        properties << ImageTagPropertyName::autodetectedPerson();
        properties << ImageTagPropertyName::autodetectedFace();
        properties << ImageTagPropertyName::tagRegion();

        foreach (const QString& property, properties)
        {
            writer.writeField(QLatin1String("imagetagproperty"), includeChildTags ? SearchXml::InTree : SearchXml::Equal);

            if (tagId != -1)
            {
                writer.writeAttribute(QLatin1String("tagid"), QString::number(tagId));
            }

            writer.writeValue(property);
            writer.finishField();
        }

/*
        if (flags & TagAssigned && tagId)
        {
            writer.writeField("tagid", SearchXml::Equal);
            writer.writeValue(tagId);
            writer.finishField();
        }
*/

        writer.finishGroup();

        return writer.xml();
    }
    else
    {
        return QString();
    }
}

} // namespace Digikam
