/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : Template information container.
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

#include "template.h"

namespace Digikam
{

class TemplatePrivate
{
public:

    TemplatePrivate()
    {
        valid = false;
    }

    bool    valid;

    QString author;
    QString authorPosition;
    QString credit;
    QString copyright;
    QString rightUsageTerms;
    QString source;
    QString instructions;
};

Template::Template()
        : d(new TemplatePrivate)
{
}

Template::~Template()
{
    delete d;
}

Template::Template(const Template& t)
        : d(new TemplatePrivate)
{
    setAuthor(t.d->author);
    setAuthorPosition(t.d->authorPosition);
    setCredit(t.d->credit);
    setCopyright(t.d->copyright);
    setRightUsageTerms(t.d->rightUsageTerms);
    setSource(t.d->source);
    setInstructions(t.d->instructions);
    setValid(t.d->valid);
}

Template& Template::operator=(const Template& t)
{
    if (this != &t)
    {
        setAuthor(t.d->author);
        setAuthorPosition(t.d->authorPosition);
        setCredit(t.d->credit);
        setCopyright(t.d->copyright);
        setRightUsageTerms(t.d->rightUsageTerms);
        setSource(t.d->source);
        setInstructions(t.d->instructions);
        setValid(t.d->valid);
    }
    return *this;
}

bool Template::operator==(const Template& t) const
{                                                  
    return valid           == t.valid
        && author          == t.author
        && authorPosition  == t.authorPosition
        && credit          == t.credit
        && copyright       == t.copyright
        && rightUsageTerms == t.rightUsageTerms
        && source          == t.source
        && instructions    == t.instructions
      ;                                                        
}                                                             

void Template::setAuthor(const QString& author)
{
    d->author = author;
}

void Template::setAuthorPosition(const QString& authorPosition)
{
    d->authorPosition = authorPosition;
}

void Template::setCredit(const QString& credit)
{
    d->credit = credit;
}

void Template::setCopyright(const QString& copyright)
{
    d->copyright = copyright;
}

void Template::setRightUsageTerms(const QString& rightUsageTerms)
{
    d->rightUsageTerms = rightUsageTerms;
}

void Template::setSource(const QString& source)
{
    d->source = source;
}

void Template::setInstructions(const QString& instructions)
{
    d->instructions = instructions;
}

void Template::setValid(bool valid)
{
    d->valid = valid;
}

QString Template::author() const
{
    return d->author;
}

QString Template::authorPosition() const
{
    return d->authorPosition;
}

QString Template::credit() const
{
    return d->credit;
}

QString Template::copyright() const
{
    return d->copyright;
}

QString Template::rightUsageTerms() const
{
    return d->rightUsageTerms;
}

QString Template::source() const
{
    return d->source;
}

QString Template::instructions() const
{
    return d->instructions;
}

bool Template::valid() const
{
    return d->valid;
}

}  // namespace Digikam
