/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : Autodetect enfuse binary program and version
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#include "enfusebinary.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

double EnfuseBinary::getVersion() const
{
    return versionDouble;
}

bool EnfuseBinary::parseHeader(const QString& output)
{
    // Work around Enfuse <= 3.2
    // The output look like this : ==== enfuse, version 3.2 ====
    QString headerStartsOld = QLatin1String("==== enfuse, version ");
    QString firstLine = output.section(QChar::fromLatin1('\n'), m_headerLine, m_headerLine);

    qCDebug(DIGIKAM_GENERAL_LOG) << path() << " help header line: \n" << firstLine;

    if (firstLine.startsWith(m_headerStarts))
    {
        setVersion(firstLine.remove(0, m_headerStarts.length()));
        QStringList versionList = version().split(QChar::fromLatin1('.'));
        versionList.pop_back();
        versionDouble = versionList.join(QChar::fromLatin1('.')).toDouble();
        emit signalEnfuseVersion(versionDouble);
        qCDebug(DIGIKAM_GENERAL_LOG) << "Found " << path() << " version: " << version();
        return true;
    }
    else if (firstLine.startsWith(headerStartsOld))
    {
        setVersion(firstLine.remove(0, headerStartsOld.length()));
        QStringList versionList = version().split(QChar::fromLatin1('.'));
        versionList.pop_back();
        versionDouble = versionList.join(QChar::fromLatin1('.')).toDouble();
        emit signalEnfuseVersion(versionDouble);
        qCDebug(DIGIKAM_GENERAL_LOG) << "Found " << path() << " version: " << version();
        return true;
    }

    return false;
}

} // namespace Digikam
