/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : modifier to change the case of a renaming option
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

#include "casemodifier.h"

// KDE includes

#include <klocale.h>

namespace Digikam
{

CaseModifier::CaseModifier()
    : Modifier(i18n("Change Case"), i18n("change the case of a renaming option"))
{
    setUseTokenMenu(true);

    addToken("{upper}",      i18n("Convert to uppercase"),
             i18n("Uppercase"));

    addToken("{lower}",      i18n("Convert to lowercase"),
             i18n("Lowercase"));

    addToken("{firstupper}", i18n("Convert the first letter of each word to uppercase"),
             i18n("First Letter of Each Word Uppercase"));

    QRegExp reg("\\{(firstupper|lower|upper)\\}");
    reg.setMinimal(true);
    setRegExp(reg);
}

QString CaseModifier::parseOperation(ParseSettings& settings)
{
    Q_UNUSED(settings);

    const QRegExp& reg   = regExp();
    const QString& token = reg.cap(1);
    QString result       = settings.str2Modify;

    if (token == QString("upper"))
    {
        result = upper(settings.str2Modify);
    }
    else if (token == QString("firstupper"))
    {
        result = firstupper(settings.str2Modify);
    }
    else if (token == QString("lower"))
    {
        result = lower(settings.str2Modify);
    }

    return result;
}

QString CaseModifier::firstupper(const QString& str2Modify)
{
    if (str2Modify.isNull() || str2Modify.isEmpty())
    {
        return QString();
    }

    QString result = str2Modify.toLower();

    if (result.at(0).isLetter())
    {
        result[0] = result.at(0).toUpper();
    }

    for (int i = 0; i < result.length() - 1; ++i)
    {
        if ( result.at(i + 1).isLetter() &&
             !result.at(i).isLetter()    &&
             result.at(i) != '\''        &&
             result.at(i) != '?'         &&
             result.at(i) != '`'
           )
        {
            result[i + 1] = result.at(i + 1).toUpper();
        }
    }

    return result;
}

QString CaseModifier::lower(const QString& str2Modify)
{
    QString result = str2Modify.toLower();
    return result;
}

QString CaseModifier::upper(const QString& str2Modify)
{
    QString result = str2Modify.toUpper();
    return result;
}

} // namespace Digikam
