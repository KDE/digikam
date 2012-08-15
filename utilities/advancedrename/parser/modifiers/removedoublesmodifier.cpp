/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : a modifier for deleting duplicate words
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "removedoublesmodifier.moc"

// Qt includes

#include <QSet>
#include <QString>
#include <QStringList>

// KDE includes

#include <klineedit.h>
#include <klocale.h>

namespace Digikam
{

RemoveDoublesModifier::RemoveDoublesModifier()
    : Modifier(i18n("Remove Doubles"),
               i18n("Remove duplicate words"),
               "edit-copy")
{
    addToken("{removedoubles}", description());

    QRegExp reg("\\{removedoubles\\}");
    reg.setMinimal(true);
    setRegExp(reg);
}

QString RemoveDoublesModifier::parseOperation(ParseSettings& settings)
{
    QString result = settings.str2Modify;

    QSet<QString> knownWords;
    QStringList words = result.split(QChar(' '));
    QStringList newString;
    foreach(const QString& word, words)
    {
        if (!knownWords.contains(word))
        {
            knownWords.insert(word);
            newString << word;
        }
    }

    if (!newString.isEmpty())
    {
        result = newString.join(QChar(' '));
    }

    return result;
}

} // namespace Digikam
