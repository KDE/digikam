/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : a camera name parser class
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

#include "cameranameparser.h"
#include "cameranameparser.moc"

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

namespace Digikam
{
namespace ManualRename
{

CameraNameParser::CameraNameParser()
                : Parser(i18n("Camera Name"), SmallIcon("camera-photo"))
{
    addToken("[cam]", i18n("Camera Name"), i18n("camera name"));
}

void CameraNameParser::parse(QString& parseString, const ParseInformation& info)
{
    if (!stringIsValid(parseString))
        return;

    QString cameraName = info.cameraName;

    QRegExp regExp("\\[cam\\]");
    regExp.setCaseSensitivity(Qt::CaseInsensitive);

    int pos = 0;
    while (pos > -1)
    {
        pos  = regExp.indexIn(parseString, pos);
        if (pos > -1)
        {
            QString tmp    = stringIsValid(cameraName) ? cameraName : QString();
            QString result = markResult(regExp.matchedLength(), tmp);
            parseString.replace(pos, regExp.matchedLength(), result);
        }
    }
}

} // namespace ManualRename
} // namespace Digikam
