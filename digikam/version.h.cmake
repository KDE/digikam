/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-09
 * Description : digiKam release ID header.
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes.

#include "svnversion.h"

static const char digikam_version_short[] = "${DIGIKAM_VERSION_SHORT}";
static const char digikam_version[]       = "${DIGIKAM_VERSION_STRING}";

static inline const QString digiKamVersion()
{
    // We only take the mixed revision
    QString svnVer      = QString(SVNVERSION).section(":", 0, 0);

    QString digiKamVer  = QString(digikam_version);
    if (!svnVer.isEmpty() && !svnVer.startsWith("unknow") && !svnVer.startsWith("export"))
        digiKamVer = i18nc("%1 is digiKam version, %2 is the svn revision", "%1 (rev.: %2)", digikam_version, svnVer);

    return digiKamVer;
}

#endif // DIGIKAM_VERSION_H
