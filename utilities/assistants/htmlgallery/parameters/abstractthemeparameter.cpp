/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "abstractthemeparameter.h"

// Qt include

#include <QByteArray>
#include <QString>

// KDE includes

#include <kconfiggroup.h>

static const char* NAME_KEY          = "Name";
static const char* DEFAULT_VALUE_KEY = "Default";

namespace Digikam
{

class AbstractThemeParameter::Private
{
public:

    explicit Private()
    {
    }

    QByteArray internalName;
    QString    name;
    QString    defaultValue;
};

AbstractThemeParameter::AbstractThemeParameter()
    : d(new Private)
{
}

AbstractThemeParameter::~AbstractThemeParameter()
{
    delete d;
}

void AbstractThemeParameter::init(const QByteArray& internalName, const KConfigGroup* group)
{
    d->internalName = internalName;
    d->name         = group->readEntry(NAME_KEY);
    d->defaultValue = group->readEntry(DEFAULT_VALUE_KEY);
}

QByteArray AbstractThemeParameter::internalName() const
{
    return d->internalName;
}

QString AbstractThemeParameter::name() const
{
    return d->name;
}

QString AbstractThemeParameter::defaultValue() const
{
    return d->defaultValue;
}

} // namespace Digikam
