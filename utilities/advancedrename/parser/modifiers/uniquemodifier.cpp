/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-27
 * Description : A modifier for appending a suffix number to a renaming option.
 *               This guarantees a unique string for duplicate values.
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
#include <kdebug.h>

namespace Digikam
{

UniqueModifier::UniqueModifier()
              : Modifier(i18nc("unique value for duplicate strings", "Unique"),
                         i18n("Add a suffix number to have unique strings in duplicate values"),
                         SmallIcon("button_more"))
{
    addToken("{unique}", description());

    QRegExp reg("\\{unique\\}");
    reg.setMinimal(true);
    setRegExp(reg);
}

QString UniqueModifier::modifyOperation(const ParseSettings& settings)
{
    ParseResults::ResultsKey key = settings.results.currentKey();
    ParseResults results         = settings.results;

    cache[key] << settings.result2Modify;

    if (cache[key].count(settings.result2Modify) > 1)
    {
        QString tmp = settings.result2Modify;
        int index   = cache[key].count(settings.result2Modify) - 1;
        tmp        += QString("_%1").arg(QString::number(index));
        return tmp;
    }

    return settings.result2Modify;
}

void UniqueModifier::reset()
{
    cache.clear();
}

} // namespace Digikam
