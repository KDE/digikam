/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Listing information from database.
 *
 * Copyright (C) 2005      by Renchi Raju    <renchi dot raju at gmail dot com>
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

ItemLister::ItemLister()
    : d(new Private)
{
}

ItemLister::~ItemLister()
{
    delete d;
}

void ItemLister::setRecursive(bool recursive)
{
    d->recursive = recursive;
}

void ItemLister::setListOnlyAvailable(bool listOnlyAvailable)
{
    d->listOnlyAvailableImages = listOnlyAvailable;
}

void ItemLister::setAllowExtraValues(bool useExtraValue)
{
    d->allowExtraValues = useExtraValue;
}

void ItemLister::list(ItemListerReceiver* const receiver,
                      const CoreDbUrl& url)
{
    if (url.isAlbumUrl())
    {
        int albumRootId = url.albumRootId();
        QString album   = url.album();

        listPAlbum(receiver, albumRootId, album);
    }
    else if (url.isTagUrl())
    {
        listTag(receiver, url.tagIds());
    }
    else if (url.isDateUrl())
    {
        listDateRange(receiver, url.startDate(), url.endDate());
    }
    else if (url.isMapImagesUrl())
    {
        double lat1, lat2, lon1, lon2;
        url.areaCoordinates(&lat1, &lat2, &lon1, &lon2);
        listAreaRange(receiver, lat1, lat2, lon1, lon2);
    }
}

void ItemLister::listDateRange(ItemListerReceiver* const receiver,
                               const QDate& startDate,
                               const QDate& endDate)
{
    QList<QVariant> values;

    {
        CoreDbAccess access;
        access.backend()->execSql(QString::fromUtf8("SELECT DISTINCT Images.id, Images.name, Images.album, "
                                          "       Albums.albumRoot, "
                                          "       ImageInformation.rating, Images.category, "
                                          "       ImageInformation.format, ImageInformation.creationDate, "
                                          "       Images.modificationDate, Images.fileSize, "
                                          "       ImageInformation.width, ImageInformation.height "
                                          " FROM Images "
                                          "       LEFT JOIN ImageInformation ON Images.id=ImageInformation.imageid "
                                          "       INNER JOIN Albums ON Albums.id=Images.album "
                                          " WHERE Images.status=1 "
                                          "   AND ImageInformation.creationDate < ? "
                                          "   AND ImageInformation.creationDate >= ? "
                                          " ORDER BY Images.album;"),
                                  QDateTime(endDate),
                                  QDateTime(startDate),
                                  &values);
    }

    QSet<int> albumRoots = albumRootsToList();
    int       width, height;

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

        receiver->receive(record);
    }
}

} // namespace Digikam
