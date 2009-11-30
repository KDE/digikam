/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : trimmed token modifier
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

#include "trimmedmodifier.h"

// KDE includes

#include <klocale.h>
#include <kiconloader.h>

namespace Digikam
{

TrimmedModifier::TrimmedModifier()
               : Modifier(i18n("Trimmed"), i18n("Remove leading, trailing and extra whitespace"),
                          SmallIcon("edit-cut"))
{
    addToken("{trim}", description());

    QRegExp reg("\\{trim\\}");
    reg.setMinimal(true);
    setRegExp(reg);
}

QString TrimmedModifier::modifyOperation(const ParseSettings& settings)
{
    return settings.result2Modify.simplified();
}

} // namespace Digikam
