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
    addToken("[ext]",  i18n("File extension, prepend with a '.' character, to modify the real file extension"),
             i18nc("File extension", "Extension"));

    QRegExp reg("(\\[file\\]|(\\.?)\\[ext\\])");
    reg.setMinimal(true);
    setRegExp(reg);
}

QString FilePropertiesOption::parseOperation(const QRegExp& regExp, ParseSettings& settings)
{
    QFileInfo fi(settings.fileUrl.toLocalFile());

    QString tmp;
    if (regExp.cap(1) == QString("[file]"))
    {
        tmp = fi.baseName();
    }
    else if (regExp.cap(1) == QString("[ext]"))
    {
        tmp = fi.suffix();
    }
    else if (regExp.cap(1) == QString(".[ext]"))
    {
        tmp = "." + fi.suffix();
        settings.useOriginalFileExtension = false;
    }

    return tmp;
}

} // namespace Digikam
