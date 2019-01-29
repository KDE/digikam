/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect jAlbum jar archive
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "jalbumjar.h"

// Qt includes

#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericJAlbumPlugin
{

JalbumJar::JalbumJar()
    : DBinaryIface(QLatin1String("JAlbum.jar"),
                   QLatin1String("jAlbum"),
                   QLatin1String("https://jalbum.net/"),
                   QLatin1String("jAlbum Export"),
                   QStringList(QLatin1String("-version")),  // TODO: to check version with "java -jar " prefix
                   i18n("jAlbum Gallery Generator."))
{
    setup();
}

JalbumJar::~JalbumJar()
{
}

bool JalbumJar::checkDirForPath(const QString& possibleDir)
{
    QString possiblePath = path(possibleDir);
    qCDebug(DIGIKAM_GENERAL_LOG) << "Testing " << possiblePath << "...";

    bool ret = QFile::exists(possiblePath);

    if (ret)
    {
        m_isFound = true;
        m_pathDir = possibleDir;
        writeConfig();

        qCDebug(DIGIKAM_GENERAL_LOG) << "Found " << path();
    }

    emit signalBinaryValid();
    return ret;
}

} // namespace DigikamGenericJAlbumPlugin
