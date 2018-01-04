/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2013-05-18
 * Description : Wrapper class for face recognition
 *
 * Copyright (C)      2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "identity.h"

namespace Digikam
{

class Identity::Private : public QSharedData
{
public:

    Private()
        : id(-1)
    {
    }

public:

    int                    id;
    QMap<QString, QString> attributes;
};

Identity::Identity()
    : d(new Private)
{
}

Identity::Identity(const Identity& other)
{
    d = other.d;
}

Identity& Identity::operator=(const Identity& other)
{
    d = other.d;
    return *this;
}

Identity::~Identity()
{
}

bool Identity::isNull() const
{
    return (d->id == -1);
}

bool Identity::operator==(const Identity& other) const
{
    return (d->id == other.d->id);
}

int Identity::id() const
{
    return d->id;
}

void Identity::setId(int id)
{
    d->id = id;
}

QString Identity::attribute(const QString& att) const
{
    return d->attributes.value(att);
}

void Identity::setAttribute(const QString& att, const QString& val)
{
    d->attributes[att] = val;
}

QMap<QString, QString> Identity::attributesMap() const
{
    return d->attributes;
}

void Identity::setAttributesMap(const QMap<QString, QString>& attributes)
{
    d->attributes = attributes;
}

} // namespace Digikam
