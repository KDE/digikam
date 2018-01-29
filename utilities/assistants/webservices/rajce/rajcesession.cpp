/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * Copyright (C) 2011 by Lukas Krejci <krejci.l at centrum dot cz>
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

#include "rajcesession.h"

QDebug operator<<(QDebug d, const Digikam::RajceSession& s)
{
    QString     ss;
    QTextStream str(&ss);

    str << "SessionState[";
    str << "sessionToken='"       << s.sessionToken()     << "'";
    str << ", nickname='"         << s.nickname()         << "'";
    str << ", username='"         << s.username()         << "'";
    str << ", albumToken='"       << s.openAlbumToken()   << "'";
    str << ", lastErrorMessage='" << s.lastErrorMessage() << "'";
    str << ", lastErrorCode="     << s.lastErrorCode();
    str << ", maxWidth="          << s.maxWidth();
    str << ", maxHeight="         << s.maxHeight();
    str << ", imageQuality="      << s.imageQuality();
    str << ", albums=[";

    Digikam::RajceAlbum a;

    foreach(a, s.albums())
    {
        str << a << ", ";
    }

    str << "]";

    d << *str.string();

    return d;
}
