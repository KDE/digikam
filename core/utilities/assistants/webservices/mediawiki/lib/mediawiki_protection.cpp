/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Vincent Garcia <xavier dot vincent dot garcia at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "mediawiki_protection.h"

// C++ includes

#include <algorithm>

namespace mediawiki
{

class Q_DECL_HIDDEN Protection::ProtectionPrivate
{
public:

    QString type;
    QString level;
    QString expiry;
    QString source;
};

Protection::Protection()
    : d(new ProtectionPrivate())
{
}

Protection::~Protection()
{
    delete d;
}

Protection::Protection(const Protection& other)
    : d(new ProtectionPrivate(*(other.d)))
{
}

Protection& Protection::operator=(Protection other)
{
    *d = *other.d;
    return *this;
}

bool Protection::operator==(const Protection& other) const
{
    return type()   == other.type()   &&
           level()  == other.level()  &&
           expiry() == other.expiry() &&
           source() == other.source();
}

void Protection::setType(const QString& type)
{
    d->type = type;
}

QString Protection::type() const
{
    return d->type;
}

void Protection::setLevel(const QString& level)
{
    d->level = level;
}

QString Protection::level() const
{
    return d->level;
}

void Protection::setExpiry(const QString& expiry)
{
    d->expiry = expiry;
}

QString Protection::expiry() const
{
    return d->expiry;
}

void Protection::setSource(const QString& source)
{
    d->source = source;
}

QString Protection::source() const
{
    return d->source;
}

} // namespace mediawiki
