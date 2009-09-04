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
namespace ManualRename
{

DirectoryNameParser::DirectoryNameParser()
              : Parser(i18n("Directory Name"), SmallIcon("folder"))
{
    addToken("[dir]", i18nc("current directory name", "Current"),
            i18n("current directory name"));

    addToken("[dir.]", i18nc("directory name", "Parent Directory Name"),
            i18n("directory name of the parent"));

    useTokenMenu(false);
}

void DirectoryNameParser::parse(QString& parseString, const ParseInformation& info)
{
    if (!stringIsValid(parseString))
        return;

    QFileInfo fi(info.filePath);
    QStringList folders = fi.absolutePath().split('/', QString::SkipEmptyParts);

    if (folders.isEmpty())
        return;

    QRegExp regExp("\\[dir(\\.*)\\]");
    regExp.setMinimal(true);

    int folderCount = folders.count();
    int pos         = 0;
    while (pos > -1)
    {
        pos = regExp.indexIn(parseString, pos);
        if (pos > -1)
        {
            QString tmp;

            int matchedLength = regExp.cap(1).length();

            if (matchedLength == 0)
                tmp = folders.last();
            else if (matchedLength > (folderCount - 1))
                tmp.clear();
            else
                tmp = folders[folderCount - matchedLength - 1];

            QString result = markResult(regExp.matchedLength(), tmp);
            parseString.replace(pos, regExp.matchedLength(), result);
            pos += result.count();
        }
    }
}

} // namespace ManualRename
} // namespace Digikam
