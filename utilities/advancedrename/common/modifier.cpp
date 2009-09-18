/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a token result modifier class
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

#include "modifier.h"

namespace Digikam
{

class ModifierPriv
{
public:

    ModifierPriv()
    {
    }

    QString id;
    QString alias;
    QString description;
};

// --------------------------------------------------------

Modifier::Modifier(const QString& id, const QString& alias, const QString& description)
        : d(new ModifierPriv)
{
    d->id          = id;
    d->alias       = alias;
    d->description = description;
}

QString Modifier::id() const
{
    return d->id;
}

QString Modifier::alias() const
{
    return d->alias;
}

QString Modifier::description() const
{
    return d->description;
}

QString Modifier::modify(const QString& str)
{
    if (str.isEmpty())
        return QString();

    return modifyOperation(str);
}

} // namespace Digikam
