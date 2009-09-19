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

namespace Digikam
{

RangeModifier::RangeModifier()
             : Modifier(QString("{<i>start</i>-<i>stop</i>}"), i18n("Range"),
                        i18n("display only a specific range (if stop is omitted, move to end of the string)"))
{
    setRegExp("\\{\\s*(\\d+)\\s*(-\\s*((-1|\\d+)\\s*)?)?\\}");
}

QString RangeModifier::modifyOperation(const QString& parseString, const QString& result)
{
    QRegExp reg = regExp();

    int pos = 0;
    pos     = reg.indexIn(parseString, pos);
    if (pos > -1)
    {
        /*
         * extract range parameters
         */

        bool ok = false;

        // if the start parameter can not be extracted, set it to 1
        int start = reg.cap(1).simplified().toInt(&ok);
        if (!ok)
            start = 1;

        // If no range is defined at all ({start}), set stop = start.
        // If the stop parameter is omitted ({start-}), set stop = -1 (end of string)
        ok = false;
        int stop;
        if (!reg.cap(2).isEmpty())
        {
            ok   = true;
            stop = (reg.cap(3).isEmpty()) ? -1 : reg.cap(4).simplified().toInt(&ok);
        }
        else
        {
            stop = start;
        }

        if (!ok)
            stop = start;

        // --------------------------------------------------------

        /*
         * replace the string according to the given range
         */

        if (start > result.count())
            return QString();

        if (stop > result.count())
            stop = -1;

        --start;
        if (stop != -1)
            --stop;

        if ((start < 0) || (stop < -1))
            return QString();

        if (stop == -1)
            stop = result.count() - 1;
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
