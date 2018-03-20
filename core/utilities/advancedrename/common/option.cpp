/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an abstract option class
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

#include "option.h"

namespace Digikam
{

class Option::Private
{
public:

    Private()
    {}

    ParseResults parsedResults;
};

Option::Option(const QString& name, const QString& description)
    : Rule(name), d(new Private)
{
    setDescription(description);
}

Option::Option(const QString& name, const QString& description, const QString& icon)
    : Rule(name, icon), d(new Private)
{
    setDescription(description);
}

Option::~Option()
{
    delete d;
}

} // namespace Digikam
