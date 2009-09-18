/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-18
 * Description : a modifier for displaying only a range of a token result
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

#include "rangemodifier.h"

// KDE includes

#include <klocale.h>
#include <kdebug.h>

namespace Digikam
{

RangeModifier::RangeModifier()
             : Modifier(QString("[start, stop]"), i18n("Range"), i18n("display only a specific range"))
{
    setRegExp("\\[\\s*(\\d+)\\s*,\\s*(\\d+)\\s*\\]");
}

QString RangeModifier::modifyOperation(const QString& parseString, const QString& result)
{
    QRegExp reg = regExp();
    int pos = 0;

    pos     = reg.indexIn(parseString, pos);
    if (pos > -1)
    {
        bool ok   = false;

        int start = reg.cap(1).toInt(&ok);
        if (!ok)
            start = -1;

        int stop = reg.cap(2).toInt(&ok);
        if (!ok)
            stop = -1;

        if ((start == -1 || stop == -1))
            return QString();

        if ((start > result.count()) || (stop > result.count()))
            return QString();

        start -= 1;
        stop  -= 1;

        if ((start < 0) || (stop < 0))
            return QString();

        QString tmp;
        for (int i = start; i <= stop; ++i)
        {
            tmp.append(result.at(i));
        }
        return tmp;
    }
    return QString();
}

} // namespace Digikam
