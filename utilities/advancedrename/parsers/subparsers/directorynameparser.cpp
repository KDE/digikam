/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-02
 * Description : a directory name parser class
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

#include "directorynameparser.h"
#include "directorynameparser.moc"

// Qt includes

#include <QFileInfo>
#include <QString>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

namespace Digikam
{

DirectoryNameParser::DirectoryNameParser()
              : SubParser(i18n("Directory Name"), SmallIcon("folder"))
{
    setUseTokenMenu(false);

    addToken("[dir]", i18nc("current directory name", "Current"),
            i18n("current directory name"));

    addToken("[dir.]", i18nc("directory name", "Parent Directory Name"),
            i18n("directory name of the parent, additional '.' characters move up "
                 "in the directory hierarchy"));
}

void DirectoryNameParser::parseOperation(const QString& parseString, const ParseInformation& info, ParseResults& results)
{
    QFileInfo fi(info.filePath);
    QStringList folders = fi.absolutePath().split('/', QString::SkipEmptyParts);

    QRegExp regExp("\\[dir(\\.*)\\]");
    regExp.setMinimal(true);

    int folderCount = folders.count();

    // --------------------------------------------------------

    QString tmp;
    PARSE_LOOP_START(parseString, regExp)
    {
        int matchedLength = regExp.cap(1).length();

        if (matchedLength == 0)
            tmp = folders.last();
        else if (matchedLength > (folderCount - 1))
            tmp.clear();
        else
            tmp = folders[folderCount - matchedLength - 1];
    }
    PARSE_LOOP_END(parseString, regExp, tmp, results)
}

} // namespace Digikam
