/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : first letter of each word uppercase modifier
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

#include "firstletterofeachworduppercasemodifier.h"

// KDE includes

#include <klocale.h>

namespace Digikam
{

FirstLetterEachWordUpperCaseModifier::FirstLetterEachWordUpperCaseModifier()
                                    : Modifier(QString("*"), i18n("First Letter Of Each Word Uppercase"),
                                               i18n("convert the first letter of each word to uppercase"))
{
}

QString FirstLetterEachWordUpperCaseModifier::modify(const QString& str)
{
    if (str.isEmpty())
        return QString();

    QString tmp = str.toLower();

    if( tmp[0].isLetter() )
        tmp[0] = tmp[0].toUpper();

    for( int i = 0; i < tmp.length(); ++i )
    {
        if( tmp[i+1].isLetter() && !tmp[i].isLetter() &&
                tmp[i] != '\'' && tmp[i] != '?' && tmp[i] != '`' )
        {
            tmp[i+1] = tmp[i+1].toUpper();
        }
    }
    return tmp;
}

} // namespace Digikam
