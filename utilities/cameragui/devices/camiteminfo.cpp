/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-19
 * Description : camera item info container
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

bool CamItemInfo::operator==(const CamItemInfo& t) const
{
    bool b1  = size             == t.size;
    bool b2  = width            == t.width;
    bool b3  = height           == t.height;
    bool b4  = readPermissions  == t.readPermissions;
    bool b5  = writePermissions == t.writePermissions;
    bool b6  = name             == t.name;
    bool b7  = folder           == t.folder;
    bool b8  = mime             == t.mime;
    bool b9  = mtime            == t.mtime;
    bool b10 = photoInfo        == t.photoInfo;

    return b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10;
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

    return ds;
}

}  // namespace Digikam
