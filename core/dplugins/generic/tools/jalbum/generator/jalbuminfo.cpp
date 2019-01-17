/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate jAlbum image galleries
 *
 * Copyright (C) 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "jalbuminfo.h"

// KDE includes

#include <kconfigbase.h>

namespace Digikam
{

JAlbumInfo::JAlbumInfo(DInfoInterface* const iface)
{
    m_iface     = iface;
    m_getOption = IMAGES;
}

JAlbumInfo::~JAlbumInfo()
{
}

QDebug operator<<(QDebug dbg, const JAlbumInfo& t)
{
    dbg.nospace() << "JAlbumInfo::Albums: "
                  << t.m_albumList << ", ";
    dbg.nospace() << "JAlbumInfo::DestUrl: "
                  << t.destUrl();
    dbg.nospace() << "JAlbumInfo::JarUrl: "
                  << t.jarUrl();
    dbg.nospace() << "JAlbumInfo::ImageSelectionTitle: "
                  << t.imageSelectionTitle();
    return dbg.space();
}

} // namespace Digikam
