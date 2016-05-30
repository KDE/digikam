/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide file information to the parser
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

#include "filepropertiesoption.h"

// Qt includes

#include <QFileInfo>

// KDE includes

#include <klocalizedstring.h>

namespace
{
static const QString KEY_FILE(QLatin1String("[file]"));
static const QString KEY_EXT(QLatin1String("[ext]"));
static const QString KEY_USER(QLatin1String("[user]"));
static const QString KEY_GROUP(QLatin1String("[group]"));
}

namespace Digikam
{

FilePropertiesOption::FilePropertiesOption()
    : Option(i18n("File"),
             i18n("Add file properties"),
             QLatin1String("folder-pictures"))
{
    setUseTokenMenu(true);

    addToken(KEY_FILE, i18n("Filename"),
             i18nc("File name", "Name"));

    addToken(KEY_EXT, i18n("File extension, prepend with a '.' character, to modify the real file extension"),
             i18nc("File extension", "Extension"));

    addToken(KEY_USER, i18n("Owner of the file"),
             i18nc("Owner of the file", "Owner"));

    addToken(KEY_GROUP, i18n("Group of the file"),
             i18nc("Group of the file", "Group"));

    QString regExpStr;
    regExpStr.append(QLatin1Char('('));
    regExpStr.append(escapeToken(KEY_FILE)).append(QLatin1Char('|'));
    regExpStr.append(escapeToken(KEY_USER)).append(QLatin1Char('|'));
    regExpStr.append(escapeToken(KEY_GROUP)).append(QLatin1Char('|'));
    regExpStr.append(QLatin1String("(\\.?)")).append(escapeToken(KEY_EXT));
    regExpStr.append(QLatin1Char(')'));

    QRegExp reg(regExpStr);
    reg.setMinimal(true);
    setRegExp(reg);
}

QString FilePropertiesOption::parseOperation(ParseSettings& settings)
{
    QString result;
    QFileInfo fi(settings.fileUrl.toLocalFile());

    const QRegExp& reg   = regExp();
    const QString& token = reg.cap(1);

    if (token == KEY_FILE)
    {
        result = fi.completeBaseName();
    }
    else if (token == KEY_USER)
    {
        result = fi.owner();
    }
    else if (token == KEY_GROUP)
    {
        result = fi.group();
    }
    else if (token == KEY_EXT)
    {
        result = fi.suffix();
    }
    else if (token == (QLatin1Char('.') + KEY_EXT))
    {
        result                            = QLatin1Char('.') + fi.suffix();
        settings.useOriginalFileExtension = false;
    }

    return result;
}

} // namespace Digikam
