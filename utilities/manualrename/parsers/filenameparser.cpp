/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : a filename parser class
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

#include "filenameparser.h"
#include "filenameparser.moc"

// Qt includes

#include <QFileInfo>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

namespace Digikam
{
namespace ManualRename
{

FilenameParser::FilenameParser()
              : Parser(i18n("File Name"), SmallIcon("folder-image"))
{
    addToken("$", i18nc("original filename", "Original"),
            i18n("filename (original)"));

    addToken("&", i18nc("uppercase filename", "Uppercase"),
             i18n("filename (uppercase)"));

    addToken("%", i18nc("lowercase filename", "Lowercase"),
             i18n("filename (lowercase)"));

    addToken("*", i18nc("first letter uppercase filename", "First Letter Uppercase"),
             i18n("filename (first letter of each word uppercase)"));
}

void FilenameParser::parseTokenString(QString& parseString, const ParseInformation& info)
{
    QFileInfo fi(info.filePath);
    QString baseFileName = fi.baseName();

    QRegExp regExp("\\*{1}");
    regExp.setMinimal(true);

    int pos = 0;
    while (pos > -1)
    {
        pos = regExp.indexIn(parseString, pos);
        if (pos > -1)
        {
            QString tmp    = firstLetterUppercase(baseFileName.toLower());
            QString result = markResult(regExp.matchedLength(), tmp);
            parseString.replace(pos, regExp.matchedLength(), result);
            pos += result.count();
        }
    }

    parseString.replace(QString('$'), markResult(1, baseFileName));
    parseString.replace(QString('&'), markResult(1, baseFileName.toUpper()));
    parseString.replace(QString('%'), markResult(1, baseFileName.toLower()));
}

QString FilenameParser::firstLetterUppercase(const QString& str)
{
    if (str.isEmpty())
        return str;

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

} // namespace ManualRename
} // namespace Digikam
