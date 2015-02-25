/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-02
 * Description : an option to provide directory information to the parser
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

#include "directorynameoption.h"

// Qt includes

#include <QFileInfo>
#include <QString>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

DirectoryNameOption::DirectoryNameOption()
    : Option(i18nc("renaming option for directory related information",
                   "Directory"),
             i18n("Add the directory name"),
             QLatin1String("folder"))
{
    addToken(QLatin1String("[dir]"),  i18n("Directory name"));
    addToken(QLatin1String("[dir.]"), i18n("Name of the parent directory, additional '.' characters move up "
                                           "in the directory hierarchy"));

    QRegExp reg(QLatin1String("\\[dir(\\.*)\\]"));
    reg.setMinimal(true);
    setRegExp(reg);
}

QString DirectoryNameOption::parseOperation(ParseSettings& settings)
{
    QString result;
    QFileInfo fi(settings.fileUrl.toLocalFile());

    if (!fi.exists())
    {
        return result;
    }

    QStringList folders = fi.absolutePath().split(QLatin1Char('/'), QString::SkipEmptyParts);
    int folderCount     = folders.count();
    const QRegExp& reg  = regExp();
    int matchedLength   = reg.cap(1).length();

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
        result = folders.at(folderCount - matchedLength - 1);
    }

    return result;
}

} // namespace Digikam
