/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Listing information from database - PAlbum helpers.
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

void ItemLister::listPAlbum(ItemListerReceiver* const receiver,
                            int albumRootId,
                            const QString& album)
{
    if (d->listOnlyAvailableImages)
    {
        if (!CollectionManager::instance()->locationForAlbumRootId(albumRootId).isAvailable())
        {
            return;
        }
    }

    QList<QVariant> albumIds;

    if (d->recursive)
    {
        QList<int> intAlbumIds = CoreDbAccess().db()->getAlbumAndSubalbumsForPath(albumRootId, album);

        if (intAlbumIds.isEmpty())
        {
            return;
        }

        foreach (int id, intAlbumIds)
        {
            albumIds << id;
        }
    }
    else
    {
        int albumId = CoreDbAccess().db()->getAlbumForPath(albumRootId, album, false);

        if (albumId == -1)
        {
            return;
        }

        albumIds << albumId;
    }

    QList<QVariant> values;

    QString query = QString::fromUtf8("SELECT DISTINCT Images.id, Images.name, Images.album, "
                    "       ImageInformation.rating, Images.category, "
                    "       ImageInformation.format, ImageInformation.creationDate, "
                    "       Images.modificationDate, Images.fileSize, "
                    "       ImageInformation.width, ImageInformation.height "
                    " FROM Images "
                    "       LEFT JOIN ImageInformation ON Images.id=ImageInformation.imageid "
                    " WHERE Images.status=1 AND ");

    if (d->recursive)
    {
        // SQLite allows no more than 999 parameters
        const int maxParams = CoreDbAccess().backend()->maximumBoundValues();

        for (int i = 0 ; i < albumIds.size() ; ++i)
        {
            QString q           = query;
            QList<QVariant> ids =  (albumIds.size() <= maxParams) ? albumIds : albumIds.mid(i, maxParams);
            i                  += ids.count();

            QList<QVariant> v;
            CoreDbAccess  access;
            q += QString::fromUtf8("Images.album IN (");
            access.db()->addBoundValuePlaceholders(q, ids.size());
            q += QString::fromUtf8(");");
            access.backend()->execSql(q, ids, &v);

            values += v;
        }
    }
    else
    {
        CoreDbAccess access;
        query += QString::fromUtf8("Images.album = ?;");
        access.backend()->execSql(query, albumIds, &values);
    }

    int width, height;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        ItemListerRecord record;
        record.imageID           = (*it).toLongLong();
        ++it;
        record.name              = (*it).toString();
        ++it;
        record.albumID           = (*it).toInt();
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

        record.albumRootID = albumRootId;

        receiver->receive(record);
    }
}

QSet<int> ItemLister::albumRootsToList() const
{
    if (!d->listOnlyAvailableImages)
    {
        return QSet<int>();    // invalid value, all album roots shall be listed
    }

    QList<CollectionLocation> locations = CollectionManager::instance()->allAvailableLocations();
    QSet<int>                 ids;

    foreach (const CollectionLocation& location, locations)
    {
        ids << location.id();
    }

    return ids;
}

} // namespace Digikam
