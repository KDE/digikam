/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : Photographer information container.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "photographer.h"

namespace Digikam
{

class PhotographerPrivate
{
public:

    PhotographerPrivate()
    {
        valid = false;
    }

    bool    valid;

    QString author;
    QString authorName;
    QString credit;
    QString source;
    QString copyright;
};

Photographer::Photographer()
            : d(new PhotographerPrivate)
{
}

Photographer::~Photographer()
{
    delete d;
}

Photographer::Photographer(const Photographer& photographer)
            : d(new PhotographerPrivate)
{
    setAuthor(photographer.d->author);
    setAuthorName(photographer.d->authorName);
    setCredit(photographer.d->credit);
    setSource(photographer.d->source);
    setCopyright(photographer.d->copyright);
    setValid(photographer.d->valid);
}

Photographer& Photographer::operator=(const Photographer& photographer)
{
    if (this != &photographer)
    {
        setAuthor(photographer.d->author);
        setAuthorName(photographer.d->authorName);
        setCredit(photographer.d->credit);
        setSource(photographer.d->source);
        setCopyright(photographer.d->copyright);
        setValid(photographer.d->valid);
    }
    return *this;
}

void Photographer::setAuthor(const QString& author)
{
    d->author = author;
}

void Photographer::setAuthorName(const QString& authorName)
{
    d->authorName = authorName;
}

void Photographer::setCredit(const QString& credit)
{
    d->credit = credit;
}

void Photographer::setSource(const QString& source)
{
    d->source = source;
}

void Photographer::setCopyright(const QString& copyright)
{
    d->copyright = copyright;
}

void Photographer::setValid(bool valid)
{
    d->valid = valid;
}

QString Photographer::author() const
{
    return d->author;
}

QString Photographer::authorName() const
{
    return d->authorName;
}

QString Photographer::credit() const
{
    return d->credit;
}

QString Photographer::source() const
{
    return d->source;
}

QString Photographer::copyright() const
{
    return d->copyright;
}

bool Photographer::valid() const
{
    return d->valid;
}

}  // namespace Digikam
