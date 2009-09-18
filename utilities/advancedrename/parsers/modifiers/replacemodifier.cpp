/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-18
 * Description : a modifier for replacing text in a token result
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

#include "replacemodifier.h"

// KDE includes

#include <klocale.h>

namespace Digikam
{

ReplaceModifier::ReplaceModifier()
               : Modifier(QString("{'old', 'new'}"), i18n("Replace"), i18n("replace text"))
{
    setRegExp("\\{\\s*'(.+)'\\s*,\\s*'(.*)'\\s*\\}");
}

QString ReplaceModifier::modifyOperation(const QString& parseString, const QString& result)
{
    QRegExp reg = regExp();
    int pos = 0;

    pos     = reg.indexIn(parseString, pos);
    if (pos > -1)
    {
        QString original    = reg.cap(1);
        QString replacement = reg.cap(2);

        QString _result = result;
        _result.replace(original, replacement);
        return _result;
    }
    return QString();
}

} // namespace Digikam
