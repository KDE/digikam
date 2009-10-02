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
              : SubParser(i18n("File Name"), i18n("Add the current file name"), SmallIcon("folder-image"))
{
    addTokenDescription("[file]", i18nc("image filename", "Filename"),
             i18n("Filename"));

    setRegExp("\\[file\\]");
}

void FilenameParser::parseOperation(const QString& parseString, const ParseInformation& info, ParseResults& results)
{
    QFileInfo fi(info.filePath);
    QString baseFileName = fi.baseName();

    QRegExp reg = regExp();
    reg.setCaseSensitivity(Qt::CaseInsensitive);

    // --------------------------------------------------------

    QString tmp;
    PARSE_LOOP_START(parseString, reg)
    {
        tmp = baseFileName;
    }
    PARSE_LOOP_END(parseString, reg, tmp, results)
}

} // namespace Digikam
