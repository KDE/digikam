/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-19
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qdatastream.h>

#include "gpiteminfo.h"

QDataStream& operator<<( QDataStream& ds, const GPItemInfo& info)
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

    return ds;
}
