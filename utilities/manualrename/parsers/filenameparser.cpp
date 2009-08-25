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
              : Parser(i18n("File Name"), SmallIcon("list-add-font"))
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

void FilenameParser::parse(QString& parseString, const ParseInformation& info)
{
    if (!stringIsValid(parseString))
        return;

    QFileInfo fi(info.fileName);
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
            QString result = markResult(tmp);
            parseString.replace(pos, regExp.matchedLength(), result);
            pos += result.count();
        }
    }

    parseString.replace(QString('$'), markResult(baseFileName));
    parseString.replace(QString('&'), markResult(baseFileName.toUpper()));
    parseString.replace(QString('%'), markResult(baseFileName.toLower()));
}

} // namespace ManualRename
} // namespace Digikam
