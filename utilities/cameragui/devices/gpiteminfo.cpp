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

#include "gpiteminfo.h"

// Qt includes

#include <QDataStream>

namespace Digikam
{

GPItemInfo::GPItemInfo()
{
    size             = -1;
    width            = -1;
    height           = -1;
    readPermissions  = -1;
    writePermissions = -1;
    downloaded       = DownloadUnknown;
}

GPItemInfo::~GPItemInfo()
{
}

bool GPItemInfo::isNull() const
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
           photoInfo.isNull();
}

KUrl GPItemInfo::url() const
{
    KUrl url;
    url.addPath(folder);
    url.setFileName(name);
    return url;
}

QDataStream& operator<<(QDataStream& ds, const GPItemInfo& info)
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

    return ds;
}

QDataStream& operator>>(QDataStream& ds, GPItemInfo& info)
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

    return ds;
}

}  // namespace Digikam
