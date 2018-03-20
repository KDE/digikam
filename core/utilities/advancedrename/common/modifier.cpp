/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a class to manipulate the results of an renaming options
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

#include "modifier.h"

namespace Digikam
{

class Modifier::Private
{
public:

    Private()
    {}

    ParseResults parsedResults;
};

Modifier::Modifier(const QString& name, const QString& description)
    : Rule(name), d(new Private)
{
    setDescription(description);
}

Modifier::Modifier(const QString& name, const QString& description, const QString& icon)
    : Rule(name, icon), d(new Private)
{
    setDescription(description);
}

Modifier::~Modifier()
{
    delete d;
}

} // namespace Digikam
