/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Listing information from database - FAlbum helpers.
 *
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

void ItemLister::listFaces(ItemListerReceiver* const receiver, int personId)
{
    QList<qlonglong> list;
    QList<QVariant>  values;
    CoreDbAccess     access;

    access.backend()->execSql(QString::fromUtf8("SELECT Images.id "
                                                " FROM Images "
                                                "       LEFT JOIN ImageInformation ON Images.id=ImageInformation.imageid "
                                                "       INNER JOIN Albums ON Albums.id=")+
                              QString::number(personId)+
                              QString::fromUtf8(" WHERE Images.status=1 "
                                                " ORDER BY Albums.id;"),
                              &values);

    QListIterator<QVariant> it(values);

    while (it.hasNext())
    {
        TagsCache* const cache = TagsCache::instance();

        ItemTagPair pair(list.last(), cache->tagForPath(QLatin1String("/People/Unknown")));
        QList<QString> nameList = pair.values(QLatin1String("face"));

        // push the image into the list every time a face with the name is found in the image
        int count = nameList.count(cache->tagName(personId));

        for (int i = 0 ; i < count ; ++i)
        {
            list += it.next().toLongLong();
        }
    }

    listFromIdList(receiver, list);
}

void ItemLister::listFromIdList(ItemListerReceiver* const receiver,
                                const QList<qlonglong>& imageIds)
{
    QList<QVariant> values;
    QString         errMsg;
    bool            executionSuccess = true;

    {
/*
        // Unfortunately, we need to convert to QVariant
        QList<QVariant> variantIdList;

        foreach (const qlonglong& id, imageIds)
        {
            variantIdList << id;
        }

        CoreDbAccess access;
        QSqlQuery query = access.backend()->prepareQuery(QString::fromUtf8(
                    "SELECT DISTINCT Images.id, Images.name, Images.album, "
                    "       ImageInformation.rating, ImageInformation.creationDate, "
                    "       Images.modificationDate, Images.fileSize, "
                    "       ImageInformation.width, ImageInformation.height "
                    " FROM Images "
                    "       LEFT JOIN ImageInformation ON Images.id=ImageInformation.imageid "
                    " WHERE Images.id = ?;"));

        query.addBindValue(variantIdList);
        executionSuccess = query.execBatch
*/
        CoreDbAccess access;
        DbEngineSqlQuery query = access.backend()->prepareQuery(QString::fromUtf8(
                             "SELECT DISTINCT Images.id, Images.name, Images.album, "
                             "       Albums.albumRoot, "
                             "       ImageInformation.rating, Images.category, "
                             "       ImageInformation.format, ImageInformation.creationDate, "
                             "       Images.modificationDate, Images.fileSize, "
                             "       ImageInformation.width, ImageInformation.height "
                             " FROM Images "
                             "       LEFT JOIN ImageInformation ON Images.id=ImageInformation.imageid "
                             "       LEFT JOIN Albums ON Albums.id=Images.album "
                             " WHERE Images.status=1 AND Images.id = ?;"));

        foreach (const qlonglong& id, imageIds)
        {
            query.bindValue(0, id);
            executionSuccess = access.backend()->exec(query);

            if (!executionSuccess)
            {
                errMsg = access.backend()->lastError();
                break;
            }

            // append results to list
            values << access.backend()->readToList(query);
        }
    }

    if (!executionSuccess)
    {
        receiver->error(errMsg);
        return;
    }

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

        record.imageSize         = QSize(width, height);

        receiver->receive(record);
    }
}

} // namespace Digikam
