/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : Autodetects cpfind binary program and version
 *
 * Copyright (C) 2011-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#include "cpfindbinary.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

bool CPFindBinary::parseHeader(const QString& output)
{
    QStringList lines    = output.split(QChar::fromLatin1('\n'));
    m_developmentVersion = false;

    foreach(const QString& line, lines)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << path() << " help header line: \n" << line;

        if (line.contains(headerRegExp))
        {
            m_version = headerRegExp.cap(2);

            if (!headerRegExp.cap(1).isEmpty())
            {
                m_developmentVersion = true;
            }

            return true;
        }

        m_developmentVersion = true;
    }

    return false;
}

}  // namespace Digikam

