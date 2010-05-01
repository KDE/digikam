/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a class to manipulate the results of an renaming options
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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


#include "modifier.moc"

namespace Digikam
{

Modifier::Modifier(const QString& name, const QString& description)
        : Parseable(name)
{
    setDescription(description);
}

Modifier::Modifier(const QString& name, const QString& description, const QPixmap& icon)
        : Parseable(name, icon)
{
    setDescription(description);
}

QString Modifier::modify(const ParseSettings& settings, const QString& str2Modify)
{
    if (settings.parseString.isEmpty())
    {
        return QString();
    }

    const QRegExp& reg         = regExp();
    const QString& parseString = settings.parseString;

    QString result;

    int pos = 0;
    while (pos > -1)
    {
        pos = reg.indexIn(parseString, pos);
        if (pos > -1)
        {
            result += modifyOperation(settings, str2Modify);
            pos += reg.matchedLength();
        }
    }

    return result;
}

} // namespace Digikam
