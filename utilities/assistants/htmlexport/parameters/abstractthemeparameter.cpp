/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

    QByteArray mInternalName;
    QString    mName;
    QString    mDefaultValue;
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
    d->mInternalName = internalName;
    d->mName         = group->readEntry(NAME_KEY);
    d->mDefaultValue = group->readEntry(DEFAULT_VALUE_KEY);
}

QByteArray AbstractThemeParameter::internalName() const
{
    return d->mInternalName;
}

QString AbstractThemeParameter::name() const
{
    return d->mName;
}

QString AbstractThemeParameter::defaultValue() const
{
    return d->mDefaultValue;
}

} // namespace Digikam
