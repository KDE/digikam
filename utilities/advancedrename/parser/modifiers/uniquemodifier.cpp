/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-27
 * Description : a modifier for setting an additional string to a renaming option
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

#include "uniquemodifier.moc"

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

namespace Digikam
{

UniqueModifier::UniqueModifier()
              : Modifier(i18nc("unique value for duplicate strings", "Unique"),
                         i18n("Add a suffix number to have unique strings on duplicate values"),
                         SmallIcon("button_more"))
{
    addToken("{unique}", description());

    QRegExp reg("\\{unique\\}");
    reg.setMinimal(true);
    setRegExp(reg);
}

QString UniqueModifier::modifyOperation(const QString& parseString, const QString& result)
{
    cache << result;

    QRegExp reg = regExp();
    int pos     = 0;
    pos         = reg.indexIn(parseString, pos);
    if (pos > -1)
    {
        if (cache.count(result) > 1)
        {
            QString tmp  = result;
            int index    = cache.count(result) - 1;
            tmp         += QString("_%1").arg(QString::number(index));
            return tmp;
        }
    }
    return result;
}

void UniqueModifier::reset()
{
    cache.clear();
}

} // namespace Digikam
