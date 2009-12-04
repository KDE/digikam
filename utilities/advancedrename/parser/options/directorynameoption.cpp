/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-02
 * Description : an option to provide directory information to the parser
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

#include "directorynameoption.moc"

// Qt includes

#include <QFileInfo>
#include <QString>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

namespace Digikam
{

DirectoryNameOption::DirectoryNameOption()
                   : Option(i18nc("renaming option for directory related information",
                                  "Directory"),
                            i18n("Add the directory name"), SmallIcon("folder"))
{
    addToken("[dir]",  i18n("Directory name"));
    addToken("[dir.]", i18n("Directory name of the parent, additional '.' characters move up "
                            "in the directory hierarchy"));

    QRegExp reg("\\[dir(\\.*)\\]");
    reg.setMinimal(true);
    setRegExp(reg);
}

QString DirectoryNameOption::parseOperation(const QRegExp& regExp, ParseSettings& settings)
{
    QFileInfo fi(settings.fileUrl.toLocalFile());
    QStringList folders = fi.absolutePath().split('/', QString::SkipEmptyParts);
    int folderCount     = folders.count();

    QString result;
    int matchedLength = regExp.cap(1).length();

    if (matchedLength == 0)
    {
        result = folders.last();
    }
    else if (matchedLength > (folderCount - 1))
    {
        result.clear();
    }
    else
    {
        result = folders[folderCount - matchedLength - 1];
    }

    return result;
}

} // namespace Digikam
