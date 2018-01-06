/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-19
 * Description : camera item info container
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "camiteminfo.h"

// Qt includes

#include <QDataStream>

namespace Digikam
{

CamItemInfo::CamItemInfo()
{
    size             = -1;
    width            = -1;
    height           = -1;
    readPermissions  = -1;
    writePermissions = -1;
    downloaded       = DownloadUnknown;
    id               = -1;
    rating           = 0;
    pickLabel        = 0;
    colorLabel       = 0;
    previewPossible  = false;
}

CamItemInfo::~CamItemInfo()
{
}

bool CamItemInfo::isNull() const
{
    return (size             == -1)              &&
           (width            == -1)              &&
           (height           == -1)              &&
           (readPermissions  == -1)              &&
           (writePermissions == -1)              &&
           (downloaded       == DownloadUnknown) &&
           (id               == -1)              &&
           (rating           == 0)               &&
           (pickLabel        == 0)               &&
           (colorLabel       == 0)               &&
           name.isNull()                         &&
           folder.isNull()                       &&
           mime.isNull()                         &&
           ctime.isNull()                        &&
           photoInfo.isNull()                    &&
           downloadName.isNull()                 &&
           tagIds.isEmpty();
}

QUrl CamItemInfo::url() const
{
    QUrl url = QUrl::fromLocalFile(folder);
    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + (name));

    return url;
}

bool CamItemInfo::operator==(const CamItemInfo& info) const
{
    bool b1  = size             == info.size;
    bool b2  = width            == info.width;
    bool b3  = height           == info.height;
    bool b4  = readPermissions  == info.readPermissions;
    bool b5  = writePermissions == info.writePermissions;
    bool b6  = name             == info.name;
    bool b7  = folder           == info.folder;
    bool b8  = mime             == info.mime;
    bool b9  = ctime            == info.ctime;
    bool b10 = photoInfo        == info.photoInfo;
    bool b11 = id               == info.id;
    bool b12 = rating           == info.rating;
    bool b13 = pickLabel        == info.pickLabel;
    bool b14 = colorLabel       == info.colorLabel;
    bool b15 = tagIds           == info.tagIds;

    return b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10 && b11 && b12 && b13 && b14 && b15;
}

bool CamItemInfo::operator!=(const CamItemInfo& info) const
{
    return !operator==(info);
}

QDataStream& operator<<(QDataStream& ds, const CamItemInfo& info)
{
    ds << info.name;
    ds << info.folder;
    ds << info.ctime;
    ds << info.mime;
    ds << info.size;
    ds << info.width;
    ds << info.height;
    ds << info.downloaded;
    ds << info.readPermissions;
    ds << info.writePermissions;
    ds << info.photoInfo;
    ds << info.downloadName;
    ds << info.id;
    ds << info.rating;
    ds << info.pickLabel;
    ds << info.colorLabel;
    ds << info.tagIds;

    return ds;
}

QDataStream& operator>>(QDataStream& ds, CamItemInfo& info)
{
    ds >> info.name;
    ds >> info.folder;
    ds >> info.ctime;
    ds >> info.mime;
    ds >> info.size;
    ds >> info.width;
    ds >> info.height;
    ds >> info.downloaded;
    ds >> info.readPermissions;
    ds >> info.writePermissions;
    ds >> info.photoInfo;
    ds >> info.downloadName;
    ds >> info.id;
    ds >> info.rating;
    ds >> info.pickLabel;
    ds >> info.colorLabel;
    ds >> info.tagIds;

    return ds;
}

QDebug operator<<(QDebug dbg, const CamItemInfo& info)
{
    dbg.nospace() << "CamItemInfo::size: "
                  << info.size << ", ";
    dbg.nospace() << "CamItemInfo::width: "
                  << info.width << ", ";
    dbg.nospace() << "CamItemInfo::height: "
                  << info.height << ", ";
    dbg.nospace() << "CamItemInfo::readPermissions: "
                  << info.readPermissions << ", ";
    dbg.nospace() << "CamItemInfo::writePermissions: "
                  << info.writePermissions << ", ";
    dbg.nospace() << "CamItemInfo::name: "
                  << info.name << ", ";
    dbg.nospace() << "CamItemInfo::folder: "
                  << info.folder << ", ";
    dbg.nospace() << "CamItemInfo::mime: "
                  << info.mime << ", ";
    dbg.nospace() << "CamItemInfo::ctime: "
                  << info.ctime << ", ";
    dbg.nospace() << "CamItemInfo::downloaded: "
                  << info.downloaded;
    dbg.nospace() << "CamItemInfo::downloadName: "
                  << info.downloadName;
    dbg.nospace() << "CamItemInfo::id: "
                  << info.id;
    dbg.nospace() << "CamItemInfo::rating: "
                  << info.rating;
    dbg.nospace() << "CamItemInfo::pickLabel: "
                  << info.pickLabel;
    dbg.nospace() << "CamItemInfo::colorLabel: "
                  << info.colorLabel;
    dbg.nospace() << "CamItemInfo::tagIds: "
                  << info.tagIds;
    dbg.nospace() << "CamItemInfo::previewPossible: "
                  << info.previewPossible;
    return dbg.space();
}

}  // namespace Digikam
