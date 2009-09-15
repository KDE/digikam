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

FilenameParser::FilenameParser()
              : SubParser(i18n("File Name"), SmallIcon("folder-image"))
{
    addToken("[file]", i18nc("image filename", "Filename"),
             i18n("image filename"));
}

void FilenameParser::parseOperation(QString& parseString, const ParseInformation& info)
{
    QFileInfo fi(info.filePath);
    QString baseFileName = fi.baseName();

    QRegExp regExp("\\[file\\]");
    regExp.setCaseSensitivity(Qt::CaseInsensitive);

    // --------------------------------------------------------

    QString tmp;
    PARSE_LOOP_START(parseString, regExp, tmp)
    {
        tmp = baseFileName;
    }
    PARSE_LOOP_END(parseString, regExp, tmp)
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

} // namespace Digikam
