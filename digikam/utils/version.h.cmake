/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-09
 * Description : digiKam release ID header.
 *
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_VERSION_H
#define DIGIKAM_VERSION_H

// Qt includes.

#include <QString>

// KDE includes.

#include <klocale.h>
#include <klocalizedstring.h>

// Local includes.

#include "gitversion.h"

namespace Digikam
{

static const char digikam_version_short[]  = "@DIGIKAM_VERSION_SHORT@";
static const char digikam_version[]        = "@DIGIKAM_VERSION_STRING@";
static const char digikam_version_suffix[] = "@DIGIKAM_SUFFIX_VERSION@";

static inline const QString digiKamVersion()
{
    return QString(digikam_version);
}

static inline KLocalizedString additionalInformation()
{
    QString gitVer       = QString(GITVERSION);
    KLocalizedString ret = ki18n("IRC:\n"
                                 "irc.freenode.net - #digikam\n\n"
                                 "Feedback:\n"
                                 "digikam-devel@kde.org\n\n"
                                 "Build date: %1 (target: %2)")
                                 .subs(__DATE__)
                                 .subs("@CMAKE_BUILD_TYPE@");

    if (!gitVer.isEmpty() && !gitVer.startsWith("unknow") && !gitVer.startsWith("export"))
    {
        ret = ki18n("IRC:\n"
                    "irc.freenode.net - #digikam\n\n"
                    "Feedback:\n"
                    "digikam-devel@kde.org\n\n"
                    "Build date: %1 (target: %2)\n"
                    "Rev.: %3")
                    .subs(__DATE__)
                    .subs("@CMAKE_BUILD_TYPE@")
                    .subs(QString("<a href='http://commits.kde.org/digikam/%1'>%2</a>").arg(gitVer).arg(gitVer));
    }

    return ret;
}

}  // namespace Digikam

#endif /* DIGIKAM_VERSION_H */
