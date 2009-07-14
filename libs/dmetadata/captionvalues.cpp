/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-13
 * Description : caption values container
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "captionvalues.h"

// KDE includes

#include <kdebug.h>

using namespace KExiv2Iface;

namespace Digikam
{

CaptionValues::CaptionValues()
{
}

CaptionValues::~CaptionValues()
{
}

bool CaptionValues::operator==(const CaptionValues& val) const
{
    bool b1  = author  == val.author;
    bool b2  = caption == val.caption;
    bool b3  = date    == val.date;

    return b1 && b2 && b3;
}

QDebug operator<<(QDebug dbg, const CaptionValues& val)
{
    dbg.nospace() << "CaptionValues::caption: "
                  << val.caption << ", ";
    dbg.nospace() << "CaptionValues::author: "
                  << val.author << ", ";
    dbg.nospace() << "CaptionValues::date: "
                  << val.date;
    return dbg.space();
}

CaptionsMap::CaptionsMap()
{
}

CaptionsMap::~CaptionsMap()
{
}

KExiv2::AltLangMap CaptionsMap::toAltLangMap() const
{
    KExiv2::AltLangMap map;
    for (CaptionsMap::const_iterator it = constBegin(); it != constEnd(); ++it)
    {
        map.insert(it.key(), (*it).caption);
    }
    return map;
}

void CaptionsMap::fromAltLangMap(const KExiv2::AltLangMap& map)
{
    clear();
    for (KExiv2::AltLangMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it)
    {
        CaptionValues val;
        val.caption = it.value();
        insert(it.key(), val);
    }
}

}  // namespace Digikam
