/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide file information to the parser
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

#include "filepropertiesoption.moc"

// Qt includes

#include <QFileInfo>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

namespace Digikam
{

FilePropertiesOption::FilePropertiesOption()
                    : Option(i18n("File"), i18n("Add file properties"), SmallIcon("folder-image"))
{
    setUseTokenMenu(true);

    addToken("[file]", i18n("Filename"),
             i18nc("File name", "Name"));

    addToken("[ext]", i18n("File extension, prepend with a '.' character, to modify the real file extension"),
             i18nc("File extension", "Extension"));

    addToken("[user]", i18n("Owner of the file"),
             i18nc("Owner of the file", "Owner"));

    addToken("[userid]", i18n("Owner ID of the file"),
             i18nc("Owner id of the file", "Owner ID"));

    addToken("[group]", i18n("Group of the file"),
             i18nc("Group of the file", "Group"));

    addToken("[groupid]", i18n("Group ID of the file"),
             i18nc("Group id of the file", "Group ID"));

    QString regExpStr;
    regExpStr.append('(');
    regExpStr.append(escapeToken("[file]")).append('|');
    regExpStr.append(escapeToken("[user]")).append('|');
    regExpStr.append(escapeToken("[userid]")).append('|');
    regExpStr.append(escapeToken("[group]")).append('|');
    regExpStr.append(escapeToken("[groupid]")).append('|');
    regExpStr.append("(\\.?)").append(escapeToken("[ext]"));
    regExpStr.append(')');

    QRegExp reg(regExpStr);
    reg.setMinimal(true);
    setRegExp(reg);
}

QString FilePropertiesOption::parseOperation(ParseSettings& settings)
{
    QFileInfo fi(settings.fileUrl.toLocalFile());
    const QRegExp& reg   = regExp();
    const QString& token = reg.cap(1);

    QString result;
    if (token == QString("[file]"))
    {
        result = fi.completeBaseName();
    }
    else if (token == QString("[user]"))
    {
        result = fi.owner();
    }
    else if (token == QString("[userid]"))
    {
        result = QString::number(fi.ownerId());
    }
    else if (token == QString("[group]"))
    {
        result = fi.group();
    }
    else if (token == QString("[groupid]"))
    {
        result = QString::number(fi.groupId());
    }
    else if (token == QString("[ext]"))
    {
        result = fi.suffix();
    }
    else if (token == QString(".[ext]"))
    {
        result = "." + fi.suffix();
        settings.useOriginalFileExtension = false;
    }

    return result;
}

} // namespace Digikam
