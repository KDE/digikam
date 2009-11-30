/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : lowercase modifier
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

#include "lowercasemodifier.h"

// KDE includes

#include <klocale.h>

namespace Digikam
{

LowerCaseModifier::LowerCaseModifier()
                 : Modifier(i18n("Lowercase"), i18n("Convert to lowercase"))
{
    addToken("{lower}", description());

    QRegExp reg("\\{lower\\}");
    reg.setMinimal(true);
    setRegExp(reg);
}

QString LowerCaseModifier::modifyOperation(const ParseSettings& settings)
{
    return settings.result2Modify.toLower();
}

} // namespace Digikam
