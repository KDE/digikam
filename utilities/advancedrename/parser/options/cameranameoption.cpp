/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide camera information to the parser
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

#include "cameranameoption.moc"

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "parser.h"

namespace Digikam
{

CameraNameOption::CameraNameOption()
                : Option(i18n("Camera"), i18n("Add the camera name"), SmallIcon("camera-photo"))
{
    QString token = "[cam]";
    addToken(token, i18n("Camera name"));

    QRegExp reg(escapeToken(token));
    reg.setMinimal(true);
    setRegExp(reg);
}

QString CameraNameOption::parseOperation(ParseSettings& settings)
{
    return Parser::parseStringIsValid(settings.cameraName) ? settings.cameraName : QString();
}

} // namespace Digikam
