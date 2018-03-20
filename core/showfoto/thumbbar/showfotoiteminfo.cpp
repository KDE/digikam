/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-1
 * Description : Showfoto item info container
 *
 * Copyright (C) 2013 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "showfotoiteminfo.h"

// Qt includes

#include <QDataStream>

namespace ShowFoto
{

ShowfotoItemInfo::ShowfotoItemInfo()
{
    size             = -1;
    id               = -1;
    width            = 0;
    height           = 0;
}

ShowfotoItemInfo::~ShowfotoItemInfo()
{
}

bool ShowfotoItemInfo::isNull() const
{
    return (size             == -1) &&
           (id               == -1) &&
           name.isNull()            &&
           folder.isNull()          &&
           mime.isNull();
}

bool ShowfotoItemInfo::operator==(const ShowfotoItemInfo& info) const
{
    bool b1  = size      == info.size;
    bool b2  = name      == info.name;
    bool b3  = folder    == info.folder;
    bool b4  = mime      == info.mime;
    bool b5  = id        == info.id;
    bool b6  = photoInfo == info.photoInfo;

    return (b1 && b2 && b3 && b4 && b5 && b6);
}

bool ShowfotoItemInfo::operator!=(const ShowfotoItemInfo& info) const
{
    return !operator==(info);
}

QDataStream& operator<<(QDataStream& ds, const ShowfotoItemInfo& info)
{
    ds << info.name;
    ds << info.folder;
    ds << info.mime;
    ds << info.size;
    ds << info.id;
    ds << info.photoInfo;

    return ds;
}

QDataStream& operator>>(QDataStream& ds, ShowfotoItemInfo& info)
{
    ds >> info.name;
    ds >> info.folder;
    ds >> info.mime;
    ds >> info.size;
    ds >> info.id;
    ds >> info.photoInfo;

    return ds;
}

QDebug operator<<(QDebug dbg, const ShowfotoItemInfo& info)
{
    dbg.nospace() << "ShowfotoItemInfo::size: "
                  << info.size << endl;
    dbg.nospace() << "ShowfotoItemInfo::name: "
                  << info.name << endl;
    dbg.nospace() << "ShowfotoitemInfo::folder: "
                  << info.folder << endl;
    dbg.nospace() << "ShowfotoItemInfo::mime: "
                  << info.mime << endl;
    dbg.nospace() << "ShowfotoItemInfo::id: "
                  << info.id << endl << endl;

    return dbg.space();
}

} // namespace Showfoto
