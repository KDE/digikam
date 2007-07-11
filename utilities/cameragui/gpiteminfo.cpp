/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-19
 * Description : camera item info container
 * 
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QDataStream>

// Local includes.

#include "gpiteminfo.h"

namespace Digikam
{

QDataStream& operator<<( QDataStream& ds, const GPItemInfo& info)
{
    qint64 mtime = (qint64)info.mtime;
    qint64 size  = (qint64)info.size;

    ds << info.name;
    ds << info.folder;
    ds << mtime;
    ds << info.mime;
    ds << size;
    ds << info.width;
    ds << info.height;
    ds << info.downloaded;
    ds << info.readPermissions;
    ds << info.writePermissions;

    return ds;
}

QDataStream& operator>>(QDataStream& ds, GPItemInfo& info)
{
    qint64 mtime;
    qint64 size;

    ds >> info.name;
    ds >> info.folder;
    ds >> mtime;
    ds >> info.mime;
    ds >> size;
    ds >> info.width;
    ds >> info.height;
    ds >> info.downloaded;
    ds >> info.readPermissions;
    ds >> info.writePermissions;

    info.mtime = (time_t)mtime;
    info.size  = (long)size;

    return ds;
}

}  // namespace Digikam
