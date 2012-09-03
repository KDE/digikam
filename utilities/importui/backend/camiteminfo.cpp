/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-19
 * Description : camera item info container
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
           name.isNull()                         &&
           folder.isNull()                       &&
           mime.isNull()                         &&
           mtime.isNull()                        &&
           photoInfo.isNull()                    &&
           downloadName.isNull();
}

KUrl CamItemInfo::url() const
{
    KUrl url;
    url.addPath(folder);
    url.setFileName(name);
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
    bool b9  = mtime            == info.mtime;
    bool b10 = photoInfo        == info.photoInfo;
    bool b11 = id               == info.id;

    return b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10 && b11;
}

QDataStream& operator<<(QDataStream& ds, const CamItemInfo& info)
{
    ds << info.name;
    ds << info.folder;
    ds << info.mtime;
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

    return ds;
}

QDataStream& operator>>(QDataStream& ds, CamItemInfo& info)
{
    ds >> info.name;
    ds >> info.folder;
    ds >> info.mtime;
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
    dbg.nospace() << "CamItemInfo::mtime: "
                  << info.mtime << ", ";
    dbg.nospace() << "CamItemInfo::downloaded: "
                  << info.downloaded;
    dbg.nospace() << "CamItemInfo::downloadName: "
                  << info.downloadName;
    dbg.nospace() << "CamItemInfo::id: "
                  << info.id;
    return dbg.space();
}

}  // namespace Digikam
