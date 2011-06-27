/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-27
 * Description : lensfun config header
 *
 * Copyright (C) 2010-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "config-lensfun.h"

// Qt includes

#include <QFile>

// KDE includes

#include <kdebug.h>
#include <kstandarddirs.h>

#ifndef CONF_DATADIR

char* _lf_get_database_dir()
{
    QString dir(KStandardDirs::installPath("data") + QString("digikam/lensfun"));
    kDebug() << "Lensfun database dir: " << dir;
    return QFile::encodeName(dir).data();
}

#endif
